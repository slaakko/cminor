// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/symbols/DelegateSymbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/symbols/EnumTypeFun.hpp>
#include <cminor/symbols/ObjectFun.hpp>
#include <cminor/symbols/StringFun.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/util/Unicode.hpp>

namespace cminor { namespace symbols {

using namespace cminor::unicode;

std::string FunctionSymbolFlagStr(FunctionSymbolFlags flags)
{
    std::string s;
    if ((flags & FunctionSymbolFlags::inline_) != FunctionSymbolFlags::none)
    {
        s.append("inline");
    }
    if ((flags & FunctionSymbolFlags::external_) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("extern");
    }
    if ((flags & FunctionSymbolFlags::conversionFun) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("conversion");
    }
    if ((flags & FunctionSymbolFlags::exported) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("export");
    }
    return s;
}

FunctionSymbol::FunctionSymbol(const Span& span_, Constant name_) : 
    ContainerSymbol(span_, name_), returnType(nullptr), flags(FunctionSymbolFlags::none), machineFunction(nullptr), declarationBlockId(0), nextContainerScopeId(0)
{
}

void FunctionSymbol::Write(SymbolWriter& writer)
{
    ContainerSymbol::Write(writer);
    ConstantId groupNameId = GetAssembly()->GetConstantPool().GetIdFor(groupName);
    Assert(groupNameId != noConstantId, "no id for group name found from constant pool");
    groupNameId.Write(writer.AsMachineWriter());
    if (vmf.Value().AsStringLiteral() != nullptr)
    {
        writer.AsMachineWriter().Put(true);
        ConstantId vmfid = GetAssembly()->GetConstantPool().GetIdFor(vmf);
        Assert(vmfid != noConstantId, "no id for vm function id");
        vmfid.Write(writer.AsMachineWriter());
    }
    else
    {
        writer.AsMachineWriter().Put(false);
    }
    std::u32string returnTypeFullName;
    if (returnType)
    {
        returnTypeFullName = returnType->FullName();
    }
    ConstantId returnTypeNameId = GetAssembly()->GetConstantPool().GetIdFor(returnTypeFullName);
    Assert(returnTypeNameId != noConstantId, "got no id for return type");
    returnTypeNameId.Write(writer);
    writer.AsMachineWriter().Put(uint8_t(flags));
    bool hasMachineFunction = machineFunction != nullptr;
    writer.AsMachineWriter().Put(hasMachineFunction);
    if (hasMachineFunction)
    {
        Assembly* machineFunctionAssembly = reinterpret_cast<Assembly*>(machineFunction->GetAssembly());
        bool differentMachineFunctionAssembly = machineFunctionAssembly != nullptr && machineFunctionAssembly != GetAssembly();
        writer.AsMachineWriter().Put(differentMachineFunctionAssembly);
        if (differentMachineFunctionAssembly)
        {
            writer.AsMachineWriter().Put(ToUtf8(machineFunctionAssembly->Name().Value()));
        }
        writer.AsMachineWriter().PutEncodedUInt(machineFunction->Id());
    }
    uint32_t n = uint32_t(pcContainerScopeIdMap.size());
    writer.AsMachineWriter().PutEncodedUInt(n);
    for (const std::pair<uint32_t, uint32_t>& p : pcContainerScopeIdMap)
    {
        writer.AsMachineWriter().PutEncodedUInt(p.first);
        writer.AsMachineWriter().PutEncodedUInt(p.second);
    }
}

void FunctionSymbol::Read(SymbolReader& reader)
{
    reader.SetCurrentFunctionSymbol(this);
    reader.ResetLocalVariables();
    ContainerSymbol::Read(reader);
    ConstantId groupNameId;
    groupNameId.Read(reader);
    groupName = reader.GetAssembly()->GetConstantPool().GetConstant(groupNameId);
    bool hasVmFunctionId = reader.GetBool();
    if (hasVmFunctionId)
    {
        ConstantId vmfid;
        vmfid.Read(reader);
        vmf = reader.GetAssembly()->GetConstantPool().GetConstant(vmfid);
    }
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
        bool hasDifferentAssembly = reader.GetBool();
        if (hasDifferentAssembly)
        {
            std::string assemblyName = reader.GetUtf8String();
            uint32_t machineFunctionId = reader.GetEncodedUInt();
            Assembly* assembly = AssemblyTable::Instance().GetAssembly(assemblyName);
            machineFunction = assembly->GetMachineFunctionTable().GetFunction(machineFunctionId);
        }
        else
        {
            uint32_t machineFunctionId = reader.GetEncodedUInt();
            machineFunction = GetAssembly()->GetMachineFunctionTable().GetFunction(machineFunctionId);
        }
        machineFunction->SetFunctionSymbol(this);
        if (StringPtr(groupName.Value().AsStringLiteral()) == StringPtr(U"main"))
        {
            machineFunction->SetMain();
        }
        FunctionTable::AddFunction(machineFunction, reader.ReadingClassTemplateSpecialization());
    }
    uint32_t n = reader.GetEncodedUInt();
    for (uint32_t i = 0; i < n; ++i)
    {
        uint32_t pc = reader.GetEncodedUInt();
        uint32_t containerScopeId = reader.GetEncodedUInt();
        pcContainerScopeIdMap[pc] = containerScopeId;
    }
    std::vector<LocalVariableSymbol*> readLocalVariables = reader.GetLocalVariables();
    int nl = int(readLocalVariables.size());
    for (int i = 0; i < nl; ++i)
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
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        SetExternal();
        SetCanThrow();
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        throw Exception("only member functions can be new", GetSpan());
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
    std::u32string name;
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
        name.append(std::u32string(parameter->Name().Value()));
    }
    name.append(1, U')');
    SetName(StringPtr(name.c_str()));
}

std::u32string FunctionSymbol::FullName() const
{
    std::u32string fullName;
    std::u32string parentFullName = Parent()->FullName();
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

std::u32string FunctionSymbol::FullParsingName() const
{
    std::u32string fullParsingName;
    std::u32string parentFullName = Parent()->FullName();
    fullParsingName.append(parentFullName);
    if (!parentFullName.empty())
    {
        fullParsingName.append(1, U'.');
    }
    fullParsingName.append(GroupName().Value());
    fullParsingName.append(1, U'(');
    int n = int(Parameters().size());
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
        {
            fullParsingName.append(U", ");
        }
        ParameterSymbol* parameter = Parameters()[i];
        fullParsingName.append(parameter->GetType()->FullName());
    }
    fullParsingName.append(1, U')');
    return fullParsingName;
}

