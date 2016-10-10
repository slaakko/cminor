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
    utf32_string typeFullName = type->FullName();
    ConstantId id = GetAssembly()->GetConstantPool().GetIdFor(typeFullName);
    Assert(id != noConstantId, "got no id");
    id.Write(writer);
}

void VariableSymbol::Read(SymbolReader& reader)
{
    ConstantId id;
    id.Read(reader);
    Constant constant = GetAssembly()->GetConstantPool().GetConstant(id);
    utf32_string typeFullName = constant.Value().AsStringLiteral();
    type = GetAssembly()->GetSymbolTable().GetType(typeFullName);
}

ParameterSymbol::ParameterSymbol(const Span& span_, Constant name_) : VariableSymbol(span_, name_), index(-1)
{
}

LocalVariableSymbol::LocalVariableSymbol(const Span& span_, Constant name_) : VariableSymbol(span_, name_), index(-1)
{
}

void LocalVariableSymbol::Read(SymbolReader& reader)
{
    VariableSymbol::Read(reader);
    reader.AddLocalVariable(this);
}

MemberVariableSymbol::MemberVariableSymbol(const Span& span_, Constant name_) : VariableSymbol(span_, name_), index(-1)
{
}

} } // namespace cminor::symbols
