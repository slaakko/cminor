// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Instruction.hpp>
#include <cminor/machine/Error.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Util.hpp>
#include <cminor/machine/String.hpp>
#include <cminor/machine/Constant.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/machine/Class.hpp>
#include <cminor/machine/Type.hpp>

namespace cminor { namespace machine {

Instruction::Instruction(const std::string& name_) : opCode(255), name(name_), groupName(""), typeName(""), parent(nullptr)
{
}

Instruction::Instruction(const std::string& name_, const std::string& groupName_, const std::string& typeName_) : opCode(255), name(name_), groupName(groupName_), typeName(typeName_), parent(nullptr)
{
}

Instruction::~Instruction()
{
}

void Instruction::Encode(Writer& writer)
{
    if (parent)
    {
        parent->Encode(writer);
    }
    writer.Put(opCode);
}

Instruction* Instruction::Decode(Reader& reader)
{
    return this;
}

void Instruction::SetIndex(int32_t index_)
{
    throw std::runtime_error("instruction '" + name + "' does not support set index");
}

void Instruction::Execute(Frame& frame)
{
}

void Instruction::GetOpCodes(std::string& opCodes)
{
    if (parent && !parent->IsRoot())
    {
        parent->GetOpCodes(opCodes);
    }
    if (!opCodes.empty())
    {
        opCodes.append(1, ' ');
    }
    opCodes.append(ToHexString(opCode));
}

void Instruction::SetTarget(int32_t target)
{
    throw std::runtime_error("cannot set target of '" + name + "' instruction");
}

void Instruction::Dump(CodeFormatter& formatter)
{
    std::string opCodes;
    GetOpCodes(opCodes);
    while (opCodes.size() < 5)
    {
        opCodes.append(1, ' ');
    }
    formatter.Write(opCodes + " " + name);
}

InvalidInst::InvalidInst() : Instruction("<invalid_instruction>", "", "")
{
}

void InvalidInst::Encode(Writer& writer)
{
    throw std::runtime_error("tried to encode invalid instruction " + std::to_string(OpCode()));
}

Instruction* InvalidInst::Decode(Reader& reader)
{
    throw std::runtime_error("tried to decode invalid instruction " + std::to_string(OpCode()));
}

void InvalidInst::Execute(Frame& frame)
{
    throw std::runtime_error("invalid instruction " + std::to_string(OpCode()));
}

NopInst::NopInst() : Instruction("nop", "", "")
{
}

ContainerInst::ContainerInst(Machine& machine_, const std::string& name_, bool root_) : Instruction(name_, "", ""), machine(machine_), root(root_)
{
    childInsts.resize(256);
    for (int i = 0; i < 256; ++i)
    {
        SetInst(i, new InvalidInst());
    }
}

void ContainerInst::SetInst(uint8_t opCode, Instruction* inst)
{
    inst->SetOpCode(opCode);
    childInsts[opCode].reset(inst);
    if (dynamic_cast<InvalidInst*>(inst) == nullptr)
    {
        inst->SetParent(this);
        machine.AddInst(inst);
    }
}

void ContainerInst::Encode(Writer& writer)
{
    if (!root)
    {
        Instruction::Encode(writer);
    }
}

Instruction* ContainerInst::Decode(Reader& reader)
{
    uint8_t opCode = reader.GetByte();
    Instruction* inst = childInsts[opCode].get();
    return inst->Decode(reader);
}

InstructionTypeGroup::InstructionTypeGroup() : instGroupName("")
{
}

InstructionTypeGroup::InstructionTypeGroup(const std::string& instGroupName_) : instGroupName(instGroupName_)
{
}

void InstructionTypeGroup::AddInst(Instruction* inst)
{
    instructionMap[inst->TypeName()] = inst;
}

std::unique_ptr<Instruction> InstructionTypeGroup::CreateInst(const std::string& typeName) const
{
    auto it = instructionMap.find(typeName);
    if (it != instructionMap.cend())
    {
        Instruction* inst = it->second;
        return std::unique_ptr<Instruction>(inst->Clone());
    }
    throw std::runtime_error("instruction for type '" + typeName + "' not found in instruction group '" + instGroupName + "'");
}

LogicalNotInst::LogicalNotInst() : Instruction("not", "not", "System.Boolean")
{
}

void LogicalNotInst::Execute(Frame& frame)
{
    IntegralValue value = frame.OpStack().Pop();
    Assert(value.GetType() == ValueType::boolType, "bool operand expected");
    bool operand = value.AsBool();
    frame.OpStack().Push(IntegralValue(!operand, ValueType::boolType));
}

IndexParamInst::IndexParamInst(const std::string& name_) : Instruction(name_, "", ""), index(-1)
{
}

void IndexParamInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    writer.Put(index);
}

Instruction* IndexParamInst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    index = reader.GetInt();
    return this;
}