std::u32string FunctionSymbol::FullNameWithSpecifiers() const
{
    std::u32string fullName;
    fullName.append(ToUtf32(SymbolFlagStr(Symbol::Flags())));
    std::string functionSymbolFlagStr = FunctionSymbolFlagStr(flags);
    if (!functionSymbolFlagStr.empty())
    {
        if (!fullName.empty())
        {
            fullName.append(1, U' ');
        }
        fullName.append(ToUtf32(functionSymbolFlagStr));
    }
    if (returnType)
    {
        if (!fullName.empty())
        {
            fullName.append(1, U' ');
        }
        fullName.append(returnType->FullName());
    }
    std::u32string parentFullName = Parent()->FullName();
    if (!parentFullName.empty())
    {
        if (!fullName.empty())
        {
            fullName.append(1, U' ');
        }
        fullName.append(parentFullName);
        fullName.append(1, U'.');
    }
    else
    {
        if (!fullName.empty())
        {
            fullName.append(1, U' ');
        }
    }
    fullName.append(GroupName().Value());
    fullName.append(1, U'(');
    int start = 0;
    if (IsPropertyGetterOrSetter() || IsIndexerGetterOrSetter())
    {
        if (!IsStatic())
        {
            start = 1;
        }
    }
    bool first = true;
    int n = int(Parameters().size());
    for (int i = start; i < n; ++i)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            fullName.append(U", ");
        }
        ParameterSymbol* parameter = Parameters()[i];
        fullName.append(parameter->GetType()->FullName());
        fullName.append(1, U' ');
        fullName.append(std::u32string(parameter->Name().Value()));
    }
    fullName.append(1, U')');
    return fullName;
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

void FunctionSymbol::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    int n = int(objects.size());
    for (int i = start; i < n; ++i)
    {
        GenObject* genObject = objects[i];
        genObject->GenLoad(machine, function);
    }
    std::unique_ptr<Instruction> inst = machine.CreateInst("call");
    CallInst* callInst = dynamic_cast<CallInst*>(inst.get());
    std::u32string callName = FullName();
    ConstantPool& constantPool = assembly.GetConstantPool();
    Constant callNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(callName.c_str())));
    callInst->SetFunctionCallName(callNameConstant);
    InstIndexRequest startFunctionCall;
    function.GetEmitter()->AddIndexRequest(&startFunctionCall);
    function.AddInst(std::move(inst));
    function.GetEmitter()->BackpatchConDisSet(startFunctionCall.Index());
    function.MapPCToSourceLine(startFunctionCall.Index(), function.GetEmitter()->CurrentSourceLine());
    if (CanThrow())
    {
        function.SetCanThrow();
    }
}

void FunctionSymbol::CreateMachineFunction()
{
    if (IsIntrinsic())
    {
        return;
    }
    SymbolAccess access = Access();
    if (access == SymbolAccess::public_ || access == SymbolAccess::protected_)
    {
        SetExported();
    }
    bool memberOfClassTemplateSpecialization = false;
    if (Parent()->GetSymbolType() == SymbolType::classTemplateSpecializationSymbol)
    {
        memberOfClassTemplateSpecialization = true;
        std::u32string fullName = FullName();
        Function* prevMachineFunction = FunctionTable::GetFunctionNothrow(StringPtr(fullName.c_str()));
        if (prevMachineFunction)
        {
            machineFunction = prevMachineFunction;
            machineFunction->SetAlreadyGenerated();
            return;
        }
        SetExported();
    }
    machineFunction = GetAssembly()->GetMachineFunctionTable().CreateFunction(this);
    FunctionTable::AddFunction(machineFunction, memberOfClassTemplateSpecialization);
    if (IsExported())
    {
        machineFunction->SetExported();
    }
    if (CanThrow())
    {
        machineFunction->SetCanThrow();
    }
    if (IsInline())
    {
        machineFunction->SetInlineAttribute();
    }
    int32_t numLocals = int32_t(parameters.size() + localVariables.size());
    machineFunction->SetNumLocals(numLocals);
    int32_t numParameters = int32_t(parameters.size());
    machineFunction->SetNumParameters(numParameters);
    for (int32_t i = 0; i < numParameters; ++i)
    {
        ParameterSymbol* parameter = parameters[i];
        ValueType valueType = parameter->GetType()->GetValueType();
        machineFunction->LocalTypes()[i] = valueType;
        machineFunction->ParameterTypes()[i] = valueType;
    }
    int n = int(localVariables.size());
    for (int32_t i = 0; i < n; ++i)
    {
        VariableSymbol* variable = localVariables[i];
        ValueType valueType = variable->GetType()->GetValueType();
        machineFunction->LocalTypes()[numParameters + i] = valueType;
    }
    if (ReturnType() && !ReturnType()->IsVoidType())
    {
        machineFunction->SetReturnsValue();
        machineFunction->SetReturnType(ReturnType()->GetValueType());
    }
    machineFunction->AddInst(GetAssembly()->GetMachine().CreateInst("receive"));
    machineFunction->AddInst(GetAssembly()->GetMachine().CreateInst("gcpoll"));
    if (IsExternal() && !IsDerived())
    {
        Assert(vmf.Value().AsStringLiteral() != nullptr, "vm function name not set");
        std::unique_ptr<Instruction> vmCallInst = GetAssembly()->GetMachine().CreateInst("callvm");
        ConstantPool& constantPool = GetAssembly()->GetConstantPool();
        ConstantId vmfid = constantPool.GetIdFor(vmf);
        Assert(vmfid != noConstantId, "got no constant id");
        vmCallInst->SetIndex(int32_t(vmfid.Value()));
        machineFunction->AddInst(std::move(vmCallInst));
        std::unique_ptr<Instruction> jmpEofInst = GetAssembly()->GetMachine().CreateInst("jump");
        jmpEofInst->SetTarget(endOfFunction);
        machineFunction->AddInst(std::move(jmpEofInst));
        machineFunction->SetCanThrow();
    }
}

