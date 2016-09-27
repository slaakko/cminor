// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_OBJECT_INCLUDED
#define CMINOR_MACHINE_OBJECT_INCLUDED
#include <stdint.h>
#include <unordered_map>

namespace cminor { namespace machine {

class ObjectHandle
{
public:
    ObjectHandle(uint64_t value_) : value(value_) {}
    uint64_t Value() const { return value; }
private:
    uint64_t value;
};

inline bool operator==(ObjectHandle left, ObjectHandle right)
{
    return left.Value() == right.Value();
}

struct ObjectHandleHash
{
    size_t operator()(ObjectHandle handle) const { return static_cast<size_t>(handle.Value()); }
};

class ObjectPtr
{
public:
    ObjectPtr(void* value_) : value(value_) {}
    void* Value() const { return value; }
private:
    void* value;
};

class FieldOffset
{
public:
    FieldOffset(uint32_t value_) : value(value_) {}
    uint32_t Value() const { return value; }
private:
    uint32_t value;
};

class ObjectLayout
{
public:
private:
    std::vector<FieldOffset> fieldOffsets;
};

class Object
{
public:
    ObjectPtr Ptr() const { return ptr; }
private:
    ObjectPtr ptr;
};

class ObjectPool
{
public:
    ObjectPtr GetObjectPtr(ObjectHandle handle) const;
private:
    std::unordered_map<ObjectHandle, Object, ObjectHandleHash> objects;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_OBJECT_INCLUDED
