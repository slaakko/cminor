// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Object.hpp>
#include <cminor/machine/Type.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Runtime.hpp>
#include <cminor/util/String.hpp>
#include <cminor/util/TextUtils.hpp>
#include <cminor/machine/Class.hpp>
#include <cminor/util/Random.hpp>
#include <cminor/machine/Log.hpp>
#include <cminor/util/Unicode.hpp>
#include <cstring>
#include <iostream>

namespace cminor { namespace machine {

using namespace cminor::unicode;

std::string ValueTypeStr(ValueType type)
{
    switch (type)
    {
        case ValueType::none: return "System.Void";
        case ValueType::boolType: return "System.Boolean";
        case ValueType::sbyteType: return "System.Int8";
        case ValueType::byteType: return "System.UInt8";
        case ValueType::shortType: return "System.Int16";
        case ValueType::ushortType: return "System.UInt16";
        case ValueType::intType: return "System.Int32";
        case ValueType::uintType: return "System.UInt32";
        case ValueType::longType: return "System.Int64";
        case ValueType::ulongType: return "System.UInt64";
        case ValueType::floatType: return "System.Float";
        case ValueType::doubleType: return "System.Double";
        case ValueType::charType: return "System.Char";
        case ValueType::memPtr: return "memory pointer";
        case ValueType::stringLiteral: return "string literal";
        case ValueType::allocationHandle: return "allocation handle";
        case ValueType::objectReference: return "object reference";
        case ValueType::functionPtr: return "function pointer";
        case ValueType::classDataPtr: return "class data pointer";
        case ValueType::typePtr: return "type pointer";
        case ValueType::variableReference: return "variable reference";
    }
    return "no type";
}

void IntegralValue::Write(Writer& writer)
{
    writer.Put(uint8_t(type));
    switch (type)
    {
        case ValueType::boolType: writer.Put(AsBool()); break;
        case ValueType::sbyteType: writer.Put(AsSByte()); break;
        case ValueType::byteType: writer.Put(AsByte()); break;
        case ValueType::shortType: writer.Put(AsShort()); break;
        case ValueType::ushortType: writer.Put(AsUShort()); break;
        case ValueType::intType: writer.Put(AsInt()); break;
        case ValueType::uintType: writer.Put(AsUInt()); break;
        case ValueType::longType: writer.Put(AsLong()); break;
        case ValueType::ulongType: writer.Put(AsULong()); break;
        case ValueType::floatType: writer.Put(AsFloat()); break;
        case ValueType::doubleType: writer.Put(AsDouble()); break;
        case ValueType::charType: writer.Put(AsChar()); break;
        case ValueType::stringLiteral: writer.Put(std::u32string(AsStringLiteral())); break;
        case ValueType::allocationHandle: throw SystemException("cannot write allocation handles"); writer.Put(AsULong()); break;
        case ValueType::objectReference: if (value != 0) { throw SystemException("cannot write nonnull object references"); } writer.Put(AsULong()); break;
        default: throw SystemException("invalid integral value type to write");
    }
}

void IntegralValue::Read(Reader& reader)
{
    type = static_cast<ValueType>(reader.GetByte());
    switch (type)
    {
        case ValueType::boolType: *static_cast<bool*>(ValuePtr()) = reader.GetBool(); break;
        case ValueType::sbyteType: *static_cast<int8_t*>(ValuePtr()) = reader.GetSByte(); break;
        case ValueType::byteType: *static_cast<uint8_t*>(ValuePtr()) = reader.GetByte(); break;
        case ValueType::shortType: *static_cast<int16_t*>(ValuePtr()) = reader.GetShort(); break;
        case ValueType::ushortType: *static_cast<uint16_t*>(ValuePtr()) = reader.GetUShort(); break;
        case ValueType::intType: *static_cast<int32_t*>(ValuePtr()) = reader.GetInt(); break;
        case ValueType::uintType: *static_cast<uint32_t*>(ValuePtr()) = reader.GetUInt(); break;
        case ValueType::longType: *static_cast<int64_t*>(ValuePtr()) = reader.GetLong(); break;
        case ValueType::ulongType: *static_cast<uint64_t*>(ValuePtr()) = reader.GetULong(); break;
        case ValueType::floatType: *static_cast<float*>(ValuePtr()) = reader.GetFloat(); break;
        case ValueType::doubleType: *static_cast<double*>(ValuePtr()) = reader.GetDouble(); break;
        case ValueType::charType: *static_cast<char32_t*>(ValuePtr()) = reader.GetChar(); break;
        case ValueType::stringLiteral: /* do not read yet */ break;
        case ValueType::allocationHandle: throw SystemException("read allocation handle");  break;
        case ValueType::objectReference: value = reader.GetULong(); if (value != 0) throw SystemException("read nonull object reference"); break;
        default: throw SystemException("invalid integral value type to read");
    }
}

void IntegralValue::Dump(CodeFormatter& formatter)
{
    formatter.WriteLine("type: " + ValueTypeStr(type) + ", value: " + ValueStr());
}

std::string IntegralValue::ValueStr()
{
    switch (type)
    {
        case ValueType::boolType: if (AsBool()) return "true";  else return "false";
        case ValueType::sbyteType: return std::to_string(AsSByte());
        case ValueType::byteType: return std::to_string(AsByte());
        case ValueType::shortType: return std::to_string(AsShort());
        case ValueType::ushortType: return std::to_string(AsUShort());
        case ValueType::intType: return std::to_string(AsInt());
        case ValueType::uintType: return std::to_string(AsUInt());
        case ValueType::longType: return std::to_string(AsLong());
        case ValueType::ulongType: return std::to_string(AsULong());
        case ValueType::floatType: return std::to_string(AsFloat());
        case ValueType::doubleType: return std::to_string(AsDouble());
        case ValueType::charType: return "'" + ToUtf8(CharStr(AsChar())) + "'";
        case ValueType::memPtr: return ToHexString(AsULong());
        case ValueType::stringLiteral: return "\"" + StringStr(ToUtf8(AsStringLiteral())) + "\"";
        case ValueType::allocationHandle: return AsULong() == 0 ? "null" : std::to_string(AsULong());
        case ValueType::objectReference: return AsULong() == 0 ? "null" : std::to_string(AsULong());
        case ValueType::functionPtr: return ToUtf8(AsFunctionPtr()->CallName().Value().AsStringLiteral());
        case ValueType::classDataPtr: return ToUtf8(AsClassDataPtr()->Type()->Name().Value());
        case ValueType::typePtr: return ToUtf8(AsTypePtr()->Name().Value());
        case ValueType::variableReference: return std::to_string(AsULong());
    }
    return "";
}

uint32_t ValueSize(ValueType type)
{
    switch (type)
    {
        case ValueType::boolType: return sizeof(bool);
        case ValueType::sbyteType: return sizeof(int8_t);
        case ValueType::byteType: return sizeof(uint8_t);
        case ValueType::shortType: return sizeof(int16_t);
        case ValueType::ushortType: return sizeof(uint16_t);
        case ValueType::intType: return sizeof(int32_t);
        case ValueType::uintType: return sizeof(uint32_t);
        case ValueType::longType: return sizeof(int64_t);
        case ValueType::ulongType: return sizeof(uint64_t);
        case ValueType::floatType: return sizeof(float);
        case ValueType::doubleType: return sizeof(double);
        case ValueType::charType: return sizeof(char32_t);
        case ValueType::memPtr: return sizeof(uint8_t*);
        case ValueType::stringLiteral: return sizeof(const char32_t*);
        case ValueType::allocationHandle: return sizeof(uint64_t);
        case ValueType::objectReference: return sizeof(uint64_t);
        case ValueType::classDataPtr: return sizeof(ClassData*);
        case ValueType::typePtr: return sizeof(Type*);
    }
    return sizeof(uint64_t);
}

IntegralValue GetObjectField(void* object, int index)
{
    ObjectHeader* header = &GetAllocationHeader(object)->objectHeader;
    Field field = header->GetType()->GetField(index);
    void* fieldPtr = static_cast<uint8_t*>(object) + field.Offset().Value();
    switch (field.GetType())
    {
        case ValueType::boolType: return MakeIntegralValue<bool>(*static_cast<bool*>(fieldPtr), field.GetType());
        case ValueType::sbyteType: return MakeIntegralValue<int8_t>(*static_cast<int8_t*>(fieldPtr), field.GetType());
        case ValueType::byteType: return MakeIntegralValue<uint8_t>(*static_cast<uint8_t*>(fieldPtr), field.GetType());
        case ValueType::shortType: return MakeIntegralValue<int16_t>(*static_cast<int16_t*>(fieldPtr), field.GetType());
        case ValueType::ushortType: return MakeIntegralValue<uint16_t>(*static_cast<uint16_t*>(fieldPtr), field.GetType());
        case ValueType::intType: return MakeIntegralValue<int32_t>(*static_cast<int32_t*>(fieldPtr), field.GetType());
        case ValueType::uintType: return MakeIntegralValue<uint32_t>(*static_cast<uint32_t*>(fieldPtr), field.GetType());
        case ValueType::longType: return MakeIntegralValue<int64_t>(*static_cast<int64_t*>(fieldPtr), field.GetType());
        case ValueType::ulongType: return MakeIntegralValue<uint64_t>(*static_cast<uint64_t*>(fieldPtr), field.GetType());
        case ValueType::floatType: return MakeIntegralValue<float>(*static_cast<float*>(fieldPtr), field.GetType());
        case ValueType::doubleType: return MakeIntegralValue<double>(*static_cast<double*>(fieldPtr), field.GetType());
        case ValueType::charType: return MakeIntegralValue<char32_t>(*static_cast<char32_t*>(fieldPtr), field.GetType());
        case ValueType::memPtr: return IntegralValue(static_cast<uint8_t*>(fieldPtr));
        case ValueType::classDataPtr: return IntegralValue(*static_cast<ClassData**>(fieldPtr));
        case ValueType::typePtr: return IntegralValue(*static_cast<ObjectType**>(fieldPtr));
        case ValueType::stringLiteral: return IntegralValue(static_cast<const char32_t*>(fieldPtr));
        case ValueType::allocationHandle: return MakeIntegralValue<uint64_t>(*static_cast<uint64_t*>(fieldPtr), field.GetType());
        case ValueType::objectReference: return MakeIntegralValue<uint64_t>(*static_cast<uint64_t*>(fieldPtr), field.GetType());
        case ValueType::functionPtr: return IntegralValue(*static_cast<Function**>(fieldPtr));
        default: return MakeIntegralValue<uint64_t>(*static_cast<uint64_t*>(fieldPtr), field.GetType());
    }
}

void SetObjectField(void* object, IntegralValue fieldValue, int index)
{
    ObjectHeader* header = &GetAllocationHeader(object)->objectHeader;
    Field field = header->GetType()->GetField(index);
    Assert(field.GetType() == fieldValue.GetType(), "value types do not match in set field");
    void* fieldPtr = static_cast<uint8_t*>(object) + field.Offset().Value();
    switch (field.GetType())
    {
        case ValueType::boolType: *static_cast<bool*>(fieldPtr) = fieldValue.AsBool(); break;
        case ValueType::sbyteType: *static_cast<int8_t*>(fieldPtr) = fieldValue.AsSByte(); break;
        case ValueType::byteType: *static_cast<uint8_t*>(fieldPtr) = fieldValue.AsByte(); break;
        case ValueType::shortType: *static_cast<int16_t*>(fieldPtr) = fieldValue.AsShort(); break;
        case ValueType::ushortType: *static_cast<uint16_t*>(fieldPtr) = fieldValue.AsUShort(); break;
        case ValueType::intType: *static_cast<int32_t*>(fieldPtr) = fieldValue.AsInt(); break;
        case ValueType::uintType: *static_cast<uint32_t*>(fieldPtr) = fieldValue.AsUInt(); break;
        case ValueType::longType: *static_cast<int64_t*>(fieldPtr) = fieldValue.AsLong(); break;
        case ValueType::ulongType: *static_cast<uint64_t*>(fieldPtr) = fieldValue.AsULong(); break;
        case ValueType::floatType: *static_cast<float*>(fieldPtr) = fieldValue.AsFloat(); break;
        case ValueType::doubleType: *static_cast<double*>(fieldPtr) = fieldValue.AsDouble(); break;
        case ValueType::charType: *static_cast<char32_t*>(fieldPtr) = fieldValue.AsChar(); break;
        case ValueType::memPtr: *static_cast<uint8_t**>(fieldPtr) = fieldValue.AsMemPtr(); break;
        case ValueType::classDataPtr: *static_cast<ClassData**>(fieldPtr) = fieldValue.AsClassDataPtr(); break;
        case ValueType::typePtr: *static_cast<Type**>(fieldPtr) = fieldValue.AsTypePtr(); break;
        case ValueType::stringLiteral: *static_cast<const char32_t**>(fieldPtr) = fieldValue.AsStringLiteral(); break;
        case ValueType::allocationHandle: *static_cast<uint64_t*>(fieldPtr) = fieldValue.Value(); break;
        case ValueType::objectReference: *static_cast<uint64_t*>(fieldPtr) = fieldValue.Value(); break;
        case ValueType::functionPtr: *static_cast<Function**>(fieldPtr) = fieldValue.AsFunctionPtr(); break;
        default: throw SystemException("invalid field type " + std::to_string(int(field.GetType())));
    }
}

int32_t ObjectFieldCount(void* object)
{
    ObjectHeader* header = &GetAllocationHeader(object)->objectHeader;
    return header->GetType()->FieldCount();
}

void MarkObjectLiveAllocations(void* object, std::unordered_set<AllocationHandle, AllocationHandleHash>& checked)
{
    ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
    int32_t n = ObjectFieldCount(object);
    for (int32_t i = 0; i < n; ++i)
    {
        IntegralValue value = GetObjectField(object, i);
        if (value.GetType() == ValueType::objectReference)
        {
            ObjectReference objectRef(value.Value());
            if (!objectRef.IsNull() && checked.find(objectRef) == checked.cend())
            {
                checked.insert(objectRef);
                void* object = memoryPool.GetObjectNoThrowNoLock(objectRef);
                if (object)
                {
                    ManagedAllocationHeader* header = GetAllocationHeader(object);
                    header->SetLive();
                    MarkObjectLiveAllocations(object, checked);
                }
            }
        }
        else if (value.GetType() == ValueType::allocationHandle)
        {
            AllocationHandle allocationHandle(value.Value());
            if (allocationHandle.Value())
            {
                if (checked.find(allocationHandle) == checked.cend())
                {
                    checked.insert(allocationHandle);
                    void* allocation = memoryPool.GetAllocationNoThrowNoLock(allocationHandle);
                    if (allocation)
                    {
                        ManagedAllocationHeader* allocationHeader = GetAllocationHeader(allocation);
                        allocationHeader->SetLive();
                        if (allocationHeader->IsObject())
                        {
                            MarkObjectLiveAllocations(allocation, checked);
                        }
                        else if (allocationHeader->IsArrayElements())
                        {
                            MarkArrayElementsLiveAllocations(allocation, checked);
                        }
                    }
                }
            }
        }
    }
}

MACHINE_API IntegralValue GetElement(void* arrayElements, int32_t index)
{
    ArrayElementsHeader* header = &GetAllocationHeader(arrayElements)->arrayElementsHeader;
    int32_t numElements = header->NumElements();
    if (index < 0 || index >= numElements)
    {
        throw IndexOutOfRangeException("array index out of range");
    }
    Type* elementType = header->GetElementType();
    ValueType valueType = elementType->GetValueType();
    void* elementPtr = static_cast<uint8_t*>(arrayElements) + ValueSize(valueType) * index;
    switch (valueType)
    {
        case ValueType::boolType: return MakeIntegralValue<bool>(*static_cast<bool*>(elementPtr), valueType);
        case ValueType::sbyteType: return MakeIntegralValue<int8_t>(*static_cast<int8_t*>(elementPtr), valueType);
        case ValueType::byteType: return MakeIntegralValue<uint8_t>(*static_cast<uint8_t*>(elementPtr), valueType);
        case ValueType::shortType: return MakeIntegralValue<int16_t>(*static_cast<int16_t*>(elementPtr), valueType);
        case ValueType::ushortType: return MakeIntegralValue<uint16_t>(*static_cast<uint16_t*>(elementPtr), valueType);
        case ValueType::intType: return MakeIntegralValue<int32_t>(*static_cast<int32_t*>(elementPtr), valueType);
        case ValueType::uintType: return MakeIntegralValue<uint32_t>(*static_cast<uint32_t*>(elementPtr), valueType);
        case ValueType::longType: return MakeIntegralValue<int64_t>(*static_cast<int64_t*>(elementPtr), valueType);
        case ValueType::ulongType: return MakeIntegralValue<uint64_t>(*static_cast<uint64_t*>(elementPtr), valueType);
        case ValueType::floatType: return MakeIntegralValue<float>(*static_cast<float*>(elementPtr), valueType);
        case ValueType::doubleType: return MakeIntegralValue<double>(*static_cast<double*>(elementPtr), valueType);
        case ValueType::charType: return MakeIntegralValue<char32_t>(*static_cast<char32_t*>(elementPtr), valueType);
        case ValueType::memPtr: return IntegralValue(static_cast<uint8_t*>(elementPtr));
        case ValueType::classDataPtr: return IntegralValue(*static_cast<ClassData**>(elementPtr));
        case ValueType::typePtr: return IntegralValue(*static_cast<ObjectType**>(elementPtr));
        case ValueType::stringLiteral: return IntegralValue(static_cast<const char32_t*>(elementPtr));
        case ValueType::allocationHandle: return MakeIntegralValue<uint64_t>(*static_cast<uint64_t*>(elementPtr), valueType);
        case ValueType::objectReference: return MakeIntegralValue<uint64_t>(*static_cast<uint64_t*>(elementPtr), valueType);
        default: return MakeIntegralValue<uint64_t>(*static_cast<uint64_t*>(elementPtr), valueType);
    }
}

MACHINE_API void SetElement(void* arrayElements, IntegralValue elementValue, int32_t index)
{
    ArrayElementsHeader* header = &GetAllocationHeader(arrayElements)->arrayElementsHeader;
    int32_t numElements = header->NumElements();
    if (index < 0 || index >= numElements)
    {
        throw IndexOutOfRangeException("array index out of range");
    }
    Type* elementType = header->GetElementType();
    ValueType valueType = elementType->GetValueType();
    void* elementPtr = static_cast<uint8_t*>(arrayElements) + ValueSize(valueType) * index;
    switch (valueType)
    {
        case ValueType::boolType: *static_cast<bool*>(elementPtr) = elementValue.AsBool(); break;
        case ValueType::sbyteType: *static_cast<int8_t*>(elementPtr) = elementValue.AsSByte(); break;
        case ValueType::byteType: *static_cast<uint8_t*>(elementPtr) = elementValue.AsByte(); break;
        case ValueType::shortType: *static_cast<int16_t*>(elementPtr) = elementValue.AsShort(); break;
        case ValueType::ushortType: *static_cast<uint16_t*>(elementPtr) = elementValue.AsUShort(); break;
        case ValueType::intType: *static_cast<int32_t*>(elementPtr) = elementValue.AsInt(); break;
        case ValueType::uintType: *static_cast<uint32_t*>(elementPtr) = elementValue.AsUInt(); break;
        case ValueType::longType: *static_cast<int64_t*>(elementPtr) = elementValue.AsLong(); break;
        case ValueType::ulongType: *static_cast<uint64_t*>(elementPtr) = elementValue.AsULong(); break;
        case ValueType::floatType: *static_cast<float*>(elementPtr) = elementValue.AsFloat(); break;
        case ValueType::doubleType: *static_cast<double*>(elementPtr) = elementValue.AsDouble(); break;
        case ValueType::charType: *static_cast<char32_t*>(elementPtr) = elementValue.AsChar(); break;
        case ValueType::memPtr: *static_cast<uint8_t**>(elementPtr) = elementValue.AsMemPtr(); break;
        case ValueType::classDataPtr: *static_cast<ClassData**>(elementPtr) = elementValue.AsClassDataPtr(); break;
        case ValueType::typePtr: *static_cast<Type**>(elementPtr) = elementValue.AsTypePtr(); break;
        case ValueType::stringLiteral: *static_cast<const char32_t**>(elementPtr) = elementValue.AsStringLiteral(); break;
        case ValueType::allocationHandle: *static_cast<uint64_t*>(elementPtr) = elementValue.Value(); break;
        case ValueType::objectReference: *static_cast<uint64_t*>(elementPtr) = elementValue.Value(); break;
        default: throw SystemException("invalid element type " + std::to_string(int(valueType)));
    }
}

MACHINE_API int32_t NumElements(void* arrayElements)
{
    ArrayElementsHeader* header = &GetAllocationHeader(arrayElements)->arrayElementsHeader;
    return header->NumElements();
}

void MarkArrayElementsLiveAllocations(void* arrayElements, std::unordered_set<AllocationHandle, AllocationHandleHash>& checked)
{
    ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
    ArrayElementsHeader* header = &GetAllocationHeader(arrayElements)->arrayElementsHeader;
    Type* elementType = header->GetElementType();
    int32_t numElements = header->NumElements();
    if (elementType->GetValueType() == ValueType::objectReference)
    {
        for (int32_t i = 0; i < numElements; ++i)
        {
            IntegralValue value = GetElement(arrayElements, i);
            Assert(value.GetType() == ValueType::objectReference, "object reference expected");
            ObjectReference objectReference(value.Value());
            if (!objectReference.IsNull() && checked.find(objectReference) == checked.cend())
            {
                checked.insert(objectReference);
                void* object = memoryPool.GetObjectNoThrowNoLock(objectReference);
                if (object)
                {
                    ManagedAllocationHeader* header = GetAllocationHeader(object);
                    header->SetLive();
                    MarkObjectLiveAllocations(object, checked);
                }
            }
        }
    }
}

IntegralValue GetChar(void* stringCharacters, int32_t index)
{
    StringCharactersHeader* header = &GetAllocationHeader(stringCharacters)->stringCharactersHeader;
    if (index < 0 || index >= header->NumChars())
    {
        throw IndexOutOfRangeException("string index out of range");
    }
    return MakeIntegralValue<char32_t>(header->Str()[index], ValueType::charType);
}

uint64_t poolThreshold = defaultPoolThreshold;

void SetPoolThreshold(uint64_t poolThreshold_)
{
    poolThreshold = poolThreshold_;
}

uint64_t GetPoolThreshold()
{
    return poolThreshold;
}

ManagedMemoryPool::ManagedMemoryPool(Machine& machine_) : machine(machine_), nextAllocationHandleValue(firstAllocationHandleValue)
{
}

AllocationHandle ManagedMemoryPool::AddAllocation(Thread& thread, ManagedAllocationHeader* header, std::unique_lock<std::recursive_mutex>& lock)
{
    AllocationHandle allocationHandle;
    if (thread.HasAllocationHandles())
    {
        allocationHandle = thread.PopAllocationHandle();
    }
    else
    {
        allocationHandle = nextAllocationHandleValue++;
    }
    if (!lock.owns_lock())
    {
        lock.lock();
    }
    if (allocationHandle.Value() < uint64_t(allocations.size()))
    {
        Assert(!allocations[allocationHandle.Value()], "overwriting existing allocation");
        allocations[allocationHandle.Value()] = GetAllocationPtr(header);
    }
    else
    {
        void* allocationPtr = GetAllocationPtr(header);
        while (allocationHandle.Value() >= uint64_t(allocations.size()))
        {
            allocations.push_back(nullptr);
        }
        allocations[allocationHandle.Value()] = allocationPtr;
    }
    return allocationHandle;
}

void* ManagedMemoryPool::MoveAllocation(int32_t newSegmentId, void* newAllocWithHeader, ManagedAllocationHeader* header)
{
    std::memcpy(newAllocWithHeader, header, header->AllocationSize());
    ManagedAllocationHeader* newHeader = static_cast<ManagedAllocationHeader*>(newAllocWithHeader);
    newHeader->SetSegmentId(newSegmentId);
    void* allocationPtr = GetAllocationPtr(newHeader);
    if (header->IsStringCharacters())
    {
        if (!header->IsStringLiteral())
        {
            StringCharactersHeader* newStringCharsHeader = &newHeader->stringCharactersHeader;
            const char32_t* str = static_cast<const char32_t*>(allocationPtr);
            newStringCharsHeader->SetStr(str);
        }
    }
    return allocationPtr;
}

DestroyLockFn destroyLock = nullptr;

MACHINE_API void SetDestroyLockFn(DestroyLockFn destroyLock_)
{
    destroyLock = destroyLock_;
}

DestroyConditionVariableFn destroyCondVar = nullptr;

MACHINE_API void SetDestroyConditionVariableFn(DestroyConditionVariableFn destroyCondVar_)
{
    destroyCondVar = destroyCondVar_;
}

void ManagedMemoryPool::DestroyAllocation(AllocationHandle handle)
{ 
    void* allocation = allocations[handle.Value()];
    ManagedAllocationHeader* header = GetAllocationHeader(allocation);
    if (header->IsObject())
    {
        if (header->LockId() != lockNotAllocated)
        {
            if (destroyLock)
            {
                destroyLock(header->LockId());
            }
        }
        if (header->IsConditionVariable())
        {
            IntegralValue conditionVariableId = GetObjectField(allocation, 1);
            Assert(conditionVariableId.GetType() == ValueType::intType, "int type expected");
            if (destroyCondVar)
            {
                destroyCondVar(conditionVariableId.AsInt());
            }
        }
    }
    allocations[handle.Value()] = nullptr;
}

void ManagedMemoryPool::DestroyAllocations(std::vector<AllocationHandle>& toBeDestroyed)
{
    int32_t n = int32_t(toBeDestroyed.size());
    int32_t m = int32_t(GetMachine().Threads().size());
    if (m == 1)
    {
        Thread& thread = GetMachine().MainThread();
        for (int32_t i = 0; i < n; ++i)
        {
            AllocationHandle handle = toBeDestroyed[i];
            thread.PushAllocationHandle(handle);
        }
    }
    else
    {
        for (int32_t i = 0; i < n; ++i)
        {
            AllocationHandle handle = toBeDestroyed[i];
            int32_t threadId = i % m;
            int32_t startTheadId = threadId;
            Thread* thread = GetMachine().Threads()[threadId].get();
            while (thread->GetState() == ThreadState::waiting)
            {
                threadId = (threadId + 1) % m;
                thread = GetMachine().Threads()[threadId].get();
                if (threadId == startTheadId)
                {
                    break;
                }
            }
            thread->PushAllocationHandle(handle);
        }
    }
    for (AllocationHandle handle : toBeDestroyed)
    {
        DestroyAllocation(handle);
    }
    toBeDestroyed.clear();
}

ObjectReference ManagedMemoryPool::CreateObject(Thread& thread, ObjectType* type, std::unique_lock<std::recursive_mutex>& lock)
{
    void* ptr = nullptr;
    int32_t segmentId = -1;
    Assert(type->ObjectSize() < std::numeric_limits<uint32_t>::max(), "object too big");
    uint32_t objectSize = static_cast<uint32_t>(type->ObjectSize());
    uint32_t allocationSize = objectSize + sizeof(ManagedAllocationHeader);
    machine.AllocateMemory(thread, allocationSize, ptr, segmentId, lock);
    ManagedAllocationHeader* header = static_cast<ManagedAllocationHeader*>(ptr);
    ObjectHeader* objectHeader = &header->objectHeader;
    header->SetAllocationSize(allocationSize);
    header->SetSegmentId(segmentId);
    header->SetLockId(lockNotAllocated);
    header->SetFlags(AllocationFlags::object);
    objectHeader->SetType(type);
    if (!lock.owns_lock())
    {
        lock.lock();
    }
    objectHeader->SetHashCode(Random64());
    return ObjectReference(AddAllocation(thread, header, lock).Value());
}

ObjectReference ManagedMemoryPool::CreateObject(Thread& thread, ObjectType* type)
{
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex, std::defer_lock_t());
    return CreateObject(thread, type, lock);
}

ObjectReference ManagedMemoryPool::CopyObject(Thread& thread, ObjectReference from)
{
    if (from.IsNull())
    {
        return from;
    }
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex, std::defer_lock_t());
    void* object = GetObject(from, lock);
    return ObjectReference(AddAllocation(thread, GetAllocationHeader(object), lock).Value());
}

