// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_EXPRESSION_INCLUDED
#define CMINOR_AST_EXPRESSION_INCLUDED
#include <cminor/ast/Identifier.hpp>

namespace cminor { namespace ast {

class DisjunctionNode : public BinaryNode
{
public:
    DisjunctionNode(const Span& span_);
    DisjunctionNode(const Span& span_, Node* left_, Node* right_);
    NodeType GetNodeType() const override { return NodeType::disjunctionNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class ConjunctionNode : public BinaryNode
{
public:
    ConjunctionNode(const Span& span_);
    ConjunctionNode(const Span& span_, Node* left_, Node* right_);
    NodeType GetNodeType() const override { return NodeType::conjunctionNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class BitOrNode : public BinaryNode
{
public:
    BitOrNode(const Span& span_);
    BitOrNode(const Span& span_, Node* left_, Node* right_);
    NodeType GetNodeType() const override { return NodeType::bitOrNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class BitXorNode : public BinaryNode
{
public:
    BitXorNode(const Span& span_);
    BitXorNode(const Span& span_, Node* left_, Node* right_);
    NodeType GetNodeType() const override { return NodeType::bitXorNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class BitAndNode : public BinaryNode
{
public:
    BitAndNode(const Span& span_);
    BitAndNode(const Span& span_, Node* left_, Node* right_);
    NodeType GetNodeType() const override { return NodeType::bitAndNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class EqualNode : public BinaryNode
{
public:
    EqualNode(const Span& span_);
    EqualNode(const Span& span_, Node* left_, Node* right_);
    NodeType GetNodeType() const override { return NodeType::equalNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class NotEqualNode : public BinaryNode
{
public:
    NotEqualNode(const Span& span_);
    NotEqualNode(const Span& span_, Node* left_, Node* right_);
    NodeType GetNodeType() const override { return NodeType::notEqualNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class LessNode : public BinaryNode
{
public:
    LessNode(const Span& span_);
    LessNode(const Span& span_, Node* left_, Node* right_);
    NodeType GetNodeType() const override { return NodeType::lessNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class GreaterNode : public BinaryNode
{
public:
    GreaterNode(const Span& span_);
    GreaterNode(const Span& span_, Node* left_, Node* right_);
    NodeType GetNodeType() const override { return NodeType::greaterNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class LessOrEqualNode : public BinaryNode
{
public:
    LessOrEqualNode(const Span& span_);
    LessOrEqualNode(const Span& span_, Node* left_, Node* right_);
    NodeType GetNodeType() const override { return NodeType::lessOrEqualNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class GreaterOrEqualNode : public BinaryNode
{
public:
    GreaterOrEqualNode(const Span& span_);
    GreaterOrEqualNode(const Span& span_, Node* left_, Node* right_);
    NodeType GetNodeType() const override { return NodeType::greaterOrEqualNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class ShiftLeftNode : public BinaryNode
{
public:
    ShiftLeftNode(const Span& span_);
    ShiftLeftNode(const Span& span_, Node* left_, Node* right_);
    NodeType GetNodeType() const override { return NodeType::shiftLeftNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class ShiftRightNode : public BinaryNode
{
public:
    ShiftRightNode(const Span& span_);
    ShiftRightNode(const Span& span_, Node* left_, Node* right_);
    NodeType GetNodeType() const override { return NodeType::shiftRightNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class AddNode : public BinaryNode
{
public:
    AddNode(const Span& span_);
    AddNode(const Span& span_, Node* left_, Node* right_);
    NodeType GetNodeType() const override { return NodeType::addNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class SubNode : public BinaryNode
{
public:
    SubNode(const Span& span_);
    SubNode(const Span& span_, Node* left_, Node* right_);
    NodeType GetNodeType() const override { return NodeType::subNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class MulNode : public BinaryNode
{
public:
    MulNode(const Span& span_);
    MulNode(const Span& span_, Node* left_, Node* right_);
    NodeType GetNodeType() const override { return NodeType::mulNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class DivNode : public BinaryNode
{
public:
    DivNode(const Span& span_);
    DivNode(const Span& span_, Node* left_, Node* right_);
    NodeType GetNodeType() const override { return NodeType::divNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class RemNode : public BinaryNode
{
public:
    RemNode(const Span& span_);
    RemNode(const Span& span_, Node* left_, Node* right_);
    NodeType GetNodeType() const override { return NodeType::remNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class NotNode : public UnaryNode
{
public:
    NotNode(const Span& span_);
    NotNode(const Span& span_, Node* subject_);
    NodeType GetNodeType() const override { return NodeType::notNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class UnaryPlusNode : public UnaryNode
{
public:
    UnaryPlusNode(const Span& span_);
    UnaryPlusNode(const Span& span_, Node* subject_);
    NodeType GetNodeType() const override { return NodeType::unaryPlusNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class UnaryMinusNode : public UnaryNode
{
public:
    UnaryMinusNode(const Span& span_);
    UnaryMinusNode(const Span& span_, Node* subject_);
    NodeType GetNodeType() const override { return NodeType::unaryMinusNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class ComplementNode : public UnaryNode
{
public:
    ComplementNode(const Span& span_);
    ComplementNode(const Span& span_, Node* subject_);
    NodeType GetNodeType() const override { return NodeType::complementNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

} } // namespace cminor::ast

#endif // CMINOR_AST_EXPRESSION_INCLUDED
