// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/Value.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>

namespace cminor { namespace symbols {

const char* valueKindStr[] =
{
    "none", "boolValue", "charValue", "sbyteValue", "byteValue", "shortValue", "ushortValue", "intValue", "uintValue", "longValue", "ulongValue", "floatValue", "doubleValue"
};

std::string ValueKindStr(ValueKind valueKind)
{
    return valueKindStr[uint8_t(valueKind)];
}

ValueKind commonType[uint8_t(ValueKind::max)][uint8_t(ValueKind::max)] = 
{
    { ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::none, 
      ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::none },
    { ValueKind::none, ValueKind::boolValue, ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::none, 
      ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::none },
    { ValueKind::none, ValueKind::none, ValueKind::charValue, ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::none, 
      ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::none }, 
    { ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::sbyteValue, ValueKind::shortValue, ValueKind::shortValue, ValueKind::intValue, 
      ValueKind::intValue, ValueKind::longValue, ValueKind::longValue, ValueKind::none, ValueKind::floatValue, ValueKind::doubleValue },
    { ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::shortValue, ValueKind::byteValue, ValueKind::shortValue, ValueKind::ushortValue,
      ValueKind::intValue, ValueKind::uintValue, ValueKind::longValue, ValueKind::ulongValue, ValueKind::floatValue, ValueKind::doubleValue },
    { ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::shortValue, ValueKind::shortValue, ValueKind::shortValue, ValueKind::intValue,
      ValueKind::intValue, ValueKind::longValue, ValueKind::longValue, ValueKind::none, ValueKind::floatValue, ValueKind::doubleValue },
    { ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::intValue, ValueKind::ushortValue, ValueKind::intValue, ValueKind::ushortValue,
      ValueKind::intValue, ValueKind::uintValue, ValueKind::longValue, ValueKind::ulongValue, ValueKind::floatValue, ValueKind::doubleValue },
    { ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::intValue, ValueKind::intValue, ValueKind::intValue, ValueKind::intValue,
      ValueKind::intValue, ValueKind::longValue, ValueKind::longValue, ValueKind::none, ValueKind::floatValue, ValueKind::doubleValue },
    { ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::longValue, ValueKind::uintValue, ValueKind::longValue, ValueKind::uintValue,
      ValueKind::longValue, ValueKind::uintValue, ValueKind::longValue, ValueKind::ulongValue, ValueKind::floatValue, ValueKind::doubleValue },
    { ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::longValue, ValueKind::longValue, ValueKind::longValue, ValueKind::longValue,
      ValueKind::longValue, ValueKind::longValue, ValueKind::longValue, ValueKind::none, ValueKind::floatValue, ValueKind::doubleValue },
    { ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::ulongValue, ValueKind::none, ValueKind::ulongValue,
      ValueKind::none, ValueKind::ulongValue, ValueKind::none, ValueKind::ulongValue, ValueKind::floatValue, ValueKind::doubleValue },
    { ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::floatValue, ValueKind::floatValue, ValueKind::floatValue, ValueKind::floatValue, 
      ValueKind::floatValue, ValueKind::floatValue, ValueKind::floatValue, ValueKind::floatValue, ValueKind::floatValue, ValueKind::doubleValue },
    { ValueKind::none, ValueKind::none, ValueKind::none, ValueKind::doubleValue, ValueKind::doubleValue, ValueKind::doubleValue, ValueKind::doubleValue,
      ValueKind::doubleValue, ValueKind::doubleValue, ValueKind::doubleValue, ValueKind::doubleValue, ValueKind::doubleValue, ValueKind::doubleValue }
};

ValueKind GetCommonType(ValueKind left, ValueKind right)
{
    return commonType[uint8_t(left)][uint8_t(right)];
}

ValueKind GetValueKindFor(SymbolType symbolType, const Span& span)
{
    switch (symbolType)
    {
        case SymbolType::boolTypeSymbol: return ValueKind::boolValue;
        case SymbolType::charTypeSymbol: return ValueKind::charValue;
        case SymbolType::sbyteTypeSymbol: return ValueKind::sbyteValue;
        case SymbolType::byteTypeSymbol: return ValueKind::byteValue;
        case SymbolType::shortTypeSymbol: return ValueKind::shortValue;
        case SymbolType::ushortTypeSymbol: return ValueKind::ushortValue;
        case SymbolType::intTypeSymbol: return ValueKind::intValue;
        case SymbolType::uintTypeSymbol: return ValueKind::uintValue;
        case SymbolType::longTypeSymbol: return ValueKind::longValue;
        case SymbolType::ulongTypeSymbol: return ValueKind::ulongValue;
        case SymbolType::floatTypeSymbol: return ValueKind::floatValue;
        case SymbolType::doubleTypeSymbol: return ValueKind::doubleValue;
        default: throw Exception("cannot evaluate statically", span);
    }
}

Value::~Value()
{
}

BoolValue::BoolValue() : value(false)
{
}

BoolValue::BoolValue(bool value_) : value(value_)
{
}

Value* BoolValue::Clone() const
{
    return new BoolValue(value);
}

void BoolValue::Write(SymbolWriter& writer)
{
    writer.AsMachineWriter().Put(value);
}

void BoolValue::Read(SymbolReader& reader)
{
    value = reader.GetBool();
}

Value* BoolValue::As(ValueKind targetKind, bool cast, const Span& span) const
{
    switch (targetKind)
    {
        case ValueKind::boolValue:
        {
            return new BoolValue(value);
        }
        case ValueKind::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::shortValue:
        {
            if (cast)
            {
                return new ShortValue(static_cast<int16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::intValue:
        {
            if (cast)
            {
                return new IntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::uintValue:
        {
            if (cast)
            {
                return new UIntValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::longValue:
        {
            if (cast)
            {
                return new LongValue(static_cast<int64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(static_cast<int64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::floatValue:
        {
            if (cast)
            {
                return new FloatValue(static_cast<float>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::doubleValue:
        {
            if (cast)
            {
                return new DoubleValue(static_cast<double>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

CharValue::CharValue() : value('\0')
{
}

CharValue::CharValue(char32_t value_) : value(value_)
{
}

Value* CharValue::Clone() const
{
    return new CharValue(value);
}

void CharValue::Write(SymbolWriter& writer)
{
    writer.AsMachineWriter().Put(value);
}

void CharValue::Read(SymbolReader& reader)
{
    value = reader.GetChar();
}

Value* CharValue::As(ValueKind targetKind, bool cast, const Span& span) const
{
    switch (targetKind)
    {
        case ValueKind::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::charValue:
        {
            return new CharValue(value);
        }
        case ValueKind::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::shortValue:
        {
            if (cast)
            {
                return new ShortValue(static_cast<int16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::intValue:
        {
            if (cast)
            {
                return new IntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::uintValue:
        {
            if (cast)
            {
                return new UIntValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::longValue:
        {
            if (cast)
            {
                return new LongValue(static_cast<int64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(static_cast<uint64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::floatValue:
        {
            if (cast)
            {
                return new FloatValue(static_cast<float>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::doubleValue:
        {
            if (cast)
            {
                return new DoubleValue(static_cast<double>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

SByteValue::SByteValue() : value(0)
{
}

SByteValue::SByteValue(int8_t value_) : value(value_)
{
}

Value* SByteValue::Clone() const
{
    return new SByteValue(value);
}

void SByteValue::Write(SymbolWriter& writer)
{
    writer.AsMachineWriter().Put(value);
}

void SByteValue::Read(SymbolReader& reader)
{
    value = reader.GetSByte();
}

Value* SByteValue::As(ValueKind targetKind, bool cast, const Span& span) const
{
    switch (targetKind)
    {
        case ValueKind::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::charValue:
        {
            if (cast)
            {
                return new CharValue(static_cast<char>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::sbyteValue:
        {
            return new SByteValue(value);
        }
        case ValueKind::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::shortValue:
        {
            return new ShortValue(value);
        }
        case ValueKind::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::intValue:
        {
            return new IntValue(value);
        }
        case ValueKind::uintValue:
        {
            if (cast)
            {
                return new UIntValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::longValue:
        {
            return new LongValue(value);
        }
        case ValueKind::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(static_cast<uint64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::floatValue:
        {
            return new FloatValue(value);
        }
        case ValueKind::doubleValue:
        {
            return new DoubleValue(value);
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

ByteValue::ByteValue() : value(0)
{
}

ByteValue::ByteValue(uint8_t value_) : value(value_)
{
}

Value* ByteValue::Clone() const
{
    return new ByteValue(value);
}

void ByteValue::Write(SymbolWriter& writer)
{
    writer.AsMachineWriter().Put(value);
}

void ByteValue::Read(SymbolReader& reader)
{
    value = reader.GetByte();
}

Value* ByteValue::As(ValueKind targetKind, bool cast, const Span& span) const
{
    switch (targetKind)
    {
        case ValueKind::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::charValue:
        {
            if (cast)
            {
                return new CharValue(static_cast<char>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::byteValue:
        {
            return new ByteValue(value);
        }
        case ValueKind::shortValue:
        {
            return new ShortValue(value);
        }
        case ValueKind::ushortValue:
        {
            return new UShortValue(value);
        }
        case ValueKind::intValue:
        {
            return new IntValue(value);
        }
        case ValueKind::uintValue:
        {
            return new UIntValue(value);
        }
        case ValueKind::longValue:
        {
            return new LongValue(value);
        }
        case ValueKind::ulongValue:
        {
            return new ULongValue(value);
        }
        case ValueKind::floatValue:
        {
            return new FloatValue(value);
        }
        case ValueKind::doubleValue:
        {
            return new DoubleValue(value);
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

ShortValue::ShortValue() : value(0)
{
}

ShortValue::ShortValue(int16_t value_) : value(value_)
{
}

Value* ShortValue::Clone() const
{
    return new ShortValue(value);
}

void ShortValue::Write(SymbolWriter& writer)
{
    writer.AsMachineWriter().Put(value);
}

void ShortValue::Read(SymbolReader& reader)
{
    value = reader.GetShort();
}

Value* ShortValue::As(ValueKind targetKind, bool cast, const Span& span) const
{
    switch (targetKind)
    {
        case ValueKind::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::charValue:
        {
            if (cast)
            {
                return new CharValue(static_cast<char>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::shortValue:
        {
            return new ShortValue(value);
        }
        case ValueKind::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::intValue:
        {
            return new IntValue(value);
        }
        case ValueKind::uintValue:
        {
            if (cast)
            {
                return new UIntValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::longValue:
        {
            return new LongValue(value);
        }
        case ValueKind::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(static_cast<uint64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::floatValue:
        {
            return new FloatValue(value);
        }
        case ValueKind::doubleValue:
        {
            return new DoubleValue(value);
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

UShortValue::UShortValue() : value(0)
{
}

UShortValue::UShortValue(uint16_t value_) : value(value_)
{
}

Value* UShortValue::Clone() const
{
    return new UShortValue(value);
}

void UShortValue::Write(SymbolWriter& writer)
{
    writer.AsMachineWriter().Put(value);
}

void UShortValue::Read(SymbolReader& reader)
{
    value = reader.GetUShort();
}

Value* UShortValue::As(ValueKind targetKind, bool cast, const Span& span) const
{
    switch (targetKind)
    {
        case ValueKind::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::charValue:
        {
            if (cast)
            {
                return new CharValue(static_cast<char>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::shortValue:
        {
            if (cast)
            {
                return new ShortValue(static_cast<int16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::ushortValue:
        {
            return new UShortValue(value);
        }
        case ValueKind::intValue:
        {
            return new IntValue(value);
        }
        case ValueKind::uintValue:
        {
            return new UIntValue(value);
        }
        case ValueKind::longValue:
        {
            return new LongValue(value);
        }
        case ValueKind::ulongValue:
        {
            return new ULongValue(value);
        }
        case ValueKind::floatValue:
        {
            return new FloatValue(value);
        }
        case ValueKind::doubleValue:
        {
            return new DoubleValue(value);
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

IntValue::IntValue() : value(0)
{
}

IntValue::IntValue(int32_t value_) : value(value_)
{
}

Value* IntValue::Clone() const
{
    return new IntValue(value);
}

void IntValue::Write(SymbolWriter& writer)
{
    writer.AsMachineWriter().Put(value);
}

void IntValue::Read(SymbolReader& reader)
{
    value = reader.GetInt();
}

Value* IntValue::As(ValueKind targetKind, bool cast, const Span& span) const
{
    switch (targetKind)
    {
        case ValueKind::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::charValue:
        {
            if (cast)
            {
                return new CharValue(static_cast<char>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::shortValue:
        {
            if (cast)
            {
                return new ShortValue(static_cast<int16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::intValue:
        {
            return new IntValue(value);
        }
        case ValueKind::uintValue:
        {
            if (cast)
            {
                return new UIntValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::longValue:
        {
            return new LongValue(value);
        }
        case ValueKind::ulongValue:
        {
            if (cast)
            {
                return new UIntValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::floatValue:
        {
            return new FloatValue(static_cast<float>(value));
        }
        case ValueKind::doubleValue:
        {
            return new DoubleValue(value);
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }

}

UIntValue::UIntValue() : value(0)
{
}

UIntValue::UIntValue(uint32_t value_) : value(value_)
{
}

Value* UIntValue::Clone() const
{
    return new UIntValue(value);
}

void UIntValue::Write(SymbolWriter& writer)
{
    writer.AsMachineWriter().Put(value);
}

void UIntValue::Read(SymbolReader& reader)
{
    value = reader.GetUInt();
}

Value* UIntValue::As(ValueKind targetKind, bool cast, const Span& span) const
{
    switch (targetKind)
    {
        case ValueKind::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::charValue:
        {
            if (cast)
            {
                return new CharValue(static_cast<char>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::shortValue:
        {
            if (cast)
            {
                return new ShortValue(static_cast<int16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::intValue:
        {
            if (cast)
            {
                return new IntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::uintValue:
        {
            return new UIntValue(value);
        }
        case ValueKind::longValue:
        {
            return new LongValue(value);
        }
        case ValueKind::ulongValue:
        {
            return new ULongValue(value);
        }
        case ValueKind::floatValue:
        {
            return new FloatValue(static_cast<float>(value));
        }
        case ValueKind::doubleValue:
        {
            return new DoubleValue(value);
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

LongValue::LongValue() : value(0)
{
}

LongValue::LongValue(int64_t value_) : value(value_)
{
}

Value* LongValue::Clone() const
{
    return new LongValue(value);
}

void LongValue::Write(SymbolWriter& writer)
{
    writer.AsMachineWriter().Put(value);
}

void LongValue::Read(SymbolReader& reader)
{
    value = reader.GetLong();
}

Value* LongValue::As(ValueKind targetKind, bool cast, const Span& span) const
{
    switch (targetKind)
    {
        case ValueKind::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::charValue:
        {
            if (cast)
            {
                return new CharValue(static_cast<char>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::shortValue:
        {
            if (cast)
            {
                return new ShortValue(static_cast<int16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::intValue:
        {
            if (cast)
            {
                return new IntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::uintValue:
        {
            if (cast)
            {
                return new UIntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::longValue:
        {
            return new LongValue(value);
        }
        case ValueKind::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(static_cast<uint64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::floatValue:
        {
            return new FloatValue(static_cast<float>(value));
        }
        case ValueKind::doubleValue:
        {
            return new DoubleValue(static_cast<double>(value));
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

ULongValue::ULongValue() : value(0)
{
}

ULongValue::ULongValue(uint64_t value_) : value(value_)
{
}

Value* ULongValue::Clone() const
{
    return new ULongValue(value);
}

void ULongValue::Write(SymbolWriter& writer)
{
    writer.AsMachineWriter().Put(value);
}

void ULongValue::Read(SymbolReader& reader)
{
    value = reader.GetULong();
}

Value* ULongValue::As(ValueKind targetKind, bool cast, const Span& span) const
{
    switch (targetKind)
    {
        case ValueKind::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::charValue:
        {
            if (cast)
            {
                return new CharValue(static_cast<char>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::shortValue:
        {
            if (cast)
            {
                return new ShortValue(static_cast<int16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::intValue:
        {
            if (cast)
            {
                return new IntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::uintValue:
        {
            if (cast)
            {
                return new UIntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::longValue:
        {
            if (cast)
            {
                return new LongValue(static_cast<int64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::ulongValue:
        {
            return new ULongValue(value);
        }
        case ValueKind::floatValue:
        {
            return new FloatValue(static_cast<float>(value));
        }
        case ValueKind::doubleValue:
        {
            return new DoubleValue(static_cast<double>(value));
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

FloatValue::FloatValue() : value(0)
{
}

FloatValue::FloatValue(float value_) : value(value_)
{
}

Value* FloatValue::Clone() const
{
    return new FloatValue(value);
}

void FloatValue::Write(SymbolWriter& writer)
{
    writer.AsMachineWriter().Put(value);
}

void FloatValue::Read(SymbolReader& reader)
{
    value = reader.GetFloat();
}

Value* FloatValue::As(ValueKind targetKind, bool cast, const Span& span) const 
{
    switch (targetKind)
    {
        case ValueKind::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::charValue:
        {
            if (cast)
            {
                return new CharValue(static_cast<char>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::shortValue:
        {
            if (cast)
            {
                return new ShortValue(static_cast<int16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::intValue:
        {
            if (cast)
            {
                return new IntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::uintValue:
        {
            if (cast)
            {
                return new UIntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::longValue:
        {
            if (cast)
            {
                return new LongValue(static_cast<int64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(static_cast<uint64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::floatValue:
        {
            return new FloatValue(value);
        }
        case ValueKind::doubleValue:
        {
            return new DoubleValue(value);
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

DoubleValue::DoubleValue() : value(0.0)
{
}

DoubleValue::DoubleValue(double value_) : value(value_)
{
}

Value* DoubleValue::Clone() const
{
    return new DoubleValue(value);
}

void DoubleValue::Write(SymbolWriter& writer)
{
    writer.AsMachineWriter().Put(value);
}

void DoubleValue::Read(SymbolReader& reader)
{
    value = reader.GetDouble();
}

Value* DoubleValue::As(ValueKind targetKind, bool cast, const Span& span) const
{
    switch (targetKind)
    {
        case ValueKind::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::charValue:
        {
            if (cast)
            {
                return new CharValue(static_cast<char>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::shortValue:
        {
            if (cast)
            {
                return new ShortValue(static_cast<int16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::intValue:
        {
            if (cast)
            {
                return new IntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::uintValue:
        {
            if (cast)
            {
                return new UIntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::longValue:
        {
            if (cast)
            {
                return new LongValue(static_cast<int64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(static_cast<uint64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::floatValue:
        {
            if (cast)
            {
                return new FloatValue(static_cast<float>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueKindStr(GetValueKind()) + " to " + ValueKindStr(targetKind) + " without a cast", span);
            }
        }
        case ValueKind::doubleValue:
        {
            return new DoubleValue(value);
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

ValueCreator::~ValueCreator()
{
}

std::unique_ptr<ValueFactory> ValueFactory::instance;

void ValueFactory::Init()
{
    instance.reset(new ValueFactory());
}

void ValueFactory::Done()
{
    instance.reset();
}

ValueFactory& ValueFactory::Instance()
{
    Assert(instance, "value factory instance not set");
    return *instance;
}

ValueFactory::ValueFactory()
{
    creators.resize(int(ValueKind::max));
}

void ValueFactory::Register(ValueKind valueKind, ValueCreator* creator)
{
    creators[int(valueKind)].reset(creator);
}

Value* ValueFactory::CreateValue(ValueKind kind)
{
    ValueCreator* creator = creators[int(kind)].get();
    if (creator)
    {
        return creator->CreateValue();
    }
    else
    {
        throw std::runtime_error("no creator for value kind '" + ValueKindStr(kind) + "'");
    }
}

template<typename T>
class ConcreteValueCreator : public ValueCreator
{
public:
    Value* CreateValue() const override
    {
        return new T();
    }
};

void ValueInit()
{
    ValueFactory::Init();
    ValueFactory::Instance().Register(ValueKind::boolValue, new ConcreteValueCreator<BoolValue>());
    ValueFactory::Instance().Register(ValueKind::charValue, new ConcreteValueCreator<CharValue>());
    ValueFactory::Instance().Register(ValueKind::sbyteValue, new ConcreteValueCreator<SByteValue>());
    ValueFactory::Instance().Register(ValueKind::byteValue, new ConcreteValueCreator<ByteValue>());
    ValueFactory::Instance().Register(ValueKind::shortValue, new ConcreteValueCreator<ShortValue>());
    ValueFactory::Instance().Register(ValueKind::ushortValue, new ConcreteValueCreator<UShortValue>());
    ValueFactory::Instance().Register(ValueKind::intValue, new ConcreteValueCreator<IntValue>());
    ValueFactory::Instance().Register(ValueKind::uintValue, new ConcreteValueCreator<UIntValue>());
    ValueFactory::Instance().Register(ValueKind::longValue, new ConcreteValueCreator<LongValue>());
    ValueFactory::Instance().Register(ValueKind::ulongValue, new ConcreteValueCreator<ULongValue>());
    ValueFactory::Instance().Register(ValueKind::floatValue, new ConcreteValueCreator<FloatValue>());
    ValueFactory::Instance().Register(ValueKind::doubleValue, new ConcreteValueCreator<DoubleValue>());
}

void ValueDone()
{
    ValueFactory::Done();
}

} } // namespace cminor::symbols
