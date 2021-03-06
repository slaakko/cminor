// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/MappingSymbolVisitor.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/PropertySymbol.hpp>
#include <cminor/symbols/IndexerSymbol.hpp>
#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/ast/CompileUnit.hpp>
#include <cminor/ast/Statement.hpp>
#include <cminor/ast/Class.hpp>
#include <cminor/ast/Interface.hpp>

namespace cminor { namespace symbols {

MappingSymbolVisitor::MappingSymbolVisitor(Assembly& targetAssembly_, Assembly& sourceAssembly_) : targetAssembly(targetAssembly_), sourceAssembly(&sourceAssembly_), implementedInterfaces(nullptr)
{
}

void MappingSymbolVisitor::SetImplementedInterfaces(std::vector<InterfaceTypeSymbol*>* implementedInterfaces_)
{
    implementedInterfaces = implementedInterfaces_;
}

void MappingSymbolVisitor::Visit(NamespaceNode& namespaceNode)
{
    targetAssembly.GetSymbolTable().BeginNamespace(namespaceNode);
    NodeList<Node>& members = namespaceNode.Members();
    int n = members.Count();
    for (int i = 0; i < n; ++i)
    {
        Node* member = members[i];
        member->Accept(*this);
    }
    targetAssembly.GetSymbolTable().EndNamespace();
}

void MappingSymbolVisitor::Visit(ClassNode& classNode)
{
    Symbol* symbol = sourceAssembly->GetSymbolTable().GetSymbol(classNode.SymbolId());
    ClassTypeSymbol* classTypeSymbol = dynamic_cast<ClassTypeSymbol*>(symbol);
    Assert(classTypeSymbol, "class type symbol expected");
    targetAssembly.GetSymbolTable().MapNode(classNode, classTypeSymbol);
    targetAssembly.GetSymbolTable().BeginContainer(classTypeSymbol);
    classTypeSymbol->SetAssembly(&targetAssembly);
    ConstantPool& constantPool = targetAssembly.GetConstantPool();
    constantPool.Install(classTypeSymbol->Name());
    std::u32string fullName = classTypeSymbol->FullName();
    constantPool.Install(StringPtr(fullName.c_str()));
    int n = classNode.Members().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* member = classNode.Members()[i];
        member->Accept(*this);
    }
    targetAssembly.GetSymbolTable().EndContainer();
}

void MappingSymbolVisitor::Visit(FunctionNode& functionNode)
{
    Symbol* symbol = sourceAssembly->GetSymbolTable().GetSymbol(functionNode.SymbolId());
    FunctionSymbol* functionSymbol = dynamic_cast<FunctionSymbol*>(symbol);
    Assert(functionSymbol, "function symbol expected");
    targetAssembly.GetSymbolTable().MapNode(functionNode, functionSymbol);
    targetAssembly.GetSymbolTable().BeginContainer(functionSymbol);
    targetAssembly.GetSymbolTable().SetFunction(functionSymbol);
    functionSymbol->SetAssembly(&targetAssembly);
    functionSymbol->SetProject();
    ConstantPool& constantPool = targetAssembly.GetConstantPool();
    constantPool.Install(functionSymbol->Name());
    std::u32string fullName = functionSymbol->FullName();
    constantPool.Install(StringPtr(fullName.c_str()));
    int n = functionNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = functionNode.Parameters()[i];
        parameterNode->Accept(*this);
    }
    if (functionSymbol->IsInstantiationRequested())
    {
        functionNode.SwitchToBody();
        if (functionNode.HasBody())
        {
            functionNode.Body()->Accept(*this);
        }
    }
    targetAssembly.GetSymbolTable().EndContainer();
}

void MappingSymbolVisitor::Visit(ParameterNode& parameterNode)
{
    Symbol* symbol = sourceAssembly->GetSymbolTable().GetSymbol(parameterNode.SymbolId());
    ParameterSymbol* parameterSymbol = dynamic_cast<ParameterSymbol*>(symbol);
    Assert(parameterSymbol, "parameter symbol expected");
    targetAssembly.GetSymbolTable().MapNode(parameterNode, parameterSymbol);
}