void* ManagedMemoryPool::GetObject(ObjectReference reference)
{
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex);
    return GetObject(reference, lock);
}

void* ManagedMemoryPool::GetObject(ObjectReference reference, std::unique_lock<std::recursive_mutex>& lock)
{
    if (reference.IsNull())
    {
        throw NullReferenceException("object reference is null");
    }
    uint64_t index = reference.Value();
    if (!lock.owns_lock())
    {
        lock.lock();
    }
    if (index < uint64_t(allocations.size()))
    {
        return allocations[index];
    }
    throw SystemException("object with reference " + std::to_string(reference.Value()) + " not found");
}

void* ManagedMemoryPool::GetObjectNoThrow(ObjectReference reference, std::unique_lock<std::recursive_mutex>& lock)
{
    uint64_t index = reference.Value();
    if (!lock.owns_lock())
    {
        lock.lock();
    }
    if (index < uint64_t(allocations.size()))
    {
        return allocations[index];
    }
    return nullptr;
}

void* ManagedMemoryPool::GetObjectNoThrowNoLock(ObjectReference reference)
{
    uint64_t index = reference.Value();
    if (index < uint64_t(allocations.size()))
    {
        return allocations[index];
    }
    return nullptr;
}

void* ManagedMemoryPool::GetAllocation(AllocationHandle handle, std::unique_lock<std::recursive_mutex>& lock)
{
    uint64_t index = handle.Value();
    if (!lock.owns_lock())
    {
        lock.lock();
    }
    if (index < uint64_t(allocations.size()))
    {
        return allocations[index];
    }
    throw SystemException("allocation with handle " + std::to_string(handle.Value()) + " not found");
}

