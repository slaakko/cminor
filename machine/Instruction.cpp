// =================================
// Copyright (c) 2017 Seppo Laakko
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

void Instruction::Clear()
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

void Instruction::SetIndex(uint8_t index_)
{
    throw std::runtime_error("instruction '" + name + "' does not support set index");
}

void Instruction::SetIndex(uint16_t index_)
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

void Instruction::DispatchTo(InstAdder& adder)
{
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

void IndexParamInst::Clear()
{
    index = -1;
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

ByteParamInst::ByteParamInst(const std::string& name_) : Instruction(name_, "", ""), index(0)
{
}

void ByteParamInst::Clear()
{
    index = 0;
}

void ByteParamInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    writer.Put(index);
}

Instruction* ByteParamInst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    index = reader.GetByte();
    return this;
}

void ByteParamInst::Dump(CodeFormatter& formatter)
{
    Instruction::Dump(formatter);
    formatter.Write(" " + std::to_string(index));
}

UShortParamInst::UShortParamInst(const std::string& name_) : Instruction(name_, "", ""), index(0)
{
}

void UShortParamInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    writer.Put(index);
}

void UShortParamInst::Clear()
{
    index = 0;
}

Instruction* UShortParamInst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    index = reader.GetUShort();
    return this;
}

void UShortParamInst::Dump(CodeFormatter& formatter)
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

LoadLocal0Inst::LoadLocal0Inst() : Instruction("loadlocal.0")
{
}

void LoadLocal0Inst::Execute(Frame& frame)
{
    frame.OpStack().Push(frame.Local(0).GetValue());
}

LoadLocal1Inst::LoadLocal1Inst() : Instruction("loadlocal.1")
{
}

void LoadLocal1Inst::Execute(Frame& frame)
{
    frame.OpStack().Push(frame.Local(1).GetValue());
}

LoadLocal2Inst::LoadLocal2Inst() : Instruction("loadlocal.2")
{
}

void LoadLocal2Inst::Execute(Frame& frame)
{
    frame.OpStack().Push(frame.Local(2).GetValue());
}

LoadLocal3Inst::LoadLocal3Inst() : Instruction("loadlocal.3")
{
}

void LoadLocal3Inst::Execute(Frame& frame)
{
    frame.OpStack().Push(frame.Local(3).GetValue());
}

LoadLocalBInst::LoadLocalBInst() : ByteParamInst("loadlocal.b")
{
}

void LoadLocalBInst::Execute(Frame& frame)
{
    frame.OpStack().Push(frame.Local(Index()).GetValue());
}

LoadLocalSInst::LoadLocalSInst() : UShortParamInst("loadlocal.s")
{
}

void LoadLocalSInst::Execute(Frame& frame)
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

StoreLocal0Inst::StoreLocal0Inst() : Instruction("storelocal.0")
{
}

void StoreLocal0Inst::Execute(Frame& frame)
{
    frame.Local(0).SetValue(frame.OpStack().Pop());
}

StoreLocal1Inst::StoreLocal1Inst() : Instruction("storelocal.1")
{
}

void StoreLocal1Inst::Execute(Frame& frame)
{
    frame.Local(1).SetValue(frame.OpStack().Pop());
}

StoreLocal2Inst::StoreLocal2Inst() : Instruction("storelocal.2")
{
}

void StoreLocal2Inst::Execute(Frame& frame)
{
    frame.Local(2).SetValue(frame.OpStack().Pop());
}

StoreLocal3Inst::StoreLocal3Inst() : Instruction("storelocal.3")
{
}

void StoreLocal3Inst::Execute(Frame& frame)
{
    frame.Local(3).SetValue(frame.OpStack().Pop());
}

StoreLocalBInst::StoreLocalBInst() : ByteParamInst("storelocal.b")
{
}

void StoreLocalBInst::Execute(Frame& frame)
{
    frame.Local(Index()).SetValue(frame.OpStack().Pop());
}

StoreLocalSInst::StoreLocalSInst() : UShortParamInst("storelocal.s")
{
}

void StoreLocalSInst::Execute(Frame& frame)
{
    frame.Local(Index()).SetValue(frame.OpStack().Pop());
}

LoadFieldInst::LoadFieldInst() : IndexParamInst("loadfield")
{
}

void LoadFieldInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = frame.GetManagedMemoryPool().GetField(reference, Index());
        frame.OpStack().Push(fieldValue);
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

LoadField0Inst::LoadField0Inst() : Instruction("loadfield.0")
{
}

void LoadField0Inst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = frame.GetManagedMemoryPool().GetField(reference, 0);
        frame.OpStack().Push(fieldValue);
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

LoadField1Inst::LoadField1Inst() : Instruction("loadfield.1")
{
}

void LoadField1Inst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = frame.GetManagedMemoryPool().GetField(reference, 1);
        frame.OpStack().Push(fieldValue);
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

LoadField2Inst::LoadField2Inst() : Instruction("loadfield.2")
{
}

void LoadField2Inst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = frame.GetManagedMemoryPool().GetField(reference, 2);
        frame.OpStack().Push(fieldValue);
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

LoadField3Inst::LoadField3Inst() : Instruction("loadfield.3")
{
}

void LoadField3Inst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = frame.GetManagedMemoryPool().GetField(reference, 3);
        frame.OpStack().Push(fieldValue);
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

LoadFieldBInst::LoadFieldBInst() : ByteParamInst("loadfield.b")
{
}

void LoadFieldBInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = frame.GetManagedMemoryPool().GetField(reference, Index());
        frame.OpStack().Push(fieldValue);
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

LoadFieldSInst::LoadFieldSInst() : UShortParamInst("loadfield.s")
{
}

void LoadFieldSInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = frame.GetManagedMemoryPool().GetField(reference, Index());
        frame.OpStack().Push(fieldValue);
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

