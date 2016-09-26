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
    //clone->beginBraceSpan = beginBraceSpan;
    //clone->endBraceSpan = endBraceSpan;
    return clone;
}

void CompoundStatementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::ast
