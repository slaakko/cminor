// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_OBJECT_INCLUDED
#define CMINOR_MACHINE_OBJECT_INCLUDED
#include <cminor/machine/Error.hpp>
#include <stdint.h>
#include <atomic>
#include <unordered_map>
#include <unordered_set>

namespace cminor { namespace machine {

class Machine;
class Thread;
class Writer;
class Reader;
class Arena;
class Function;
class ClassData;
class Type;
class ObjectType;
class ManagedMemoryPool;

enum class ArenaId : uint8_t
{
    notGCMem = 0, gen1Arena = 1, gen2Arena = 2
};

enum class ValueType : uint8_t
{
    none = 0, boolType = 1, sbyteType = 2, byteType = 3, shortType = 4, ushortType = 5, intType = 6, uintType = 7, longType = 8, ulongType = 9, floatType = 10, doubleType = 11, charType = 12,
    memPtr = 'M', stringLiteral = 'S', allocationHandle = 'H', objectReference = 'O', functionPtr = 'F', classDataPtr = 'C', typePtr = 'T'
};

std::string ValueTypeStr(ValueType type);

class IntegralValue
{
public:
    IntegralValue() : value(0), type(ValueType::none) {}
    IntegralValue(uint64_t value_, ValueType type_) : value(value_), type(type_) {}
    IntegralValue(uint8_t* value_) : memPtr(value_), type(ValueType::memPtr) {}
    IntegralValue(const char32_t* value_) : strValue(value_), type(ValueType::stringLiteral) {}
    IntegralValue(Function* value_) : funPtr(value_), type(ValueType::functionPtr) {}
    IntegralValue(ClassData* value_) : classDataPtr(value_), type(ValueType::classDataPtr) {}
    IntegralValue(Type* value_) : typePtr(value_), type(ValueType::typePtr) {}
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
    Function* AsFunctionPtr() const { return funPtr; }
    ClassData* AsClassDataPtr() const { return classDataPtr; }
    Type* AsTypePtr() const { return typePtr; }
    void Write(Writer& writer);
    void Read(Reader& reader);
private:
    union { uint64_t value; uint8_t* memPtr; const char32_t* strValue; Function* funPtr; ClassData* classDataPtr; Type* typePtr;  };
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

class AllocationHandle : public IntegralValue
{
public:
    AllocationHandle() : IntegralValue(0, ValueType::allocationHandle) {}
    AllocationHandle(uint64_t value_, ValueType type_) : IntegralValue(value_, type_) {}
    AllocationHandle(uint64_t value_) : IntegralValue(value_, ValueType::allocationHandle) {}
};

struct AllocationHandleHash
{
    size_t operator()(AllocationHandle handle) const { return static_cast<size_t>(handle.Value()); }
};

inline bool operator==(AllocationHandle left, AllocationHandle right)
{
    return left.Value() == right.Value();
}

class ObjectReference : public AllocationHandle
{
public:
    ObjectReference() : AllocationHandle(0, ValueType::objectReference) {}
    ObjectReference(uint64_t value_) : AllocationHandle(value_, ValueType::objectReference) {}
    bool IsNull() const { return Value() == 0; }
};

struct ObjectReferenceHash
{
    size_t operator()(AllocationHandle handle) const { return static_cast<size_t>(handle.Value()); }
};

inline bool operator==(ObjectReference left, ObjectReference right)
{
    return left.Value() == right.Value();
}

class MemPtr
{
public:
    MemPtr() : value(nullptr) {}
    MemPtr(void* value_) : value(value_) {}
    MemPtr(const char32_t* strValue_) : strValue(strValue_) {}
    void* Value() const { return value; }
    const char32_t* StrValue() const { return strValue; }
private:
    union { void* value; const char32_t* strValue; };
};

inline bool operator==(MemPtr left, MemPtr right)
{
    return left.Value() == right.Value();
}

inline bool operator<(MemPtr left, MemPtr right)
{
    return left.Value() < right.Value();
}

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

class ManagedAllocation
{
public:
    ManagedAllocation(ArenaId arenaId_, MemPtr memPtr_, uint64_t size_);
    virtual ~ManagedAllocation();
    ArenaId GetArenaId() const { return arenaId; }
    void SetArenaId(ArenaId arenaId_) { arenaId = arenaId_; }
    MemPtr GetMemPtr() const { return memPtr; }
    void SetMemPtr(MemPtr newMemPtr) { memPtr = newMemPtr; }
    uint64_t Size() const { return size; }
    void SetSize(uint64_t size_) { size = size_; }
    virtual void Set(ManagedMemoryPool* pool) = 0;
private:
    ArenaId arenaId;
    MemPtr memPtr;
    uint64_t size;
};

class Object : public ManagedAllocation
{
public:
    Object(ObjectReference reference_, ArenaId arenaId_, MemPtr memPtr_, ObjectType* type_, uint64_t size_);
    ObjectReference Reference() const { return reference; }
    ObjectType* GetType() const { return type; }
    IntegralValue GetField(int index) const;
    void SetField(IntegralValue fieldValue, int index);
    int32_t FieldCount() const;
    bool IsLive() const { return GetFlag(ObjectFlags::live); }
    void SetLive() { SetFlag(ObjectFlags::live); }
    void ResetLive() { ResetFlag(ObjectFlags::live); }
    void MarkLiveObjects(std::unordered_set<ObjectReference, ObjectReferenceHash>& checked, ManagedMemoryPool& managedMemoryPool);
    void Set(ManagedMemoryPool* pool) override;
private:
    ObjectReference reference;
    ObjectType* type;
    ObjectFlags flags;
    bool GetFlag(ObjectFlags flag) const { return (flags & flag) != ObjectFlags::none; }
    void SetFlag(ObjectFlags flag) { flags = flags | flag; }
    void ResetFlag(ObjectFlags flag) { flags = flags & (~flag); }
};

class ArrayElements : public ManagedAllocation
{
public:
    ArrayElements(AllocationHandle handle_, ArenaId arenaId_, MemPtr memPtr_, Type* elementType_, int32_t numElements_, uint64_t size_);
    AllocationHandle Handle() const { return handle; }
    Type* GetElementType() const { return elementType; }
    IntegralValue GetElement(int32_t index) const;
    void SetElement(IntegralValue elementValue, int32_t index);
    int32_t NumElements() const { return numElements; }
    void Set(ManagedMemoryPool* pool) override;
private:
    AllocationHandle handle;
    Type* elementType;
    int32_t numElements;
};

class StringCharacters : public ManagedAllocation
{
public:
    StringCharacters(AllocationHandle handle_, ArenaId arenaId_, MemPtr memPtr_, int32_t numChars_, uint64_t size_);
    AllocationHandle Handle() const { return handle; }
    IntegralValue GetChar(int32_t index) const;
    int32_t NumChars() const { return numChars; }
    void Set(ManagedMemoryPool* pool) override;
private:
    AllocationHandle handle;
    int32_t numChars;
};

class ManagedMemoryPool
{
public:
    ManagedMemoryPool(Machine& machine_);
    ObjectReference CreateObject(Thread& thread, ObjectType* type);
    ObjectReference CopyObject(ObjectReference from);
    void DestroyObject(ObjectReference reference);
    Object& GetObject(ObjectReference reference);
    IntegralValue GetField(ObjectReference reference, int32_t fieldIndex);
    void SetField(ObjectReference reference, int32_t fieldIndex, IntegralValue fieldValue);
    AllocationHandle CreateStringCharsFromLiteral(const char32_t* strLit, uint32_t len);
    std::pair<AllocationHandle, int32_t> CreateStringCharsFromCharArray(Thread& thread, ObjectReference charArray);
    ObjectReference CreateString(Thread& thread, const utf32_string& s);
    IntegralValue GetStringChar(ObjectReference str, int32_t index);
    std::string GetUtf8String(ObjectReference str);
    std::vector<uint8_t> GetBytes(ObjectReference arr);
    void SetBytes(ObjectReference arr, const std::vector<uint8_t>& bytes, int32_t count);
    void AllocateArrayElements(Thread& thread, ObjectReference arr, Type* elementType, int32_t length);
    IntegralValue GetArrayElement(ObjectReference reference, int32_t index);
    void SetArrayElement(ObjectReference reference, int32_t index, IntegralValue elementValue);
    int32_t GetNumArrayElements(ObjectReference arr);
    void ResetObjectsLiveFlag();
    void MoveLiveObjectsToArena(ArenaId fromArenaId, Arena& toArena);
    MemPtr GetMemPtr(AllocationHandle handle) const;
    void Set(Object* object_) { object = object_; }
    void Set(ArrayElements* arrayElements_) { arrayElements = arrayElements_; }
    void Set(StringCharacters* stringCharacters_) { stringCharacters = stringCharacters_;  }
private:
    Machine& machine;
    Object* object;
    ArrayElements* arrayElements;
    StringCharacters* stringCharacters;
    std::unordered_map<AllocationHandle, std::unique_ptr<ManagedAllocation>, AllocationHandleHash> allocations;
    std::atomic_uint64_t nextReferenceValue;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_OBJECT_INCLUDED