void FunctionSymbol::SetVmf(StringPtr vmf_)
{
    ConstantPool& constantPool = GetAssembly()->GetConstantPool();
    vmf = constantPool.GetConstant(constantPool.Install(vmf_));
}

void FunctionSymbol::DumpHeader(CodeFormatter& formatter)
{
    std::string returnTypeStr;
    if (returnType)
    {
        returnTypeStr = ": " + ToUtf8(returnType->FullName());
    }
    formatter.WriteLine(TypeString() + " " + ToUtf8(Name().Value()) + returnTypeStr);
}

void FunctionSymbol::MapContainerScope(uint32_t containerScopeId, ContainerScope* containerScope)
{
    containerScopeMap[containerScopeId] = containerScope;
}

void FunctionSymbol::MapPCToContainerScopeId(uint32_t pc, uint32_t containerScopeId)
{
    pcContainerScopeIdMap[pc] = containerScopeId;
}

ContainerScope* FunctionSymbol::GetMappedContainerScopeForPC(uint32_t pc) const
{
    auto it = pcContainerScopeIdMap.find(pc);
    if (it != pcContainerScopeIdMap.cend())
    {
        uint32_t containerScopeId = it->second;
        auto it  = containerScopeMap.find(containerScopeId);
        if (it != containerScopeMap.cend())
        {
            return it->second;
        }
    }
    return nullptr;
}

StaticConstructorSymbol::StaticConstructorSymbol(const Span& span_, Constant name_) : FunctionSymbol(span_, name_)
{
    SetExported();
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
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception("static constructor cannot be external", GetSpan());
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        throw Exception("static constructor cannot be new", GetSpan());
    }
}

std::u32string StaticConstructorSymbol::FullParsingName() const
{
    std::u32string fullParsingName = U"static ";
    std::u32string parentFullName = Parent()->FullName();
    fullParsingName.append(parentFullName);
    if (!parentFullName.empty())
    {
        fullParsingName.append(1, U'.');
    }
    fullParsingName.append(Parent()->Name().Value());
    fullParsingName.append(U"()");
    return fullParsingName;
}

std::u32string StaticConstructorSymbol::FullNameWithSpecifiers() const
{
    return FullParsingName();
}

void StaticConstructorSymbol::AddTo(ClassTypeSymbol* classTypeSymbol)
{
    classTypeSymbol->Add(this);
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
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        SetExternal();
        SetCanThrow();
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        throw Exception("constructor cannot be new", GetSpan());
    }
}

std::u32string ConstructorSymbol::FullParsingName() const
{
    std::u32string fullParsingName;
    std::u32string parentFullName = Parent()->FullName();
    fullParsingName.append(parentFullName);
    if (!parentFullName.empty())
    {
        fullParsingName.append(1, U'.');
    }
    fullParsingName.append(Parent()->Name().Value());
    fullParsingName.append(1, U'(');
    int n = int(Parameters().size());
    for (int i = 1; i < n; ++i)
    {
        if (i > 0)
        {
            fullParsingName.append(U", ");
        }
        ParameterSymbol* parameter = Parameters()[i];
        fullParsingName.append(parameter->GetType()->FullName());
    }
    fullParsingName.append(1, U')');
    return fullParsingName;
}

std::u32string ConstructorSymbol::FullNameWithSpecifiers() const
{
    std::u32string fullName;
    fullName.append(ToUtf32(SymbolFlagStr(Symbol::Flags())));
    std::string functionSymbolFlagStr = FunctionSymbolFlagStr(GetFlags());
    if (!functionSymbolFlagStr.empty())
    {
        if (!fullName.empty())
        {
        fullName.append(1, U' ');
        }
        fullName.append(ToUtf32(functionSymbolFlagStr));
    }
    std::u32string parentFullName = Parent()->FullName();
    if (!parentFullName.empty())
    {
        if (!fullName.empty())
        {
            fullName.append(1, U' ');
        }
        fullName.append(parentFullName);
        fullName.append(1, U'.');
    }
    fullName.append(Parent()->Name().Value());
    fullName.append(1, U'(');
    int n = int(Parameters().size());
    for (int i = 1; i < n; ++i)
    {
        if (i > 1)
        {
            fullName.append(U", ");
        }
        ParameterSymbol* parameter = Parameters()[i];
        fullName.append(parameter->GetType()->FullName());
        fullName.append(1, U' ');
        fullName.append(std::u32string(parameter->Name().Value()));
    }
    fullName.append(1, U')');
    return fullName;
}

void ConstructorSymbol::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    int n = int(objects.size());
    for (int i = start; i < n; ++i)
    {
        GenObject* genObject = objects[i];
        genObject->GenLoad(machine, function);
    }
    std::unique_ptr<Instruction> callInst = machine.CreateInst("call");
    CallInst* callInstruction = dynamic_cast<CallInst*>(callInst.get());
    Assert(callInst, "call inst expceted");
    std::u32string callName = FullName();
    ConstantPool& constantPool = assembly.GetConstantPool();
    Constant callNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(callName.c_str())));
    callInstruction->SetFunctionCallName(callNameConstant);
    InstIndexRequest startFunctionCall;
    if (function.GetEmitter())
    {
        function.GetEmitter()->AddIndexRequest(&startFunctionCall);
    }
    function.AddInst(std::move(callInst));
    if (function.GetEmitter())
    {
        function.GetEmitter()->BackpatchConDisSet(startFunctionCall.Index());
        function.MapPCToSourceLine(startFunctionCall.Index(), function.GetEmitter()->CurrentSourceLine());
    }
    if (CanThrow())
    {
        function.SetCanThrow();
    }
}