void* ManagedMemoryPool::GetAllocationNoThrowNoLock(AllocationHandle handle)
{
    uint64_t index = handle.Value();
    if (index < uint64_t(allocations.size()))
    {
        return allocations[index];
    }
    return nullptr;
}

IntegralValue ManagedMemoryPool::GetField(ObjectReference reference, int32_t fieldIndex, std::unique_lock<std::recursive_mutex>& lock)
{
    if (reference.IsNull())
    {
        throw NullReferenceException("cannot get field of a null object reference");
    }
    void* object = GetObject(reference, lock);
    return GetObjectField(object, fieldIndex);
}

IntegralValue ManagedMemoryPool::GetField(ObjectReference reference, int32_t fieldIndex)
{
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex, std::defer_lock_t());
    return GetField(reference, fieldIndex, lock);
}

void ManagedMemoryPool::SetField(ObjectReference reference, int32_t fieldIndex, IntegralValue fieldValue, std::unique_lock<std::recursive_mutex>& lock)
{
    if (reference.IsNull())
    {
        throw NullReferenceException("cannot set field of a null object reference");
    }
    void* object = GetObject(reference, lock);
    SetObjectField(object, fieldValue, fieldIndex);
}

void ManagedMemoryPool::SetField(ObjectReference reference, int32_t fieldIndex, IntegralValue fieldValue)
{
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex, std::defer_lock_t());
    SetField(reference, fieldIndex, fieldValue, lock);
}

