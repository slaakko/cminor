// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/BoundStatement.hpp>

namespace cminor { namespace binder {

void BoundCompoundStatement::AddStatement(std::unique_ptr<BoundStatement>&& statement)
{
    statements.push_back(std::move(statement));
}

BoundConstructionStatement::BoundConstructionStatement(std::unique_ptr<BoundFunctionCall>&& constructorCall_): constructorCall(std::move(constructorCall_))
{
}

BoundAssignmentStatement::BoundAssignmentStatement(std::unique_ptr<BoundExpression>&& targetExpr_, std::unique_ptr<BoundExpression>&& sourceExpr_) : 
    targetExpr(std::move(targetExpr_)), sourceExpr(std::move(sourceExpr_))
{
}

} } // namespace cminor::binder
