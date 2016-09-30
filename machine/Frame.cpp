// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Frame.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Function.hpp>

namespace cminor { namespace machine {

Frame::Frame(Machine& machine_, Thread& thread_, Function& fun_, ConstantPool& constantPool_) :
    machine(machine_), thread(thread_), fun(fun_), objectPool(machine.GetObjectPool()), constantPool(constantPool_), locals(fun.NumLocals()), pc(0)
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

} } // namespace cminor::machine
