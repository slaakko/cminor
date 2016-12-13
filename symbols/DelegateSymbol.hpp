// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_DELEGATE_SYMBOL_INCLUDED
#define CMINOR_SYMBOLS_DELEGATE_SYMBOL_INCLUDED
#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/symbols/BasicTypeFun.hpp>

namespace cminor { namespace symbols {

class DelegateTypeSymbol : public TypeSymbol
{
public:
    DelegateTypeSymbol(const Span& span_, Constant name_);
    int Arity() const { return int(parameters.size()); }
    SymbolType GetSymbolType() const override { return SymbolType::delegateTypeSymbol; }
    std::string TypeString() const override { return "delegate"; }
    void SetSpecifiers(Specifiers specifiers);
    void AddSymbol(std::unique_ptr<Symbol>&& symbol) override;
    TypeSymbol* GetReturnType() const { return returnType; }
    void SetReturnType(TypeSymbol* returnType_) { returnType = returnType_; }
    const std::vector<ParameterSymbol*>& Parameters() const { return parameters; }
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void EmplaceType(TypeSymbol* type, int index) override;
    Type* GetMachineType() const override { Assert(machineType, "machine type not set"); return machineType.get(); }
    bool IsDelegateType() const override { return true; }
    void DumpHeader(CodeFormatter& formatter) override;
private:
    TypeSymbol* returnType;
    std::vector<ParameterSymbol*> parameters;
    std::unique_ptr<Type> machineType;
};

class DelegateDefaultInit : public BasicTypeInit
{
public:
    DelegateDefaultInit(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::delegateDefaultInit; }
    void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start) override;
};

void CreateDelegateFun(Assembly& assembly, TypeSymbol* type);

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_DELEGATE_SYMBOL_INCLUDED