int32_t ManagedMemoryPool::GetFieldCount(ObjectReference reference)
{
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex, std::defer_lock_t());
    void* object = GetObject(reference, lock);
    return ObjectFieldCount(object);
}

AllocationHandle ManagedMemoryPool::CreateStringCharsFromLiteral(Thread& thread, const char32_t* strLit, uint32_t len)
{
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex, std::defer_lock_t());
    return CreateStringCharsFromLiteral(thread, strLit, len, lock);
}

AllocationHandle ManagedMemoryPool::CreateStringCharsFromLiteral(Thread& thread, const char32_t* strLit, uint32_t len, std::unique_lock<std::recursive_mutex>& lock)
{
    void* ptr = nullptr;
    int32_t segmentId = -1;
    uint32_t stringContentSize = 0;
    uint32_t allocationSize = stringContentSize + sizeof(ManagedAllocationHeader);
    machine.AllocateMemory(thread, allocationSize, ptr, segmentId, lock);
    ManagedAllocationHeader* header = static_cast<ManagedAllocationHeader*>(ptr);
    StringCharactersHeader* stringCharsHeader = &header->stringCharactersHeader;
    header->SetAllocationSize(allocationSize);
    header->SetSegmentId(segmentId);
    header->SetFlags(AllocationFlags::stringChars);
    header->SetStringLiteral();
    stringCharsHeader->SetNumChars(len);
    stringCharsHeader->SetStr(strLit);
    return AddAllocation(thread, header, lock);
}

