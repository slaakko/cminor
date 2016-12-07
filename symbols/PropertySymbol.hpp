// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_PROPERTY_SYMBOL_INCLUDED
#define CMINOR_SYMBOLS_PROPERTY_SYMBOL_INCLUDED
#include <cminor/symbols/FunctionSymbol.hpp>

namespace cminor { namespace symbols {

class PropertyGetterFunctionSymbol;
class PropertySetterFunctionSymbol;

class PropertySymbol : public ContainerSymbol
{
public:
    PropertySymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::propertySymbol; }
    std::string TypeString() const override { return "property"; }
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void AddSymbol(std::unique_ptr<Symbol>&& symbol) override;
    TypeSymbol* GetType()  const { return type; }
    void SetType(TypeSymbol* type_) { type = type_; }
    void EmplaceType(TypeSymbol* type, int index) override;
    void SetSpecifiers(Specifiers specifiers);
    PropertyGetterFunctionSymbol* Getter() const { return getter; }
    PropertySetterFunctionSymbol* Setter() const { return setter; }
    void AddTo(ClassTypeSymbol* classTypeSymbol) override;
    void MergeTo(ClassTemplateSpecializationSymbol* classTemplateSpecializationSymbol) override;
    void Merge(const PropertySymbol& that);
private:
    TypeSymbol* type;
    PropertyGetterFunctionSymbol* getter;
    PropertySetterFunctionSymbol* setter;
};

class PropertyGetterFunctionSymbol : public FunctionSymbol
{
public:
    PropertyGetterFunctionSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::propertyGetterSymbol; }
    std::string TypeString() const override { return "getter"; }
    ParameterSymbol* GetThisParam() const override { return Parameters()[0]; }
};

class PropertySetterFunctionSymbol : public FunctionSymbol
{
public:
    PropertySetterFunctionSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::propertySetterSymbol; }
    std::string TypeString() const override { return "setter"; }
    ParameterSymbol* GetThisParam() const override { return Parameters()[0]; }
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_PROPERTY_SYMBOL_INCLUDED
