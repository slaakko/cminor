// =================================
// Copyright (c) 2016 Seppo Laakko
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

MappingSymbolVisitor::MappingSymbolVisitor(Assembly& targetAssembly_, Assembly& sourceAssembly_) : targetAssembly(targetAssembly_), sourceAssembly(sourceAssembly_), implementedInterfaces(nullptr)
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
    Symbol* symbol = sourceAssembly.GetSymbolTable().GetSymbol(classNode.SymbolId());
    ClassTypeSymbol* classTypeSymbol = dynamic_cast<ClassTypeSymbol*>(symbol);
    Assert(classTypeSymbol, "class type symbol expected");
    targetAssembly.GetSymbolTable().MapNode(classNode, classTypeSymbol);
    targetAssembly.GetSymbolTable().BeginContainer(classTypeSymbol);
    classTypeSymbol->SetAssembly(&targetAssembly);
    ConstantPool& constantPool = targetAssembly.GetConstantPool();
    constantPool.Install(classTypeSymbol->Name());
    utf32_string fullName = classTypeSymbol->FullName();
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
    Symbol* symbol = sourceAssembly.GetSymbolTable().GetSymbol(functionNode.SymbolId());
    FunctionSymbol* functionSymbol = dynamic_cast<FunctionSymbol*>(symbol);
    Assert(functionSymbol, "function symbol expected");
    targetAssembly.GetSymbolTable().MapNode(functionNode, functionSymbol);
    targetAssembly.GetSymbolTable().BeginContainer(functionSymbol);
    functionSymbol->SetAssembly(&targetAssembly);
    functionSymbol->SetProject();
    ConstantPool& constantPool = targetAssembly.GetConstantPool();
    constantPool.Install(functionSymbol->Name());
    utf32_string fullName = functionSymbol->FullName();
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
    Symbol* symbol = sourceAssembly.GetSymbolTable().GetSymbol(parameterNode.SymbolId());
    ParameterSymbol* parameterSymbol = dynamic_cast<ParameterSymbol*>(symbol);
    Assert(parameterSymbol, "parameter symbol expected");
    targetAssembly.GetSymbolTable().MapNode(parameterNode, parameterSymbol);
}

void MappingSymbolVisitor::Visit(StaticConstructorNode& staticConstructorNode)
{
    Symbol* symbol = sourceAssembly.GetSymbolTable().GetSymbol(staticConstructorNode.SymbolId());
    StaticConstructorSymbol* staticConstructorSymbol = dynamic_cast<StaticConstructorSymbol*>(symbol);
    Assert(staticConstructorSymbol, "static constructor symbol expected");
    targetAssembly.GetSymbolTable().MapNode(staticConstructorNode, staticConstructorSymbol);
    targetAssembly.GetSymbolTable().BeginContainer(staticConstructorSymbol);
    staticConstructorSymbol->SetAssembly(&targetAssembly);
    staticConstructorSymbol->SetProject();
    ConstantPool& constantPool = targetAssembly.GetConstantPool();
    constantPool.Install(staticConstructorSymbol->Name());
    utf32_string fullName = staticConstructorSymbol->FullName();
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
    Symbol* symbol = sourceAssembly.GetSymbolTable().GetSymbol(constructorNode.SymbolId());
    ConstructorSymbol* constructorSymbol = dynamic_cast<ConstructorSymbol*>(symbol);
    Assert(constructorSymbol, "constructor symbol expected");
    targetAssembly.GetSymbolTable().MapNode(constructorNode, constructorSymbol);
    targetAssembly.GetSymbolTable().BeginContainer(constructorSymbol);
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
        utf32_string fullName = constructorSymbol->FullName();
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
    Symbol* symbol = sourceAssembly.GetSymbolTable().GetSymbol(memberFunctionNode.SymbolId());
    MemberFunctionSymbol* memberFunctionSymbol = dynamic_cast<MemberFunctionSymbol*>(symbol);
    Assert(memberFunctionSymbol, "member function symbol expected");
    targetAssembly.GetSymbolTable().MapNode(memberFunctionNode, memberFunctionSymbol);
    targetAssembly.GetSymbolTable().BeginContainer(memberFunctionSymbol);
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
        utf32_string fullName = memberFunctionSymbol->FullName();
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
    Symbol* symbol = sourceAssembly.GetSymbolTable().GetSymbol(memberVariableNode.SymbolId());
    MemberVariableSymbol* memberVariableSymbol = dynamic_cast<MemberVariableSymbol*>(symbol);
    Assert(memberVariableSymbol, "member variable symbol expected");
    targetAssembly.GetSymbolTable().MapNode(memberVariableNode, memberVariableSymbol);
}