std::pair<AllocationHandle, int32_t> ManagedMemoryPool::CreateStringCharsFromCharArray(Thread& thread, ObjectReference charArray)
{
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex, std::defer_lock_t());
    return CreateStringCharsFromCharArray(thread, charArray, lock);
}

std::pair<AllocationHandle, int32_t> ManagedMemoryPool::CreateStringCharsFromCharArray(Thread& thread, ObjectReference charArray, std::unique_lock<std::recursive_mutex>& lock)
{
    IntegralValue charElementsValue = GetField(charArray, 2, lock);
    Assert(charElementsValue.GetType() == ValueType::allocationHandle, "allocation handle expected");
    AllocationHandle charElementsHandle(charElementsValue.Value());
    void* charElements = GetAllocation(charElementsHandle, lock);
    ManagedAllocationHeader* charElementsHeader = GetAllocationHeader(charElements);
    charElementsHeader->Reference();
    ArrayElementsHeader* arrayHeader = &charElementsHeader->arrayElementsHeader;
    int32_t numChars = arrayHeader->NumElements();
    if (numChars > 0)
    {
        uint32_t stringContentSize = numChars * sizeof(char32_t);
        uint32_t allocationSize = stringContentSize + sizeof(ManagedAllocationHeader);
        void* ptr = nullptr;
        int32_t segmentId = -1;
        machine.AllocateMemory(thread, allocationSize, ptr, segmentId, lock);
        ManagedAllocationHeader* header = static_cast<ManagedAllocationHeader*>(ptr);
        StringCharactersHeader* stringCharsHeader = &header->stringCharactersHeader;
        header->SetAllocationSize(allocationSize);
        header->SetSegmentId(segmentId);
        header->SetFlags(AllocationFlags::stringChars);
        stringCharsHeader->SetNumChars(numChars);
        void* strPtr = GetAllocationPtr(header);
        stringCharsHeader->SetStr(static_cast<const char32_t*>(strPtr));
        charElements = GetAllocation(charElementsHandle, lock);
        std::memcpy(strPtr, charElements, stringContentSize);
        AllocationHandle handle = AddAllocation(thread, header, lock);
        charElements = GetAllocation(charElementsHandle, lock);
        charElementsHeader = GetAllocationHeader(charElements);
        charElementsHeader->Unreference();
        return std::make_pair(handle, numChars);
    }
    else
    {
        charElements = GetAllocation(charElementsHandle, lock);
        charElementsHeader = GetAllocationHeader(charElements);
        charElementsHeader->Unreference();
        AllocationHandle handle = CreateStringCharsFromLiteral(thread, U"", 0, lock);
        return std::make_pair(handle, numChars);
    }
}

