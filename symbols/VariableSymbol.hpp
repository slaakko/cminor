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
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    TypeSymbol* GetType()  const { return type; }
    void SetType(TypeSymbol* type_) { type = type_; }
private:
    TypeSymbol* type;
};

class ParameterSymbol : public VariableSymbol
{
public:
    ParameterSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::parameterSymbol; }
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    int32_t Index() const { return index; }
    void SetIndex(int32_t index_) { index = index_; }
private:
    int32_t index;
};

class LocalVariableSymbol : public VariableSymbol
{
public:
    LocalVariableSymbol(const Span& span_, Constant name_);
    void Read(SymbolReader& reader) override;
    SymbolType GetSymbolType() const override { return SymbolType::localVariableSymbol; }
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    int32_t Index() const { return index; }
    void SetIndex(int32_t index_) { index = index_; }
private:
    int32_t index;
};

class MemberVariableSymbol : public VariableSymbol
{
public:
    MemberVariableSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::memberVariableSymbol; }
    int32_t Index() const { return index; }
    void SetIndex(int32_t index_) { index = index_; }
private:
    int32_t index;
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_VARIABLE_SYMBOL_INCLUDED