void MappingSymbolVisitor::Visit(StaticConstructorNode& staticConstructorNode)
{
    Symbol* symbol = sourceAssembly->GetSymbolTable().GetSymbol(staticConstructorNode.SymbolId());
    StaticConstructorSymbol* staticConstructorSymbol = dynamic_cast<StaticConstructorSymbol*>(symbol);
    Assert(staticConstructorSymbol, "static constructor symbol expected");
    targetAssembly.GetSymbolTable().MapNode(staticConstructorNode, staticConstructorSymbol);
    targetAssembly.GetSymbolTable().BeginContainer(staticConstructorSymbol);
    targetAssembly.GetSymbolTable().SetFunction(staticConstructorSymbol);
    staticConstructorSymbol->SetAssembly(&targetAssembly);
    staticConstructorSymbol->SetProject();
    ConstantPool& constantPool = targetAssembly.GetConstantPool();
    constantPool.Install(staticConstructorSymbol->Name());
    std::u32string fullName = staticConstructorSymbol->FullName();
    constantPool.Install(StringPtr(fullName.c_str()));
    constantPool.Install(staticConstructorSymbol->GroupName());
    if (staticConstructorSymbol->IsInstantiationRequested())
    {
        staticConstructorNode.SwitchToBody();
        if (staticConstructorNode.HasBody())
        {
            staticConstructorNode.Body()->Accept(*this);
        }
    }
    targetAssembly.GetSymbolTable().EndContainer();
}

void MappingSymbolVisitor::Visit(ConstructorNode& constructorNode)
{
    Symbol* symbol = sourceAssembly->GetSymbolTable().GetSymbol(constructorNode.SymbolId());
    ConstructorSymbol* constructorSymbol = dynamic_cast<ConstructorSymbol*>(symbol);
    Assert(constructorSymbol, "constructor symbol expected");
    targetAssembly.GetSymbolTable().MapNode(constructorNode, constructorSymbol);
    targetAssembly.GetSymbolTable().BeginContainer(constructorSymbol);
    targetAssembly.GetSymbolTable().SetFunction(constructorSymbol);
    int n = constructorNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = constructorNode.Parameters()[i];
        parameterNode->Accept(*this);
    }
    if (constructorSymbol->IsInstantiationRequested())
    {
        constructorSymbol->SetAssembly(&targetAssembly);
        constructorSymbol->SetProject();
        ConstantPool& constantPool = targetAssembly.GetConstantPool();
        constantPool.Install(constructorSymbol->Name());
        std::u32string fullName = constructorSymbol->FullName();
        constantPool.Install(StringPtr(fullName.c_str()));
        constantPool.Install(constructorSymbol->GroupName());
        constructorNode.SwitchToBody();
        if (constructorNode.HasBody())
        {
            constructorNode.Body()->Accept(*this);
        }
    }
    targetAssembly.GetSymbolTable().EndContainer();
}

void MappingSymbolVisitor::Visit(MemberFunctionNode& memberFunctionNode)
{
    Symbol* symbol = sourceAssembly->GetSymbolTable().GetSymbol(memberFunctionNode.SymbolId());
    MemberFunctionSymbol* memberFunctionSymbol = dynamic_cast<MemberFunctionSymbol*>(symbol);
    Assert(memberFunctionSymbol, "member function symbol expected");
    targetAssembly.GetSymbolTable().MapNode(memberFunctionNode, memberFunctionSymbol);
    targetAssembly.GetSymbolTable().BeginContainer(memberFunctionSymbol);
    targetAssembly.GetSymbolTable().SetFunction(memberFunctionSymbol);
    int n = memberFunctionNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = memberFunctionNode.Parameters()[i];
        parameterNode->Accept(*this);
    }
    if (memberFunctionSymbol->IsVirtualAbstractOrOverride() && !memberFunctionSymbol->IsInstantiated())
    {
        memberFunctionSymbol->SetInstantiationRequested();
    }
    else if (implementedInterfaces && !memberFunctionSymbol->IsInstantiated())
    {
        for (InterfaceTypeSymbol* intf : *implementedInterfaces)
        {
            if (memberFunctionSymbol->ImplementsInterfaceMemFun(intf))
            {
                memberFunctionSymbol->SetInstantiationRequested();
                break;
            }
        }
    }
    if (memberFunctionSymbol->IsInstantiationRequested())
    {
        memberFunctionSymbol->SetAssembly(&targetAssembly);
        memberFunctionSymbol->SetProject();
        ConstantPool& constantPool = targetAssembly.GetConstantPool();
        constantPool.Install(memberFunctionSymbol->Name());
        std::u32string fullName = memberFunctionSymbol->FullName();
        constantPool.Install(StringPtr(fullName.c_str()));
        constantPool.Install(memberFunctionSymbol->GroupName());
        memberFunctionNode.SwitchToBody();
        if (memberFunctionNode.HasBody())
        {
            memberFunctionNode.Body()->Accept(*this);
        }
    }
    targetAssembly.GetSymbolTable().EndContainer();
}