ObjectReference ManagedMemoryPool::CreateString(Thread& thread, const std::u32string& s)
{
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex, std::defer_lock_t());
    return CreateString(thread, s, lock);
}

ObjectReference ManagedMemoryPool::CreateString(Thread& thread, const std::u32string& s, std::unique_lock<std::recursive_mutex>& lock)
{
    ClassData* classData = ClassDataTable::GetSystemStringClassData();
    int32_t numChars = int32_t(s.length());
    AllocationHandle handle;
    if (numChars > 0)
    {
        uint32_t stringContentSize = numChars * sizeof(char32_t);
        uint32_t allocationSize = stringContentSize + sizeof(ManagedAllocationHeader);
        void* ptr = nullptr;
        int32_t segmentId = -1;
        machine.AllocateMemory(thread, allocationSize, ptr, segmentId, lock);
        ManagedAllocationHeader* header = static_cast<ManagedAllocationHeader*>(ptr);
        StringCharactersHeader* stringCharsHeader = &header->stringCharactersHeader;
        header->SetAllocationSize(allocationSize);
        header->SetSegmentId(segmentId);
        header->SetFlags(AllocationFlags::stringChars);
        stringCharsHeader->SetNumChars(numChars);
        void* strPtr = GetAllocationPtr(header);
        stringCharsHeader->SetStr(static_cast<const char32_t*>(strPtr));
        std::memcpy(strPtr, s.c_str(), stringContentSize);
        handle = AddAllocation(thread, header, lock);
    }
    else
    {
        handle = CreateStringCharsFromLiteral(thread, U"", 0, lock);
    }
    ClassData* classDataPtr = ClassDataTable::GetSystemStringClassData();
    ObjectReference str = CreateObject(thread, classData->Type(), lock);
    void* strObject = GetObject(str, lock);
    SetObjectField(strObject, IntegralValue(classDataPtr), 0);
    SetObjectField(strObject, MakeIntegralValue<int32_t>(numChars, ValueType::intType), 1);
    SetObjectField(strObject, handle, 2);
    return str;
}

IntegralValue ManagedMemoryPool::GetStringChar(ObjectReference str, int32_t index)
{
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex, std::defer_lock_t());
    return GetStringChar(str, index, lock);
}

IntegralValue ManagedMemoryPool::GetStringChar(ObjectReference str, int32_t index, std::unique_lock<std::recursive_mutex>& lock)
{
    if (str.IsNull())
    {
        throw NullReferenceException("cannot index a null string");
    }
    IntegralValue charsHandleValue = GetField(str, 2, lock);
    Assert(charsHandleValue.GetType() == ValueType::allocationHandle, "allocation handle expected");
    AllocationHandle handle(charsHandleValue.Value());
    void* stringChars = GetAllocation(handle, lock);
    return GetChar(stringChars, index);
}

std::string ManagedMemoryPool::GetUtf8String(ObjectReference str)
{
    if (str.IsNull())
    {
        throw NullReferenceException("cannot get value of a null string");
    }
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex);
    IntegralValue charsHandleValue = GetField(str, 2, lock);
    Assert(charsHandleValue.GetType() == ValueType::allocationHandle, "allocation handle expected");
    AllocationHandle handle(charsHandleValue.Value());
    void* stringChars = GetAllocation(handle, lock);
    ManagedAllocationHeader* header = GetAllocationHeader(stringChars);
    StringCharactersHeader* stringCharsHeader = &header->stringCharactersHeader;
    std::u32string s;
    int32_t n = stringCharsHeader->NumChars();
    for (int32_t i = 0; i < n; ++i)
    {
        s.append(1, GetChar(stringChars, i).AsChar());
    }
    return ToUtf8(s);
}

std::vector<uint8_t> ManagedMemoryPool::GetBytes(ObjectReference arr)
{
    std::vector<uint8_t> bytes;
    if (arr.IsNull())
    {
        throw NullReferenceException("cannot get value of a null array");
    }
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex);
    IntegralValue elementsHandleValue = GetField(arr, 2, lock);
    Assert(elementsHandleValue.GetType() == ValueType::allocationHandle, "allocation handle expected");
    AllocationHandle handle(elementsHandleValue.Value());
    void* arrayElements = GetAllocation(handle, lock);
    lock.unlock();
    ManagedAllocationHeader* header = GetAllocationHeader(arrayElements);
    ArrayElementsHeader* arrayElementsHeader = &header->arrayElementsHeader;
    Assert(arrayElementsHeader->GetElementType() == TypeTable::GetType(StringPtr(U"System.UInt8")), "byte array expected");
    int32_t n = arrayElementsHeader->NumElements();
    for (int32_t i = 0; i < n; ++i)
    {
        bytes.push_back(GetElement(arrayElements, i).AsByte());
    }
    return bytes;
}

