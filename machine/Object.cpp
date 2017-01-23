// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Object.hpp>
#include <cminor/machine/Type.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/String.hpp>
#include <cminor/machine/Class.hpp>
#include <cminor/machine/Random.hpp>

namespace cminor { namespace machine {

std::string ValueTypeStr(ValueType type)
{
    switch (type)
    {
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
        case ValueType::stringLiteral: writer.Put(utf32_string(AsStringLiteral())); break;
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
        case ValueType::boolType: value = reader.GetBool(); break;
        case ValueType::sbyteType: value = reader.GetSByte(); break;
        case ValueType::byteType: value = reader.GetByte(); break;
        case ValueType::shortType: value = reader.GetShort(); break;
        case ValueType::ushortType: value = reader.GetUShort(); break;
        case ValueType::intType: value = reader.GetInt(); break;
        case ValueType::uintType: value = reader.GetUInt(); break;
        case ValueType::longType: value = reader.GetLong(); break;
        case ValueType::ulongType: value = reader.GetULong(); break;
#pragma warning(disable : 4244)
        case ValueType::floatType: value = reader.GetFloat(); break;
        case ValueType::doubleType: value = reader.GetDouble(); break;
#pragma warning(default : 4244)
        case ValueType::charType: value = reader.GetChar(); break;
        case ValueType::stringLiteral: /* do not read yet */ break;
        case ValueType::allocationHandle: throw SystemException("read allocation handle");  break;
        case ValueType::objectReference: value = reader.GetULong(); if (value != 0) throw SystemException("read nonull object reference"); break;
        default: throw SystemException("invalid integral value type to read");
    }
}

uint64_t ValueSize(ValueType type)
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

ManagedAllocation::ManagedAllocation(AllocationHandle handle_, MemPtr memPtr_, int32_t segmentId_, uint64_t size_) : 
    handle(handle_), memPtr(memPtr_), segmentId(segmentId_), size(size_), flags(AllocationFlags::none)
{
}

ManagedAllocation::~ManagedAllocation()
{
}

void ManagedAllocation::MarkLiveAllocations(std::unordered_set<AllocationHandle, AllocationHandleHash>& checked, ManagedMemoryPool& managedMemoryPool)
{
}

Object::Object(ObjectReference reference_, MemPtr memPtr_, int32_t segmentId_, ObjectType* type_, uint64_t size_) : 
    ManagedAllocation(reference_, memPtr_, segmentId_, size_), reference(reference_), type(type_)
{
}

IntegralValue Object::GetField(int index) const
{
    Field field = type->GetField(index);
    MemPtr fieldPtr = GetMemPtr() + field.Offset();
    switch (field.GetType())
    {
        case ValueType::boolType: return IntegralValue(*static_cast<bool*>(fieldPtr.Value()), field.GetType());
        case ValueType::sbyteType: return IntegralValue(*static_cast<int8_t*>(fieldPtr.Value()), field.GetType());
        case ValueType::byteType: return IntegralValue(*static_cast<uint8_t*>(fieldPtr.Value()), field.GetType());
        case ValueType::shortType: return IntegralValue(*static_cast<int16_t*>(fieldPtr.Value()), field.GetType());
        case ValueType::ushortType: return IntegralValue(*static_cast<uint16_t*>(fieldPtr.Value()), field.GetType());
        case ValueType::intType: return IntegralValue(*static_cast<int32_t*>(fieldPtr.Value()), field.GetType());
        case ValueType::uintType: return IntegralValue(*static_cast<uint32_t*>(fieldPtr.Value()), field.GetType());
        case ValueType::longType: return IntegralValue(*static_cast<int64_t*>(fieldPtr.Value()), field.GetType());
        case ValueType::ulongType: return IntegralValue(*static_cast<uint64_t*>(fieldPtr.Value()), field.GetType());
        case ValueType::floatType: return IntegralValue(static_cast<uint64_t>(*static_cast<float*>(fieldPtr.Value())), field.GetType());
        case ValueType::doubleType: return IntegralValue(static_cast<uint64_t>(*static_cast<double*>(fieldPtr.Value())), field.GetType());
        case ValueType::charType: return IntegralValue(*static_cast<char32_t*>(fieldPtr.Value()), field.GetType());
        case ValueType::memPtr: return IntegralValue(static_cast<uint8_t*>(fieldPtr.Value()));
        case ValueType::classDataPtr: return IntegralValue(*static_cast<ClassData**>(fieldPtr.Value()));
        case ValueType::typePtr: return IntegralValue(*static_cast<ObjectType**>(fieldPtr.Value()));
        case ValueType::stringLiteral: return IntegralValue(static_cast<const char32_t*>(fieldPtr.Value()));
        case ValueType::allocationHandle: return IntegralValue(*static_cast<uint64_t*>(fieldPtr.Value()), field.GetType());
        case ValueType::objectReference: return IntegralValue(*static_cast<uint64_t*>(fieldPtr.Value()), field.GetType());
        case ValueType::functionPtr: return IntegralValue(*static_cast<Function**>(fieldPtr.Value()));
        default: return IntegralValue(*static_cast<uint64_t*>(fieldPtr.Value()), field.GetType());
    }
}

void Object::SetField(IntegralValue fieldValue, int index)
{
    Field field = type->GetField(index);
    Assert(field.GetType() == fieldValue.GetType(), "value types do not match in set field");
    MemPtr fieldPtr = GetMemPtr() + field.Offset();
    switch (field.GetType())
    {
        case ValueType::boolType: *static_cast<bool*>(fieldPtr.Value()) = fieldValue.AsBool(); break;
        case ValueType::sbyteType: *static_cast<int8_t*>(fieldPtr.Value()) = fieldValue.AsSByte(); break;
        case ValueType::byteType: *static_cast<uint8_t*>(fieldPtr.Value()) = fieldValue.AsByte(); break;
        case ValueType::shortType: *static_cast<int16_t*>(fieldPtr.Value()) = fieldValue.AsShort(); break;
        case ValueType::ushortType: *static_cast<uint16_t*>(fieldPtr.Value()) = fieldValue.AsUShort(); break;
        case ValueType::intType: *static_cast<int32_t*>(fieldPtr.Value()) = fieldValue.AsInt(); break;
        case ValueType::uintType: *static_cast<uint32_t*>(fieldPtr.Value()) = fieldValue.AsUInt(); break;
        case ValueType::longType: *static_cast<int64_t*>(fieldPtr.Value()) = fieldValue.AsLong(); break;
        case ValueType::ulongType: *static_cast<uint64_t*>(fieldPtr.Value()) = fieldValue.AsULong(); break;
        case ValueType::floatType: *static_cast<float*>(fieldPtr.Value()) = fieldValue.AsFloat(); break;
        case ValueType::doubleType: *static_cast<double*>(fieldPtr.Value()) = fieldValue.AsDouble(); break;
        case ValueType::charType: *static_cast<char32_t*>(fieldPtr.Value()) = fieldValue.AsChar(); break;
        case ValueType::memPtr: *static_cast<uint8_t**>(fieldPtr.Value()) = fieldValue.AsMemPtr(); break;
        case ValueType::classDataPtr: *static_cast<ClassData**>(fieldPtr.Value()) = fieldValue.AsClassDataPtr(); break;
        case ValueType::typePtr: *static_cast<Type**>(fieldPtr.Value()) = fieldValue.AsTypePtr(); break;
        case ValueType::stringLiteral: *static_cast<const char32_t**>(fieldPtr.Value()) = fieldValue.AsStringLiteral(); break;
        case ValueType::allocationHandle: *static_cast<uint64_t*>(fieldPtr.Value()) = fieldValue.Value(); break;
        case ValueType::objectReference: *static_cast<uint64_t*>(fieldPtr.Value()) = fieldValue.Value(); break;
        case ValueType::functionPtr: *static_cast<Function**>(fieldPtr.Value()) = fieldValue.AsFunctionPtr(); break;
        default: throw SystemException("invalid field type " + std::to_string(int(field.GetType())));
    }
}

int32_t Object::FieldCount() const
{
    return type->FieldCount();
}

void Object::MarkLiveAllocations(std::unordered_set<AllocationHandle, AllocationHandleHash>& checked, ManagedMemoryPool& managedMemoryPool)
{
    int32_t n = type->FieldCount();
    for (int32_t i = 0; i < n; ++i)
    {
        IntegralValue value = GetField(i);
        if (value.GetType() == ValueType::objectReference)
        {
            ObjectReference objectRef(value.Value());
            if (!objectRef.IsNull() && checked.find(objectRef) == checked.cend())
            {
                checked.insert(objectRef);
                Object& object = managedMemoryPool.GetObject(objectRef);
                object.SetLive();
                object.MarkLiveAllocations(checked, managedMemoryPool);
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
                    ManagedAllocation* allocation = managedMemoryPool.GetAllocation(allocationHandle);
                    allocation->SetLive();
                    allocation->MarkLiveAllocations(checked, managedMemoryPool);
                }
            }
        }
    }
}

void Object::Set(ManagedMemoryPool* pool)
{
    pool->Set(this);
}

ArrayElements::ArrayElements(AllocationHandle handle_, MemPtr memPtr_, int32_t segmentId_, Type* elementType_, int32_t numElements_, uint64_t size_) :
    ManagedAllocation(handle_, memPtr_, segmentId_, size_), elementType(elementType_), numElements(numElements_)
{
}

IntegralValue ArrayElements::GetElement(int32_t index) const
{
    if (index < 0 || index >= numElements)
    {
        throw IndexOutOfRangeException("array index out of range");
    }
    ValueType valueType = elementType->GetValueType();
    MemPtr elementPtr = ElementPtr(GetMemPtr(), valueType, index);
    switch (valueType)
    {
        case ValueType::boolType: return IntegralValue(*static_cast<bool*>(elementPtr.Value()), valueType);
        case ValueType::sbyteType: return IntegralValue(*static_cast<int8_t*>(elementPtr.Value()), valueType);
        case ValueType::byteType: return IntegralValue(*static_cast<uint8_t*>(elementPtr.Value()), valueType);
        case ValueType::shortType: return IntegralValue(*static_cast<int16_t*>(elementPtr.Value()), valueType);
        case ValueType::ushortType: return IntegralValue(*static_cast<uint16_t*>(elementPtr.Value()), valueType);
        case ValueType::intType: return IntegralValue(*static_cast<int32_t*>(elementPtr.Value()), valueType);
        case ValueType::uintType: return IntegralValue(*static_cast<uint32_t*>(elementPtr.Value()), valueType);
        case ValueType::longType: return IntegralValue(*static_cast<int64_t*>(elementPtr.Value()), valueType);
        case ValueType::ulongType: return IntegralValue(*static_cast<uint64_t*>(elementPtr.Value()), valueType);
        case ValueType::floatType: return IntegralValue(static_cast<uint64_t>(*static_cast<float*>(elementPtr.Value())), valueType);
        case ValueType::doubleType: return IntegralValue(static_cast<uint64_t>(*static_cast<double*>(elementPtr.Value())), valueType);
        case ValueType::charType: return IntegralValue(*static_cast<char32_t*>(elementPtr.Value()), valueType);
        case ValueType::memPtr: return IntegralValue(static_cast<uint8_t*>(elementPtr.Value()));
        case ValueType::classDataPtr: return IntegralValue(*static_cast<ClassData**>(elementPtr.Value()));
        case ValueType::typePtr: return IntegralValue(*static_cast<ObjectType**>(elementPtr.Value()));
        case ValueType::stringLiteral: return IntegralValue(static_cast<const char32_t*>(elementPtr.Value()));
        case ValueType::allocationHandle: return IntegralValue(*static_cast<uint64_t*>(elementPtr.Value()), valueType);
        case ValueType::objectReference: return IntegralValue(*static_cast<uint64_t*>(elementPtr.Value()), valueType);
        default: return IntegralValue(*static_cast<uint64_t*>(elementPtr.Value()), valueType);
    }
}

void ArrayElements::SetElement(IntegralValue elementValue, int32_t index)
{
    if (index < 0 || index >= numElements) 
    {
        throw IndexOutOfRangeException("array index out of range");
    }
    ValueType valueType = elementType->GetValueType();
    MemPtr elementPtr = ElementPtr(GetMemPtr(), valueType, index);
    switch (valueType)
    {
        case ValueType::boolType: *static_cast<bool*>(elementPtr.Value()) = elementValue.AsBool(); break;
        case ValueType::sbyteType: *static_cast<int8_t*>(elementPtr.Value()) = elementValue.AsSByte(); break;
        case ValueType::byteType: *static_cast<uint8_t*>(elementPtr.Value()) = elementValue.AsByte(); break;
        case ValueType::shortType: *static_cast<int16_t*>(elementPtr.Value()) = elementValue.AsShort(); break;
        case ValueType::ushortType: *static_cast<uint16_t*>(elementPtr.Value()) = elementValue.AsUShort(); break;
        case ValueType::intType: *static_cast<int32_t*>(elementPtr.Value()) = elementValue.AsInt(); break;
        case ValueType::uintType: *static_cast<uint32_t*>(elementPtr.Value()) = elementValue.AsUInt(); break;
        case ValueType::longType: *static_cast<int64_t*>(elementPtr.Value()) = elementValue.AsLong(); break;
        case ValueType::ulongType: *static_cast<uint64_t*>(elementPtr.Value()) = elementValue.AsULong(); break;
        case ValueType::floatType: *static_cast<float*>(elementPtr.Value()) = elementValue.AsFloat(); break;
        case ValueType::doubleType: *static_cast<double*>(elementPtr.Value()) = elementValue.AsDouble(); break;
        case ValueType::charType: *static_cast<char32_t*>(elementPtr.Value()) = elementValue.AsChar(); break;
        case ValueType::memPtr: *static_cast<uint8_t**>(elementPtr.Value()) = elementValue.AsMemPtr(); break;
        case ValueType::classDataPtr: *static_cast<ClassData**>(elementPtr.Value()) = elementValue.AsClassDataPtr(); break;
        case ValueType::typePtr: *static_cast<Type**>(elementPtr.Value()) = elementValue.AsTypePtr(); break;
        case ValueType::stringLiteral: *static_cast<const char32_t**>(elementPtr.Value()) = elementValue.AsStringLiteral(); break;
        case ValueType::allocationHandle: *static_cast<uint64_t*>(elementPtr.Value()) = elementValue.Value(); break;
        case ValueType::objectReference: *static_cast<uint64_t*>(elementPtr.Value()) = elementValue.Value(); break;
        default: throw SystemException("invalid element type " + std::to_string(int(valueType)));
    }
}

void ArrayElements::Set(ManagedMemoryPool* pool)
{
    pool->Set(this);
}

void ArrayElements::MarkLiveAllocations(std::unordered_set<AllocationHandle, AllocationHandleHash>& checked, ManagedMemoryPool& managedMemoryPool)
{
    if (elementType->GetValueType() == ValueType::objectReference)
    {
        for (int32_t i = 0; i < numElements; ++i)
        {
            IntegralValue value = GetElement(i);
            Assert(value.GetType() == ValueType::objectReference, "object reference expected");
            ObjectReference objectReference(value.Value());
            if (!objectReference.IsNull() && checked.find(objectReference) == checked.cend())
            {
                checked.insert(objectReference);
                Object& object = managedMemoryPool.GetObject(objectReference);
                object.SetLive();
                object.MarkLiveAllocations(checked, managedMemoryPool);
            }
        }
    }
}

StringCharacters::StringCharacters(AllocationHandle handle_, MemPtr memPtr_, int32_t segmentId_, int32_t numChars_, uint64_t size_) :
    ManagedAllocation(handle_, memPtr_, segmentId_, size_), numChars(numChars_)
{
}

IntegralValue StringCharacters::GetChar(int32_t index) const
{
    MemPtr memPtr = GetMemPtr();
    return IntegralValue(memPtr.StrValue()[index], ValueType::charType);
}

void StringCharacters::Set(ManagedMemoryPool* pool)
{
    pool->Set(this);
}

uint64_t poolDiffSize = defaultPoolDiffSize;

void SetPoolDiffSize(uint64_t poolDiffSize_)
{
    poolDiffSize = poolDiffSize_;
}

uint64_t GetPoolDiffSize()
{
    return poolDiffSize;
}

ManagedMemoryPool::ManagedMemoryPool(Machine& machine_) : machine(machine_), nextReferenceValue(1), object(nullptr), arrayElements(nullptr), stringCharacters(nullptr),
    objectCount(0), arrayContentCount(0), stringContentCount(0), prevSize(0), size(0), poolRoot(AllocationHandle(0))
{
}

ObjectReference ManagedMemoryPool::CreateObject(Thread& thread, ObjectType* type)
{
    ObjectReference reference(nextReferenceValue++);
    std::pair<MemPtr, int32_t> memPtrSegmentId = machine.AllocateMemory(thread, type->ObjectSize());
    memPtrSegmentId.first.SetHashCode(Random64());
    Object* obj = new Object(reference, memPtrSegmentId.first, memPtrSegmentId.second, type, type->ObjectSize());
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex);
    auto pairItBool = allocations.insert(std::make_pair(reference, std::unique_ptr<ManagedAllocation>(obj)));
    if (!pairItBool.second)
    {
        throw SystemException("could not insert object to pool because an object with reference " + std::to_string(reference.Value()) + " already exists");
    }
    ++objectCount;
    CheckSize(thread, lock, reference);
    return reference;
}

