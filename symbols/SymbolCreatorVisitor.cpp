// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/SymbolCreatorVisitor.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/ast/CompileUnit.hpp>
#include <cminor/ast/Statement.hpp>

namespace cminor { namespace symbols {

SymbolCreatorVisitor::SymbolCreatorVisitor(Assembly& assembly_) : assembly(assembly_), symbolTable(assembly.GetSymbolTable())
{
}

void SymbolCreatorVisitor::Visit(CompileUnitNode& compileUnitNode)
{
    compileUnitNode.GlobalNs()->Accept(*this);
}

void SymbolCreatorVisitor::Visit(NamespaceNode& namespaceNode)
{
    symbolTable.BeginNamespace(namespaceNode);
    NodeList<Node>& members = namespaceNode.Members();
    int n = members.Count();
    for (int i = 0; i < n; ++i)
    {
        Node* member = members[i];
        member->Accept(*this);
    }
    symbolTable.EndNamespace();
}

void SymbolCreatorVisitor::Visit(FunctionNode& functionNode)
{
    symbolTable.BeginFunction(functionNode);
    int n = functionNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = functionNode.Parameters()[i];
        parameterNode->Accept(*this);
    }
    functionNode.Body()->Accept(*this);
    symbolTable.EndFunction();
}

void SymbolCreatorVisitor::Visit(ParameterNode& parameterNode)
{
    symbolTable.AddParameter(parameterNode);
}

void SymbolCreatorVisitor::Visit(CompoundStatementNode& compoundStatementNode)
{
    symbolTable.BeginDeclarationBlock(compoundStatementNode);
    int n = compoundStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = compoundStatementNode.Statements()[i];
        statementNode->Accept(*this);
    }
    symbolTable.EndDeclarationBlock();
}

void SymbolCreatorVisitor::Visit(IfStatementNode& ifStatementNode)
{
    ifStatementNode.ThenS()->Accept(*this);
    if (ifStatementNode.ElseS())
    {
        ifStatementNode.ElseS()->Accept(*this);
    }
}

void SymbolCreatorVisitor::Visit(WhileStatementNode& whileStatementNode)
{
    whileStatementNode.Statement()->Accept(*this);
}

void SymbolCreatorVisitor::Visit(DoStatementNode& doStatementNode)
{
    doStatementNode.Statement()->Accept(*this);
}

void SymbolCreatorVisitor::Visit(ForStatementNode& forStatementNode)
{
    symbolTable.BeginDeclarationBlock(forStatementNode);
    forStatementNode.InitS()->Accept(*this);
    forStatementNode.ActionS()->Accept(*this);
    symbolTable.EndDeclarationBlock();
}

void SymbolCreatorVisitor::Visit(ConstructionStatementNode& constructionStatementNode)
{
    symbolTable.AddLocalVariable(constructionStatementNode);
}

} } // namespace cminor::symbols
