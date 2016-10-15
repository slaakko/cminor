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
    utf32_string returnTypeFullName;
    if (returnType)
    {
        returnTypeFullName = returnType->FullName();
    }
    ConstantId returnTypeNameId = GetAssembly()->GetConstantPool().GetIdFor(returnTypeFullName);
    returnTypeNameId.Write(writer);
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
    ConstantId returnTypeNameId;
    returnTypeNameId.Read(reader);
    if (returnTypeNameId != reader.GetAssembly()->GetConstantPool().GetEmptyStringConstantId())
    {
        reader.EmplaceTypeRequest(this, returnTypeNameId, 0);
    }
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

void FunctionSymbol::EmplaceType(TypeSymbol* type, int index)
{
    if (index == 0)
    {
        returnType = type;
    }
    else
    {
        throw std::runtime_error("function symbol emplace type got invalid type index " + std::to_string(index));
    }
}

void FunctionSymbol::ComputeName()
{
    utf32_string name;
    name.append(GroupName().Value());
    name.append(1, U'(');
    int n = int(Parameters().size());
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
        {
            name.append(U", ");
        }
        ParameterSymbol* parameter = Parameters()[i];
        name.append(parameter->GetType()->FullName());
        name.append(1, U' ');
        name.append(utf32_string(parameter->Name().Value()));
    }
    name.append(1, U')');
    SetName(StringPtr(name.c_str()));
}

utf32_string FunctionSymbol::FullName() const
{
    utf32_string fullName;
    utf32_string parentFullName = Parent()->FullName();
    fullName.append(parentFullName);
    if (!parentFullName.empty())
    {
        fullName.append(1, U'.');
    }
    fullName.append(GroupName().Value());
    fullName.append(1, U'(');
    int n = int(Parameters().size());
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
        {
            fullName.append(U", ");
        }
        ParameterSymbol* parameter = Parameters()[i];
        fullName.append(parameter->GetType()->FullName());
    }
    fullName.append(1, U')');
    return fullName;
}

utf32_string FunctionSymbol::FriendlyName() const
{
    utf32_string friendlyName;
    if (returnType)
    {
        friendlyName.append(returnType->FullName()).append(1, U' ');
    }
    utf32_string parentFullName = Parent()->FullName();
    friendlyName.append(parentFullName);
    if (!parentFullName.empty())
    {
        friendlyName.append(1, U'.');
    }
    friendlyName.append(GroupName().Value());
    friendlyName.append(1, U'(');
    int n = int(Parameters().size());
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
        {
            friendlyName.append(U", ");
        }
        ParameterSymbol* parameter = Parameters()[i];
        friendlyName.append(parameter->GetType()->FullName());
        friendlyName.append(1, U' ');
        friendlyName.append(utf32_string(parameter->Name().Value()));
    }
    friendlyName.append(1, U')');
    return friendlyName;
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
    utf32_string callName = FullName();
    ConstantPool& constantPool = assembly.GetConstantPool();
    Constant callNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(callName.c_str())));
    callInst->SetFunctionCallName(callNameConstant);
    function.AddInst(std::move(inst));
}

void FunctionSymbol::CreateMachineFunction()
{
    machineFunction = GetAssembly()->GetMachineFunctionTable().CreateFunction(this);
    int32_t numLocals = int32_t(parameters.size() + localVariables.size());
    machineFunction->SetNumLocals(numLocals);
    int32_t numParameters = int32_t(parameters.size());
    machineFunction->SetNumParameters(numParameters);
    machineFunction->AddInst(GetAssembly()->GetMachine().CreateInst("receive"));
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
