// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/BoundStatement.hpp>
#include <cminor/binder/BoundNodeVisitor.hpp>

namespace cminor { namespace binder {

BoundStatement::BoundStatement(Assembly& assembly_) : BoundNode(assembly_)
{
}

BoundCompoundStatement::BoundCompoundStatement(Assembly& assembly_) : BoundStatement(assembly_)
{
}

void BoundCompoundStatement::AddStatement(std::unique_ptr<BoundStatement>&& statement)
{
    statements.push_back(std::move(statement));
}

void BoundCompoundStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundConstructionStatement::BoundConstructionStatement(Assembly& assembly_, std::unique_ptr<BoundFunctionCall>&& constructorCall_): BoundStatement(assembly_), constructorCall(std::move(constructorCall_))
{
}

void BoundConstructionStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundAssignmentStatement::BoundAssignmentStatement(Assembly& assembly_, std::unique_ptr<BoundFunctionCall>&& assignmentCall_) : BoundStatement(assembly_), assignmentCall(std::move(assignmentCall_))
{
}

void BoundAssignmentStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::binder