StoreFieldInst::StoreFieldInst() : IndexParamInst("storefield")
{
}

void StoreFieldInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = frame.OpStack().Pop();
        frame.GetManagedMemoryPool().SetField(reference, Index(), fieldValue);
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

StoreField0Inst::StoreField0Inst() : Instruction("storefield.0")
{
}

void StoreField0Inst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = frame.OpStack().Pop();
        frame.GetManagedMemoryPool().SetField(reference, 0, fieldValue);
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

StoreField1Inst::StoreField1Inst() : Instruction("storefield.1")
{
}

void StoreField1Inst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = frame.OpStack().Pop();
        frame.GetManagedMemoryPool().SetField(reference, 1, fieldValue);
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

StoreField2Inst::StoreField2Inst() : Instruction("storefield.2")
{
}

void StoreField2Inst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = frame.OpStack().Pop();
        frame.GetManagedMemoryPool().SetField(reference, 2, fieldValue);
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

StoreField3Inst::StoreField3Inst() : Instruction("storefield.3")
{
}

void StoreField3Inst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = frame.OpStack().Pop();
        frame.GetManagedMemoryPool().SetField(reference, 3, fieldValue);
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

StoreFieldBInst::StoreFieldBInst() : ByteParamInst("storefield.b")
{
}

void StoreFieldBInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = frame.OpStack().Pop();
        frame.GetManagedMemoryPool().SetField(reference, Index(), fieldValue);
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

StoreFieldSInst::StoreFieldSInst() : UShortParamInst("storefield.s")
{
}

void StoreFieldSInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = frame.OpStack().Pop();
        frame.GetManagedMemoryPool().SetField(reference, Index(), fieldValue);
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

LoadElemInst::LoadElemInst() : Instruction("loadarrayelem")
{
}

void LoadElemInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue index = frame.OpStack().Pop();
        Assert(index.GetType() == ValueType::intType, "int expected");
        IntegralValue arr = frame.OpStack().Pop();
        Assert(arr.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference arrayReference(arr.Value());
        frame.OpStack().Push(frame.GetManagedMemoryPool().GetArrayElement(arrayReference, index.AsInt()));
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const IndexOutOfRangeException& ex)
    {
        ThrowIndexOutOfRangeException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

StoreElemInst::StoreElemInst() : Instruction("storearrayelem")
{
}

void StoreElemInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue index = frame.OpStack().Pop();
        Assert(index.GetType() == ValueType::intType, "int expected");
        IntegralValue arr = frame.OpStack().Pop();
        Assert(arr.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference arrayReference(arr.Value());
        IntegralValue elementValue = frame.OpStack().Pop();
        frame.GetManagedMemoryPool().SetArrayElement(arrayReference, index.AsInt(), elementValue);
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const IndexOutOfRangeException& ex)
    {
        ThrowIndexOutOfRangeException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

LoadConstantInst::LoadConstantInst() : IndexParamInst("loadconstant")
{
}

void LoadConstantInst::Execute(Frame& frame)
{
    ConstantId constantId(Index());
    frame.OpStack().Push(frame.GetConstantPool().GetConstant(constantId).Value());
}

LoadConstantBInst::LoadConstantBInst() : ByteParamInst("loadconstant.b")
{
}

void LoadConstantBInst::Execute(Frame& frame)
{
    ConstantId constantId(Index());
    frame.OpStack().Push(frame.GetConstantPool().GetConstant(constantId).Value());
}

LoadConstantSInst::LoadConstantSInst() : UShortParamInst("loadconstant.s")
{
}

void LoadConstantSInst::Execute(Frame& frame)
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

ExitBlockInst::ExitBlockInst() : Instruction("exitblock"), exceptionBlockId(-1)
{
}

void ExitBlockInst::Clear()
{
    exceptionBlockId = -1;
}

void ExitBlockInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    writer.Put(exceptionBlockId);
}

Instruction* ExitBlockInst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    exceptionBlockId = reader.GetInt();
    return this;
}

void ExitBlockInst::Execute(Frame& frame)
{
    if (exceptionBlockId != -1)
    {
        ExceptionBlock* exceptionBlock = frame.Fun().GetExceptionBlock(exceptionBlockId);
        Assert(exceptionBlock, "exception block not found");
        if (exceptionBlock->HasFinally())
        {
            frame.GetThread().PushExitBlock(frame.PC() + 1);
            frame.SetPC(exceptionBlock->FinallyStart());
        }
    }
}

void ExitBlockInst::Dump(CodeFormatter& formatter)
{
    Instruction::Dump(formatter);
    if (exceptionBlockId != -1)
    {
        formatter.Write(" ");
        formatter.Write(std::to_string(exceptionBlockId));
    }
}

ContinuousSwitchInst::ContinuousSwitchInst() : Instruction("cswitch"), begin(), end(), defaultTarget(-1)
{
}

void ContinuousSwitchInst::Clear()
{
    begin = IntegralValue();
    end = IntegralValue();
    targets.clear();
    defaultTarget = -1;
    nextTargetIndices.clear();
}

void ContinuousSwitchInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    begin.Write(writer);
    end.Write(writer);
    uint32_t numTargets = uint32_t(targets.size());
    writer.PutEncodedUInt(numTargets);
    for (int32_t target : targets)
    {
        writer.Put(target);
    }
    writer.Put(defaultTarget);
}

Instruction* ContinuousSwitchInst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    begin.Read(reader);
    end.Read(reader);
    uint32_t numTargets = reader.GetEncodedUInt();
    targets.clear();
    for (uint32_t i = 0; i < numTargets; ++i)
    {
        int32_t target = reader.GetInt();
        targets.push_back(target);
    }
    defaultTarget = reader.GetInt();
    return this;
}

void ContinuousSwitchInst::AddTarget(int32_t target)
{
    targets.push_back(target);
}