void MappingSymbolVisitor::Visit(MemberVariableNode& memberVariableNode)
{
    Symbol* symbol = sourceAssembly->GetSymbolTable().GetSymbol(memberVariableNode.SymbolId());
    MemberVariableSymbol* memberVariableSymbol = dynamic_cast<MemberVariableSymbol*>(symbol);
    Assert(memberVariableSymbol, "member variable symbol expected");
    targetAssembly.GetSymbolTable().MapNode(memberVariableNode, memberVariableSymbol);
}

void MappingSymbolVisitor::Visit(PropertyNode& propertyNode)
{
    Symbol* symbol = sourceAssembly->GetSymbolTable().GetSymbol(propertyNode.SymbolId());
    PropertySymbol* propertySymbol = dynamic_cast<PropertySymbol*>(symbol);
    Assert(propertySymbol, "property symbol expected");
    targetAssembly.GetSymbolTable().MapNode(propertyNode, propertySymbol);
    targetAssembly.GetSymbolTable().BeginContainer(propertySymbol);
    if (propertyNode.Getter())
    {
        if (propertySymbol->Getter()->IsInstantiationRequested())
        {
            propertySymbol->SetAssembly(&targetAssembly);
            propertySymbol->SetProject();
            ConstantPool& constantPool = targetAssembly.GetConstantPool();
            constantPool.Install(propertySymbol->Name());
            std::u32string fullName = propertySymbol->FullName();
            constantPool.Install(StringPtr(fullName.c_str()));

            targetAssembly.GetSymbolTable().SetFunction(propertySymbol->Getter());
            propertySymbol->Getter()->SetAssembly(&targetAssembly);
            propertySymbol->Getter()->SetProject();
            constantPool.Install(propertySymbol->Getter()->Name());
            std::u32string getterFullName = propertySymbol->Getter()->FullName();
            constantPool.Install(StringPtr(getterFullName.c_str()));
            constantPool.Install(propertySymbol->Getter()->GroupName());

            Assembly* prevSourceAssembly = sourceAssembly;
            Assembly* originalSourceAssembly = propertySymbol->Getter()->GetOriginalSourceAssembly();
            if (originalSourceAssembly)
            {
                sourceAssembly = originalSourceAssembly;
            }
            propertyNode.Getter()->Accept(*this);
            if (originalSourceAssembly)
            {
                sourceAssembly = prevSourceAssembly;
            }
        }
        else if (!propertySymbol->Getter()->GetOriginalSourceAssembly())
        {
            propertySymbol->Getter()->SetOriginalSourceAssembly(sourceAssembly);
        }
    }
    if (propertyNode.Setter())
    {
        if (propertySymbol->Setter()->IsInstantiationRequested())
        {
            propertySymbol->SetAssembly(&targetAssembly);
            propertySymbol->SetProject();
            ConstantPool& constantPool = targetAssembly.GetConstantPool();
            constantPool.Install(propertySymbol->Name());
            std::u32string fullName = propertySymbol->FullName();
            constantPool.Install(StringPtr(fullName.c_str()));

            targetAssembly.GetSymbolTable().SetFunction(propertySymbol->Setter());
            propertySymbol->Setter()->SetAssembly(&targetAssembly);
            propertySymbol->Setter()->SetProject();
            constantPool.Install(propertySymbol->Setter()->Name());
            std::u32string setterFullName = propertySymbol->Setter()->FullName();
            constantPool.Install(StringPtr(setterFullName.c_str()));
            constantPool.Install(propertySymbol->Setter()->GroupName());
            Assembly* prevSourceAssembly = sourceAssembly;
            Assembly* originalSourceAssembly = propertySymbol->Setter()->GetOriginalSourceAssembly();
            if (originalSourceAssembly)
            {
                sourceAssembly = originalSourceAssembly;
            }
            propertyNode.Setter()->Accept(*this);
            if (originalSourceAssembly)
            {
                sourceAssembly = prevSourceAssembly;
            }
        }
        else if (!propertySymbol->Setter()->GetOriginalSourceAssembly())
        {
            propertySymbol->Setter()->SetOriginalSourceAssembly(sourceAssembly);
        }
    }
    targetAssembly.GetSymbolTable().EndContainer();
}

