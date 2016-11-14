// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Type.hpp>
#include <cminor/machine/Writer.hpp>
#include <cminor/machine/Reader.hpp>

namespace cminor { namespace machine {

void FieldOffset::Write(Writer& writer)
{
    writer.PutEncodedUInt(value);
}

void FieldOffset::Read(Reader& reader)
{
    value = reader.GetEncodedUInt();
}

Field::Field() : fieldType(), offset()
{
}

void Field::Write(Writer& writer)
{
    writer.Put(uint8_t(fieldType));
    offset.Write(writer);
}

void Field::Read(Reader& reader)
{
    fieldType = ValueType(reader.GetByte());
    offset.Read(reader);
}

Layout::Layout() : size(0)
{
}

void Layout::AddFields(const std::vector<Field>& fields_)
{ 
    fields.insert(fields.end(), fields_.cbegin(), fields_.cend()); 
    for (const Field& field : fields_)
    {
        size += ValueSize(field.GetType());
    }
}

void Layout::AddField(ValueType fieldType)
{
    Assert(size < std::numeric_limits<uint32_t>::max(), "size too big");
    FieldOffset fieldOffset(static_cast<uint32_t>(size));
    fields.push_back(Field(fieldType, fieldOffset));
    size += ValueSize(fieldType);
}

void Layout::Write(Writer& writer)
{
    uint32_t numFields = uint32_t(fields.size());
    writer.PutEncodedUInt(numFields);
    for (uint32_t i = 0; i < numFields; ++i)
    {
        fields[i].Write(writer);
    }
    writer.Put(size);
}

void Layout::Read(Reader& reader)
{
    uint32_t numFields = reader.GetEncodedUInt();
    for (uint32_t i = 0; i < numFields; ++i)
    {
        Field field;
        field.Read(reader);
        fields.push_back(field);
    }
    size = reader.GetULong();
}

Type::Type() : name()
{
}

Type::~Type()
{
}

void Type::Write(Writer& writer)
{
    ConstantId nameId = writer.GetConstantPool()->GetIdFor(name);
    Assert(nameId != noConstantId, "got no id for name");
    nameId.Write(writer);
}

void Type::Read(Reader& reader)
{
    ConstantId nameId;
    nameId.Read(reader);
    name = reader.GetConstantPool()->GetConstant(nameId);
}

ObjectType::ObjectType() : id(0)
{
}

void ObjectType::Write(Writer& writer)
{
    Type::Write(writer);
    objectLayout.Write(writer);
}

void ObjectType::Read(Reader& reader)
{
    Type::Read(reader);
    objectLayout.Read(reader);
}

std::unique_ptr<TypeTable> TypeTable::instance;

void TypeTable::Init()
{
    instance.reset(new TypeTable());
}

void TypeTable::Done()
{
    instance.reset();
}

TypeTable::TypeTable()
{
}

TypeTable& TypeTable::Instance()
{
    Assert(instance, "object type table instance not set");
    return *instance;
}

Type* TypeTable::GetType(StringPtr fullTypeName)
{
    auto it = typeMap.find(fullTypeName);
    if (it != typeMap.cend())
    {
        return it->second;
    }
    else
    {
        throw std::runtime_error("type for type name '" + ToUtf8(fullTypeName.Value()) + "' not found");
    }
}

void TypeTable::SetType(Type* type)
{
    auto it = typeMap.find(type->Name());
    if (it != typeMap.cend())
    {
        throw std::runtime_error("type with type name '" + ToUtf8(type->Name().Value()) + "' already in use");
    }
    typeMap[type->Name()] = type;
}

ObjectType* GetBoxedType(ValueType valueType)
{
    switch (valueType)
    {
        case ValueType::sbyteType: 
        {
            Type* type = TypeTable::Instance().GetType(U"System.BoxedInt8");
            ObjectType* boxedType = dynamic_cast<ObjectType*>(type);
            Assert(boxedType, "object type expected");
            return boxedType;
        }
        case ValueType::byteType:
        {
            Type* type = TypeTable::Instance().GetType(U"System.BoxedUInt8");
            ObjectType* boxedType = dynamic_cast<ObjectType*>(type);
            Assert(boxedType, "object type expected");
            return boxedType;
        }
        case ValueType::shortType:
        {
            Type* type = TypeTable::Instance().GetType(U"System.BoxedInt16");
            ObjectType* boxedType = dynamic_cast<ObjectType*>(type);
            Assert(boxedType, "object type expected");
            return boxedType;
        }
        case ValueType::ushortType:
        {
            Type* type = TypeTable::Instance().GetType(U"System.BoxedUInt16");
            ObjectType* boxedType = dynamic_cast<ObjectType*>(type);
            Assert(boxedType, "object type expected");
            return boxedType;
        }
        case ValueType::intType:
        {
            Type* type = TypeTable::Instance().GetType(U"System.BoxedInt32");
            ObjectType* boxedType = dynamic_cast<ObjectType*>(type);
            Assert(boxedType, "object type expected");
            return boxedType;
        }
        case ValueType::uintType:
        {
            Type* type = TypeTable::Instance().GetType(U"System.BoxedUInt32");
            ObjectType* boxedType = dynamic_cast<ObjectType*>(type);
            Assert(boxedType, "object type expected");
            return boxedType;
        }
        case ValueType::longType:
        {
            Type* type = TypeTable::Instance().GetType(U"System.BoxedInt64");
            ObjectType* boxedType = dynamic_cast<ObjectType*>(type);
            Assert(boxedType, "object type expected");
            return boxedType;
        }
        case ValueType::ulongType:
        {
            Type* type = TypeTable::Instance().GetType(U"System.BoxedUInt64");
            ObjectType* boxedType = dynamic_cast<ObjectType*>(type);
            Assert(boxedType, "object type expected");
            return boxedType;
        }
        case ValueType::floatType:
        {
            Type* type = TypeTable::Instance().GetType(U"System.BoxedFloat");
            ObjectType* boxedType = dynamic_cast<ObjectType*>(type);
            Assert(boxedType, "object type expected");
            return boxedType;
        }
        case ValueType::doubleType:
        {
            Type* type = TypeTable::Instance().GetType(U"System.BoxedDouble");
            ObjectType* boxedType = dynamic_cast<ObjectType*>(type);
            Assert(boxedType, "object type expected");
            return boxedType;
        }
        case ValueType::charType:
        {
            Type* type = TypeTable::Instance().GetType(U"System.BoxedChar");
            ObjectType* boxedType = dynamic_cast<ObjectType*>(type);
            Assert(boxedType, "object type expected");
            return boxedType;
        }
        case ValueType::boolType:
        {
            Type* type = TypeTable::Instance().GetType(U"System.BoxedBoolean");
            ObjectType* boxedType = dynamic_cast<ObjectType*>(type);
            Assert(boxedType, "object type expected");
            return boxedType;
        }
    }
    Assert(false, "unknown value type");
    return nullptr;
}

void TypeInit()
{
    TypeTable::Init();
}

void TypeDone()
{
    TypeTable::Done();
}

} } // namespace cminor::machine
