// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Thread.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Function.hpp>

namespace cminor { namespace machine {

Thread::Thread(Machine& machine_, Function& fun_) : 
    machine(machine_), fun(fun_), instructionCount(0), checkWantToCollectGarbageCount(100), paused(), sleeping(), pausedCond(), handlingException(false), currentExceptionBlock(nullptr), 
    exceptionObjectType(nullptr)
{
    frames.push_back(Frame(machine, *this, fun));
}

inline void Thread::CheckPause()
{
    if (CheckWantToCollectGarbage())
    {
        if (GetMachine().GetGarbageCollector().WantToCollectGarbage())
        {
            PauseUntilGarbageCollected();
        }
    }
}

void Thread::RunToEnd()
{
    Assert(!frames.empty(), "thread got no frame");
    while (true)
    {
        CheckPause();
        Frame* frame = &frames.back();
        Instruction* inst = frame->GetNextInst();
        while (!inst)
        {
            Assert(!frames.empty(), "thread got no frame");
            frames.pop_back();
            if (!frames.empty())
            {
                frame = &frames.back();
                inst = frame->GetNextInst();
            }
            else
            {
                return;
            }
        }
        inst->Execute(*frame);
        IncInstructionCount();
    }
}

void Thread::Run()
{
    RunToEnd();
    std::lock_guard<std::mutex> lock(mtx);
    paused.store(true);
    pausedCond.notify_one();
}

void Thread::RunDebug()
{
    Assert(!frames.empty(), "thread got no frame");
    while (true)
    {
        CheckPause();
        Frame* frame = &frames.back();
        std::pair<int32_t, int32_t> point = std::make_pair(frame->Id(), frame->PC());
        if (breakPoints.find(point) != breakPoints.cend())
        {
            return;
        }
        Instruction* inst = frame->GetNextInst();
        while (!inst)
        {
            Assert(!frames.empty(), "thread got no frame");
            frames.pop_back();
            if (!frames.empty())
            {
                frame = &frames.back();
                std::pair<int32_t, int32_t> point = std::make_pair(frame->Id(), frame->PC());
                if (breakPoints.find(point) != breakPoints.cend())
                {
                    return;
                }
                inst = frame->GetNextInst();
            }
            else
            {
                return;
            }
        }
        inst->Execute(*frame);
        IncInstructionCount();
    }
}

void Thread::Step()
{
    Frame* frame = &frames.back();
    Instruction* inst = frame->GetNextInst();
    while (!inst)
    {
        if (frames.empty())
        {
            return;
        }
        frames.pop_back();
        if (!frames.empty())
        {
            frame = &frames.back();
            inst = frame->GetNextInst();
        }
        else
        {
            return;
        }
    }
    inst->Execute(*frame);
}

void Thread::Next()
{
    Frame* frame = &frames.back();
    int instIndex = frame->PC();
    if (instIndex >= frame->Fun().NumInsts())
    {
        Step();
        return;
    }
    else if (instIndex >= 0 && instIndex < frame->Fun().NumInsts())
    {
        Instruction* inst = frame->Fun().GetInst(instIndex);
        if (inst->IsJumpingInst())
        {
            Step();
            return;
        }
    }
    int32_t frameId = frame->Id();
    int32_t nextPc = frame->PC() + 1;
    std::pair<int32_t, int32_t> bp = std::make_pair(frameId, nextPc);
    breakPoints.insert(bp);
    RunDebug();
    breakPoints.erase(bp);
}

void Thread::PauseUntilGarbageCollected()
{
    {
        std::lock_guard<std::mutex> lock(mtx);
        paused.store(true);
        pausedCond.notify_one();
    }
    machine.GetGarbageCollector().WaitUntilGarbageCollected();
}

void Thread::WaitPaused()
{
    std::unique_lock<std::mutex> lock(mtx);
    pausedCond.wait(lock, [this] { return paused.load(); });
}

bool Thread::Sleeping()
{
    return sleeping.load();
}

void Thread::Sleep()
{
    sleeping.store(true);
    // todo: sleep
    sleeping.store(false);
    if (machine.GetGarbageCollector().CollectingGarbage())
    {
        machine.GetGarbageCollector().WaitUntilGarbageCollected();
    }
}

void Thread::HandleException(ObjectReference exception_)
{
    if (frames.empty())
    {
        throw std::runtime_error("no handler for exception found");
    }
    handlingException = true;
    exception = exception_;
    Frame* frame = &frames.back();
    Object& exceptionObject = frame->GetManagedMemoryPool().GetObject(exception);
    exceptionObjectType = exceptionObject.GetType();
    FindExceptionBlock(frame);
}

void Thread::EndCatch()
{
    Assert(handlingException, "not handling exception");
    handlingException = false;
    Assert(!frames.empty(), "got no frame");
    Frame* frame = &frames.back();
    Assert(currentExceptionBlock, "got no exception block");
    if (currentExceptionBlock->HasFinally())
    {
        frame->SetPC(currentExceptionBlock->FinallyStart());
    }
    else
    { 
        frame->SetPC(currentExceptionBlock->NextTarget());
    }
}

void Thread::EndFinally()
{
    if (!handlingException) return;
    Assert(!frames.empty(), "got no frame");
    Frame* frame = &frames.back();
    while (currentExceptionBlock->HasParent())
    {
        ExceptionBlock* exceptionBlock = frame->Fun().GetExceptionBlock(currentExceptionBlock->ParentId());
        Assert(exceptionBlock, "parent exception block not found");
        currentExceptionBlock = exceptionBlock;
        bool handlerOrFinallyDispatched = DispatchToHandlerOrFinally(frame);
        if (handlerOrFinallyDispatched)
        {
            return;
        }
    }
    frame = nullptr;
    frames.pop_back();
    if (!frames.empty())
    {
        frame = &frames.back();
    }
    FindExceptionBlock(frame);
}

void Thread::FindExceptionBlock(Frame* frame)
{
    while (frame)
    {
        int32_t pc = frame->PC();
        ExceptionBlock* exceptionBlock = frame->Fun().FindExceptionBlock(pc);
        while (exceptionBlock)
        {
            currentExceptionBlock = exceptionBlock;
            bool handlerOrFinallyDispatched = DispatchToHandlerOrFinally(frame);
            if (handlerOrFinallyDispatched)
            {
                return;
            }
            if (currentExceptionBlock->HasParent())
            {
                exceptionBlock = frame->Fun().GetExceptionBlock(currentExceptionBlock->ParentId());
                Assert(exceptionBlock, "parent exception block not found");
            }
            else
            {
                exceptionBlock = nullptr;
            }
        }
        frames.pop_back();
        if (frames.empty())
        {
            frame = nullptr;
        }
        else
        {
            frame = &frames.back();
        }
    }
    throw std::runtime_error("no handler for exception found");
}

bool Thread::DispatchToHandlerOrFinally(Frame* frame)
{
    Assert(currentExceptionBlock, "current exception block not set");
    int n = int(currentExceptionBlock->CatchBlocks().size());
    for (int i = 0; i < n; ++i)
    {
        CatchBlock* catchBlock = currentExceptionBlock->CatchBlocks()[i].get();
        ObjectType* catchedType = catchBlock->GetExceptionVarType();
        if (catchedType->Id() % exceptionObjectType->Id() == 0)
        {
            int32_t exceptionVarIndex = catchBlock->GetExceptionVarIndex();
            Assert(exceptionVarIndex != -1, "invalid exception variable index");
            frame->Local(exceptionVarIndex).SetValue(exception);
            frame->SetPC(catchBlock->CatchBlockStart());
            return true;
        }
    }
    if (currentExceptionBlock->HasFinally())
    {
        frame->SetPC(currentExceptionBlock->FinallyStart());
        return true;
    }
    return false;
}

} } // namespace cminor::machine