void ConstructorSymbol::CreateMachineFunction()
{
    FunctionSymbol::CreateMachineFunction();
    Symbol* parent = Parent();
    ClassTypeSymbol* classTypeSymbol = dynamic_cast<ClassTypeSymbol*>(parent);
    Assert(classTypeSymbol, "class type symbol expected");
    if (classTypeSymbol->NeedsStaticInitialization())
    {
        std::unique_ptr<Instruction> inst = GetAssembly()->GetMachine().CreateInst("staticinit");
        StaticInitInst* staticInitInst = dynamic_cast<StaticInitInst*>(inst.get());
        ConstantPool& constantPool = GetAssembly()->GetConstantPool();
        std::u32string fullClassName = classTypeSymbol->FullName();
        Constant classFullNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(fullClassName.c_str())));
        staticInitInst->SetTypeName(classFullNameConstant);
        MachineFunction()->AddInst(std::move(inst));
        SetCanThrow();
        MachineFunction()->SetCanThrow();
    }
    std::unique_ptr<Instruction> loadLocal = GetAssembly()->GetMachine().CreateInst("loadlocal.0");
    MachineFunction()->AddInst(std::move(loadLocal));
    std::unique_ptr<Instruction> inst = GetAssembly()->GetMachine().CreateInst("setclassdata");
    MachineFunction()->SetCanThrow();
    SetCanThrow();
    SetClassDataInst* setClassDataInst = dynamic_cast<SetClassDataInst*>(inst.get());
    Assert(setClassDataInst, "set class data inst expected");
    ClassTypeSymbol* containingClass = ContainingClass();;
    Assert(containingClass, "containing class not found");
    std::u32string fullClassName = containingClass->FullName();
    ConstantPool& constantPool = GetAssembly()->GetConstantPool();
    Constant fullClassNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(fullClassName.c_str())));
    setClassDataInst->SetClassName(fullClassNameConstant);
    MachineFunction()->AddInst(std::move(inst));
    if (!BaseConstructorCallGenerated() && containingClass->BaseClass())
    {
        if (!containingClass->BaseClass()->DefaultConstructorSymbol())
        {
            throw Exception("cannot generate default constructor for the class because its base class has no default constructor", containingClass->GetSpan(), containingClass->BaseClass()->GetSpan());
        }
        std::unique_ptr<Instruction> loadLocal = GetAssembly()->GetMachine().CreateInst("loadlocal.0");
        MachineFunction()->AddInst(std::move(loadLocal));
        std::vector<GenObject*> objects;
        containingClass->BaseClass()->DefaultConstructorSymbol()->GenerateCall(GetAssembly()->GetMachine(), *GetAssembly(), *MachineFunction(), objects, 1);
    }
    if (IsExternal())
    {
        Assert(Vmf().Value().AsStringLiteral() != nullptr, "vmf not set");
        std::unique_ptr<Instruction> vmCallInst = GetAssembly()->GetMachine().CreateInst("callvm");
        ConstantPool& constantPool = GetAssembly()->GetConstantPool();
        ConstantId vmfid = constantPool.GetIdFor(Vmf());
        Assert(vmfid != noConstantId, "got no constant id");
        vmCallInst->SetIndex(int32_t(vmfid.Value()));
        MachineFunction()->AddInst(std::move(vmCallInst));
        std::unique_ptr<Instruction> jmpEofInst = GetAssembly()->GetMachine().CreateInst("jump");
        jmpEofInst->SetTarget(endOfFunction);
        MachineFunction()->AddInst(std::move(jmpEofInst));
        SetCanThrow();
        MachineFunction()->SetCanThrow();
    }
}

bool ConstructorSymbol::IsDefaultConstructorSymbol() const
{
    return Parameters().size() == 1;
}

bool ConstructorSymbol::IsIntConstructorSymbol() const
{
    return Parameters().size() == 2 && Parameters()[1]->GetType() == GetAssembly()->GetSymbolTable().GetType(U"System.Int32");
}

void ConstructorSymbol::AddTo(ClassTypeSymbol* classTypeSymbol)
{
    classTypeSymbol->Add(this);
}

void ConstructorSymbol::MergeTo(ClassTemplateSpecializationSymbol* classTemplateSpecializationSymbol, Assembly* assembly)
{
    classTemplateSpecializationSymbol->MergeConstructorSymbol(*this, assembly);
}

void ConstructorSymbol::Merge(ConstructorSymbol& that, Assembly* assembly)
{
    Symbol::Merge(that, assembly);
}

ArraySizeConstructorSymbol::ArraySizeConstructorSymbol(const Span& span_, Constant name_) : ConstructorSymbol(span_, name_)
{
}

void ArraySizeConstructorSymbol::CreateMachineFunction()
{
    SetBaseConstructorCallGenerated();

    ConstructorSymbol::CreateMachineFunction();

    ClassTypeSymbol* containingClass = ContainingClass();;
    Assert(containingClass, "containing class not found");
    Assert(containingClass->BaseClass(), "base class expected");
    bool intConstructorFound = false;
    for (ConstructorSymbol* constructorSymbol : containingClass->BaseClass()->Constructors())
    {
        if (constructorSymbol->IsIntConstructorSymbol())
        {
            std::unique_ptr<Instruction> loadArrayLocal = GetAssembly()->GetMachine().CreateInst("loadlocal.0");
            MachineFunction()->AddInst(std::move(loadArrayLocal));

            std::unique_ptr<Instruction> loadSizeLocal = GetAssembly()->GetMachine().CreateInst("loadlocal.1");
            MachineFunction()->AddInst(std::move(loadSizeLocal));

            std::vector<GenObject*> objects;
            constructorSymbol->GenerateCall(GetAssembly()->GetMachine(), *GetAssembly(), *MachineFunction(), objects, 2);
            intConstructorFound = true;
            break;
        }
    }
    Assert(intConstructorFound, "Array class has no constructor taking an int");

    std::unique_ptr<Instruction> loadArrayLocal = GetAssembly()->GetMachine().CreateInst("loadlocal.0");
    MachineFunction()->AddInst(std::move(loadArrayLocal));

    std::unique_ptr<Instruction> loadSizeLocal = GetAssembly()->GetMachine().CreateInst("loadlocal.1");
    MachineFunction()->AddInst(std::move(loadSizeLocal));

    std::unique_ptr<Instruction> allocElems = GetAssembly()->GetMachine().CreateInst("allocelems");
    AllocateArrayElementsInst* allocElemsInst = dynamic_cast<AllocateArrayElementsInst*>(allocElems.get());

    TypeSymbol* type = Parameters()[0]->GetType();
    ArrayTypeSymbol* arrayType = dynamic_cast<ArrayTypeSymbol*>(type);
    Assert(arrayType, "array type expected");
    TypeSymbol* elementType = arrayType->ElementType();
    std::u32string elementTypeName = elementType->FullName();
    ConstantPool& constantPool = GetAssembly()->GetConstantPool();
    Constant elementTypeNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(elementTypeName.c_str())));
    allocElemsInst->SetTypeName(elementTypeNameConstant);
    MachineFunction()->AddInst(std::move(allocElems));
}

