// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_TYPE_INCLUDED
#define CMINOR_MACHINE_TYPE_INCLUDED
#include <cminor/machine/MachineApi.hpp>
#include <cminor/machine/Constant.hpp>
#include <algorithm>

namespace cminor { namespace machine {

class MACHINE_API FunctionType
{
public:
    FunctionType();
    void SetReturnType(ValueType returnType_) { returnType = returnType_; }
    ValueType ReturnType() const { return returnType; }
    void AddParamType(ValueType paramType);
    const std::vector<ValueType>& ParameterTypes() const { return parameterTypes; }
    void Write(Writer& writer);
    void Read(Reader& reader);
private:
    ValueType returnType;
    std::vector<ValueType> parameterTypes;
};

class MACHINE_API FieldOffset
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

class MACHINE_API Field
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

class MACHINE_API Layout
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

class MACHINE_API Type
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

class MACHINE_API BasicType : public Type
{
};

class MACHINE_API SByte : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::sbyteType; }
};

class MACHINE_API Byte : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::byteType; }
};

class MACHINE_API Short : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::shortType; }
};

class MACHINE_API UShort : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::ushortType; }
};

class MACHINE_API Int : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::intType; }
};

class MACHINE_API UInt : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::uintType; }
};

class MACHINE_API Long : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::longType; }
};

class MACHINE_API ULong : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::ulongType; }
};

class MACHINE_API Float : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::floatType; }
};

class MACHINE_API Double : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::doubleType; }
};

class MACHINE_API Char : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::charType; }
};

class MACHINE_API Bool : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::boolType; }
};

class MACHINE_API Void : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::none; }
};

class MACHINE_API Null : public BasicType
{
public:
    ValueType GetValueType() const override { return ValueType::ulongType; }
};

class MACHINE_API DelegateType : public Type
{
public:
    ValueType GetValueType() const override { return ValueType::functionPtr; }
};

class MACHINE_API RefType : public Type
{
public:
    ValueType GetValueType() const override { return ValueType::variableReference; };
};

class MACHINE_API ObjectType : public Type
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
    MACHINE_API static void Init();
    MACHINE_API static void Done();
    MACHINE_API static Type* GetType(StringPtr fullTypeName);
    MACHINE_API static void SetType(Type* type);
};

MACHINE_API ObjectType* GetBoxedType(ValueType valueType);

MACHINE_API void TypeInit();
MACHINE_API void TypeDone();

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_TYPE_INCLUDED
