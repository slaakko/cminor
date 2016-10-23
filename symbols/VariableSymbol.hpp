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
    std::string TypeString() const override { return "variable symbol"; }
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    TypeSymbol* GetType()  const { return type; }
    void SetType(TypeSymbol* type_) { type = type_; }
    void EmplaceType(TypeSymbol* type, int index) override;
private:
    TypeSymbol* type;
};

class ParameterSymbol : public VariableSymbol
{
public:
    ParameterSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::parameterSymbol; }
    std::string TypeString() const override { return "parameter"; }
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
    std::string TypeString() const override { return "local variable"; }
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
    std::string TypeString() const override { return "member variable"; }
    int32_t Index() const { return index; }
    void SetIndex(int32_t index_) { index = index_; }
    void SetSpecifiers(Specifiers specifiers);
private:
    int32_t index;
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_VARIABLE_SYMBOL_INCLUDED
