// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/StatementBinderVisitor.hpp>
#include <cminor/binder/BoundCompileUnit.hpp>
#include <cminor/binder/BoundClass.hpp>
#include <cminor/binder/BoundFunction.hpp>
#include <cminor/binder/BoundStatement.hpp>
#include <cminor/binder/ExpressionBinder.hpp>
#include <cminor/binder/OverloadResolution.hpp>
#include <cminor/binder/Access.hpp>
#include <cminor/symbols/ConstantPoolInstallerVisitor.hpp>
#include <cminor/binder/TypeBinderVisitor.hpp>
#include <cminor/binder/Evaluator.hpp>
#include <cminor/symbols/PropertySymbol.hpp>
#include <cminor/symbols/IndexerSymbol.hpp>
#include <cminor/symbols/SymbolCreatorVisitor.hpp>
#include <cminor/ast/CompileUnit.hpp>
#include <cminor/ast/Literal.hpp>
#include <cminor/ast/Expression.hpp>
#include <cminor/ast/BasicType.hpp>
#include <cminor/util/Unicode.hpp>

namespace cminor { namespace binder {

using namespace cminor::unicode;

bool TerminatesFunction(StatementNode* statement, bool inForEverLoop, ContainerScope* containerScope, BoundCompileUnit& boundCompileUnit)
{
    switch (statement->GetNodeType())
    {
        case NodeType::compoundStatementNode:
        {
            CompoundStatementNode* compoundStatement = static_cast<CompoundStatementNode*>(statement);
            int n = compoundStatement->Statements().Count();
            for (int i = 0; i < n; ++i)
            {
                StatementNode* statement = compoundStatement->Statements()[i];
                if (TerminatesFunction(statement, inForEverLoop, containerScope, boundCompileUnit)) return true;
            }
            break;
        }
        case NodeType::ifStatementNode:
        {
            IfStatementNode* ifStatement = static_cast<IfStatementNode*>(statement);
            if (inForEverLoop || ifStatement->ElseS())
            {
                if (TerminatesFunction(ifStatement->ThenS(), inForEverLoop, containerScope, boundCompileUnit) &&
                    inForEverLoop || (ifStatement->ElseS() && TerminatesFunction(ifStatement->ElseS(), inForEverLoop, containerScope, boundCompileUnit)))
                {
                    return true;
                }
            }
            break;
        }
        case NodeType::whileStatementNode:
        {
            WhileStatementNode* whileStatement = static_cast<WhileStatementNode*>(statement);
            if (IsAlwaysTrue(containerScope, boundCompileUnit, whileStatement->Condition()))
            {
                if (TerminatesFunction(whileStatement->Statement(), true, containerScope, boundCompileUnit)) return true;
            }
            break;
        }
        case NodeType::doStatementNode:
        {
            DoStatementNode* doStatement = static_cast<DoStatementNode*>(statement);
            if (IsAlwaysTrue(containerScope, boundCompileUnit, doStatement->Condition()))
            {
                if (TerminatesFunction(doStatement->Statement(), true, containerScope, boundCompileUnit)) return true;
            }
            break;
        }
        case NodeType::forStatementNode:
        {
            ForStatementNode* forStatement = static_cast<ForStatementNode*>(statement);
            if (!forStatement->Condition() || IsAlwaysTrue(containerScope, boundCompileUnit, forStatement->Condition()))
            {
                if (TerminatesFunction(forStatement->ActionS(), true, containerScope, boundCompileUnit)) return true;
            }
            break;
        }
        default:
        {
            if (statement->IsFunctionTerminatingNode())
            {
                return true;
            }
            break;
        }
    }
    return false;
}

void CheckFunctionReturnPaths(FunctionSymbol* functionSymbol, CompoundStatementNode* bodyNode, const Span& span, ContainerScope* containerScope, BoundCompileUnit& boundCompileUnit);

void CheckFunctionReturnPaths(FunctionSymbol* functionSymbol, FunctionNode& functionNode, ContainerScope* containerScope, BoundCompileUnit& boundCompileUnit)
{
    CheckFunctionReturnPaths(functionSymbol, functionNode.Body(), functionNode.GetSpan(), containerScope, boundCompileUnit);
}

void CheckFunctionReturnPaths(FunctionSymbol* functionSymbol, CompoundStatementNode* bodyNode, const Span& span, ContainerScope* containerScope, BoundCompileUnit& boundCompileUnit)
{
    TypeSymbol* returnType = functionSymbol->ReturnType();
    if (!returnType || dynamic_cast<VoidTypeSymbol*>(returnType)) return;
    if (functionSymbol->IsExternal()) return;
    if (dynamic_cast<ClassTemplateSpecializationSymbol*>(functionSymbol->ContainingClass()) != nullptr && !bodyNode) return;
    if (functionSymbol->IsAbstract()) return;
    CompoundStatementNode* body = bodyNode;
    int n = body->Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statement = body->Statements()[i];
        if (TerminatesFunction(statement, false, containerScope, boundCompileUnit)) return;
    }
    throw Exception("not all control paths terminate in return or throw statement", span);
}

StatementBinderVisitor::StatementBinderVisitor(BoundCompileUnit& boundCompileUnit_) : 
    boundCompileUnit(boundCompileUnit_), containerScope(nullptr), boundClass(nullptr), function(nullptr), compoundStatement(nullptr), doNotInstantiate(false), instantiateRequested(false), 
    insideCatch(false), switchConditionType(nullptr), caseValueMap(nullptr), gotoCaseStatements(nullptr), gotoDefaultStatements(nullptr)
{
}

void StatementBinderVisitor::Visit(CompileUnitNode& compileUnitNode)
{
    compileUnitNode.GlobalNs()->Accept(*this);
    int n = int(boundCompileUnit.ClassNodes().size());
    for (int i = 0; i < n; ++i)
    {
        ClassNode* classNode = boundCompileUnit.ClassNodes()[i].get();
        classNode->Accept(*this);
    }
}

void StatementBinderVisitor::Visit(NamespaceNode& namespaceNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(namespaceNode);
    containerScope = symbol->GetContainerScope();
    int n = namespaceNode.Members().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* member = namespaceNode.Members()[i];
        member->Accept(*this);
    }
    containerScope = prevContainerScope;
}

void StatementBinderVisitor::Visit(ClassNode& classNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(classNode);
    ClassTypeSymbol* classTypeSymbol = dynamic_cast<ClassTypeSymbol*>(symbol);
    Assert(classTypeSymbol, "class type symbol expected");
    if (classTypeSymbol->IsClassTemplate())
    {
        ConstantPoolInstallerVisitor constantPoolInstallerVisitor(boundCompileUnit.GetAssembly().GetConstantPool());
        classNode.Accept(constantPoolInstallerVisitor);
        return;
    }
    containerScope = symbol->GetContainerScope();
    std::unique_ptr<BoundClass> currentClass(new BoundClass(classTypeSymbol));
    BoundClass* prevClass = boundClass;
    boundClass = currentClass.get();
    int n = classNode.Members().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* classMember = classNode.Members()[i];
        classMember->Accept(*this);
    }
    ConstructorSymbol* defaultConstructorSymbol = classTypeSymbol->DefaultConstructorSymbol();
    if (defaultConstructorSymbol)
    {
        if (!defaultConstructorSymbol->IsBound())
        {
            defaultConstructorSymbol->SetBound();
            std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(defaultConstructorSymbol));
            boundFunction->SetContainerScope(defaultConstructorSymbol->GetContainerScope());
            boundClass->AddMember(std::move(boundFunction));
        }
    }
    boundCompileUnit.AddBoundNode(std::move(currentClass));
    boundClass = prevClass;
    containerScope = prevContainerScope;
}

