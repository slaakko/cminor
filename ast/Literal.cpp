// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Literal.hpp>
#include <cminor/ast/Visitor.hpp>
#include <cminor/ast/AstWriter.hpp>
#include <cminor/ast/AstReader.hpp>
#include <cminor/machine/Constant.hpp>

namespace cminor { namespace ast {

BooleanLiteralNode::BooleanLiteralNode(const Span& span_) : Node(span_), value(false)
{
}

BooleanLiteralNode::BooleanLiteralNode(const Span& span_, bool value_) : Node(span_), value(value_)
{
}

Node* BooleanLiteralNode::Clone(CloneContext& cloneContext) const
{
    return new BooleanLiteralNode(GetSpan(), value);
}

void BooleanLiteralNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.AsMachineWriter().Put(value);
}

void BooleanLiteralNode::Read(AstReader& reader)
{
    Node::Read(reader);
    value = reader.GetBool();
}

void BooleanLiteralNode::Accept(Visitor& visitor) 
{
    visitor.Visit(*this);
}

SByteLiteralNode::SByteLiteralNode(const Span& span_) : Node(span_), value(0)
{
}

SByteLiteralNode::SByteLiteralNode(const Span& span_, int8_t value_) : Node(span_), value(value_)
{
}

Node* SByteLiteralNode::Clone(CloneContext& cloneContext) const
{
    return new SByteLiteralNode(GetSpan(), value);
}

void SByteLiteralNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.AsMachineWriter().Put(value);
}

void SByteLiteralNode::Read(AstReader& reader)
{
    Node::Read(reader);
    value = reader.GetSByte();
}

void SByteLiteralNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ByteLiteralNode::ByteLiteralNode(const Span& span_) : Node(span_), value(0)
{
}

ByteLiteralNode::ByteLiteralNode(const Span& span_, uint8_t value_) : Node(span_), value(value_)
{
}

Node* ByteLiteralNode::Clone(CloneContext& cloneContext) const
{
    return new ByteLiteralNode(GetSpan(), value);
}

void ByteLiteralNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.AsMachineWriter().Put(value);
}

void ByteLiteralNode::Read(AstReader& reader)
{
    Node::Read(reader);
    value = reader.GetByte();
}

void ByteLiteralNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ShortLiteralNode::ShortLiteralNode(const Span& span_) : Node(span_), value(0)
{
}

ShortLiteralNode::ShortLiteralNode(const Span& span_, int16_t value_) : Node(span_), value(value_)
{
}

Node* ShortLiteralNode::Clone(CloneContext& cloneContext) const
{
    return new ShortLiteralNode(GetSpan(), value);
}

void ShortLiteralNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.AsMachineWriter().Put(value);
}

void ShortLiteralNode::Read(AstReader& reader)
{
    Node::Read(reader);
    value = reader.GetShort();
}

void ShortLiteralNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

UShortLiteralNode::UShortLiteralNode(const Span& span_) : Node(span_), value(0)
{
}

UShortLiteralNode::UShortLiteralNode(const Span& span_, uint16_t value_) : Node(span_), value(value_)
{
}

Node* UShortLiteralNode::Clone(CloneContext& cloneContext) const
{
    return new UShortLiteralNode(GetSpan(), value);
}

void UShortLiteralNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.AsMachineWriter().Put(value);
}

void UShortLiteralNode::Read(AstReader& reader)
{
    Node::Read(reader);
    value = reader.GetUShort();
}

void UShortLiteralNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

IntLiteralNode::IntLiteralNode(const Span& span_) : Node(span_), value(0)
{
}

IntLiteralNode::IntLiteralNode(const Span& span_, int32_t value_) : Node(span_), value(value_)
{
}

Node* IntLiteralNode::Clone(CloneContext& cloneContext) const
{
    return new IntLiteralNode(GetSpan(), value);
}

void IntLiteralNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.AsMachineWriter().Put(value);
}

void IntLiteralNode::Read(AstReader& reader)
{
    Node::Read(reader);
    value = reader.GetInt();
}

void IntLiteralNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

UIntLiteralNode::UIntLiteralNode(const Span& span_) : Node(span_), value(0)
{
}

UIntLiteralNode::UIntLiteralNode(const Span& span_, uint32_t value_) : Node(span_), value(value_)
{
}

Node* UIntLiteralNode::Clone(CloneContext& cloneContext) const
{
    return new UIntLiteralNode(GetSpan(), value);
}

void UIntLiteralNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.AsMachineWriter().Put(value);
}

void UIntLiteralNode::Read(AstReader& reader)
{
    Node::Read(reader);
    value = reader.GetUInt();
}

void UIntLiteralNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

LongLiteralNode::LongLiteralNode(const Span& span_) : Node(span_), value(0)
{
}

LongLiteralNode::LongLiteralNode(const Span& span_, int64_t value_) : Node(span_), value(value_)
{
}

Node* LongLiteralNode::Clone(CloneContext& cloneContext) const
{
    return new LongLiteralNode(GetSpan(), value);
}

void LongLiteralNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.AsMachineWriter().Put(value);
}

void LongLiteralNode::Read(AstReader& reader)
{
    Node::Read(reader);
    value = reader.GetLong();
}

void LongLiteralNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ULongLiteralNode::ULongLiteralNode(const Span& span_) : Node(span_), value(0)
{
}

