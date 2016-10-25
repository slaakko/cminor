// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_STATEMENT_BINDER_VISITOR_INCLUDED
#define CMINOR_BINDER_STATEMENT_BINDER_VISITOR_INCLUDED
#include <cminor/binder/BoundStatement.hpp>
#include <cminor/ast/Visitor.hpp>
#include <cminor/symbols/Symbol.hpp>

namespace cminor { namespace binder {

using namespace cminor::ast;
using namespace cminor::symbols;

class BoundCompileUnit;
class BoundClass;
class BoundFunction;

class StatementBinderVisitor : public Visitor
{
public:
    StatementBinderVisitor(BoundCompileUnit& boundCompileUnit_);
    void Visit(CompileUnitNode& compileUnitNode) override;
    void Visit(NamespaceNode& namespaceNode) override;
    void Visit(ClassNode& classNode) override;
    void Visit(ConstructorNode& constructorNode) override;
    void Visit(BaseInitializerNode& baseInitializerNode) override;
    void Visit(ThisInitializerNode& thisInitializerNode) override;
    void Visit(FunctionNode& functionNode) override;
    void Visit(CompoundStatementNode& compoundStatementNode) override;
    void Visit(ReturnStatementNode& returnStatementNode) override;
    void Visit(IfStatementNode& ifStatementNode) override;
    void Visit(WhileStatementNode& whileStatementNode) override;
    void Visit(DoStatementNode& doStatementNode) override;
    void Visit(ForStatementNode& forStatementNode) override;
    void Visit(BreakStatementNode& breakStatementNode) override;
    void Visit(ContinueStatementNode& continueStatementNode) override;
    void Visit(ConstructionStatementNode& constructionStatementNode) override;
    void Visit(AssignmentStatementNode& assignmentStatementNode) override;
    void Visit(ExpressionStatementNode& expressionStatementNode) override;
    void Visit(EmptyStatementNode& emptyStatementNode) override;
    void Visit(IncrementStatementNode& incrementStatementNode) override;
    void Visit(DecrementStatementNode& decrementStatementNode) override;
private:
    BoundCompileUnit& boundCompileUnit;
    ContainerScope* containerScope;
    BoundClass* boundClass;
    BoundFunction* function;
    BoundCompoundStatement* compoundStatement;
    std::unique_ptr<BoundStatement> statement;
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_STATEMENT_BINDER_VISITOR_INCLUDED
