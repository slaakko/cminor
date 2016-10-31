// =================================
// Copyright (c) 2016 Seppo Laakko
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
#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/ast/CompileUnit.hpp>
#include <cminor/ast/Literal.hpp>
#include <cminor/ast/Expression.hpp>

namespace cminor { namespace binder {

bool TerminatesFunction(StatementNode* statement, bool inForEverLoop)
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
                if (TerminatesFunction(statement, inForEverLoop)) return true;
            }
            break;
        }
        case NodeType::ifStatementNode:
        {
            IfStatementNode* ifStatement = static_cast<IfStatementNode*>(statement);
            if (inForEverLoop || ifStatement->ElseS())
            {
                if (TerminatesFunction(ifStatement->ThenS(), inForEverLoop) && 
                    inForEverLoop || (ifStatement->ElseS() && TerminatesFunction(ifStatement->ElseS(), inForEverLoop)))
                {
                    return true;
                }
            }
            break;
        }
        case NodeType::whileStatementNode:
        {
            WhileStatementNode* whileStatement = static_cast<WhileStatementNode*>(statement);
            // todo for ever loop detection
            break;
        }
        case NodeType::doStatementNode:
        {
            DoStatementNode* doStatement = static_cast<DoStatementNode*>(statement);
            // todo for ever loop detection
            break;
        }
        case NodeType::forStatementNode:
        {
            ForStatementNode* forStatement = static_cast<ForStatementNode*>(statement);
            // todo for ever loop detection
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

void CheckFunctionReturnPaths(FunctionSymbol* functionSymbol, FunctionNode& functionNode)
{
    TypeSymbol* returnType = functionSymbol->ReturnType();
    if (!returnType || dynamic_cast<VoidTypeSymbol*>(returnType)) return;
    if (functionSymbol->IsExternal()) return;
    CompoundStatementNode* body = functionNode.Body();
    int n = body->Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statement = body->Statements()[i];
        if (TerminatesFunction(statement, false)) return;
    }
    throw Exception("not all control paths terminate in return or throw statement", functionNode.GetSpan());
}

StatementBinderVisitor::StatementBinderVisitor(BoundCompileUnit& boundCompileUnit_) : 
    boundCompileUnit(boundCompileUnit_), containerScope(nullptr), boundClass(nullptr), function(nullptr), compoundStatement(nullptr)
{
}

void StatementBinderVisitor::Visit(CompileUnitNode& compileUnitNode)
{
    compileUnitNode.GlobalNs()->Accept(*this);
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
    if (!defaultConstructorSymbol->IsBound())
    {
        defaultConstructorSymbol->SetBound();
        std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(defaultConstructorSymbol));
        boundClass->AddMember(std::move(boundFunction));
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
    constructorSymbol->SetBaseConstructorCallGenerated();
    Assert(constructorSymbol, "constructor symbol expected");
    containerScope = symbol->GetContainerScope();
    std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(constructorSymbol));
    BoundFunction* prevFunction = function;
    function = boundFunction.get();
    BoundCompoundStatement* prevCompoundStatement = compoundStatement;
    if (constructorNode.HasBody())
    {
        constructorNode.Body()->Accept(*this);
        compoundStatement = dynamic_cast<BoundCompoundStatement*>(statement.release());
        Assert(compoundStatement, "compound statement expected");
        function->SetBody(std::unique_ptr<BoundCompoundStatement>(compoundStatement));
    }
    if (constructorNode.Initializer())
    {
        constructorNode.Initializer()->Accept(*this);
    }
    else if (boundClass->GetClassTypeSymbol()->BaseClass())
    {
        BoundFunctionCall* baseConstructorCall = new BoundFunctionCall(boundCompileUnit.GetAssembly(), boundClass->GetClassTypeSymbol()->BaseClass()->DefaultConstructorSymbol());
        BoundExpression* boundThisParam = new BoundParameter(boundCompileUnit.GetAssembly(), constructorSymbol->Parameters()[0]->GetType(), constructorSymbol->Parameters()[0]);
        BoundConversion* thisAsBase = new BoundConversion(boundCompileUnit.GetAssembly(), std::unique_ptr<BoundExpression>(boundThisParam),
            boundCompileUnit.GetConversion(boundClass->GetClassTypeSymbol(), boundClass->GetClassTypeSymbol()->BaseClass()));
        baseConstructorCall->AddArgument(std::unique_ptr<BoundExpression>(thisAsBase));
        BoundExpressionStatement* baseConstructorCallStatement = new BoundExpressionStatement(boundCompileUnit.GetAssembly(), std::unique_ptr<BoundExpression>(baseConstructorCall));
        compoundStatement->InsertFront(std::unique_ptr<BoundStatement>(baseConstructorCallStatement));
    }
    compoundStatement = prevCompoundStatement;
    boundClass->AddMember(std::move(boundFunction));
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
    compoundStatement->InsertFront(std::unique_ptr<BoundStatement>(thisConstructorCallStatement));
}

