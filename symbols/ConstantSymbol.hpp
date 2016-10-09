// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_CONSTANT_SYMBOL_INCLUDED
#define CMINOR_SYMBOLS_CONSTANT_SYMBOL_INCLUDED
#include <cminor/symbols/Symbol.hpp>

namespace cminor { namespace symbols {

class ConstantSymbol : public Symbol
{
public:
    ConstantSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::constantSymbol; }
    TypeSymbol* GetType() const { return type; }
    void SetType(TypeSymbol* type_) { type = type_; }
    void SetValue(Constant value_);
    Constant Value() const { return value; }
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    ConstantId GetId() const;
private:
    TypeSymbol* type;
    Constant value;
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_CONSTANT_SYMBOL_INCLUDED