ObjectReference ManagedMemoryPool::CopyObject(Thread& thread, ObjectReference from)
{
    if (from.IsNull())
    {
        return from;
    }
    ObjectReference reference(nextReferenceValue++);
    Object& object = GetObject(from);
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex);
    Object* obj = new Object(reference, object.GetMemPtr(), object.SegmentId(), object.GetType(), object.Size());
    auto pairItBool = allocations.insert(std::make_pair(reference, std::unique_ptr<ManagedAllocation>(obj)));
    if (!pairItBool.second)
    {
        throw SystemException("could not insert object to pool because an object with reference " + std::to_string(reference.Value()) + " already exists");
    }
    ++objectCount;
    CheckSize(thread, lock, reference);
    return reference;
}

void ManagedMemoryPool::DestroyAllocation(AllocationHandle handle)
{
    if (!handle.Value())
    {
        throw SystemException("cannot destroy null handle");
    }
    auto it = allocations.find(handle);
    if (it != allocations.cend())
    {
        ManagedAllocation* allocation = it->second.get();
        if (dynamic_cast<Object*>(allocation))
        {
            --objectCount;
        }
        else if (dynamic_cast<ArrayElements*>(allocation))
        {
            --arrayContentCount;
        }
        else if (dynamic_cast<StringCharacters*>(allocation))
        {
            --stringContentCount;
        }
    }
    auto n = allocations.erase(handle);
    if (n != 1)
    {
        throw SystemException("could not erase: allocation with handle " + std::to_string(handle.Value()) + " not found");
    }

}

