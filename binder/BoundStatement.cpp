// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/BoundStatement.hpp>
#include <cminor/binder/BoundNodeVisitor.hpp>

namespace cminor { namespace binder {

BoundStatement::BoundStatement(Assembly& assembly_) : BoundNode(assembly_), parent(nullptr)
{
}

BoundCompoundStatement* BoundStatement::Block() const
{
    if (const BoundCompoundStatement* compound = dynamic_cast<const BoundCompoundStatement*>(this))
    {
        return const_cast<BoundCompoundStatement*>(compound);
    }
    else
    {
        return parent->Block();
    }
}

BoundCompoundStatement::BoundCompoundStatement(Assembly& assembly_) : BoundStatement(assembly_)
{
}

void BoundCompoundStatement::InsertFront(std::unique_ptr<BoundStatement>&& statement)
{
    statement->SetParent(this);
    statements.insert(statements.begin(), std::move(statement));
}

void BoundCompoundStatement::AddStatement(std::unique_ptr<BoundStatement>&& statement)
{
    statement->SetParent(this);
    statements.push_back(std::move(statement));
}

void BoundCompoundStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundReturnStatement::BoundReturnStatement(Assembly& assembly_, std::unique_ptr<BoundFunctionCall>&& returnFunctionCall_) : BoundStatement(assembly_), returnFunctionCall(std::move(returnFunctionCall_))
{
}

void BoundReturnStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundIfStatement::BoundIfStatement(Assembly& assembly_, std::unique_ptr<BoundExpression>&& condition_, std::unique_ptr<BoundStatement>&& thenS_, std::unique_ptr<BoundStatement>&& elseS_) :
    BoundStatement(assembly_), condition(std::move(condition_)), thenS(std::move(thenS_)), elseS(std::move(elseS_))
{
    thenS->SetParent(this);
    if (elseS)
    {
        elseS->SetParent(this);
    }
}

void BoundIfStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundWhileStatement::BoundWhileStatement(Assembly& assembly_, std::unique_ptr<BoundExpression>&& condition_, std::unique_ptr<BoundStatement>&& statement_ ): 
    BoundStatement(assembly_), condition(std::move(condition_)), statement(std::move(statement_))
{
    statement->SetParent(this);
}

void BoundWhileStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundDoStatement::BoundDoStatement(Assembly& assembly_, std::unique_ptr<BoundStatement>&& statement_, std::unique_ptr<BoundExpression>&& condition_) : 
    BoundStatement(assembly_), statement(std::move(statement_)), condition(std::move(condition_))
{
    statement->SetParent(this);
}

void BoundDoStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundForStatement::BoundForStatement(Assembly& assembly_, std::unique_ptr<BoundStatement>&& initS_, std::unique_ptr<BoundExpression>&& condition_, std::unique_ptr<BoundStatement>&& loopS_,
    std::unique_ptr<BoundStatement>&& actionS_) : BoundStatement(assembly_), initS(std::move(initS_)), condition(std::move(condition_)), loopS(std::move(loopS_)), actionS(std::move(actionS_))
{
    initS->SetParent(this);
    loopS->SetParent(this);
    actionS->SetParent(this);
}

void BoundForStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundBreakStatement::BoundBreakStatement(Assembly& assembly_) : BoundStatement(assembly_)
{
}

void BoundBreakStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundContinueStatement::BoundContinueStatement(Assembly& assembly_) : BoundStatement(assembly_)
{
}

void BoundContinueStatement::Accept(BoundNodeVisitor& visitor)
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

BoundExpressionStatement::BoundExpressionStatement(Assembly& assembly_, std::unique_ptr<BoundExpression>&& expression_) : BoundStatement(assembly_), expression(std::move(expression_))
{
}

void BoundExpressionStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundEmptyStatement::BoundEmptyStatement(Assembly& assembly_) : BoundStatement(assembly_)
{
}

void BoundEmptyStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundThrowStatement::BoundThrowStatement(Assembly& assembly_) : BoundStatement(assembly_)
{
}

BoundThrowStatement::BoundThrowStatement(Assembly& assembly_, std::unique_ptr<BoundExpression>&& expression_) : BoundStatement(assembly_), expression(std::move(expression_))
{
}

void BoundThrowStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundStaticInitStatement::BoundStaticInitStatement(Assembly& assembly_, Constant classNameConstant_) : BoundStatement(assembly_), classNameConstant(classNameConstant_)
{
}

void BoundStaticInitStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundDoneStaticInitStatement::BoundDoneStaticInitStatement(Assembly& assembly_, Constant classNameConstant_) : BoundStatement(assembly_), classNameConstant(classNameConstant_)
{
}

void BoundDoneStaticInitStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::binder
