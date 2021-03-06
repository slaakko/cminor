// =================================
// Copyright (c) 2017 Seppo Laakko
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
    void SetDoNotInstantiate() { doNotInstantiate = true; }
    void SetInstantiateRequested() { instantiateRequested = true; }
    void SetBoundFunction(BoundFunction* boundFunction) { function = boundFunction; }
    void Visit(CompileUnitNode& compileUnitNode) override;
    void Visit(NamespaceNode& namespaceNode) override;
    void Visit(ClassNode& classNode) override;
    void Visit(ConstructorNode& constructorNode) override;
    void Visit(StaticConstructorNode& staticConstructorNode) override;
    void Visit(BaseInitializerNode& baseInitializerNode) override;
    void Visit(ThisInitializerNode& thisInitializerNode) override;
    void Visit(MemberFunctionNode& memberFunctionNode) override;
    void Visit(FunctionNode& functionNode) override;
    void Visit(PropertyNode& propertyNode) override;
    void Visit(IndexerNode& indexerNode) override;
    void Visit(CompoundStatementNode& compoundStatementNode) override;
    void Visit(ReturnStatementNode& returnStatementNode) override;
    void Visit(IfStatementNode& ifStatementNode) override;
    void Visit(WhileStatementNode& whileStatementNode) override;
    void Visit(DoStatementNode& doStatementNode) override;
    void Visit(ForStatementNode& forStatementNode) override;
    void Visit(BreakStatementNode& breakStatementNode) override;
    void Visit(ContinueStatementNode& continueStatementNode) override;
    void Visit(GotoStatementNode& gotoStatementNode) override;
    void Visit(ConstructionStatementNode& constructionStatementNode) override;
    void Visit(AssignmentStatementNode& assignmentStatementNode) override;
    void Visit(ExpressionStatementNode& expressionStatementNode) override;
    void Visit(EmptyStatementNode& emptyStatementNode) override;
    void Visit(IncrementStatementNode& incrementStatementNode) override;
    void Visit(DecrementStatementNode& decrementStatementNode) override;
    void Visit(ForEachStatementNode& forEachStatementNode) override;
    void Visit(SwitchStatementNode& switchStatementNode) override;
    void Visit(CaseStatementNode& caseStatementNode) override;
    void Visit(DefaultStatementNode& defaultStatementNode) override;
    void Visit(GotoCaseStatementNode& gotoCaseStatementNode) override;
    void Visit(GotoDefaultStatementNode& gotoDefaultStatementNode) override;
    void Visit(ThrowStatementNode& throwStatementNode) override;
    void Visit(TryStatementNode& tryStatementNode) override;
    void Visit(CatchNode& catchNode) override;
    void Visit(UsingStatementNode& usingStatementNode) override;
    void Visit(LockStatementNode& lockStatementNode) override;
    BoundStatement* ReleaseStatement() { return statement.release(); }
private:
    BoundCompileUnit& boundCompileUnit;
    ContainerScope* containerScope;
    BoundClass* boundClass;
    BoundFunction* function;
    BoundCompoundStatement* compoundStatement;
    std::unique_ptr<BoundStatement> statement;
    bool doNotInstantiate;
    bool instantiateRequested;
    bool insideCatch;
    TypeSymbol* switchConditionType;
    std::unordered_map<IntegralValue, CaseStatementNode*, IntegralValueHash>* caseValueMap;
    std::vector<std::pair<GotoCaseStatementNode*, IntegralValue>>* gotoCaseStatements;
    std::vector<GotoDefaultStatementNode*>* gotoDefaultStatements;
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_STATEMENT_BINDER_VISITOR_INCLUDED