void StatementBinderVisitor::Visit(ConstructorNode& constructorNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(constructorNode);
    ConstructorSymbol* constructorSymbol = dynamic_cast<ConstructorSymbol*>(symbol);
    Assert(constructorSymbol, "constructor symbol expected");
    if (instantiateRequested)
    {
        if (!constructorSymbol->IsInstantiated() && constructorSymbol->IsInstantiationRequested() && constructorSymbol->TypesResolved())
        {
            constructorSymbol->ResetFlag(SymbolFlags::instantiationRequested);
            constructorSymbol->SetInstantiated();
            constructorSymbol->SetProject();
            constructorSymbol->SetAssembly(&boundCompileUnit.GetAssembly());
            boundCompileUnit.GetAssembly().GetConstantPool().Install(constructorSymbol->Name());
        }
        else
        {
            return;
        }
    }
    constructorSymbol->SetBaseConstructorCallGenerated();
    Assert(constructorSymbol, "constructor symbol expected");
    containerScope = symbol->GetContainerScope();
    std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(constructorSymbol));
    boundFunction->SetContainerScope(containerScope);
    BoundFunction* prevFunction = function;
    function = boundFunction.get();
    BoundCompoundStatement* prevCompoundStatement = compoundStatement;
    if (constructorNode.HasBody())
    {
        constructorNode.Body()->Accept(*this);
        compoundStatement = dynamic_cast<BoundCompoundStatement*>(statement.release());
        Assert(compoundStatement, "compound statement expected");
        function->SetBody(std::unique_ptr<BoundCompoundStatement>(compoundStatement));
        if (constructorNode.Initializer())
        {
            constructorNode.Initializer()->Accept(*this);
        }
        else 
        {
            ClassTypeSymbol* baseClass = boundClass->GetClassTypeSymbol()->BaseClass();
            if (baseClass)
            {
                if (!baseClass->DefaultConstructorSymbol())
                {
                    throw Exception("base class has no default constructor to call", constructorNode.GetSpan(), baseClass->GetSpan());
                }
                BoundFunctionCall* baseConstructorCall = new BoundFunctionCall(boundCompileUnit.GetAssembly(), baseClass->DefaultConstructorSymbol());
                BoundExpression* boundThisParam = new BoundParameter(boundCompileUnit.GetAssembly(), constructorSymbol->Parameters()[0]->GetType(), constructorSymbol->Parameters()[0]);
                BoundConversion* thisAsBase = new BoundConversion(boundCompileUnit.GetAssembly(), std::unique_ptr<BoundExpression>(boundThisParam),
                    boundCompileUnit.GetConversion(boundClass->GetClassTypeSymbol(), boundClass->GetClassTypeSymbol()->BaseClass()));
                baseConstructorCall->AddArgument(std::unique_ptr<BoundExpression>(thisAsBase));
                BoundExpressionStatement* baseConstructorCallStatement = new BoundExpressionStatement(boundCompileUnit.GetAssembly(), std::unique_ptr<BoundExpression>(baseConstructorCall));
                baseConstructorCallStatement->SetContainerScope(containerScope);
                compoundStatement->InsertFront(std::unique_ptr<BoundStatement>(baseConstructorCallStatement));
            }
        }
        Symbol* parent = constructorSymbol->Parent();
        ClassTypeSymbol* classTypeSymbol = dynamic_cast<ClassTypeSymbol*>(parent);
        Assert(classTypeSymbol, "class type symbol expected");
        if (classTypeSymbol->NeedsStaticInitialization())
        {
            ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
            std::u32string classTypeFullName = classTypeSymbol->FullName();
            Constant classNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(classTypeFullName.c_str())));
            BoundStaticInitStatement* boundStaticInitStatement = new BoundStaticInitStatement(boundCompileUnit.GetAssembly(), classNameConstant);
            boundStaticInitStatement->SetContainerScope(containerScope);
            compoundStatement->InsertFront(std::unique_ptr<BoundStatement>(boundStaticInitStatement));
        }
    }
    compoundStatement = prevCompoundStatement;
    if (!doNotInstantiate)
    {
        boundClass->AddMember(std::move(boundFunction));
    }
    containerScope = prevContainerScope;
    function = prevFunction;
}

void StatementBinderVisitor::Visit(StaticConstructorNode& staticConstructorNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(staticConstructorNode);
    StaticConstructorSymbol* staticConstructorSymbol = dynamic_cast<StaticConstructorSymbol*>(symbol);
    Assert(staticConstructorSymbol, "static constructor symbol expected");
    containerScope = symbol->GetContainerScope();
    std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(staticConstructorSymbol));
    boundFunction->SetContainerScope(containerScope);
    BoundFunction* prevFunction = function;
    function = boundFunction.get();
    BoundCompoundStatement* prevCompoundStatement = compoundStatement;
    if (staticConstructorNode.HasBody())
    {
        ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
        staticConstructorNode.Body()->Accept(*this);
        compoundStatement = dynamic_cast<BoundCompoundStatement*>(statement.release());
        Assert(compoundStatement, "compound statement expected");
        Symbol* parent = staticConstructorSymbol->Parent();
        ClassTypeSymbol* classTypeSymbol = dynamic_cast<ClassTypeSymbol*>(parent);
        Assert(classTypeSymbol, "class type symbol expected");
        if (classTypeSymbol->BaseClass() && classTypeSymbol->BaseClass()->NeedsStaticInitialization())
        {
            std::u32string baseClassFullName = classTypeSymbol->BaseClass()->FullName();
            Constant baseClassNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(baseClassFullName.c_str())));
            BoundStaticInitStatement* boundStaticInitStatement = new BoundStaticInitStatement(boundCompileUnit.GetAssembly(), baseClassNameConstant);
            boundStaticInitStatement->SetContainerScope(containerScope);
            compoundStatement->InsertFront(std::unique_ptr<BoundStatement>(boundStaticInitStatement));
        }
        std::u32string classTypeFullName = classTypeSymbol->FullName();
        Constant classNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(classTypeFullName.c_str())));
        BoundStatement* boundDoneStaticInitStatement = new BoundDoneStaticInitStatement(boundCompileUnit.GetAssembly(), classNameConstant);
        boundDoneStaticInitStatement->SetContainerScope(containerScope);
        compoundStatement->AddStatement(std::unique_ptr<BoundStatement>(boundDoneStaticInitStatement));
        function->SetBody(std::unique_ptr<BoundCompoundStatement>(compoundStatement));
    }
    compoundStatement = prevCompoundStatement;
    if (!doNotInstantiate)
    {
        boundClass->AddMember(std::move(boundFunction));
    }
    containerScope = prevContainerScope;
    function = prevFunction;
}

void StatementBinderVisitor::Visit(BaseInitializerNode& baseInitializerNode)
{
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    ParameterSymbol* thisParam = function->GetFunctionSymbol()->Parameters()[0];
    BoundParameter* boundThisParam = new BoundParameter(boundCompileUnit.GetAssembly(), thisParam->GetType(), thisParam);
    FunctionSymbol* thisToBaseConversion = boundCompileUnit.GetConversion(thisParam->GetType(), boundClass->GetClassTypeSymbol()->BaseClass());
    if (!thisToBaseConversion)
    {
        throw Exception("no implicit conversion from '" + ToUtf8(thisParam->GetType()->FullName()) + "' to '" + ToUtf8(boundClass->GetClassTypeSymbol()->BaseClass()->FullName()) + "' exists.",
            baseInitializerNode.GetSpan());
    }
    BoundConversion* thisAsBase = new BoundConversion(boundCompileUnit.GetAssembly(), std::unique_ptr<BoundExpression>(boundThisParam), thisToBaseConversion);
    arguments.push_back(std::unique_ptr<BoundExpression>(thisAsBase));
    int n = baseInitializerNode.Arguments().Count();
    for (int i = 0; i < n; ++i)
    {
        arguments.push_back(BindExpression(boundCompileUnit, function, containerScope, baseInitializerNode.Arguments()[i]));
    }
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, boundClass->GetClassTypeSymbol()->BaseClass()->ClassOrNsScope()));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
    std::unique_ptr<BoundFunctionCall> baseConstructorCall = ResolveOverload(boundCompileUnit, U"@constructor", functionScopeLookups, arguments, baseInitializerNode.GetSpan());
    BoundExpressionStatement* baseConstructorCallStatement = new BoundExpressionStatement(boundCompileUnit.GetAssembly(), std::move(baseConstructorCall));
    baseConstructorCallStatement->SetContainerScope(containerScope);
    compoundStatement->InsertFront(std::unique_ptr<BoundStatement>(baseConstructorCallStatement));
}

void StatementBinderVisitor::Visit(ThisInitializerNode& thisInitializerNode)
{
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    ParameterSymbol* thisParam = function->GetFunctionSymbol()->Parameters()[0];
    BoundParameter* boundThisParam = new BoundParameter(boundCompileUnit.GetAssembly(), thisParam->GetType(), thisParam);
    arguments.push_back(std::unique_ptr<BoundExpression>(boundThisParam));
    int n = thisInitializerNode.Arguments().Count();
    for (int i = 0; i < n; ++i)
    {
        arguments.push_back(BindExpression(boundCompileUnit, function, containerScope, thisInitializerNode.Arguments()[i]));
    }
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, boundClass->GetClassTypeSymbol()->BaseClass()->ClassOrNsScope()));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
    std::unique_ptr<BoundFunctionCall> thisConstructorCall = ResolveOverload(boundCompileUnit, U"@constructor", functionScopeLookups, arguments, thisInitializerNode.GetSpan());
    if (thisConstructorCall->GetFunctionSymbol() == function->GetFunctionSymbol())
    {
        throw Exception("self-recursive initializer function call detected", thisInitializerNode.GetSpan());
    }
    BoundExpressionStatement* thisConstructorCallStatement = new BoundExpressionStatement(boundCompileUnit.GetAssembly(), std::move(thisConstructorCall));
    thisConstructorCallStatement->SetContainerScope(containerScope);
    compoundStatement->InsertFront(std::unique_ptr<BoundStatement>(thisConstructorCallStatement));
}