void MappingSymbolVisitor::Visit(PropertyNode& propertyNode)
{
    Symbol* symbol = sourceAssembly.GetSymbolTable().GetSymbol(propertyNode.SymbolId());
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
            utf32_string fullName = propertySymbol->FullName();
            constantPool.Install(StringPtr(fullName.c_str()));

            propertySymbol->Getter()->SetAssembly(&targetAssembly);
            propertySymbol->Getter()->SetProject();
            constantPool.Install(propertySymbol->Getter()->Name());
            utf32_string getterFullName = propertySymbol->Getter()->FullName();
            constantPool.Install(StringPtr(getterFullName.c_str()));
            constantPool.Install(propertySymbol->Getter()->GroupName());

            propertyNode.Getter()->Accept(*this);
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
            utf32_string fullName = propertySymbol->FullName();
            constantPool.Install(StringPtr(fullName.c_str()));

            propertySymbol->Setter()->SetAssembly(&targetAssembly);
            propertySymbol->Setter()->SetProject();
            constantPool.Install(propertySymbol->Setter()->Name());
            utf32_string setterFullName = propertySymbol->Setter()->FullName();
            constantPool.Install(StringPtr(setterFullName.c_str()));
            constantPool.Install(propertySymbol->Setter()->GroupName());

            propertyNode.Setter()->Accept(*this);
        }
    }
    targetAssembly.GetSymbolTable().EndContainer();
}

void MappingSymbolVisitor::Visit(IndexerNode& indexerNode)
{
    Symbol* symbol = sourceAssembly.GetSymbolTable().GetSymbol(indexerNode.SymbolId());
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
            utf32_string fullName = indexerSymbol->FullName();
            constantPool.Install(StringPtr(fullName.c_str()));

            indexerSymbol->Getter()->SetAssembly(&targetAssembly);
            indexerSymbol->Getter()->SetProject();
            constantPool.Install(indexerSymbol->Getter()->Name());
            utf32_string getterFullName = indexerSymbol->Getter()->FullName();
            constantPool.Install(StringPtr(getterFullName.c_str()));
            constantPool.Install(indexerSymbol->Getter()->GroupName());

            indexerNode.Getter()->Accept(*this);
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
            utf32_string fullName = indexerSymbol->FullName();
            constantPool.Install(StringPtr(fullName.c_str()));

            indexerSymbol->Setter()->SetAssembly(&targetAssembly);
            indexerSymbol->Setter()->SetProject();
            constantPool.Install(indexerSymbol->Setter()->Name());
            utf32_string setterFullName = indexerSymbol->Setter()->FullName();
            constantPool.Install(StringPtr(setterFullName.c_str()));
            constantPool.Install(indexerSymbol->Setter()->GroupName());

            indexerNode.Setter()->Accept(*this);
        }
    }
    targetAssembly.GetSymbolTable().EndContainer();
}

void MappingSymbolVisitor::Visit(CompoundStatementNode& compoundStatementNode)
{
    Symbol* symbol = sourceAssembly.GetSymbolTable().GetSymbolNothrow(compoundStatementNode.SymbolId());
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
    Symbol* symbol = sourceAssembly.GetSymbolTable().GetSymbolNothrow(forStatementNode.SymbolId());
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

void MappingSymbolVisitor::Visit(ConstructionStatementNode& constructionStatementNode)
{
    Symbol* symbol = sourceAssembly.GetSymbolTable().GetSymbolNothrow(constructionStatementNode.SymbolId());
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
