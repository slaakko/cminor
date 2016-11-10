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

MappingSymbolVisitor::MappingSymbolVisitor(Assembly& assembly_) : assembly(assembly_)
{
}

void MappingSymbolVisitor::Visit(NamespaceNode& namespaceNode)
{
    assembly.GetSymbolTable().BeginNamespace(namespaceNode);
    NodeList<Node>& members = namespaceNode.Members();
    int n = members.Count();
    for (int i = 0; i < n; ++i)
    {
        Node* member = members[i];
        member->Accept(*this);
    }
    assembly.GetSymbolTable().EndNamespace();
}

void MappingSymbolVisitor::Visit(ClassNode& classNode)
{
    Symbol* symbol = assembly.GetSymbolTable().GetSymbol(classNode.SymbolId());
    ClassTypeSymbol* classTypeSymbol = dynamic_cast<ClassTypeSymbol*>(symbol);
    Assert(classTypeSymbol, "class type symbol expected");
    assembly.GetSymbolTable().MapNode(classNode, classTypeSymbol);
    assembly.GetSymbolTable().BeginContainer(classTypeSymbol);
    int n = classNode.Members().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* member = classNode.Members()[i];
        member->Accept(*this);
    }
    assembly.GetSymbolTable().EndContainer();
}

void MappingSymbolVisitor::Visit(FunctionNode& functionNode)
{
    Symbol* symbol = assembly.GetSymbolTable().GetSymbol(functionNode.SymbolId());
    FunctionSymbol* functionSymbol = dynamic_cast<FunctionSymbol*>(symbol);
    Assert(functionSymbol, "function symbol expected");
    assembly.GetSymbolTable().MapNode(functionNode, functionSymbol);
    assembly.GetSymbolTable().BeginContainer(functionSymbol);
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
        assembly.GetSymbolTable().EndContainer();
    }
}

void MappingSymbolVisitor::Visit(ParameterNode& parameterNode)
{
    Symbol* symbol = assembly.GetSymbolTable().GetSymbol(parameterNode.SymbolId());
    ParameterSymbol* parameterSymbol = dynamic_cast<ParameterSymbol*>(symbol);
    Assert(parameterSymbol, "parameter symbol expected");
    assembly.GetSymbolTable().MapNode(parameterNode, parameterSymbol);
}

void MappingSymbolVisitor::Visit(StaticConstructorNode& staticConstructorNode)
{
    Symbol* symbol = assembly.GetSymbolTable().GetSymbol(staticConstructorNode.SymbolId());
    StaticConstructorSymbol* staticConstructorSymbol = dynamic_cast<StaticConstructorSymbol*>(symbol);
    Assert(staticConstructorSymbol, "static constructor symbol expected");
    assembly.GetSymbolTable().MapNode(staticConstructorNode, staticConstructorSymbol);
    assembly.GetSymbolTable().BeginContainer(staticConstructorSymbol);
    if (staticConstructorSymbol->IsInstantiationRequested())
    {
        staticConstructorNode.SwitchToBody();
        if (staticConstructorNode.HasBody())
        {
            staticConstructorNode.Body()->Accept(*this);
        }
        assembly.GetSymbolTable().EndContainer();
    }
}

void MappingSymbolVisitor::Visit(ConstructorNode& constructorNode)
{
    Symbol* symbol = assembly.GetSymbolTable().GetSymbol(constructorNode.SymbolId());
    ConstructorSymbol* constructorSymbol = dynamic_cast<ConstructorSymbol*>(symbol);
    Assert(constructorSymbol, "constructor symbol expected");
    assembly.GetSymbolTable().MapNode(constructorNode, constructorSymbol);
    assembly.GetSymbolTable().BeginContainer(constructorSymbol);
    int n = constructorNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = constructorNode.Parameters()[i];
        parameterNode->Accept(*this);
    }
    if (constructorSymbol->IsInstantiationRequested())
    {
        constructorNode.SwitchToBody();
        if (constructorNode.HasBody())
        {
            constructorNode.Body()->Accept(*this);
        }
        assembly.GetSymbolTable().EndContainer();
    }
}

void MappingSymbolVisitor::Visit(MemberFunctionNode& memberFunctionNode)
{
    Symbol* symbol = assembly.GetSymbolTable().GetSymbol(memberFunctionNode.SymbolId());
    MemberFunctionSymbol* memberFunctionSymbol = dynamic_cast<MemberFunctionSymbol*>(symbol);
    Assert(memberFunctionSymbol, "member function symbol expected");
    assembly.GetSymbolTable().MapNode(memberFunctionNode, memberFunctionSymbol);
    assembly.GetSymbolTable().BeginContainer(memberFunctionSymbol);
    int n = memberFunctionNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = memberFunctionNode.Parameters()[i];
        parameterNode->Accept(*this);
    }
    if (memberFunctionSymbol->IsInstantiationRequested())
    {
        memberFunctionNode.SwitchToBody();
        if (memberFunctionNode.HasBody())
        {
            memberFunctionNode.Body()->Accept(*this);
        }
        assembly.GetSymbolTable().EndContainer();
    }
}

