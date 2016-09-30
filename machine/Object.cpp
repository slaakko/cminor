// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Object.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Type.hpp>
#include <cminor/machine/String.hpp>

namespace cminor { namespace machine {

std::string ValueTypeStr(ValueType type)
{
    switch (type)
    {
        case ValueType::boolType: return "bool";
        case ValueType::sbyteType: return "sbyte";
        case ValueType::byteType: return "byte";
        case ValueType::shortType: return "short";
        case ValueType::ushortType: return "ushort";
        case ValueType::intType: return "int";
        case ValueType::uintType: return "uint";
        case ValueType::longType: return "long";
        case ValueType::ulongType: return "ulong";
        case ValueType::floatType: return "float";
        case ValueType::doubleType: return "double";
        case ValueType::charType: return "char";
        case ValueType::stringLiteral: return "string literal";
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
        case ValueType::stringLiteral: writer.Put(AsStringLiteral()); break;
        case ValueType::objectReference: throw std::runtime_error("cannot write object reference");
        default: throw std::runtime_error("invalid integral value type to write");
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
        case ValueType::objectReference: throw std::runtime_error("cannot read object reference");
        default: throw std::runtime_error("invalid integral value type to read");
    }
}


uint64_t Size(ValueType type)
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
        case ValueType::objectReference: return sizeof(uint64_t);
    }
    return sizeof(uint64_t);
}

ObjectLayout::ObjectLayout() : objectSize(0)
{
}

void ObjectLayout::AddField(ValueType fieldType)
{
    FieldOffset fieldOffset(objectSize);
    fields.push_back(Field(fieldType, fieldOffset));
    objectSize += Size(fieldType);
}

Object::Object(ObjectReference reference_, ArenaId arenaId_, ObjectMemPtr memPtr_, Type* type_) :
    reference(reference_), arenaId(arenaId_), memPtr(memPtr_), type(type_), flags(ObjectFlags::none)
{
}

IntegralValue Object::GetField(int index) const
{
    Field field = type->GetField(index);
    ObjectMemPtr fieldPtr = memPtr + field.Offset();
    switch (field.GetType())
    {
        case ValueType::boolType: return IntegralValue(*reinterpret_cast<bool*>(fieldPtr.Value()), field.GetType());
        case ValueType::sbyteType: return IntegralValue(*reinterpret_cast<int8_t*>(fieldPtr.Value()), field.GetType());
        case ValueType::byteType: return IntegralValue(*reinterpret_cast<uint8_t*>(fieldPtr.Value()), field.GetType());
        case ValueType::shortType: return IntegralValue(*reinterpret_cast<int16_t*>(fieldPtr.Value()), field.GetType());
        case ValueType::ushortType: return IntegralValue(*reinterpret_cast<uint16_t*>(fieldPtr.Value()), field.GetType());
        case ValueType::intType: return IntegralValue(*reinterpret_cast<int32_t*>(fieldPtr.Value()), field.GetType());
        case ValueType::uintType: return IntegralValue(*reinterpret_cast<uint32_t*>(fieldPtr.Value()), field.GetType());
        case ValueType::longType: return IntegralValue(*reinterpret_cast<int64_t*>(fieldPtr.Value()), field.GetType());
        case ValueType::ulongType: return IntegralValue(*reinterpret_cast<uint64_t*>(fieldPtr.Value()), field.GetType());
        case ValueType::floatType: return IntegralValue(static_cast<uint64_t>(*reinterpret_cast<float*>(fieldPtr.Value())), field.GetType());
        case ValueType::doubleType: return IntegralValue(static_cast<uint64_t>(*reinterpret_cast<double*>(fieldPtr.Value())), field.GetType());
        case ValueType::charType: return IntegralValue(*reinterpret_cast<char32_t*>(fieldPtr.Value()), field.GetType());
        case ValueType::objectReference: return IntegralValue(*reinterpret_cast<uint64_t*>(fieldPtr.Value()), field.GetType());
        default: return IntegralValue(*reinterpret_cast<uint64_t*>(fieldPtr.Value()), field.GetType());
    }
}

void Object::SetField(IntegralValue fieldValue, int index)
{
    Field field = type->GetField(index);
    assert(field.GetType() == fieldValue.GetType(), "value types do not match in set field");
    ObjectMemPtr fieldPtr = memPtr + field.Offset();
    switch (field.GetType())
    {
        case ValueType::boolType: *reinterpret_cast<bool*>(fieldPtr.Value()) = fieldValue.AsBool(); break;
        case ValueType::sbyteType: *reinterpret_cast<int8_t*>(fieldPtr.Value()) = fieldValue.AsSByte(); break;
        case ValueType::byteType: *reinterpret_cast<uint8_t*>(fieldPtr.Value()) = fieldValue.AsByte(); break;
        case ValueType::shortType: *reinterpret_cast<int16_t*>(fieldPtr.Value()) = fieldValue.AsShort(); break;
        case ValueType::ushortType: *reinterpret_cast<uint16_t*>(fieldPtr.Value()) = fieldValue.AsUShort(); break;
        case ValueType::intType: *reinterpret_cast<int32_t*>(fieldPtr.Value()) = fieldValue.AsInt(); break;
        case ValueType::uintType: *reinterpret_cast<uint32_t*>(fieldPtr.Value()) = fieldValue.AsUInt(); break;
        case ValueType::longType: *reinterpret_cast<int64_t*>(fieldPtr.Value()) = fieldValue.AsLong(); break;
        case ValueType::ulongType: *reinterpret_cast<uint64_t*>(fieldPtr.Value()) = fieldValue.AsULong(); break;
        case ValueType::floatType: *reinterpret_cast<float*>(fieldPtr.Value()) = fieldValue.AsFloat(); break;
        case ValueType::doubleType: *reinterpret_cast<double*>(fieldPtr.Value()) = fieldValue.AsDouble(); break;
        case ValueType::charType: *reinterpret_cast<char32_t*>(fieldPtr.Value()) = fieldValue.AsChar(); break;
        case ValueType::objectReference: *reinterpret_cast<uint64_t*>(fieldPtr.Value()) = fieldValue.Value(); break;
        default: throw std::runtime_error("invalid field type " + std::to_string(int(field.GetType())));
    }
}