void StatementBinderVisitor::Visit(MemberFunctionNode& memberFunctionNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(memberFunctionNode);
    MemberFunctionSymbol* memberFunctionSymbol = dynamic_cast<MemberFunctionSymbol*>(symbol);
    Assert(memberFunctionSymbol, "member function symbol expected");
    if (memberFunctionSymbol->IsAbstract()) return;
    if (instantiateRequested)
    {
        if (!memberFunctionSymbol->IsInstantiated() && memberFunctionSymbol->IsInstantiationRequested() && memberFunctionSymbol->TypesResolved())
        {
            memberFunctionSymbol->ResetFlag(SymbolFlags::instantiationRequested);
            memberFunctionSymbol->SetInstantiated();
        }
        else
        {
            return;
        }
    }
    containerScope = symbol->GetContainerScope();
    std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(memberFunctionSymbol));
    boundFunction->SetContainerScope(containerScope);
    BoundFunction* prevFunction = function;
    function = boundFunction.get();
    if (memberFunctionNode.HasBody())
    {
        memberFunctionNode.Body()->Accept(*this);
        BoundCompoundStatement* compoundStatement = dynamic_cast<BoundCompoundStatement*>(statement.release());
        Assert(compoundStatement, "compound statement expected");
        if (memberFunctionSymbol->IsStatic())
        {
            Symbol* parent = memberFunctionSymbol->Parent();
            ClassTypeSymbol* classTypeSymbol = dynamic_cast<ClassTypeSymbol*>(parent);
            Assert(classTypeSymbol, "class type symbol expected");
            if (classTypeSymbol->NeedsStaticInitialization())
            {
                ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
                std::u32string classTypeFullName = classTypeSymbol->FullName();
                Constant classNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(classTypeFullName.c_str())));
                BoundStatement* boundStaticInitStatement = new BoundStaticInitStatement(boundCompileUnit.GetAssembly(), classNameConstant);
                boundStaticInitStatement->SetContainerScope(containerScope);
                compoundStatement->InsertFront(std::unique_ptr<BoundStatement>(boundStaticInitStatement));
            }
        }
        function->SetBody(std::unique_ptr<BoundCompoundStatement>(compoundStatement));
    }
    CheckFunctionReturnPaths(memberFunctionSymbol, memberFunctionNode, containerScope, boundCompileUnit);
    if (!doNotInstantiate)
    {
        boundClass->AddMember(std::move(boundFunction));
    }
    containerScope = prevContainerScope;
    function = prevFunction;
}

void StatementBinderVisitor::Visit(FunctionNode& functionNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(functionNode);
    FunctionSymbol* functionSymbol = dynamic_cast<FunctionSymbol*>(symbol);
    Assert(functionSymbol, "function symbol expected");
    containerScope = symbol->GetContainerScope();
    std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(functionSymbol));
    boundFunction->SetContainerScope(containerScope);
    BoundFunction* prevFunction = function;
    function = boundFunction.get();
    if (functionNode.HasBody())
    {
        functionNode.Body()->Accept(*this);
        BoundCompoundStatement* compoundStatement = dynamic_cast<BoundCompoundStatement*>(statement.release());
        Assert(compoundStatement, "compound statement expected");
        function->SetBody(std::unique_ptr<BoundCompoundStatement>(compoundStatement));
    }
    CheckFunctionReturnPaths(functionSymbol, functionNode, containerScope, boundCompileUnit);
    boundCompileUnit.AddBoundNode(std::move(boundFunction));
    containerScope = prevContainerScope;
    function = prevFunction;
}

void StatementBinderVisitor::Visit(PropertyNode& propertyNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(propertyNode);
    PropertySymbol* propertySymbol = dynamic_cast<PropertySymbol*>(symbol);
    Assert(propertySymbol, "property symbol expected");
    prevContainerScope = containerScope;
    PropertyGetterFunctionSymbol* getter = propertySymbol->Getter();
    PropertySetterFunctionSymbol* setter = propertySymbol->Setter();
    bool instantiateGetter = true;
    if (instantiateRequested)
    {
        if (getter)
        {
            if (!getter->IsInstantiated() && getter->IsInstantiationRequested() && getter->TypesResolved())
            {
                getter->ResetFlag(SymbolFlags::instantiationRequested);
                getter->SetInstantiated();
            }
            else
            {
                instantiateGetter = false;
            }
        }
    }
    if (getter && !doNotInstantiate && instantiateGetter)
    {
        prevContainerScope = containerScope;
        std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(getter));
        BoundFunction* prevFunction = function;
        function = boundFunction.get();
        containerScope = getter->GetContainerScope();
        boundFunction->SetContainerScope(containerScope);
        propertyNode.Getter()->Accept(*this);
        CheckFunctionReturnPaths(getter, propertyNode.Getter(), propertyNode.Getter()->GetSpan(), containerScope, boundCompileUnit);
        BoundCompoundStatement* compoundStatement = dynamic_cast<BoundCompoundStatement*>(statement.release());
        Assert(compoundStatement, "compound statement expected");
        function->SetBody(std::unique_ptr<BoundCompoundStatement>(compoundStatement));
        boundClass->AddMember(std::move(boundFunction));
        function = prevFunction;
        containerScope = prevContainerScope;
    }
    bool instantiateSetter = true;
    if (instantiateRequested)
    {
        if (setter)
        {
            if (!setter->IsInstantiated() && setter->IsInstantiationRequested() && setter->TypesResolved())
            {
                setter->ResetFlag(SymbolFlags::instantiationRequested);
                setter->SetInstantiated();
            }
            else
            {
                instantiateSetter = false;
            }
        }
    }
    if (setter && !doNotInstantiate && instantiateSetter)
    {
        prevContainerScope = containerScope;
        containerScope = setter->GetContainerScope();
        std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(setter));
        boundFunction->SetContainerScope(containerScope);
        BoundFunction* prevFunction = function;
        function = boundFunction.get();
        propertyNode.Setter()->Accept(*this);
        CheckFunctionReturnPaths(setter, propertyNode.Setter(), propertyNode.Setter()->GetSpan(), containerScope, boundCompileUnit);
        BoundCompoundStatement* compoundStatement = dynamic_cast<BoundCompoundStatement*>(statement.release());
        Assert(compoundStatement, "compound statement expected");
        function->SetBody(std::unique_ptr<BoundCompoundStatement>(compoundStatement));
        boundClass->AddMember(std::move(boundFunction));
        function = prevFunction;
        containerScope = prevContainerScope;
    }
    containerScope = prevContainerScope;
}

void StatementBinderVisitor::Visit(IndexerNode& indexerNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(indexerNode);
    IndexerSymbol* indexerSymbol = dynamic_cast<IndexerSymbol*>(symbol);
    Assert(indexerSymbol, "indexer symbol expected");
    prevContainerScope = containerScope;
    IndexerGetterFunctionSymbol* getter = indexerSymbol->Getter();
    IndexerSetterFunctionSymbol* setter = indexerSymbol->Setter();
    bool instantiateGetter = true;
    if (instantiateRequested)
    {
        if (getter)
        {
            if (!getter->IsInstantiated() && getter->IsInstantiationRequested() && getter->TypesResolved())
            {
                getter->ResetFlag(SymbolFlags::instantiationRequested);
                getter->SetInstantiated();
            }
            else
            {
                instantiateGetter = false;
            }
        }
    }
    if (getter && !doNotInstantiate && instantiateGetter)
    {
        prevContainerScope = containerScope;
        std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(getter));
        BoundFunction* prevFunction = function;
        function = boundFunction.get();
        containerScope = getter->GetContainerScope();
        boundFunction->SetContainerScope(containerScope);
        indexerNode.Getter()->Accept(*this);
        CheckFunctionReturnPaths(getter, indexerNode.Getter(), indexerNode.Getter()->GetSpan(), containerScope, boundCompileUnit);
        BoundCompoundStatement* compoundStatement = dynamic_cast<BoundCompoundStatement*>(statement.release());
        Assert(compoundStatement, "compound statement expected");
        function->SetBody(std::unique_ptr<BoundCompoundStatement>(compoundStatement));
        boundClass->AddMember(std::move(boundFunction));
        function = prevFunction;
        containerScope = prevContainerScope;
    }
    bool instantiateSetter = true;
    if (instantiateRequested)
    {
        if (setter)
        {
            if (!setter->IsInstantiated() && setter->IsInstantiationRequested() && setter->TypesResolved())
            {
                setter->ResetFlag(SymbolFlags::instantiationRequested);
                setter->SetInstantiated();
            }
            else
            {
                instantiateSetter = false;
            }
        }
    }
    if (setter && !doNotInstantiate && instantiateSetter)
    {
        prevContainerScope = containerScope;
        containerScope = setter->GetContainerScope();
        std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(setter));
        boundFunction->SetContainerScope(containerScope);
        BoundFunction* prevFunction = function;
        function = boundFunction.get();
        indexerNode.Setter()->Accept(*this);
        CheckFunctionReturnPaths(setter, indexerNode.Setter(), indexerNode.Setter()->GetSpan(), containerScope, boundCompileUnit);
        BoundCompoundStatement* compoundStatement = dynamic_cast<BoundCompoundStatement*>(statement.release());
        Assert(compoundStatement, "compound statement expected");
        function->SetBody(std::unique_ptr<BoundCompoundStatement>(compoundStatement));
        boundClass->AddMember(std::move(boundFunction));
        function = prevFunction;
        containerScope = prevContainerScope;
    }
    containerScope = prevContainerScope;
}