void MappingSymbolVisitor::Visit(IndexerNode& indexerNode)
{
    Symbol* symbol = sourceAssembly->GetSymbolTable().GetSymbol(indexerNode.SymbolId());
    IndexerSymbol* indexerSymbol = dynamic_cast<IndexerSymbol*>(symbol);
    Assert(indexerSymbol, "indexer symbol expected");
    targetAssembly.GetSymbolTable().MapNode(indexerNode, indexerSymbol);
    targetAssembly.GetSymbolTable().BeginContainer(indexerSymbol);
    if (indexerNode.Getter())
    {
        if (indexerSymbol->Getter()->IsInstantiationRequested())
        {
            indexerSymbol->SetAssembly(&targetAssembly);
            indexerSymbol->SetProject();
            ConstantPool& constantPool = targetAssembly.GetConstantPool();
            constantPool.Install(indexerSymbol->Name());
            std::u32string fullName = indexerSymbol->FullName();
            constantPool.Install(StringPtr(fullName.c_str()));

            targetAssembly.GetSymbolTable().SetFunction(indexerSymbol->Getter());
            indexerSymbol->Getter()->SetAssembly(&targetAssembly);
            indexerSymbol->Getter()->SetProject();
            constantPool.Install(indexerSymbol->Getter()->Name());
            std::u32string getterFullName = indexerSymbol->Getter()->FullName();
            constantPool.Install(StringPtr(getterFullName.c_str()));
            constantPool.Install(indexerSymbol->Getter()->GroupName());

            Assembly* prevSourceAssembly = sourceAssembly;
            Assembly* originalSourceAssembly = indexerSymbol->Getter()->GetOriginalSourceAssembly();
            if (originalSourceAssembly)
            {
                sourceAssembly = originalSourceAssembly;
            }
            indexerNode.Getter()->Accept(*this);
            if (originalSourceAssembly)
            {
                sourceAssembly = prevSourceAssembly;
            }
        }
        else if (!indexerSymbol->Getter()->GetOriginalSourceAssembly())
        {
            indexerSymbol->Getter()->SetOriginalSourceAssembly(sourceAssembly);
        }
    }
    if (indexerNode.Setter())
    {
        if (indexerSymbol->Setter()->IsInstantiationRequested())
        {
            indexerSymbol->SetAssembly(&targetAssembly);
            indexerSymbol->SetProject();
            ConstantPool& constantPool = targetAssembly.GetConstantPool();
            constantPool.Install(indexerSymbol->Name());
            std::u32string fullName = indexerSymbol->FullName();
            constantPool.Install(StringPtr(fullName.c_str()));

            targetAssembly.GetSymbolTable().SetFunction(indexerSymbol->Setter());
            indexerSymbol->Setter()->SetAssembly(&targetAssembly);
            indexerSymbol->Setter()->SetProject();
            constantPool.Install(indexerSymbol->Setter()->Name());
            std::u32string setterFullName = indexerSymbol->Setter()->FullName();
            constantPool.Install(StringPtr(setterFullName.c_str()));
            constantPool.Install(indexerSymbol->Setter()->GroupName());

            Assembly* prevSourceAssembly = sourceAssembly;
            Assembly* originalSourceAssembly = indexerSymbol->Setter()->GetOriginalSourceAssembly();
            if (originalSourceAssembly)
            {
                sourceAssembly = originalSourceAssembly;
            }
            indexerNode.Setter()->Accept(*this);
            if (originalSourceAssembly)
            {
                sourceAssembly = prevSourceAssembly;
            }
        }
        else if (!indexerSymbol->Setter()->GetOriginalSourceAssembly())
        {
            indexerSymbol->Setter()->SetOriginalSourceAssembly(sourceAssembly);
        }
    }
    targetAssembly.GetSymbolTable().EndContainer();
}

