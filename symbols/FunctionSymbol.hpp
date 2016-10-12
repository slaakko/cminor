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
class LocalVariableSymbol;

enum class ConversionType : uint8_t
{
    implicit_, explicit_
};

enum class FunctionSymbolFlags : uint8_t
{
    none = 0,
    conversionFun = 1 << 0
};

inline FunctionSymbolFlags operator&(FunctionSymbolFlags left, FunctionSymbolFlags right)
{
    return FunctionSymbolFlags(uint8_t(left) & uint8_t(right));
}

inline FunctionSymbolFlags operator|(FunctionSymbolFlags left, FunctionSymbolFlags right)
{
    return FunctionSymbolFlags(uint8_t(left) | uint8_t(right));
}

class FunctionSymbol : public ContainerSymbol
{
public:
    FunctionSymbol(const Span& span_, Constant name_);
    SymbolType GetSymbolType() const override { return SymbolType::functionSymbol; }
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    StringPtr GroupName() const { return StringPtr(groupName.Value().AsStringLiteral()); }
    void SetGroupNameConstant(Constant groupName_) { groupName = groupName_; }
    virtual void ComputeName();
    int Arity() const { return int(parameters.size()); }
    void AddSymbol(std::unique_ptr<Symbol>&& symbol) override;
    const std::vector<ParameterSymbol*>& Parameters() const { return parameters; }
    TypeSymbol* ReturnType() const { return returnType; }
    void SetReturnType(TypeSymbol* returnType_) { returnType = returnType_;  }
    const std::vector<LocalVariableSymbol*>& LocalVariables() const { return localVariables; }
    void AddLocalVariable(LocalVariableSymbol* localVariable);
    bool IsConversionFun() const { return GetFlag(FunctionSymbolFlags::conversionFun);  }
    void SetConversionFun() { SetFlag(FunctionSymbolFlags::conversionFun); }
    virtual ConversionType GetConversionType() const { return ConversionType::implicit_; }
    virtual int ConversionDistance() const { return 0; }
    virtual TypeSymbol* ConversionSourceType() const { return nullptr; }
    virtual TypeSymbol* ConversionTargetType() const { return nullptr; }
    virtual void GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects);
    bool GetFlag(FunctionSymbolFlags flag) const { return (flags & flag) != FunctionSymbolFlags::none; }
    void SetFlag(FunctionSymbolFlags flag) { flags = flags | flag; }
    Function* MachineFunction() const { return machineFunction; }
    void CreateMachineFunction();
    void EmplaceType(TypeSymbol* type, int index) override;
private:
    Constant groupName;
    std::vector<ParameterSymbol*> parameters;
    std::vector<LocalVariableSymbol*> localVariables;
    TypeSymbol* returnType;
    FunctionSymbolFlags flags;
    Function* machineFunction;
};

class FunctionGroupSymbol : public Symbol
{
public:
    FunctionGroupSymbol(const Span& span_, Constant name_, ContainerScope* containerScope_);
    SymbolType GetSymbolType() const override { return SymbolType::functionGroupSymbol; }
    bool IsExportSymbol() const override { return false; }
    void AddFunction(FunctionSymbol* function);
    void CollectViableFunctions(int arity, std::unordered_set<FunctionSymbol*>& viableFunctions);
    FunctionSymbol* GetOverload() const;
private:
    std::unordered_map<int, std::vector<FunctionSymbol*>> arityFunctionListMap;
    ContainerScope* containerScope;
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_FUNCTION_SYMBOL_INCLUDED