Object& ManagedMemoryPool::GetObject(ObjectReference reference)
{
    if (reference.IsNull())
    {
        throw NullReferenceException("object reference is null");
    }
    std::lock_guard<std::recursive_mutex> lock(allocationsMutex);
    auto it = allocations.find(reference);
    if (it != allocations.cend())
    {
        ManagedAllocation* allocation = it->second.get();
        object = nullptr;
        allocation->Set(this);
        Assert(object, "object expected");
        return *object;
    }
    throw SystemException("object with reference " + std::to_string(reference.Value()) + " not found");
}

IntegralValue ManagedMemoryPool::GetField(ObjectReference reference, int32_t fieldIndex)
{
    if (reference.IsNull())
    {
        throw NullReferenceException("cannot get field of a null object reference");
    }
    Object& object = GetObject(reference);
    return object.GetField(fieldIndex);
}

void ManagedMemoryPool::SetField(ObjectReference reference, int32_t fieldIndex, IntegralValue fieldValue)
{
    if (reference.IsNull())
    {
        throw NullReferenceException("cannot set field of a null object reference");
    }
    Object& object = GetObject(reference);
    object.SetField(fieldValue, fieldIndex);
}

AllocationHandle ManagedMemoryPool::CreateStringCharsFromLiteral(Thread& thread, const char32_t* strLit, uint32_t len)
{
    AllocationHandle handle(nextReferenceValue++);
    uint64_t stringSize = static_cast<uint64_t>(sizeof(char32_t)) * len;
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex);
    MemPtr memPtr(strLit);
    memPtr.SetHashCode(Random64());
    StringCharacters* strChars = new StringCharacters(handle, memPtr, notGarbageCollectedSegment, len, stringSize);
    auto pairItBool = allocations.insert(std::make_pair(handle, std::unique_ptr<ManagedAllocation>(strChars)));
    if (!pairItBool.second)
    {
        throw SystemException("could not insert object to pool because an object with handle " + std::to_string(handle.Value()) + " already exists");
    }
    ++stringContentCount;
    CheckSize(thread, lock, handle);
    return handle;
}