void IndexParamInst::Dump(CodeFormatter& formatter)
{
    Instruction::Dump(formatter);
    formatter.Write(" " + std::to_string(index));
}

LoadLocalInst::LoadLocalInst() : IndexParamInst("loadlocal")
{
}

void LoadLocalInst::Execute(Frame& frame)
{
    frame.OpStack().Push(frame.Local(Index()).GetValue());
}

StoreLocalInst::StoreLocalInst() : IndexParamInst("storelocal")
{
}

void StoreLocalInst::Execute(Frame& frame)
{
    frame.Local(Index()).SetValue(frame.OpStack().Pop());
}

LoadFieldInst::LoadFieldInst() : IndexParamInst("loadfield")
{
}

void LoadFieldInst::Execute(Frame& frame)
{
    IntegralValue operand = frame.OpStack().Pop();
    Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
    ObjectReference reference = ObjectReference(operand.Value());
    IntegralValue fieldValue = frame.GetManagedMemoryPool().GetField(reference, Index());
    frame.OpStack().Push(fieldValue);
}

StoreFieldInst::StoreFieldInst() : IndexParamInst("storefield")
{
}

void StoreFieldInst::Execute(Frame& frame)
{
    IntegralValue operand = frame.OpStack().Pop();
    Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
    ObjectReference reference = ObjectReference(operand.Value());
    IntegralValue fieldValue = frame.OpStack().Pop();
    frame.GetManagedMemoryPool().SetField(reference, Index(), fieldValue);
}

LoadElemInst::LoadElemInst() : Instruction("loadarrayelem")
{
}

