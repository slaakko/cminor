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
};

class BoundCompoundStatement : public BoundStatement
{
public:
    void AddStatement(std::unique_ptr<BoundStatement>&& statement);
private:
    std::vector<std::unique_ptr<BoundStatement>> statements;
};

class BoundConstructionStatement : public BoundStatement
{
public:
    BoundConstructionStatement(std::unique_ptr<BoundFunctionCall>&& constructorCall_);
private:
    std::unique_ptr<BoundFunctionCall> constructorCall;
};

class BoundAssignmentStatement : public BoundStatement
{
public:
    BoundAssignmentStatement(std::unique_ptr<BoundExpression>&& targetExpr_, std::unique_ptr<BoundExpression>&& sourceExpr_);
    BoundExpression* Target() const { return targetExpr.get(); }
    BoundExpression* Source() const { return sourceExpr.get(); }
private:
    std::unique_ptr<BoundExpression> targetExpr;
    std::unique_ptr<BoundExpression> sourceExpr;
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_BOUND_STATEMENT_INCLUDED