std::pair<AllocationHandle, int32_t> ManagedMemoryPool::CreateStringCharsFromCharArray(Thread& thread, ObjectReference charArray)
{
    Object& charArrayObject = GetObject(charArray);
    IntegralValue charElementsValue = charArrayObject.GetField(2);
    Assert(charElementsValue.GetType() == ValueType::allocationHandle, "allocation handle expected");
    AllocationHandle charElementsHandle(charElementsValue.Value());
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex);
    auto it = allocations.find(charElementsHandle);
    if (it != allocations.cend())
    {
        ManagedAllocation* allocation = it->second.get();
        lock.unlock();
        ArrayElements* charElements = dynamic_cast<ArrayElements*>(allocation);
        Assert(charElements, "array elements expected");
        int32_t numChars = charElements->NumElements();
        MemPtr characters = charElements->GetMemPtr();
        AllocationHandle handle(nextReferenceValue++);
        uint64_t stringSize = numChars * ValueSize(ValueType::charType);
        if (stringSize > 0)
        {
            std::pair<MemPtr, int32_t> memPtrSegmentId = machine.AllocateMemory(thread, stringSize);
            memPtrSegmentId.first.SetHashCode(Random64());
            StringCharacters* strChars = new StringCharacters(handle, memPtrSegmentId.first, memPtrSegmentId.second, numChars, stringSize);
            lock.lock();
            auto pairItBool = allocations.insert(std::make_pair(handle, std::unique_ptr<ManagedAllocation>(strChars)));
            if (!pairItBool.second)
            {
                throw SystemException("could not insert object to pool because an object with handle " + std::to_string(handle.Value()) + " already exists");
            }
            MemPtr stringMem = memPtrSegmentId.first;
            std::memcpy(stringMem.Value(), characters.Value(), stringSize);
            ++stringContentCount;
            CheckSize(thread, lock, handle);
            return std::make_pair(handle, numChars);
        }
        else
        {
            lock.lock();
            StringCharacters* strChars = new StringCharacters(handle, MemPtr(), notGarbageCollectedSegment, numChars, stringSize);
            auto pairItBool = allocations.insert(std::make_pair(handle, std::unique_ptr<ManagedAllocation>(strChars)));
            if (!pairItBool.second)
            {
                throw SystemException("could not insert object to pool because an object with handle " + std::to_string(handle.Value()) + " already exists");
            }
            ++stringContentCount;
            CheckSize(thread, lock, handle);
            return std::make_pair(handle, numChars);
        }
    }
    else
    {
        throw SystemException("array element allocation with handle " + std::to_string(charElementsHandle.Value()) + " not found");
    }
}

