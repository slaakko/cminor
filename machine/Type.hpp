// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_TYPE_INCLUDED
#define CMINOR_MACHINE_TYPE_INCLUDED
#include <cminor/machine/Object.hpp>

namespace cminor { namespace machine {

class FieldOffset
{
public:
    FieldOffset(uint64_t value_) : value(value_) {}
    uint64_t Value() const { return value; }
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
    Field(ValueType fieldType_, FieldOffset offset_) : fieldType(fieldType_), offset(offset_) {}
    ValueType GetType() const { return fieldType; }
    FieldOffset Offset() const { return offset; }
private:
    ValueType fieldType;
    FieldOffset offset;
};

class Layout
{
public:
    Layout();
    void AddField(ValueType fieldType);
    Field GetField(int32_t index) const { Assert(index >= 0 && index < fields.size(), "invalid field index"); return fields[index]; }
    int32_t FieldCount() const { return int32_t(fields.size()); }
    uint64_t Size() const { return size; }
private:
    std::vector<Field> fields;
    uint64_t size;
};

class Type
{
public:
    virtual ~Type();
    virtual uint64_t ObjectSize() const = 0;
};

class ObjectType : public Type
{
public:
    void AddField(ValueType fieldType) { objectLayout.AddField(fieldType); }
    Field GetField(int32_t index) const { return objectLayout.GetField(index); }
    void AddStaticField(ValueType fieldType) { staticLayout.AddField(fieldType); }
    Field GetStaticField(int32_t index) const { return staticLayout.GetField(index); }
    int32_t FieldCount() const { return objectLayout.FieldCount(); }
    int32_t StaticFieldCount() const { return staticLayout.FieldCount(); }
    uint64_t ObjectSize() const override { return objectLayout.Size(); }
private:
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