void LoadElemInst::Execute(Frame& frame)
{
    IntegralValue index = frame.OpStack().Pop();
    Assert(index.GetType() == ValueType::intType, "int expected");
    IntegralValue arr = frame.OpStack().Pop();
    Assert(arr.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference arrayReference(arr.Value());
    frame.OpStack().Push(frame.GetManagedMemoryPool().GetArrayElement(arrayReference, index.AsInt()));
}

StoreElemInst::StoreElemInst() : Instruction("storearrayelem")
{
}

void StoreElemInst::Execute(Frame& frame)
{
    IntegralValue index = frame.OpStack().Pop();
    Assert(index.GetType() == ValueType::intType, "int expected");
    IntegralValue arr = frame.OpStack().Pop();
    Assert(arr.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference arrayReference(arr.Value());
    IntegralValue elementValue = frame.OpStack().Pop();
    frame.GetManagedMemoryPool().SetArrayElement(arrayReference, index.AsInt(), elementValue);
}

LoadConstantInst::LoadConstantInst() : IndexParamInst("loadconstant")
{
}

void LoadConstantInst::Execute(Frame& frame)
{
    ConstantId constantId(Index());
    frame.OpStack().Push(frame.GetConstantPool().GetConstant(constantId).Value());
}

ReceiveInst::ReceiveInst() : Instruction("receive")
{
}

void ReceiveInst::Execute(Frame& frame)
{
    int32_t n = frame.Fun().NumParameters();
    for (int32_t i = n - 1; i >= 0; --i)
    {
        IntegralValue argument = frame.OpStack().Pop();
        frame.Local(i).SetValue(argument);
    }
}

JumpInst::JumpInst() : IndexParamInst("jump")
{
    SetIndex(endOfFunction);
}

void JumpInst::SetTarget(int32_t target)
{
    SetIndex(target);
}

void JumpInst::Execute(Frame& frame)
{
    frame.SetPC(Index());
}

void JumpInst::Dump(CodeFormatter& formatter)
{
    Instruction::Dump(formatter);
    if (Index() == endOfFunction)
    {
        formatter.Write(" eof");
    }
    else
    {
        formatter.Write(" " + std::to_string(Index()));
    }
}

JumpTrueInst::JumpTrueInst() : IndexParamInst("jumptrue")
{
    SetIndex(endOfFunction);
}

void JumpTrueInst::SetTarget(int32_t target)
{
    SetIndex(target);
}

void JumpTrueInst::Execute(Frame& frame)
{
    IntegralValue value = frame.OpStack().Pop();
    Assert(value.GetType() == ValueType::boolType, "bool operand expected");
    if (value.AsBool())
    {
        frame.SetPC(Index());
    }
}

void JumpTrueInst::Dump(CodeFormatter& formatter)
{
    Instruction::Dump(formatter);
    if (Index() == endOfFunction)
    {
        formatter.Write(" eof");
    }
    else
    {
        formatter.Write(" " + std::to_string(Index()));
    }
}

JumpFalseInst::JumpFalseInst() : IndexParamInst("jumpfalse")
{
    SetIndex(endOfFunction);
}

void JumpFalseInst::SetTarget(int32_t target)
{
    SetIndex(target);
}

void JumpFalseInst::Execute(Frame& frame)
{
    IntegralValue value = frame.OpStack().Pop();
    Assert(value.GetType() == ValueType::boolType, "bool operand expected");
    if (!value.AsBool())
    {
        frame.SetPC(Index());
    }
}

void JumpFalseInst::Dump(CodeFormatter& formatter)
{
    Instruction::Dump(formatter);
    if (Index() == endOfFunction)
    {
        formatter.Write(" eof");
    }
    else
    {
        formatter.Write(" " + std::to_string(Index()));
    }
}

EnterBlockInst::EnterBlockInst() : Instruction("enterblock")
{
}

void EnterBlockInst::Execute(Frame& frame)
{
}

ExitBlockInst::ExitBlockInst() : Instruction("exitblock")
{
}

void ExitBlockInst::Execute(Frame& frame)
{
}

CallInst::CallInst() : Instruction("call")
{
}

void CallInst::SetFunctionCallName(Constant functionCallName)
{
    Assert(functionCallName.Value().GetType() == ValueType::stringLiteral, "string literal expected");
    function = functionCallName;
}

StringPtr CallInst::GetFunctionCallName() const
{
    Assert(function.Value().GetType() == ValueType::stringLiteral, "string literal expected");
    return StringPtr(function.Value().AsStringLiteral());
}

void CallInst::SetFunction(Function* fun)
{
    function.SetValue(IntegralValue(fun));
}

void CallInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    ConstantId id = writer.GetConstantPool()->GetIdFor(function);
    Assert(id != noConstantId, "id for call inst not found");
    id.Write(writer);
}

Instruction* CallInst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    ConstantId id = reader.GetInt();
    function = reader.GetConstantPool()->GetConstant(id);
    return this;
}

void CallInst::Execute(Frame& frame)
{
    Assert(function.Value().GetType() == ValueType::functionPtr, "function pointer expected");
    Function* fun = function.Value().AsFunctionPtr();
    frame.GetThread().Frames().push_back(Frame(frame.GetMachine(), frame.GetThread(), *fun));
}

void CallInst::Dump(CodeFormatter& formatter)
{
    Instruction::Dump(formatter);
    Assert(function.Value().GetType() == ValueType::functionPtr, "function pointer expected");
    Function* fun = function.Value().AsFunctionPtr();
    formatter.Write(" " + ToUtf8(fun->CallName().Value().AsStringLiteral()));
}

VirtualCallInst::VirtualCallInst() : Instruction("callv"), numArgs(0), vmtIndex(-1)
{
}

void VirtualCallInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    Assert(numArgs != 0, "invalid number of arguments");
    writer.Put(numArgs);
    Assert(vmtIndex != -1, "invalid vmt index");
    writer.Put(vmtIndex);
}

