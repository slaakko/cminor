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

namespace cminor { namespace symbols {

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
    if (vmFunctionName.Value().AsStringLiteral() != nullptr)
    {
        writer.AsMachineWriter().Put(true);
        ConstantId vmFunctionNameId = GetAssembly()->GetConstantPool().GetIdFor(vmFunctionName);
        Assert(vmFunctionNameId != noConstantId, "no id for vm function id");
        vmFunctionNameId.Write(writer.AsMachineWriter());
    }
    else
    {
        writer.AsMachineWriter().Put(false);
    }
    utf32_string returnTypeFullName;
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
        writer.AsMachineWriter().PutEncodedUInt(machineFunction->Id());
    }
}

void FunctionSymbol::Read(SymbolReader& reader)
{
    reader.ResetLocalVariables();
    ContainerSymbol::Read(reader);
    ConstantId groupNameId;
    groupNameId.Read(reader);
    groupName = reader.GetAssembly()->GetConstantPool().GetConstant(groupNameId);
    bool hasVmFunctionId = reader.GetBool();
    if (hasVmFunctionId)
    {
        ConstantId vmFunctionNameId;
        vmFunctionNameId.Read(reader);
        vmFunctionName = reader.GetAssembly()->GetConstantPool().GetConstant(vmFunctionNameId);
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
        uint32_t machineFunctionId = reader.GetEncodedUInt();
        machineFunction = GetAssembly()->GetMachineFunctionTable().GetFunction(machineFunctionId);
        if (StringPtr(groupName.Value().AsStringLiteral()) == StringPtr(U"main"))
        {
            machineFunction->SetMain();
        }
        FunctionTable::Instance().AddFunction(machineFunction, reader.ReadingClassTemplateSpecialization());
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
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        SetExternal();
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

utf32_string FunctionSymbol::FullParsingName() const
{
    utf32_string fullParsingName;
    utf32_string parentFullName = Parent()->FullName();
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
    utf32_string callName = FullName();
    ConstantPool& constantPool = assembly.GetConstantPool();
    Constant callNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(callName.c_str())));
    callInst->SetFunctionCallName(callNameConstant);
    InstIndexRequest startFunctionCall;
    function.GetEmitter()->AddIndexRequest(&startFunctionCall);
    function.AddInst(std::move(inst));
    function.GetEmitter()->BackpatchConDisSet(startFunctionCall.Index());
    function.MapPCToSourceLine(startFunctionCall.Index(), function.GetEmitter()->CurrentSourceLine());
}

void FunctionSymbol::CreateMachineFunction()
{
    machineFunction = GetAssembly()->GetMachineFunctionTable().CreateFunction(this);
    int32_t numLocals = int32_t(parameters.size() + localVariables.size());
    machineFunction->SetNumLocals(numLocals);
    int32_t numParameters = int32_t(parameters.size());
    machineFunction->SetNumParameters(numParameters);
    machineFunction->AddInst(GetAssembly()->GetMachine().CreateInst("receive"));
    if (IsExternal() && !IsDerived())
    {
        Assert(vmFunctionName.Value().AsStringLiteral() != nullptr, "vm function name not set");
        std::unique_ptr<Instruction> vmCallInst = GetAssembly()->GetMachine().CreateInst("callvm");
        ConstantPool& constantPool = GetAssembly()->GetConstantPool();
        ConstantId vmFunctionNameId = constantPool.GetIdFor(vmFunctionName);
        Assert(vmFunctionNameId != noConstantId, "got no constant id");
        vmCallInst->SetIndex(int32_t(vmFunctionNameId.Value()));
        machineFunction->AddInst(std::move(vmCallInst));
    }
}

void FunctionSymbol::SetVmFunctionName(StringPtr vmFunctionName_)
{
    ConstantPool& constantPool = GetAssembly()->GetConstantPool();
    vmFunctionName = constantPool.GetConstant(constantPool.Install(vmFunctionName_));
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
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception("static constructor cannot be external", GetSpan());
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        throw Exception("static constructor cannot be new", GetSpan());
    }
}

utf32_string StaticConstructorSymbol::FullParsingName() const
{
    utf32_string fullParsingName = U"static ";
    utf32_string parentFullName = Parent()->FullName();
    fullParsingName.append(parentFullName);
    if (!parentFullName.empty())
    {
        fullParsingName.append(1, U'.');
    }
    fullParsingName.append(Parent()->Name().Value());
    fullParsingName.append(U"()");
    return fullParsingName;
}