ObjectReference ManagedMemoryPool::CreateString(Thread& thread, const utf32_string& s)
{
    Type* type = TypeTable::Instance().GetType(StringPtr(U"System.String"));
    ObjectType* stringType = dynamic_cast<ObjectType*>(type);
    Assert(stringType, "object type expected");
    ObjectReference str = CreateObject(thread, stringType);
    int32_t numChars = int32_t(s.length());
    AllocationHandle handle(nextReferenceValue++);
    uint64_t stringSize = numChars * ValueSize(ValueType::charType);
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex);
    if (stringSize > 0)
    {
        lock.unlock();
        std::pair<MemPtr, int32_t> memPtrSegmentId = machine.AllocateMemory(thread, stringSize);
        memPtrSegmentId.first.SetHashCode(Random64());
        StringCharacters* strChars = new StringCharacters(handle, memPtrSegmentId.first, memPtrSegmentId.second, numChars, stringSize);
        lock.lock();
        auto pairItBool = allocations.insert(std::make_pair(handle, std::unique_ptr<ManagedAllocation>(strChars)));
        if (!pairItBool.second)
        {
            throw SystemException("could not insert object to pool because an object with handle " + std::to_string(handle.Value()) + " already exists");
        }
        MemPtr stringMem = memPtrSegmentId.first;
        std::memcpy(stringMem.Value(), s.c_str(), stringSize);
        ++stringContentCount;
        CheckSize(thread, lock, handle);
    }
    else
    {
        auto pairItBool = allocations.insert(std::make_pair(handle, std::unique_ptr<ManagedAllocation>(new StringCharacters(handle, MemPtr(), notGarbageCollectedSegment, numChars, stringSize))));
        if (!pairItBool.second)
        {
            throw SystemException("could not insert object to pool because an object with handle " + std::to_string(handle.Value()) + " already exists");
        }
        ++stringContentCount;
        CheckSize(thread, lock, handle);
    }
    Object& strObject = GetObject(str);
    ClassData* classDataPtr = ClassDataTable::Instance().GetClassData(StringPtr(U"System.String"));
    strObject.SetField(IntegralValue(classDataPtr), 0);
    strObject.SetField(IntegralValue(numChars, ValueType::intType), 1);
    strObject.SetField(handle, 2);
    return str;
}

IntegralValue ManagedMemoryPool::GetStringChar(ObjectReference str, int32_t index)
{
    if (str.IsNull())
    {
        throw NullReferenceException("cannot index a null string");
    }
    Object& o = GetObject(str);
    IntegralValue charsHandleValue = o.GetField(2);
    Assert(charsHandleValue.GetType() == ValueType::allocationHandle, "allocation handle expected");
    AllocationHandle handle(charsHandleValue.Value());
    std::lock_guard<std::recursive_mutex> lock(allocationsMutex);
    auto it = allocations.find(handle);
    if (it != allocations.cend())
    {
        ManagedAllocation* allocation = it->second.get();
        StringCharacters* chars = dynamic_cast<StringCharacters*>(allocation);
        Assert(chars, "string characters expected");
        return chars->GetChar(index);
    }
    else
    {
        throw SystemException("string characters allocation with handle " + std::to_string(handle.Value()) + " not found");
    }
}

std::string ManagedMemoryPool::GetUtf8String(ObjectReference str)
{
    if (str.IsNull())
    {
        throw NullReferenceException("cannot get value of a null string");
    }
    Object& o = GetObject(str);
    IntegralValue charsHandleValue = o.GetField(2);
    Assert(charsHandleValue.GetType() == ValueType::allocationHandle, "allocation handle expected");
    AllocationHandle handle(charsHandleValue.Value());
    std::lock_guard<std::recursive_mutex> lock(allocationsMutex);
    auto it = allocations.find(handle);
    if (it != allocations.cend())
    {
        ManagedAllocation* allocation = it->second.get();
        StringCharacters* chars = dynamic_cast<StringCharacters*>(allocation);
        Assert(chars, "string characters expected");
        utf32_string s;
        int32_t n = chars->NumChars();
        for (int32_t i = 0; i < n; ++i)
        {
            s.append(1, chars->GetChar(i).AsChar());
        }
        return ToUtf8(s);
    }
    else
    {
        throw SystemException("string characters allocation with handle " + std::to_string(handle.Value()) + " not found");
    }
}