void StatementBinderVisitor::Visit(CompoundStatementNode& compoundStatementNode)
{
    ContainerScope* prevContainerScope = containerScope;
    containerScope = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(compoundStatementNode)->GetContainerScope();
    BoundCompoundStatement* prevCompound = compoundStatement;
    std::unique_ptr<BoundCompoundStatement> boundCompoundStatement(new BoundCompoundStatement(boundCompileUnit.GetAssembly()));
    compoundStatement = boundCompoundStatement.get();
    boundCompoundStatement->SetContainerScope(containerScope);
    boundCompoundStatement->SetBeginBraceSpan(compoundStatementNode.BeginBraceSpan());
    boundCompoundStatement->SetEndBraceSpan(compoundStatementNode.EndBraceSpan());
    boundCompoundStatement->SetSpan(compoundStatementNode.GetSpan());
    int n = compoundStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = compoundStatementNode.Statements()[i];
        statementNode->Accept(*this);
        statement->SetSpan(statementNode->GetSpan());
        if (statementNode->Label())
        {
            statement->SetLabel(statementNode->Label()->Label());
        }
        boundCompoundStatement->AddStatement(std::unique_ptr<BoundStatement>(statement.release()));
    }
    containerScope = prevContainerScope;
    statement.reset(boundCompoundStatement.release());
    if (compoundStatementNode.Label())
    {
        statement->SetLabel(compoundStatementNode.Label()->Label());
    }
    compoundStatement = prevCompound;
}

void StatementBinderVisitor::Visit(ReturnStatementNode& returnStatementNode)
{
    if (returnStatementNode.Expression())
    {
        TypeSymbol* returnType = function->GetFunctionSymbol()->ReturnType();
        bool returnDelegateType = returnType->IsDelegateType();
        bool returnClassDelegateType = returnType->IsClassDelegateType();
        if (returnType && !dynamic_cast<VoidTypeSymbol*>(returnType))
        {
            std::vector<std::unique_ptr<BoundExpression>> returnTypeArgs;
            BoundTypeExpression* boundTypeExpression = new BoundTypeExpression(boundCompileUnit.GetAssembly(), returnType);
            returnTypeArgs.push_back(std::unique_ptr<BoundTypeExpression>(boundTypeExpression));
            std::vector<FunctionScopeLookup> functionScopeLookups;
            functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
            functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, returnType->ClassInterfaceOrNsScope()));
            functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
            std::unique_ptr<BoundFunctionCall> returnFunctionCall = ResolveOverload(boundCompileUnit, U"@return", functionScopeLookups, returnTypeArgs, returnStatementNode.GetSpan());
            std::unique_ptr<BoundExpression> expression = BindExpression(boundCompileUnit, function, containerScope, returnStatementNode.Expression(), false, 
                returnDelegateType || returnClassDelegateType, returnClassDelegateType);
            std::vector<std::unique_ptr<BoundExpression>> returnValueArguments;
            returnValueArguments.push_back(std::move(expression));
            FunctionMatch functionMatch(returnFunctionCall->GetFunctionSymbol());
            bool conversionFound = FindConversions(boundCompileUnit, returnFunctionCall->GetFunctionSymbol(), returnValueArguments, functionMatch, ConversionType::implicit_);
            if (conversionFound)
            {
                Assert(!functionMatch.argumentMatches.empty(), "argument match expected");
                FunctionSymbol* conversionFun = functionMatch.argumentMatches[0].conversionFun;
                if (conversionFun)
                {
                    BoundConversion* boundConversion = new BoundConversion(boundCompileUnit.GetAssembly(),
                        std::unique_ptr<BoundExpression>(returnValueArguments[0].release()), conversionFun);
                    returnValueArguments[0].reset(boundConversion);
                }
                returnFunctionCall->SetArguments(std::move(returnValueArguments));
            }
            else
            {
                throw Exception("no implicit conversion from '" + ToUtf8(returnValueArguments[0]->GetType()->FullName()) + "' to '" + ToUtf8(returnType->FullName()) + "' exists",
                    returnStatementNode.GetSpan(), function->GetFunctionSymbol()->GetSpan());
            }
            CheckAccess(function->GetFunctionSymbol(), returnFunctionCall->GetFunctionSymbol());
            statement.reset(new BoundReturnStatement(boundCompileUnit.GetAssembly(), std::move(returnFunctionCall)));
            statement->SetContainerScope(containerScope);
        }
        else
        {
            if (returnType)
            {
                throw Exception("void function cannot return a value", returnStatementNode.Expression()->GetSpan(), function->GetFunctionSymbol()->GetSpan());
            }
            else
            {
                throw Exception("constructor or assignment function cannot return a value", returnStatementNode.Expression()->GetSpan(), function->GetFunctionSymbol()->GetSpan());
            }
        }
    }
    else
    {
        TypeSymbol* returnType = function->GetFunctionSymbol()->ReturnType();
        if (!returnType || dynamic_cast<VoidTypeSymbol*>(returnType))
        {
            std::unique_ptr<BoundFunctionCall> returnFunctionCall;
            statement.reset(new BoundReturnStatement(boundCompileUnit.GetAssembly(), std::move(returnFunctionCall)));
            statement->SetContainerScope(containerScope);
        }
        else
        {
            throw Exception("nonvoid function must return a value", returnStatementNode.GetSpan(), function->GetFunctionSymbol()->GetSpan());
        }
    }
}

void StatementBinderVisitor::Visit(IfStatementNode& ifStatementNode)
{
    std::unique_ptr<BoundExpression> condition = BindExpression(boundCompileUnit, function, containerScope, ifStatementNode.Condition());
    if (!dynamic_cast<BoolTypeSymbol*>(condition->GetType()))
    {
        throw Exception("condition of if statement must be Boolean expression", ifStatementNode.Condition()->GetSpan());
    }
    ifStatementNode.ThenS()->Accept(*this);
    BoundStatement* thenS = statement.release();
    thenS->SetSpan(ifStatementNode.ThenS()->GetSpan());
    if (ifStatementNode.ThenS()->Label())
    {
        thenS->SetLabel(ifStatementNode.ThenS()->Label()->Label());
    }
    BoundStatement* elseS = nullptr;
    if (ifStatementNode.ElseS())
    {
        ifStatementNode.ElseS()->Accept(*this);
        elseS = statement.release();
        elseS->SetSpan(ifStatementNode.ElseS()->GetSpan());
        if (ifStatementNode.ElseS()->Label())
        {
            elseS->SetLabel(ifStatementNode.ElseS()->Label()->Label());
        }
    }
    statement.reset(new BoundIfStatement(boundCompileUnit.GetAssembly(), std::move(condition), std::unique_ptr<BoundStatement>(thenS), std::unique_ptr<BoundStatement>(elseS)));
    statement->SetContainerScope(containerScope);
}

void StatementBinderVisitor::Visit(WhileStatementNode& whileStatementNode)
{
    std::unique_ptr<BoundExpression> condition = BindExpression(boundCompileUnit, function, containerScope, whileStatementNode.Condition());
    if (!dynamic_cast<BoolTypeSymbol*>(condition->GetType()))
    {
        throw Exception("condition of while statement must be Boolean expression", whileStatementNode.Condition()->GetSpan());
    }
    whileStatementNode.Statement()->Accept(*this);
    statement->SetSpan(whileStatementNode.Statement()->GetSpan());
    if (whileStatementNode.Statement()->Label())
    {
        statement->SetLabel(whileStatementNode.Statement()->Label()->Label());
    }
    statement.reset(new BoundWhileStatement(boundCompileUnit.GetAssembly(), std::move(condition), std::unique_ptr<BoundStatement>(statement.release())));
    statement->SetContainerScope(containerScope);
}

