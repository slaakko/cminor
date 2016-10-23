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
    writer.Put(value);
}

void FieldOffset::Read(Reader& reader)
{
    value = reader.GetULong();
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

void Layout::AddField(ValueType fieldType)
{
    FieldOffset fieldOffset(size);
    fields.push_back(Field(fieldType, fieldOffset));
    size += ValueSize(fieldType);
}

void Layout::Write(Writer& writer)
{
    int32_t numFields = int32_t(fields.size());
    writer.Put(numFields);
    for (int32_t i = 0; i < numFields; ++i)
    {
        fields[i].Write(writer);
    }
    writer.Put(size);
}

void Layout::Read(Reader& reader)
{
    int32_t numFields = reader.GetInt();
    for (int32_t i = 0; i < numFields; ++i)
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
    staticLayout.Write(writer);
}

void ObjectType::Read(Reader& reader)
{
    Type::Read(reader);
    objectLayout.Read(reader);
    staticLayout.Read(reader);
}

ArrayType::ArrayType(std::unique_ptr<Type>&& elementType_) : elementType(std::move(elementType_))
{
}

} } // namespace cminor::machine