void MappingSymbolVisitor::Visit(MemberVariableNode& memberVariableNode)
{
    Symbol* symbol = assembly.GetSymbolTable().GetSymbol(memberVariableNode.SymbolId());
    MemberVariableSymbol* memberVariableSymbol = dynamic_cast<MemberVariableSymbol*>(symbol);
    Assert(memberVariableSymbol, "member variable symbol expected");
    assembly.GetSymbolTable().MapNode(memberVariableNode, memberVariableSymbol);
}

void MappingSymbolVisitor::Visit(PropertyNode& propertyNode)
{
    Symbol* symbol = assembly.GetSymbolTable().GetSymbol(propertyNode.SymbolId());
    PropertySymbol* propertySymbol = dynamic_cast<PropertySymbol*>(symbol);
    Assert(propertySymbol, "property symbol expected");
    assembly.GetSymbolTable().MapNode(propertyNode, propertySymbol);
    assembly.GetSymbolTable().BeginContainer(propertySymbol);
    if (propertyNode.Getter())
    {
        if (propertySymbol->Getter()->IsInstantiationRequested())
        {
            propertyNode.Getter()->Accept(*this);
        }
    }
    if (propertyNode.Setter())
    {
        if (propertySymbol->Setter()->IsInstantiationRequested())
        {
            propertyNode.Setter()->Accept(*this);
        }
    }
    assembly.GetSymbolTable().EndContainer();
}

void MappingSymbolVisitor::Visit(IndexerNode& indexerNode)
{
    Symbol* symbol = assembly.GetSymbolTable().GetSymbol(indexerNode.SymbolId());
    IndexerSymbol* indexerSymbol = dynamic_cast<IndexerSymbol*>(symbol);
    Assert(indexerSymbol, "indexer symbol expected");
    assembly.GetSymbolTable().MapNode(indexerNode, indexerSymbol);
    assembly.GetSymbolTable().BeginContainer(indexerSymbol);
    if (indexerNode.Getter())
    {
        if (indexerSymbol->Getter()->IsInstantiationRequested())
        {
            indexerNode.Getter()->Accept(*this);
        }
    }
    if (indexerNode.Setter())
    {
        if (indexerSymbol->Setter()->IsInstantiationRequested())
        {
            indexerNode.Setter()->Accept(*this);
        }
    }
    assembly.GetSymbolTable().EndContainer();
}

void MappingSymbolVisitor::Visit(CompoundStatementNode& compoundStatementNode)
{
    Symbol* symbol = assembly.GetSymbolTable().GetSymbolNothrow(compoundStatementNode.SymbolId());
    if (symbol)
    {
        DeclarationBlock* declarationBlock = dynamic_cast<DeclarationBlock*>(symbol);
        Assert(declarationBlock, "declaration block expected");
        assembly.GetSymbolTable().MapNode(compoundStatementNode, declarationBlock);
        assembly.GetSymbolTable().BeginContainer(declarationBlock);
    }
    else
    {
        assembly.GetSymbolTable().BeginDeclarationBlock(compoundStatementNode);
    }
    int n = compoundStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = compoundStatementNode.Statements()[i];
        statementNode->Accept(*this);
    }
    if (symbol)
    {
        assembly.GetSymbolTable().EndContainer();
    }
    else
    {
        assembly.GetSymbolTable().EndDeclarationBlock();
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
    Symbol* symbol = assembly.GetSymbolTable().GetSymbolNothrow(forStatementNode.SymbolId());
    if (symbol)
    {
        DeclarationBlock* declarationBlock = dynamic_cast<DeclarationBlock*>(symbol);
        Assert(declarationBlock, "declaration block expected");
        assembly.GetSymbolTable().MapNode(forStatementNode, declarationBlock);
        assembly.GetSymbolTable().BeginContainer(declarationBlock);
    }
    else
    {
        assembly.GetSymbolTable().BeginDeclarationBlock(forStatementNode);
    }
    forStatementNode.InitS()->Accept(*this);
    forStatementNode.ActionS()->Accept(*this);
    if (symbol)
    {
        assembly.GetSymbolTable().EndContainer();
    }
    else
    {
        assembly.GetSymbolTable().EndDeclarationBlock();
    }
}

void MappingSymbolVisitor::Visit(ConstructionStatementNode& constructionStatementNode)
{
    Symbol* symbol = assembly.GetSymbolTable().GetSymbolNothrow(constructionStatementNode.SymbolId());
    if (symbol)
    {
        LocalVariableSymbol* localVariableSymbol = dynamic_cast<LocalVariableSymbol*>(symbol);
        Assert(localVariableSymbol, "local variable symbol expected");
        assembly.GetSymbolTable().MapNode(constructionStatementNode, localVariableSymbol);
    }
    else
    {
        assembly.GetSymbolTable().AddLocalVariable(constructionStatementNode);
    }
}

} } // namespace cminor::symbols
