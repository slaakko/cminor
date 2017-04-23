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
    IntegralValue(ValueType type_) : value(0), type(type_) {}
    IntegralValue(uint8_t* value_) : memPtr(value_), type(ValueType::memPtr) {}
    IntegralValue(const char32_t* value_) : strValue(value_), type(ValueType::stringLiteral) {}
    IntegralValue(Function* value_) : funPtr(value_), type(ValueType::functionPtr) {}
    IntegralValue(ClassData* value_) : classDataPtr(value_), type(ValueType::classDataPtr) {}
    IntegralValue(Type* value_) : typePtr(value_), type(ValueType::typePtr) {}
    uint64_t Value() const { return value; }
    ValueType GetType() const { return type; }
    bool AsBool() const { return *static_cast<const bool*>(ValuePtr()); }
    int8_t AsSByte() const { return *static_cast<const int8_t*>(ValuePtr()); }
    uint8_t AsByte() const { return *static_cast<const uint8_t*>(ValuePtr()); }
    int16_t AsShort() const { return *static_cast<const int16_t*>(ValuePtr()); }
    uint16_t AsUShort() const { return *static_cast<const uint16_t*>(ValuePtr()); }
    int32_t AsInt() const { return *static_cast<const int32_t*>(ValuePtr()); }
    uint32_t AsUInt() const { return *static_cast<const uint32_t*>(ValuePtr()); }
    int64_t AsLong() const { return *static_cast<const int64_t*>(ValuePtr()); }
    uint64_t AsULong() const { return *static_cast<const uint64_t*>(ValuePtr()); }
    float AsFloat() const { return *static_cast<const float*>(ValuePtr()); }
    double AsDouble() const { return *static_cast<const double*>(ValuePtr()); }
    char32_t AsChar() const { return *static_cast<const char32_t*>(ValuePtr()); }
    uint8_t* AsMemPtr() const { return memPtr; }
    const char32_t* AsStringLiteral() const { return strValue; }
    Function* AsFunctionPtr() const { return funPtr; }
    ClassData* AsClassDataPtr() const { return classDataPtr; }
    Type* AsTypePtr() const { return typePtr; }
    void Write(Writer& writer);
    void Read(Reader& reader);
    void Dump(CodeFormatter& formatter);
    std::string ValueStr();
    void* ValuePtr() { return &value; }
    const void* ValuePtr() const { return &value; }
private:
    union { uint64_t value; double doubleValue; uint8_t* memPtr; const char32_t* strValue; Function* funPtr; ClassData* classDataPtr; Type* typePtr; };
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

template<typename T>
inline IntegralValue MakeIntegralValue(T value, ValueType type)
{
    IntegralValue integralValue(type);
    *static_cast<T*>(integralValue.ValuePtr()) = value;
    return integralValue;
}

class AllocationHandle : public IntegralValue
{
public:
    AllocationHandle() : IntegralValue(ValueType::allocationHandle) {}
    AllocationHandle(uint64_t value_, ValueType type_) : IntegralValue(MakeIntegralValue<uint64_t>(value_, type_)) {}
    AllocationHandle(uint64_t value_) : IntegralValue(MakeIntegralValue<uint64_t>(value_, ValueType::allocationHandle)) {}
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

uint32_t ValueSize(ValueType type);

enum class AllocationFlags : uint8_t
{
    none = 0,
    live = 1 << 0,
    referenced = 1 << 1,
    object = 1 << 2,
    arrayElements = 1 << 3,
    stringChars = 1 << 4,
    stringLiteral = 1 << 5,
    conditionVariable = 1 << 6
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

const int32_t lockNotAllocated = -1;

struct MACHINE_API ObjectHeader
{
    ObjectType* GetType() const { return type; }
    void SetType(ObjectType* type_) { type = type_; }
    uint64_t HashCode() const { return hashCode; }
    void SetHashCode(uint64_t hashCode_) { hashCode = hashCode_; }

    ObjectType* type;
    uint64_t hashCode;
};

struct MACHINE_API ArrayElementsHeader
{
    Type* GetElementType() const { return elementType; }
    void SetElementType(Type* elementType_) { elementType = elementType_; }
    int32_t NumElements() const { return numElements; }
    void SetNumElements(int32_t numElements_) { numElements = numElements_; }