std::string MemberFunctionSymbolFlagStr(MemberFunctionSymbolFlags flags)
{
    std::string s;
    if ((flags & MemberFunctionSymbolFlags::abstract_) != MemberFunctionSymbolFlags::none)
    {
        s.append("abstract");
    }
    if ((flags & MemberFunctionSymbolFlags::override_) != MemberFunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("override");
    }
    if ((flags & MemberFunctionSymbolFlags::virtual_) != MemberFunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("virtual");
    }
    if ((flags & MemberFunctionSymbolFlags::new_) != MemberFunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("new");
    }
    return s;
}

MemberFunctionSymbol::MemberFunctionSymbol(const Span& span_, Constant name_) : FunctionSymbol(span_, name_), vmtIndex(-1), imtIndex(-1), flags(MemberFunctionSymbolFlags::none)
{
}

std::u32string MemberFunctionSymbol::FullNameWithSpecifiers() const
{
    std::u32string fullName;
    std::string flagStr = SymbolFlagStr(Symbol::Flags());
    std::string functionSymbolFlagStr = FunctionSymbolFlagStr(GetFlags());
    if (!functionSymbolFlagStr.empty())
    {
        if (!flagStr.empty())
        {
            flagStr.append(1, ' ');
        }
        flagStr.append(functionSymbolFlagStr);
    }
    std::string myFlagStr = MemberFunctionSymbolFlagStr(flags);
    if (!myFlagStr.empty())
    {
        if (!flagStr.empty())
        {
            flagStr.append(1, ' ');
        }
        flagStr.append(myFlagStr);
    }
    if (!flagStr.empty())
    {
        fullName.append(ToUtf32(flagStr));
    }
    if (ReturnType())
    {
        if (!fullName.empty())
        {
            fullName.append(1, U' ');
        }
        fullName.append(ReturnType()->FullName());
    }
    std::u32string parentFullName = Parent()->FullName();
    if (!parentFullName.empty())
    {
        if (!fullName.empty())
        {
            fullName.append(1, U' ');
        }
        fullName.append(parentFullName);
        fullName.append(1, U'.');
    }
    else
    {
        if (!fullName.empty())
        {
            fullName.append(1, U' ');
        }
    }
    fullName.append(GroupName().Value());
    fullName.append(1, U'(');
    int start = 1;
    if (IsStatic())
    {
        start = 0;
    }
    bool first = true;
    int n = int(Parameters().size());
    for (int i = start; i < n; ++i)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            fullName.append(U", ");
        }
        ParameterSymbol* parameter = Parameters()[i];
        fullName.append(parameter->GetType()->FullName());
        fullName.append(1, U' ');
        fullName.append(std::u32string(parameter->Name().Value()));
    }
    fullName.append(1, U')');
    return fullName;

}
void MemberFunctionSymbol::Write(SymbolWriter& writer)
{
    FunctionSymbol::Write(writer);
    bool hasVmtIndex = vmtIndex != -1;
    writer.AsMachineWriter().Put(hasVmtIndex);
    if (hasVmtIndex)
    {
        writer.AsMachineWriter().PutEncodedUInt(vmtIndex);
    }
    bool hasImtIndex = imtIndex != -1;
    writer.AsMachineWriter().Put(hasImtIndex);
    if (hasImtIndex)
    {
        writer.AsMachineWriter().PutEncodedUInt(imtIndex);
    }
    writer.AsMachineWriter().Put(uint8_t(flags));
}

void MemberFunctionSymbol::Read(SymbolReader& reader)
{
    FunctionSymbol::Read(reader);
    bool hasVmtIndex = reader.GetBool();
    if (hasVmtIndex)
    {
        vmtIndex = reader.GetEncodedUInt();
    }
    bool hasImtIndex = reader.GetBool();
    if (hasImtIndex)
    {
        imtIndex = reader.GetEncodedUInt();
    }
    flags = MemberFunctionSymbolFlags(reader.GetByte());
}

void MemberFunctionSymbol::CreateMachineFunction()
{
    if (IsVirtualAbstractOrOverride())
    {
        SetExported();
    }
    FunctionSymbol::CreateMachineFunction();
}

void MemberFunctionSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        SetStatic();
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        if (IsStatic())
        {
            throw Exception("member function cannot be at the same time virtual and static", GetSpan());
        }
        SetVirtual();
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        if (IsStatic())
        {
            throw Exception("member function cannot be at the same time virtual and override", GetSpan());
        }
        if (IsVirtual())
        {
            throw Exception("member function cannot be at the same time virtual and override", GetSpan());
        }
        SetOverride();
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        if (IsStatic())
        {
            throw Exception("member function cannot be at the same time virtual and abstract", GetSpan());
        }
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
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        SetExternal();
        SetCanThrow();
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        SetNew();
    }
}

std::u32string MemberFunctionSymbol::FullParsingName() const
{
    std::u32string fullParsingName;
    std::u32string parentFullName = Parent()->FullName();
    fullParsingName.append(parentFullName);
    if (!parentFullName.empty())
    {
        fullParsingName.append(1, U'.');
    }
    fullParsingName.append(GroupName().Value());
    fullParsingName.append(1, U'(');
    int n = int(Parameters().size());
    for (int i = 1; i < n; ++i)
    {
        if (i > 0)
        {
            fullParsingName.append(U", ");
        }
        ParameterSymbol* parameter = Parameters()[i];
        fullParsingName.append(parameter->GetType()->FullName());
    }
    fullParsingName.append(1, U')');
    return fullParsingName;
}

