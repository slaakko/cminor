// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_STATEMENT_INCLUDED
#define CMINOR_AST_STATEMENT_INCLUDED
#include <cminor/ast/Node.hpp>
#include <cminor/ast/NodeList.hpp>

namespace cminor { namespace ast {

class IdentifierNode;

class LabelNode : public Node
{
public:
    LabelNode(const Span& span_);
    LabelNode(const Span& span_, const std::string& label_);
    NodeType GetNodeType() const override { return NodeType::labelNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const std::string& Label() const { return label; }
private:
    std::string label;
};

class StatementNode : public Node
{
public:
    StatementNode(const Span& span_);
    void SetLabelNode(LabelNode* labelNode_);
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void CloneLabelTo(StatementNode* clone, CloneContext& cloneContext) const;
    LabelNode* Label() const { return labelNode.get(); }
    virtual bool IsFunctionTerminatingNode() const { return false; }
    virtual bool IsCaseTerminatingNode() const { return false; }
    virtual bool IsDefaultTerminatingNode() const { return false; }
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
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    void SetBeginBraceSpan(const Span& beginBraceSpan_) { beginBraceSpan = beginBraceSpan_; }
    const Span& BeginBraceSpan() const { return beginBraceSpan; }
    void SetEndBraceSpan(const Span& endBraceSpan_) { endBraceSpan = endBraceSpan_; }
    const Span& EndBraceSpan() const { return endBraceSpan; }
private:
    NodeList<StatementNode> statements;
    Span beginBraceSpan;
    Span endBraceSpan;
};

class ReturnStatementNode : public StatementNode
{
public:
    ReturnStatementNode(const Span& span_);
    ReturnStatementNode(const Span& span_, Node* expression_);
    NodeType GetNodeType() const override { return NodeType::returnStatementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    Node* Expression() const { return expression.get(); }
    bool IsFunctionTerminatingNode() const override { return true; }
    bool IsCaseTerminatingNode() const override { return true; }
    bool IsDefaultTerminatingNode() const override { return true; }
private:
    std::unique_ptr<Node> expression;
};

class IfStatementNode : public StatementNode
{
public:
    IfStatementNode(const Span& span_);
    IfStatementNode(const Span& span_, Node* condition_, StatementNode* thenS_, StatementNode* elseS_);
    NodeType GetNodeType() const override { return NodeType::ifStatementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    Node* Condition() const { return condition.get(); }
    StatementNode* ThenS() const { return thenS.get(); }
    StatementNode* ElseS() const { return elseS.get(); }
private:
    std::unique_ptr<Node> condition;
    std::unique_ptr<StatementNode> thenS;
    std::unique_ptr<StatementNode> elseS;
};

class WhileStatementNode : public StatementNode
{
public:
    WhileStatementNode(const Span& span_);
    WhileStatementNode(const Span& span_, Node* condition_, StatementNode* statement_);
    NodeType GetNodeType() const override { return NodeType::whileStatementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    Node* Condition() const { return condition.get(); }
    StatementNode* Statement() const { return statement.get(); }
    bool IsBreakEnclosingStatementNode() const override { return true; }
    bool IsContinueEnclosingStatementNode() const override { return true; }
private:
    std::unique_ptr<Node> condition;
    std::unique_ptr<StatementNode> statement;
};

class DoStatementNode : public StatementNode
{
public:
    DoStatementNode(const Span& span_);
    DoStatementNode(const Span& span_, StatementNode* statement_, Node* condition_);
    NodeType GetNodeType() const override { return NodeType::doStatementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    StatementNode* Statement() const { return statement.get(); }
    Node* Condition() const { return condition.get(); }
    bool IsBreakEnclosingStatementNode() const override { return true; }
    bool IsContinueEnclosingStatementNode() const override { return true; }
private:
    std::unique_ptr<StatementNode> statement;
    std::unique_ptr<Node> condition;
};

class ForStatementNode : public StatementNode
{
public:
    ForStatementNode(const Span& span_);
    ForStatementNode(const Span& span_, StatementNode* initS_, Node* condition_, StatementNode* loopS_, StatementNode* actionS_);
    NodeType GetNodeType() const override { return NodeType::forStatementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    StatementNode* InitS() const { return initS.get(); }
    Node* Condition() const { return condition.get(); }
    StatementNode* LoopS() const { return loopS.get(); }
    StatementNode* ActionS() const { return actionS.get(); }
    bool IsBreakEnclosingStatementNode() const override { return true; }
    bool IsContinueEnclosingStatementNode() const override { return true; }
private:
    std::unique_ptr<StatementNode> initS;
    std::unique_ptr<Node> condition;
    std::unique_ptr<StatementNode> loopS;
    std::unique_ptr<StatementNode> actionS;
};

class BreakStatementNode : public StatementNode
{
public:
    BreakStatementNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::breakStatementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    bool IsCaseTerminatingNode() const override { return true; }
    bool IsDefaultTerminatingNode() const override { return true; }
};

class ContinueStatementNode : public StatementNode
{
public:
    ContinueStatementNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::continueStatementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    bool IsCaseTerminatingNode() const override { return true; }
    bool IsDefaultTerminatingNode() const override { return true; }
};

class ConstructionStatementNode : public StatementNode
{
public:
    ConstructionStatementNode(const Span& span_);
    ConstructionStatementNode(const Span& span_, Node* typeExpr_, IdentifierNode* id_);
    NodeType GetNodeType() const override { return NodeType::constructionStatementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    Node* TypeExpr() const { return typeExpr.get(); }
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
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    Node* TargetExpr() const { return targetExpr.get(); }
    Node* SourceExpr() const { return sourceExpr.get(); }
private:
    std::unique_ptr<Node> targetExpr;
    std::unique_ptr<Node> sourceExpr;
};

class ExpressionStatementNode : public StatementNode
{
public:
    ExpressionStatementNode(const Span& span_);
    ExpressionStatementNode(const Span& span_, Node* expression_);
    NodeType GetNodeType() const override { return NodeType::expressionStatementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    Node* Expression() const { return expression.get(); }
private:
    std::unique_ptr<Node> expression;
};

class EmptyStatementNode : public StatementNode
{
public:
    EmptyStatementNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::emptyStatementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class IncrementStatementNode : public StatementNode
{
public:
    IncrementStatementNode(const Span& span_);
    IncrementStatementNode(const Span& span_, Node* expression_);
    NodeType GetNodeType() const override { return NodeType::incrementStatementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    Node* Expression() const { return expression.get(); }
private:
    std::unique_ptr<Node> expression;
};

class DecrementStatementNode : public StatementNode
{
public:
    DecrementStatementNode(const Span& span_);
    DecrementStatementNode(const Span& span_, Node* expression_);
    NodeType GetNodeType() const override { return NodeType::decrementStatementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    Node* Expression() const { return expression.get(); }
private:
    std::unique_ptr<Node> expression;
};

class ForEachStatementNode : public StatementNode
{
public:
    ForEachStatementNode(const Span& span_);
    ForEachStatementNode(const Span& span_, Node* typeExpr_, IdentifierNode* id_, Node* container_, CompoundStatementNode* action_);
    NodeType GetNodeType() const override { return NodeType::forEachStatementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    Node* TypeExpr() const { return typeExpr.get(); }
    IdentifierNode* Id() const { return id.get(); }
    Node* Container() const { return container.get(); }
    CompoundStatementNode* Action() const { return action.get(); }
private:
    std::unique_ptr<Node> typeExpr;
    std::unique_ptr<IdentifierNode> id;
    std::unique_ptr<Node> container;
    std::unique_ptr<CompoundStatementNode> action;
};

class CaseStatementNode;
class DefaultStatementNode;

class SwitchStatementNode : public StatementNode
{
public:
    SwitchStatementNode(const Span& span_);
    SwitchStatementNode(const Span& span_, Node* condition_);
    NodeType GetNodeType() const override { return NodeType::switchStatementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    Node* Condition() const { return condition.get(); }
    void AddCase(CaseStatementNode* caseS);
    const NodeList<CaseStatementNode>& Cases() const;
    void SetDefault(DefaultStatementNode* defaultS_);
    DefaultStatementNode* Default() const;
    bool IsBreakEnclosingStatementNode() const override { return true; }
private:
    std::unique_ptr<Node> condition;
    NodeList<CaseStatementNode> cases;
    std::unique_ptr<DefaultStatementNode> defaultS;
};

class CaseStatementNode : public StatementNode
{
public:
    CaseStatementNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::caseStatementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    void AddCaseExpr(Node* caseExpr);
    const NodeList<Node>& CaseExprs() const;
    void AddStatement(StatementNode* statement);
    const NodeList<StatementNode>& Statements() const;
private:
    NodeList<Node> caseExprs;
    NodeList<StatementNode> statements;
};

class DefaultStatementNode : public StatementNode
{
public:
    DefaultStatementNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::defaultStatementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    void AddStatement(StatementNode* statement);
    const NodeList<StatementNode>& Statements() const;
private:
    NodeList<StatementNode> statements;
};

class GotoCaseStatementNode : public StatementNode
{
public:
    GotoCaseStatementNode(const Span& span_);
    GotoCaseStatementNode(const Span& span_, Node* caseExpr_);
    NodeType GetNodeType() const override { return NodeType::gotoCaseStatementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    Node* CaseExpr() const { return caseExpr.get(); }
    bool IsCaseTerminatingNode() const override { return true; }
    bool IsDefaultTerminatingNode() const override { return true; }
private:
    std::unique_ptr<Node> caseExpr;
};

class GotoDefaultStatementNode : public StatementNode
{
public:
    GotoDefaultStatementNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::gotoDefaultStatementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    bool IsCaseTerminatingNode() const override { return true; }
};

class ThrowStatementNode : public StatementNode
{
public:
    ThrowStatementNode(const Span& span_);
    ThrowStatementNode(const Span& span_, Node* expression_);
    NodeType GetNodeType() const override { return NodeType::throwStatementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    Node* Expression() const { return expression.get(); }
    bool IsFunctionTerminatingNode() const override { return true; }
    bool IsCaseTerminatingNode() const override { return true; }
private:
    std::unique_ptr<Node> expression;
};

class CatchNode : public Node
{
public:
    CatchNode(const Span& span_);
    CatchNode(const Span& span_, Node* typeExpr_, IdentifierNode* id_, CompoundStatementNode* catchBlock_);
    CatchNode(const Span& span_, CompoundStatementNode* catchBlock_);
    NodeType GetNodeType() const override { return NodeType::catchNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    Node* TypeExpr() const { return typeExpr.get(); }
    IdentifierNode* Id() const { return id.get(); }
    CompoundStatementNode* CatchBlock() const { return catchBlock.get(); }
private:
    std::unique_ptr<Node> typeExpr;
    std::unique_ptr<IdentifierNode> id;
    std::unique_ptr<CompoundStatementNode> catchBlock;
};

class TryStatementNode : public StatementNode
{
public:
    TryStatementNode(const Span& span_);
    TryStatementNode(const Span& span_, CompoundStatementNode* tryBlock_);
    NodeType GetNodeType() const override { return NodeType::tryStatementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    CompoundStatementNode* TryBlock() const { return tryBlock.get(); }
    const NodeList<CatchNode>& Catches() const { return catches; }
    CompoundStatementNode* FinallyBlock() const { return finallyBlock.get(); }
    void AddCatch(CatchNode* catch_);
    void SetFinally(CompoundStatementNode* finallyBlock_);
private:
    std::unique_ptr<CompoundStatementNode> tryBlock;
    NodeList<CatchNode> catches;
    std::unique_ptr<CompoundStatementNode> finallyBlock;
};

} } // namespace cminor::ast

#endif // CMINOR_AST_STATEMENT_INCLUDED
