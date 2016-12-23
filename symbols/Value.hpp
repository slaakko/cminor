// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_VALUE_INCLUDED
#define CMINOR_SYMBOLS_VALUE_INCLUDED
#include <cminor/symbols/Symbol.hpp>

namespace cminor { namespace symbols {

enum class ValueKind : uint8_t
{
    none, boolValue, charValue, sbyteValue, byteValue, shortValue, ushortValue, intValue, uintValue, longValue, ulongValue, floatValue, doubleValue, max
};

std::string ValueKindStr(ValueKind valueKind);
ValueKind GetCommonType(ValueKind left, ValueKind right);
ValueKind GetValueKindFor(SymbolType symbolType, const Span& span);

class Value
{
public:
    virtual ~Value();
    virtual ValueKind GetValueKind() const = 0;
    virtual Value* Clone() const = 0;
    virtual void Write(SymbolWriter& writer) = 0;
    virtual void Read(SymbolReader& reader) = 0;
    virtual Value* As(ValueKind targetKind, bool cast, const Span& span, bool dontThrow) const = 0;
    virtual bool IsComplete() const { return true; }
    virtual IntegralValue GetIntegralValue() const = 0;
};

class BoolValue : public Value
{
public:
    typedef bool OperandType;
    BoolValue();
    BoolValue(bool value_);
    ValueKind GetValueKind() const override { return ValueKind::boolValue; }
    Value* Clone() const override;
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    Value* As(ValueKind targetKind, bool cast, const Span& span, bool dontThrow) const override;
    bool Value() const { return value; }
    IntegralValue GetIntegralValue() const override { return IntegralValue(value, ValueType::boolType); }
private:
    bool value;
};

class CharValue : public Value
{
public:
    typedef char32_t OperandType;
    CharValue();
    CharValue(char32_t value_);
    ValueKind GetValueKind() const override { return ValueKind::charValue; }
    Value* Clone() const override;
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    Value* As(ValueKind targetKind, bool cast, const Span& span, bool dontThrow) const override;
    char32_t Value() const { return value; }
    IntegralValue GetIntegralValue() const override { return IntegralValue(value, ValueType::charType); }
private:
    char32_t value;
};

class SByteValue : public Value
{
public:
    typedef int8_t OperandType;
    SByteValue();
    SByteValue(int8_t value_);
    ValueKind GetValueKind() const override { return ValueKind::sbyteValue; }
    Value* Clone() const override;
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    Value* As(ValueKind targetKind, bool cast, const Span& span, bool dontThrow) const override;
    int8_t Value() const { return value; }
    IntegralValue GetIntegralValue() const override { return IntegralValue(value, ValueType::sbyteType); }
private:
    int8_t value;
};

class ByteValue : public Value
{
public:
    typedef uint8_t OperandType;
    ByteValue();
    ByteValue(uint8_t value_);
    ValueKind GetValueKind() const override { return ValueKind::byteValue; }
    Value* Clone() const override;
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    Value* As(ValueKind targetKind, bool cast, const Span& span, bool dontThrow) const override;
    uint8_t Value() const { return value; }
    IntegralValue GetIntegralValue() const override { return IntegralValue(value, ValueType::byteType); }
private:
    uint8_t value;
};

class ShortValue : public Value
{
public:
    typedef int16_t OperandType;
    ShortValue();
    ShortValue(int16_t value_);
    ValueKind GetValueKind() const override { return ValueKind::shortValue; }
    Value* Clone() const override;
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    Value* As(ValueKind targetKind, bool cast, const Span& span, bool dontThrow) const override;
    int16_t Value() const { return value; }
    IntegralValue GetIntegralValue() const override { return IntegralValue(value, ValueType::shortType); }
private:
    int16_t value;
};

class UShortValue : public Value
{
public:
    typedef uint16_t OperandType;
    UShortValue();
    UShortValue(uint16_t value_);
    ValueKind GetValueKind() const override { return ValueKind::ushortValue; }
    Value* Clone() const override;
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    Value* As(ValueKind targetKind, bool cast, const Span& span, bool dontThrow) const override;
    uint16_t Value() const { return value; }
    IntegralValue GetIntegralValue() const override { return IntegralValue(value, ValueType::ushortType); }
private:
    uint16_t value;
};

class IntValue : public Value
{
public:
    typedef int32_t OperandType;
    IntValue();
    IntValue(int32_t value_);
    ValueKind GetValueKind() const override { return ValueKind::intValue; }
    Value* Clone() const override;
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    Value* As(ValueKind targetKind, bool cast, const Span& span, bool dontThrow) const override;
    int32_t Value() const { return value; }
    IntegralValue GetIntegralValue() const override { return IntegralValue(value, ValueType::intType); }
private:
    int32_t value;
};

class UIntValue : public Value
{
public:
    typedef uint32_t OperandType;
    UIntValue();
    UIntValue(uint32_t value_);
    ValueKind GetValueKind() const override { return ValueKind::uintValue; }
    Value* Clone() const override;
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    Value* As(ValueKind targetKind, bool cast, const Span& span, bool dontThrow) const override;
    uint32_t Value() const { return value; }
    IntegralValue GetIntegralValue() const override { return IntegralValue(value, ValueType::uintType); }
private:
    uint32_t value;
};

class LongValue : public Value
{
public:
    typedef int64_t OperandType;
    LongValue();
    LongValue(int64_t value_);
    ValueKind GetValueKind() const override { return ValueKind::longValue; }
    Value* Clone() const override;
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    Value* As(ValueKind targetKind, bool cast, const Span& span, bool dontThrow) const override;
    int64_t Value() const { return value; }
    IntegralValue GetIntegralValue() const override { return IntegralValue(value, ValueType::longType); }
private:
    int64_t value;
};

class ULongValue : public Value
{
public:
    typedef uint64_t OperandType;
    ULongValue();
    ULongValue(uint64_t value_);
    ValueKind GetValueKind() const override { return ValueKind::ulongValue; }
    Value* Clone() const override;
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    Value* As(ValueKind targetKind, bool cast, const Span& span, bool dontThrow) const override;
    uint64_t Value() const { return value; }
    IntegralValue GetIntegralValue() const override { return IntegralValue(value, ValueType::ulongType); }
private:
    uint64_t value;
};

class FloatValue : public Value
{
public:
    typedef float OperandType;
    FloatValue();
    FloatValue(float value_);
    ValueKind GetValueKind() const override { return ValueKind::floatValue; }
    Value* Clone() const override;
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    Value* As(ValueKind targetKind, bool cast, const Span& span, bool dontThrow) const override;
    float Value() const { return value; }
    IntegralValue GetIntegralValue() const override { return IntegralValue(static_cast<uint64_t>(value), ValueType::floatType); }
private:
    float value;
};

class DoubleValue : public Value
{
public:
    typedef double OperandType;
    DoubleValue();
    DoubleValue(double value_);
    ValueKind GetValueKind() const override { return ValueKind::doubleValue; }
    Value* Clone() const override;
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    Value* As(ValueKind targetKind, bool cast, const Span& span, bool dontThrow) const override;
    double Value() const { return value; }
    IntegralValue GetIntegralValue() const override { return IntegralValue(static_cast<uint64_t>(value), ValueType::doubleType); }
private:
    double value;
};

class ValueCreator
{
public:
    virtual ~ValueCreator();
    virtual Value* CreateValue() const = 0;
};

class ValueFactory
{
public:
    static void Init();
    static void Done();
    static ValueFactory& Instance();
    void Register(ValueKind valueKind, ValueCreator* creator);
    Value* CreateValue(ValueKind kind);
private:
    static std::unique_ptr<ValueFactory> instance;
    ValueFactory();
    std::vector<std::unique_ptr<ValueCreator>> creators;
};

void ValueInit();
void ValueDone();

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_VALUE_INCLUDED
