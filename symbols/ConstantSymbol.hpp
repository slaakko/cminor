// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_CONSTANT_SYMBOL_INCLUDED
#define CMINOR_SYMBOLS_CONSTANT_SYMBOL_INCLUDED
#include <cminor/symbols/Symbol.hpp>
#include <cminor/symbols/Value.hpp>

namespace cminor { namespace symbols {

class ConstantSymbol : public Symbol
{
public:
    ConstantSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::constantSymbol; }
    std::string TypeString() const override { return "constant"; }
    void SetSpecifiers(Specifiers specifiers);
    TypeSymbol* GetType() const { return type; }
    void SetType(TypeSymbol* type_) { type = type_; }
    Value* GetValue() const { return value.get(); }
    void SetValue(Value* value_);
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void EmplaceType(TypeSymbol* type, int index) override;
    bool Evaluating() const { return evaluating; }
    void SetEvaluating() { evaluating = true; }
    void ResetEvaluating() { evaluating = false; }
    void Evaluate(SymbolEvaluator* evaluator, const Span& span) override;
    Constant GetConstant() const { return constant; }
private:
    TypeSymbol* type;
    std::unique_ptr<Value> value;
    bool evaluating;
    Constant constant;
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_CONSTANT_SYMBOL_INCLUDED
