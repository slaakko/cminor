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

std::string FunctionSymbolFlagStr(FunctionSymbolFlags flags)
{
    std::string s;
    if ((flags & FunctionSymbolFlags::inline_) != FunctionSymbolFlags::none)
    {
        s.append("inline");
    }
    if ((flags & FunctionSymbolFlags::conversionFun) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("conversion");
    }
    return s;
}

FunctionSymbol::FunctionSymbol(const Span& span_, Constant name_) : ContainerSymbol(span_, name_), returnType(nullptr), flags(FunctionSymbolFlags::none), machineFunction(nullptr)
{
}

void FunctionSymbol::Write(SymbolWriter& writer)
{
    ContainerSymbol::Write(writer);
    ConstantId groupNameId = GetAssembly()->GetConstantPool().GetIdFor(groupName);
    Assert(groupNameId != noConstantId, "no id for group name found from constant pool");
    groupNameId.Write(writer.AsMachineWriter());
    utf32_string returnTypeFullName;
    if (returnType)
    {
        returnTypeFullName = returnType->FullName();
    }
    ConstantId returnTypeNameId = GetAssembly()->GetConstantPool().GetIdFor(returnTypeFullName);
    returnTypeNameId.Write(writer);
    writer.AsMachineWriter().Put(uint8_t(flags));
    bool hasMachineFunction = machineFunction != nullptr;
    writer.AsMachineWriter().Put(hasMachineFunction);
    if (hasMachineFunction)
    {
        writer.AsMachineWriter().Put(machineFunction->Id());
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

void FunctionSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        throw Exception("only member functions can be static", GetSpan());
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception("only member functions can be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception("only member functions can be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        throw Exception("only member functions can be abstract", GetSpan());
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        SetInline();
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
    friendlyName.append(ToUtf32(SymbolFlagStr(Symbol::Flags())));
    std::string functionSymbolFlagStr = FunctionSymbolFlagStr(flags);
    if (!functionSymbolFlagStr.empty())
    {
        if (!friendlyName.empty())
        {
            friendlyName.append(1, U' ');
        }
        friendlyName.append(ToUtf32(functionSymbolFlagStr));
    }
    if (returnType)
    {
        if (!friendlyName.empty())
        {
            friendlyName.append(1, U' ');
        }
        friendlyName.append(returnType->FullName());
    }
    utf32_string parentFullName = Parent()->FullName();
    if (!parentFullName.empty())
    {
        if (!friendlyName.empty())
        {
            friendlyName.append(1, U' ');
        }
        friendlyName.append(parentFullName);
        friendlyName.append(1, U'.');
    }
    else
    {
        if (!friendlyName.empty())
        {
            friendlyName.append(1, U' ');
        }
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

StaticConstructorSymbol::StaticConstructorSymbol(const Span& span_, Constant name_) : FunctionSymbol(span_, name_)
{
}

void StaticConstructorSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    SetStatic();
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception("static constructor cannot be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception("static constructor cannot be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        throw Exception("static constructor cannot be abstract", GetSpan());
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        throw Exception("static constructor cannot be inline", GetSpan());
    }
}

ConstructorSymbol::ConstructorSymbol(const Span& span_, Constant name_) : FunctionSymbol(span_, name_), flags(ConstructorSymbolFlags::none)
{
}

void ConstructorSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception("constructor cannot be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception("constructor cannot be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        throw Exception("constructor cannot be abstract", GetSpan());
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        throw Exception("constructor cannot be inline", GetSpan());
    }
}

void ConstructorSymbol::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects)
{
    int n = int(objects.size());
    for (int i = 1; i < n; ++i)
    {
        GenObject* genObject = objects[i];
        genObject->GenLoad(machine, function);
    }
    std::unique_ptr<Instruction> callInst = machine.CreateInst("call");
    CallInst* callInstruction = dynamic_cast<CallInst*>(callInst.get());
    Assert(callInst, "call inst expceted");
    utf32_string callName = FullName();
    ConstantPool& constantPool = assembly.GetConstantPool();
    Constant callNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(callName.c_str())));
    callInstruction->SetFunctionCallName(callNameConstant);
    function.AddInst(std::move(callInst));
}

