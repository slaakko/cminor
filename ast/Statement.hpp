// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_STATEMENT_INCLUDED
#define CMINOR_AST_STATEMENT_INCLUDED
#include <cminor/ast/Node.hpp>

namespace cminor { namespace ast {

class IdentifierNode;

class LabelNode : public Node
{
public:
    LabelNode(const Span& span_);
    LabelNode(const Span& span_, const std::string& label_);
    NodeType GetNodeType() const override { return NodeType::labelNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    const std::string& Label() const { return label; }
private:
    std::string label;
};

class StatementNode : public Node
{
public:
    StatementNode(const Span& span_);
    void SetLabelNode(LabelNode* labelNode_);
    void CloneLabelTo(StatementNode* clone, CloneContext& cloneContext) const;
    LabelNode* Label() const { return labelNode.get(); }
private:
    std::unique_ptr<LabelNode> labelNode;
};

class CompoundStatementNode : public StatementNode
{
public:
    CompoundStatementNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::compoundStatementNode; }
    void AddStatement(StatementNode* statement);
    NodeList<StatementNode>& Statements() { return statements; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
private:
    NodeList<StatementNode> statements;
};

class ConstructionStatementNode : public StatementNode
{
public:
    ConstructionStatementNode(const Span& span_);
    ConstructionStatementNode(const Span& span_, Node* typeExpr_, IdentifierNode* id_);
    NodeType GetNodeType() const override { return NodeType::constructionStatementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    IdentifierNode* Id() const { return id.get(); }
    void AddArgument(Node* argument) override;
    NodeList<Node>& Arguments() { return arguments; }
private:
    std::unique_ptr<Node> typeExpr;
    std::unique_ptr<IdentifierNode> id;
    NodeList<Node> arguments;
};

class AssignmentStatementNode : public StatementNode
{
public:
    AssignmentStatementNode(const Span& span_);
    AssignmentStatementNode(const Span& span_, Node* targetExpr_, Node* sourceExpr_);
    NodeType GetNodeType() const override { return NodeType::assignmentStatementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    Node* TargetExpr() const { return targetExpr.get(); }
    Node* SourceExpr() const { return sourceExpr.get(); }
private:
    std::unique_ptr<Node> targetExpr;
    std::unique_ptr<Node> sourceExpr;
};

} } // namespace cminor::ast

#endif // CMINOR_AST_STATEMENT_INCLUDED