int32_t Object::FieldCount() const
{
    return type->FieldCount();
}

uint64_t Object::Size() const 
{ 
    return type->ObjectSize(); 
}

void Object::MarkLiveObjects(std::unordered_set<ObjectReference, ObjectReferenceHash>& checked, ObjectPool& objectBool)
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
                Object& object = objectBool.GetObject(objectRef);
                object.SetLive();
                object.MarkLiveObjects(checked, objectBool);
            }
        }
    }
}

ObjectPool::ObjectPool(Machine& machine_) : machine(machine_), nextReferenceValue(1)
{
}

ObjectReference ObjectPool::CreateObject(Thread& thread, Type* type)
{
    ObjectReference reference(nextReferenceValue++);
    std::pair<ArenaId, ObjectMemPtr> arenaMemPtr = machine.AllocateMemory(thread, type->ObjectSize());
    auto pairItBool = objects.insert(std::make_pair(reference, Object(reference, arenaMemPtr.first, arenaMemPtr.second, type)));
    if (!pairItBool.second)
    {
        throw std::runtime_error("could not insert object to pool because an object with reference " + std::to_string(reference.Value()) + " already exists");
    }
    return reference;
}

ObjectReference ObjectPool::CreateString(Thread& thread, IntegralValue stringValue)
{
    if (stringValue.Value() == 0)
    {
        throw std::runtime_error("could not insert string to pool because of null string value");
    }
    ObjectReference reference(nextReferenceValue++);
    const char32_t* s = stringValue.AsStringLiteral();
    auto pairItBool = objects.insert(std::make_pair(reference, Object(reference, ArenaId(-1), ObjectMemPtr(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(s))), nullptr)));
    if (!pairItBool.second)
    {
        throw std::runtime_error("could not insert string to pool because an object with reference " + std::to_string(reference.Value()) + " already exists");
    }
    return reference;
}

void ObjectPool::DestroyObject(ObjectReference reference)
{
    if (reference.IsNull())
    {
        throw std::runtime_error("cannot destroy null reference");
    }
    auto n = objects.erase(reference);
    if (n != 1)
    {
        throw std::runtime_error("could not erase: object with reference " + std::to_string(reference.Value()) + " not found");
    }
}

Object& ObjectPool::GetObject(ObjectReference reference)
{
    if (reference.IsNull())
    {
        throw std::runtime_error("cannot get null reference from object pool");
    }
    auto it = objects.find(reference);
    if (it != objects.cend())
    {
        return it->second;
    }
    throw std::runtime_error("object with reference " + std::to_string(reference.Value()) + " not found");
}

IntegralValue ObjectPool::GetField(ObjectReference reference, int32_t fieldIndex)
{
    if (reference.IsNull())
    {
        throw std::runtime_error("cannot get field of null reference");
    }
    auto it = objects.find(reference);
    if (it != objects.cend())
    {
        const Object& object = it->second;
        return object.GetField(fieldIndex);
    }
    throw std::runtime_error("object with reference " + std::to_string(reference.Value()) + " not found");
}

void ObjectPool::SetField(ObjectReference reference, int32_t fieldIndex, IntegralValue fieldValue)
{
    if (reference.IsNull())
    {
        throw std::runtime_error("cannot set field of null reference");
    }
    auto it = objects.find(reference);
    if (it != objects.cend())
    {
        Object& object = it->second;
        object.SetField(fieldValue, fieldIndex);
    }
    else
    {
        throw std::runtime_error("object with reference " + std::to_string(reference.Value()) + " not found");
    }
}

ObjectMemPtr ObjectPool::GetObjectMemPtr(ObjectReference reference)
{
    if (reference.IsNull())
    {
        throw std::runtime_error("cannot retrieve pointer of null reference");
    }
    auto it = objects.find(reference);
    if (it != objects.cend())
    {
        const Object& object = it->second;
        return object.MemPtr();
    }
    throw std::runtime_error("object with reference " + std::to_string(reference.Value()) + " not found");
}

void ObjectPool::ResetObjectsLiveFlag()
{
    for (auto& objectRefObjectPair : objects)
    {
        Object& object = objectRefObjectPair.second;
        object.ResetLive();
    }
}

void ObjectPool::MoveLiveObjectsToArena(ArenaId fromArenaId, Arena& toArena)
{
    std::vector<ObjectReference> toBeDestroyed;
    for (auto& objectRefObjectPair : objects)
    {
        Object& object = objectRefObjectPair.second;
        if (object.GetArenaId() == fromArenaId)
        {
            if (object.IsLive())
            {
                uint64_t n = object.Size();
                ObjectMemPtr oldMemPtr = object.MemPtr();
                ObjectMemPtr newMemPtr = toArena.Allocate(n);
                std::memcpy(newMemPtr.Value(), oldMemPtr.Value(), n);
                object.SetMemPtr(newMemPtr);
                object.SetArenaId(toArena.Id());
            }
            else
            {
                toBeDestroyed.push_back(object.Reference());
            }
        }
    }
    for (ObjectReference reference : toBeDestroyed)
    {
        DestroyObject(reference);
    }
}

} } // namespace cminor::machine