std::vector<uint8_t> ManagedMemoryPool::GetBytes(ObjectReference arr)
{
    std::vector<uint8_t> bytes;
    if (arr.IsNull())
    {
        throw NullReferenceException("cannot get value of a null array");
    }
    Object& a = GetObject(arr);
    IntegralValue elementsHandleValue = a.GetField(2);
    Assert(elementsHandleValue.GetType() == ValueType::allocationHandle, "allocation handle expected");
    AllocationHandle handle(elementsHandleValue.Value());
    std::lock_guard<std::recursive_mutex> lock(allocationsMutex);
    auto it = allocations.find(handle);
    if (it != allocations.cend())
    {
        ManagedAllocation* allocation = it->second.get();
        ArrayElements* elements = dynamic_cast<ArrayElements*>(allocation);
        Assert(elements, "array elements expected");
        Assert(elements->GetElementType() == TypeTable::Instance().GetType(StringPtr(U"System.UInt8")), "byte array expected");
        int32_t n = elements->NumElements();
        for (int32_t i = 0; i < n; ++i)
        {
            bytes.push_back(elements->GetElement(i).AsByte());
        }
        return bytes;
    }
    else
    {
        throw SystemException("array elements allocation with handle " + std::to_string(handle.Value()) + " not found");
    }
}

void ManagedMemoryPool::SetBytes(ObjectReference arr, const std::vector<uint8_t>& bytes, int32_t count)
{
    if (arr.IsNull())
    {
        throw NullReferenceException("cannot set value of a null array");
    }
    Object& a = GetObject(arr);
    IntegralValue elementsHandleValue = a.GetField(2);
    Assert(elementsHandleValue.GetType() == ValueType::allocationHandle, "allocation handle expected");
    AllocationHandle handle(elementsHandleValue.Value());
    std::lock_guard<std::recursive_mutex> lock(allocationsMutex);
    auto it = allocations.find(handle);
    if (it != allocations.cend())
    {
        ManagedAllocation* allocation = it->second.get();
        ArrayElements* elements = dynamic_cast<ArrayElements*>(allocation);
        Assert(elements, "array elements expected");
        Assert(elements->GetElementType() == TypeTable::Instance().GetType(StringPtr(U"System.UInt8")), "byte array expected");
        for (int32_t i = 0; i < count; ++i)
        {
            elements->SetElement(IntegralValue(bytes[i], ValueType::byteType), i);
        }
    }
    else
    {
        throw SystemException("array elements allocation with handle " + std::to_string(handle.Value()) + " not found");
    }
}

void ManagedMemoryPool::AllocateArrayElements(Thread& thread, ObjectReference arr, Type* elementType, int32_t length)
{
    if (length < 0)
    {
        throw ArgumentOutOfRangeException("invalid array length");
    }
    Object& a = GetObject(arr);
    AllocationHandle handle(nextReferenceValue++);
    uint64_t arraySize = ValueSize(elementType->GetValueType()) * uint64_t(length);
    std::unique_lock<std::recursive_mutex> lock(allocationsMutex);
    if (arraySize > 0)
    {
        lock.unlock();
        std::pair<MemPtr, int32_t> memPtrSegmentId = machine.AllocateMemory(thread, arraySize);
        memPtrSegmentId.first.SetHashCode(Random64());
        lock.lock();
        auto pairItBool = allocations.insert(std::make_pair(handle, std::unique_ptr<ManagedAllocation>(new ArrayElements(handle, memPtrSegmentId.first, memPtrSegmentId.second, elementType, length, arraySize))));
        if (!pairItBool.second)
        {
            throw SystemException("could not insert object to pool because an object with handle " + std::to_string(handle.Value()) + " already exists");
        }
        ++arrayContentCount;
        CheckSize(thread, lock, handle);
    }
    else
    {
        auto pairItBool = allocations.insert(std::make_pair(handle, std::unique_ptr<ManagedAllocation>(new ArrayElements(handle, MemPtr(), notGarbageCollectedSegment, elementType, length, arraySize))));
        if (!pairItBool.second)
        {
            throw SystemException("could not insert object to pool because an object with handle " + std::to_string(handle.Value()) + " already exists");
        }
        ++arrayContentCount;
        CheckSize(thread, lock, handle);
    }
    a.SetField(handle, 2);
}

IntegralValue ManagedMemoryPool::GetArrayElement(ObjectReference reference, int32_t index)
{
    if (reference.IsNull())
    { 
        throw NullReferenceException("cannot get item of a null array");
    }
    Object& arr = GetObject(reference);
    IntegralValue elementsHandleValue = arr.GetField(2);
    Assert(elementsHandleValue.GetType() == ValueType::allocationHandle, "allocation handle expected");
    AllocationHandle handle(elementsHandleValue.Value());
    std::lock_guard<std::recursive_mutex> lock(allocationsMutex);
    auto it = allocations.find(handle);
    if (it != allocations.cend())
    {
        ManagedAllocation* allocation = it->second.get();
        ArrayElements* elements = dynamic_cast<ArrayElements*>(allocation);
        Assert(elements, "array elements expected");
        return elements->GetElement(index);
    }
    else
    {
        throw SystemException("array element allocation with handle " + std::to_string(handle.Value()) + " not found");
    }
}