    Type* elementType;
    int32_t numElements;
};

struct MACHINE_API StringCharactersHeader
{
    int32_t NumChars() const { return numChars; }
    void SetNumChars(int32_t numChars_) { numChars = numChars_; }
    const char32_t* Str() const { return str; }
    void SetStr(const char32_t* str_) { str = str_; }

    const char32_t* str;
    int32_t numChars;
};

struct MACHINE_API ManagedAllocationHeader
{
    uint32_t AllocationSize() const { return allocationSize; }
    void SetAllocationSize(uint32_t allocationSize_) { allocationSize = allocationSize_; }
    int32_t SegmentId() const { return segmentId; }
    void SetSegmentId(int32_t segmentId_) { segmentId = segmentId_; }
    AllocationFlags Flags() const { return flags; }
    void SetFlags(AllocationFlags flags_) { flags = flags_; }
    bool IsLive() const { return GetFlag(AllocationFlags::live); }
    void SetLive() { SetFlag(AllocationFlags::live); }
    void ResetLive() { ResetFlag(AllocationFlags::live); }
    void Reference() { SetFlag(AllocationFlags::referenced); }
    void Unreference() { ResetFlag(AllocationFlags::referenced); }
    bool IsReferenced() const { return GetFlag(AllocationFlags::referenced); }
    bool IsObject() const { return GetFlag(AllocationFlags::object); }
    bool IsArrayElements() const { return GetFlag(AllocationFlags::arrayElements); }
    bool IsStringCharacters() const { return GetFlag(AllocationFlags::stringChars); }
    bool IsStringLiteral() const { return GetFlag(AllocationFlags::stringLiteral); }
    void SetStringLiteral() { SetFlag(AllocationFlags::stringLiteral); }
    bool IsConditionVariable() const { return GetFlag(AllocationFlags::conditionVariable); }
    void SetConditionVariable() { SetFlag(AllocationFlags::conditionVariable); }
    int32_t LockId() const { return lockId; }
    void SetLockId(int32_t lockId_) { lockId = lockId_; }
    bool GetFlag(AllocationFlags flag) const { return (flags & flag) != AllocationFlags::none; }
    void SetFlag(AllocationFlags flag) { flags = flags | flag; }
    void ResetFlag(AllocationFlags flag) { flags = flags & (~flag); }

    uint32_t allocationSize;
    int32_t segmentId;
    std::atomic<int32_t> lockId;
    AllocationFlags flags;

