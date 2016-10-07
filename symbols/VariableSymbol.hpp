// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_VARIABLE_SYMBOL_INCLUDED
#define CMINOR_SYMBOLS_VARIABLE_SYMBOL_INCLUDED
#include <cminor/symbols/Symbol.hpp>

namespace cminor { namespace symbols {

class VariableSymbol : public Symbol
{
public:
    VariableSymbol(const Span& span_, Constant name_);
private:
    TypeSymbol* type;
};

class ParameterSymbol : public VariableSymbol
{
public:
    ParameterSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::parameterSymbol; }
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    bool IsExportSymbol() const override;
};

class LocalVariableSymbol : public VariableSymbol
{
public:
    LocalVariableSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::localVariableSymbol; }
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    bool IsExportSymbol() const override;
};

class MemberVariableSymbol : public VariableSymbol
{
public:
    MemberVariableSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::memberVariableSymbol; }
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    bool IsExportSymbol() const override;
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_VARIABLE_SYMBOL_INCLUDED