ULongLiteralNode::ULongLiteralNode(const Span& span_, uint64_t value_) : Node(span_), value(value_)
{
}

Node* ULongLiteralNode::Clone(CloneContext& cloneContext) const
{
    return new ULongLiteralNode(GetSpan(), value);
}

void ULongLiteralNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.AsMachineWriter().Put(value);
}

void ULongLiteralNode::Read(AstReader& reader)
{
    Node::Read(reader);
    value = reader.GetULong();
}

void ULongLiteralNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

Node* CreateIntegerLiteralNode(const Span& span, uint64_t value, bool unsignedSuffix)
{
    if (unsignedSuffix)
    {
        if (value <= std::numeric_limits<uint8_t>::max()) return new ByteLiteralNode(span, static_cast<uint8_t>(value));
        if (value <= std::numeric_limits<uint16_t>::max()) return new UShortLiteralNode(span, static_cast<uint16_t>(value));
        if (value <= std::numeric_limits<uint32_t>::max()) return new UIntLiteralNode(span, static_cast<uint32_t>(value));
        return new ULongLiteralNode(span, value);
    }
    else
    {
        if (value <= std::numeric_limits<int8_t>::max()) return new SByteLiteralNode(span, static_cast<int8_t>(value));
        if (value <= std::numeric_limits<uint8_t>::max()) return new ByteLiteralNode(span, static_cast<uint8_t>(value));
        if (value <= std::numeric_limits<int16_t>::max()) return new ShortLiteralNode(span, static_cast<int16_t>(value));
        if (value <= std::numeric_limits<uint16_t>::max()) return new UShortLiteralNode(span, static_cast<uint16_t>(value));
        if (value <= std::numeric_limits<int32_t>::max()) return new IntLiteralNode(span, static_cast<int32_t>(value));
        if (value <= std::numeric_limits<uint32_t>::max()) return new UIntLiteralNode(span, static_cast<uint32_t>(value));
#pragma warning(disable : 4018)
        if (value <= std::numeric_limits<int64_t>::max()) return new LongLiteralNode(span, static_cast<int64_t>(value));
#pragma warning(default : 4018)
        return new ULongLiteralNode(span, value);
    }
}

FloatLiteralNode::FloatLiteralNode(const Span& span_) : Node(span_), value(0.0f)
{
}

FloatLiteralNode::FloatLiteralNode(const Span& span_, float value_) : Node(span_), value(value_)
{
}

Node* FloatLiteralNode::Clone(CloneContext& cloneContext) const
{
    return new FloatLiteralNode(GetSpan(), value);
}

void FloatLiteralNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.AsMachineWriter().Put(value);
}

void FloatLiteralNode::Read(AstReader& reader)
{
    Node::Read(reader);
    value = reader.GetFloat();
}

void FloatLiteralNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

DoubleLiteralNode::DoubleLiteralNode(const Span& span_) : Node(span_), value(0.0)
{
}

DoubleLiteralNode::DoubleLiteralNode(const Span& span_, double value_) : Node(span_), value(value_)
{
}

Node* DoubleLiteralNode::Clone(CloneContext& cloneContext) const
{
    return new DoubleLiteralNode(GetSpan(), value);
}

void DoubleLiteralNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.AsMachineWriter().Put(value);
}

void DoubleLiteralNode::Read(AstReader& reader)
{
    Node::Read(reader);
    value = reader.GetDouble();
}

void DoubleLiteralNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

Node* CreateFloatingLiteralNode(const Span& span, double value, bool float_)
{
    if (float_)
    {
        return new FloatLiteralNode(span, static_cast<float>(value));
    }
    else
    {
        return new DoubleLiteralNode(span, value);
    }
}

CharLiteralNode::CharLiteralNode(const Span& span_) : Node(span_), value('\0')
{
}

CharLiteralNode::CharLiteralNode(const Span& span_, char32_t value_) : Node(span_), value(value_)
{
}

Node* CharLiteralNode::Clone(CloneContext& cloneContext) const
{
    return new CharLiteralNode(GetSpan(), value);
}

void CharLiteralNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.AsMachineWriter().Put(value);
}

void CharLiteralNode::Read(AstReader& reader)
{
    Node::Read(reader);
    value = reader.GetChar();
}

void CharLiteralNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

StringLiteralNode::StringLiteralNode(const Span& span_) : Node(span_), value()
{
}

StringLiteralNode::StringLiteralNode(const Span& span_, const std::u32string& value_) : Node(span_), value(value_)
{
}

Node* StringLiteralNode::Clone(CloneContext& cloneContext) const
{
    return new StringLiteralNode(GetSpan(), value);
}

void StringLiteralNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    ConstantId id = writer.GetConstantPool()->GetIdFor(value);
    Assert(id != noConstantId, "got no id for constant");
    id.Write(writer);
}

void StringLiteralNode::Read(AstReader& reader)
{
    Node::Read(reader);
    ConstantId id;
    id.Read(reader);
    Constant constant = reader.GetConstantPool()->GetConstant(id);
    Assert(constant.Value().GetType() == ValueType::stringLiteral, "string literal expected");
    value = constant.Value().AsStringLiteral();
}

void StringLiteralNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

NullLiteralNode::NullLiteralNode(const Span& span_) : Node(span_)
{
}

Node* NullLiteralNode::Clone(CloneContext& cloneContext) const
{
    return new NullLiteralNode(GetSpan());
}

void NullLiteralNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::ast
