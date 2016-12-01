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

void LabelNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.AsMachineWriter().Put(label);
}

void LabelNode::Read(AstReader& reader)
{
    Node::Read(reader);
    label = reader.GetUtf8String();
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

void StatementNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    bool hasLabel = labelNode != nullptr;
    writer.AsMachineWriter().Put(hasLabel);
    if (hasLabel)
    {
        writer.Put(labelNode.get());
    }
}

void StatementNode::Read(AstReader& reader)
{
    Node::Read(reader);
    bool hasLabel = reader.GetBool();
    if (hasLabel)
    {
        Node* node = reader.GetNode();
        LabelNode* lblNode = dynamic_cast<LabelNode*>(node);
        Assert(lblNode, "label node expected");
        labelNode.reset(lblNode);
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

void ConstructionStatementNode::Write(AstWriter& writer)
{
    StatementNode::Write(writer);
    writer.Put(typeExpr.get());
    writer.Put(id.get());
    arguments.Write(writer);
}

void ConstructionStatementNode::Read(AstReader& reader)
{
    StatementNode::Read(reader);
    typeExpr.reset(reader.GetNode());
    typeExpr->SetParent(this);
    id.reset(reader.GetIdentifierNode());
    id->SetParent(this);
    arguments.Read(reader);
    arguments.SetParent(this);
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

void AssignmentStatementNode::Write(AstWriter& writer)
{
    StatementNode::Write(writer);
    writer.Put(targetExpr.get());
    writer.Put(sourceExpr.get());
}

void AssignmentStatementNode::Read(AstReader& reader)
{
    StatementNode::Read(reader);
    targetExpr.reset(reader.GetNode());
    targetExpr->SetParent(this);
    sourceExpr.reset(reader.GetNode());
    sourceExpr->SetParent(this);
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

void CompoundStatementNode::Write(AstWriter& writer)
{
    StatementNode::Write(writer);
    statements.Write(writer);
    writer.AsMachineWriter().Put(beginBraceSpan);
    writer.AsMachineWriter().Put(endBraceSpan);
}

void CompoundStatementNode::Read(AstReader& reader)
{
    StatementNode::Read(reader);
    statements.Read(reader);
    statements.SetParent(this);
    beginBraceSpan = reader.GetSpan();
    endBraceSpan = reader.GetSpan();
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

void ReturnStatementNode::Write(AstWriter& writer)
{
    StatementNode::Write(writer);
    bool hasExpression = expression != nullptr;
    writer.AsMachineWriter().Put(hasExpression);
    if (hasExpression)
    {
        writer.Put(expression.get());
    }
}

void ReturnStatementNode::Read(AstReader& reader)
{
    StatementNode::Read(reader);
    bool hasExpression = reader.GetBool();
    if (hasExpression)
    {
        expression.reset(reader.GetNode());
        expression->SetParent(this);
    }
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

void IfStatementNode::Write(AstWriter& writer)
{
    StatementNode::Write(writer);
    writer.Put(condition.get());
    writer.Put(thenS.get());
    bool hasElseS = elseS != nullptr;
    writer.AsMachineWriter().Put(hasElseS);
    if (hasElseS)
    {
        writer.Put(elseS.get());
    }
}

void IfStatementNode::Read(AstReader& reader)
{
    StatementNode::Read(reader);
    condition.reset(reader.GetNode());
    condition->SetParent(this);
    thenS.reset(reader.GetStatementNode());
    thenS->SetParent(this);
    bool hasElseS = reader.GetBool();
    if (hasElseS)
    {
        elseS.reset(reader.GetStatementNode());
        elseS->SetParent(this);
    }
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

void WhileStatementNode::Write(AstWriter& writer)
{
    StatementNode::Write(writer);
    writer.Put(condition.get());
    writer.Put(statement.get());
}

void WhileStatementNode::Read(AstReader& reader)
{
    StatementNode::Read(reader);
    condition.reset(reader.GetNode());
    condition->SetParent(this);
    statement.reset(reader.GetStatementNode());
    statement->SetParent(this);
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

void DoStatementNode::Write(AstWriter& writer)
{
    StatementNode::Write(writer);
    writer.Put(statement.get());
    writer.Put(condition.get());
}

void DoStatementNode::Read(AstReader& reader)
{
    StatementNode::Read(reader);
    statement.reset(reader.GetStatementNode());
    statement->SetParent(this);
    condition.reset(reader.GetNode());
    condition->SetParent(this);
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

void ForStatementNode::Write(AstWriter& writer)
{
    StatementNode::Write(writer);
    writer.Put(initS.get());
    bool hasCondition = condition != nullptr;
    writer.AsMachineWriter().Put(hasCondition);
    if (hasCondition)
    {
        writer.Put(condition.get());
    }
    writer.Put(loopS.get());
    writer.Put(actionS.get());
}

void ForStatementNode::Read(AstReader& reader)
{
    StatementNode::Read(reader);
    initS.reset(reader.GetStatementNode());
    initS->SetParent(this);
    bool hasCondition = reader.GetBool();
    if (hasCondition)
    {
        condition.reset(reader.GetNode());
        condition->SetParent(this);
    }
    loopS.reset(reader.GetStatementNode());
    loopS->SetParent(this);
    actionS.reset(reader.GetStatementNode());
    actionS->SetParent(this);
}

void ForStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

BreakStatementNode::BreakStatementNode(const Span& span_) : StatementNode(span_)
{
}

Node* BreakStatementNode::Clone(CloneContext& cloneContext) const
{
    return new BreakStatementNode(GetSpan());
}

void BreakStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ContinueStatementNode::ContinueStatementNode(const Span& span_) : StatementNode(span_)
{
}

Node* ContinueStatementNode::Clone(CloneContext& cloneContext) const
{
    return new ContinueStatementNode(GetSpan());
}

void ContinueStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

GotoStatementNode::GotoStatementNode(const Span& span_) : StatementNode(span_)
{
}

GotoStatementNode::GotoStatementNode(const Span& span_, const std::string& target_) : StatementNode(span_), target(target_)
{
}

Node* GotoStatementNode::Clone(CloneContext& cloneContext) const
{
    return new GotoStatementNode(GetSpan(), target);
}

void GotoStatementNode::Write(AstWriter& writer)
{
    StatementNode::Write(writer);
    writer.AsMachineWriter().Put(target);
}

void GotoStatementNode::Read(AstReader& reader)
{
    StatementNode::Read(reader);
    target = reader.GetUtf8String();
}

void GotoStatementNode::Accept(Visitor& visitor)
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

void ExpressionStatementNode::Write(AstWriter& writer)
{
    StatementNode::Write(writer);
    writer.Put(expression.get());
}

void ExpressionStatementNode::Read(AstReader& reader)
{
    StatementNode::Read(reader);
    expression.reset(reader.GetNode());
    expression->SetParent(this);
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

void IncrementStatementNode::Write(AstWriter& writer)
{
    StatementNode::Write(writer);
    writer.Put(expression.get());
}

void IncrementStatementNode::Read(AstReader& reader)
{
    StatementNode::Read(reader);
    expression.reset(reader.GetNode());
    expression->SetParent(this);
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

void DecrementStatementNode::Write(AstWriter& writer)
{
    StatementNode::Write(writer);
    writer.Put(expression.get());
}

void DecrementStatementNode::Read(AstReader& reader)
{
    StatementNode::Read(reader);
    expression.reset(reader.GetNode());
    expression->SetParent(this);
}

void DecrementStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ForEachStatementNode::ForEachStatementNode(const Span& span_) : StatementNode(span_)
{
}

ForEachStatementNode::ForEachStatementNode(const Span& span_, Node* typeExpr_, IdentifierNode* id_, Node* container_, CompoundStatementNode* action_) : 
    StatementNode(span_), typeExpr(typeExpr_), id(id_), container(container_), action(action_)
{
    typeExpr->SetParent(this);
    id->SetParent(this);
    container->SetParent(this);
    action->SetParent(this);
}

Node* ForEachStatementNode::Clone(CloneContext& cloneContext) const
{
    return new ForEachStatementNode(GetSpan(), typeExpr->Clone(cloneContext), static_cast<IdentifierNode*>(id->Clone(cloneContext)), container->Clone(cloneContext), 
        static_cast<CompoundStatementNode*>(action->Clone(cloneContext)));
}

void ForEachStatementNode::Write(AstWriter& writer)
{
    StatementNode::Write(writer);
    writer.Put(typeExpr.get());
    writer.Put(id.get());
    writer.Put(container.get());
    writer.Put(action.get());
}

void ForEachStatementNode::Read(AstReader& reader)
{
    StatementNode::Read(reader);
    typeExpr.reset(reader.GetNode());
    typeExpr->SetParent(this);
    id.reset(reader.GetIdentifierNode());
    id->SetParent(this);
    container.reset(reader.GetNode());
    container->SetParent(this);
    action.reset(reader.GetCompoundStatementNode());
    action->SetParent(this);
}

void ForEachStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

SwitchStatementNode::SwitchStatementNode(const Span& span_) : StatementNode(span_)
{
}

SwitchStatementNode::SwitchStatementNode(const Span& span_, Node* condition_) : StatementNode(span_), condition(condition_)
{
    condition->SetParent(this);
}

Node* SwitchStatementNode::Clone(CloneContext& cloneContext) const
{
    SwitchStatementNode* clone = new SwitchStatementNode(GetSpan(), condition->Clone(cloneContext));
    int n = cases.Count();
    for (int i = 0; i < n; ++i)
    {
        CaseStatementNode* caseS = cases[i];
        clone->AddCase(static_cast<CaseStatementNode*>(caseS->Clone(cloneContext)));
    }
    if (defaultS)
    {
        clone->SetDefault(static_cast<DefaultStatementNode*>(defaultS->Clone(cloneContext)));
    }
    return clone;
}

void SwitchStatementNode::Write(AstWriter& writer)
{
    StatementNode::Write(writer);
    writer.Put(condition.get());
    cases.Write(writer);
    if (defaultS)
    {
        writer.AsMachineWriter().Put(true);
        writer.Put(defaultS.get());
    }
    else
    {
        writer.AsMachineWriter().Put(false);
    }
}

void SwitchStatementNode::Read(AstReader& reader)
{
    StatementNode::Read(reader);
    condition.reset(reader.GetNode());
    condition->SetParent(this);
    cases.Read(reader);
    cases.SetParent(this);
    bool hasDefault = reader.GetBool();
    if (hasDefault)
    {
        defaultS.reset(dynamic_cast<DefaultStatementNode*>(reader.GetNode()));
        Assert(defaultS, "default statement node expected");
        defaultS->SetParent(this);
    }
}

void SwitchStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void SwitchStatementNode::AddCase(CaseStatementNode* caseS)
{
    caseS->SetParent(this);
    cases.Add(caseS);
}

const NodeList<CaseStatementNode>& SwitchStatementNode::Cases() const
{
    return cases;
}

void SwitchStatementNode::SetDefault(DefaultStatementNode* defaultS_)
{
    if (defaultS)
    {
        throw Exception("already has a default statement", defaultS->GetSpan(), defaultS_->GetSpan());
    }
    defaultS.reset(defaultS_);
    defaultS->SetParent(this);
}

DefaultStatementNode* SwitchStatementNode::Default() const
{ 
    return defaultS.get(); 
}

CaseStatementNode::CaseStatementNode(const Span& span_) : StatementNode(span_)
{
}


Node* CaseStatementNode::Clone(CloneContext& cloneContext) const
{
    CaseStatementNode* clone = new CaseStatementNode(GetSpan());
    int n = caseExprs.Count();
    for (int i = 0; i < n; ++i)
    {
        Node* caseExpr = caseExprs[i];
        clone->AddCaseExpr(caseExpr->Clone(cloneContext));
    }
    int m = statements.Count();
    for (int i = 0; i < m; ++i)
    {
        StatementNode* statement = statements[i];
        clone->AddStatement(static_cast<StatementNode*>(statement->Clone(cloneContext)));
    }
    return clone;
}

void CaseStatementNode::Write(AstWriter& writer)
{
    StatementNode::Write(writer);
    caseExprs.Write(writer);
    statements.Write(writer);
}

void CaseStatementNode::Read(AstReader& reader)
{
    StatementNode::Read(reader);
    caseExprs.Read(reader);
    caseExprs.SetParent(this);
    statements.Read(reader);
    statements.SetParent(this);
}

void CaseStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void CaseStatementNode::AddCaseExpr(Node* caseExpr)
{
    caseExpr->SetParent(this);
    caseExprs.Add(caseExpr);
}

void CaseStatementNode::AddStatement(StatementNode* statement)
{
    statement->SetParent(this);
    statements.Add(statement);
}

const NodeList<Node>& CaseStatementNode::CaseExprs() const
{ 
    return caseExprs; 
}

const NodeList<StatementNode>& CaseStatementNode::Statements() const
{ 
    return statements; 
}

DefaultStatementNode::DefaultStatementNode(const Span& span_) : StatementNode(span_)
{
}

Node* DefaultStatementNode::Clone(CloneContext& cloneContext) const
{
    DefaultStatementNode* clone = new DefaultStatementNode(GetSpan());
    int n = statements.Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statement = statements[i];
        clone->AddStatement(static_cast<StatementNode*>(statement->Clone(cloneContext)));
    }
    return clone;
}

void DefaultStatementNode::Write(AstWriter& writer)
{
    StatementNode::Write(writer);
    statements.Write(writer);
}

void DefaultStatementNode::Read(AstReader& reader)
{
    StatementNode::Read(reader);
    statements.Read(reader);
    statements.SetParent(this);
}

void DefaultStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void DefaultStatementNode::AddStatement(StatementNode* statement)
{
    statement->SetParent(this);
    statements.Add(statement);
}

const NodeList<StatementNode>& DefaultStatementNode::Statements() const
{ 
    return statements; 
}

GotoCaseStatementNode::GotoCaseStatementNode(const Span& span_) : StatementNode(span_)
{
}

GotoCaseStatementNode::GotoCaseStatementNode(const Span& span_, Node* caseExpr_) : StatementNode(span_), caseExpr(caseExpr_)
{
    caseExpr->SetParent(this);
}

Node* GotoCaseStatementNode::Clone(CloneContext& cloneContext) const
{
    GotoCaseStatementNode* clone = new GotoCaseStatementNode(GetSpan(), caseExpr->Clone(cloneContext));
    return clone;
}

void GotoCaseStatementNode::Write(AstWriter& writer)
{
    StatementNode::Write(writer);
    writer.Put(caseExpr.get());
}

void GotoCaseStatementNode::Read(AstReader& reader)
{
    StatementNode::Read(reader);
    caseExpr.reset(reader.GetNode());
    caseExpr->SetParent(this);
}

void GotoCaseStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

GotoDefaultStatementNode::GotoDefaultStatementNode(const Span& span_) : StatementNode(span_)
{
}

Node* GotoDefaultStatementNode::Clone(CloneContext& cloneContext) const
{
    return new GotoDefaultStatementNode(GetSpan());
}

void GotoDefaultStatementNode::Write(AstWriter& writer)
{
    StatementNode::Write(writer);
}

void GotoDefaultStatementNode::Read(AstReader& reader)
{
    StatementNode::Read(reader);
}

void GotoDefaultStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ThrowStatementNode::ThrowStatementNode(const Span& span_) : StatementNode(span_)
{
}

ThrowStatementNode::ThrowStatementNode(const Span& span_, Node* expression_) : StatementNode(span_), expression(expression_)
{
    if (expression)
    {
        expression->SetParent(this);
    }
}

Node* ThrowStatementNode::Clone(CloneContext& cloneContext) const
{
    Node* clonedExpression = nullptr;
    if (expression)
    {
        clonedExpression = expression->Clone(cloneContext);
    }
    return new ThrowStatementNode(GetSpan(), clonedExpression);
}

void ThrowStatementNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    bool hasExpression = expression != nullptr;
    writer.AsMachineWriter().Put(hasExpression);
    if (hasExpression)
    {
        writer.Put(expression.get());
    }
}

void ThrowStatementNode::Read(AstReader& reader)
{
    Node::Read(reader);
    bool hasExpression = reader.GetBool();
    if (hasExpression)
    {
        expression.reset(reader.GetNode());
        expression->SetParent(this);
    }
}

void ThrowStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

CatchNode::CatchNode(const Span& span_) : Node(span_)
{
}

CatchNode::CatchNode(const Span& span_, Node* typeExpr_, IdentifierNode* id_, CompoundStatementNode* catchBlock_) : Node(span_), typeExpr(typeExpr_), id(id_), catchBlock(catchBlock_)
{
    if (typeExpr)
    {
        typeExpr->SetParent(this);
    }
    if (id)
    {
        id->SetParent(this);
    }
    catchBlock->SetParent(this);
}

CatchNode::CatchNode(const Span& span_, CompoundStatementNode* catchBlock_) : Node(span_), catchBlock(catchBlock_)
{
}

Node* CatchNode::Clone(CloneContext& cloneContext) const
{
    Node* clonedTypeExpr = nullptr;
    if (typeExpr)
    {
        clonedTypeExpr = typeExpr->Clone(cloneContext);
    }
    IdentifierNode* clonedId = nullptr;
    if (id)
    {
        clonedId = static_cast<IdentifierNode*>(id->Clone(cloneContext));
    }
    CatchNode* clone = new CatchNode(GetSpan(), clonedTypeExpr, clonedId, static_cast<CompoundStatementNode*>(catchBlock->Clone(cloneContext)));
    return clone;
}

void CatchNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    bool hasTypeExpr = typeExpr != nullptr;
    writer.AsMachineWriter().Put(hasTypeExpr);
    if (hasTypeExpr)
    {
        writer.Put(typeExpr.get());
    }
    bool hasId = id != nullptr;
    writer.AsMachineWriter().Put(hasId);
    if (hasId)
    {
        writer.Put(id.get());
    }
    writer.Put(catchBlock.get());
}

void CatchNode::Read(AstReader& reader)
{
    Node::Read(reader);
    bool hasTypeExpr = reader.GetBool();
    if (hasTypeExpr)
    {
        typeExpr.reset(reader.GetNode());
        typeExpr->SetParent(this);
    }
    bool hasId = reader.GetBool();
    if (hasId)
    {
        id.reset(reader.GetIdentifierNode());
        id->SetParent(this);
    }
    catchBlock.reset(reader.GetCompoundStatementNode());
    catchBlock->SetParent(this);
}

void CatchNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

TryStatementNode::TryStatementNode(const Span& span_) : StatementNode(span_)
{
}

TryStatementNode::TryStatementNode(const Span& span_, CompoundStatementNode* tryBlock_) : StatementNode(span_), tryBlock(tryBlock_)
{
    tryBlock->SetParent(this);
}

Node* TryStatementNode::Clone(CloneContext& cloneContext) const
{
    TryStatementNode* clone = new TryStatementNode(GetSpan(), static_cast<CompoundStatementNode*>(tryBlock->Clone(cloneContext)));
    int n = catches.Count();
    for (int i = 0; i < n; ++i)
    {
        CatchNode* catch_ = catches[i];
        clone->AddCatch(static_cast<CatchNode*>(catch_->Clone(cloneContext)));
    }
    if (finallyBlock)
    {
        clone->SetFinally(static_cast<CompoundStatementNode*>(finallyBlock->Clone(cloneContext)));
    }
    return clone;
}

void TryStatementNode::Write(AstWriter& writer)
{
    StatementNode::Write(writer);
    writer.Put(tryBlock.get());
    catches.Write(writer);
    bool hasFinallyBlock = finallyBlock != nullptr;
    writer.AsMachineWriter().Put(hasFinallyBlock);
    if (hasFinallyBlock)
    {
        writer.Put(finallyBlock.get());
    }
}

void TryStatementNode::Read(AstReader& reader)
{
    StatementNode::Read(reader);
    tryBlock.reset(reader.GetCompoundStatementNode());
    tryBlock->SetParent(this);
    catches.Read(reader);
    catches.SetParent(this);
    bool hasFinallyBlock = reader.GetBool();
    if (hasFinallyBlock)
    {
        finallyBlock.reset(reader.GetCompoundStatementNode());
        finallyBlock->SetParent(this);
    }
}

void TryStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void TryStatementNode::AddCatch(CatchNode* catch_)
{
    catch_->SetParent(this);
    catches.Add(catch_);
}

void TryStatementNode::SetFinally(CompoundStatementNode* finallyBlock_)
{
    finallyBlock.reset(finallyBlock_);
    finallyBlock->SetParent(this);
}

UsingStatementNode::UsingStatementNode(const Span& span_) : StatementNode(span_)
{
}

UsingStatementNode::UsingStatementNode(const Span& span_, ConstructionStatementNode* constructionStatement_, StatementNode* statement_) :
    StatementNode(span_), constructionStatement(constructionStatement_), statement(statement_)
{
    constructionStatement->SetParent(this);
    statement->SetParent(this);
}

Node* UsingStatementNode::Clone(CloneContext& cloneContext) const
{
    return new UsingStatementNode(GetSpan(), static_cast<ConstructionStatementNode*>(constructionStatement->Clone(cloneContext)),
        static_cast<StatementNode*>(statement->Clone(cloneContext)));
}

void UsingStatementNode::Write(AstWriter& writer)
{
    StatementNode::Write(writer);
    writer.Put(constructionStatement.get());
    writer.Put(statement.get());
}

void UsingStatementNode::Read(AstReader& reader)
{
    StatementNode::Read(reader);
    constructionStatement.reset(reader.GetConstructionStatementNode());
    constructionStatement->SetParent(this);
    statement.reset(reader.GetStatementNode());
    statement->SetParent(this);
}

void UsingStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::ast