utf32_string StaticConstructorSymbol::FriendlyName() const
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
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        throw Exception("constructor cannot be new", GetSpan());
    }
}

utf32_string ConstructorSymbol::FullParsingName() const
{
    utf32_string fullParsingName;
    utf32_string parentFullName = Parent()->FullName();
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

utf32_string ConstructorSymbol::FriendlyName() const
{
    utf32_string friendlyName;
    friendlyName.append(ToUtf32(SymbolFlagStr(Symbol::Flags())));
    std::string functionSymbolFlagStr = FunctionSymbolFlagStr(GetFlags());
    if (!functionSymbolFlagStr.empty())
    {
        if (!friendlyName.empty())
        {
            friendlyName.append(1, U' ');
        }
        friendlyName.append(ToUtf32(functionSymbolFlagStr));
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
    friendlyName.append(Parent()->Name().Value());
    friendlyName.append(1, U'(');
    int n = int(Parameters().size());
    for (int i = 1; i < n; ++i)
    {
        if (i > 1)
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
    utf32_string callName = FullName();
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
        utf32_string fullClassName = classTypeSymbol->FullName();
        Constant classFullNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(fullClassName.c_str())));
        staticInitInst->SetTypeName(classFullNameConstant);
        MachineFunction()->AddInst(std::move(inst));
    }
    std::unique_ptr<Instruction> loadLocal = GetAssembly()->GetMachine().CreateInst("loadlocal.0");
    MachineFunction()->AddInst(std::move(loadLocal));
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
        std::unique_ptr<Instruction> loadLocal = GetAssembly()->GetMachine().CreateInst("loadlocal.0");
        MachineFunction()->AddInst(std::move(loadLocal));
        std::unique_ptr<Instruction> upCast = GetAssembly()->GetMachine().CreateInst("upcast");
        UpCastInst* upCastInst = dynamic_cast<UpCastInst*>(upCast.get());
        Assert(upCastInst, "up cast inst expected");
        utf32_string baseClassFullName = containingClass->BaseClass()->FullName();
        Constant baseClassNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(baseClassFullName.c_str())));
        upCastInst->SetTypeName(baseClassNameConstant);
        MachineFunction()->AddInst(std::move(upCast));
        std::vector<GenObject*> objects;
        containingClass->BaseClass()->DefaultConstructorSymbol()->GenerateCall(GetAssembly()->GetMachine(), *GetAssembly(), *MachineFunction(), objects, 1);
    }
    if (IsExternal())
    {
        Assert(VmFunctionName().Value().AsStringLiteral() != nullptr, "vm function name not set");
        std::unique_ptr<Instruction> vmCallInst = GetAssembly()->GetMachine().CreateInst("callvm");
        ConstantPool& constantPool = GetAssembly()->GetConstantPool();
        ConstantId vmFunctionNameId = constantPool.GetIdFor(VmFunctionName());
        Assert(vmFunctionNameId != noConstantId, "got no constant id");
        vmCallInst->SetIndex(int32_t(vmFunctionNameId.Value()));
        MachineFunction()->AddInst(std::move(vmCallInst));
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

void ConstructorSymbol::MergeTo(ClassTemplateSpecializationSymbol* classTemplateSpecializationSymbol)
{
    classTemplateSpecializationSymbol->MergeConstructorSymbol(*this);
}

void ConstructorSymbol::Merge(const ConstructorSymbol& that)
{
    Symbol::Merge(that);
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

            std::unique_ptr<Instruction> upCast = GetAssembly()->GetMachine().CreateInst("upcast");
            UpCastInst* upCastInst = dynamic_cast<UpCastInst*>(upCast.get());
            Assert(upCastInst, "up cast inst expected");
            utf32_string baseClassFullName = containingClass->BaseClass()->FullName();
            ConstantPool& constantPool = GetAssembly()->GetConstantPool();
            Constant baseClassNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(baseClassFullName.c_str())));
            upCastInst->SetTypeName(baseClassNameConstant);
            MachineFunction()->AddInst(std::move(upCast));

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
    utf32_string elementTypeName = elementType->FullName();
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