void StatementBinderVisitor::Visit(MemberFunctionNode& memberFunctionNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(memberFunctionNode);
    MemberFunctionSymbol* memberFunctionSymbol = dynamic_cast<MemberFunctionSymbol*>(symbol);
    Assert(memberFunctionSymbol, "member function symbol expected");
    containerScope = symbol->GetContainerScope();
    std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(memberFunctionSymbol));
    BoundFunction* prevFunction = function;
    function = boundFunction.get();
    if (memberFunctionNode.HasBody())
    {
        memberFunctionNode.Body()->Accept(*this);
        BoundCompoundStatement* compoundStatement = dynamic_cast<BoundCompoundStatement*>(statement.release());
        Assert(compoundStatement, "compound statement expected");
        function->SetBody(std::unique_ptr<BoundCompoundStatement>(compoundStatement));
    }
    CheckFunctionReturnPaths(memberFunctionSymbol, memberFunctionNode);
    boundClass->AddMember(std::move(boundFunction));
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
    BoundFunction* prevFunction = function;
    function = boundFunction.get();
    if (functionNode.HasBody())
    {
        functionNode.Body()->Accept(*this);
        BoundCompoundStatement* compoundStatement = dynamic_cast<BoundCompoundStatement*>(statement.release());
        Assert(compoundStatement, "compound statement expected");
        function->SetBody(std::unique_ptr<BoundCompoundStatement>(compoundStatement));
    }
    CheckFunctionReturnPaths(functionSymbol, functionNode);
    boundCompileUnit.AddBoundNode(std::move(boundFunction));
    containerScope = prevContainerScope;
    function = prevFunction;
}

void StatementBinderVisitor::Visit(CompoundStatementNode& compoundStatementNode)
{
    ContainerScope* prevContainerScope = containerScope;
    containerScope = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(compoundStatementNode)->GetContainerScope();
    std::unique_ptr<BoundCompoundStatement> boundCompoundStatement(new BoundCompoundStatement(boundCompileUnit.GetAssembly()));
    int n = compoundStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = compoundStatementNode.Statements()[i];
        statementNode->Accept(*this);
        boundCompoundStatement->AddStatement(std::unique_ptr<BoundStatement>(statement.release()));
    }
    containerScope = prevContainerScope;
    statement.reset(boundCompoundStatement.release());
}

void StatementBinderVisitor::Visit(ReturnStatementNode& returnStatementNode)
{
    if (returnStatementNode.Expression())
    {
        TypeSymbol* returnType = function->GetFunctionSymbol()->ReturnType();
        if (returnType && !dynamic_cast<VoidTypeSymbol*>(returnType))
        {
            std::vector<std::unique_ptr<BoundExpression>> returnTypeArgs;
            returnTypeArgs.push_back(std::unique_ptr<BoundTypeExpression>(new BoundTypeExpression(boundCompileUnit.GetAssembly(), returnType)));
            std::vector<FunctionScopeLookup> functionScopeLookups;
            functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
            functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, returnType->ClassOrNsScope()));
            functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
            std::unique_ptr<BoundFunctionCall> returnFunctionCall = ResolveOverload(boundCompileUnit, U"@return", functionScopeLookups, returnTypeArgs, returnStatementNode.GetSpan());
            std::unique_ptr<BoundExpression> expression = BindExpression(boundCompileUnit, function, containerScope, returnStatementNode.Expression());
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
                    returnValueArguments[0].reset(new BoundConversion(boundCompileUnit.GetAssembly(), std::unique_ptr<BoundExpression>(returnValueArguments[0].release()), conversionFun));
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
    BoundStatement* elseS = nullptr;
    if (ifStatementNode.ElseS())
    {
        ifStatementNode.ElseS()->Accept(*this);
        elseS = statement.release();
    }
    statement.reset(new BoundIfStatement(boundCompileUnit.GetAssembly(), std::move(condition), std::unique_ptr<BoundStatement>(thenS), std::unique_ptr<BoundStatement>(elseS)));
}

