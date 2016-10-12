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
#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/ast/CompileUnit.hpp>

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
    CompoundStatementNode* body = functionNode.Body();
    int n = body->Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statement = body->Statements()[i];
        if (TerminatesFunction(statement, false)) return;
    }
    throw Exception("not all control paths terminate in return or throw statement", functionNode.GetSpan());
}

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
    FunctionSymbol* functionSymbol = dynamic_cast<FunctionSymbol*>(symbol);
    Assert(functionSymbol, "function symbol expected");
    containerScope = symbol->GetContainerScope();
    std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(functionSymbol));
    BoundFunction* prevFunction = function;
    function = boundFunction.get();
    functionNode.Body()->Accept(*this);
    CheckFunctionReturnPaths(functionSymbol,functionNode);
    boundCompileUnit.AddBoundNode(std::move(boundFunction));
    containerScope = prevContainerScope;
    function = prevFunction;
}

void StatementBinderVisitor::Visit(CompoundStatementNode& compoundStatementNode)
{
    ContainerScope* prevContainerScope = containerScope;
    containerScope = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(compoundStatementNode)->GetContainerScope();
    BoundCompoundStatement* prevCompoundStatement = compoundStatement;
    std::unique_ptr<BoundCompoundStatement> boundCompoundStatement(new BoundCompoundStatement(boundCompileUnit.GetAssembly()));
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
            std::unique_ptr<BoundExpression> expression = BindExpression(boundCompileUnit, containerScope, returnStatementNode.Expression());
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
            std::unique_ptr<BoundReturnStatement> boundReturnStatement(new BoundReturnStatement(boundCompileUnit.GetAssembly(), std::move(returnFunctionCall)));
            compoundStatement->AddStatement(std::move(boundReturnStatement));
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
            std::unique_ptr<BoundReturnStatement> boundReturnStatement(new BoundReturnStatement(boundCompileUnit.GetAssembly(), std::move(returnFunctionCall)));
            compoundStatement->AddStatement(std::move(boundReturnStatement));
        }
        else
        {
            throw Exception("nonvoid function must return a value", returnStatementNode.GetSpan(), function->GetFunctionSymbol()->GetSpan());
        }
    }
}

void StatementBinderVisitor::Visit(ConstructionStatementNode& constructionStatementNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(constructionStatementNode);
    LocalVariableSymbol* localVariableSymbol = dynamic_cast<LocalVariableSymbol*>(symbol);
    Assert(localVariableSymbol, "local variable symbol expected");
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    arguments.push_back(std::unique_ptr<BoundExpression>(new BoundLocalVariable(boundCompileUnit.GetAssembly(), localVariableSymbol->GetType(), localVariableSymbol)));
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, localVariableSymbol->GetType()->ClassOrNsScope()));
    int n = constructionStatementNode.Arguments().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* argumentNode = constructionStatementNode.Arguments()[i];
        std::unique_ptr<BoundExpression> argument = BindExpression(boundCompileUnit, containerScope, argumentNode);
        arguments.push_back(std::move(argument));
    }
    std::unique_ptr<BoundFunctionCall> constructorCall = ResolveOverload(boundCompileUnit, U"@constructor", functionScopeLookups, arguments, constructionStatementNode.GetSpan());
    std::unique_ptr<BoundConstructionStatement> boundConstructionStatement(new BoundConstructionStatement(boundCompileUnit.GetAssembly(), std::move(constructorCall)));
    compoundStatement->AddStatement(std::move(boundConstructionStatement));
}

void StatementBinderVisitor::Visit(AssignmentStatementNode& assignmentStatementNode)
{
    std::unique_ptr<BoundExpression> target = BindExpression(boundCompileUnit, containerScope, assignmentStatementNode.TargetExpr());
    std::unique_ptr<BoundExpression> source = BindExpression(boundCompileUnit, containerScope, assignmentStatementNode.SourceExpr());
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    arguments.push_back(std::move(target));
    arguments.push_back(std::move(source));
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, target->GetType()->ClassOrNsScope()));
    std::unique_ptr<BoundFunctionCall> assignmentCall = ResolveOverload(boundCompileUnit, U"@assignment", functionScopeLookups, arguments, assignmentStatementNode.GetSpan());
    std::unique_ptr<BoundAssignmentStatement> boundAssignmentStatement(new BoundAssignmentStatement(boundCompileUnit.GetAssembly(), std::move(assignmentCall)));
    compoundStatement->AddStatement(std::move(boundAssignmentStatement));
}

} } // namespace cminor::binder