void ConstructorSymbol::CreateMachineFunction()
{
    FunctionSymbol::CreateMachineFunction();
    std::unique_ptr<Instruction> loadLocal = GetAssembly()->GetMachine().CreateInst("loadlocal");
    LoadLocalInst* loadLocalInst = dynamic_cast<LoadLocalInst*>(loadLocal.get());
    Assert(loadLocalInst, "load local inst expected");
    MachineFunction()->AddInst(std::move(loadLocal));
    loadLocalInst->SetIndex(0);
    std::unique_ptr<Instruction> inst = GetAssembly()->GetMachine().CreateInst("setclassdata");
    SetClassDataInst* setClassDataInst = dynamic_cast<SetClassDataInst*>(inst.get());
    Assert(setClassDataInst, "set class data inst expected");
    ClassTypeSymbol* containingClass = ContainingClass();;
    Assert(containingClass, "containing class not found");
    utf32_string fullClassName = containingClass->FullName();
    ConstantPool& constantPool = GetAssembly()->GetConstantPool();
    Constant fullClassNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(fullClassName.c_str())));
    setClassDataInst->SetClassName(fullClassNameConstant);
    MachineFunction()->AddInst(std::move(inst));
    if (!BaseConstructorCallGenerated() && containingClass->BaseClass())
    {
        Assert(containingClass->BaseClass()->DefaultConstructorSymbol(), "base class has no default constructor");
        std::unique_ptr<Instruction> loadLocal = GetAssembly()->GetMachine().CreateInst("loadlocal");
        LoadLocalInst* loadLocalInst = dynamic_cast<LoadLocalInst*>(loadLocal.get());
        Assert(loadLocalInst, "load local inst expected");
        MachineFunction()->AddInst(std::move(loadLocal));
        loadLocalInst->SetIndex(0);
        std::vector<GenObject*> objects;
        containingClass->BaseClass()->DefaultConstructorSymbol()->GenerateCall(GetAssembly()->GetMachine(), *GetAssembly(), *MachineFunction(), objects);
    }
}

bool ConstructorSymbol::IsDefaultConstructorSymbol() const
{
    return Parameters().size() == 1;
}

MemberFunctionSymbol::MemberFunctionSymbol(const Span& span_, Constant name_) : FunctionSymbol(span_, name_), vmtIndex(-1)
{
}

void MemberFunctionSymbol::Write(SymbolWriter& writer)
{
    FunctionSymbol::Write(writer);
    writer.AsMachineWriter().Put(vmtIndex);
    writer.AsMachineWriter().Put(uint8_t(flags));
}

void MemberFunctionSymbol::Read(SymbolReader& reader)
{
    FunctionSymbol::Read(reader);
    vmtIndex = reader.GetInt();
    flags = MemberFunctionSymbolFlags(reader.GetByte());
}

void MemberFunctionSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        SetVirtual();
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        if (IsVirtual())
        {
            throw Exception("member function cannot be at the same time virtual and override", GetSpan());
        }
        SetOverride();
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        if (IsVirtual())
        {
            throw Exception("member function cannot be at the same time virtual and abstract", GetSpan());
        }
        if (IsOverride())
        {
            throw Exception("member function cannot be at the same time override and abstract", GetSpan());
        }
        SetAbstract();
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        SetInline();
    }
}


ClassTypeConversion::ClassTypeConversion(const Span& span_, Constant name_) : FunctionSymbol(span_, name_)
{
    SetConversionFun();
}