void StatementBinderVisitor::Visit(WhileStatementNode& whileStatementNode)
{
    std::unique_ptr<BoundExpression> condition = BindExpression(boundCompileUnit, function, containerScope, whileStatementNode.Condition());
    if (!dynamic_cast<BoolTypeSymbol*>(condition->GetType()))
    {
        throw Exception("condition of while statement must be Boolean expression", whileStatementNode.Condition()->GetSpan());
    }
    whileStatementNode.Statement()->Accept(*this);
    statement.reset(new BoundWhileStatement(boundCompileUnit.GetAssembly(), std::move(condition), std::unique_ptr<BoundStatement>(statement.release())));
}

void StatementBinderVisitor::Visit(DoStatementNode& doStatementNode)
{
    std::unique_ptr<BoundExpression> condition = BindExpression(boundCompileUnit, function, containerScope, doStatementNode.Condition());
    if (!dynamic_cast<BoolTypeSymbol*>(condition->GetType()))
    {
        throw Exception("condition of do statement must be Boolean expression", doStatementNode.Condition()->GetSpan());
    }
    doStatementNode.Statement()->Accept(*this);
    statement.reset(new BoundDoStatement(boundCompileUnit.GetAssembly(), std::unique_ptr<BoundStatement>(statement.release()), std::move(condition)));
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
    forStatementNode.LoopS()->Accept(*this);
    BoundStatement* loopS = statement.release();
    forStatementNode.ActionS()->Accept(*this);
    BoundStatement* actionS = statement.release();
    containerScope = prevContainerScope;
    statement.reset(new BoundForStatement(boundCompileUnit.GetAssembly(), std::unique_ptr<BoundStatement>(initS), std::move(condition), std::unique_ptr<BoundStatement>(loopS),
        std::unique_ptr<BoundStatement>(actionS)));
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
}

void StatementBinderVisitor::Visit(ConstructionStatementNode& constructionStatementNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(constructionStatementNode);
    LocalVariableSymbol* localVariableSymbol = dynamic_cast<LocalVariableSymbol*>(symbol);
    Assert(localVariableSymbol, "local variable symbol expected");
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    arguments.push_back(std::unique_ptr<BoundExpression>(new BoundLocalVariable(boundCompileUnit.GetAssembly(), localVariableSymbol->GetType(), localVariableSymbol)));
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, localVariableSymbol->GetType()->ClassOrNsScope()));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
    int n = constructionStatementNode.Arguments().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* argumentNode = constructionStatementNode.Arguments()[i];
        std::unique_ptr<BoundExpression> argument = BindExpression(boundCompileUnit, function, containerScope, argumentNode);
        arguments.push_back(std::move(argument));
    }
    std::unique_ptr<BoundFunctionCall> constructorCall = ResolveOverload(boundCompileUnit, U"@init", functionScopeLookups, arguments, constructionStatementNode.GetSpan());
    CheckAccess(function->GetFunctionSymbol(), constructorCall->GetFunctionSymbol());
    statement.reset(new BoundConstructionStatement(boundCompileUnit.GetAssembly(), std::move(constructorCall)));
}

void StatementBinderVisitor::Visit(AssignmentStatementNode& assignmentStatementNode)
{
    std::unique_ptr<BoundExpression> target = BindExpression(boundCompileUnit, function, containerScope, assignmentStatementNode.TargetExpr());
    TypeSymbol* targetType = target->GetType();
    std::unique_ptr<BoundExpression> source = BindExpression(boundCompileUnit, function, containerScope, assignmentStatementNode.SourceExpr());
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    arguments.push_back(std::move(target));
    arguments.push_back(std::move(source));
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, targetType->ClassOrNsScope()));
    std::unique_ptr<BoundFunctionCall> assignmentCall = ResolveOverload(boundCompileUnit, U"@assignment", functionScopeLookups, arguments, assignmentStatementNode.GetSpan());
    CheckAccess(function->GetFunctionSymbol(), assignmentCall->GetFunctionSymbol());
    statement.reset(new BoundAssignmentStatement(boundCompileUnit.GetAssembly(), std::move(assignmentCall)));
}

void StatementBinderVisitor::Visit(ExpressionStatementNode& expressionStatementNode)
{
    std::unique_ptr<BoundExpression> expression = BindExpression(boundCompileUnit, function, containerScope, expressionStatementNode.Expression());
    statement.reset(new BoundExpressionStatement(boundCompileUnit.GetAssembly(), std::move(expression)));
}

void StatementBinderVisitor::Visit(EmptyStatementNode& emptyStatementNode)
{
    statement.reset(new BoundEmptyStatement(boundCompileUnit.GetAssembly()));
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

} } // namespace cminor::binder
