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

LogicalNotInst::LogicalNotInst() : Instruction("not", "not", "bool")
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
    IntegralValue fieldValue = frame.GetObjectPool().GetField(reference, Index());
    frame.OpStack().Push(fieldValue);
}

StoreFieldInst::StoreFieldInst() : IndexParamInst("storefield")
{
}

void StoreFieldInst::Execute(Frame& frame)
{
    IntegralValue fieldValue = frame.OpStack().Pop();
    IntegralValue operand = frame.OpStack().Pop();
    Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
    ObjectReference reference = ObjectReference(operand.Value());
    frame.GetObjectPool().SetField(reference, Index(), fieldValue);
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
}

void JumpInst::Execute(Frame& frame)
{
    frame.SetPC(Index());
}

JumpTrueInst::JumpTrueInst() : IndexParamInst("jumptrue")
{
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

JumpFalseInst::JumpFalseInst() : IndexParamInst("jumpfalse")
{
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

CallInst::CallInst() : Instruction("call")
{
}

void CallInst::SetFunctionFullName(Constant functionFullName)
{
    Assert(functionFullName.Value().GetType() == ValueType::stringLiteral, "string literal expected");
    function = functionFullName;
}

StringPtr CallInst::GetFunctionFullName() const
{
    Assert(function.Value().GetType() == ValueType::stringLiteral, "string literal expected");
    return StringPtr(function.Value().AsStringLiteral());
}

void CallInst::SetFunction(Function* fun)
{
    function = Constant(IntegralValue(fun));
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
    reader.AddCallInst(this);
    return this;
}

void CallInst::Execute(Frame& frame)
{
    Assert(function.Value().GetType() == ValueType::functionPtr, "function pointer expected");
    Function* fun = function.Value().AsFunctionPtr();
    frame.GetThread().Frames().push_back(Frame(frame.GetMachine(), frame.GetThread(), *fun));
}

} } // namespace cminor::machine
