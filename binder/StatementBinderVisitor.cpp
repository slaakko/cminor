// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/StatementBinderVisitor.hpp>
#include <cminor/binder/BoundCompileUnit.hpp>
#include <cminor/binder/BoundFunction.hpp>
#include <cminor/binder/BoundStatement.hpp>
#include <cminor/binder/ExpressionBinder.hpp>
#include <cminor/binder/OverloadResolution.hpp>
#include <cminor/ast/CompileUnit.hpp>

namespace cminor { namespace binder {

StatementBinderVisitor::StatementBinderVisitor(BoundCompileUnit& boundCompileUnit_) : boundCompileUnit(boundCompileUnit_), containerScope(nullptr), function(nullptr), compoundStatement(nullptr)
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

void StatementBinderVisitor::Visit(FunctionNode& functionNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(functionNode);
    containerScope = symbol->GetContainerScope();
    std::unique_ptr<BoundFunction> boundFunction(new BoundFunction());
    BoundFunction* prevFunction = function;
    function = boundFunction.get();
    functionNode.Body()->Accept(*this);
    boundCompileUnit.AddBoundNode(std::move(boundFunction));
    containerScope = prevContainerScope;
    function = prevFunction;
}

void StatementBinderVisitor::Visit(CompoundStatementNode& compoundStatementNode)
{
    ContainerScope* prevContainerScope = containerScope;
    containerScope = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(compoundStatementNode)->GetContainerScope();
    BoundCompoundStatement* prevCompoundStatement = compoundStatement;
    std::unique_ptr<BoundCompoundStatement> boundCompoundStatement(new BoundCompoundStatement());
    compoundStatement = boundCompoundStatement.get();
    int n = compoundStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = compoundStatementNode.Statements()[i];
        statementNode->Accept(*this);
    }
    function->SetBody(std::move(boundCompoundStatement));
    compoundStatement = prevCompoundStatement;
    containerScope = prevContainerScope;
}

void StatementBinderVisitor::Visit(ConstructionStatementNode& constructionStatementNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(constructionStatementNode);
    LocalVariableSymbol* localVariableSymbol = dynamic_cast<LocalVariableSymbol*>(symbol);
    Assert(localVariableSymbol, "local variable symbol expected");
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, localVariableSymbol->GetType()->GetContainerScope()));
    int n = constructionStatementNode.Arguments().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* argumentNode = constructionStatementNode.Arguments()[i];
        std::unique_ptr<BoundExpression> argument = BindExpression(boundCompileUnit, containerScope, argumentNode);
        arguments.push_back(std::move(argument));
    }
    std::unique_ptr<BoundFunctionCall> constructorCall = ResolveOverload(boundCompileUnit, U"@constructor", functionScopeLookups, arguments, constructionStatementNode.GetSpan());
    std::unique_ptr<BoundConstructionStatement> boundConstructionStatement(new BoundConstructionStatement(std::move(constructorCall)));
    compoundStatement->AddStatement(std::move(boundConstructionStatement));
}

void StatementBinderVisitor::Visit(AssignmentStatementNode& assignmentStatementNode)
{
    std::unique_ptr<BoundExpression> target = BindExpression(boundCompileUnit, containerScope, assignmentStatementNode.TargetExpr());
    std::unique_ptr<BoundExpression> source = BindExpression(boundCompileUnit, containerScope, assignmentStatementNode.SourceExpr());
    std::unique_ptr<BoundAssignmentStatement> boundAssignmentStatement(new BoundAssignmentStatement(std::move(target), std::move(source)));
    compoundStatement->AddStatement(std::move(boundAssignmentStatement));
}

} } // namespace cminor::binder