utf32_string MemberFunctionSymbol::FriendlyName() const
{
    utf32_string friendlyName;
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
        friendlyName.append(ToUtf32(flagStr));
    }
    if (ReturnType())
    {
        if (!friendlyName.empty())
        {
            friendlyName.append(1, U' ');
        }
        friendlyName.append(ReturnType()->FullName());
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
    for (int i = 1; i < n; ++i)
    {
        if (i > 1)
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
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        SetNew();
    }
}

utf32_string MemberFunctionSymbol::FullParsingName() const
{
    utf32_string fullParsingName;
    utf32_string parentFullName = Parent()->FullName();
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
    vcall->SetNumArgs(n);
    vcall->SetVmtIndex(vmtIndex);
    InstIndexRequest startFunctionCall;
    function.GetEmitter()->AddIndexRequest(&startFunctionCall);
    function.AddInst(std::move(inst));
    function.GetEmitter()->BackpatchConDisSet(startFunctionCall.Index());
    function.MapPCToSourceLine(startFunctionCall.Index(), function.GetEmitter()->CurrentSourceLine());
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
    InstIndexRequest startFunctionCall;
    function.GetEmitter()->AddIndexRequest(&startFunctionCall);
    function.AddInst(std::move(inst));
    function.GetEmitter()->BackpatchConDisSet(startFunctionCall.Index());
    function.MapPCToSourceLine(startFunctionCall.Index(), function.GetEmitter()->CurrentSourceLine());
}

void MemberFunctionSymbol::AddTo(ClassTypeSymbol* classTypeSymbol)
{
    classTypeSymbol->Add(this);
}

void MemberFunctionSymbol::MergeTo(ClassTemplateSpecializationSymbol* classTemplateSpecializationSymbol)
{
    classTemplateSpecializationSymbol->MergeMemberFunctionSymbol(*this);
}

void MemberFunctionSymbol::Merge(const MemberFunctionSymbol& that)
{
    Symbol::Merge(that);
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
        case ConversionType::implicit_: inst = std::move(machine.CreateInst("upcast")); break;
        case ConversionType::explicit_: inst = std::move(machine.CreateInst("downcast")); break;
        default: throw std::runtime_error("invalid conversion type");
    }
    TypeInstruction* typeInstruction = dynamic_cast<TypeInstruction*>(inst.get());
    Assert(typeInstruction, "type instruction expected");
    utf32_string targetTypeClassName = targetType->FullName();
    ConstantPool& constantPool = assembly.GetConstantPool();
    Constant targetTypeClassNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(targetTypeClassName.c_str())));
    typeInstruction->SetTypeName(targetTypeClassNameConstant);
    function.AddInst(std::move(inst));
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
    utf32_string targetTypeInterfaceName = targetType->FullName();
    Constant targetTypeInterfaceNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(targetTypeInterfaceName.c_str())));
    CreateObjectInst* createObjectInst = dynamic_cast<CreateObjectInst*>(createInst.get());
    Assert(createObjectInst, "CreateObject instruction expected");
    createObjectInst->SetTypeName(targetTypeInterfaceNameConstant);
    function.AddInst(std::move(createInst));
    std::unique_ptr<Instruction> dupInst;
    dupInst = machine.CreateInst("dup");
    function.AddInst(std::move(dupInst));
    Constant itabIndexConstant(IntegralValue(itabIndex, ValueType::intType));
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
            utf32_string conversionName = sourceType->FullName() + U"2" + targetType->FullName();
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
                    utf32_string conversionName = sourceType->FullName() + U"2" + targetType->FullName();
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
                utf32_string fullFunctionName = viableFunction->FullName();
                Constant functionNameContant = constantPool.GetConstant(constantPool.Install(StringPtr(fullFunctionName.c_str())));
                Constant groupName = constantPool.GetConstant(constantPool.Install(StringPtr(U"@conversion")));
                utf32_string conversionName = sourceType->FullName() + U"2" + targetType->FullName();
                Constant conversionNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(conversionName.c_str())));
                std::unique_ptr<FunctionToDelegateConversion> fun2DlgConversion(new FunctionToDelegateConversion(Span(), conversionNameConstant));
                fun2DlgConversion->SetAssembly(&assembly);
                fun2DlgConversion->SetGroupNameConstant(groupName);
                fun2DlgConversion->SetSourceType(sourceType);
                fun2DlgConversion->SetTargetType(targetType);
                fun2DlgConversion->SetFunctionName(functionNameContant);
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