void ClassTypeConversion::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects) 
{
    std::unique_ptr<Instruction> inst;
    switch (conversionType)
    {
        case ConversionType::implicit_: inst = std::move(machine.CreateInst("upcast")); break;
        case ConversionType::explicit_: inst = std::move(machine.CreateInst("downcast")); break;
        default: throw std::runtime_error("invalid conversion type");
    }
    TypeInstruction* typeInstruction = dynamic_cast<TypeInstruction*>(inst.get());
    Assert(typeInstruction, "type instruction expected");
    utf32_string targetTypeClassName = targetType->FullName();
    ConstantPool& constantPool = assembly.GetConstantPool();
    Constant targetTypeClassNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(targetTypeClassName.c_str())));
    typeInstruction->SetClassName(targetTypeClassNameConstant);
    function.AddInst(std::move(inst));
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

ConversionTable::ConversionTable(Assembly& assembly_) : assembly(assembly_)
{
}

void ConversionTable::AddConversion(FunctionSymbol* conversionFun)
{
    TypeSymbol* sourceType = conversionFun->ConversionSourceType();
    TypeSymbol* targetType = conversionFun->ConversionTargetType();
    conversionMap[std::make_pair(sourceType, targetType)] = conversionFun;
}

FunctionSymbol* ConversionTable::GetConversion(TypeSymbol* sourceType, TypeSymbol* targetType)
{
    auto it = conversionMap.find(std::make_pair(sourceType, targetType));
    if (it != conversionMap.cend())
    {
        return it->second;
    }
    if (ClassTypeSymbol* sourceClassType = dynamic_cast<ClassTypeSymbol*>(sourceType))
    {
        if (ClassTypeSymbol* targetClassType = dynamic_cast<ClassTypeSymbol*>(targetType))
        {
            ConstantPool& constantPool = assembly.GetConstantPool();
            int distance = 0;
            if (sourceClassType->HasBaseClass(targetClassType, distance))
            {
                Constant groupName = constantPool.GetConstant(constantPool.Install(StringPtr(U"@conversion")));
                utf32_string conversionName = sourceType->FullName() + U"2" + targetType->FullName();
                Constant conversionNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(conversionName.c_str())));
                std::unique_ptr<ClassTypeConversion> conversion(new ClassTypeConversion(sourceClassType->GetSpan(), conversionNameConstant));
                FunctionSymbol* conversionFun = conversion.get();
                conversion->SetAssembly(&assembly);
                conversion->SetConversionType(ConversionType::implicit_);
                conversion->SetConversionDistance(distance);
                conversion->SetSourceType(sourceType);
                conversion->SetTargetType(targetType);
                conversionMap[std::make_pair(sourceType, targetType)] = conversionFun;
                classTypeConversions.push_back(std::move(conversion));
                return conversionFun;
            }
            else 
            {
                int distance = 0;
                if (targetClassType->HasBaseClass(sourceClassType, distance))
                {
                    Constant groupName = constantPool.GetConstant(constantPool.Install(StringPtr(U"@conversion")));
                    utf32_string conversionName = sourceType->FullName() + U"2" + targetType->FullName();
                    Constant conversionNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(conversionName.c_str())));
                    std::unique_ptr<ClassTypeConversion> conversion(new ClassTypeConversion(sourceClassType->GetSpan(), conversionNameConstant));
                    FunctionSymbol* conversionFun = conversion.get();
                    conversion->SetAssembly(&assembly);
                    conversion->SetConversionType(ConversionType::explicit_);
                    conversion->SetConversionDistance(distance);
                    conversion->SetSourceType(sourceType);
                    conversion->SetTargetType(targetType);
                    conversionMap[std::make_pair(sourceType, targetType)] = conversionFun;
                    classTypeConversions.push_back(std::move(conversion));
                    return conversionFun;
                }
            }
        }
    }
    return nullptr;
}

void ConversionTable::SetConversionMap(const std::unordered_map<std::pair<TypeSymbol*, TypeSymbol*>, FunctionSymbol*, ConversionTypeHash>& conversionMap_)
{
    conversionMap = conversionMap_;
}

void ConversionTable::ImportConversionMap(const std::unordered_map<std::pair<TypeSymbol*, TypeSymbol*>, FunctionSymbol*, ConversionTypeHash>& conversionMap_)
{
    for (const auto& x : conversionMap_)
    {
        conversionMap.insert(x);
    }
}

} } // namespace cminor::symbols
