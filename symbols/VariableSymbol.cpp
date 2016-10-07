// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/VariableSymbol.hpp>

namespace cminor { namespace symbols {

VariableSymbol::VariableSymbol(const Span& span_, Constant name_) : Symbol(span_, name_), type(nullptr)
{
}

ParameterSymbol::ParameterSymbol(const Span& span_, Constant name_) : VariableSymbol(span_, name_)
{
}

bool ParameterSymbol::IsExportSymbol() const
{
    return Source() == SymbolSource::project;
}

LocalVariableSymbol::LocalVariableSymbol(const Span& span_, Constant name_) : VariableSymbol(span_, name_)
{
}

bool LocalVariableSymbol::IsExportSymbol() const
{
    return Source() == SymbolSource::project;
}

MemberVariableSymbol::MemberVariableSymbol(const Span& span_, Constant name_) : VariableSymbol(span_, name_)
{
}

bool MemberVariableSymbol::IsExportSymbol() const
{
    return Source() == SymbolSource::project;
}

} } // namespace cminor::symbols
