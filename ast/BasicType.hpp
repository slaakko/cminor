// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_BASIC_TYPE_INCLUDED
#define CMINOR_AST_BASIC_TYPE_INCLUDED
#include <cminor/ast/Node.hpp>

namespace cminor { namespace ast {

class BoolNode : public Node
{
public:
    BoolNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::boolNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class SByteNode : public Node
{
public:
    SByteNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::sbyteNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class ByteNode : public Node
{
public:
    ByteNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::byteNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class ShortNode : public Node
{
public:
    ShortNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::shortNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class UShortNode : public Node
{
public:
    UShortNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::ushortNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class IntNode : public Node
{
public:
    IntNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::intNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class UIntNode : public Node
{
public:
    UIntNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::uintNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class LongNode : public Node
{
public:
    LongNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::longNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class ULongNode : public Node
{
public:
    ULongNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::ulongNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class FloatNode : public Node
{
public:
    FloatNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::floatNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class DoubleNode : public Node
{
public:
    DoubleNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::doubleNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class CharNode : public Node
{
public:
    CharNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::charNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class VoidNode : public Node
{
public:
    VoidNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::voidNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

} } // namespace cminor::ast

#endif // CMINOR_AST_BASIC_TYPE_INCLUDED
