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

class BoundReturnStatement : public BoundStatement
{
public:
    BoundReturnStatement(Assembly& assembly_, std::unique_ptr<BoundFunctionCall>&& returnFunctionCall_);
    BoundFunctionCall* ReturnFunctionCall() const { return returnFunctionCall.get(); }
    void Accept(BoundNodeVisitor& visitor) override;
private:
    std::unique_ptr<BoundFunctionCall> returnFunctionCall;
};

class BoundIfStatement : public BoundStatement
{
public:
    BoundIfStatement(Assembly& assembly_, std::unique_ptr<BoundExpression>&& condition_, std::unique_ptr<BoundStatement>&& thenS_, std::unique_ptr<BoundStatement>&& elseS_);
    BoundExpression* Condition() const { return condition.get(); }
    BoundStatement* ThenS() const { return thenS.get(); }
    BoundStatement* ElseS() const { return elseS.get(); }
    void Accept(BoundNodeVisitor& visitor) override;
private:
    std::unique_ptr<BoundExpression> condition;
    std::unique_ptr<BoundStatement> thenS;
    std::unique_ptr<BoundStatement> elseS;
};

class BoundWhileStatement : public BoundStatement
{
public:
    BoundWhileStatement(Assembly& assembly_, std::unique_ptr<BoundExpression>&& condition_, std::unique_ptr<BoundStatement>&& statement_);
    BoundExpression* Condition() const { return condition.get(); }
    BoundStatement* Statement() const { return statement.get(); }
    void Accept(BoundNodeVisitor& visitor) override;
private:
    std::unique_ptr<BoundExpression> condition;
    std::unique_ptr<BoundStatement> statement;
};

class BoundDoStatement : public BoundStatement
{
public:
    BoundDoStatement(Assembly& assembly_, std::unique_ptr<BoundStatement>&& statement_, std::unique_ptr<BoundExpression>&& condition_);
    BoundStatement* Statement() const { return statement.get(); }
    BoundExpression* Condition() const { return condition.get(); }
    void Accept(BoundNodeVisitor& visitor) override;
private:
    std::unique_ptr<BoundStatement> statement;
    std::unique_ptr<BoundExpression> condition;
};

class BoundForStatement : public BoundStatement
{
public:
    BoundForStatement(Assembly& assembly_, std::unique_ptr<BoundStatement>&& initS_, std::unique_ptr<BoundExpression>&& condition_, std::unique_ptr<BoundStatement>&& loopS_,
        std::unique_ptr<BoundStatement>&& actionS_);
    BoundStatement* InitS() const { return initS.get(); }
    BoundExpression* Condition() const { return condition.get(); }
    BoundStatement* LoopS() const { return loopS.get(); }
    BoundStatement* ActionS() const { return actionS.get(); }
    void Accept(BoundNodeVisitor& visitor) override;
private:
    std::unique_ptr<BoundStatement> initS;
    std::unique_ptr<BoundExpression> condition;
    std::unique_ptr<BoundStatement> loopS;
    std::unique_ptr<BoundStatement> actionS;
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

class BoundExpressionStatement : public BoundStatement
{
public:
    BoundExpressionStatement(Assembly& assembly_, std::unique_ptr<BoundExpression>&& expression_);
    BoundExpression* Expression() const { return expression.get(); }
    void Accept(BoundNodeVisitor& visitor) override;
private:
    std::unique_ptr<BoundExpression> expression;
};

class BoundEmptyStatement : public BoundStatement
{
public:
    BoundEmptyStatement(Assembly& assembly_);
    void Accept(BoundNodeVisitor& visitor) override;
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_BOUND_STATEMENT_INCLUDED