void ManagedMemoryPool::SetBytes(ObjectReference arr, const std::vector<uint8_t>& bytes, int32_t count)
{
    if (arr.IsNull())
    {
        throw NullReferenceException("cannot set value of a null array");
    }
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex);
    IntegralValue elementsHandleValue = GetField(arr, 2, lock);
    Assert(elementsHandleValue.GetType() == ValueType::allocationHandle, "allocation handle expected");
    AllocationHandle handle(elementsHandleValue.Value());
    void* arrayElements = GetAllocation(handle, lock);
    ManagedAllocationHeader* header = GetAllocationHeader(arrayElements);
    ArrayElementsHeader* arrayElementsHeader = &header->arrayElementsHeader;
    Assert(arrayElementsHeader->GetElementType() == TypeTable::GetType(StringPtr(U"System.UInt8")), "byte array expected");
    for (int32_t i = 0; i < count; ++i)
    {
        SetElement(arrayElements, MakeIntegralValue<uint8_t>(bytes[i], ValueType::byteType), i);
    }
}

void ManagedMemoryPool::AllocateArrayElements(Thread& thread, ObjectReference arr, Type* elementType, int32_t length)
{
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex);
    AllocateArrayElements(thread, arr, elementType, length, lock);
}

void ManagedMemoryPool::AllocateArrayElements(Thread& thread, ObjectReference arr, Type* elementType, int32_t length, std::unique_lock<std::recursive_mutex>& lock)
{
    if (length < 0)
    {
        throw ArgumentOutOfRangeException("invalid array length");
    }
    uint32_t arraySize = ValueSize(elementType->GetValueType()) * uint32_t(length);
    void* ptr = nullptr;
    int32_t segmentId = -1;
    uint32_t allocationSize = arraySize + sizeof(ManagedAllocationHeader);
    machine.AllocateMemory(thread, allocationSize, ptr, segmentId, lock);
    ManagedAllocationHeader* header = static_cast<ManagedAllocationHeader*>(ptr);
    header->SetAllocationSize(allocationSize);
    header->SetSegmentId(segmentId);
    header->SetFlags(AllocationFlags::arrayElements);
    ArrayElementsHeader* arrayElementsHeader = &header->arrayElementsHeader;
    arrayElementsHeader->SetElementType(elementType);
    arrayElementsHeader->SetNumElements(length);
    AllocationHandle handle = AddAllocation(thread, header, lock);
    void* a = GetObject(arr, lock);
    SetObjectField(a, handle, 2);
}

IntegralValue ManagedMemoryPool::GetArrayElement(ObjectReference reference, int32_t index)
{
    if (reference.IsNull())
    { 
        throw NullReferenceException("cannot get item of a null array");
    }
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex);
    IntegralValue elementsHandleValue = GetField(reference, 2, lock);
    Assert(elementsHandleValue.GetType() == ValueType::allocationHandle, "allocation handle expected");
    AllocationHandle handle(elementsHandleValue.Value());
    void* arrayElements = GetAllocation(handle, lock);
    return GetElement(arrayElements, index);
}

void ManagedMemoryPool::SetArrayElement(ObjectReference reference, int32_t index, IntegralValue elementValue)
{
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex, std::defer_lock_t());
    SetArrayElement(reference, index, elementValue, lock);
}

void ManagedMemoryPool::SetArrayElement(ObjectReference reference, int32_t index, IntegralValue elementValue, std::unique_lock<std::recursive_mutex>& lock)
{
    if (reference.IsNull())
    {
        throw NullReferenceException("cannot set item of a null array");
    }
    IntegralValue elementsHandleValue = GetField(reference, 2, lock);
    Assert(elementsHandleValue.GetType() == ValueType::allocationHandle, "allocation handle expected");
    AllocationHandle handle(elementsHandleValue.Value());
    void* arrayElements = GetAllocation(handle, lock);
    SetElement(arrayElements, elementValue, index);
}

int32_t ManagedMemoryPool::GetNumArrayElements(ObjectReference arr)
{
    if (arr.IsNull())
    {
        throw NullReferenceException("cannot get number of items of a null array");
    }
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex);
    IntegralValue elementsHandleValue = GetField(arr, 2, lock);
    Assert(elementsHandleValue.GetType() == ValueType::allocationHandle, "allocation handle expected");
    AllocationHandle handle(elementsHandleValue.Value());
    void* arrayElements = GetAllocation(handle, lock);
    ManagedAllocationHeader* header = GetAllocationHeader(arrayElements);
    ArrayElementsHeader* arrayElementsHeader = &header->arrayElementsHeader;
    return arrayElementsHeader->NumElements();
}

ObjectReference ManagedMemoryPool::CreateStringArray(Thread& thread, const std::vector<std::u32string>& strings, ObjectType* argsArrayObjectType)
{
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex, std::defer_lock_t());
    ObjectReference arrayReference = CreateObject(thread, argsArrayObjectType, lock);
    void* object = GetObject(arrayReference, lock);
    ClassData* classData = ClassDataTable::GetClassData(StringPtr(U"System.String[]"));
    SetObjectField(object, IntegralValue(classData), 0);
    int32_t length = int32_t(strings.size());
    SetObjectField(object, MakeIntegralValue<int32_t>(length, ValueType::intType), 1);
    AllocateArrayElements(thread, arrayReference, classData->Type(), length, lock);
    for (int32_t i = 0; i < length; ++i)
    {
        const std::u32string& s = strings[i];
        ObjectReference str = CreateString(thread, s, lock);
        SetArrayElement(arrayReference, i, str, lock);
    }
    return arrayReference;
}

void ManagedMemoryPool::ResetLiveFlags()
{
    for (void* allocation : allocations)
    {
        if (allocation)
        {
            ManagedAllocationHeader* header = GetAllocationHeader(allocation);
            header->ResetLive();
        }
    }
}

