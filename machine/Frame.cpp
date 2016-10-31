// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Frame.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Function.hpp>

namespace cminor { namespace machine {

Frame::Frame(Machine& machine_, Thread& thread_, Function& fun_) :
    machine(machine_), id(machine.GetNextFrameId()), thread(thread_), fun(fun_), managedMemoryPool(machine.GetManagedMemoryPool()), constantPool(fun.GetConstantPool()), opStack(thread.OpStack()), 
    locals(fun.NumLocals()), pc(0)
{
}

Instruction* Frame::GetNextInst()
{
    if (pc == fun.NumInsts())
    {
        return nullptr;
    }
    Instruction* inst = fun.GetInst(pc);
    ++pc;
    return inst;
}

void Frame::SetPC(int32_t pc_)
{ 
    if (pc_ == endOfFunction)
    {
        pc = fun.NumInsts();
    }
    else
    {
        Assert(pc_ >= 0 && pc_ <= fun.NumInsts(), "invalid instruction index");
        pc = pc_;
    }
}

} } // namespace cminor::machine