void StatementBinderVisitor::Visit(DoStatementNode& doStatementNode)
{
    std::unique_ptr<BoundExpression> condition = BindExpression(boundCompileUnit, function, containerScope, doStatementNode.Condition());
    if (!dynamic_cast<BoolTypeSymbol*>(condition->GetType()))
    {
        throw Exception("condition of do statement must be Boolean expression", doStatementNode.Condition()->GetSpan());
    }
    doStatementNode.Statement()->Accept(*this);
    statement->SetSpan(doStatementNode.Statement()->GetSpan());
    if (doStatementNode.Statement()->Label())
    {
        statement->SetLabel(doStatementNode.Statement()->Label()->Label());
    }
    statement.reset(new BoundDoStatement(boundCompileUnit.GetAssembly(), std::unique_ptr<BoundStatement>(statement.release()), std::move(condition)));
    statement->SetContainerScope(containerScope);
}

void StatementBinderVisitor::Visit(ForStatementNode& forStatementNode)
{
    ContainerScope* prevContainerScope = containerScope;
    containerScope = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(forStatementNode)->GetContainerScope();
    std::unique_ptr<BoundExpression> condition;
    if (!forStatementNode.Condition())
    {
        BooleanLiteralNode trueNode(forStatementNode.GetSpan(), true);
        condition = BindExpression(boundCompileUnit, function, containerScope, &trueNode);
    }
    else
    {
        condition = BindExpression(boundCompileUnit, function, containerScope, forStatementNode.Condition());
    }
    forStatementNode.InitS()->Accept(*this);
    BoundStatement* initS = statement.release();
    initS->SetSpan(forStatementNode.InitS()->GetSpan());
    if (forStatementNode.InitS()->Label())
    {
        initS->SetLabel(forStatementNode.InitS()->Label()->Label());
    }
    forStatementNode.LoopS()->Accept(*this);
    BoundStatement* loopS = statement.release();
    loopS->SetSpan(forStatementNode.LoopS()->GetSpan());
    if (forStatementNode.LoopS()->Label())
    {
        loopS->SetLabel(forStatementNode.LoopS()->Label()->Label());
    }
    forStatementNode.ActionS()->Accept(*this);
    BoundStatement* actionS = statement.release();
    actionS->SetSpan(forStatementNode.ActionS()->GetSpan());
    if (forStatementNode.ActionS()->Label())
    {
        actionS->SetLabel(forStatementNode.ActionS()->Label()->Label());
    }
    statement.reset(new BoundForStatement(boundCompileUnit.GetAssembly(), std::unique_ptr<BoundStatement>(initS), std::move(condition), std::unique_ptr<BoundStatement>(loopS),
        std::unique_ptr<BoundStatement>(actionS)));
    statement->SetContainerScope(containerScope);
    containerScope = prevContainerScope;
}

void StatementBinderVisitor::Visit(BreakStatementNode& breakStatementNode)
{
    Node* parent = breakStatementNode.Parent();
    while (parent && !parent->IsBreakEnclosingStatementNode())
    {
        parent = parent->Parent();
    }
    if (!parent)
    {
        throw Exception("break statement must be enclosed in while, do, for, foreach or switch statement", breakStatementNode.GetSpan());
    }
    statement.reset(new BoundBreakStatement(boundCompileUnit.GetAssembly()));
    statement->SetContainerScope(containerScope);
}

void StatementBinderVisitor::Visit(ContinueStatementNode& continueStatementNode)
{
    Node* parent = continueStatementNode.Parent();
    while (parent && !parent->IsContinueEnclosingStatementNode())
    {
        parent = parent->Parent();
    }
    if (!parent)
    {
        throw Exception("continue statement must be enclosed in while, do, for or foreach statement", continueStatementNode.GetSpan());
    }
    statement.reset(new BoundContinueStatement(boundCompileUnit.GetAssembly()));
    statement->SetContainerScope(containerScope);
}

void StatementBinderVisitor::Visit(GotoStatementNode& gotoStatementNode)
{
    function->SetHasGotos();
    boundCompileUnit.SetHasGotos();
    BoundGotoStatement* boundGotoStatement = new BoundGotoStatement(boundCompileUnit.GetAssembly(), gotoStatementNode.Target());
    statement.reset(boundGotoStatement);
    statement->SetContainerScope(containerScope);
}

void StatementBinderVisitor::Visit(ConstructionStatementNode& constructionStatementNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(constructionStatementNode);
    LocalVariableSymbol* localVariableSymbol = dynamic_cast<LocalVariableSymbol*>(symbol);
    Assert(localVariableSymbol, "local variable symbol expected");
    if (localVariableSymbol->GetType() == boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Boolean") && 
        constructionStatementNode.Initializer() && 
        constructionStatementNode.Initializer()->ContainsDisjunctionOrConjuctionNode())
    {
        CloneContext cloneContext; 
        const Span& span = constructionStatementNode.GetSpan();
        ConstructionStatementNode constructFalse(span, new BoolNode(span), static_cast<IdentifierNode*>(constructionStatementNode.Id()->Clone(cloneContext)));
        constructFalse.SetInitializer(new BooleanLiteralNode(span, false));
        IfStatementNode ifStatementNode(span, constructionStatementNode.Initializer()->Clone(cloneContext),
            new AssignmentStatementNode(span, static_cast<IdentifierNode*>(constructionStatementNode.Id()->Clone(cloneContext)), new BooleanLiteralNode(span, true)), nullptr);
        boundCompileUnit.GetAssembly().GetSymbolTable().MapNode(constructFalse, localVariableSymbol);
        TypeBinderVisitor typeBinder(boundCompileUnit);
        constructFalse.Accept(typeBinder);
        ifStatementNode.Accept(typeBinder);
        constructFalse.Accept(*this);
        compoundStatement->AddStatement(std::unique_ptr<BoundStatement>(ReleaseStatement()));
        ifStatementNode.Accept(*this);
        return;
    }
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    BoundExpression* localVariable = new BoundLocalVariable(boundCompileUnit.GetAssembly(), localVariableSymbol->GetType(), localVariableSymbol);
    arguments.push_back(std::unique_ptr<BoundExpression>(localVariable));
    bool constructDelegateType = localVariableSymbol->GetType()->IsDelegateType();
    bool constructClassDelegateType = localVariableSymbol->GetType()->IsClassDelegateType();
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, localVariableSymbol->GetType()->ClassInterfaceOrNsScope()));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
    if (constructionStatementNode.Initializer())
    {
        std::unique_ptr<BoundExpression> argument = BindExpression(boundCompileUnit, function, containerScope, constructionStatementNode.Initializer(), false, 
            constructDelegateType || constructClassDelegateType, constructClassDelegateType);
        arguments.push_back(std::move(argument));
    }
    std::unique_ptr<BoundFunctionCall> constructorCall = ResolveOverload(boundCompileUnit, U"@init", functionScopeLookups, arguments, constructionStatementNode.GetSpan());
    CheckAccess(function->GetFunctionSymbol(), constructorCall->GetFunctionSymbol());
    statement.reset(new BoundConstructionStatement(boundCompileUnit.GetAssembly(), std::move(constructorCall)));
    statement->SetContainerScope(containerScope);
}

void StatementBinderVisitor::Visit(AssignmentStatementNode& assignmentStatementNode)
{
    if (assignmentStatementNode.SourceExpr()->ContainsDisjunctionOrConjuctionNode())
    {
        CloneContext cloneContext;
        const Span& span = assignmentStatementNode.GetSpan();
        IfStatementNode ifStatementNode(span, assignmentStatementNode.SourceExpr()->Clone(cloneContext),
            new AssignmentStatementNode(span, assignmentStatementNode.TargetExpr()->Clone(cloneContext), new BooleanLiteralNode(span, true)),
            new AssignmentStatementNode(span, assignmentStatementNode.TargetExpr()->Clone(cloneContext), new BooleanLiteralNode(span, false)));
        TypeBinderVisitor typeBinder(boundCompileUnit);
        ifStatementNode.Accept(typeBinder);
        ifStatementNode.Accept(*this);
        return;
    }
    std::unique_ptr<BoundExpression> target = BindExpression(boundCompileUnit, function, containerScope, assignmentStatementNode.TargetExpr(), true);
    TypeSymbol* targetType = target->GetType();
    bool assignDelegateType = targetType->IsDelegateType();
    bool assignClassDelegateType = targetType->IsClassDelegateType();
    std::unique_ptr<BoundExpression> source = BindExpression(boundCompileUnit, function, containerScope, assignmentStatementNode.SourceExpr(), false, assignDelegateType || assignClassDelegateType, 
        assignClassDelegateType);
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    arguments.push_back(std::move(target));
    arguments.push_back(std::move(source));
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, targetType->ClassInterfaceOrNsScope()));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
    std::unique_ptr<BoundFunctionCall> assignmentCall = ResolveOverload(boundCompileUnit, U"@assignment", functionScopeLookups, arguments, assignmentStatementNode.GetSpan());
    CheckAccess(function->GetFunctionSymbol(), assignmentCall->GetFunctionSymbol());
    statement.reset(new BoundAssignmentStatement(boundCompileUnit.GetAssembly(), std::move(assignmentCall)));
    statement->SetContainerScope(containerScope);
}

