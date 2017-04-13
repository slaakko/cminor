// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Instruction.hpp>
#include <cminor/machine/Error.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/util/Util.hpp>
#include <cminor/util/String.hpp>
#include <cminor/util/TextUtils.hpp>
#include <cminor/machine/Constant.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/machine/Class.hpp>
#include <cminor/machine/Type.hpp>
#include <cminor/machine/MachineFunctionVisitor.hpp>

namespace cminor { namespace machine {

Machine* machine = nullptr;
ManagedMemoryPool* managedMemoryPool = nullptr;

MACHINE_API Machine& GetMachine() 
{ 
    Assert(machine, "machine not set"); 
    return *machine; 
}

void SetMachine(Machine* machine_)
{
    machine = machine_;
}

MACHINE_API ManagedMemoryPool& GetManagedMemoryPool()
{ 
    Assert(managedMemoryPool, "managed memory pool not set"); 
    return *managedMemoryPool; 
}

void SetManagedMemoryPool(ManagedMemoryPool* managedMemoryPool_)
{
    managedMemoryPool = managedMemoryPool_;
}

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

void Instruction::Accept(MachineFunctionVisitor& visitor)
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

void InvalidInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitInvalidInst(*this);
}

NopInst::NopInst() : Instruction("nop", "", "")
{
}

void NopInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitNopInst(*this);
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

LoadDefaultValueBaseInst::LoadDefaultValueBaseInst(const std::string& name_, const std::string& groupName_, const std::string& typeName_) : Instruction(name_, groupName_, typeName_)
{
}

void LoadDefaultValueBaseInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLoadDefaultValueBaseInst(*this);
}

UnaryOpBaseInst::UnaryOpBaseInst(const std::string& name_, const std::string& groupName_, const std::string& typeName_) : Instruction(name_, groupName_, typeName_)
{
}

void UnaryOpBaseInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitUnaryOpBaseInst(*this);
}

BinaryOpBaseInst::BinaryOpBaseInst(const std::string& name_, const std::string& groupName_, const std::string& typeName_) : Instruction(name_, groupName_, typeName_)
{
}

void BinaryOpBaseInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitBinaryOpBaseInst(*this);
}

BinaryPredBaseInst::BinaryPredBaseInst(const std::string& name_, const std::string& groupName_, const std::string& typeName_) : Instruction(name_, groupName_, typeName_)
{
}

void BinaryPredBaseInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitBinaryPredBaseInst(*this);
}

LogicalNotInst::LogicalNotInst() : Instruction("not", "not", "System.Boolean")
{
}

void LogicalNotInst::Execute(Frame& frame)
{
    IntegralValue value = frame.OpStack().Pop();
    Assert(value.GetType() == ValueType::boolType, "bool operand expected");
    bool operand = value.AsBool();
    frame.OpStack().Push(IntegralValue(static_cast<uint64_t>(!operand), ValueType::boolType));
}

void LogicalNotInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLogicalNotInst(*this);
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

void LoadLocalInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLoadLocalInst(Index());
}

LoadLocal0Inst::LoadLocal0Inst() : Instruction("loadlocal.0")
{
}

void LoadLocal0Inst::Execute(Frame& frame)
{
    frame.OpStack().Push(frame.Local(0).GetValue());
}

void LoadLocal0Inst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLoadLocalInst(0);
}

LoadLocal1Inst::LoadLocal1Inst() : Instruction("loadlocal.1")
{
}

void LoadLocal1Inst::Execute(Frame& frame)
{
    frame.OpStack().Push(frame.Local(1).GetValue());
}

void LoadLocal1Inst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLoadLocalInst(1);
}

LoadLocal2Inst::LoadLocal2Inst() : Instruction("loadlocal.2")
{
}

void LoadLocal2Inst::Execute(Frame& frame)
{
    frame.OpStack().Push(frame.Local(2).GetValue());
}

void LoadLocal2Inst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLoadLocalInst(2);
}

LoadLocal3Inst::LoadLocal3Inst() : Instruction("loadlocal.3")
{
}

void LoadLocal3Inst::Execute(Frame& frame)
{
    frame.OpStack().Push(frame.Local(3).GetValue());
}

void LoadLocal3Inst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLoadLocalInst(3);
}

LoadLocalBInst::LoadLocalBInst() : ByteParamInst("loadlocal.b")
{
}

void LoadLocalBInst::Execute(Frame& frame)
{
    frame.OpStack().Push(frame.Local(Index()).GetValue());
}

void LoadLocalBInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLoadLocalInst(Index());
}

LoadLocalSInst::LoadLocalSInst() : UShortParamInst("loadlocal.s")
{
}

void LoadLocalSInst::Execute(Frame& frame)
{
    frame.OpStack().Push(frame.Local(Index()).GetValue());
}

void LoadLocalSInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLoadLocalInst(Index());
}

StoreLocalInst::StoreLocalInst() : IndexParamInst("storelocal")
{
}

void StoreLocalInst::Execute(Frame& frame)
{
    frame.Local(Index()).SetValue(frame.OpStack().Pop());
}

void StoreLocalInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitStoreLocalInst(Index());
}

StoreLocal0Inst::StoreLocal0Inst() : Instruction("storelocal.0")
{
}

void StoreLocal0Inst::Execute(Frame& frame)
{
    frame.Local(0).SetValue(frame.OpStack().Pop());
}

void StoreLocal0Inst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitStoreLocalInst(0);
}

StoreLocal1Inst::StoreLocal1Inst() : Instruction("storelocal.1")
{
}

void StoreLocal1Inst::Execute(Frame& frame)
{
    frame.Local(1).SetValue(frame.OpStack().Pop());
}

void StoreLocal1Inst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitStoreLocalInst(1);
}

StoreLocal2Inst::StoreLocal2Inst() : Instruction("storelocal.2")
{
}

void StoreLocal2Inst::Execute(Frame& frame)
{
    frame.Local(2).SetValue(frame.OpStack().Pop());
}

void StoreLocal2Inst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitStoreLocalInst(2);
}

StoreLocal3Inst::StoreLocal3Inst() : Instruction("storelocal.3")
{
}

