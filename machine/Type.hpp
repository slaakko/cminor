// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_TYPE_INCLUDED
#define CMINOR_MACHINE_TYPE_INCLUDED
#include <cminor/machine/Constant.hpp>
#include <algorithm>

namespace cminor { namespace machine {

inline MemPtr ElementPtr(MemPtr memPtr, ValueType valueType, int32_t index)
{
    return MemPtr(static_cast<uint8_t*>(memPtr.Value()) + ValueSize(valueType) * index);
}

class FieldOffset
{
public:
    FieldOffset() : value(0) {}
    FieldOffset(uint32_t value_) : value(value_) {}
    uint64_t Value() const { return value; }
    void Write(Writer& writer);
    void Read(Reader& reader);
private:
    uint32_t value;
};

inline MemPtr operator+(MemPtr memPtr, FieldOffset offset)
{
    return MemPtr(static_cast<uint8_t*>(memPtr.Value()) + offset.Value());
}

class Field
{
public:
    Field();
    Field(ValueType fieldType_, FieldOffset offset_) : fieldType(fieldType_), offset(offset_) {}
    ValueType GetType() const { return fieldType; }
    FieldOffset Offset() const { return offset; }
    void Write(Writer& writer);
    void Read(Reader& reader);
private:
    ValueType fieldType;
    FieldOffset offset;
};

class Layout
{
public:
    Layout();
    const std::vector<Field>& Fields() const { return fields; }
    void AddFields(const std::vector<Field>& fields_);
    void AddField(ValueType fieldType);
    Field GetField(int32_t index) const { Assert(index >= 0 && index < fields.size(), "invalid field index"); return fields[index]; }
    int32_t FieldCount() const { return int32_t(fields.size()); }
    uint64_t Size() const { return size; }
    void Write(Writer& writer);
    void Read(Reader& reader);
private:
    std::vector<Field> fields;
    uint64_t size;
};

class Type
{
public:
    Type();
    virtual ~Type();
    StringPtr Name() const { return StringPtr(name.Value().AsStringLiteral()); }
    Constant NameConstant() const { return name; }
    void SetNameConstant(Constant name_) { name = name_; }
    void Write(Writer& writer);
    void Read(Reader& reader);
    virtual ValueType GetValueType() const = 0;
private:
    Constant name;
};

class BasicType : public Type
{
};

class SByte : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::sbyteType; }
};

class Byte : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::byteType; }
};

class Short : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::shortType; }
};

class UShort : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::ushortType; }
};

class Int : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::intType; }
};

class UInt : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::uintType; }
};

class Long : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::longType; }
};

class ULong : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::ulongType; }
};

class Float : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::floatType; }
};

class Double : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::doubleType; }
};

class Char : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::charType; }
};

class Bool : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::boolType; }
};

class Void : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::none; }
};

class Null : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::ulongType; }
};

class DelegateType : public Type
{
public:
    ValueType GetValueType() const override { return ValueType::functionPtr; }
};

class RefType : public Type
{
public:
    ValueType GetValueType() const override { return ValueType::variableReference; };
};

class ObjectType : public Type
{
public:
    ObjectType();
    const std::vector<Field>& Fields() const { return objectLayout.Fields(); }
    void AddFields(const std::vector<Field>& fields_) { objectLayout.AddFields(fields_); }
    void AddField(ValueType fieldType) { objectLayout.AddField(fieldType); }
    Field GetField(int32_t index) const { return objectLayout.GetField(index); }
    int32_t FieldCount() const { return objectLayout.FieldCount(); }
    uint64_t ObjectSize() const { return std::max(uint64_t(1), objectLayout.Size()); }
    uint64_t Id() const { return id; }
    void SetId(uint64_t id_) { id = id_; }
    void Write(Writer& writer);
    void Read(Reader& reader);
    ValueType GetValueType() const override { return ValueType::objectReference; }
private:
    uint64_t id;
    Layout objectLayout;
};

class TypeTable
{
public:
    static void Init();
    static void Done();
    static TypeTable& Instance();
    Type* GetType(StringPtr fullTypeName);
    void SetType(Type* type);
private:
    static std::unique_ptr<TypeTable> instance;
    TypeTable();
    std::unordered_map<StringPtr, Type*, StringPtrHash> typeMap;
};

ObjectType* GetBoxedType(ValueType valueType);

void TypeInit();
void TypeDone();

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_TYPE_INCLUDED