void StatementBinderVisitor::Visit(ExpressionStatementNode& expressionStatementNode)
{
    std::unique_ptr<BoundExpression> expression = BindExpression(boundCompileUnit, function, containerScope, expressionStatementNode.Expression());
    BoundExpressionStatement* boundExpressionStatement = new BoundExpressionStatement(boundCompileUnit.GetAssembly(), std::move(expression));
    statement.reset(boundExpressionStatement);
    statement->SetContainerScope(containerScope);
}

void StatementBinderVisitor::Visit(EmptyStatementNode& emptyStatementNode)
{
    statement.reset(new BoundEmptyStatement(boundCompileUnit.GetAssembly()));
    statement->SetContainerScope(containerScope);
}

void StatementBinderVisitor::Visit(IncrementStatementNode& incrementStatementNode)
{
    std::unique_ptr<BoundExpression> expression = BindExpression(boundCompileUnit, function, containerScope, incrementStatementNode.Expression());
    if (expression->GetType()->IsUnsignedType())
    {
        CloneContext cloneContext;
        AssignmentStatementNode assignmentStatement(incrementStatementNode.GetSpan(), incrementStatementNode.Expression()->Clone(cloneContext),
            new AddNode(incrementStatementNode.GetSpan(), incrementStatementNode.Expression()->Clone(cloneContext), new ByteLiteralNode(incrementStatementNode.GetSpan(), 1u)));
        assignmentStatement.Accept(*this);
    }
    else
    {
        CloneContext cloneContext;
        AssignmentStatementNode assignmentStatement(incrementStatementNode.GetSpan(), incrementStatementNode.Expression()->Clone(cloneContext),
            new AddNode(incrementStatementNode.GetSpan(), incrementStatementNode.Expression()->Clone(cloneContext), new SByteLiteralNode(incrementStatementNode.GetSpan(), 1)));
        assignmentStatement.Accept(*this);
    }
}

void StatementBinderVisitor::Visit(DecrementStatementNode& decrementStatementNode)
{
    std::unique_ptr<BoundExpression> expression = BindExpression(boundCompileUnit, function, containerScope, decrementStatementNode.Expression());
    if (expression->GetType()->IsUnsignedType())
    {
        CloneContext cloneContext;
        AssignmentStatementNode assignmentStatement(decrementStatementNode.GetSpan(), decrementStatementNode.Expression()->Clone(cloneContext),
            new SubNode(decrementStatementNode.GetSpan(), decrementStatementNode.Expression()->Clone(cloneContext), new ByteLiteralNode(decrementStatementNode.GetSpan(), 1u)));
        assignmentStatement.Accept(*this);
    }
    else
    {
        CloneContext cloneContext;
        AssignmentStatementNode assignmentStatement(decrementStatementNode.GetSpan(), decrementStatementNode.Expression()->Clone(cloneContext),
            new SubNode(decrementStatementNode.GetSpan(), decrementStatementNode.Expression()->Clone(cloneContext), new SByteLiteralNode(decrementStatementNode.GetSpan(), 1)));
        assignmentStatement.Accept(*this);
    }
}

void StatementBinderVisitor::Visit(ForEachStatementNode& forEachStatementNode)
{
    const Span& span = forEachStatementNode.GetSpan();
    CompoundStatementNode forEachBlock(span);
    if (forEachStatementNode.Label())
    {
        forEachBlock.SetLabelNode(new LabelNode(span, forEachStatementNode.Label()->Label()));
    }
    ConstructionStatementNode* constructEnumerable = new ConstructionStatementNode(span, new IdentifierNode(span, U"System.Enumerable"), new IdentifierNode(span, U"@enumerable"));
    CloneContext cloneContext;
    constructEnumerable->SetInitializer(forEachStatementNode.Container()->Clone(cloneContext));
    forEachBlock.AddStatement(constructEnumerable);
    ConstructionStatementNode* constructEnumerator = new ConstructionStatementNode(span, new IdentifierNode(span, U"System.Enumerator"), new IdentifierNode(span, U"@enumerator"));
    constructEnumerator->SetInitializer(new InvokeNode(span, new DotNode(span, new IdentifierNode(span, U"@enumerable"), new IdentifierNode(span, U"GetEnumerator"))));
    forEachBlock.AddStatement(constructEnumerator);
    CompoundStatementNode* loopBody = new CompoundStatementNode(span);
    ConstructionStatementNode* constructLoopVar = new ConstructionStatementNode(span, 
        forEachStatementNode.TypeExpr()->Clone(cloneContext), 
        static_cast<IdentifierNode*>(forEachStatementNode.Id()->Clone(cloneContext)));
    constructLoopVar->SetInitializer(new CastNode(span, forEachStatementNode.TypeExpr()->Clone(cloneContext),
        new InvokeNode(span, new DotNode(span, new IdentifierNode(span, U"@enumerator"), new IdentifierNode(span, U"GetCurrent")))));
    loopBody->AddStatement(constructLoopVar);
    loopBody->AddStatement(static_cast<StatementNode*>(forEachStatementNode.Action()->Clone(cloneContext)));
    ExpressionStatementNode* moveNext = new ExpressionStatementNode(span, new InvokeNode(span, new DotNode(span, new IdentifierNode(span, U"@enumerator"), new IdentifierNode(span, U"MoveNext"))));
    ForStatementNode* forStatement = new ForStatementNode(span, 
        new EmptyStatementNode(span), 
        new NotNode(span, new InvokeNode(span, new DotNode(span, new IdentifierNode(span, U"@enumerator"), new IdentifierNode(span, U"AtEnd")))),
        moveNext, 
        loopBody);
    forEachBlock.AddStatement(forStatement);
    int declarationBlockId = function->GetFunctionSymbol()->DeclarationBlockId();
    boundCompileUnit.GetAssembly().GetSymbolTable().SetDeclarationBlockId(declarationBlockId);
    boundCompileUnit.GetAssembly().GetSymbolTable().BeginContainer(containerScope->Container());
    boundCompileUnit.GetAssembly().GetSymbolTable().SetFunction(function->GetFunctionSymbol());
    SymbolCreatorVisitor symbolCreatorVisitor(boundCompileUnit.GetAssembly());
    forEachBlock.Accept(symbolCreatorVisitor);
    function->GetFunctionSymbol()->SetDeclarationBlockId(boundCompileUnit.GetAssembly().GetSymbolTable().GetDeclarationBlockId());
    boundCompileUnit.GetAssembly().GetSymbolTable().EndContainer();
    TypeBinderVisitor typeBinderVisitor(boundCompileUnit);
    forEachBlock.Accept(typeBinderVisitor);
    forEachBlock.Accept(*this);
}

void StatementBinderVisitor::Visit(SwitchStatementNode& switchStatementNode)
{
    std::unordered_map<IntegralValue, CaseStatementNode*, IntegralValueHash>* prevCaseValueMap = caseValueMap;
    TypeSymbol* prevSwitchTypeConditionType = switchConditionType;
    std::unordered_map<IntegralValue, CaseStatementNode*, IntegralValueHash> switchsCaseValueMap;
    std::vector<std::pair<GotoCaseStatementNode*, IntegralValue>>* prevGotoCaseStatements = gotoCaseStatements;
    std::vector<std::pair<GotoCaseStatementNode*, IntegralValue>> gotoCaseStatements_;
    gotoCaseStatements = &gotoCaseStatements_;
    std::vector<GotoDefaultStatementNode*>* prevGotoDefaultStatements = gotoDefaultStatements; 
    std::vector<GotoDefaultStatementNode*> gotoDefaultStatements_;
    gotoDefaultStatements = &gotoDefaultStatements_;
    caseValueMap = &switchsCaseValueMap;
    std::unique_ptr<BoundExpression> condition = BindExpression(boundCompileUnit, function, containerScope, switchStatementNode.Condition());
    TypeSymbol* condType = condition->GetType();
    switchConditionType = condType;
    if (condType->IsSwitchConditionType())
    {
        if (EnumTypeSymbol* enumTypeSymbol = dynamic_cast<EnumTypeSymbol*>(switchConditionType))
        {
            switchConditionType = enumTypeSymbol->GetUnderlyingType();
        }
        std::unique_ptr<BoundSwitchStatement> switchStatement(new BoundSwitchStatement(boundCompileUnit.GetAssembly(), std::move(condition)));
        switchStatement->SetContainerScope(containerScope);
        int n = switchStatementNode.Cases().Count();
        for (int i = 0; i < n; ++i)
        {
            CaseStatementNode* caseS = switchStatementNode.Cases()[i];
            caseS->Accept(*this);
            BoundCaseStatement* bcs = dynamic_cast<BoundCaseStatement*>(statement.release());
            Assert(bcs, "bound case statement expected");
            bcs->SetSpan(caseS->GetSpan());
            if (caseS->Label())
            {
                bcs->SetLabel(caseS->Label()->Label());
            }
            switchStatement->AddCaseStatement(std::unique_ptr<BoundCaseStatement>(bcs));
        }
        if (switchStatementNode.Default())
        {
            switchStatementNode.Default()->Accept(*this);
            BoundDefaultStatement* bds = dynamic_cast<BoundDefaultStatement*>(statement.release());
            Assert(bds, "bound default statement expected");
            bds->SetSpan(switchStatementNode.Default()->GetSpan());
            if (switchStatementNode.Default()->Label())
            {
                bds->SetLabel(switchStatementNode.Default()->Label()->Label());
            }
            switchStatement->SetDefaultStatement(std::unique_ptr<BoundDefaultStatement>(bds));
        }
        statement.reset(switchStatement.release());
    }
    else
    {
        throw Exception("switch statement condition must be of integer, character, enumerated or Boolean type", switchStatementNode.Condition()->GetSpan());
    }
    for (const std::pair<GotoCaseStatementNode*, IntegralValue>& p : gotoCaseStatements_)
    {
        if (switchsCaseValueMap.find(p.second) == switchsCaseValueMap.cend())
        {
            throw Exception("case " + std::to_string(p.second.Value()) + " not found", p.first->GetSpan());
        }
    }
    if (!gotoDefaultStatements->empty() && !switchStatementNode.Default())
    {
        throw Exception("switch does not have a default statement", gotoDefaultStatements->front()->GetSpan());
    }
    switchConditionType = prevSwitchTypeConditionType;
    caseValueMap = prevCaseValueMap;
    gotoCaseStatements = prevGotoCaseStatements;
    gotoDefaultStatements = prevGotoDefaultStatements;
}