void StoreLocal3Inst::Execute(Frame& frame)
{
    frame.Local(3).SetValue(frame.OpStack().Pop());
}

void StoreLocal3Inst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitStoreLocalInst(3);
}

StoreLocalBInst::StoreLocalBInst() : ByteParamInst("storelocal.b")
{
}

void StoreLocalBInst::Execute(Frame& frame)
{
    frame.Local(Index()).SetValue(frame.OpStack().Pop());
}

void StoreLocalBInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitStoreLocalInst(Index());
}

StoreLocalSInst::StoreLocalSInst() : UShortParamInst("storelocal.s")
{
}

void StoreLocalSInst::Execute(Frame& frame)
{
    frame.Local(Index()).SetValue(frame.OpStack().Pop());
}

void StoreLocalSInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitStoreLocalInst(Index());
}

LoadFieldInst::LoadFieldInst() : IndexParamInst("loadfield"), fieldType(ValueType::none)
{
}

void LoadFieldInst::Encode(Writer& writer)
{
    IndexParamInst::Encode(writer);
    writer.Put(uint8_t(fieldType));
}

Instruction* LoadFieldInst::Decode(Reader& reader)
{
    IndexParamInst::Decode(reader);
    fieldType = ValueType(reader.GetByte());
    return this;
}

void LoadFieldInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, Index());
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

void LoadFieldInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLoadFieldInst(Index(), fieldType);
}

LoadField0Inst::LoadField0Inst() : Instruction("loadfield.0"), fieldType(ValueType::none)
{
}

void LoadField0Inst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    writer.Put(uint8_t(fieldType));
}

Instruction* LoadField0Inst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    fieldType = ValueType(reader.GetByte());
    return this;
}

void LoadField0Inst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, 0);
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

void LoadField0Inst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLoadFieldInst(0, fieldType);
}

LoadField1Inst::LoadField1Inst() : Instruction("loadfield.1"), fieldType(ValueType::none)
{
}

void LoadField1Inst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    writer.Put(uint8_t(fieldType));
}

Instruction* LoadField1Inst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    fieldType = ValueType(reader.GetByte());
    return this;
}

void LoadField1Inst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, 1);
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

void LoadField1Inst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLoadFieldInst(1, fieldType);
}

LoadField2Inst::LoadField2Inst() : Instruction("loadfield.2"), fieldType(ValueType::none)
{
}

void LoadField2Inst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    writer.Put(uint8_t(fieldType));
}

Instruction* LoadField2Inst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    fieldType = ValueType(reader.GetByte());
    return this;
}

void LoadField2Inst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, 2);
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

void LoadField2Inst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLoadFieldInst(2, fieldType);
}

LoadField3Inst::LoadField3Inst() : Instruction("loadfield.3"), fieldType(ValueType::none)
{
}

void LoadField3Inst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    writer.Put(uint8_t(fieldType));
}

Instruction* LoadField3Inst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    fieldType = ValueType(reader.GetByte());
    return this;
}

void LoadField3Inst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, 3);
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

void LoadField3Inst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLoadFieldInst(3, fieldType);
}

LoadFieldBInst::LoadFieldBInst() : ByteParamInst("loadfield.b"), fieldType(ValueType::none)
{
}

void LoadFieldBInst::Encode(Writer& writer)
{
    ByteParamInst::Encode(writer);
    writer.Put(uint8_t(fieldType));
}

Instruction* LoadFieldBInst::Decode(Reader& reader)
{
    ByteParamInst::Decode(reader);
    fieldType = ValueType(reader.GetByte());
    return this;
}

void LoadFieldBInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, Index());
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

void LoadFieldBInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLoadFieldInst(Index(), fieldType);
}

LoadFieldSInst::LoadFieldSInst() : UShortParamInst("loadfield.s"), fieldType(ValueType::none)
{
}

void LoadFieldSInst::Encode(Writer& writer)
{
    UShortParamInst::Encode(writer);
    writer.Put(uint8_t(fieldType));
}

Instruction* LoadFieldSInst::Decode(Reader& reader)
{
    UShortParamInst::Decode(reader);
    fieldType = ValueType(reader.GetByte());
    return this;
}

void LoadFieldSInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, Index());
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

void LoadFieldSInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLoadFieldInst(Index(), fieldType);
}

StoreFieldInst::StoreFieldInst() : IndexParamInst("storefield"), fieldType(ValueType::none)
{
}

void StoreFieldInst::Encode(Writer& writer)
{
    IndexParamInst::Encode(writer);
    writer.Put(uint8_t(fieldType));
}

Instruction* StoreFieldInst::Decode(Reader& reader)
{
    IndexParamInst::Decode(reader);
    fieldType = ValueType(reader.GetByte());
    return this;
}

void StoreFieldInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = frame.OpStack().Pop();
        GetManagedMemoryPool().SetField(reference, Index(), fieldValue);
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

void StoreFieldInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitStoreFieldInst(Index(), fieldType);
}

StoreField0Inst::StoreField0Inst() : Instruction("storefield.0"), fieldType(ValueType::none)
{
}

void StoreField0Inst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    writer.Put(uint8_t(fieldType));
}

Instruction* StoreField0Inst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    fieldType = ValueType(reader.GetByte());
    return this;
}

void StoreField0Inst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = frame.OpStack().Pop();
        GetManagedMemoryPool().SetField(reference, 0, fieldValue);
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

void StoreField0Inst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitStoreFieldInst(0, fieldType);
}

StoreField1Inst::StoreField1Inst() : Instruction("storefield.1"), fieldType(ValueType::none)
{
}

void StoreField1Inst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    writer.Put(uint8_t(fieldType));
}

Instruction* StoreField1Inst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    fieldType = ValueType(reader.GetByte());
    return this;
}

void StoreField1Inst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = frame.OpStack().Pop();
        GetManagedMemoryPool().SetField(reference, 1, fieldValue);
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

void StoreField1Inst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitStoreFieldInst(1, fieldType);
}

StoreField2Inst::StoreField2Inst() : Instruction("storefield.2"), fieldType(ValueType::none)
{
}

void StoreField2Inst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    writer.Put(uint8_t(fieldType));
}