void MemberFunctionSymbol::GenerateVirtualCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects)
{
    int n = int(objects.size());
    for (int i = 0; i < n; ++i)
    {
        GenObject* genObject = objects[i];
        genObject->GenLoad(machine, function);
    }
    std::unique_ptr<Instruction> inst = machine.CreateInst("callv");
    VirtualCallInst* vcall = dynamic_cast<VirtualCallInst*>(inst.get());
    Assert(vcall, "virtual call instruction expected");
    std::u32string callName = FullName();
    ConstantPool& constantPool = assembly.GetConstantPool();
    Constant callNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(callName.c_str())));
    vcall->SetNumArgs(n);
    vcall->SetVmtIndex(vmtIndex);
    FunctionType functionType;
    if (ReturnType())
    {
        functionType.SetReturnType(ReturnType()->GetValueType());
    }
    for (ParameterSymbol* param : Parameters())
    {
        functionType.AddParamType(param->GetType()->GetValueType());
    }
    vcall->SetFunctionType(functionType);
    InstIndexRequest startFunctionCall;
    function.GetEmitter()->AddIndexRequest(&startFunctionCall);
    function.AddInst(std::move(inst));
    function.GetEmitter()->BackpatchConDisSet(startFunctionCall.Index());
    function.MapPCToSourceLine(startFunctionCall.Index(), function.GetEmitter()->CurrentSourceLine());
    SetCanThrow();
    function.SetCanThrow();
}

void MemberFunctionSymbol::GenerateInterfaceCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects)
{
    int n = int(objects.size());
    for (int i = 0; i < n; ++i)
    {
        GenObject* genObject = objects[i];
        genObject->GenLoad(machine, function);
    }
    std::unique_ptr<Instruction> inst = machine.CreateInst("calli");
    InterfaceCallInst* icall = dynamic_cast<InterfaceCallInst*>(inst.get());
    Assert(icall, "interface call instruction expected");
    icall->SetNumArgs(n);
    icall->SetImtIndex(imtIndex);
    FunctionType functionType;
    if (ReturnType())
    {
        functionType.SetReturnType(ReturnType()->GetValueType());
    }
    for (ParameterSymbol* param : Parameters())
    {
        functionType.AddParamType(param->GetType()->GetValueType());
    }
    icall->SetFunctionType(functionType);
    InstIndexRequest startFunctionCall;
    function.GetEmitter()->AddIndexRequest(&startFunctionCall);
    function.AddInst(std::move(inst));
    function.GetEmitter()->BackpatchConDisSet(startFunctionCall.Index());
    function.MapPCToSourceLine(startFunctionCall.Index(), function.GetEmitter()->CurrentSourceLine());
    SetCanThrow();
    function.SetCanThrow();
}

void MemberFunctionSymbol::AddTo(ClassTypeSymbol* classTypeSymbol)
{
    classTypeSymbol->Add(this);
}

void MemberFunctionSymbol::MergeTo(ClassTemplateSpecializationSymbol* classTemplateSpecializationSymbol, Assembly* assembly)
{
    classTemplateSpecializationSymbol->MergeMemberFunctionSymbol(*this, assembly);
}

void MemberFunctionSymbol::Merge(MemberFunctionSymbol& that, Assembly* assembly)
{
    Symbol::Merge(that, assembly);
}

bool MemberFunctionSymbol::ImplementsInterfaceMemFun(InterfaceTypeSymbol* intf)
{
    for (MemberFunctionSymbol* intfMemFun : intf->MemberFunctions())
    {
        if (Implements(this, intfMemFun)) return true;
    }
    return false;
}

ArrayGetEnumeratorMemberFunctionSymbol::ArrayGetEnumeratorMemberFunctionSymbol(const Span& span_, Constant name_) : MemberFunctionSymbol(span_, name_)
{
}

ClassTypeConversion::ClassTypeConversion(const Span& span_, Constant name_) : FunctionSymbol(span_, name_)
{
    SetConversionFun();
}

void ClassTypeConversion::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    std::unique_ptr<Instruction> inst;
    switch (conversionType)
    {
        case ConversionType::implicit_: break;
        case ConversionType::explicit_: inst = std::move(machine.CreateInst("downcast")); function.SetCanThrow(); break;
        default: throw std::runtime_error("invalid conversion type");
    }
    if (inst)
    {
        TypeInstruction* typeInstruction = dynamic_cast<TypeInstruction*>(inst.get());
        Assert(typeInstruction, "type instruction expected");
        std::u32string targetTypeClassName = targetType->FullName();
        ConstantPool& constantPool = assembly.GetConstantPool();
        Constant targetTypeClassNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(targetTypeClassName.c_str())));
        typeInstruction->SetTypeName(targetTypeClassNameConstant);
        function.AddInst(std::move(inst));
    }
}

ClassToInterfaceConversion::ClassToInterfaceConversion(const Span& span_, Constant name_) : FunctionSymbol(span_, name_), itabIndex(-1)
{
    SetConversionFun();
}

