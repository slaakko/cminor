// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/machine/Machine.hpp>

namespace cminor { namespace symbols {

FunctionSymbol::FunctionSymbol(const Span& span_, Constant name_) : ContainerSymbol(span_, name_), returnType(nullptr), flags(FunctionSymbolFlags::none), machineFunction(nullptr)
{
}

void FunctionSymbol::Write(SymbolWriter& writer)
{
    ContainerSymbol::Write(writer);
    ConstantId groupNameId = GetAssembly()->GetConstantPool().GetIdFor(groupName);
    Assert(groupNameId != noConstantId, "no id for group name found from constant pool");
    groupNameId.Write(static_cast<Writer&>(writer));
    static_cast<Writer&>(writer).Put(uint8_t(flags));
    bool hasMachineFunction = machineFunction != nullptr;
    static_cast<Writer&>(writer).Put(hasMachineFunction);
    if (hasMachineFunction)
    {
        static_cast<Writer&>(writer).Put(machineFunction->Id());
    }
}

void FunctionSymbol::Read(SymbolReader& reader)
{
    reader.ResetLocalVariables();
    ContainerSymbol::Read(reader);
    ConstantId groupNameId;
    groupNameId.Read(reader);
    groupName = reader.GetAssembly()->GetConstantPool().GetConstant(groupNameId);
    flags = FunctionSymbolFlags(reader.GetByte());
    bool hasMachineFunction = reader.GetBool();
    if (hasMachineFunction)
    {
        int32_t machineFunctionId = reader.GetInt();
        machineFunction = GetAssembly()->GetMachineFunctionTable().GetFunction(machineFunctionId);
        if (StringPtr(groupName.Value().AsStringLiteral()) == StringPtr(U"main"))
        {
            machineFunction->SetMain();
        }
        FunctionTable::Instance().AddFunction(machineFunction);
    }
    std::vector<LocalVariableSymbol*> readLocalVariables = reader.GetLocalVariables();
    int n = int(readLocalVariables.size());
    for (int i = 0; i < n; ++i)
    {
        LocalVariableSymbol* localVariable = readLocalVariables[i];
        AddLocalVariable(localVariable);
    }
}

void FunctionSymbol::AddSymbol(std::unique_ptr<Symbol>&& symbol)
{
    Symbol* s = symbol.get();
    ContainerSymbol::AddSymbol(std::move(symbol));
    if (ParameterSymbol* parameterSymbol = dynamic_cast<ParameterSymbol*>(s))
    {
        parameterSymbol->SetIndex(int32_t(parameters.size()));
        parameters.push_back(parameterSymbol);
    }
}

void FunctionSymbol::AddLocalVariable(LocalVariableSymbol* localVariable)
{
    localVariable->SetIndex(int32_t(parameters.size() + localVariables.size()));
    localVariables.push_back(localVariable);
}

void FunctionSymbol::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects)
{
    int n = int(objects.size());
    for (int i = 0; i < n; ++i)
    {
        GenObject* genObject = objects[i];
        genObject->GenLoad(machine, function);
    }
    std::unique_ptr<Instruction> inst = machine.CreateInst("call");
    CallInst* callInst = dynamic_cast<CallInst*>(inst.get());
    utf32_string fullName = FullName();
    ConstantPool& constantPool = assembly.GetConstantPool();
    Constant fullNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(fullName.c_str())));
    callInst->SetFunctionFullName(fullNameConstant);
    function.AddInst(std::move(inst));
}

void FunctionSymbol::CreateMachineFunction()
{
    machineFunction = GetAssembly()->GetMachineFunctionTable().CreateFunction(this);
    int32_t numLocals = int32_t(parameters.size() + localVariables.size());
    machineFunction->SetNumLocals(numLocals);
}

FunctionGroupSymbol::FunctionGroupSymbol(const Span& span_, Constant name_, ContainerScope* containerScope_) : Symbol(span_, name_), containerScope(containerScope_)
{
}

void FunctionGroupSymbol::AddFunction(FunctionSymbol* function)
{
    Assert(function->GroupName() == Name(), "attempt to insert a function with group name '" + ToUtf8(function->GroupName().Value()) + "' to wrong function group '" + ToUtf8(Name().Value()) + "'");
    int arity = function->Arity();
    std::vector<FunctionSymbol*>& functionList = arityFunctionListMap[arity];
    functionList.push_back(function);
}

void FunctionGroupSymbol::CollectViableFunctions(int arity, std::unordered_set<FunctionSymbol*>& viableFunctions)
{
    auto it = arityFunctionListMap.find(arity);
    if (it != arityFunctionListMap.cend())
    {
        std::vector<FunctionSymbol*>& functionList = it->second;
        for (FunctionSymbol* function : functionList)
        {
            viableFunctions.insert(function);
        }
    }
}

FunctionSymbol* FunctionGroupSymbol::GetOverload() const
{
    if (arityFunctionListMap.size() == 1)
    {
        const std::vector<FunctionSymbol*>& overloads = arityFunctionListMap.begin()->second;
        if (overloads.size() == 1) return overloads.front();
    }
    return nullptr;
}

} } // namespace cminor::symbols