void ContinuousSwitchInst::SetTarget(int index, int32_t target)
{
    Assert(index >= 0 && index < targets.size(), "invalid switch target index");
    targets[index] = target;
}

void ContinuousSwitchInst::AddNextTargetIndex(int nextTargetIndex)
{
    nextTargetIndices.push_back(nextTargetIndex);
}

void ContinuousSwitchInst::SetTarget(int32_t target)
{
    for (int nextTargetIndex : nextTargetIndices)
    {
        targets[nextTargetIndex] = target;
    }
    if (defaultTarget == -1)
    {
        defaultTarget = target;
    }
}

void ContinuousSwitchInst::Execute(Frame& frame)
{
    IntegralValue cond = frame.OpStack().Pop();
    if (cond.Value() < begin.Value() || cond.Value() > end.Value())
    {
        frame.SetPC(defaultTarget);
    }
    else
    {
        uint64_t index = cond.Value() - begin.Value();
        Assert(index >= 0 && index < targets.size(), "invalid switch index");
        frame.SetPC(targets[index]);
    }
}

BinarySearchSwitchInst::BinarySearchSwitchInst() : Instruction("bswitch"), targets(), defaultTarget(-1)
{
}

void BinarySearchSwitchInst::Clear()
{
    targets.clear();
    defaultTarget = -1;
}

void BinarySearchSwitchInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    uint32_t n = uint32_t(targets.size());
    writer.PutEncodedUInt(n);
    for (const std::pair<IntegralValue, int32_t>& p : targets)
    {
        IntegralValue value = p.first;
        value.Write(writer);
        writer.Put(p.second);
    }
    writer.Put(defaultTarget);
}

Instruction* BinarySearchSwitchInst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    uint32_t n = reader.GetEncodedUInt();
    targets.clear();
    for (uint32_t i = 0; i < n; ++i)
    {
        IntegralValue value;
        value.Read(reader);
        int32_t target = reader.GetInt();
        targets.push_back(std::make_pair(value, target));
    }
    defaultTarget = reader.GetInt();
    return this;
}

void BinarySearchSwitchInst::Execute(Frame& frame)
{
    IntegralValue cond = frame.OpStack().Pop();
    std::pair<IntegralValue, int32_t> x(cond, -1);
    const auto& it = std::lower_bound(targets.cbegin(), targets.cend(), x, IntegralValueLess());
    if (it != targets.cend() && it->first.Value() == cond.Value())
    {
        frame.SetPC(it->second);
    }
    else
    {
        frame.SetPC(defaultTarget);
    }
}

void BinarySearchSwitchInst::SetTargets(const std::vector<std::pair<IntegralValue, int32_t>>& targets_)
{
    targets = targets_;
}

void BinarySearchSwitchInst::SetTarget(int32_t target)
{
    if (defaultTarget == -1)
    {
        defaultTarget = target;
    }
}

CallInst::CallInst() : Instruction("call")
{
}

void CallInst::Clear()
{
    function = Constant();
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
    ConstantId id;
    id.Read(reader);
    function = reader.GetConstantPool()->GetConstant(id);
    return this;
}

void CallInst::Execute(Frame& frame)
{
    Assert(function.Value().GetType() == ValueType::functionPtr, "function pointer expected");
    Function* fun = function.Value().AsFunctionPtr();
    Thread& thread = frame.GetThread();
    thread.Frames().push_back(std::unique_ptr<Frame>(new Frame(frame.GetMachine(), thread, *fun)));
    thread.MapFrame(thread.Frames().back().get());
}

void CallInst::Dump(CodeFormatter& formatter)
{
    Instruction::Dump(formatter);
    Assert(function.Value().GetType() == ValueType::functionPtr, "function pointer expected");
    Function* fun = function.Value().AsFunctionPtr();
    formatter.Write(" " + ToUtf8(fun->CallName().Value().AsStringLiteral()));
}

void CallInst::DispatchTo(InstAdder& adder)
{
    adder.Add(this);
}

VirtualCallInst::VirtualCallInst() : Instruction("callv"), numArgs(0), vmtIndex(-1)
{
}

void VirtualCallInst::Clear()
{
    numArgs = 0;
    vmtIndex = -1;
}

void VirtualCallInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    Assert(numArgs != 0, "invalid number of arguments");
    writer.PutEncodedUInt(numArgs);
    Assert(vmtIndex != -1, "invalid vmt index");
    writer.PutEncodedUInt(vmtIndex);
}

Instruction* VirtualCallInst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    numArgs = reader.GetEncodedUInt();
    vmtIndex = reader.GetEncodedUInt();
    return this;
}

void VirtualCallInst::Execute(Frame& frame)
{
    try
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
                Thread& thread = frame.GetThread();
                thread.Frames().push_back(std::unique_ptr<Frame>(new Frame(frame.GetMachine(), thread, *method)));
                thread.MapFrame(thread.Frames().back().get());
            }
            else
            {
                throw SystemException("tried to call an abstract method");
            }
        }
        else
        {
            throw SystemException("class data field not set");
        }
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
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

void InterfaceCallInst::Clear()
{
    numArgs = 0;
    imtIndex = -1;
}

void InterfaceCallInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    Assert(numArgs != 0, "invalid number of arguments");
    writer.PutEncodedUInt(numArgs);
    Assert(imtIndex != -1, "invalid imt index");
    writer.PutEncodedUInt(imtIndex);
}

Instruction* InterfaceCallInst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    numArgs = reader.GetEncodedUInt();
    imtIndex = reader.GetEncodedUInt();
    return this;
}

void InterfaceCallInst::Execute(Frame& frame)
{
    try
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
                Thread& thread = frame.GetThread();
                thread.Frames().push_back(std::unique_ptr<Frame>(new Frame(frame.GetMachine(), thread, *method)));
                thread.MapFrame(thread.Frames().back().get());
            }
            else
            {
                throw SystemException("interface method has no implementation");
            }
        }
        else
        {
            throw SystemException("class data field not set");
        }
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