void ClassToInterfaceConversion::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    std::unique_ptr<Instruction> createInst;
    createInst = machine.CreateInst("createo");
    ConstantPool& constantPool = assembly.GetConstantPool();
    std::u32string targetTypeInterfaceName = targetType->FullName();
    Constant targetTypeInterfaceNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(targetTypeInterfaceName.c_str())));
    CreateObjectInst* createObjectInst = dynamic_cast<CreateObjectInst*>(createInst.get());
    Assert(createObjectInst, "CreateObject instruction expected");
    createObjectInst->SetTypeName(targetTypeInterfaceNameConstant);
    function.AddInst(std::move(createInst));
    std::unique_ptr<Instruction> dupInst;
    dupInst = machine.CreateInst("dup");
    function.AddInst(std::move(dupInst));
    Constant itabIndexConstant(MakeIntegralValue<int32_t>(itabIndex, ValueType::intType));
    ConstantId itabIndexId = constantPool.Install(itabIndexConstant);
    std::unique_ptr<Instruction> loadConstant;
    if (itabIndexId.Value() < 256)
    {
        loadConstant = machine.CreateInst("loadconstant.b");
        loadConstant->SetIndex(static_cast<uint8_t>(itabIndexId.Value()));
    }
    else if (itabIndexId.Value() < 65536)
    {
        loadConstant = machine.CreateInst("loadconstant.s");
        loadConstant->SetIndex(static_cast<uint16_t>(itabIndexId.Value()));
    }
    else
    {
        loadConstant = machine.CreateInst("loadconstant");
        loadConstant->SetIndex(int32_t(itabIndexId.Value()));
    }
    function.AddInst(std::move(loadConstant));
    std::unique_ptr<Instruction> swap;
    swap = machine.CreateInst("swap");
    function.AddInst(std::move(swap));
    std::unique_ptr<Instruction> storeField;
    storeField = machine.CreateInst("storefield.1");
    StoreField1Inst* storeField1Inst = static_cast<StoreField1Inst*>(storeField.get());
    storeField1Inst->SetFieldType(ValueType::intType);
    function.AddInst(std::move(storeField));
    std::unique_ptr<Instruction> dupInst2;
    dupInst2 = machine.CreateInst("dup");
    function.AddInst(std::move(dupInst2));
    std::unique_ptr<Instruction> rotate;
    rotate = machine.CreateInst("rotate");
    function.AddInst(std::move(rotate));
    std::unique_ptr<Instruction> swap2;
    swap2 = machine.CreateInst("swap");
    function.AddInst(std::move(swap2));
    std::unique_ptr<Instruction> storeField2;
    storeField2 = machine.CreateInst("storefield.0");
    StoreField0Inst* storeField0Inst = static_cast<StoreField0Inst*>(storeField2.get());
    storeField0Inst->SetFieldType(ValueType::objectReference);
    function.AddInst(std::move(storeField2));
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

FunctionGroupTypeSymbol::FunctionGroupTypeSymbol(FunctionGroupSymbol* functionGroup_) : TypeSymbol(functionGroup_->GetSpan(), functionGroup_->NameConstant()), functionGroup(functionGroup_)
{
}

MemberExpressionTypeSymbol::MemberExpressionTypeSymbol(Constant name_, void* boundMemberExpression_) : TypeSymbol(Span(), name_), boundMemberExpression(boundMemberExpression_)
{
}

FunctionToDelegateConversion::FunctionToDelegateConversion(const Span& span_, Constant name_) : FunctionSymbol(span_, name_)
{
    SetConversionFun();
}

void FunctionToDelegateConversion::SetFunctionName(Constant functionName_)
{
    functionName = functionName_;
}

void FunctionToDelegateConversion::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    std::unique_ptr<Instruction> inst = machine.CreateInst("fun2dlg");
    Fun2DlgInst* fun2DlgInst = dynamic_cast<Fun2DlgInst*>(inst.get());
    fun2DlgInst->SetFunctionName(functionName);
    function.AddInst(std::move(inst));
}

MemFunToClassDelegateConversion::MemFunToClassDelegateConversion(const Span& span_, Constant name_) : FunctionSymbol(span_, name_)
{
    SetConversionFun();
}

void MemFunToClassDelegateConversion::SetFunctionName(Constant functionName_)
{
    functionName = functionName_;
}

void MemFunToClassDelegateConversion::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    std::unique_ptr<Instruction> inst = machine.CreateInst("memfun2classdlg");
    MemFun2ClassDlgInst* memfun2ClassDlgInst = dynamic_cast<MemFun2ClassDlgInst*>(inst.get());
    memfun2ClassDlgInst->SetFunctionName(functionName);
    function.AddInst(std::move(inst));
}

RequestGcFunctionSymbol::RequestGcFunctionSymbol(const Span& span_, Constant name_) : FunctionSymbol(span_, name_)
{
}