Instruction* VirtualCallInst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    numArgs = reader.GetInt();
    vmtIndex = reader.GetInt();
    return this;
}

void VirtualCallInst::Execute(Frame& frame)
{
    IntegralValue receiverValue = frame.OpStack().GetValue(numArgs);
    Assert(receiverValue.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference receiver(receiverValue.Value());
    IntegralValue classDataField = frame.GetManagedMemoryPool().GetField(receiver, 0);
    Assert(classDataField.GetType() == ValueType::classDataPtr, "class data field expected");
    ClassData* classData = classDataField.AsClassDataPtr();
    if (classData)
    {
        Function* method = classData->Vmt().GetMethod(vmtIndex);
        if (method)
        {
            frame.GetThread().Frames().push_back(Frame(frame.GetMachine(), frame.GetThread(), *method));
        }
        else
        {
            throw std::runtime_error("tried to call an abstract method");
        }
    }
    else
    {
        throw std::runtime_error("class data field not set");
    }
}

void VirtualCallInst::Dump(CodeFormatter& formatter)
{
    Instruction::Dump(formatter);
    formatter.Write(" " + std::to_string(numArgs) + " " + std::to_string(vmtIndex));
}

InterfaceCallInst::InterfaceCallInst() : Instruction("calli"), numArgs(0), imtIndex(-1)
{
}

void InterfaceCallInst::Execute(Frame& frame)
{
    IntegralValue interfaceObjectValue = frame.OpStack().GetValue(numArgs);
    Assert(interfaceObjectValue.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference interfaceObject(interfaceObjectValue.Value());
    IntegralValue receiverField = frame.GetManagedMemoryPool().GetField(interfaceObject, 0);
    Assert(receiverField.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference receiver(receiverField.Value());
    IntegralValue classDataField = frame.GetManagedMemoryPool().GetField(receiver, 0);
    Assert(classDataField.GetType() == ValueType::classDataPtr, "class data field expected");
    ClassData* classData = classDataField.AsClassDataPtr();
    if (classData)
    {
        IntegralValue itabIndex = frame.GetManagedMemoryPool().GetField(interfaceObject, 1);
        Assert(itabIndex.GetType() == ValueType::intType, "int expected");
        MethodTable& imt = classData->Imt(itabIndex.AsInt());
        Function* method = imt.GetMethod(imtIndex);
        if (method)
        {
            frame.OpStack().SetValue(numArgs, receiver);
            frame.GetThread().Frames().push_back(Frame(frame.GetMachine(), frame.GetThread(), *method));
        }
        else
        {
            throw std::runtime_error("interface method has no implementation");
        }
    }
    else
    {
        throw std::runtime_error("class data field not set");
    }
}

VmCallInst::VmCallInst() : IndexParamInst("callvm")
{
}

void VmCallInst::Execute(Frame& frame)
{
    ConstantId vmFunctionId(Index());
    Constant vmFunctionName = frame.GetConstantPool().GetConstant(vmFunctionId);
    Assert(vmFunctionName.Value().GetType() == ValueType::stringLiteral, "string literal expected");
    VmFunction* vmFunction = VmFunctionTable::Instance().GetVmFunction(StringPtr(vmFunctionName.Value().AsStringLiteral()));
    vmFunction->Execute(frame);
}

SetClassDataInst::SetClassDataInst() : Instruction("setclassdata")
{
}

void SetClassDataInst::SetClassName(Constant fullClassName)
{
    Assert(fullClassName.Value().GetType() == ValueType::stringLiteral, "string literal expected");
    classData = fullClassName;
}

StringPtr SetClassDataInst::GetClassName() const
{
    Assert(classData.Value().GetType() == ValueType::stringLiteral, "string literal expected");
    return StringPtr(classData.Value().AsStringLiteral());
}

void SetClassDataInst::SetClassData(ClassData* classDataPtr)
{
    classData.SetValue(IntegralValue(classDataPtr));
}

void SetClassDataInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    ConstantId id = writer.GetConstantPool()->GetIdFor(classData);
    Assert(id != noConstantId, "id for call inst not found");
    id.Write(writer);
}

Instruction* SetClassDataInst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    ConstantId id = reader.GetInt();
    classData = reader.GetConstantPool()->GetConstant(id);
    return this;
}

void SetClassDataInst::Execute(Frame& frame)
{
    IntegralValue value = frame.OpStack().Pop();
    Assert(value.GetType() == ValueType::objectReference, "object reference operand expected");
    ObjectReference objectReference(value.Value());
    IntegralValue classDataFieldValue = frame.GetManagedMemoryPool().GetField(objectReference, 0);
    Assert(classDataFieldValue.GetType() == ValueType::classDataPtr, "class data pointer expected");
    if (!classDataFieldValue.AsClassDataPtr())
    {
        Assert(classData.Value().GetType() == ValueType::classDataPtr, "class data pointer expected");
        ClassData* cd = classData.Value().AsClassDataPtr();
        classDataFieldValue = IntegralValue(cd);
        frame.GetManagedMemoryPool().SetField(objectReference, 0, classDataFieldValue);
    }
}

void SetClassDataInst::Dump(CodeFormatter& formatter)
{
    Instruction::Dump(formatter);
    Assert(classData.Value().GetType() == ValueType::classDataPtr, "class data pointer expected");
    ClassData* cd = classData.Value().AsClassDataPtr();
    formatter.Write(" " + ToUtf8(cd->Type()->Name().Value()) + " " + std::to_string(cd->Type()->Id()));
}

TypeInstruction::TypeInstruction(const std::string& name_) : Instruction(name_)
{
}

void TypeInstruction::SetTypeName(Constant fullTypeName)
{
    Assert(fullTypeName.Value().GetType() == ValueType::stringLiteral, "string literal expected");
    type = fullTypeName;
}

StringPtr TypeInstruction::GetTypeName() const
{
    Assert(type.Value().GetType() == ValueType::stringLiteral, "string literal expected");
    return StringPtr(type.Value().AsStringLiteral());
}

void TypeInstruction::SetType(Type* typePtr)
{
    type.SetValue(IntegralValue(typePtr));
}

Type* TypeInstruction::GetType()
{
    Assert(type.Value().GetType() == ValueType::typePtr, "type pointer expected");
    return type.Value().AsTypePtr();
}

void TypeInstruction::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    ConstantId id = writer.GetConstantPool()->GetIdFor(type);
    Assert(id != noConstantId, "id for type inst not found");
    id.Write(writer);
}