Instruction* StoreField2Inst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    fieldType = ValueType(reader.GetByte());
    return this;
}

void StoreField2Inst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = frame.OpStack().Pop();
        GetManagedMemoryPool().SetField(reference, 2, fieldValue);
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

void StoreField2Inst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitStoreFieldInst(2, fieldType);
}

StoreField3Inst::StoreField3Inst() : Instruction("storefield.3"), fieldType(ValueType::none)
{
}

void StoreField3Inst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    writer.Put(uint8_t(fieldType));
}

Instruction* StoreField3Inst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    fieldType = ValueType(reader.GetByte());
    return this;
}

void StoreField3Inst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = frame.OpStack().Pop();
        GetManagedMemoryPool().SetField(reference, 3, fieldValue);
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

void StoreField3Inst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitStoreFieldInst(3, fieldType);
}

StoreFieldBInst::StoreFieldBInst() : ByteParamInst("storefield.b"), fieldType(ValueType::none)
{
}

void StoreFieldBInst::Encode(Writer& writer)
{
    ByteParamInst::Encode(writer);
    writer.Put(uint8_t(fieldType));
}

Instruction* StoreFieldBInst::Decode(Reader& reader)
{
    ByteParamInst::Decode(reader);
    fieldType = ValueType(reader.GetByte());
    return this;
}

void StoreFieldBInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = frame.OpStack().Pop();
        GetManagedMemoryPool().SetField(reference, Index(), fieldValue);
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

void StoreFieldBInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitStoreFieldInst(Index(), fieldType);
}

StoreFieldSInst::StoreFieldSInst() : UShortParamInst("storefield.s"), fieldType(ValueType::none)
{
}

void StoreFieldSInst::Encode(Writer& writer)
{
    UShortParamInst::Encode(writer);
    writer.Put(uint8_t(fieldType));
}

Instruction* StoreFieldSInst::Decode(Reader& reader)
{
    UShortParamInst::Decode(reader);
    fieldType = ValueType(reader.GetByte());
    return this;
}

void StoreFieldSInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
        ObjectReference reference = ObjectReference(operand.Value());
        IntegralValue fieldValue = frame.OpStack().Pop();
        GetManagedMemoryPool().SetField(reference, Index(), fieldValue);
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

void StoreFieldSInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitStoreFieldInst(Index(), fieldType);
}

LoadElemInst::LoadElemInst() : Instruction("loadarrayelem"), elemType(ValueType::none)
{
}

void LoadElemInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    writer.Put(uint8_t(elemType));
}

Instruction* LoadElemInst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    elemType = ValueType(reader.GetByte());
    return this;
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
        frame.OpStack().Push(GetManagedMemoryPool().GetArrayElement(arrayReference, index.AsInt()));
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

void LoadElemInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLoadElemInst(*this);
}

StoreElemInst::StoreElemInst() : Instruction("storearrayelem"), elemType(ValueType::none)
{
}

void StoreElemInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    writer.Put(uint8_t(elemType));
}

Instruction* StoreElemInst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    elemType = ValueType(reader.GetByte());
    return this;
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
        GetManagedMemoryPool().SetArrayElement(arrayReference, index.AsInt(), elementValue);
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

void StoreElemInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitStoreElemInst(*this);
}

LoadConstantInst::LoadConstantInst() : IndexParamInst("loadconstant")
{
}

void LoadConstantInst::Execute(Frame& frame)
{
    ConstantId constantId(Index());
    frame.OpStack().Push(frame.GetConstantPool().GetConstant(constantId).Value());
}

void LoadConstantInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLoadConstantInst(Index());
}

LoadConstantBInst::LoadConstantBInst() : ByteParamInst("loadconstant.b")
{
}

void LoadConstantBInst::Execute(Frame& frame)
{
    ConstantId constantId(Index());
    frame.OpStack().Push(frame.GetConstantPool().GetConstant(constantId).Value());
}

void LoadConstantBInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLoadConstantInst(Index());
}

LoadConstantSInst::LoadConstantSInst() : UShortParamInst("loadconstant.s")
{
}

void LoadConstantSInst::Execute(Frame& frame)
{
    ConstantId constantId(Index());
    frame.OpStack().Push(frame.GetConstantPool().GetConstant(constantId).Value());
}

void LoadConstantSInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLoadConstantInst(Index());
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

void ReceiveInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitReceiveInst(*this);
}

ConversionBaseInst::ConversionBaseInst(const std::string& name_) : Instruction(name_)
{
}

void ConversionBaseInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitConversionBaseInst(*this);
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

void JumpInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitJumpInst(*this);
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

void JumpTrueInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitJumpTrueInst(*this);
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

void JumpFalseInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitJumpFalseInst(*this);
}

EnterBlockInst::EnterBlockInst() : Instruction("enterblock")
{
}

void EnterBlockInst::Execute(Frame& frame)
{
}

void EnterBlockInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitEnterBlockInst(*this);
}

ExitBlockInst::ExitBlockInst() : Instruction("exitblock")
{
}

void ExitBlockInst::Execute(Frame& frame)
{
}

void ExitBlockInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitExitBlockInst(*this);
}

ContinuousSwitchInst::ContinuousSwitchInst() : Instruction("cswitch"), condType(ValueType::none), begin(), end(), defaultTarget(-1)
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
    writer.Put(uint8_t(condType));
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
    condType = ValueType(reader.GetByte());
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

void ContinuousSwitchInst::Dump(CodeFormatter& formatter)
{
    Instruction::Dump(formatter);
    std::string targetsStr(" [");
    bool first = true;
    for (int32_t target : targets)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            targetsStr.append(", ");
        }
        if (target == endOfFunction)
        {
            targetsStr.append("eof");
        }
        else
        {
            targetsStr.append(std::to_string(target));
        }
    }
    targetsStr.append(" : ");
    if (defaultTarget == endOfFunction)
    {
        targetsStr.append("eof");
    }
    else
    {
        targetsStr.append(std::to_string(defaultTarget));
    }
    targetsStr.append("]");
    formatter.Write(targetsStr);
}

void ContinuousSwitchInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitContinuousSwitchInst(*this);
}

