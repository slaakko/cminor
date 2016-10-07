// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Type.hpp>

namespace cminor { namespace machine {

Layout::Layout() : size(0)
{
}

void Layout::AddField(ValueType fieldType)
{
    FieldOffset fieldOffset(size);
    fields.push_back(Field(fieldType, fieldOffset));
    size += ValueSize(fieldType);
}

Type::~Type()
{
}

ArrayType::ArrayType(std::unique_ptr<Type>&& elementType_) : elementType(std::move(elementType_))
{
}

} } // namespace cminor::machine
