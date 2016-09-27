// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Instruction.hpp>
#include <cminor/machine/Error.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Util.hpp>
#include <cminor/machine/String.hpp>

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

void Instruction::Execute(Frame& frame)
{
}

void Instruction::Dump(CodeFormatter& formatter)
{
    formatter.Write(ToHexString(opCode) + " " + name);
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

LogicalNotInst::LogicalNotInst() : Instruction("not", "", "")
{
}

void LogicalNotInst::Execute(Frame& frame)
{
    bool operand = static_cast<bool>(frame.OpStack().Pop());
    frame.OpStack().Push(static_cast<uint64_t>(!operand));
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

StringEqualInst::StringEqualInst() : Instruction("equalst", "equal", "string")
{
}

void StringEqualInst::Execute(Frame& frame) 
{
    ObjectHandle right = static_cast<ObjectHandle>(frame.OpStack().Pop());
    ObjectHandle left = static_cast<ObjectHandle>(frame.OpStack().Pop());
    ObjectPtr leftPtr = frame.GetObjectPool().GetObjectPtr(left);
    ObjectPtr rightPtr = frame.GetObjectPool().GetObjectPtr(right);
    char32_t* leftChars = static_cast<char32_t*>(leftPtr.Value());
    char32_t* rightChars = static_cast<char32_t*>(rightPtr.Value());
    bool result = StringPtr(leftChars) == StringPtr(rightChars);
    frame.OpStack().Push(static_cast<uint64_t>(result));
}

StringLessInst::StringLessInst() : Instruction("lessst", "less", "string")
{
}

void StringLessInst::Execute(Frame& frame)
{
    ObjectHandle right = static_cast<ObjectHandle>(frame.OpStack().Pop());
    ObjectHandle left = static_cast<ObjectHandle>(frame.OpStack().Pop());
    ObjectPtr leftPtr = frame.GetObjectPool().GetObjectPtr(left);
    ObjectPtr rightPtr = frame.GetObjectPool().GetObjectPtr(right);
    char32_t* leftChars = static_cast<char32_t*>(leftPtr.Value());
    char32_t* rightChars = static_cast<char32_t*>(rightPtr.Value());
    bool result = StringPtr(leftChars) < StringPtr(rightChars);
    frame.OpStack().Push(static_cast<uint64_t>(result));
}

} } // namespace cminor::machine
