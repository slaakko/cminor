// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_LITERAL_INCLUDED
#define CMINOR_AST_LITERAL_INCLUDED
#include <cminor/ast/Node.hpp>
#include <cminor/machine/Unicode.hpp>

namespace cminor { namespace ast {

using cminor::machine::utf32_string;

class BooleanLiteralNode : public Node
{
public:
    BooleanLiteralNode(const Span& span_);
    BooleanLiteralNode(const Span& span_, bool value_);
    NodeType GetNodeType() const override { return NodeType::booleanLiteralNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    bool Value() const { return value; }
private:
    bool value;
};

class SByteLiteralNode : public Node
{
public:
    SByteLiteralNode(const Span& span_);
    SByteLiteralNode(const Span& span_, int8_t value_);
    NodeType GetNodeType() const override { return NodeType::sbyteLiteralNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    int8_t Value() const { return value; }
private:
    int8_t value;
};

class ByteLiteralNode : public Node
{
public:
    ByteLiteralNode(const Span& span_);
    ByteLiteralNode(const Span& span_, uint8_t value_);
    NodeType GetNodeType() const override { return NodeType::byteLiteralNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    uint8_t Value() const { return value; }
private:
    uint8_t value;
};

class ShortLiteralNode : public Node
{
public:
    ShortLiteralNode(const Span& span_);
    ShortLiteralNode(const Span& span_, int16_t value_);
    NodeType GetNodeType() const override { return NodeType::shortLiteralNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    int16_t Value() const { return value; }
private:
    int16_t value;
};

class UShortLiteralNode : public Node
{
public:
    UShortLiteralNode(const Span& span_);
    UShortLiteralNode(const Span& span_, uint16_t value_);
    NodeType GetNodeType() const override { return NodeType::ushortLiteralNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    uint16_t Value() const { return value; }
private:
    uint16_t value;
};

class IntLiteralNode : public Node
{
public:
    IntLiteralNode(const Span& span_);
    IntLiteralNode(const Span& span_, int32_t value_);
    NodeType GetNodeType() const override { return NodeType::intLiteralNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    int32_t Value() const { return value; }
private:
    int32_t value;
};

class UIntLiteralNode : public Node
{
public:
    UIntLiteralNode(const Span& span_);
    UIntLiteralNode(const Span& span_, uint32_t value_);
    NodeType GetNodeType() const override { return NodeType::uintLiteralNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    uint32_t Value() const { return value; }
private:
    uint32_t value;
};

class LongLiteralNode : public Node
{
public:
    LongLiteralNode(const Span& span_);
    LongLiteralNode(const Span& span_, int64_t value_);
    NodeType GetNodeType() const override { return NodeType::longLiteralNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    int64_t Value() const { return value; }
private:
    int64_t value;
};

class ULongLiteralNode : public Node
{
public:
    ULongLiteralNode(const Span& span_);
    ULongLiteralNode(const Span& span_, uint64_t value_);
    NodeType GetNodeType() const override { return NodeType::ulongLiteralNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    uint64_t Value() const { return value; }
private:
    uint64_t value;
};

Node* CreateIntegerLiteralNode(const Span& span, uint64_t value, bool unsignedSuffix);

class FloatLiteralNode : public Node
{
public:
    FloatLiteralNode(const Span& span_);
    FloatLiteralNode(const Span& span_, float value_);
    NodeType GetNodeType() const override { return NodeType::floatLiteralNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    float Value() const { return value; }
private:
    float value;
};

class DoubleLiteralNode : public Node
{
public:
    DoubleLiteralNode(const Span& span_);
    DoubleLiteralNode(const Span& span_, double value_);
    NodeType GetNodeType() const override { return NodeType::doubleLiteralNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    double Value() const { return value; }
private:
    double value;
};

Node* CreateFloatingLiteralNode(const Span& span, double value, bool float_);

class CharLiteralNode : public Node
{
public:
    CharLiteralNode(const Span& span_);
    CharLiteralNode(const Span& span_, char32_t value_);
    NodeType GetNodeType() const override { return NodeType::charLiteralNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    char32_t Value() const { return value; }
private:
    char32_t value;
};

class StringLiteralNode : public Node
{
public:
    StringLiteralNode(const Span& span_);
    StringLiteralNode(const Span& span_, const utf32_string& value_);
    NodeType GetNodeType() const override { return NodeType::stringLiteralNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    const utf32_string& Value() const { return value; }
private:
    utf32_string value;
};

class NullLiteralNode : public Node
{
public:
    NullLiteralNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::nullLiteralNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

} } // namespace cminor::ast

#endif // CMINOR_AST_LITERAL_INCLUDED