bool TerminatesCase(StatementNode* statementNode)
{
    if (IfStatementNode* ifStatementNode = dynamic_cast<IfStatementNode*>(statementNode))
    {
        if (ifStatementNode->ElseS())
        {
            if (TerminatesCase(ifStatementNode->ThenS()) && TerminatesCase(ifStatementNode->ElseS()))
            {
                return true;
            }
        }
    }
    else if (CompoundStatementNode* compoundStatement = dynamic_cast<CompoundStatementNode*>(statementNode))
    {
        int n = compoundStatement->Statements().Count();
        for (int i = 0; i < n; ++i)
        {
            StatementNode* statement = compoundStatement->Statements()[i];
            if (TerminatesCase(statement)) return true;
        }
    }
    else
    {
        return statementNode->IsCaseTerminatingNode();
    }
    return false;
}

bool TerminatesDefault(StatementNode* statementNode)
{
    if (IfStatementNode* ifStatementNode = dynamic_cast<IfStatementNode*>(statementNode))
    {
        if (ifStatementNode->ElseS())
        {
            if (TerminatesDefault(ifStatementNode->ThenS()) && TerminatesDefault(ifStatementNode->ElseS()))
            {
                return true;
            }
        }
    }
    else if (CompoundStatementNode* compoundStatement = dynamic_cast<CompoundStatementNode*>(statementNode))
    {
        int n = compoundStatement->Statements().Count();
        for (int i = 0; i < n; ++i)
        {
            StatementNode* statement = compoundStatement->Statements()[i];
            if (TerminatesDefault(statement)) return true;
        }
    }
    else
    {
        return statementNode->IsDefaultTerminatingNode();
    }
    return false;
}

void StatementBinderVisitor::Visit(CaseStatementNode& caseStatementNode)
{
    std::unique_ptr<BoundCaseStatement> caseStatement(new BoundCaseStatement(boundCompileUnit.GetAssembly()));
    caseStatement->CompoundStatement()->SetContainerScope(containerScope);
    bool terminated = false;
    int n = caseStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = caseStatementNode.Statements()[i];
        if (TerminatesCase(statementNode))
        {
            terminated = true;
        }
        statementNode->Accept(*this);
        statement->SetSpan(statementNode->GetSpan());
        if (statementNode->Label())
        {
            statement->SetLabel(statementNode->Label()->Label());
        }
        caseStatement->AddStatement(std::move(statement));
    }
    if (!terminated)
    {
        throw Exception("case must end in break, continue, return, throw, goto, goto case or goto default statement", caseStatementNode.GetSpan());
    }
    int m = caseStatementNode.CaseExprs().Count();
    for (int i = 0; i < m; ++i)
    {
        Node* caseExpr = caseStatementNode.CaseExprs()[i];
        std::unique_ptr<Value> value(Evaluate(GetValueKindFor(switchConditionType->GetSymbolType(), caseStatementNode.GetSpan()), false, containerScope, boundCompileUnit, caseExpr));
        IntegralValue caseValue = value->GetIntegralValue();
        auto it = caseValueMap->find(caseValue);
        if (it != caseValueMap->cend())
        {
            throw Exception("case value " + std::to_string(caseValue.Value()) + " already defined", caseStatementNode.GetSpan(), it->second->GetSpan());
        }
        (*caseValueMap)[caseValue] = &caseStatementNode;
        caseStatement->AddCaseValue(caseValue);
    }
    statement.reset(caseStatement.release());
    statement->SetContainerScope(containerScope);
}

void StatementBinderVisitor::Visit(DefaultStatementNode& defaultStatementNode)
{
    std::unique_ptr<BoundDefaultStatement> defaultStatement(new BoundDefaultStatement(boundCompileUnit.GetAssembly()));
    defaultStatement->CompoundStatement()->SetContainerScope(containerScope);
    bool terminated = false;
    int n = defaultStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = defaultStatementNode.Statements()[i];
        if (TerminatesDefault(statementNode))
        {
            terminated = true;
        }
        statementNode->Accept(*this);
        statement->SetSpan(statementNode->GetSpan());
        if (statementNode->Label())
        {
            statement->SetLabel(statementNode->Label()->Label());
        }
        defaultStatement->AddStatement(std::move(statement));
    }
    if (!terminated)
    {
        throw Exception("default must end in break, continue, return, throw, goto, or goto case statement", defaultStatementNode.GetSpan());
    }
    statement.reset(defaultStatement.release());
    statement->SetContainerScope(containerScope);
}

void StatementBinderVisitor::Visit(GotoCaseStatementNode& gotoCaseStatementNode)
{
    Node* parent = gotoCaseStatementNode.Parent();
    while (parent && !dynamic_cast<CaseStatementNode*>(parent) && !dynamic_cast<DefaultStatementNode*>(parent))
    {
        parent = parent->Parent();
    }
    if (!parent)
    {
        throw Exception("goto case statement must be enclosed in case or default statement", gotoCaseStatementNode.GetSpan());
    }
    std::unique_ptr<Value> value(Evaluate(GetValueKindFor(switchConditionType->GetSymbolType(), gotoCaseStatementNode.GetSpan()), false, containerScope, boundCompileUnit, gotoCaseStatementNode.CaseExpr()));
    IntegralValue caseValue = value->GetIntegralValue();
    gotoCaseStatements->push_back(std::make_pair(&gotoCaseStatementNode, caseValue));
    statement.reset(new BoundGotoCaseStatement(boundCompileUnit.GetAssembly(), caseValue));
    statement->SetContainerScope(containerScope);
}

void StatementBinderVisitor::Visit(GotoDefaultStatementNode& gotoDefaultStatementNode)
{
    Node* parent = gotoDefaultStatementNode.Parent();
    while (parent && !dynamic_cast<CaseStatementNode*>(parent))
    {
        parent = parent->Parent();
    }
    if (!parent)
    {
        throw Exception("goto default statement must be enclosed in case statement", gotoDefaultStatementNode.GetSpan());
    }
    gotoDefaultStatements->push_back(&gotoDefaultStatementNode);
    statement.reset(new BoundGotoDefaultStatement(boundCompileUnit.GetAssembly()));
    statement->SetContainerScope(containerScope);
}

void StatementBinderVisitor::Visit(ThrowStatementNode& throwStatementNode)
{
    if (throwStatementNode.Expression())
    {
        std::unique_ptr<BoundExpression> expression = BindExpression(boundCompileUnit, function, containerScope, throwStatementNode.Expression());
        TypeSymbol* exceptionType = expression->GetType();
        if (ClassTypeSymbol* exceptionClassType = dynamic_cast<ClassTypeSymbol*>(exceptionType))
        {
            TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Exception");
            ClassTypeSymbol* classType = dynamic_cast<ClassTypeSymbol*>(type);
            Assert(classType, "class type symbol expected");
            if (exceptionClassType == classType || exceptionClassType->HasBaseClass(classType))
            {
                statement.reset(new BoundThrowStatement(boundCompileUnit.GetAssembly(), std::move(expression)));
                statement->SetContainerScope(containerScope);
            }
            else
            {
                throw Exception("exception type must be a class type derived from System.Exception class", throwStatementNode.GetSpan());
            }
        }
        else
        {
            throw Exception("exception type must be a class type derived from System.Exception class", throwStatementNode.GetSpan());
        }
    }
    else
    {
        if (insideCatch)
        {
            statement.reset(new BoundThrowStatement(boundCompileUnit.GetAssembly()));
            statement->SetContainerScope(containerScope);
        }
        else
        {
            throw Exception("exception can be rethrown only from inside a catch clause", throwStatementNode.GetSpan());
        }
    }
}

