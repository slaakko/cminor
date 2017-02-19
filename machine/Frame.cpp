// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Frame.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/util/Util.hpp>

namespace cminor { namespace machine {

bool debugging = false;

inline bool Debugging()
{
    return debugging;
}

MACHINE_API void SetDebugging()
{
    debugging = true;
}

Frame::Frame(uint64_t size_, Thread& thread_, Function& fun_) : size(size_), thread(thread_), fun(fun_), id(GetMachine().GetNextFrameId()), pc(0), prevPC(0)
{
    if (Debugging())
    {
        debugContextId = thread.AllocateDebugContext();
    }
    else
    {
        debugContextId = -1;
    }
    uint8_t* end = reinterpret_cast<uint8_t*>(this) + Align(sizeof(*this), 8);
    locals = reinterpret_cast<LocalVariable*>(end);
}

Frame::~Frame()
{
    for (VariableReference* variableReference : variableReferences)
    {
        thread.RemoveVariableReference(variableReference->Id());
        delete variableReference;
    }
    if (debugContextId != -1)
    {
        thread.FreeDebugContext();
    }
}

void Frame::AddVariableReference(VariableReference* variableReference)
{
    variableReferences.push_back(variableReference);
    thread.AddVariableReference(variableReference);
}

ConstantPool& Frame::GetConstantPool() 
{ 
    return fun.GetConstantPool(); 
}

OperandStack& Frame::OpStack() 
{ 
    return thread.OpStack();
}

const OperandStack& Frame::OpStack() const
{ 
    return thread.OpStack();
}

int Frame::NumLocals() const
{ 
    return fun.NumLocals(); 
}

Instruction* Frame::GetNextInst()
{
    prevPC = pc;
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

bool Frame::HasBreakpointAt(int32_t pc) const
{
    if (Debugging())
    {
        DebugContext* debugContext = thread.GetDebugContext(debugContextId);
        return debugContext->HasBreakpointAt(pc);
    }
    else
    {
        return false;
    }
}

void Frame::SetBreakpointAt(int32_t pc)
{
    if (Debugging())
    {
        DebugContext* debugContext = thread.GetDebugContext(debugContextId);
        debugContext->SetBreakpointAt(pc);
    }
}

void Frame::RemoveBreakpointAt(int32_t pc)
{
    if (Debugging())
    {
        DebugContext* debugContext = thread.GetDebugContext(debugContextId);
        debugContext->RemoveBreakpointAt(pc);
    }
}

} } // namespace cminor::machine