void ManagedMemoryPool::MoveLiveAllocationsToArena(ArenaId fromArenaId, Arena& toArena)
{
    std::unordered_map<void*, void*> moveMap;
    std::vector<AllocationHandle> toBeDestroyed;
    for (uint64_t i = firstAllocationHandleValue; i < uint64_t(allocations.size()); ++i)
    {
        AllocationHandle allocationHandle = i;
        void* allocation = allocations[i];
        if (allocation)
        {
            ManagedAllocationHeader* header = GetAllocationHeader(allocation);
            int32_t segmentId = header->SegmentId();
            if (segmentId != notGarbageCollectedSegment)
            {
                if (machine.GetSegment(segmentId)->GetArenaId() == fromArenaId)
                {
                    if (header->IsLive() || header->IsReferenced())
                    {
                        auto it = moveMap.find(allocation);
                        if (it == moveMap.cend())
                        {
                            uint32_t n = header->AllocationSize();
                            void* newAllocWithHeader = nullptr;
                            int32_t newSegmentId = -1;
                            toArena.Allocate(n, newAllocWithHeader, newSegmentId);
                            void* movedAllocation = MoveAllocation(newSegmentId, newAllocWithHeader, header);
                            allocations[i] = movedAllocation;
                            moveMap[allocation] = movedAllocation;
                        }
                        else
                        {
                            void* movedAllocation = it->second;
                            allocations[i] = movedAllocation;
                        }
                    }
                    else 
                    {
                        toBeDestroyed.push_back(allocationHandle);
                    }
                }
            }
            else
            {
                throw std::runtime_error("invalid segment id -1");
            }
        }
    }
    DestroyAllocations(toBeDestroyed);
}

struct SegmentIdLess
{
    bool operator()(AllocationHandle left, AllocationHandle right) const
    {
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        void* leftAllocation = memoryPool.GetAllocationNoThrowNoLock(left);
        ManagedAllocationHeader* leftHeader = GetAllocationHeader(leftAllocation);
        void* rightAllocation = memoryPool.GetAllocationNoThrowNoLock(right);
        ManagedAllocationHeader* rightHeader = GetAllocationHeader(rightAllocation);
        return leftHeader->SegmentId() < rightHeader->SegmentId();
    }
};

void MoveSegment(Arena& arena, std::unordered_map<void*, void*>& moveMap, int32_t segmentId, 
    std::vector<AllocationHandle>::iterator segmentBegin, std::vector<AllocationHandle>::iterator segmentEnd, bool allocateNewSegment, std::vector<AllocationHandle>& toBeDestroyed)
{
    ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
    for (auto sit = segmentBegin; sit != segmentEnd; ++sit)
    {
        AllocationHandle handle = *sit;
        if (handle.Value() != 0)
        {
            void* allocation = memoryPool.GetAllocationNoThrowNoLock(handle);
            if (allocation)
            {
                auto mit = moveMap.find(allocation);
                if (mit == moveMap.cend())
                {
                    ManagedAllocationHeader* header = GetAllocationHeader(allocation);
                    uint32_t n = header->AllocationSize();
                    void* newAllocWithHeader = nullptr;
                    int32_t newSegmentId = -1;
                    arena.Allocate(n, newAllocWithHeader, newSegmentId, allocateNewSegment);
                    allocateNewSegment = false;
                    void* movedAllocation = memoryPool.MoveAllocation(newSegmentId, newAllocWithHeader, header);
                    memoryPool.SetAllocation(handle, movedAllocation);
                    moveMap[allocation] = movedAllocation;
                }
                else
                {
                    void* movedAllocation = mit->second;
                    memoryPool.SetAllocation(handle, movedAllocation);
                }
            }
        }
    }
    memoryPool.DestroyAllocations(toBeDestroyed);
    arena.RemoveSegment(segmentId);
}

void ManagedMemoryPool::MoveLiveAllocationsToNewSegments(Arena& arena)
{
    std::unordered_map<void*, void*> moveMap;
    std::vector<AllocationHandle> liveAllocations;
    std::vector<AllocationHandle> toBeDestroyed;
    std::unordered_set<int32_t> liveSegments;
    for (uint64_t i = firstAllocationHandleValue; i < uint64_t(allocations.size()); ++i)
    {
        AllocationHandle allocationHandle = i;
        void* allocation = allocations[i];
        if (allocation)
        {
            ManagedAllocationHeader* header = GetAllocationHeader(allocation);
            if (header->SegmentId() != notGarbageCollectedSegment)
            {
                if (machine.GetSegment(header->SegmentId())->GetArenaId() == arena.Id())
                {
                    if (header->IsLive() || header->IsReferenced())
                    {
                        liveAllocations.push_back(allocationHandle);
                        liveSegments.insert(header->SegmentId());
                    }
                    else
                    {
                        toBeDestroyed.push_back(allocationHandle);
                    }
                }
            }
            else
            {
                throw std::runtime_error("invalid segment id -1");
            }
        }
    }
    DestroyAllocations(toBeDestroyed);
    arena.RemoveEmptySegments(liveSegments);
    std::sort(liveAllocations.begin(), liveAllocations.end(), SegmentIdLess());
    int32_t currentSegmentId = -1;
    auto segmentBegin = liveAllocations.begin();
    auto end = liveAllocations.end();
    bool firstMove = true;
    for (auto it = liveAllocations.begin(); it != end; ++it)
    {
        AllocationHandle liveAllocationHandle = *it;
        if (liveAllocationHandle.Value() != 0)
        {
            void* liveAllocation = allocations[liveAllocationHandle.Value()];
            if (liveAllocation)
            {
                ManagedAllocationHeader* liveAllocationHeader = GetAllocationHeader(liveAllocation);
                if (liveAllocationHeader->SegmentId() != currentSegmentId)
                {
                    bool moveSegment = currentSegmentId != -1;
                    if (it - segmentBegin == 1)
                    {
                        AllocationHandle firstHandle = *segmentBegin;
                        void* firstAllocation = allocations[firstHandle.Value()];
                        ManagedAllocationHeader* firstAllocationHeader = GetAllocationHeader(firstAllocation);
                        if (firstAllocationHeader->AllocationSize() >= GetSegmentSize())
                        {
                            moveSegment = false;
                        }
                    }
                    if (moveSegment)
                    {
                        MoveSegment(arena, moveMap, currentSegmentId, segmentBegin, it, firstMove, toBeDestroyed);
                        firstMove = false;
                    }
                    currentSegmentId = liveAllocationHeader->SegmentId();
                    segmentBegin = it;
                }
            }
        }
    }
    bool moveSegment = currentSegmentId != -1;
    if (end - segmentBegin == 1)
    {
        AllocationHandle singletonHandle = *segmentBegin;
        if (singletonHandle.Value() != 0)
        {
            void* singleton = allocations[singletonHandle.Value()];
            if (singleton)
            {
                ManagedAllocationHeader* singletonHeader = GetAllocationHeader(singleton);
                if (singletonHeader->AllocationSize() >= GetSegmentSize())
                {
                    moveSegment = false;
                }
            }
        }
    }
    if (moveSegment)
    {
        MoveSegment(arena, moveMap, currentSegmentId, segmentBegin, end, firstMove, toBeDestroyed);
        firstMove = false;
    }
}

} } // namespace cminor::machine