void ManagedMemoryPool::SetArrayElement(ObjectReference reference, int32_t index, IntegralValue elementValue)
{
    if (reference.IsNull())
    {
        throw NullReferenceException("cannot set item of a null array");
    }
    Object& arr = GetObject(reference);
    IntegralValue elementsHandleValue = arr.GetField(2);
    Assert(elementsHandleValue.GetType() == ValueType::allocationHandle, "allocation handle expected");
    AllocationHandle handle(elementsHandleValue.Value());
    std::lock_guard<std::recursive_mutex> lock(allocationsMutex);
    auto it = allocations.find(handle);
    if (it != allocations.cend())
    {
        ManagedAllocation* allocation = it->second.get();
        ArrayElements* elements = dynamic_cast<ArrayElements*>(allocation);
        Assert(elements, "array elements expected");
        elements->SetElement(elementValue, index);
    }
    else
    {
        throw SystemException("array element allocation with handle " + std::to_string(handle.Value()) + " not found");
    }
}

int32_t ManagedMemoryPool::GetNumArrayElements(ObjectReference arr)
{
    if (arr.IsNull())
    {
        throw NullReferenceException("cannot get number of items of a null array");
    }
    Object& a = GetObject(arr);
    IntegralValue elementsHandleValue = a.GetField(2);
    Assert(elementsHandleValue.GetType() == ValueType::allocationHandle, "allocation handle expected");
    AllocationHandle handle(elementsHandleValue.Value());
    std::lock_guard<std::recursive_mutex> lock(allocationsMutex);
    auto it = allocations.find(handle);
    if (it != allocations.cend())
    {
        ManagedAllocation* allocation = it->second.get();
        ArrayElements* elements = dynamic_cast<ArrayElements*>(allocation);
        Assert(elements, "array elements expected");
        return elements->NumElements();
    }
    else
    {
        throw SystemException("array element allocation with handle " + std::to_string(handle.Value()) + " not found");
    }
}

ObjectReference ManagedMemoryPool::CreateStringArray(Thread& thread, const std::vector<utf32_string>& programArguments, ObjectType* argsArrayObjectType)
{
    ObjectReference arrayReference = CreateObject(thread, argsArrayObjectType);
    Object& object = GetObject(arrayReference);
    ClassData* classData = ClassDataTable::Instance().GetClassData(StringPtr(U"System.String[]"));
    object.SetField(IntegralValue(classData), 0);
    int32_t length = int32_t(programArguments.size());
    object.SetField(IntegralValue(length, ValueType::intType), 1);
    AllocateArrayElements(thread, arrayReference, TypeTable::Instance().GetType(U"System.String"), length);
    for (int32_t i = 0; i < length; ++i)
    {
        const utf32_string& arg = programArguments[i];
        ObjectReference argStr = CreateString(thread, arg);
        SetArrayElement(arrayReference, i, argStr);
    }
    return arrayReference;
}

MemPtr ManagedMemoryPool::GetMemPtr(AllocationHandle handle) 
{
    std::lock_guard<std::recursive_mutex> lock(allocationsMutex);
    auto it = allocations.find(handle);
    if (it != allocations.cend())
    {
        ManagedAllocation* allocation = it->second.get();
        return allocation->GetMemPtr();
    }
    else
    {
        throw SystemException("allocation with handle " + std::to_string(handle.Value()) + " not found");
    }
}

ManagedAllocation* ManagedMemoryPool::GetAllocation(AllocationHandle handle)
{
    std::lock_guard<std::recursive_mutex> lock(allocationsMutex);
    auto it = allocations.find(handle);
    if (it != allocations.cend())
    {
        ManagedAllocation* allocation = it->second.get();
        return allocation;
    }
    else
    {
        throw SystemException("allocation with handle " + std::to_string(handle.Value()) + " not found");
    }
}

void ManagedMemoryPool::ComputeSize()
{
    uint64_t allocationCount = allocations.size();
    size = allocationCount * allocationSize + objectCount * objectSize + arrayContentCount * arrayContentSize + stringContentCount * stringContentSize;
}

void ManagedMemoryPool::CheckSize(Thread& thread, std::unique_lock<std::recursive_mutex>& lock, AllocationHandle handle)
{
    ComputeSize();
    uint64_t sizeDiff = size - prevSize;
    if (sizeDiff >= GetPoolDiffSize())
    {
        poolRoot = handle;
        lock.unlock();
        thread.SetState(ThreadState::paused);
#ifdef GC_LOGGING
        LogMessage(">" + std::to_string(thread.Id()) + " (paused)");
#endif
        thread.GetMachine().GetGarbageCollector().RequestGarbageCollection(thread);
#ifdef GC_LOGGING
        LogMessage(">" + std::to_string(thread.Id()) + " (collection requested)");
#endif
        thread.GetMachine().GetGarbageCollector().WaitUntilGarbageCollected(thread);
#ifdef GC_LOGGING
        LogMessage(">" + std::to_string(thread.Id()) + " (collection ended)");
#endif
        ComputeSize();
        prevSize = size;
        poolRoot = AllocationHandle(0);
        thread.SetState(ThreadState::running);
#ifdef GC_LOGGING
        LogMessage(">" + std::to_string(thread.Id()) + " (running)");
#endif
        thread.GetMachine().GetGarbageCollector().WaitForIdle(thread);
    }
}

void ManagedMemoryPool::ResetLiveFlags()
{
    for (auto& p : allocations)
    {
        ManagedAllocation* allocation = p.second.get();
        allocation->ResetLive();
    }
}

