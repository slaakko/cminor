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

class BoundCompoundStatement;

class BoundStatement : public BoundNode
{
public:
    BoundStatement(Assembly& assembly_);
    void SetParent(BoundStatement* parent_) { parent = parent_; }
    BoundStatement* Parent() const { return parent; }
    BoundCompoundStatement* Block() const;
private:
    BoundStatement* parent;
};

class BoundCompoundStatement : public BoundStatement
{
public:
    BoundCompoundStatement(Assembly& assembly_);
    const std::vector<std::unique_ptr<BoundStatement>>& Statements() const { return statements; }
    void InsertFront(std::unique_ptr<BoundStatement>&& statement);
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

class BoundCaseStatement;
class BoundDefaultStatement;

class BoundSwitchStatement : public BoundStatement
{
public:
    BoundSwitchStatement(Assembly& assembly_, std::unique_ptr<BoundExpression>&& condition_);
    BoundExpression* Condition() const { return condition.get(); }
    void Accept(BoundNodeVisitor& visitor) override;
    void AddCaseStatement(std::unique_ptr<BoundCaseStatement>&& caseStatement);
    const std::vector<std::unique_ptr<BoundCaseStatement>>& CaseStatements() const { return caseStatements; }
    void SetDefaultStatement(std::unique_ptr<BoundDefaultStatement>&& defaultStatement_);
    BoundDefaultStatement* DefaultStatement() const { return defaultStatement.get(); }
private:
    std::unique_ptr<BoundExpression> condition;
    std::vector<std::unique_ptr<BoundCaseStatement>> caseStatements;
    std::unique_ptr<BoundDefaultStatement> defaultStatement;
};

class BoundCaseStatement : public BoundStatement
{
public:
    BoundCaseStatement(Assembly& assembly_);
    void AddCaseValue(IntegralValue caseValue);
    void AddStatement(std::unique_ptr<BoundStatement>&& statement);
    const std::vector<IntegralValue>& CaseValues() const { return caseValues; }
    BoundCompoundStatement* CompoundStatement() const { return const_cast<BoundCompoundStatement*>(&compoundStatement); }
    void Accept(BoundNodeVisitor& visitor) override;
private:
    std::vector<IntegralValue> caseValues;
    BoundCompoundStatement compoundStatement;
};

class BoundDefaultStatement : public BoundStatement
{
public:
    BoundDefaultStatement(Assembly& assembly_);
    void AddStatement(std::unique_ptr<BoundStatement>&& statement);
    BoundCompoundStatement* CompoundStatement() const { return const_cast<BoundCompoundStatement*>(&compoundStatement); }
    void Accept(BoundNodeVisitor& visitor) override;
private:
    BoundCompoundStatement compoundStatement;
};

class BoundGotoCaseStatement : public BoundStatement
{
public:
    BoundGotoCaseStatement(Assembly& assembly_, IntegralValue caseValue_);
    void Accept(BoundNodeVisitor& visitor) override;
    IntegralValue CaseValue() const { return caseValue; }
private:
    IntegralValue caseValue;
};

class BoundGotoDefaultStatement : public BoundStatement
{
public:
    BoundGotoDefaultStatement(Assembly& assembly_);
    void Accept(BoundNodeVisitor& visitor) override;
};

class BoundBreakStatement : public BoundStatement
{
public:
    BoundBreakStatement(Assembly& assembly_);
    void Accept(BoundNodeVisitor& visitor) override;
};

class BoundContinueStatement : public BoundStatement
{
public:
    BoundContinueStatement(Assembly& assembly_);
    void Accept(BoundNodeVisitor& visitor) override;
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

class BoundThrowStatement : public BoundStatement
{
public:
    BoundThrowStatement(Assembly& assembly_);
    BoundThrowStatement(Assembly& assembly_, std::unique_ptr<BoundExpression>&& expression_);
    BoundExpression* Expression() const { return expression.get(); }
    void Accept(BoundNodeVisitor& visitor) override;
private:
    std::unique_ptr<BoundExpression> expression;
};

class BoundStaticInitStatement : public BoundStatement
{
public:
    BoundStaticInitStatement(Assembly& assembly_, Constant classNameConstant_);
    void Accept(BoundNodeVisitor& visitor) override;
    Constant ClassNameConstant() const { return classNameConstant; }
private:
    Constant classNameConstant;
};

class BoundDoneStaticInitStatement : public BoundStatement
{
public:
    BoundDoneStaticInitStatement(Assembly& assembly_, Constant classNameConstant_);
    void Accept(BoundNodeVisitor& visitor) override;
    Constant ClassNameConstant() const { return classNameConstant; }
private:
    Constant classNameConstant;
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_BOUND_STATEMENT_INCLUDED
