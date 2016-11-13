// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>

namespace cminor { namespace symbols {

VariableSymbol::VariableSymbol(const Span& span_, Constant name_) : Symbol(span_, name_), type(nullptr)
{
}

void VariableSymbol::Write(SymbolWriter& writer)
{
    Symbol::Write(writer);
    utf32_string typeFullName = type->FullName();
    ConstantId id = GetAssembly()->GetConstantPool().GetIdFor(typeFullName);
    Assert(id != noConstantId, "got no id");
    id.Write(writer);
}

void VariableSymbol::Read(SymbolReader& reader)
{
    Symbol::Read(reader);
    ConstantId id;
    id.Read(reader);
    reader.EmplaceTypeRequest(this, id, 0);
}

void VariableSymbol::EmplaceType(TypeSymbol* type, int index)
{
    if (index == 0)
    {
        SetType(type);
    }
    else
    {
        throw std::runtime_error("variable symbol emplace type got invalid type index " + std::to_string(index));
    }
}

ParameterSymbol::ParameterSymbol(const Span& span_, Constant name_) : VariableSymbol(span_, name_), index(-1)
{
}

void ParameterSymbol::Write(SymbolWriter& writer)
{
    VariableSymbol::Write(writer);
    writer.AsMachineWriter().PutEncodedUInt(index);
}

void ParameterSymbol::Read(SymbolReader& reader)
{
    VariableSymbol::Read(reader);
    index = reader.GetEncodedUInt();
}

LocalVariableSymbol::LocalVariableSymbol(const Span& span_, Constant name_) : VariableSymbol(span_, name_), index(-1)
{
}

void LocalVariableSymbol::Write(SymbolWriter& writer)
{
    VariableSymbol::Write(writer);
    writer.AsMachineWriter().PutEncodedUInt(index);
}

void LocalVariableSymbol::Read(SymbolReader& reader)
{
    VariableSymbol::Read(reader);
    index = reader.GetEncodedUInt();
    reader.AddLocalVariable(this);
}

MemberVariableSymbol::MemberVariableSymbol(const Span& span_, Constant name_) : VariableSymbol(span_, name_), index(-1)
{
}

void MemberVariableSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        SetStatic();
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception("member variables cannot be external", GetSpan());
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception("member variables cannot be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception("member variables cannot be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        throw Exception("member variables cannot be abstract", GetSpan());
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        throw Exception("member variables cannot be inline", GetSpan());
    }
}

void MemberVariableSymbol::Write(SymbolWriter& writer)
{
    VariableSymbol::Write(writer);
    writer.AsMachineWriter().PutEncodedUInt(index);
}

void MemberVariableSymbol::Read(SymbolReader& reader)
{
    VariableSymbol::Read(reader);
    index = reader.GetEncodedUInt();
}

void MemberVariableSymbol::AddTo(ClassTypeSymbol* classTypeSymbol)
{
    classTypeSymbol->Add(this);
}

} } // namespace cminor::symbols