Instruction* TypeInstruction::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    ConstantId id = reader.GetInt();
    type = reader.GetConstantPool()->GetConstant(id);
    return this;
}

void TypeInstruction::Dump(CodeFormatter& formatter)
{
    Instruction::Dump(formatter);
    Assert(type.Value().GetType() == ValueType::typePtr, "type pointer expected");
    Type* objectType = type.Value().AsTypePtr();
    formatter.Write(" " + ToUtf8(objectType->Name().Value()));
}

CreateObjectInst::CreateObjectInst() : TypeInstruction("createo")
{
}

void CreateObjectInst::Execute(Frame& frame)
{
    Type* type = GetType();
    ObjectType* objectType = dynamic_cast<ObjectType*>(type);
    Assert(objectType, "object type expected");
    ObjectReference objectReference = frame.GetManagedMemoryPool().CreateObject(frame.GetThread(), objectType);
    frame.OpStack().Push(objectReference);
}

CopyObjectInst::CopyObjectInst() : Instruction("copyo", "copy", "object")
{
}

void CopyObjectInst::Execute(Frame& frame)
{
    IntegralValue value = frame.OpStack().Pop();
    Assert(value.GetType() == ValueType::objectReference, "object reference operand expected");
    ObjectReference objectReference(value.Value());
    ObjectReference copy = frame.GetManagedMemoryPool().CopyObject(objectReference);
    frame.OpStack().Push(copy);
}

StrLitToStringInst::StrLitToStringInst() : Instruction("slit2s")
{
}

