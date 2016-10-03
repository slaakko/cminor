// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_FUNCTION_SYMBOL_INCLUDED
#define CMINOR_SYMBOLS_FUNCTION_SYMBOL_INCLUDED
#include <cminor/symbols/Symbol.hpp>

namespace cminor { namespace symbols {

class Assembly;
class ParameterSymbol;

class FunctionSymbol : public ContainerSymbol
{
public:
    FunctionSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::functionSymbol; }
    StringPtr GroupName() const { return StringPtr(groupName.Value().AsStringLiteral()); }
    void SetGroupName(StringPtr groupNameStr);
    int Arity() const { return int(parameters.size()); }
    void AddSymbol(std::unique_ptr<Symbol>&& symbol) override;
private:
    Constant groupName;
    std::vector<ParameterSymbol*> parameters;
};

class FunctionGroupSymbol : public Symbol
{
public:
    FunctionGroupSymbol(const Span& span_, Constant name_, ContainerScope* containerScope_);
    SymbolType GetSymbolType() const override { return SymbolType::functionGroupSymbol; }
    void AddFunction(FunctionSymbol* function);
private:
    std::unordered_map<int, std::vector<FunctionSymbol*>> arityFunctionListMap;
    ContainerScope* containerScope;
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_FUNCTION_SYMBOL_INCLUDED
