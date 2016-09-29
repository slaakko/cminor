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

class ObjectLayout
{
public:
    ObjectLayout();
    void AddField(ValueType fieldType);
    Field GetField(int32_t index) const { assert(index >= 0 && index < fields.size(), "invalid field index"); return fields[index]; }
    uint64_t ObjectSize() const { return objectSize; }
private:
    std::vector<Field> fields;
    uint64_t objectSize;
};

class Type
{
public:
    Type(const std::string& fullName_);
    const std::string& FullName() const { return fullName; }
    void AddField(ValueType fieldType) { layout.AddField(fieldType); }
    Field GetField(int32_t index) const { return layout.GetField(index); }
    uint64_t ObjectSize() const { return layout.ObjectSize(); }
private:
    std::string fullName;
    ObjectLayout layout;
};

class TypeRepository
{
public:
    Type* CreateType(const std::string& fullName);
    Type* GetType(const std::string& fullName) const;
private:
    std::unordered_map<std::string, std::unique_ptr<Type>> typeMap;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_TYPE_INCLUDED