void InterfaceCallInst::Dump(CodeFormatter& formatter)
{
    Instruction::Dump(formatter);
    formatter.Write(" " + std::to_string(numArgs) + " " + std::to_string(imtIndex));
}

VmCallInst::VmCallInst() : IndexParamInst("callvm")
{
}

void VmCallInst::Execute(Frame& frame)
{
    try
    {
        ConstantId vmFunctionId(Index());
        Constant vmFunctionName = frame.GetConstantPool().GetConstant(vmFunctionId);
        Assert(vmFunctionName.Value().GetType() == ValueType::stringLiteral, "string literal expected");
        VmFunction* vmFunction = VmFunctionTable::Instance().GetVmFunction(StringPtr(vmFunctionName.Value().AsStringLiteral()));
        vmFunction->Execute(frame);
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const IndexOutOfRangeException& ex)
    {
        ThrowIndexOutOfRangeException(ex, frame);
    }
    catch (const ArgumentOutOfRangeException& ex)
    {
        ThrowArgumentOutOfRangeException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

DelegateCallInst::DelegateCallInst() : Instruction("calld")
{
}

void DelegateCallInst::Execute(Frame& frame)
{
    IntegralValue dlg = frame.OpStack().Pop();
    Assert(dlg.GetType() == ValueType::functionPtr, "function pointer expected");
    Function* fun = dlg.AsFunctionPtr();
    if (!fun)
    {
        NullReferenceException ex("value of delegate is null");
        ThrowNullReferenceException(ex, frame);
    }
    else
    {
        Thread& thread = frame.GetThread();
        thread.Frames().push_back(std::unique_ptr<Frame>(new Frame(frame.GetMachine(), thread, *fun)));
        thread.MapFrame(thread.Frames().back().get());
    }
}

ClassDelegateCallInst::ClassDelegateCallInst() : Instruction("callcd")
{
}

void ClassDelegateCallInst::Execute(Frame& frame)
{
    IntegralValue classDlgValue = frame.OpStack().Pop();
    Assert(classDlgValue.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference classDelegateRef(classDlgValue.Value());
    Object& classDelegateObject = frame.GetManagedMemoryPool().GetObject(classDelegateRef);
    IntegralValue classObjectValue = classDelegateObject.GetField(1);
    Assert(classObjectValue.GetType() == ValueType::objectReference, "object reference expected");
    IntegralValue funValue = classDelegateObject.GetField(2);
    Assert(funValue.GetType() == ValueType::functionPtr, "function pointer expected");
    Function* fun = funValue.AsFunctionPtr();
    if (!fun)
    {
        NullReferenceException ex("value of class delegate is null");
        ThrowNullReferenceException(ex, frame);
    }
    else
    {
        frame.OpStack().Insert(fun->NumParameters() - 1, classObjectValue);
        Thread& thread = frame.GetThread();
        thread.Frames().push_back(std::unique_ptr<Frame>(new Frame(frame.GetMachine(), thread, *fun)));
        thread.MapFrame(thread.Frames().back().get());
    }
}


SetClassDataInst::SetClassDataInst() : Instruction("setclassdata")
{
}

void SetClassDataInst::Clear()
{
    classData = Constant();
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
    ConstantId id;
    id.Read(reader);
    classData = reader.GetConstantPool()->GetConstant(id);
    return this;
}

void SetClassDataInst::Execute(Frame& frame)
{
    try
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
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

void SetClassDataInst::Dump(CodeFormatter& formatter)
{
    Instruction::Dump(formatter);
    Assert(classData.Value().GetType() == ValueType::classDataPtr, "class data pointer expected");
    ClassData* cd = classData.Value().AsClassDataPtr();
    formatter.Write(" " + ToUtf8(cd->Type()->Name().Value()) + " " + std::to_string(cd->Type()->Id()));
}

void SetClassDataInst::DispatchTo(InstAdder& adder)
{
    adder.Add(this);
}

TypeInstruction::TypeInstruction(const std::string& name_) : Instruction(name_)
{
}

void TypeInstruction::Clear()
{
    type = Constant();
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
    ConstantId id;
    id.Read(reader);
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

void TypeInstruction::DispatchTo(InstAdder& adder)
{
    adder.Add(this);
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
    try
    {
        IntegralValue value = frame.OpStack().Pop();
        Assert(value.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference objectReference(value.Value());
        ObjectReference copy = frame.GetManagedMemoryPool().CopyObject(frame.GetThread(), objectReference);
        frame.OpStack().Push(copy);
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

StrLitToStringInst::StrLitToStringInst() : Instruction("slit2s")
{
}

void StrLitToStringInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue value = frame.OpStack().Pop();
        Assert(value.GetType() == ValueType::stringLiteral, "string literal expected");
        const char32_t* strLit = value.AsStringLiteral();
        uint32_t len = static_cast<uint32_t>(StringLen(strLit));
        Type* type = TypeTable::Instance().GetType(StringPtr(U"System.String"));
        ObjectType* objectType = dynamic_cast<ObjectType*>(type);
        Assert(objectType, "object type expected");
        ObjectReference objectReference = frame.GetManagedMemoryPool().CreateObject(frame.GetThread(), objectType);
        Object& o = frame.GetManagedMemoryPool().GetObject(objectReference);
        o.Pin();
        AllocationHandle charsHandle = frame.GetManagedMemoryPool().CreateStringCharsFromLiteral(frame.GetThread(), strLit, len);
        ClassData* classData = ClassDataTable::Instance().GetClassData(StringPtr(U"System.String"));
        o.SetField(IntegralValue(classData), 0);
        o.SetField(IntegralValue(static_cast<int32_t>(len), ValueType::intType), 1);
        o.SetField(charsHandle, 2);
        frame.OpStack().Push(objectReference);
        o.Unpin();
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

LoadStringCharInst::LoadStringCharInst() : Instruction("loadstringchar")
{
}

void LoadStringCharInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue index = frame.OpStack().Pop();
        Assert(index.GetType() == ValueType::intType, "int expected");
        IntegralValue str = frame.OpStack().Pop();
        Assert(str.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference strReference(str.Value());
        frame.OpStack().Push(frame.GetManagedMemoryPool().GetStringChar(strReference, index.AsInt()));
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
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

PopInst::PopInst() : Instruction("pop")
{
}

void PopInst::Execute(Frame& frame)
{
    frame.OpStack().Pop();
}

UpCastInst::UpCastInst() : TypeInstruction("upcast")
{
}

void UpCastInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue value = frame.OpStack().Pop();
        Assert(value.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference objectReference(value.Value());
        if (objectReference.IsNull())
        {
            frame.OpStack().Push(ObjectReference(0));
        }
        else
        {
            Type* type = GetType();
            ObjectType* objectType = dynamic_cast<ObjectType*>(type);
            Assert(objectType, "object type expected");
            ObjectReference casted = frame.GetManagedMemoryPool().CopyObject(frame.GetThread(), objectReference);
            frame.OpStack().Push(casted);
        }
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

DownCastInst::DownCastInst() : TypeInstruction("downcast")
{
}

void DownCastInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue value = frame.OpStack().Pop();
        Assert(value.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference objectReference(value.Value());
        if (objectReference.IsNull())
        {
            frame.OpStack().Push(ObjectReference(0));
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
                throw InvalidCastException("invalid cast from '" + ToUtf8(classData->Type()->Name().Value()) + "' to '" + ToUtf8(type->Name().Value()));
            }
            ObjectReference casted = frame.GetManagedMemoryPool().CopyObject(frame.GetThread(), objectReference);
            frame.OpStack().Push(casted);
        }
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const InvalidCastException& ex)
    {
        ThrowInvalidCastException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

BeginTryInst::BeginTryInst() : Instruction("begintry")
{
}

void BeginTryInst::Execute(Frame& frame)
{
    frame.GetThread().BeginTry();
}

EndTryInst::EndTryInst() : Instruction("endtry")
{
}

void EndTryInst::Execute(Frame& frame)
{
    frame.GetThread().EndTry();
}

ThrowInst::ThrowInst() : Instruction("throw")
{
}

void ThrowInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue exceptionValue = frame.OpStack().Pop();
        Assert(exceptionValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference exception(exceptionValue.Value());
        Object& exceptionObject = frame.GetManagedMemoryPool().GetObject(exception);
        exceptionObject.Pin();
        utf32_string stackTraceStr = frame.GetThread().GetStackTrace();
        ObjectReference stackTrace = frame.GetManagedMemoryPool().CreateString(frame.GetThread(), stackTraceStr);
        exceptionObject.SetField(stackTrace, 2);
        frame.GetThread().HandleException(exception);
        exceptionObject.Unpin();
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

RethrowInst::RethrowInst() : Instruction("rethrow")
{
}

void RethrowInst::Execute(Frame& frame)
{
    throw std::runtime_error("exception"); // todo
}

EndCatchInst::EndCatchInst() : Instruction("endcatch")
{
}

void EndCatchInst::Execute(Frame& frame)
{
    frame.GetThread().EndCatch();
}

EndFinallyInst::EndFinallyInst() : Instruction("endfinally")
{
}

void EndFinallyInst::Execute(Frame& frame)
{
    frame.GetThread().EndFinally();
}

NextInst::NextInst() : Instruction("next"), exceptionBlock(nullptr)
{
}

void NextInst::Clear()
{
    exceptionBlock = nullptr;
}

void NextInst::SetTarget(int32_t target)
{
    exceptionBlock->SetNextTarget(target);
}

StaticInitInst::StaticInitInst() : TypeInstruction("staticinit")
{
}

void StaticInitInst::Execute(Frame& frame)
{
    try
    {
        Type* type = GetType();
        ObjectType* objectType = dynamic_cast<ObjectType*>(type);
        Assert(objectType, "object type expected");
        ClassData* classData = ClassDataTable::Instance().GetClassData(objectType->Name());
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData) return;
        if (staticClassData->Initialized()) return;
        staticClassData->Lock();
        if (!staticClassData->Initialized() && !staticClassData->Initializing())
        {
            staticClassData->SetInitializing();
            staticClassData->AllocateStaticData();
            StringPtr staticConstructorName = staticClassData->StaticConstructorName().Value().AsStringLiteral();
            if (staticConstructorName.Value())
            {
                Function* staticConstructor = FunctionTable::Instance().GetFunction(staticConstructorName);
                Thread& thread = frame.GetThread();
                thread.Frames().push_back(std::unique_ptr<Frame>(new Frame(frame.GetMachine(), thread, *staticConstructor)));
                thread.MapFrame(thread.Frames().back().get());
            }
            else
            {
                staticClassData->ResetInitializing();
                staticClassData->SetInitialized();
                staticClassData->Unlock();
            }
        }
        else
        {
            staticClassData->Unlock();
        }
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

DoneStaticInitInst::DoneStaticInitInst() : TypeInstruction("donestaticinit")
{
}

void DoneStaticInitInst::Execute(Frame& frame)
{
    try
    {
        Type* type = GetType();
        ObjectType* objectType = dynamic_cast<ObjectType*>(type);
        Assert(objectType, "object type expected");
        ClassData* classData = ClassDataTable::Instance().GetClassData(objectType->Name());
        StaticClassData* staticClassData = classData->GetStaticClassData();
        Assert(staticClassData, "class has no static data");
        staticClassData->ResetInitializing();
        staticClassData->SetInitialized();
        staticClassData->Unlock();
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

LoadStaticFieldInst::LoadStaticFieldInst() : TypeInstruction("loadstaticfield"), index(-1)
{
}

void LoadStaticFieldInst::Clear()
{
    TypeInstruction::Clear();
    index = -1;
}

void LoadStaticFieldInst::SetIndex(int32_t index_)
{
    index = index_;
}

void LoadStaticFieldInst::Encode(Writer& writer)
{
    TypeInstruction::Encode(writer);
    writer.Put(index);
}

Instruction* LoadStaticFieldInst::Decode(Reader& reader)
{
    TypeInstruction::Decode(reader);
    index = reader.GetInt();
    return this;
}

void LoadStaticFieldInst::Execute(Frame& frame)
{
    try
    {
        Type* type = GetType();
        ObjectType* objectType = dynamic_cast<ObjectType*>(type);
        Assert(objectType, "object type expected");
        ClassData* classData = ClassDataTable::Instance().GetClassData(objectType->Name());
        StaticClassData* staticData = classData->GetStaticClassData();
        Assert(staticData, "class has no static data");
        Assert(index != -1, "index not set");
        frame.OpStack().Push(staticData->GetStaticField(index));
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

StoreStaticFieldInst::StoreStaticFieldInst() : TypeInstruction("storestaticfield"), index(-1)
{
}

void StoreStaticFieldInst::Clear()
{
    TypeInstruction::Clear();
    index = -1;
}

void StoreStaticFieldInst::SetIndex(int32_t index_)
{
    index = index_;
}

void StoreStaticFieldInst::Encode(Writer& writer)
{
    TypeInstruction::Encode(writer);
    writer.Put(index);
}

Instruction* StoreStaticFieldInst::Decode(Reader& reader)
{
    TypeInstruction::Decode(reader);
    index = reader.GetInt();
    return this;
}

void StoreStaticFieldInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue fieldValue = frame.OpStack().Pop();
        Type* type = GetType();
        ObjectType* objectType = dynamic_cast<ObjectType*>(type);
        Assert(objectType, "object type expected");
        ClassData* classData = ClassDataTable::Instance().GetClassData(objectType->Name());
        StaticClassData* staticData = classData->GetStaticClassData();
        Assert(staticData, "class has no static data");
        Assert(index != -1, "index not set");
        staticData->SetStaticField(fieldValue, index);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

EqualObjectNullInst::EqualObjectNullInst() : Instruction("equalonull")
{
}

void EqualObjectNullInst::Execute(Frame& frame)
{
    IntegralValue rightValue = frame.OpStack().Pop();
    IntegralValue leftValue = frame.OpStack().Pop();
    Assert(leftValue.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference left(leftValue.Value());
    bool result = left.IsNull();
    frame.OpStack().Push(IntegralValue(result, ValueType::boolType));
}

EqualNullObjectInst::EqualNullObjectInst() : Instruction("equalnullo")
{
}

void EqualNullObjectInst::Execute(Frame& frame)
{
    IntegralValue rightValue = frame.OpStack().Pop();
    IntegralValue leftValue = frame.OpStack().Pop();
    Assert(rightValue.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference right(rightValue.Value());
    bool result = right.IsNull();
    frame.OpStack().Push(IntegralValue(result, ValueType::boolType));
}

AllocateArrayElementsInst::AllocateArrayElementsInst() : TypeInstruction("allocelems")
{
}

void AllocateArrayElementsInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue length = frame.OpStack().Pop();
        Assert(length.GetType() == ValueType::intType, "int expected");
        IntegralValue arrayValue = frame.OpStack().Pop();
        Assert(arrayValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference arr(arrayValue.Value());
        Type* elementType = GetType();
        frame.GetManagedMemoryPool().AllocateArrayElements(frame.GetThread(), arr, elementType, length.AsInt());
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const ArgumentOutOfRangeException& ex)
    {
        ThrowArgumentOutOfRangeException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

IsInst::IsInst() : TypeInstruction("is")
{
}

void IsInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue value = frame.OpStack().Pop();
        Assert(value.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference ob(value.Value());
        if (ob.IsNull())
        {
            frame.OpStack().Push(IntegralValue(false, ValueType::boolType));
        }
        else
        {
            IntegralValue classDataField = frame.GetManagedMemoryPool().GetField(ob, 0);
            Assert(classDataField.GetType() == ValueType::classDataPtr, "class data pointer expected");
            ClassData* classData = classDataField.AsClassDataPtr();
            uint64_t sourceTypeId = classData->Type()->Id();
            Type* type = GetType();
            ObjectType* objectType = dynamic_cast<ObjectType*>(type);
            Assert(objectType, "object type expected");
            uint64_t targetId = objectType->Id();
            bool result = sourceTypeId % targetId == 0;
            frame.OpStack().Push(IntegralValue(result, ValueType::boolType));
        }
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

AsInst::AsInst() : TypeInstruction("as")
{
}

void AsInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue value = frame.OpStack().Pop();
        Assert(value.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference ob(value.Value());
        if (ob.IsNull())
        {
            frame.OpStack().Push(ObjectReference(0));
        }
        else
        {
            IntegralValue classDataField = frame.GetManagedMemoryPool().GetField(ob, 0);
            Assert(classDataField.GetType() == ValueType::classDataPtr, "class data pointer expected");
            ClassData* classData = classDataField.AsClassDataPtr();
            uint64_t sourceTypeId = classData->Type()->Id();
            Type* type = GetType();
            ObjectType* objectType = dynamic_cast<ObjectType*>(type);
            Assert(objectType, "object type expected");
            uint64_t targetId = objectType->Id();
            bool result = sourceTypeId % targetId == 0;
            if (result)
            {
                frame.OpStack().Push(ob);
            }
            else
            {
                frame.OpStack().Push(ObjectReference(0));
            }
        }
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

Fun2DlgInst::Fun2DlgInst() : Instruction("fun2dlg")
{
}

void Fun2DlgInst::Clear()
{
    function = Constant();
}

void Fun2DlgInst::SetFunctionName(Constant functionName)
{
    function = functionName;
}

StringPtr Fun2DlgInst::GetFunctionName() const
{
    Assert(function.Value().GetType() == ValueType::stringLiteral, "string literal expected");
    return StringPtr(function.Value().AsStringLiteral());
}

void Fun2DlgInst::SetFunction(Function* fun)
{
    function.SetValue(IntegralValue(fun));
}

void Fun2DlgInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    ConstantId id = writer.GetConstantPool()->GetIdFor(function);
    Assert(id != noConstantId, "id for call inst not found");
    id.Write(writer);
}

Instruction* Fun2DlgInst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    ConstantId id;
    id.Read(reader);
    function = reader.GetConstantPool()->GetConstant(id);
    return this;
}

void Fun2DlgInst::Execute(Frame& frame)
{
    Assert(function.Value().GetType() == ValueType::functionPtr, "function pointer expected");
    frame.OpStack().Push(function.Value().AsFunctionPtr());
}

void Fun2DlgInst::Dump(CodeFormatter& formatter)
{
    Instruction::Dump(formatter);
    Assert(function.Value().GetType() == ValueType::functionPtr, "function pointer expected");
    Function* fun = function.Value().AsFunctionPtr();
    formatter.Write(" " + ToUtf8(fun->CallName().Value().AsStringLiteral()));
}

void Fun2DlgInst::DispatchTo(InstAdder& adder)
{
    adder.Add(this);
}

MemFun2ClassDlgInst::MemFun2ClassDlgInst() : Instruction("memfun2classdlg")
{
}

void MemFun2ClassDlgInst::Clear()
{
    function = Constant();
}

void MemFun2ClassDlgInst::SetFunctionName(Constant functionName)
{
    function = functionName;
}

StringPtr MemFun2ClassDlgInst::GetFunctionName() const
{
    Assert(function.Value().GetType() == ValueType::stringLiteral, "string literal expected");
    return StringPtr(function.Value().AsStringLiteral());
}

void MemFun2ClassDlgInst::SetFunction(Function* fun)
{
    function.SetValue(IntegralValue(fun));
}

void MemFun2ClassDlgInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    ConstantId id = writer.GetConstantPool()->GetIdFor(function);
    Assert(id != noConstantId, "id for call inst not found");
    id.Write(writer);
}

Instruction* MemFun2ClassDlgInst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    ConstantId id;
    id.Read(reader);
    function = reader.GetConstantPool()->GetConstant(id);
    return this;
}

void MemFun2ClassDlgInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue classObjectValue = frame.OpStack().Pop();
        Assert(classObjectValue.GetType() == ValueType::objectReference, "object reference expected");
        Assert(function.Value().GetType() == ValueType::functionPtr, "function pointer expected");
        IntegralValue value = frame.OpStack().Pop();
        Assert(value.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference classDelegateObjectRef(value.Value());
        Object& object = frame.GetManagedMemoryPool().GetObject(classDelegateObjectRef);
        object.SetField(classObjectValue, 1);
        object.SetField(function.Value(), 2);
        frame.OpStack().Push(value);
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frame);
    }
}

void MemFun2ClassDlgInst::Dump(CodeFormatter& formatter)
{
    Instruction::Dump(formatter);
    Assert(function.Value().GetType() == ValueType::functionPtr, "function pointer expected");
    Function* fun = function.Value().AsFunctionPtr();
    formatter.Write(" " + ToUtf8(fun->CallName().Value().AsStringLiteral()));
}

void MemFun2ClassDlgInst::DispatchTo(InstAdder& adder)
{
    adder.Add(this);
}

CreateLocalVariableReferenceInst::CreateLocalVariableReferenceInst() : Instruction("createlocalref"), localIndex(-1)
{
}

void CreateLocalVariableReferenceInst::Clear()
{
    localIndex = -1;
}

void CreateLocalVariableReferenceInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    writer.Put(localIndex);
}

Instruction* CreateLocalVariableReferenceInst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    localIndex = reader.GetInt();
    return this;
}

void CreateLocalVariableReferenceInst::Execute(Frame& frame)
{
    int32_t variableReferenceId = frame.GetThread().GetNextVariableReferenceId();
    frame.AddVariableReference(new LocalVariableReference(variableReferenceId, frame.Id(), localIndex));
    frame.OpStack().Push(IntegralValue(variableReferenceId, ValueType::variableReference));
}

CreateMemberVariableReferenceInst::CreateMemberVariableReferenceInst() : Instruction("createfieldref"), memberVarIndex(-1)
{
}

void CreateMemberVariableReferenceInst::Clear()
{
    memberVarIndex = -1;
}

void CreateMemberVariableReferenceInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    writer.Put(memberVarIndex);
}

Instruction* CreateMemberVariableReferenceInst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    memberVarIndex = reader.GetInt();
    return this;
}

void CreateMemberVariableReferenceInst::Execute(Frame& frame)
{
    IntegralValue objectValue = frame.OpStack().Pop();
    Assert(objectValue.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference objectReference(objectValue.Value());
    int32_t variableReferenceId = frame.GetThread().GetNextVariableReferenceId();
    frame.AddVariableReference(new MemberVariableReference(variableReferenceId, objectReference, memberVarIndex));
    frame.OpStack().Push(IntegralValue(variableReferenceId, ValueType::variableReference));
}

LoadVariableReferenceInst::LoadVariableReferenceInst() : IndexParamInst("loadref")
{
}

void LoadVariableReferenceInst::Execute(Frame& frame)
{
    IntegralValue refValue = frame.Local(Index()).GetValue();
    Assert(refValue.GetType() == ValueType::variableReference, "variable reference expected");
    int32_t variableReferenceId = refValue.AsInt();
    VariableReference* variableReference = frame.GetThread().GetVariableReference(variableReferenceId);
    Assert(variableReference, "variable reference not found");
    variableReference->DispatchTo(this, frame);
}

void LoadVariableReferenceInst::Handle(Frame& frame, LocalVariableReference* localVariableReference)
{
    Frame* frm = frame.GetThread().GetFrame(localVariableReference->FrameId());
    Assert(frm, "frame not found");
    IntegralValue value = frm->Local(localVariableReference->LocalIndex()).GetValue();
    if (value.GetType() == ValueType::variableReference)
    {
        int32_t variableReferenceId = value.AsInt();
        VariableReference* variableReference = frame.GetThread().GetVariableReference(variableReferenceId);
        Assert(variableReference, "variable reference not found");
        variableReference->DispatchTo(this, frame);
    }
    else
    {
        frame.OpStack().Push(value);
    }
}

void LoadVariableReferenceInst::Handle(Frame& frame, MemberVariableReference* memberVariableRefeence)
{
    Object& object = frame.GetManagedMemoryPool().GetObject(memberVariableRefeence->GetObjectReference());
    IntegralValue value = object.GetField(memberVariableRefeence->MemberVarIndex());
    if (value.GetType() == ValueType::variableReference)
    {
        int32_t variableReferenceId = value.AsInt();
        VariableReference* variableReference = frame.GetThread().GetVariableReference(variableReferenceId);
        Assert(variableReference, "variable reference not found");
        variableReference->DispatchTo(this, frame);
    }
    else
    {
        frame.OpStack().Push(value);
    }
}

StoreVariableReferenceInst::StoreVariableReferenceInst() : IndexParamInst("storeref")
{
}

void StoreVariableReferenceInst::Execute(Frame& frame)
{
    IntegralValue refValue = frame.Local(Index()).GetValue();
    Assert(refValue.GetType() == ValueType::variableReference, "variable reference expected");
    int32_t variableReferenceId = refValue.AsInt();
    VariableReference* variableReference = frame.GetThread().GetVariableReference(variableReferenceId);
    Assert(variableReference, "variable reference not found");
    variableReference->DispatchTo(this, frame);
}

void StoreVariableReferenceInst::Handle(Frame& frame, LocalVariableReference* localVariableReference)
{
    Frame* frm = frame.GetThread().GetFrame(localVariableReference->FrameId());
    Assert(frm, "frame not found");
    IntegralValue oldValue = frm->Local(localVariableReference->LocalIndex()).GetValue();
    if (oldValue.GetType() == ValueType::variableReference)
    {
        int32_t variableReferenceId = oldValue.AsInt();
        VariableReference* variableReference = frame.GetThread().GetVariableReference(variableReferenceId);
        Assert(variableReference, "variable reference not found");
        variableReference->DispatchTo(this, frame);
    }
    else
    {
        IntegralValue value = frame.OpStack().Pop();
        frm->Local(localVariableReference->LocalIndex()).SetValue(value);
    }
}

void StoreVariableReferenceInst::Handle(Frame& frame, MemberVariableReference* memberVariableReference)
{
    Object& object = frame.GetManagedMemoryPool().GetObject(memberVariableReference->GetObjectReference());
    IntegralValue oldValue = object.GetField(memberVariableReference->MemberVarIndex());
    if (oldValue.GetType() == ValueType::variableReference)
    {
        int32_t variableReferenceId = oldValue.AsInt();
        VariableReference* variableReference = frame.GetThread().GetVariableReference(variableReferenceId);
        Assert(variableReference, "variable reference not found");
        variableReference->DispatchTo(this, frame);
    }
    else
    {
        IntegralValue value = frame.OpStack().Pop();
        object.SetField(value, memberVariableReference->MemberVarIndex());
    }
}

void ThrowException(const std::string& message, Frame& frame, const utf32_string& exceptionTypeName)
{
    Type* type = TypeTable::Instance().GetType(StringPtr(exceptionTypeName.c_str()));
    ObjectType* objectType = dynamic_cast<ObjectType*>(type);
    Assert(objectType, "object type expected");
    ObjectReference objectReference = frame.GetManagedMemoryPool().CreateObject(frame.GetThread(), objectType);
    ClassData* classData = ClassDataTable::Instance().GetClassData(StringPtr(exceptionTypeName.c_str()));
    Object& o = frame.GetManagedMemoryPool().GetObject(objectReference);
    o.Pin();
    IntegralValue classDataValue(classData);
    o.SetField(classDataValue, 0);
    ObjectReference messageStr = frame.GetManagedMemoryPool().CreateString(frame.GetThread(), ToUtf32(message));
    o.SetField(messageStr, 1);
    frame.OpStack().Push(objectReference);
    ThrowInst throwInst;
    throwInst.Execute(frame);
    o.Unpin();
}

void ThrowSystemException(const SystemException& ex, Frame& frame)
{
    ThrowException(ex.what(), frame, U"System.SystemException");
}

void ThrowNullReferenceException(const NullReferenceException& ex, Frame& frame)
{
    ThrowException(ex.what(), frame, U"System.NullReferenceException");
}

void ThrowIndexOutOfRangeException(const IndexOutOfRangeException& ex, Frame& frame)
{
    ThrowException(ex.what(), frame, U"System.IndexOutOfRangeException");
}

void ThrowArgumentOutOfRangeException(const ArgumentOutOfRangeException& ex, Frame& frame)
{
    ThrowException(ex.what(), frame, U"System.ArgumentOutOfRangeException");
}

void ThrowInvalidCastException(const InvalidCastException& ex, Frame& frame)
{
    ThrowException(ex.what(), frame, U"System.InvalidCastException");
}

void ThrowFileSystemException(const FileSystemError& ex, Frame& frame)
{
    ThrowException(ex.what(), frame, U"System.FileSystemException");
}

} } // namespace cminor::machine
