// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Statement.hpp>
#include <cminor/ast/Identifier.hpp>
#include <cminor/ast/Visitor.hpp>

namespace cminor { namespace ast {

LabelNode::LabelNode(const Span& span_) : Node(span_)
{
}

LabelNode::LabelNode(const Span& span_, const std::string& label_) : Node(span_), label(label_)
{
}

Node* LabelNode::Clone(CloneContext& cloneContext) const
{
    return new LabelNode(GetSpan(), label);
}

StatementNode::StatementNode(const Span& span_) : Node(span_)
{
}

void StatementNode::SetLabelNode(LabelNode* labelNode_)
{
    labelNode.reset(labelNode_);
    if (labelNode)
    {
        labelNode->SetParent(this);
    }
}

void StatementNode::CloneLabelTo(StatementNode* clone, CloneContext& cloneContext) const
{
    if (labelNode)
    {
        clone->SetLabelNode(static_cast<LabelNode*>(labelNode->Clone(cloneContext)));
    }
}

ConstructionStatementNode::ConstructionStatementNode(const Span& span_) : StatementNode(span_)
{
}

ConstructionStatementNode::ConstructionStatementNode(const Span& span_, Node* typeExpr_, IdentifierNode* id_) : StatementNode(span_), typeExpr(typeExpr_), id(id_)
{
    typeExpr->SetParent(this);
    id->SetParent(this);
}

void ConstructionStatementNode::AddArgument(Node* argument)
{
    argument->SetParent(this);
    arguments.Add(argument);
}

Node* ConstructionStatementNode::Clone(CloneContext& cloneContext) const
{
    ConstructionStatementNode* clone = new ConstructionStatementNode(GetSpan(), typeExpr->Clone(cloneContext), static_cast<IdentifierNode*>(id->Clone(cloneContext)));
    int n = arguments.Count();
    for (int i = 0; i < n; ++i)
    {
        Node* argument = arguments[i];
        clone->AddArgument(argument->Clone(cloneContext));
    }
    CloneLabelTo(clone, cloneContext);
    return clone;
}

void ConstructionStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

AssignmentStatementNode::AssignmentStatementNode(const Span& span_) : StatementNode(span_)
{
}

AssignmentStatementNode::AssignmentStatementNode(const Span& span_, Node* targetExpr_, Node* sourceExpr_) : StatementNode(span_), targetExpr(targetExpr_), sourceExpr(sourceExpr_)
{
    targetExpr->SetParent(this);
    sourceExpr->SetParent(this);
}

Node* AssignmentStatementNode::Clone(CloneContext& cloneContext) const
{
    AssignmentStatementNode* clone = new AssignmentStatementNode(GetSpan(), targetExpr->Clone(cloneContext), sourceExpr->Clone(cloneContext));
    CloneLabelTo(clone, cloneContext);
    return clone;
}

void AssignmentStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

CompoundStatementNode::CompoundStatementNode(const Span& span_) : StatementNode(span_)
{
}

void CompoundStatementNode::AddStatement(StatementNode* statement)
{
    statement->SetParent(this);
    statements.Add(statement);
}

Node* CompoundStatementNode::Clone(CloneContext& cloneContext) const
{
    CompoundStatementNode* clone = new CompoundStatementNode(GetSpan());
    int n = statements.Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statement = statements[i];
        clone->AddStatement(static_cast<StatementNode*>(statement->Clone(cloneContext)));
    }
    CloneLabelTo(clone, cloneContext);
    clone->beginBraceSpan = beginBraceSpan;
    clone->endBraceSpan = endBraceSpan;
    return clone;
}

void CompoundStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ReturnStatementNode::ReturnStatementNode(const Span& span_) : StatementNode(span_)
{
}

ReturnStatementNode::ReturnStatementNode(const Span& span_, Node* expression_) : StatementNode(span_), expression(expression_)
{
    if (expression)
    {
        expression->SetParent(this);
    }
}

Node* ReturnStatementNode::Clone(CloneContext& cloneContext) const
{
    Node* clonedExpression = nullptr;
    if (expression)
    {
        clonedExpression = expression->Clone(cloneContext);
    }
    ReturnStatementNode* clone = new ReturnStatementNode(GetSpan(), clonedExpression);
    CloneLabelTo(clone, cloneContext);
    return clone;
}

void ReturnStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

IfStatementNode::IfStatementNode(const Span& span_) : StatementNode(span_)
{
}

IfStatementNode::IfStatementNode(const Span& span_, Node* condition_, StatementNode* thenS_, StatementNode* elseS_) : StatementNode(span_), condition(condition_), thenS(thenS_), elseS(elseS_)
{
    condition->SetParent(this);
    thenS->SetParent(this);
    if (elseS)
    {
        elseS->SetParent(this);
    }
}

Node* IfStatementNode::Clone(CloneContext& cloneContext) const
{
    StatementNode* clonedElseS = nullptr;
    if (elseS)
    {
        clonedElseS = static_cast<StatementNode*>(elseS->Clone(cloneContext));
    }
    IfStatementNode* clone = new IfStatementNode(GetSpan(), condition->Clone(cloneContext), static_cast<StatementNode*>(thenS->Clone(cloneContext)), clonedElseS);
    CloneLabelTo(clone, cloneContext);
    return clone;
}

void IfStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

WhileStatementNode::WhileStatementNode(const Span& span_) : StatementNode(span_)
{
}

WhileStatementNode::WhileStatementNode(const Span& span_, Node* condition_, StatementNode* statement_) : StatementNode(span_), condition(condition_), statement(statement_)
{
    condition->SetParent(this);
    statement->SetParent(this);
}

Node* WhileStatementNode::Clone(CloneContext& cloneContext) const
{
    WhileStatementNode* clone = new WhileStatementNode(GetSpan(), condition->Clone(cloneContext), static_cast<StatementNode*>(statement->Clone(cloneContext))); 
    CloneLabelTo(clone, cloneContext);
    return clone;
}

void WhileStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

DoStatementNode::DoStatementNode(const Span& span_) : StatementNode(span_)
{
}

DoStatementNode::DoStatementNode(const Span& span_, StatementNode* statement_, Node* condition_) : StatementNode(span_), statement(statement_), condition(condition_)
{
    statement->SetParent(this);
    condition->SetParent(this);
}

Node* DoStatementNode::Clone(CloneContext& cloneContext) const
{
    DoStatementNode* clone = new DoStatementNode(GetSpan(), static_cast<StatementNode*>(statement->Clone(cloneContext)), condition->Clone(cloneContext));
    CloneLabelTo(clone, cloneContext);
    return clone;
}

void DoStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ForStatementNode::ForStatementNode(const Span& span_) : StatementNode(span_)
{
}

ForStatementNode::ForStatementNode(const Span& span_, StatementNode* initS_, Node* condition_, StatementNode* loopS_, StatementNode* actionS_) : 
    StatementNode(span_), initS(initS_), condition(condition_), loopS(loopS_), actionS(actionS_)
{
    initS->SetParent(this);
    if (condition)
    {
        condition->SetParent(this);
    }
    loopS->SetParent(this);
    actionS->SetParent(this);
}

Node* ForStatementNode::Clone(CloneContext& cloneContext) const
{
    Node* clonedCondition = nullptr;
    if (condition)
    {
        clonedCondition = condition->Clone(cloneContext);
    }
    ForStatementNode* clone = new ForStatementNode(GetSpan(), static_cast<StatementNode*>(initS->Clone(cloneContext)), clonedCondition, static_cast<StatementNode*>(loopS->Clone(cloneContext)),
        static_cast<StatementNode*>(actionS->Clone(cloneContext)));
    CloneLabelTo(clone, cloneContext);
    return clone;
}

void ForStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ExpressionStatementNode::ExpressionStatementNode(const Span& span_) : StatementNode(span_)
{
}

ExpressionStatementNode::ExpressionStatementNode(const Span& span_, Node* expression_) : StatementNode(span_), expression(expression_)
{
    expression->SetParent(this);
}

Node* ExpressionStatementNode::Clone(CloneContext& cloneContext) const
{
    ExpressionStatementNode* clone = new ExpressionStatementNode(GetSpan(), expression->Clone(cloneContext));
    CloneLabelTo(clone, cloneContext);
    return clone;
}

void ExpressionStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

EmptyStatementNode::EmptyStatementNode(const Span& span_) : StatementNode(span_)
{
}

Node* EmptyStatementNode::Clone(CloneContext& cloneContext) const
{
    EmptyStatementNode* clone = new EmptyStatementNode(GetSpan());
    CloneLabelTo(clone, cloneContext);
    return clone;
}

void EmptyStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

IncrementStatementNode::IncrementStatementNode(const Span& span_) : StatementNode(span_)
{
}

IncrementStatementNode::IncrementStatementNode(const Span& span_, Node* expression_) : StatementNode(span_), expression(expression_)
{
    expression->SetParent(this);
}

Node* IncrementStatementNode::Clone(CloneContext& cloneContext) const
{
    IncrementStatementNode* clone = new IncrementStatementNode(GetSpan(), expression->Clone(cloneContext));
    CloneLabelTo(clone, cloneContext);
    return clone;
}

void IncrementStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

DecrementStatementNode::DecrementStatementNode(const Span& span_) : StatementNode(span_)
{
}

DecrementStatementNode::DecrementStatementNode(const Span& span_, Node* expression_) : StatementNode(span_), expression(expression_)
{
}

Node* DecrementStatementNode::Clone(CloneContext& cloneContext) const
{
    DecrementStatementNode* clone = new DecrementStatementNode(GetSpan(), expression->Clone(cloneContext));
    CloneLabelTo(clone, cloneContext);
    return clone;
}

void DecrementStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::ast
