// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/EnumSymbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>

namespace cminor { namespace symbols {

EnumTypeSymbol::EnumTypeSymbol(const Span& span_, Constant name_) : TypeSymbol(span_, name_)
{
}

void EnumTypeSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        throw Exception("enumerated types cannot be static", GetSpan());
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception("enumerated types cannot be external", GetSpan());
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception("enumerated types cannot be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception("enumerated types cannot be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        throw Exception("enumerated types cannot be abstract", GetSpan());
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        throw Exception("enumerated types cannot be inline", GetSpan());
    }

}

void EnumTypeSymbol::Write(SymbolWriter& writer)
{
    TypeSymbol::Write(writer);
    utf32_string underlyingTypeFullName = underlyingType->FullName();
    ConstantId underlyingTypeId = GetAssembly()->GetConstantPool().GetIdFor(underlyingTypeFullName);
    Assert(underlyingTypeId != noConstantId, "got no id");
    underlyingTypeId.Write(writer);
}

void EnumTypeSymbol::Read(SymbolReader& reader)
{
    TypeSymbol::Read(reader);
    ConstantId underlyingTypeId;
    underlyingTypeId.Read(reader);
    reader.EmplaceTypeRequest(this, underlyingTypeId, 0);
}

void EnumTypeSymbol::EmplaceType(TypeSymbol* typeSymbol, int index)
{
    if (index == 0)
    {
        underlyingType = typeSymbol;
    }
    else
    {
        throw std::runtime_error("invalid emplace type index");
    }
}

void EnumTypeSymbol::SetUnderlyingType(TypeSymbol* underlyingType_)
{
    underlyingType = underlyingType_;
    ConstantPool& constantPool = GetAssembly()->GetConstantPool();
    utf32_string underlyingTypeName = underlyingType->FullName();
    constantPool.Install(StringPtr(underlyingTypeName.c_str()));
}

Type* EnumTypeSymbol::GetMachineType() const
{
    Assert(underlyingType, "underlying type not set");
    return underlyingType->GetMachineType();
}

EnumConstantSymbol::EnumConstantSymbol(const Span& span_, Constant name_) : Symbol(span_, name_), evaluating(false)
{
}

void EnumConstantSymbol::Write(SymbolWriter& writer)
{
    value->Write(writer);
}

void EnumConstantSymbol::Read(SymbolReader& reader)
{
    value->Read(reader);
}

void EnumConstantSymbol::SetValue(Value* value_)
{
    value.reset(value_);
}

void EnumConstantSymbol::Evaluate(SymbolEvaluator* evaluator, const Span& span)
{
    evaluator->EvaluateEnumConstantSymbol(this);
}

} } // namespace cminor::symbols