void MappingSymbolVisitor::Visit(CompoundStatementNode& compoundStatementNode)
{
    Symbol* symbol = sourceAssembly->GetSymbolTable().GetSymbolNothrow(compoundStatementNode.SymbolId());
    if (symbol)
    {
        DeclarationBlock* declarationBlock = dynamic_cast<DeclarationBlock*>(symbol);
        Assert(declarationBlock, "declaration block expected");
        targetAssembly.GetSymbolTable().MapNode(compoundStatementNode, declarationBlock);
        targetAssembly.GetSymbolTable().BeginContainer(declarationBlock);
    }
    else
    {
        targetAssembly.GetSymbolTable().BeginDeclarationBlock(compoundStatementNode);
    }
    int n = compoundStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = compoundStatementNode.Statements()[i];
        statementNode->Accept(*this);
    }
    if (symbol)
    {
        targetAssembly.GetSymbolTable().EndContainer();
    }
    else
    {
        targetAssembly.GetSymbolTable().EndDeclarationBlock();
    }
}

void MappingSymbolVisitor::Visit(IfStatementNode& ifStatementNode)
{
    ifStatementNode.ThenS()->Accept(*this);
    if (ifStatementNode.ElseS())
    {
        ifStatementNode.ElseS()->Accept(*this);
    }
}

void MappingSymbolVisitor::Visit(WhileStatementNode& whileStatementNode)
{
    whileStatementNode.Statement()->Accept(*this);
}

void MappingSymbolVisitor::Visit(DoStatementNode& doStatementNode)
{
    doStatementNode.Statement()->Accept(*this);
}

void MappingSymbolVisitor::Visit(ForStatementNode& forStatementNode)
{
    Symbol* symbol = sourceAssembly->GetSymbolTable().GetSymbolNothrow(forStatementNode.SymbolId());
    if (symbol)
    {
        DeclarationBlock* declarationBlock = dynamic_cast<DeclarationBlock*>(symbol);
        Assert(declarationBlock, "declaration block expected");
        targetAssembly.GetSymbolTable().MapNode(forStatementNode, declarationBlock);
        targetAssembly.GetSymbolTable().BeginContainer(declarationBlock);
    }
    else
    {
        targetAssembly.GetSymbolTable().BeginDeclarationBlock(forStatementNode);
    }
    forStatementNode.InitS()->Accept(*this);
    forStatementNode.ActionS()->Accept(*this);
    if (symbol)
    {
        targetAssembly.GetSymbolTable().EndContainer();
    }
    else
    {
        targetAssembly.GetSymbolTable().EndDeclarationBlock();
    }
}

void MappingSymbolVisitor::Visit(SwitchStatementNode& switchStatementNode)
{
    int n = switchStatementNode.Cases().Count();
    for (int i = 0; i < n; ++i)
    {
        CaseStatementNode* caseS = switchStatementNode.Cases()[i];
        caseS->Accept(*this);
    }
    if (switchStatementNode.Default())
    {
        switchStatementNode.Default()->Accept(*this);
    }
}

void MappingSymbolVisitor::Visit(CaseStatementNode& caseStatementNode)
{
    int n = caseStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* s = caseStatementNode.Statements()[i];
        s->Accept(*this);
    }
}

void MappingSymbolVisitor::Visit(DefaultStatementNode& defaultStatementNode) 
{
    int n = defaultStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* s = defaultStatementNode.Statements()[i];
        s->Accept(*this);
    }
}

void MappingSymbolVisitor::Visit(TryStatementNode& tryStatementNode)
{
    tryStatementNode.TryBlock()->Accept(*this); 
    int n = tryStatementNode.Catches().Count();
    for (int i = 0; i < n; ++i)
    {
        CatchNode* catchNode = tryStatementNode.Catches()[i];
        catchNode->Accept(*this);
    }
    if (tryStatementNode.FinallyBlock())
    {
        tryStatementNode.FinallyBlock()->Accept(*this);
    }
}

void MappingSymbolVisitor::Visit(CatchNode& catchNode)
{
    catchNode.CatchBlock()->Accept(*this);
}

void MappingSymbolVisitor::Visit(ConstructionStatementNode& constructionStatementNode)
{
    Symbol* symbol = sourceAssembly->GetSymbolTable().GetSymbolNothrow(constructionStatementNode.SymbolId());
    if (symbol)
    {
        LocalVariableSymbol* localVariableSymbol = dynamic_cast<LocalVariableSymbol*>(symbol);
        Assert(localVariableSymbol, "local variable symbol expected");
        targetAssembly.GetSymbolTable().MapNode(constructionStatementNode, localVariableSymbol);
    }
    else
    {
        targetAssembly.GetSymbolTable().AddLocalVariable(constructionStatementNode);
    }
}

} } // namespace cminor::symbols
