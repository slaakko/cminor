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
    id(id_), machine(machine_), fun(fun_), instructionCount(0), handlingException(false), currentExceptionBlock(nullptr), state(ThreadState::paused), exceptionObjectType(nullptr), exitBlockNext(-1),
    nextVariableReferenceId(1)
{
    frames.push_back(std::unique_ptr<Frame>(new Frame(machine, *this, fun)));
    MapFrame(frames.back().get());
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
        Frame* frame = frames.back().get();
        Instruction* inst = frame->GetNextInst();
        while (!inst)
        {
            Assert(!frames.empty(), "thread got no frame");
            RemoveFrame(frames.back()->Id());
            frames.pop_back();
            if (!frames.empty())
            {
                frame = frames.back().get();
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
    Frame* frame = frames.back().get();
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
    bool first = true;
    while (true)
    {
        CheckPause();
        Frame* frame = frames.back().get();
        if (frame->HasBreakpointAt(frame->PC()))
        {
            return;
        }
        if (first)
        {
            first = false;
        }
        else if (frame->Fun().HasBreakPointAt(frame->PC()))
        {
            return;
        }
        Instruction* inst = frame->GetNextInst();
        while (!inst)
        {
            Assert(!frames.empty(), "thread got no frame");
            RemoveFrame(frames.back()->Id());
            frames.pop_back();
            if (!frames.empty())
            {
                frame = frames.back().get();
                inst = frame->GetNextInst();
                if (inst)
                {
                    if (frame->HasBreakpointAt(frame->PC() - 1))
                    {
                        frame->SetPC(frame->PC() - 1);
                        return;
                    }
                }
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
    Frame* frame = frames.back().get();
    Instruction* inst = frame->GetNextInst();
    while (!inst)
    {
        if (frames.empty())
        {
            return;
        }
        RemoveFrame(frames.back()->Id());
        frames.pop_back();
        if (!frames.empty())
        {
            frame = frames.back().get();
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
    int n = int(machine.MainThread().Frames().size());
    for (int i = n - 1; i >= 0; --i)
    {
        Frame* frame = machine.MainThread().Frames()[i].get();
        if (frame->PC() < frame->Fun().NumInsts())
        {
            Instruction* inst = frame->Fun().GetInst(frame->PC());
            if (inst->IsJumpingInst())
            {
                Step();
                return;
            }
            int32_t nextPC = frame->PC() + 1;
            frame->SetBreakpointAt(nextPC);
            RunDebug();
            frame->RemoveBreakpointAt(nextPC);
            return;
        }
    }
    RunDebug();
}

void Thread::BeginTry()
{
    handlingExceptionStack.push(handlingException);
    handlingException = false;
}

void Thread::EndTry()
{
    handlingException = handlingExceptionStack.top();
    handlingExceptionStack.pop();
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
        Frame* frame = frames.back().get();
        Object& exceptionObject = frame->GetManagedMemoryPool().GetObject(exception);
        exceptionObjectType = exceptionObject.GetType();
        FindExceptionBlock(frame);
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, *frames.back());
    }
}

void Thread::EndCatch()
{
    Assert(handlingException, "not handling exception");
    handlingException = false;
    Assert(!frames.empty(), "got no frame");
    Frame* frame = frames.back().get();
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
    Frame* frame = frames.back().get();
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
    RemoveFrame(frames.back()->Id());
    frames.pop_back();
    if (!frames.empty())
    {
        frame = frames.back().get();
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
        RemoveFrame(frames.back()->Id());
        frames.pop_back();
        if (frames.empty())
        {
            frame = nullptr;
        }
        else
        {
            frame = frames.back().get();
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
        Frame* frame = frames[i].get();
        utf32_string fun = U"at ";
        fun.append(frame->Fun().FriendlyName().Value().AsStringLiteral());
        if (frame->Fun().HasSourceFilePath())
        {
            uint32_t line = frame->Fun().GetSourceLine(frame->PrevPC());
            if (line != -1)
            {
                fun.append(U" [").append(frame->Fun().SourceFilePath().Value().AsStringLiteral()).append(1, U':').append(ToUtf32(std::to_string(line))).append(1, U']');
            }
        }
        stackTrace.append(fun);
    }
    return stackTrace;
}

void Thread::MapFrame(Frame* frame)
{
    frameMap[frame->Id()] = frame;
}

void Thread::RemoveFrame(int32_t frameId)
{
    frameMap.erase(frameId);
}

Frame* Thread::GetFrame(int32_t frameId) const
{
    auto it = frameMap.find(frameId);
    if (it != frameMap.cend())
    {
        return it->second;
    }
    return nullptr;
}

void Thread::AddVariableReference(VariableReference* variableReference)
{
    variableReferenceMap[variableReference->Id()] = variableReference;
}

VariableReference* Thread::GetVariableReference(int32_t variableReferenceId) const
{
    auto it = variableReferenceMap.find(variableReferenceId);
    if (it != variableReferenceMap.cend())
    {
        return it->second;
    }
    return nullptr;
}

void Thread::RemoveVariableReference(int32_t variableReferenceId)
{
    variableReferenceMap.erase(variableReferenceId);
}

int32_t Thread::GetNextVariableReferenceId()
{
    return nextVariableReferenceId++;
}

void Thread::Compact()
{
    frames.shrink_to_fit();
}

} } // namespace cminor::machine
