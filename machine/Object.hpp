// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_OBJECT_INCLUDED
#define CMINOR_MACHINE_OBJECT_INCLUDED
#include <cminor/machine/MachineApi.hpp>
#include <cminor/machine/Error.hpp>
#include <cminor/util/CodeFormatter.hpp>
#include <stdint.h>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

namespace cminor { namespace machine {

using namespace cminor::util;

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
struct FunctionStackEntry;

enum class ArenaId : uint8_t
{
    notGCMem = 0, gen1Arena = 1, gen2Arena = 2
};

enum class ValueType : uint8_t
{
    none = 0, boolType = 1, sbyteType = 2, byteType = 3, shortType = 4, ushortType = 5, intType = 6, uintType = 7, longType = 8, ulongType = 9, floatType = 10, doubleType = 11, charType = 12,
    memPtr = 'M', stringLiteral = 'S', allocationHandle = 'H', objectReference = 'O', functionPtr = 'F', classDataPtr = 'C', typePtr = 'T', variableReference = 'V'
};

MACHINE_API std::string ValueTypeStr(ValueType type);

class MACHINE_API IntegralValue
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
    void Dump(CodeFormatter& formatter);
    std::string ValueStr();
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
    MemPtr() : value(nullptr), hashCode(0) {}
    MemPtr(void* value_) : value(value_), hashCode(0) {}
    MemPtr(const char32_t* strValue_) : strValue(strValue_), hashCode(0) {}
    void* Value() const { return value; }
    const char32_t* StrValue() const { return strValue; }
    uint64_t HashCode() const { return hashCode; }
    void SetHashCode(uint64_t hashCode_) { hashCode = hashCode_; }
private:
    union { void* value; const char32_t* strValue; };
    uint64_t hashCode;
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

enum class AllocationFlags : uint8_t
{
    none = 0,
    live = 1 << 0,
    pinned = 1 << 1
};

inline AllocationFlags operator|(AllocationFlags left, AllocationFlags right)
{
    return AllocationFlags(uint8_t(left) | uint8_t(right));
}

inline AllocationFlags operator&(AllocationFlags left, AllocationFlags right)
{
    return AllocationFlags(uint8_t(left) & uint8_t(right));
}

inline AllocationFlags operator~(AllocationFlags flag)
{
    return AllocationFlags(~uint8_t(flag));
}

class MACHINE_API ManagedAllocation
{
public:
    ManagedAllocation(AllocationHandle handle_, MemPtr memPtr_, int32_t segmentId_, uint64_t size_);
    virtual ~ManagedAllocation();
    AllocationHandle Handle() const { return handle; }
    int32_t SegmentId() const { return segmentId; }
    void SetSegmentId(int32_t segmentId_) { segmentId = segmentId_; }
    MemPtr GetMemPtr() const { return memPtr; }
    void SetMemPtr(MemPtr newMemPtr) { memPtr = newMemPtr; }
    uint64_t Size() const { return size; }
    void SetSize(uint64_t size_) { size = size_; }
    virtual void Set(ManagedMemoryPool* pool) = 0;
    bool IsLive() const { return GetFlag(AllocationFlags::live); }
    void SetLive() { SetFlag(AllocationFlags::live); }
    void ResetLive() { ResetFlag(AllocationFlags::live); }
    void Pin() { SetFlag(AllocationFlags::pinned); }
    void Unpin() { ResetFlag(AllocationFlags::pinned); }
    bool IsPinned() const { return GetFlag(AllocationFlags::pinned); }
    virtual void MarkLiveAllocations(std::unordered_set<AllocationHandle, AllocationHandleHash>& checked, ManagedMemoryPool& managedMemoryPool);
private:
    AllocationHandle handle;
    MemPtr memPtr;
    int32_t segmentId;
    uint64_t size;
    AllocationFlags flags;
    bool GetFlag(AllocationFlags flag) const { return (flags & flag) != AllocationFlags::none; }
    void SetFlag(AllocationFlags flag) { flags = flags | flag; }
    void ResetFlag(AllocationFlags flag) { flags = flags & (~flag); }
};

class MACHINE_API Object : public ManagedAllocation
{
public:
    Object(ObjectReference reference_, MemPtr memPtr_, int32_t segmentId_, ObjectType* type_, uint64_t size_);
    ObjectReference Reference() const { return reference; }
    ObjectType* GetType() const { return type; }
    IntegralValue GetField(int index) const;
    void SetField(IntegralValue fieldValue, int index);
    int32_t FieldCount() const;
    void Set(ManagedMemoryPool* pool) override;
    void MarkLiveAllocations(std::unordered_set<AllocationHandle, AllocationHandleHash>& checked, ManagedMemoryPool& managedMemoryPool) override;
private:
    ObjectReference reference;
    ObjectType* type;
};

class MACHINE_API ArrayElements : public ManagedAllocation
{
public:
    ArrayElements(AllocationHandle handle_, MemPtr memPtr_, int32_t segmentId_, Type* elementType_, int32_t numElements_, uint64_t size_);
    Type* GetElementType() const { return elementType; }
    IntegralValue GetElement(int32_t index) const;
    void SetElement(IntegralValue elementValue, int32_t index);
    int32_t NumElements() const { return numElements; }
    void Set(ManagedMemoryPool* pool) override;
    void MarkLiveAllocations(std::unordered_set<AllocationHandle, AllocationHandleHash>& checked, ManagedMemoryPool& managedMemoryPool) override;
private:
    Type* elementType;
    int32_t numElements;
};

class MACHINE_API StringCharacters : public ManagedAllocation
{
public:
    StringCharacters(AllocationHandle handle_, MemPtr memPtr_, int32_t segmentId_, int32_t numChars_, uint64_t size_);
    IntegralValue GetChar(int32_t index) const;
    int32_t NumChars() const { return numChars; }
    void Set(ManagedMemoryPool* pool) override;
private:
    int32_t numChars;
};

constexpr uint64_t allocationSize = sizeof(std::unordered_map<AllocationHandle, std::unique_ptr<ManagedAllocation>, AllocationHandleHash>::value_type);
constexpr uint64_t objectSize = sizeof(Object);
constexpr uint64_t arrayContentSize = sizeof(ArrayElements);
constexpr uint64_t stringContentSize = sizeof(StringCharacters);
constexpr uint64_t defaultPoolThreshold = static_cast<uint64_t>(16) * 1024 * 1024;

MACHINE_API void SetPoolThreshold(uint64_t poolThreshold_);
MACHINE_API uint64_t GetPoolThreshold();

class MACHINE_API ManagedMemoryPool
{
public:
    ManagedMemoryPool(Machine& machine_);
    ObjectReference CreateObject(Thread& thread, ObjectType* type);
    ObjectReference CopyObject(Thread& thread, ObjectReference from);
    void DestroyAllocation(AllocationHandle handle);
    Object& GetObject(ObjectReference reference);
    Object* GetObjectNothrow(ObjectReference reference);
    IntegralValue GetField(ObjectReference reference, int32_t fieldIndex);
    void SetField(ObjectReference reference, int32_t fieldIndex, IntegralValue fieldValue);
    AllocationHandle CreateStringCharsFromLiteral(Thread& thread, const char32_t* strLit, uint32_t len);
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
    ObjectReference CreateStringArray(Thread& thread, const std::vector<utf32_string>& programArguments, ObjectType* argsArrayObjectType);
    void ResetLiveFlags();
    void MoveLiveAllocationsToArena(ArenaId fromArenaId, Arena& toArena);
    void MoveLiveAllocationsToNewSegments(Arena& arena);
    MemPtr GetMemPtr(AllocationHandle handle);
    void Set(Object* object_) { object = object_; }
    void Set(ArrayElements* arrayElements_) { arrayElements = arrayElements_; }
    void Set(StringCharacters* stringCharacters_) { stringCharacters = stringCharacters_;  }
    ManagedAllocation* GetAllocation(AllocationHandle handle);
    ManagedAllocation* GetAllocationNothrow(AllocationHandle handle);
    AllocationHandle PoolRoot() const { return poolRoot; }
private:
    Machine& machine;
    Object* object;
    ArrayElements* arrayElements;
    StringCharacters* stringCharacters;
    std::unordered_map<AllocationHandle, std::unique_ptr<ManagedAllocation>, AllocationHandleHash> allocations;
    std::atomic_uint64_t nextReferenceValue;
    std::recursive_mutex allocationsMutex;
    void ComputeSize();
    void CheckSize(Thread& thread, std::unique_lock<std::recursive_mutex>& lock, AllocationHandle root);
    uint64_t objectCount;
    uint64_t arrayContentCount;
    uint64_t stringContentCount;
    uint64_t prevSize;
    uint64_t size;
    std::atomic<AllocationHandle> poolRoot;
    std::unordered_map<AllocationHandle, ManagedAllocation*, AllocationHandleHash> deletedAllocations;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_OBJECT_INCLUDED
