// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Instruction.hpp>
#include <cminor/machine/Error.hpp>
#include <cminor/machine/Machine.hpp>

namespace cminor { namespace machine {

Instruction::Instruction(const std::string& name_) : opCode(255), name(name_)
{
}

Instruction::~Instruction()
{
}

void Instruction::Execute(Frame& frame)
{
}

InvalidInst::InvalidInst() : Instruction("<invalid_instruction>")
{
}

void InvalidInst::Execute(Frame& frame)
{
    throw std::runtime_error("invalid instruction " + std::to_string(OpCode()));
}

GroupInst::GroupInst(Machine& machine_, const std::string& name_) : Instruction(name_), machine(machine_)
{
    for (int i = 0; i < 256; ++i)
    {
        SetInst(i, new InvalidInst());
    }
}

void GroupInst::SetInst(uint8_t opCode, Instruction* inst)
{
    inst->SetOpCode(opCode);
    childInsts[opCode].reset(inst);
    if (dynamic_cast<InvalidInst*>(inst) == nullptr)
    {
        machine.AddInst(inst);
    }
}

LogicalNotInst::LogicalNotInst() : Instruction("not")
{
}

void LogicalNotInst::Execute(Frame& frame)
{
    bool operand = static_cast<bool>(frame.OpStack().Pop());
    frame.OpStack().Push(static_cast<uint64_t>(!operand));
}

} } // namespace cminor::machine
