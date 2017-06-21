// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/ConstantSymbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>

namespace cminor { namespace symbols {

ConstantSymbol::ConstantSymbol(const Span& span_, Constant name_) : Symbol(span_, name_), type(nullptr), value(), evaluating(false)
{
}

void ConstantSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        throw Exception("constants cannot be static", GetSpan());
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception("constants cannot be external", GetSpan());
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception("constants cannot be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception("constants cannot be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        throw Exception("constants cannot be abstract", GetSpan());
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        throw Exception("constants cannot be inline", GetSpan());
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        throw Exception("constants cannot be new", GetSpan());
    }
}

void ConstantSymbol::SetValue(Value* value_)
{
    value.reset(value_);
    constant.SetValue(value->GetIntegralValue());
    GetAssembly()->GetConstantPool().Install(constant);
}

void ConstantSymbol::Write(SymbolWriter& writer)
{
    Symbol::Write(writer);
    std::u32string typeName = type->FullName();
    ConstantPool& constantPool = GetAssembly()->GetConstantPool();
    ConstantId typeId = constantPool.GetIdFor(typeName);
    typeId.Write(writer);
    writer.Put(value.get());
    ConstantId id = GetAssembly()->GetConstantPool().GetIdFor(constant);
    Assert(id != noConstantId, "got no id for constant");
    id.Write(writer);
}

void ConstantSymbol::Read(SymbolReader& reader)
{
    Symbol::Read(reader);
    ConstantId typeId;
    typeId.Read(reader);
    reader.EmplaceTypeRequest(this, typeId, 0);
    value.reset(reader.GetValue());
    ConstantId id;
    id.Read(reader);
    constant = GetAssembly()->GetConstantPool().GetConstant(id);
}

void ConstantSymbol::EmplaceType(TypeSymbol* type, int index)
{
    if (index == 0)
    {
        this->type = type;
    }
    else
    {
        throw std::runtime_error("invalid emplace type index " + std::to_string(index));
    }
}

bool ConstantSymbol::Evaluate(SymbolEvaluator* evaluator, const Span& span, bool dontThrow)
{
    evaluator->EvaluateConstantSymbol(this);
    return true;
}

} } // namespace cminor::symbols
