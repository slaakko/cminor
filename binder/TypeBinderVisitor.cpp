// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/TypeBinderVisitor.hpp>
#include <cminor/binder/BoundCompileUnit.hpp>
#include <cminor/binder/TypeResolver.hpp>
#include <cminor/ast/CompileUnit.hpp>
#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/symbols/VariableSymbol.hpp>

namespace cminor { namespace binder {

TypeBinderVisitor::TypeBinderVisitor(BoundCompileUnit& boundCompileUnit_) : boundCompileUnit(boundCompileUnit_), containerScope(nullptr)
{
}

void TypeBinderVisitor::Visit(CompileUnitNode& compileUnitNode)
{
    boundCompileUnit.AddFileScope(std::unique_ptr<FileScope>(new FileScope()));
    compileUnitNode.GlobalNs()->Accept(*this);
}

void TypeBinderVisitor::Visit(NamespaceNode& namespaceNode)
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

void TypeBinderVisitor::Visit(NamespaceImportNode& namespaceImportNode)
{
    boundCompileUnit.FirstFileScope()->InstallNamespaceImport(containerScope, &namespaceImportNode);
}

void TypeBinderVisitor::Visit(AliasNode& aliasNode)
{
    boundCompileUnit.FirstFileScope()->InstallAlias(containerScope, &aliasNode);
}

void TypeBinderVisitor::Visit(FunctionNode& functionNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(functionNode);
    FunctionSymbol* functionSymbol = dynamic_cast<FunctionSymbol*>(symbol);
    Assert(functionSymbol, "function symbol expected");
    containerScope = functionSymbol->GetContainerScope();
    TypeSymbol* returnType = ResolveType(boundCompileUnit, containerScope, functionNode.ReturnTypeExpr());
    functionSymbol->SetReturnType(returnType);
    functionNode.Body()->Accept(*this);
    containerScope = prevContainerScope;
}

void TypeBinderVisitor::Visit(CompoundStatementNode& compoundStatementNode)
{
    ContainerScope* prevContainerScope = containerScope;
    containerScope = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(compoundStatementNode)->GetContainerScope();
    int n = compoundStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = compoundStatementNode.Statements()[i];
        statementNode->Accept(*this);
    }
    containerScope = prevContainerScope;
}

void TypeBinderVisitor::Visit(ConstructionStatementNode& constructionStatementNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(constructionStatementNode);
    LocalVariableSymbol* localVariableSymbol = dynamic_cast<LocalVariableSymbol*>(symbol);
    Assert(localVariableSymbol, "local variable symbol expected");
    TypeSymbol* type = ResolveType(boundCompileUnit, containerScope, constructionStatementNode.TypeExpr());
    localVariableSymbol->SetType(type);
}

} } // namespace cminor::binder
