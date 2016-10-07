// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_OBJECT_INCLUDED
#define CMINOR_MACHINE_OBJECT_INCLUDED
#include <cminor/machine/Error.hpp>
#include <stdint.h>
#include <unordered_map>
#include <unordered_set>

namespace cminor { namespace machine {

class Machine;
class Thread;
class Writer;
class Reader;
class Arena;
class Type;
class ObjectType;
class ObjectPool;

enum class ArenaId : uint8_t
{
    notGCMem = 0, gen1Arena = 1, gen2Arena = 2
};

enum class ValueType : uint8_t
{
    none = 0, boolType = 1, sbyteType = 2, byteType = 3, shortType = 4, ushortType = 5, intType = 6, uintType = 7, longType = 8, ulongType = 9, floatType = 10, doubleType = 11, charType = 12,
    memPtr = 'M', stringLiteral = 'S', objectReference = 'O'
};

std::string ValueTypeStr(ValueType type);

class IntegralValue
{
public:
    IntegralValue() : value(0), type(ValueType::none) {}
    IntegralValue(uint64_t value_, ValueType type_) : value(value_), type(type_) {}
    IntegralValue(uint8_t* value_) : memPtr(value_), type(ValueType::memPtr) {}
    IntegralValue(const char32_t* value_) : strValue(value_), type(ValueType::stringLiteral) {}
    uint64_t Value() const { return value; }
    ValueType GetType() const { return type; }
    bool AsBool() const { return static_cast<bool>(value); }
    int8_t AsSByte() const { return static_cast<int8_t>(value); }
    uint8_t AsByte() const { return static_cast<uint8_t>(value); }
    int16_t AsShort() const { return static_cast<int16_t>(value); }
    uint16_t AsUShort() const { return static_cast<uint16_t>(value); }
    int32_t AsInt() const { return static_cast<int32_t>(value); }
    uint32_t AsUInt() const { return static_cast<uint32_t>(value); }
    int64_t AsLong() const { return static_cast<int64_t>(value); }
    uint64_t AsULong() const { return static_cast<uint64_t>(value); }
    float AsFloat() const { return static_cast<float>(value); }
    double AsDouble() const { return static_cast<double>(value); }
    char32_t AsChar() const { return static_cast<char32_t>(value); }
    uint8_t* AsMemPtr() const { return memPtr; }
    const char32_t* AsStringLiteral() const { return strValue; }
    void Write(Writer& writer);
    void Read(Reader& reader);
private:
    union { uint64_t value; uint8_t* memPtr; const char32_t* strValue; };
    ValueType type;
};

inline bool operator==(IntegralValue left, IntegralValue right)
{
    return left.GetType() == right.GetType() && left.Value() == right.Value();
}

struct IntegralValueHash
{
    size_t operator()(IntegralValue value) const
    {
        return size_t(value.GetType()) * 1099511628211 + size_t(value.Value());
    }
};

class ObjectReference : public IntegralValue
{
public:
    ObjectReference() : IntegralValue(0, ValueType::objectReference) {}
    ObjectReference(uint64_t value_) : IntegralValue(value_, ValueType::objectReference) {}
    bool IsNull() const { return Value() == 0; }
};

inline bool operator==(ObjectReference left, ObjectReference right)
{
    return left.Value() == right.Value();
}

struct ObjectReferenceHash
{
    size_t operator()(ObjectReference reference) const { return static_cast<size_t>(reference.Value()); }
};

class ObjectMemPtr
{
public:
    ObjectMemPtr(uint8_t* value_) : value(value_) {}
    uint8_t* Value() const { return value; }
private:
    uint8_t* value;
};

uint64_t ValueSize(ValueType type);

enum class ObjectFlags : uint8_t
{
    none = 0,
    live = 1 << 0
};

inline ObjectFlags operator|(ObjectFlags left, ObjectFlags right)
{
    return ObjectFlags(uint8_t(left) | uint8_t(right));
}

inline ObjectFlags operator&(ObjectFlags left, ObjectFlags right)
{
    return ObjectFlags(uint8_t(left) & uint8_t(right));
}

inline ObjectFlags operator~(ObjectFlags flag)
{
    return ObjectFlags(~uint8_t(flag));
}

class Object
{
public:
    Object(ObjectReference reference_, ArenaId arenaId_, ObjectMemPtr memPtr_, ObjectType* type_);
    ObjectReference Reference() const { return reference; }
    ArenaId GetArenaId() const { return arenaId; }
    void SetArenaId(ArenaId arenaId_) { arenaId = arenaId_; }
    ObjectMemPtr MemPtr() const { return memPtr; }
    void SetMemPtr(ObjectMemPtr newMemPtr) { memPtr = newMemPtr; }
    IntegralValue GetField(int index) const;
    void SetField(IntegralValue fieldValue, int index);
    int32_t FieldCount() const;
    bool IsLive() const { return GetFlag(ObjectFlags::live); }
    void SetLive() { SetFlag(ObjectFlags::live); }
    void ResetLive() { ResetFlag(ObjectFlags::live); }
    uint64_t Size() const;
    void MarkLiveObjects(std::unordered_set<ObjectReference, ObjectReferenceHash>& checked, ObjectPool& objectBool);
private:
    ObjectReference reference;
    ArenaId arenaId;
    ObjectMemPtr memPtr;
    ObjectType* type;
    ObjectFlags flags;
    bool GetFlag(ObjectFlags flag) const { return (flags & flag) != ObjectFlags::none; }
    void SetFlag(ObjectFlags flag) { flags = flags | flag; }
    void ResetFlag(ObjectFlags flag) { flags = flags & (~flag); }
};

class ObjectPool
{
public:
    ObjectPool(Machine& machine_);
    ObjectReference CreateObject(Thread& thread, ObjectType* type);
    ObjectReference CreateString(Thread& thread, ArenaId arenaId, IntegralValue stringValue);
    void DestroyObject(ObjectReference reference);
    Object& GetObject(ObjectReference reference);
    IntegralValue GetField(ObjectReference reference, int32_t fieldIndex);
    void SetField(ObjectReference reference, int32_t fieldIndex, IntegralValue fieldValue);
    ObjectMemPtr GetObjectMemPtr(ObjectReference reference);
    void ResetObjectsLiveFlag();
    void MoveLiveObjectsToArena(ArenaId fromArenaId, Arena& toArena);
private:
    Machine& machine;
    std::unordered_map<ObjectReference, Object, ObjectReferenceHash> objects;
    uint64_t nextReferenceValue;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_OBJECT_INCLUDED
