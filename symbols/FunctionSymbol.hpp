// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_FUNCTION_SYMBOL_INCLUDED
#define CMINOR_SYMBOLS_FUNCTION_SYMBOL_INCLUDED
#include <cminor/symbols/Symbol.hpp>
#include <cminor/machine/GenObject.hpp>

namespace cminor { namespace symbols {

using namespace cminor::machine;

class Assembly;
class ParameterSymbol;

class FunctionSymbol : public ContainerSymbol
{
public:
    FunctionSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::functionSymbol; }
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    StringPtr GroupName() const { return StringPtr(groupName.Value().AsStringLiteral()); }
    void SetGroupNameConstant(Constant groupName_) { groupName = groupName_; }
    int Arity() const { return int(parameters.size()); }
    void AddSymbol(std::unique_ptr<Symbol>&& symbol) override;
    const std::vector<ParameterSymbol*>& Parameters() const { return parameters; }
    TypeSymbol* ReturnType() const { return returnType; }
    void SetReturnType(TypeSymbol* returnType_) { returnType = returnType_;  }
    virtual void GenerateCode(Machine& machine, Function& function, std::vector<GenObject*>& objects);
private:
    Constant groupName;
    std::vector<ParameterSymbol*> parameters;
    TypeSymbol* returnType;
};

class FunctionGroupSymbol : public Symbol
{
public:
    FunctionGroupSymbol(const Span& span_, Constant name_, ContainerScope* containerScope_);
    SymbolType GetSymbolType() const override { return SymbolType::functionGroupSymbol; }
    void AddFunction(FunctionSymbol* function);
    void CollectViableFunctions(int arity, std::unordered_set<FunctionSymbol*>& viableFunctions);
private:
    std::unordered_map<int, std::vector<FunctionSymbol*>> arityFunctionListMap;
    ContainerScope* containerScope;
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_FUNCTION_SYMBOL_INCLUDED