void StrLitToStringInst::Execute(Frame& frame)
{
    IntegralValue value = frame.OpStack().Pop();
    Assert(value.GetType() == ValueType::stringLiteral, "string literal expected");
    ConstantId stringConstantId = frame.GetConstantPool().GetIdFor(Constant(value));
    Assert(stringConstantId != noConstantId, "id for string constant not found");
    const char32_t* strLit = value.AsStringLiteral();
    uint64_t len = frame.GetConstantPool().GetStringLength(stringConstantId);
    AllocationHandle handle = frame.GetManagedMemoryPool().CreateStringFromLiteral(strLit, len);
    frame.OpStack().Push(handle);
}

LengthStringInst::LengthStringInst() : Instruction("lens")
{
}

void LengthStringInst::Execute(Frame& frame)
{
    IntegralValue value = frame.OpStack().Pop();
    Assert(value.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference str(value.Value());
    int32_t len = frame.GetManagedMemoryPool().GetStringLength(str);
    frame.OpStack().Push(IntegralValue(len, ValueType::intType));
}

DupInst::DupInst() : Instruction("dup")
{
}

void DupInst::Execute(Frame& frame)
{
    frame.OpStack().Dup();
}

SwapInst::SwapInst() : Instruction("swap")
{
}

void SwapInst::Execute(Frame& frame)
{
    frame.OpStack().Swap();
}

RotateInst::RotateInst() : Instruction("rotate")
{
}

void RotateInst::Execute(Frame& frame)
{
    frame.OpStack().Rotate();
}

UpCastInst::UpCastInst() : TypeInstruction("upcast")
{
}

void UpCastInst::Execute(Frame& frame)
{
    IntegralValue value = frame.OpStack().Pop();
    Assert(value.GetType() == ValueType::objectReference, "object reference operand expected");
    ObjectReference objectReference(value.Value());
    if (objectReference.IsNull())
    {
        frame.OpStack().Push(AllocationHandle(0));
    }
    else
    {
        Type* type = GetType();
        ObjectType* objectType = dynamic_cast<ObjectType*>(type);
        Assert(objectType, "object type expected");
        ObjectReference casted = frame.GetManagedMemoryPool().CopyObject(objectReference);
        Object& castedObject = frame.GetManagedMemoryPool().GetObject(casted);
        castedObject.SetType(objectType);
        frame.OpStack().Push(casted);
    }
}

DownCastInst::DownCastInst() : TypeInstruction("downcast")
{
}

void DownCastInst::Execute(Frame& frame)
{
    IntegralValue value = frame.OpStack().Pop();
    Assert(value.GetType() == ValueType::objectReference, "object reference operand expected");
    ObjectReference objectReference(value.Value());
    if (objectReference.IsNull())
    {
        frame.OpStack().Push(AllocationHandle(0));
    }
    else
    {
        IntegralValue classDataField = frame.GetManagedMemoryPool().GetField(objectReference, 0);
        Assert(classDataField.GetType() == ValueType::classDataPtr, "class data pointer expected");
        ClassData* classData = classDataField.AsClassDataPtr();
        uint64_t sourceTypeId = classData->Type()->Id();
        Type* type = GetType();
        ObjectType* objectType = dynamic_cast<ObjectType*>(type);
        Assert(objectType, "object type expected");
        uint64_t targetTypeId = objectType->Id();
        if (sourceTypeId % targetTypeId != 0)
        {
            throw std::runtime_error("invalid cast");
        }
        ObjectReference casted = frame.GetManagedMemoryPool().CopyObject(objectReference);
        Object& castedObject = frame.GetManagedMemoryPool().GetObject(casted);
        castedObject.SetType(objectType);
        frame.OpStack().Push(casted);
    }
}

AllocateArrayElementsInst::AllocateArrayElementsInst() : TypeInstruction("allocelems")
{
}

void AllocateArrayElementsInst::Execute(Frame& frame)
{
    IntegralValue length = frame.OpStack().Pop();
    Assert(length.GetType() == ValueType::intType, "int expected");
    IntegralValue arrayValue = frame.OpStack().Pop();
    Assert(arrayValue.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference arr(arrayValue.Value());
    Type* elementType = GetType();
    frame.GetManagedMemoryPool().AllocateArrayElements(frame.GetThread(), arr, elementType, length.AsInt());
}

} } // namespace cminor::machine
