// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_BOUND_STATEMENT_INCLUDED
#define CMINOR_BINDER_BOUND_STATEMENT_INCLUDED
#include <cminor/binder/BoundExpression.hpp>
#include <cminor/symbols/VariableSymbol.hpp>

namespace cminor { namespace binder {

using namespace cminor::symbols;

class BoundStatement : public BoundNode
{
public:
    BoundStatement(Assembly& assembly_);
};

class BoundCompoundStatement : public BoundStatement
{
public:
    BoundCompoundStatement(Assembly& assembly_);
    const std::vector<std::unique_ptr<BoundStatement>>& Statements() const { return statements; }
    void AddStatement(std::unique_ptr<BoundStatement>&& statement);
    void Accept(BoundNodeVisitor& visitor) override;
private:
    std::vector<std::unique_ptr<BoundStatement>> statements;
};

class BoundConstructionStatement : public BoundStatement
{
public:
    BoundConstructionStatement(Assembly& assembly_, std::unique_ptr<BoundFunctionCall>&& constructorCall_);
    BoundFunctionCall* ConstructorCall() const { return constructorCall.get(); }
    void Accept(BoundNodeVisitor& visitor) override;
private:
    std::unique_ptr<BoundFunctionCall> constructorCall;
};

class BoundAssignmentStatement : public BoundStatement
{
public:
    BoundAssignmentStatement(Assembly& assembhly_, std::unique_ptr<BoundFunctionCall>&& assignmentCall_);
    BoundFunctionCall* AssignmentCall() const { return assignmentCall.get(); }
    void Accept(BoundNodeVisitor& visitor) override;
private:
    std::unique_ptr<BoundFunctionCall> assignmentCall;
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_BOUND_STATEMENT_INCLUDED