BinarySearchSwitchInst::BinarySearchSwitchInst() : Instruction("bswitch"), condType(ValueType::none), targets(), defaultTarget(-1)
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
    writer.Put(uint8_t(condType));
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
    condType = ValueType(reader.GetByte());
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

void BinarySearchSwitchInst::Dump(CodeFormatter& formatter)
{
    Instruction::Dump(formatter);
    std::string targetsStr(" [");
    bool first = true;
    for (auto target : targets)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            targetsStr.append(", ");
        }
        if (target.second == endOfFunction)
        {
            targetsStr.append("eof");
        }
        else
        {
            targetsStr.append(std::to_string(target.second));
        }
    }
    targetsStr.append(" : ");
    if (defaultTarget == endOfFunction)
    {
        targetsStr.append("eof");
    }
    else
    {
        targetsStr.append(std::to_string(defaultTarget));
    }
    targetsStr.append("]");
    formatter.Write(targetsStr);
}

void BinarySearchSwitchInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitBinarySearchSwitchInst(*this);
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

Function* CallInst::GetFunction() const
{
    Assert(function.Value().GetType() == ValueType::functionPtr, "function pointer expected");
    return function.Value().AsFunctionPtr();
}

void CallInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    Constant funNameConstant = function;
    if (function.Value().GetType() == ValueType::functionPtr)
    {
        funNameConstant = function.Value().AsFunctionPtr()->CallName();
    }
    ConstantId id = writer.GetConstantPool()->GetIdFor(funNameConstant);
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
    try
    {
        thread.GetStack().AllocateFrame(*fun);
    }
    catch (const StackOverflowException& ex)
    {
        ThrowStackOverflowException(ex, frame);
    }
}

void CallInst::Dump(CodeFormatter& formatter)
{
    Instruction::Dump(formatter);
    Function* fun = nullptr;
    if (function.Value().GetType() == ValueType::stringLiteral)
    {
        fun = FunctionTable::GetFunction(function.Value().AsStringLiteral());
    }
    else
    {
        Assert(function.Value().GetType() == ValueType::functionPtr, "function pointer expected");
        fun = function.Value().AsFunctionPtr();
    }
    formatter.Write(" " + ToUtf8(fun->CallName().Value().AsStringLiteral()));
}

void CallInst::DispatchTo(InstAdder& adder)
{
    adder.Add(this);
}

void CallInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitCallInst(*this);
}

bool CallInst::CreatesTemporaryObject(Function* function) const
{
    Function* fn = GetFunction();
    if (fn->ReturnsValue())
    {
        return fn->ReturnType() == ValueType::objectReference;
    }
    return false;
}

VirtualCallInst::VirtualCallInst() : Instruction("callv"), numArgs(0), vmtIndex(-1)
{
}

void VirtualCallInst::Clear()
{
    numArgs = 0;
    vmtIndex = -1;
}

void VirtualCallInst::SetFunctionType(const FunctionType& functionType_)
{
    functionType = functionType_;
}

void VirtualCallInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    Assert(numArgs != 0, "invalid number of arguments");
    writer.PutEncodedUInt(numArgs);
    Assert(vmtIndex != -1, "invalid vmt index");
    writer.PutEncodedUInt(vmtIndex);
    functionType.Write(writer);
}

Instruction* VirtualCallInst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    numArgs = reader.GetEncodedUInt();
    vmtIndex = reader.GetEncodedUInt();
    functionType.Read(reader);
    return this;
}

void VirtualCallInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue receiverValue = frame.OpStack().GetValue(numArgs);
        Assert(receiverValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference receiver(receiverValue.Value());
        IntegralValue classDataField = GetManagedMemoryPool().GetField(receiver, 0);
        Assert(classDataField.GetType() == ValueType::classDataPtr, "class data field expected");
        ClassData* classData = classDataField.AsClassDataPtr();
        if (classData)
        {
            Function* method = classData->Vmt().GetMethod(vmtIndex);
            if (method)
            {
                Thread& thread = frame.GetThread();
                try
                {
                    thread.GetStack().AllocateFrame(*method);
                }
                catch (const StackOverflowException& ex)
                {
                    ThrowStackOverflowException(ex, frame);
                }
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

bool VirtualCallInst::CreatesTemporaryObject(Function* function) const
{
    return functionType.ReturnType() == ValueType::objectReference;
}

void VirtualCallInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitVirtualCallInst(*this);
}

InterfaceCallInst::InterfaceCallInst() : Instruction("calli"), numArgs(0), imtIndex(-1)
{
}

void InterfaceCallInst::Clear()
{
    numArgs = 0;
    imtIndex = -1;
}

void InterfaceCallInst::SetFunctionType(const FunctionType& functionType_)
{
    functionType = functionType_;
}

void InterfaceCallInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    Assert(numArgs != 0, "invalid number of arguments");
    writer.PutEncodedUInt(numArgs);
    Assert(imtIndex != -1, "invalid imt index");
    writer.PutEncodedUInt(imtIndex);
    functionType.Write(writer);
}

Instruction* InterfaceCallInst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    numArgs = reader.GetEncodedUInt();
    imtIndex = reader.GetEncodedUInt();
    functionType.Read(reader);
    return this;
}

void InterfaceCallInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue interfaceObjectValue = frame.OpStack().GetValue(numArgs);
        Assert(interfaceObjectValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference interfaceObject(interfaceObjectValue.Value());
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        IntegralValue receiverField = memoryPool.GetField(interfaceObject, 0);
        Assert(receiverField.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference receiver(receiverField.Value());
        IntegralValue classDataField = memoryPool.GetField(receiver, 0);
        Assert(classDataField.GetType() == ValueType::classDataPtr, "class data field expected");
        ClassData* classData = classDataField.AsClassDataPtr();
        if (classData)
        {
            IntegralValue itabIndex = memoryPool.GetField(interfaceObject, 1);
            Assert(itabIndex.GetType() == ValueType::intType, "int expected");
            MethodTable& imt = classData->Imt(itabIndex.AsInt());
            Function* method = imt.GetMethod(imtIndex);
            if (method)
            {
                frame.OpStack().SetValue(numArgs, receiver);
                Thread& thread = frame.GetThread();
                try
                {
                    thread.GetStack().AllocateFrame(*method);
                }
                catch (const StackOverflowException& ex)
                {
                    ThrowStackOverflowException(ex, frame);
                }
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

bool InterfaceCallInst::CreatesTemporaryObject(Function* function) const
{
    return functionType.ReturnType() == ValueType::objectReference;
}

void InterfaceCallInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitInterfaceCallInst(*this);
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
        VmFunction* vmFunction = VmFunctionTable::GetVmFunction(StringPtr(vmFunctionName.Value().AsStringLiteral()));
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
    catch (const FileSystemError& ex)
    {
        ThrowFileSystemException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        ThrowSystemException(ex, frame);
    }
}

bool VmCallInst::CreatesTemporaryObject(Function* function) const 
{ 
    return function->ReturnsValue() && function->ReturnType() == ValueType::objectReference; 
}

void VmCallInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitVmCallInst(*this);
}

DelegateCallInst::DelegateCallInst() : Instruction("calld")
{
}

void DelegateCallInst::SetFunctionType(const FunctionType& functionType_)
{
    functionType = functionType_;
}

void DelegateCallInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    functionType.Write(writer);
}

Instruction* DelegateCallInst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    functionType.Read(reader);
    return this;
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
        try
        {
            thread.GetStack().AllocateFrame(*fun);
        }
        catch (const StackOverflowException& ex)
        {
            ThrowStackOverflowException(ex, frame);
        }
    }
}

void DelegateCallInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitDelegateCallInst(*this);
}

bool DelegateCallInst::CreatesTemporaryObject(Function* function) const
{
    return functionType.ReturnType() == ValueType::objectReference;
}

ClassDelegateCallInst::ClassDelegateCallInst() : Instruction("callcd")
{
}

void ClassDelegateCallInst::SetFunctionType(const FunctionType& functionType_)
{
    functionType = functionType_;
}

void ClassDelegateCallInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    functionType.Write(writer);
}

Instruction* ClassDelegateCallInst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    functionType.Read(reader);
    return this;
}

void ClassDelegateCallInst::Execute(Frame& frame)
{
    IntegralValue classDlgValue = frame.OpStack().Pop();
    Assert(classDlgValue.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference classDelegateRef(classDlgValue.Value());
    ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
    std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
    IntegralValue classObjectValue = memoryPool.GetField(classDelegateRef, 1, lock);
    if (classObjectValue.Value() == 0)
    {
        NullReferenceException ex("value of class delegate receiver is null");
        ThrowNullReferenceException(ex, frame);
    }
    Assert(classObjectValue.GetType() == ValueType::objectReference, "object reference expected");
    IntegralValue funValue = memoryPool.GetField(classDelegateRef, 2, lock);
    Assert(funValue.GetType() == ValueType::functionPtr, "function pointer expected");
    Function* fun = funValue.AsFunctionPtr();
    if (!fun)
    {
        NullReferenceException ex("value of class delegate method is null");
        ThrowNullReferenceException(ex, frame);
    }
    else
    {
        frame.OpStack().Insert(fun->NumParameters() - 1, classObjectValue);
        Thread& thread = frame.GetThread();
        try
        {
            thread.GetStack().AllocateFrame(*fun);
        }
        catch (const StackOverflowException& ex)
        {
            ThrowStackOverflowException(ex, frame);
        }
    }
}

bool ClassDelegateCallInst::CreatesTemporaryObject(Function* function) const
{
    return functionType.ReturnType() == ValueType::objectReference;
}

void ClassDelegateCallInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitClassDelegateCallInst(*this);
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
    Constant classNameConstant = classData;
    if (classData.Value().GetType() == ValueType::classDataPtr)
    {
        classNameConstant = classData.Value().AsClassDataPtr()->Type()->NameConstant();
    }
    ConstantId id = writer.GetConstantPool()->GetIdFor(classNameConstant);
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
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        IntegralValue classDataFieldValue = memoryPool.GetField(objectReference, 0, lock);
        Assert(classDataFieldValue.GetType() == ValueType::classDataPtr, "class data pointer expected");
        if (!classDataFieldValue.AsClassDataPtr())
        {
            Assert(classData.Value().GetType() == ValueType::classDataPtr, "class data pointer expected");
            ClassData* cd = classData.Value().AsClassDataPtr();
            classDataFieldValue = IntegralValue(cd);
            memoryPool.SetField(objectReference, 0, classDataFieldValue, lock);
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
    ClassData* cd = nullptr;
    if (classData.Value().GetType() == ValueType::stringLiteral)
    {
        cd = ClassDataTable::GetClassData(classData.Value().AsStringLiteral());
    }
    else
    {
        Assert(classData.Value().GetType() == ValueType::classDataPtr, "class data pointer expected");
        cd = classData.Value().AsClassDataPtr();
    }
    formatter.Write(" " + ToUtf8(cd->Type()->Name().Value()) + " " + std::to_string(cd->Type()->Id()));
}

void SetClassDataInst::DispatchTo(InstAdder& adder)
{
    adder.Add(this);
}

void SetClassDataInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitSetClassDataInst(*this);
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
    Constant typeNameConstant = type;
    if (type.Value().GetType() == ValueType::typePtr)
    {
        typeNameConstant = type.Value().AsTypePtr()->NameConstant();
    }
    ConstantId id = writer.GetConstantPool()->GetIdFor(typeNameConstant);
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
    ObjectReference objectReference = GetManagedMemoryPool().CreateObject(frame.GetThread(), objectType);
    frame.OpStack().Push(objectReference);
}

void CreateObjectInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitCreateObjectInst(*this);
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
        ObjectReference copy = GetManagedMemoryPool().CopyObject(frame.GetThread(), objectReference);
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

void CopyObjectInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitCopyObjectInst(*this);
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
        ClassData* classData = ClassDataTable::GetSystemStringClassData();
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        ObjectReference objectReference = memoryPool.CreateObject(frame.GetThread(), classData->Type(), lock);
        AllocationHandle charsHandle = memoryPool.CreateStringCharsFromLiteral(frame.GetThread(), strLit, len, lock);
        memoryPool.SetField(objectReference, 0, IntegralValue(classData), lock);
        memoryPool.SetField(objectReference, 1, IntegralValue(static_cast<uint64_t>(static_cast<int32_t>(len)), ValueType::intType), lock);
        memoryPool.SetField(objectReference, 2, charsHandle, lock);
        frame.OpStack().Push(objectReference);
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

void StrLitToStringInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitStrLitToStringInst(*this);
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
        frame.OpStack().Push(GetManagedMemoryPool().GetStringChar(strReference, index.AsInt()));
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

void LoadStringCharInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLoadStringCharInst(*this);
}

DupInst::DupInst() : Instruction("dup")
{
}

void DupInst::Execute(Frame& frame)
{
    frame.OpStack().Dup();
}

void DupInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitDupInst(*this);
}

SwapInst::SwapInst() : Instruction("swap")
{
}

void SwapInst::Execute(Frame& frame)
{
    frame.OpStack().Swap();
}

void SwapInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitSwapInst(*this);
}

RotateInst::RotateInst() : Instruction("rotate")
{
}

void RotateInst::Execute(Frame& frame)
{
    frame.OpStack().Rotate();
}

void RotateInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitRotateInst(*this);
}

PopInst::PopInst() : Instruction("pop")
{
}

void PopInst::Execute(Frame& frame)
{
    frame.OpStack().Pop();
}

void PopInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitPopInst(*this);
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
            IntegralValue classDataField = GetManagedMemoryPool().GetField(objectReference, 0);
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
            ObjectReference casted = objectReference;
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

void DownCastInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitDownCastInst(*this);
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
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        utf32_string stackTraceStr = frame.GetThread().GetStackTrace();
        ObjectReference stackTrace = memoryPool.CreateString(frame.GetThread(), stackTraceStr, lock);
        memoryPool.SetField(exception, 2, stackTrace, lock);
        frame.GetThread().HandleException(exception);
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

void ThrowInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitThrowInst(*this);
}

RethrowInst::RethrowInst() : Instruction("rethrow")
{
}

void RethrowInst::Execute(Frame& frame)
{
    frame.GetThread().RethrowCurrentException();
}

void RethrowInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitRethrowInst(*this);
}

BeginTryInst::BeginTryInst() : IndexParamInst("begintry")
{
}

void BeginTryInst::Execute(Frame& frame)
{
    frame.GetThread().BeginTry();
}

void BeginTryInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitBeginTryInst(*this);
}

EndTryInst::EndTryInst() : IndexParamInst("endtry")
{
}

void EndTryInst::Execute(Frame& frame)
{
    frame.GetThread().EndTry();
}

void EndTryInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitEndTryInst(*this);
}

BeginCatchSectionInst::BeginCatchSectionInst() : IndexParamInst("begincatchsection")
{
}

void BeginCatchSectionInst::Execute(Frame& frame)
{
}

void BeginCatchSectionInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitBeginCatchSectionInst(*this);
}

EndCatchSectionInst::EndCatchSectionInst() : IndexParamInst("endcatchsection")
{
}

void EndCatchSectionInst::Execute(Frame& frame)
{
}

void EndCatchSectionInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitEndCatchSectionInst(*this);
}

BeginCatchInst::BeginCatchInst() : IndexParamInst("begincatch")
{
}

void BeginCatchInst::Execute(Frame& frame)
{
}

void BeginCatchInst::Accept(MachineFunctionVisitor& visitor) 
{
    visitor.VisitBeginCatchInst(*this);
}

EndCatchInst::EndCatchInst() : IndexParamInst("endcatch")
{
}

void EndCatchInst::Execute(Frame& frame)
{
    frame.GetThread().EndCatch();
}

void EndCatchInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitEndCatchInst(*this);
}

BeginFinallyInst::BeginFinallyInst() : IndexParamInst("beginfinally")
{
}

void BeginFinallyInst::Execute(Frame& frame)
{
}

void BeginFinallyInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitBeginFinallyInst(*this);
}

EndFinallyInst::EndFinallyInst() : IndexParamInst("endfinally")
{
}

void EndFinallyInst::Execute(Frame& frame)
{
    frame.GetThread().EndFinally();
}

void EndFinallyInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitEndFinallyInst(*this);
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

void NextInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitNextInst(*this);
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
        ClassData* classData = ClassDataTable::GetClassData(objectType->Name());
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
                Function* staticConstructor = FunctionTable::GetFunction(staticConstructorName);
                Thread& thread = frame.GetThread();
                try
                {
                    thread.GetStack().AllocateFrame(*staticConstructor);
                }
                catch (const StackOverflowException& ex)
                {
                    ThrowStackOverflowException(ex, frame);
                }
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

void StaticInitInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitStaticInitInst(*this);
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
        ClassData* classData = ClassDataTable::GetClassData(objectType->Name());
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

void DoneStaticInitInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitDoneStaticInitInst(*this);
}

LoadStaticFieldInst::LoadStaticFieldInst() : TypeInstruction("loadstaticfield"), index(-1), fieldType(ValueType::none)
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
    writer.Put(uint8_t(fieldType));
}

Instruction* LoadStaticFieldInst::Decode(Reader& reader)
{
    TypeInstruction::Decode(reader);
    index = reader.GetInt();
    fieldType = ValueType(reader.GetByte());
    return this;
}

void LoadStaticFieldInst::Execute(Frame& frame)
{
    try
    {
        Type* type = GetType();
        ObjectType* objectType = dynamic_cast<ObjectType*>(type);
        Assert(objectType, "object type expected");
        ClassData* classData = ClassDataTable::GetClassData(objectType->Name());
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

void LoadStaticFieldInst::Dump(CodeFormatter& formatter)
{
    TypeInstruction::Dump(formatter);
    formatter.Write(" " + std::to_string(index));
}

void LoadStaticFieldInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLoadStaticFieldInst(*this);
}

StoreStaticFieldInst::StoreStaticFieldInst() : TypeInstruction("storestaticfield"), index(-1), fieldType(ValueType::none)
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
    writer.Put(uint8_t(fieldType));
}