void StatementBinderVisitor::Visit(TryStatementNode& tryStatementNode)
{
    std::unique_ptr<BoundTryStatement> tryStatement(new BoundTryStatement(boundCompileUnit.GetAssembly()));
    tryStatement->SetContainerScope(containerScope);
    tryStatementNode.TryBlock()->Accept(*this);
    BoundCompoundStatement* tryBlock = dynamic_cast<BoundCompoundStatement*>(statement.release());
    Assert(tryBlock, "bound compound statement expected");
    tryStatement->SetTryBlock(std::unique_ptr<BoundCompoundStatement>(tryBlock));
    int n = tryStatementNode.Catches().Count();
    for (int i = 0; i < n; ++i)
    {
        CatchNode* catchNode = tryStatementNode.Catches()[i];
        catchNode->Accept(*this);
        BoundCatchStatement* catchStatement = dynamic_cast<BoundCatchStatement*>(statement.release());
        Assert(catchStatement, "bound catch statement expected");
        tryStatement->AddCatchStatement(std::unique_ptr<BoundCatchStatement>(catchStatement));
    }
    if (tryStatementNode.FinallyBlock())
    {
        tryStatementNode.FinallyBlock()->Accept(*this);
        BoundCompoundStatement* finallyBlock = dynamic_cast<BoundCompoundStatement*>(statement.release());
        Assert(finallyBlock, "bound compound statement expected");
        tryStatement->SetFinallyBlock(std::unique_ptr<BoundCompoundStatement>(finallyBlock));
    }
    statement = std::move(tryStatement);
}

void StatementBinderVisitor::Visit(CatchNode& catchNode)
{
    bool prevInsideCatch = insideCatch;
    insideCatch = true;
    std::unique_ptr<BoundCatchStatement> catchStatement(new BoundCatchStatement(boundCompileUnit.GetAssembly()));
    catchStatement->SetContainerScope(containerScope);
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(*catchNode.Id());
    LocalVariableSymbol* exceptionVar = dynamic_cast<LocalVariableSymbol*>(symbol);
    Assert(exceptionVar, "local variable symbol expected");
    catchStatement->SetExceptionVar(exceptionVar);
    catchNode.CatchBlock()->Accept(*this);
    BoundCompoundStatement* catchBlock = dynamic_cast<BoundCompoundStatement*>(statement.release());
    Assert(catchBlock, "bound compound statement expected");
    catchStatement->SetCatchBlock(std::unique_ptr<BoundCompoundStatement>(catchBlock));
    statement = std::move(catchStatement);
    insideCatch = prevInsideCatch;
}

void StatementBinderVisitor::Visit(UsingStatementNode& usingStatementNode)
{
    const Span& span = usingStatementNode.GetSpan();
    CompoundStatementNode usingBlock(span);
    if (usingStatementNode.Label())
    {
        usingBlock.SetLabelNode(new LabelNode(span, usingStatementNode.Label()->Label()));
    }
    CloneContext cloneContext;
    usingBlock.AddStatement(static_cast<StatementNode*>(usingStatementNode.ConstructionStatement()->Clone(cloneContext)));
    CompoundStatementNode* tryBlock = new CompoundStatementNode(span);
    tryBlock->AddStatement(static_cast<StatementNode*>(usingStatementNode.Statement()->Clone(cloneContext)));
    CompoundStatementNode* finallyBlock = new CompoundStatementNode(span);
    ConstructionStatementNode* constructClosable = new ConstructionStatementNode(span, new IdentifierNode(span, U"System.Closable"), new IdentifierNode(span, U"@closable"));
    constructClosable->SetInitializer(usingStatementNode.ConstructionStatement()->Id()->Clone(cloneContext));
    finallyBlock->AddStatement(constructClosable);
    ExpressionStatementNode* closeClosableStatement = new ExpressionStatementNode(span, new InvokeNode(span, new DotNode(span, new IdentifierNode(span, U"@closable"), new IdentifierNode(span, U"Close"))));
    finallyBlock->AddStatement(closeClosableStatement);
    TryStatementNode* tryStatementNode = new TryStatementNode(span, tryBlock);
    tryStatementNode->SetFinally(finallyBlock);
    usingBlock.AddStatement(tryStatementNode);
    int declarationBlockId = function->GetFunctionSymbol()->DeclarationBlockId();
    boundCompileUnit.GetAssembly().GetSymbolTable().SetDeclarationBlockId(declarationBlockId);
    boundCompileUnit.GetAssembly().GetSymbolTable().BeginContainer(containerScope->Container());
    boundCompileUnit.GetAssembly().GetSymbolTable().SetFunction(function->GetFunctionSymbol());
    SymbolCreatorVisitor symbolCreatorVisitor(boundCompileUnit.GetAssembly());
    usingBlock.Accept(symbolCreatorVisitor);
    boundCompileUnit.GetAssembly().GetSymbolTable().EndContainer();
    function->GetFunctionSymbol()->SetDeclarationBlockId(boundCompileUnit.GetAssembly().GetSymbolTable().GetDeclarationBlockId());
    TypeBinderVisitor typeBinderVisitor(boundCompileUnit);
    usingBlock.Accept(typeBinderVisitor);
    usingBlock.Accept(*this);
}

void StatementBinderVisitor::Visit(LockStatementNode& lockStatementNode)
{
    const Span& span = lockStatementNode.GetSpan();
    CompoundStatementNode lockBlock(span);
    if (lockStatementNode.Label())
    {
        lockBlock.SetLabelNode(new LabelNode(span, lockStatementNode.Label()->Label()));
    }
    CloneContext cloneContext;
    InvokeNode* invokeEnterNode = new InvokeNode(span, 
        new DotNode(span, 
            new DotNode(span, 
                new DotNode(span, 
                    new IdentifierNode(span, U"System"), 
                    new IdentifierNode(span, U"Threading")), 
                new IdentifierNode(span, U"Monitor")),
            new IdentifierNode(span, U"Enter")));
    invokeEnterNode->AddArgument(lockStatementNode.Expr()->Clone(cloneContext));
    ExpressionStatementNode* monitorEnter = new ExpressionStatementNode(span, invokeEnterNode);
    lockBlock.AddStatement(monitorEnter);
    CompoundStatementNode* tryBlock = new CompoundStatementNode(span);
    tryBlock->AddStatement(static_cast<StatementNode*>(lockStatementNode.Statement()->Clone(cloneContext)));
    CompoundStatementNode* finallyBlock = new CompoundStatementNode(span);
    InvokeNode* invokeExitNode = new InvokeNode(span,
        new DotNode(span,
            new DotNode(span,
                new DotNode(span,
                    new IdentifierNode(span, U"System"),
                    new IdentifierNode(span, U"Threading")),
                new IdentifierNode(span, U"Monitor")),
            new IdentifierNode(span, U"Exit")));
    invokeExitNode->AddArgument(lockStatementNode.Expr()->Clone(cloneContext));
    ExpressionStatementNode* monitorExit = new ExpressionStatementNode(span, invokeExitNode);
    finallyBlock->AddStatement(monitorExit);
    TryStatementNode* tryStatementNode = new TryStatementNode(span, tryBlock);
    tryStatementNode->SetFinally(finallyBlock);
    lockBlock.AddStatement(tryStatementNode);
    int declarationBlockId = function->GetFunctionSymbol()->DeclarationBlockId();
    boundCompileUnit.GetAssembly().GetSymbolTable().SetDeclarationBlockId(declarationBlockId);
    boundCompileUnit.GetAssembly().GetSymbolTable().BeginContainer(containerScope->Container());
    boundCompileUnit.GetAssembly().GetSymbolTable().SetFunction(function->GetFunctionSymbol());
    SymbolCreatorVisitor symbolCreatorVisitor(boundCompileUnit.GetAssembly());
    lockBlock.Accept(symbolCreatorVisitor);
    boundCompileUnit.GetAssembly().GetSymbolTable().EndContainer();
    function->GetFunctionSymbol()->SetDeclarationBlockId(boundCompileUnit.GetAssembly().GetSymbolTable().GetDeclarationBlockId());
    TypeBinderVisitor typeBinderVisitor(boundCompileUnit);
    lockBlock.Accept(typeBinderVisitor);
    lockBlock.Accept(*this);
}

} } // namespace cminor::binder
