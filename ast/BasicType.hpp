// =================================
// Copyright (c) 2017 Seppo Laakko
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
    std::string ToString() const override { return "System.Boolean"; }
};

class SByteNode : public Node
{
public:
    SByteNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::sbyteNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    std::string ToString() const override { return "System.Int8"; }
};

class ByteNode : public Node
{
public:
    ByteNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::byteNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    std::string ToString() const override { return "System.UInt8"; }
    bool IsUnsignedTypeNode() const override { return true; }
};

class ShortNode : public Node
{
public:
    ShortNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::shortNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    std::string ToString() const override { return "System.Int16"; }
};

class UShortNode : public Node
{
public:
    UShortNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::ushortNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    std::string ToString() const override { return "System.UInt16"; }
    bool IsUnsignedTypeNode() const override { return true; }
};

class IntNode : public Node
{
public:
    IntNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::intNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    std::string ToString() const override { return "System.Int32"; }
};

class UIntNode : public Node
{
public:
    UIntNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::uintNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    std::string ToString() const override { return "System.UInt32"; }
    bool IsUnsignedTypeNode() const override { return true; }
};

class LongNode : public Node
{
public:
    LongNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::longNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    std::string ToString() const override { return "System.Int64"; }
};

class ULongNode : public Node
{
public:
    ULongNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::ulongNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    std::string ToString() const override { return "System.UInt64"; }
    bool IsUnsignedTypeNode() const override { return true; }
};

class FloatNode : public Node
{
public:
    FloatNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::floatNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    std::string ToString() const override { return "System.Float"; }
};

class DoubleNode : public Node
{
public:
    DoubleNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::doubleNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    std::string ToString() const override { return "System.Double"; }
};

class CharNode : public Node
{
public:
    CharNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::charNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    std::string ToString() const override { return "System.Char"; }
};

class StringNode : public Node
{
public:
    StringNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::stringNode;  }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    std::string ToString() const override { return "System.String"; }
};

class VoidNode : public Node
{
public:
    VoidNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::voidNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    std::string ToString() const override { return "System.Void"; }
};

class ObjectNode : public Node
{
public:
    ObjectNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::objectNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    std::string ToString() const override { return "System.Object"; }
};

} } // namespace cminor::ast

#endif // CMINOR_AST_BASIC_TYPE_INCLUDED