Instruction* StoreStaticFieldInst::Decode(Reader& reader)
{
    TypeInstruction::Decode(reader);
    index = reader.GetInt();
    fieldType = ValueType(reader.GetByte());
    return this;
}

void StoreStaticFieldInst::Dump(CodeFormatter& formatter)
{
    TypeInstruction::Dump(formatter);
    formatter.Write(" " + std::to_string(index));
}

void StoreStaticFieldInst::Execute(Frame& frame)
{
    try
    {
        IntegralValue fieldValue = frame.OpStack().Pop();
        Type* type = GetType();
        ObjectType* objectType = dynamic_cast<ObjectType*>(type);
        Assert(objectType, "object type expected");
        ClassData* classData = ClassDataTable::GetClassData(objectType->Name());
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

void StoreStaticFieldInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitStoreStaticFieldInst(*this);
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
    frame.OpStack().Push(IntegralValue(static_cast<uint64_t>(result), ValueType::boolType));
}

void EqualObjectNullInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitEqualObjectNullInst(*this);
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
    frame.OpStack().Push(IntegralValue(static_cast<uint64_t>(result), ValueType::boolType));
}

void EqualNullObjectInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitEqualNullObjectInst(*this);
}

EqualDlgNullInst::EqualDlgNullInst() : Instruction("equaldlgnull")
{
}

void EqualDlgNullInst::Execute(Frame& frame)
{
    IntegralValue rightValue = frame.OpStack().Pop();
    IntegralValue leftValue = frame.OpStack().Pop();
    Assert(leftValue.GetType() == ValueType::functionPtr, "function pointer expected");
    bool result = leftValue.AsFunctionPtr() == nullptr;
    frame.OpStack().Push(IntegralValue(static_cast<uint64_t>(result), ValueType::boolType));
}

void EqualDlgNullInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitEqualDlgNullInst(*this);
}

EqualNullDlgInst::EqualNullDlgInst() : Instruction("equalnulldlg")
{
}

void EqualNullDlgInst::Execute(Frame& frame)
{
    IntegralValue rightValue = frame.OpStack().Pop();
    IntegralValue leftValue = frame.OpStack().Pop();
    Assert(rightValue.GetType() == ValueType::functionPtr, "function pointer expected");
    bool result = rightValue.AsFunctionPtr() == nullptr;
    frame.OpStack().Push(IntegralValue(static_cast<uint64_t>(result), ValueType::boolType));
}

void EqualNullDlgInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitEqualNullDlgInst(*this);
}

BoxBaseInst::BoxBaseInst(const std::string& name_) : Instruction(name_)
{
}

void BoxBaseInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitBoxBaseInst(*this);
}

UnboxBaseInst::UnboxBaseInst(const std::string& name_) : Instruction(name_)
{
}

void UnboxBaseInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitUnboxBaseInst(*this);
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
        GetManagedMemoryPool().AllocateArrayElements(frame.GetThread(), arr, elementType, length.AsInt());
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

void AllocateArrayElementsInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitAllocateArrayElementsInst(*this);
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
            frame.OpStack().Push(IntegralValue(static_cast<uint64_t>(false), ValueType::boolType));
        }
        else
        {
            IntegralValue classDataField = GetManagedMemoryPool().GetField(ob, 0);
            Assert(classDataField.GetType() == ValueType::classDataPtr, "class data pointer expected");
            ClassData* classData = classDataField.AsClassDataPtr();
            uint64_t sourceTypeId = classData->Type()->Id();
            Type* type = GetType();
            ObjectType* objectType = dynamic_cast<ObjectType*>(type);
            Assert(objectType, "object type expected");
            uint64_t targetId = objectType->Id();
            bool result = sourceTypeId % targetId == 0;
            frame.OpStack().Push(IntegralValue(static_cast<uint64_t>(result), ValueType::boolType));
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

void IsInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitIsInst(*this);
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
            IntegralValue classDataField = GetManagedMemoryPool().GetField(ob, 0);
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

void AsInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitAsInst(*this);
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

Function* Fun2DlgInst::GetFunction()
{
    Assert(function.Value().GetType() == ValueType::functionPtr, "function pointer expected");
    return function.Value().AsFunctionPtr();
}

void Fun2DlgInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    Constant functionNameConstant = function;
    if (function.Value().GetType() == ValueType::functionPtr)
    {
        functionNameConstant = function.Value().AsFunctionPtr()->CallName();
    }
    ConstantId id = writer.GetConstantPool()->GetIdFor(functionNameConstant);
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
    Function* fun = nullptr;
    if (function.Value().GetType() == ValueType::stringLiteral)
    {
        fun = FunctionTable::GetFunction(function.Value().AsStringLiteral());
    }
    else
    {
        Assert(function.Value().GetType() == ValueType::functionPtr, "function pointer expected");
        fun = function.Value().AsFunctionPtr();
    }
    formatter.Write(" " + ToUtf8(fun->CallName().Value().AsStringLiteral()));
}

void Fun2DlgInst::DispatchTo(InstAdder& adder)
{
    adder.Add(this);
}

void Fun2DlgInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitFun2DlgInst(*this);
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

Function* MemFun2ClassDlgInst::GetFunction() const
{
    Assert(function.Value().GetType() == ValueType::functionPtr, "function pointer expected");
    return function.Value().AsFunctionPtr();
}

void MemFun2ClassDlgInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    Constant functionNameConstant = function;
    if (function.Value().GetType() == ValueType::functionPtr)
    {
        functionNameConstant = function.Value().AsFunctionPtr()->CallName();
    }
    ConstantId id = writer.GetConstantPool()->GetIdFor(functionNameConstant);
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
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        memoryPool.SetField(classDelegateObjectRef, 1, classObjectValue, lock);
        memoryPool.SetField(classDelegateObjectRef, 2, function.Value(), lock);
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
    Function* fun = nullptr;
    if (function.Value().GetType() == ValueType::stringLiteral)
    {
        fun = FunctionTable::GetFunction(function.Value().AsStringLiteral());
    }
    else
    {
        Assert(function.Value().GetType() == ValueType::functionPtr, "function pointer expected");
        fun = function.Value().AsFunctionPtr();
    }
    formatter.Write(" " + ToUtf8(fun->CallName().Value().AsStringLiteral()));
}

