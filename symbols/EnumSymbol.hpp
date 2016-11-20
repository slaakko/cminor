// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_ENUM_SYMBOL_INCLUDED
#define CMINOR_SYMBOLS_ENUM_SYMBOL_INCLUDED
#include <cminor/symbols/Symbol.hpp>
#include <cminor/symbols/Value.hpp>

namespace cminor { namespace symbols {

class EnumTypeSymbol : public TypeSymbol
{
public:
    EnumTypeSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::enumTypeSymbol; }
    void SetSpecifiers(Specifiers specifiers);
    std::string TypeString() const override { return "enum type"; };
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void EmplaceType(TypeSymbol* typeSymbol, int index) override;
    TypeSymbol* GetUnderlyingType() const { return underlyingType; }
    void SetUnderlyingType(TypeSymbol* underlyingType_);
    Type* GetMachineType() const override;
private:
    TypeSymbol* underlyingType;
};

class EnumConstantSymbol : public Symbol
{
public:
    EnumConstantSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::enumConstantSymbol; }
    std::string TypeString() const override { return "enum constant"; };
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    Value* GetValue() const { return value.get(); }
    void SetValue(Value* value_);
    bool Evaluating() const { return evaluating; }
    void SetEvaluating() { evaluating = true; }
    void ResetEvaluating() { evaluating = false; }
    void Evaluate(SymbolEvaluator* evaluator, const Span& span);
    Constant GetConstant() const { return constant; }
private:
    std::unique_ptr<Value> value;
    bool evaluating;
    Constant constant;
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_ENUM_SYMBOL_INCLUDED
