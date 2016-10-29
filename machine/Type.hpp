// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_TYPE_INCLUDED
#define CMINOR_MACHINE_TYPE_INCLUDED
#include <cminor/machine/Constant.hpp>
#include <algorithm>

namespace cminor { namespace machine {

class FieldOffset
{
public:
    FieldOffset() : value(0) {}
    FieldOffset(uint64_t value_) : value(value_) {}
    uint64_t Value() const { return value; }
    void Write(Writer& writer);
    void Read(Reader& reader);
private:
    uint64_t value;
};

inline ObjectMemPtr operator+(ObjectMemPtr memPtr, FieldOffset offset)
{
    return ObjectMemPtr(memPtr.Value() + offset.Value());
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
    void SetNameConstant(Constant name_) { name = name_; }
    void Write(Writer& writer);
    void Read(Reader& reader);
private:
    Constant name;
};

class ObjectType : public Type
{
public:
    ObjectType();
    const std::vector<Field>& Fields() const { return objectLayout.Fields(); }
    void AddFields(const std::vector<Field>& fields_) { objectLayout.AddFields(fields_); }
    void AddField(ValueType fieldType) { objectLayout.AddField(fieldType); }
    Field GetField(int32_t index) const { return objectLayout.GetField(index); }
    void AddStaticField(ValueType fieldType) { staticLayout.AddField(fieldType); }
    Field GetStaticField(int32_t index) const { return staticLayout.GetField(index); }
    int32_t FieldCount() const { return objectLayout.FieldCount(); }
    int32_t StaticFieldCount() const { return staticLayout.FieldCount(); }
    uint64_t ObjectSize() const { return std::max(uint64_t(1), objectLayout.Size()); }
    uint64_t Id() const { return id; }
    void SetId(uint64_t id_) { id = id_; }
    void Write(Writer& writer);
    void Read(Reader& reader);
private:
    uint64_t id;
    Layout objectLayout;
    Layout staticLayout;
};

class ArrayType : public ObjectType
{
public:
    ArrayType(std::unique_ptr<Type>&& elementType_);
private:
    std::unique_ptr<Type> elementType;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_TYPE_INCLUDED