void MemFun2ClassDlgInst::DispatchTo(InstAdder& adder)
{
    adder.Add(this);
}

void MemFun2ClassDlgInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitMemFun2ClassDlgInst(*this);
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
    frame.OpStack().Push(IntegralValue(static_cast<uint64_t>(variableReferenceId), ValueType::variableReference));
}

void CreateLocalVariableReferenceInst::Dump(CodeFormatter& formatter)
{
    Instruction::Dump(formatter);
    formatter.Write(" " + std::to_string(localIndex));
}

void CreateLocalVariableReferenceInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitCreateLocalVariableReferenceInst(*this);
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
    frame.OpStack().Push(IntegralValue(static_cast<uint64_t>(variableReferenceId), ValueType::variableReference));
}

void CreateMemberVariableReferenceInst::Dump(CodeFormatter& formatter)
{
    Instruction::Dump(formatter);
    formatter.Write(" " + std::to_string(memberVarIndex));
}

void CreateMemberVariableReferenceInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitCreateMemberVariableReferenceInst(*this);
}

LoadVariableReferenceInst::LoadVariableReferenceInst() : IndexParamInst("loadref"), type(ValueType::none)
{
}

void LoadVariableReferenceInst::Encode(Writer& writer)
{
    IndexParamInst::Encode(writer);
    writer.Put(uint8_t(type));
}

Instruction* LoadVariableReferenceInst::Decode(Reader& reader)
{
    IndexParamInst::Decode(reader);
    type = ValueType(reader.GetByte());
    return this;
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

void LoadVariableReferenceInst::Handle(Frame& frame, MemberVariableReference* memberVariableReference)
{
    ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
    std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
    IntegralValue value = memoryPool.GetField(memberVariableReference->GetObjectReference(), memberVariableReference->MemberVarIndex());
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

void LoadVariableReferenceInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitLoadVariableReferenceInst(*this);
}

StoreVariableReferenceInst::StoreVariableReferenceInst() : IndexParamInst("storeref"), type(ValueType::none)
{
}

void StoreVariableReferenceInst::Encode(Writer& writer)
{
    IndexParamInst::Encode(writer);
    writer.Put(uint8_t(type));
}

Instruction* StoreVariableReferenceInst::Decode(Reader& reader)
{
    IndexParamInst::Decode(reader);
    type = ValueType(reader.GetByte());
    return this;
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
    ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
    std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
    IntegralValue oldValue = memoryPool.GetField(memberVariableReference->GetObjectReference(), memberVariableReference->MemberVarIndex(), lock);
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
        memoryPool.SetField(memberVariableReference->GetObjectReference(), memberVariableReference->MemberVarIndex(), value, lock);
    }
}

void StoreVariableReferenceInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitStoreVariableReferenceInst(*this);
}

GcPollInst::GcPollInst() : Instruction("gcpoll")
{
}

void GcPollInst::Execute(Frame& frame)
{
    frame.GetThread().PollGc();
}

void GcPollInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitGcPollInst(*this);
}

RequestGcInst::RequestGcInst() : Instruction("requestgc")
{
}

void RequestGcInst::Execute(Frame& frame)
{
    Thread& thread = frame.GetThread();
    thread.RequestGc(false);
    thread.WaitUntilGarbageCollected();
}

void RequestGcInst::Accept(MachineFunctionVisitor& visitor)
{
    visitor.VisitRequestGcInst(*this);
}

void ThrowException(const std::string& message, Frame& frame, const utf32_string& exceptionTypeName)
{
    Type* type = TypeTable::GetType(StringPtr(exceptionTypeName.c_str()));
    ObjectType* objectType = dynamic_cast<ObjectType*>(type);
    Assert(objectType, "object type expected");
    ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
    std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
    ObjectReference objectReference = memoryPool.CreateObject(frame.GetThread(), objectType, lock);
    ClassData* classData = ClassDataTable::GetClassData(StringPtr(exceptionTypeName.c_str()));
    void* object = memoryPool.GetObject(objectReference, lock);
    ManagedAllocationHeader* header = GetAllocationHeader(object);
    header->Reference();
    IntegralValue classDataValue(classData);
    memoryPool.SetField(objectReference, 0, classDataValue, lock);
    ObjectReference messageStr = memoryPool.CreateString(frame.GetThread(), ToUtf32(message), lock);
    memoryPool.SetField(objectReference, 1, messageStr, lock);
    frame.OpStack().Push(objectReference);
    ThrowInst throwInst;
    throwInst.Execute(frame);
}

MACHINE_API void ThrowSystemException(const SystemException& ex, Frame& frame)
{
    ThrowException(ex.Message(), frame, U"System.SystemException");
}

MACHINE_API void ThrowNullReferenceException(const NullReferenceException& ex, Frame& frame)
{
    ThrowException(ex.Message(), frame, U"System.NullReferenceException");
}

MACHINE_API void ThrowIndexOutOfRangeException(const IndexOutOfRangeException& ex, Frame& frame)
{
    ThrowException(ex.Message(), frame, U"System.IndexOutOfRangeException");
}

MACHINE_API void ThrowArgumentOutOfRangeException(const ArgumentOutOfRangeException& ex, Frame& frame)
{
    ThrowException(ex.Message(), frame, U"System.ArgumentOutOfRangeException");
}

MACHINE_API void ThrowInvalidCastException(const InvalidCastException& ex, Frame& frame)
{
    ThrowException(ex.Message(), frame, U"System.InvalidCastException");
}

MACHINE_API void ThrowFileSystemException(const FileSystemError& ex, Frame& frame)
{
    ThrowException(ex.Message(), frame, U"System.FileSystemException");
}

MACHINE_API void ThrowStackOverflowException(const StackOverflowException& ex, Frame& frame)
{
    ThrowException(ex.Message(), frame, U"System.StackOverflowException");
}

MACHINE_API void ThrowThreadingException(const ThreadingException& ex, Frame& frame)
{
    ThrowException(ex.Message(), frame, U"System.Threading.ThreadingException");
}

} } // namespace cminor::machine