void RequestGcFunctionSymbol::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    std::unique_ptr<Instruction> inst = machine.CreateInst("requestgc");
    function.AddInst(std::move(inst));
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
    TypeSymbol* stringType = assembly.GetSymbolTable().GetType(U"System.String");
    if (sourceType == stringType && targetType == stringType)
    {
        ClassTypeSymbol* stringClassType = dynamic_cast<ClassTypeSymbol*>(stringType);
        BasicTypeConversion* strlitToString = CreateStringLiteralToStringConversion(assembly, stringClassType);
        strlitToString->SetCreatesObject();
        conversionMap[std::make_pair(sourceType, targetType)] = strlitToString;
        stringConversions.push_back(std::unique_ptr<FunctionSymbol>(strlitToString));
        return strlitToString;
    }
    else if (NullReferenceTypeSymbol* nullReferenceTypeSymbol = dynamic_cast<NullReferenceTypeSymbol*>(sourceType))
    {
        if (ClassTypeSymbol* targetClassType = dynamic_cast<ClassTypeSymbol*>(targetType))
        {
            ConstantPool& constantPool = assembly.GetConstantPool();
            Constant groupName = constantPool.GetConstant(constantPool.Install(StringPtr(U"@conversion")));
            std::u32string conversionName = sourceType->FullName() + U"2" + targetType->FullName();
            Constant conversionNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(conversionName.c_str())));
            std::unique_ptr<NullToObjectConversion> conversion(new NullToObjectConversion(targetClassType->GetSpan(), conversionNameConstant));
            FunctionSymbol* conversionFun = conversion.get();
            conversion->SetAssembly(&assembly);
            conversion->SetSourceType(sourceType);
            conversion->SetTargetType(targetType);
            conversionMap[std::make_pair(sourceType, targetType)] = conversionFun;
            nullConversions.push_back(std::move(conversion));
            return conversionFun;
        }
    }
    else if (ClassTypeSymbol* sourceClassType = dynamic_cast<ClassTypeSymbol*>(sourceType))
    {
        if (ClassTypeSymbol* targetClassType = dynamic_cast<ClassTypeSymbol*>(targetType))
        {
            ConstantPool& constantPool = assembly.GetConstantPool();
            int distance = 0;
            if (sourceClassType->HasBaseClass(targetClassType, distance))
            {
                Constant groupName = constantPool.GetConstant(constantPool.Install(StringPtr(U"@conversion")));
                std::u32string conversionName = sourceType->FullName() + U"2" + targetType->FullName();
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
                    std::u32string conversionName = sourceType->FullName() + U"2" + targetType->FullName();
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
        else if (InterfaceTypeSymbol* interfaceTypeSymbol = dynamic_cast<InterfaceTypeSymbol*>(targetType))
        {
            int32_t n = int32_t(sourceClassType->ImplementedInterfaces().size());
            for (int32_t i = 0; i < n; ++i)
            {
                InterfaceTypeSymbol* implementedInterface = sourceClassType->ImplementedInterfaces()[i];
                if (implementedInterface == interfaceTypeSymbol)
                {
                    int32_t itabIndex = i;
                    ConstantPool& constantPool = assembly.GetConstantPool();
                    Constant groupName = constantPool.GetConstant(constantPool.Install(StringPtr(U"@conversion")));
                    std::u32string conversionName = sourceType->FullName() + U"2" + targetType->FullName();
                    Constant conversionNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(conversionName.c_str())));
                    std::unique_ptr<ClassToInterfaceConversion> conversion(new ClassToInterfaceConversion(sourceClassType->GetSpan(), conversionNameConstant));
                    FunctionSymbol* conversionFun = conversion.get();
                    conversion->SetITabIndex(itabIndex);
                    conversion->SetAssembly(&assembly);
                    conversion->SetSourceType(sourceType);
                    conversion->SetTargetType(targetType);
                    conversionMap[std::make_pair(sourceType, targetType)] = conversionFun;
                    interfaceTypeConversions.push_back(std::move(conversion));
                    return conversionFun;
                }
            }
        }
    }
    else if (EnumTypeSymbol* sourceEnumType = dynamic_cast<EnumTypeSymbol*>(sourceType))
    {
        ConstantPool& constantPool = assembly.GetConstantPool();
        Constant groupName = constantPool.GetConstant(constantPool.Install(StringPtr(U"@conversion")));
        TypeSymbol* underlyingType = sourceEnumType->GetUnderlyingType();
        if (targetType == underlyingType)
        {
            std::unique_ptr<EnumTypeConversionFun> enum2underlyingType(new EnumTypeConversionFun(Span(), constantPool.GetConstant(constantPool.Install(U"enum2underlyingType"))));
            enum2underlyingType->SetAssembly(&assembly);
            enum2underlyingType->SetGroupNameConstant(groupName);
            enum2underlyingType->SetConversionType(ConversionType::implicit_);
            enum2underlyingType->SetConversionDistance(1);
            enum2underlyingType->SetSourceType(sourceEnumType);
            enum2underlyingType->SetTargetType(underlyingType);
            FunctionSymbol* conversionFun = enum2underlyingType.get();
            conversionMap[std::make_pair(sourceType, targetType)] = conversionFun;
            enumTypeConversions.push_back(std::move(enum2underlyingType));
            return conversionFun;
        }
    }
    else if (EnumTypeSymbol* targetEnumType = dynamic_cast<EnumTypeSymbol*>(targetType))
    {
        ConstantPool& constantPool = assembly.GetConstantPool();
        Constant groupName = constantPool.GetConstant(constantPool.Install(StringPtr(U"@conversion")));
        TypeSymbol* underlyingType = targetEnumType->GetUnderlyingType();
        if (sourceType == underlyingType)
        {
            std::unique_ptr<EnumTypeConversionFun> underlyingType2enum(new EnumTypeConversionFun(Span(), constantPool.GetConstant(constantPool.Install(U"underlyingType2enum"))));
            underlyingType2enum->SetAssembly(&assembly);
            underlyingType2enum->SetGroupNameConstant(groupName);
            underlyingType2enum->SetConversionType(ConversionType::explicit_);
            underlyingType2enum->SetConversionDistance(999);
            underlyingType2enum->SetSourceType(underlyingType);
            underlyingType2enum->SetTargetType(targetEnumType);
            FunctionSymbol* conversionFun = underlyingType2enum.get();
            conversionMap[std::make_pair(sourceType, targetType)] = conversionFun;
            enumTypeConversions.push_back(std::move(underlyingType2enum));
            return conversionFun;
        }
    }
    else if (targetType->IsDelegateType() && sourceType->IsFunctionGroupTypeSymbol())
    {
        DelegateTypeSymbol* targetDelegateType = static_cast<DelegateTypeSymbol*>(targetType);
        int arity = targetDelegateType->Arity();
        FunctionGroupTypeSymbol* sourceFunctionGroupType = static_cast<FunctionGroupTypeSymbol*>(sourceType);
        std::unordered_set<FunctionSymbol*> viableFunctions;
        sourceFunctionGroupType->FunctionGroup()->CollectViableFunctions(arity, viableFunctions);
        for (FunctionSymbol* viableFunction : viableFunctions)
        {
            bool functionFound = true;
            for (int i = 0; i < arity; ++i)
            {
                ParameterSymbol* sourceParam = viableFunction->Parameters()[i];
                ParameterSymbol* targetParam = targetDelegateType->Parameters()[i];
                if (sourceParam->GetType() != targetParam->GetType())
                {
                    functionFound = false;
                    break;
                }
            }
            if (viableFunction->ReturnType() != targetDelegateType->GetReturnType())
            {
                functionFound = false;
            }
            if (functionFound)
            {
                ConstantPool& constantPool = assembly.GetConstantPool();
                std::u32string fullFunctionName = viableFunction->FullName();
                viableFunction->SetExported();
                Constant functionNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(fullFunctionName.c_str())));
                Constant groupName = constantPool.GetConstant(constantPool.Install(StringPtr(U"@conversion")));
                std::u32string conversionName = sourceType->FullName() + U"2" + targetType->FullName();
                Constant conversionNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(conversionName.c_str())));
                std::unique_ptr<FunctionToDelegateConversion> fun2DlgConversion(new FunctionToDelegateConversion(Span(), conversionNameConstant));
                fun2DlgConversion->SetAssembly(&assembly);
                fun2DlgConversion->SetGroupNameConstant(groupName);
                fun2DlgConversion->SetSourceType(sourceType);
                fun2DlgConversion->SetTargetType(targetType);
                fun2DlgConversion->SetFunctionName(functionNameConstant);
                FunctionSymbol* conversionFun = fun2DlgConversion.get();
                conversionMap[std::make_pair(sourceType, targetType)] = conversionFun;
                delegateConversions.push_back(std::move(fun2DlgConversion));
                return conversionFun;
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