void ManagedMemoryPool::MoveLiveAllocationsToArena(ArenaId fromArenaId, Arena& toArena)
{
    std::unordered_map<void*, std::pair<MemPtr, int32_t>> moveMap;
    std::vector<AllocationHandle> toBeDestroyed;
    for (auto& p : allocations)
    {
        ManagedAllocation* allocation = p.second.get();
        if (allocation->SegmentId() != notGarbageCollectedSegment)
        {
            if (machine.GetSegment(allocation->SegmentId())->GetArenaId() == fromArenaId)
            {
                if (allocation->IsLive())
                {
                    MemPtr oldMemPtr = allocation->GetMemPtr();
                    auto it = moveMap.find(oldMemPtr.Value());
                    if (it == moveMap.cend())
                    {
                        uint64_t n = allocation->Size();
                        std::pair<MemPtr, int32_t> newMemPtrSegmentId = toArena.Allocate(n);
                        newMemPtrSegmentId.first.SetHashCode(oldMemPtr.HashCode());
                        std::memcpy(newMemPtrSegmentId.first.Value(), oldMemPtr.Value(), n);
                        allocation->SetMemPtr(newMemPtrSegmentId.first);
                        allocation->SetSegmentId(newMemPtrSegmentId.second);
                        moveMap[oldMemPtr.Value()] = newMemPtrSegmentId;
                    }
                    else
                    {
                        MemPtr newMemPtr(it->second.first);
                        int32_t segmentId = it->second.second;
                        allocation->SetMemPtr(it->second.first);
                        allocation->SetSegmentId(segmentId);
                    }
                }
                else
                {
                    toBeDestroyed.push_back(allocation->Handle());
                }
            }
        }
    }
    for (AllocationHandle handle : toBeDestroyed)
    {
        DestroyAllocation(handle);
    }
}

struct SegmentIdLess
{
    bool operator()(ManagedAllocation* left, ManagedAllocation* right) const
    {
        return left->SegmentId() < right->SegmentId();
    }
};

void MoveSegment(Arena& arena, std::unordered_map<void*, std::pair<MemPtr, int32_t>>& moveMap, int32_t segmentId, 
    std::vector<ManagedAllocation*>::iterator segmentBegin, std::vector<ManagedAllocation*>::iterator segmentEnd, bool allocateNewSegment)
{
    for (auto sit = segmentBegin; sit != segmentEnd; ++sit)
    {
        ManagedAllocation* allocation = *sit;
        MemPtr oldMemPtr = allocation->GetMemPtr();
        auto mit = moveMap.find(oldMemPtr.Value());
        if (mit == moveMap.cend())
        {
            uint64_t n = allocation->Size();
            std::pair<MemPtr, int32_t> newMemPtrSegmentId = arena.Allocate(n, allocateNewSegment);
            newMemPtrSegmentId.first.SetHashCode(oldMemPtr.HashCode());
            allocateNewSegment = false;
            std::memcpy(newMemPtrSegmentId.first.Value(), oldMemPtr.Value(), n);
            allocation->SetMemPtr(newMemPtrSegmentId.first);
            allocation->SetSegmentId(newMemPtrSegmentId.second);
            moveMap[oldMemPtr.Value()] = newMemPtrSegmentId;
        }
        else
        {
            MemPtr newMemPtr(mit->second.first);
            int32_t segmentId = mit->second.second;
            allocation->SetMemPtr(newMemPtr);
            allocation->SetSegmentId(segmentId);
        }
    }
    arena.RemoveSegment(segmentId);
}

void ManagedMemoryPool::MoveLiveAllocationsToNewSegments(Arena& arena)
{
    std::unordered_map<void*, std::pair<MemPtr, int32_t>> moveMap;
    std::vector<ManagedAllocation*> liveAllocations;
    std::vector<AllocationHandle> toBeDestroyed;
    std::unordered_set<int32_t> liveSegments;
    for (auto& p : allocations)
    {
        ManagedAllocation* allocation = p.second.get();
        if (allocation->SegmentId() != notGarbageCollectedSegment)
        {
            if (machine.GetSegment(allocation->SegmentId())->GetArenaId() == arena.Id())
            {
                if (allocation->IsLive())
                {
                    liveAllocations.push_back(allocation);
                    liveSegments.insert(allocation->SegmentId());
                }
                else
                {
                    toBeDestroyed.push_back(allocation->Handle());
                }
            }
        }
    }
    arena.RemoveEmptySegments(liveSegments);
    std::sort(liveAllocations.begin(), liveAllocations.end(), SegmentIdLess());
    int32_t currentSegmentId = -1;
    auto segmentBegin = liveAllocations.begin();
    auto end = liveAllocations.end();
    bool firstMove = true;
    for (auto it = liveAllocations.begin(); it != end; ++it)
    {
        ManagedAllocation* liveAllocation = *it;
        if (liveAllocation->SegmentId() != currentSegmentId)
        {
            bool moveSegment = currentSegmentId != -1;
            if (it - segmentBegin == 1)
            {
                ManagedAllocation* allocation = *segmentBegin;
                if (allocation->Size() >= GetSegmentSize())
                {
                    moveSegment = false;
                }
            }
            if (moveSegment)
            {
                MoveSegment(arena, moveMap, currentSegmentId, segmentBegin, it, firstMove);
                firstMove = false;
            }
            currentSegmentId = liveAllocation->SegmentId();
            segmentBegin = it;
        }
    }
    bool moveSegment = currentSegmentId != -1;
    if (end - segmentBegin == 1)
    {
        ManagedAllocation* allocation = *segmentBegin;
        if (allocation->Size() >= GetSegmentSize())
        {
            moveSegment = false;
        }
    }
    if (moveSegment)
    {
        MoveSegment(arena, moveMap, currentSegmentId, segmentBegin, end, firstMove);
        firstMove = false;
    }
    for (AllocationHandle handle : toBeDestroyed)
    {
        DestroyAllocation(handle);
    }
}

} } // namespace cminor::machine
