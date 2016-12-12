// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Thread.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Function.hpp>
#include <chrono>

namespace cminor { namespace machine {

Thread::Thread(int32_t id_, Machine& machine_, Function& fun_) :
    id(id_), machine(machine_), fun(fun_), instructionCount(0), handlingException(false), currentExceptionBlock(nullptr), state(ThreadState::paused), exceptionObjectType(nullptr), exitBlockNext(-1)
{
    frames.push_back(Frame(machine, *this, fun));
}

inline void Thread::CheckPause()
{
    if (GetMachine().GetGarbageCollector().WantToCollectGarbage())
    {
        SetState(ThreadState::paused);
        GetMachine().GetGarbageCollector().WaitUntilGarbageCollected(*this);
        SetState(ThreadState::running);
        GetMachine().GetGarbageCollector().WaitForIdle(*this);
    }
}

void Thread::SetState(ThreadState state_)
{
    state = state_;
}

void Thread::WaitPaused()
{
    while (state != ThreadState::paused && state != ThreadState::exited)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
    }
}

void Thread::WaitRunning()
{
    while (state != ThreadState::running && state != ThreadState::exited)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
    }
}

struct ExitSetter
{
    ExitSetter(Thread& thread_) : thread(thread_) {}
    ~ExitSetter() { thread.SetState(ThreadState::exited);  }
    Thread& thread;
};

void Thread::RunToEnd()
{
    SetState(ThreadState::running);
    ExitSetter exitSetter(*this);
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

void Thread::Run(const std::vector<utf32_string>& programArguments, ObjectType* argsArrayObjectType)
{
    Frame* frame = &frames.back();
    if (argsArrayObjectType)
    {
        ObjectReference args = frame->GetManagedMemoryPool().CreateStringArray(*this, programArguments, argsArrayObjectType);
        frame->OpStack().Push(args);
    }
    RunToEnd();
}

void Thread::RunDebug()
{
    SetState(ThreadState::running);
    ExitSetter exitSetter(*this);
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

void Thread::HandleException(ObjectReference exception_)
{
    try
    {
        if (frames.empty())
        {
            throw std::runtime_error("unhandled exception escaped from main");
        }
        handlingException = true;
        exception = exception_;
        Frame* frame = &frames.back();
        Object& exceptionObject = frame->GetManagedMemoryPool().GetObject(exception);
        exceptionObjectType = exceptionObject.GetType();
        FindExceptionBlock(frame);
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, frames.back());
    }
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
    Assert(!frames.empty(), "got no frame");
    Frame* frame = &frames.back();
    if (exitBlockNext != -1)
    {
        int32_t next = exitBlockNext;
        PopExitBlock();
        frame->SetPC(next);
        return;
    }
    if (!handlingException)
    {
        return;
    }
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
    throw std::runtime_error("unhandled exception escaped from main");
}

bool Thread::DispatchToHandlerOrFinally(Frame* frame)
{
    Assert(currentExceptionBlock, "current exception block not set");
    int n = int(currentExceptionBlock->CatchBlocks().size());
    for (int i = 0; i < n; ++i)
    {
        CatchBlock* catchBlock = currentExceptionBlock->CatchBlocks()[i].get();
        ObjectType* catchedType = catchBlock->GetExceptionVarType();
        if (exceptionObjectType->Id() % catchedType->Id() == 0)
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

void Thread::PushExitBlock(int32_t exitBlockNext_)
{
    exitBlockStack.push(exitBlockNext);
    exitBlockNext = exitBlockNext_;
}

void Thread::PopExitBlock()
{
    exitBlockNext = exitBlockStack.top();
    exitBlockStack.pop();
}

utf32_string Thread::GetStackTrace() const
{
    utf32_string stackTrace;
    int n = int(frames.size());
    bool first = true;
    for (int i = n - 1; i >= 0; --i)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            stackTrace.append(1, U'\n');
        }
        Frame& frame = const_cast<Frame&>(frames[i]);
        utf32_string fun = U"at ";
        fun.append(frame.Fun().FriendlyName().Value().AsStringLiteral());
        if (frame.Fun().HasSourceFilePath())
        {
            uint32_t line = frame.Fun().GetSourceLine(frame.PrevPC());
            if (line != -1)
            {
                fun.append(U" [").append(frame.Fun().SourceFilePath().Value().AsStringLiteral()).append(1, U':').append(ToUtf32(std::to_string(line))).append(1, U']');
            }
        }
        stackTrace.append(fun);
    }
    return stackTrace;
}

} } // namespace cminor::machine
