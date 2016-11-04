// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/PropertySymbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>

namespace cminor { namespace symbols {

PropertySymbol::PropertySymbol(const Span& span_, Constant name_) : ContainerSymbol(span_, name_), type(nullptr), getter(nullptr), setter(nullptr)
{
}

void PropertySymbol::Write(SymbolWriter& writer)
{
    ContainerSymbol::Write(writer);
    utf32_string typeFullName = type->FullName();
    ConstantId id = GetAssembly()->GetConstantPool().GetIdFor(typeFullName);
    Assert(id != noConstantId, "got no id");
    id.Write(writer);
}

void PropertySymbol::Read(SymbolReader& reader)
{
    ContainerSymbol::Read(reader);
    ConstantId id;
    id.Read(reader);
    reader.EmplaceTypeRequest(this, id, 0);
}

void PropertySymbol::EmplaceType(TypeSymbol* type, int index)
{
    if (index == 0)
    {
        SetType(type);
    }
    else
    {
        throw std::runtime_error("property symbol emplace type got invalid type index " + std::to_string(index));
    }
}

void PropertySymbol::AddSymbol(std::unique_ptr<Symbol>&& symbol)
{
    Symbol* s = symbol.get();
    ContainerSymbol::AddSymbol(std::move(symbol));
    if (PropertyGetterFunctionSymbol* getter_ = dynamic_cast<PropertyGetterFunctionSymbol*>(s))
    {
        getter = getter_;
    }
    else if (PropertySetterFunctionSymbol* setter_ = dynamic_cast<PropertySetterFunctionSymbol*>(s))
    {
        setter = setter_;
    }
}

void PropertySymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        throw Exception("properties cannot be static", GetSpan());
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception("properties cannot be external", GetSpan());
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception("properties cannot be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception("properties cannot be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        throw Exception("properties cannot be abstract", GetSpan());
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        throw Exception("properties cannot be inline", GetSpan());
    }
}

PropertyGetterFunctionSymbol::PropertyGetterFunctionSymbol(const Span& span_, Constant name_) : FunctionSymbol(span_, name_)
{
}

PropertySetterFunctionSymbol::PropertySetterFunctionSymbol(const Span& span_, Constant name_) : FunctionSymbol(span_, name_)
{
}

} } // namespace cminor::symbols