    union
    {
        ObjectHeader objectHeader;
        ArrayElementsHeader arrayElementsHeader;
        StringCharactersHeader stringCharactersHeader;
    };
};

inline ManagedAllocationHeader* GetAllocationHeader(void* allocation) { return static_cast<ManagedAllocationHeader*>(allocation) - 1; }
inline void* GetAllocationPtr(ManagedAllocationHeader* header) { return header + 1; }

IntegralValue GetObjectField(void* object, int index);
void SetObjectField(void* object, IntegralValue fieldValue, int index);
int32_t ObjectFieldCount(void* object);

void MarkObjectLiveAllocations(void* object, std::unordered_set<AllocationHandle, AllocationHandleHash>& checked);

MACHINE_API IntegralValue GetElement(void* arrayElements, int32_t index);
MACHINE_API void SetElement(void* arrayElements, IntegralValue elementValue, int32_t index);
MACHINE_API int32_t NumElements(void* arrayElements);

void MarkArrayElementsLiveAllocations(void* arrayElements, std::unordered_set<AllocationHandle, AllocationHandleHash>& checked);

IntegralValue GetChar(void* stringCharacters, int32_t index);

constexpr uint64_t allocationSize = sizeof(void*);
constexpr uint64_t defaultPoolThreshold = static_cast<uint64_t>(16) * 1024 * 1024;

MACHINE_API void SetPoolThreshold(uint64_t poolThreshold_);
MACHINE_API uint64_t GetPoolThreshold();

constexpr uint64_t firstAllocationHandleValue = 1;

class MACHINE_API ManagedMemoryPool
{
public:
    ManagedMemoryPool(Machine& machine_);
    AllocationHandle AddAllocation(Thread& thread, ManagedAllocationHeader* header, std::unique_lock<std::recursive_mutex>& lock);
    void SetAllocation(AllocationHandle handle, void* allocation) { allocations[handle.Value()] = allocation; }
    void* MoveAllocation(int32_t newSegmentId, void* newAllocWithHeader, ManagedAllocationHeader* header);
    void DestroyAllocation(AllocationHandle handle);
    void DestroyAllocations(std::vector<AllocationHandle>& toBeDestroyed);
    ObjectReference CreateObject(Thread& thread, ObjectType* type);
    ObjectReference CreateObject(Thread& thread, ObjectType* type, std::unique_lock<std::recursive_mutex>& lock);
    ObjectReference CopyObject(Thread& thread, ObjectReference from);
    void* GetObject(ObjectReference reference);
    void* GetObject(ObjectReference reference, std::unique_lock<std::recursive_mutex>& lock);
    void* GetObjectNoThrow(ObjectReference reference, std::unique_lock<std::recursive_mutex>& lock);
    void* GetObjectNoThrowNoLock(ObjectReference reference);
    void* GetAllocation(AllocationHandle handle, std::unique_lock<std::recursive_mutex>& lock);
    void* GetAllocationNoThrowNoLock(AllocationHandle handle);
    IntegralValue GetField(ObjectReference reference, int32_t fieldIndex);
    IntegralValue GetField(ObjectReference reference, int32_t fieldIndex, std::unique_lock<std::recursive_mutex>& lock);
    void SetField(ObjectReference reference, int32_t fieldIndex, IntegralValue fieldValue);
    void SetField(ObjectReference reference, int32_t fieldIndex, IntegralValue fieldValue, std::unique_lock<std::recursive_mutex>& lock);
    int32_t GetFieldCount(ObjectReference reference);
    AllocationHandle CreateStringCharsFromLiteral(Thread& thread, const char32_t* strLit, uint32_t len);
    AllocationHandle CreateStringCharsFromLiteral(Thread& thread, const char32_t* strLit, uint32_t len, std::unique_lock<std::recursive_mutex>& lock);
    std::pair<AllocationHandle, int32_t> CreateStringCharsFromCharArray(Thread& thread, ObjectReference charArray);
    std::pair<AllocationHandle, int32_t> CreateStringCharsFromCharArray(Thread& thread, ObjectReference charArray, std::unique_lock<std::recursive_mutex>& lock);
    ObjectReference CreateString(Thread& thread, const utf32_string& s);
    ObjectReference CreateString(Thread& thread, const utf32_string& s, std::unique_lock<std::recursive_mutex>& lock);
    IntegralValue GetStringChar(ObjectReference str, int32_t index);
    IntegralValue GetStringChar(ObjectReference str, int32_t index, std::unique_lock<std::recursive_mutex>& lock);
    std::string GetUtf8String(ObjectReference str);
    std::vector<uint8_t> GetBytes(ObjectReference arr);
    void SetBytes(ObjectReference arr, const std::vector<uint8_t>& bytes, int32_t count);
    void AllocateArrayElements(Thread& thread, ObjectReference arr, Type* elementType, int32_t length);
    void AllocateArrayElements(Thread& thread, ObjectReference arr, Type* elementType, int32_t length, std::unique_lock<std::recursive_mutex>& lock);
    IntegralValue GetArrayElement(ObjectReference reference, int32_t index);
    void SetArrayElement(ObjectReference reference, int32_t index, IntegralValue elementValue);
    void SetArrayElement(ObjectReference reference, int32_t index, IntegralValue elementValue, std::unique_lock<std::recursive_mutex>& lock);
    int32_t GetNumArrayElements(ObjectReference arr);
    ObjectReference CreateStringArray(Thread& thread, const std::vector<utf32_string>& programArguments, ObjectType* argsArrayObjectType);
    void ResetLiveFlags();
    void MoveLiveAllocationsToArena(ArenaId fromArenaId, Arena& toArena);
    void MoveLiveAllocationsToNewSegments(Arena& arena);
    std::recursive_mutex& AllocationsMutex() { return allocationsMutex; }
private:
    Machine& machine;
    std::vector<void*> allocations;
    std::atomic<uint64_t> nextAllocationHandleValue;
    std::recursive_mutex allocationsMutex;
};

typedef void(*DestroyLockFn)(uint32_t);

MACHINE_API void SetDestroyLockFn(DestroyLockFn destroyLock_);

typedef void(*DestroyConditionVariableFn)(int32_t);

MACHINE_API void SetDestroyConditionVariableFn(DestroyConditionVariableFn destroyCondVar_);

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_OBJECT_INCLUDED
