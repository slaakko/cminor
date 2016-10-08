// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_STATEMENT_BINDER_VISITOR_INCLUDED
#define CMINOR_BINDER_STATEMENT_BINDER_VISITOR_INCLUDED
#include <cminor/ast/Visitor.hpp>
#include <cminor/symbols/Symbol.hpp>

namespace cminor { namespace binder {

using namespace cminor::ast;
using namespace cminor::symbols;

class BoundCompileUnit;
class BoundFunction;
class BoundCompoundStatement;

class StatementBinderVisitor : public Visitor
{
public:
    StatementBinderVisitor(BoundCompileUnit& boundCompileUnit_);
    void Visit(CompileUnitNode& compileUnitNode) override;
    void Visit(NamespaceNode& namespaceNode) override;
    void Visit(FunctionNode& functionNode) override;
    void Visit(CompoundStatementNode& compoundStatementNode) override;
    void Visit(ConstructionStatementNode& constructionStatementNode) override;
    void Visit(AssignmentStatementNode& assignmentStatementNode) override;
private:
    BoundCompileUnit& boundCompileUnit;
    ContainerScope* containerScope;
    BoundFunction* function;
    BoundCompoundStatement* compoundStatement;
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_STATEMENT_BINDER_VISITOR_INCLUDED
