// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Thread.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/machine/OsInterface.hpp>
#include <cminor/machine/Runtime.hpp>
#include <cminor/machine/Log.hpp>
#include <cminor/util/Unicode.hpp>
#include <chrono>

namespace cminor { namespace machine {

using namespace cminor::unicode;

#ifdef _WIN32
    __declspec(thread) Thread* currentThread = nullptr;
#else
    __thread Thread* currentThread = nullptr;
#endif

MACHINE_API Thread& GetCurrentThread()
{
    Assert(currentThread, "current thread not set");
    return *currentThread;
}

MACHINE_API void SetCurrentThread(Thread* currentThread_)
{
    currentThread = currentThread_;
}

DebugContext::DebugContext()
{
}

bool DebugContext::HasBreakpointAt(int32_t pc) const
{
    return breakpoints.find(pc) != breakpoints.cend();
}

void DebugContext::SetBreakpointAt(int32_t pc)
{
    breakpoints.insert(pc);
}

void DebugContext::RemoveBreakpointAt(int32_t pc)
{
    breakpoints.erase(pc);
}

ThreadExitSetter::ThreadExitSetter(Thread& thread_) : thread(thread_) 
{
}

ThreadExitSetter::~ThreadExitSetter() 
{ 
    thread.SetFunctionStack(nullptr);
    thread.SetState(ThreadState::exited);
}

Thread::Thread(int32_t id_, Machine& machine_, Function& fun_) :
    stack(*this), id(id_), machine(machine_), fun(fun_), handlingException(false), currentExceptionBlock(nullptr), state(ThreadState::paused), 
    exceptionObjectType(nullptr), nextVariableReferenceId(1), threadHandle(0), functionStack(nullptr), nativeId(-1), owner('0' + id), mtx('0' + id), 
    allocationContext(nullptr)
{
    if (GetNumAllocationContextPages() > 0)
    {
        allocationContext.reset(new AllocationContext());
    }
    stack.AllocateFrame(fun);
}

Thread::~Thread()
{
    if (threadHandle != 0)
    {
        CloseThreadHandle(threadHandle);
    }
}

Mutex requestGcMutex('R');

void Thread::RequestGc(bool requestFullCollection)
{
    LockGuard lock(requestGcMutex, owner);
    if (!wantToCollectGarbage)
    {
        if (requestFullCollection)
        {
#ifdef GC_LOGGING
            LogMessage(">" + std::to_string(id) + " Thread::RequestGc: (requesting full gc)");
#endif
            GetMachine().GetGarbageCollector().RequestFullCollection(*this);
        }
        else
        {
#ifdef GC_LOGGING
            LogMessage(">" + std::to_string(id) + " Thread::RequestGc: (requesting gc)");
#endif
            GetMachine().GetGarbageCollector().RequestGarbageCollection(*this);
        }
        OwnerGuard ownerGuard(garbageCollectorMutex, owner);
        std::unique_lock<std::mutex> lock(garbageCollectorMutex.Mtx());
        while (!wantToCollectGarbage)
        {
            GetMachine().GetGarbageCollector().WantToCollectGarbageCond().wait(lock);
        }
#ifdef GC_LOGGING
        LogMessage("<" + std::to_string(id) + " Thread::RequestGc: (gc requested)");
#endif
    }
    else
    {
#ifdef GC_LOGGING
        LogMessage(">" + std::to_string(id) + " Thread::RequestGc: (gc requested by another thread)");
#endif
    }
}

void Thread::WaitUntilGarbageCollected()
{
    if (RunningNativeCode())
    {
#ifdef GC_LOGGING
        LogMessage(">" + std::to_string(id) + " (setting native function stack)");
#endif
        SetFunctionStack(RtGetFunctionStack());
#ifdef GC_LOGGING
        LogMessage(">" + std::to_string(id) + " (native function stack set)");
#endif
    }
    SetState(ThreadState::paused);
#ifdef GC_LOGGING
    LogMessage(">" + std::to_string(id) + " (paused)");
#endif
    GetMachine().GetGarbageCollector().WaitUntilGarbageCollected(*this);
#ifdef GC_LOGGING
    LogMessage(">" + std::to_string(id) + " (collection ended)");
#endif
    SetState(ThreadState::running);
#ifdef GC_LOGGING
    LogMessage(">" + std::to_string(id) + " (running)");
#endif
    GetMachine().GetGarbageCollector().WaitForIdle(*this);
#ifdef GC_LOGGING
    LogMessage(">" + std::to_string(id) + " (gc idle)");
#endif
}

void Thread::SetState(ThreadState state_)
{
    LockGuard lock(mtx, owner);
    state = state_;
    switch (state)
    {
        case ThreadState::paused:
        {
            running = false;
            paused = true;
            pausedCond.notify_all();
            break;
        }
        case ThreadState::running:
        {
            paused = false;
            running = true;
            runningCond.notify_all();
            break;
        }
        case ThreadState::waiting:
        {
            paused = false;
            running = false;
            pausedCond.notify_all();
            runningCond.notify_all();
            break;
        }
        case ThreadState::exited:
        {
            paused = false;
            running = false;
            pausedCond.notify_all();
            runningCond.notify_all();
            break;
        }
    }
}

void Thread::WaitPaused()
{
    OwnerGuard ownerGuard(mtx, gc);
    std::unique_lock<std::mutex> lock(mtx.Mtx());
    while (state != ThreadState::exited && state != ThreadState::waiting && !paused)
    {
        pausedCond.wait(lock);
    }
}

void Thread::WaitRunning()
{
    OwnerGuard ownerGuard(mtx, gc);
    std::unique_lock<std::mutex> lock(mtx.Mtx());
    while (state != ThreadState::exited && state != ThreadState::waiting && !running)
    {
        runningCond.wait(lock);
    }
}

void Thread::RunToEnd()
{
    SetState(ThreadState::running);
    ThreadExitSetter exitSetter(*this);
    Assert(!stack.IsEmpty(), "stack is empty");
    while (true)
    {
        Frame* frame = stack.CurrentFrame();
        Instruction* inst = frame->GetNextInst();
        while (!inst)
        {
            Assert(!stack.IsEmpty(), "stack is empty");
            stack.FreeFrame();
            if (!stack.IsEmpty())
            {
                frame = stack.CurrentFrame();
                inst = frame->GetNextInst();
            }
            else
            {
                return;
            }
        }
        inst->Execute(*frame);
    }
}

void Thread::RunMain(bool runWithArgs, const std::vector<std::u32string>& programArguments, ObjectType* argsArrayObjectType)
{
    Frame* frame = stack.CurrentFrame();
    if (runWithArgs)
    {
        if (!argsArrayObjectType)
        {
            throw std::runtime_error("thread.run: args array object type not set");
        }
        if (fun.NumParameters() != 1)
        {
            throw std::runtime_error("thread.run: function does not take arguments");
        }
        ObjectReference args = GetManagedMemoryPool().CreateStringArray(*this, programArguments, argsArrayObjectType);
        frame->OpStack().Push(args);
    }
    else
    {
        if (fun.NumParameters() != 0)
        {
            throw std::runtime_error("thread.run: function takes arguments but thread run without arguments");
        }
    }
    RunToEnd();
}

void Thread::RunUser()
{
    RunToEnd();
}

void Thread::RunDebug()
{
    SetState(ThreadState::running);
    ThreadExitSetter exitSetter(*this);
    Assert(!stack.IsEmpty(), "stack is empty");
    bool first = true;
    while (true)
    {
        Frame* frame = stack.CurrentFrame();
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
            Assert(!stack.IsEmpty(), "stack is empty");
            stack.FreeFrame();
            if (!stack.IsEmpty())
            {
                frame = stack.CurrentFrame();
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
    }
}

void Thread::Step()
{
    Frame* frame = stack.CurrentFrame();
    Instruction* inst = frame->GetNextInst();
    while (!inst)
    {
        if (stack.IsEmpty())
        {
            return;
        }
        stack.FreeFrame();
        if (!stack.IsEmpty())
        {
            frame = stack.CurrentFrame();
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
    int n = int(stack.Frames().size());
    for (int i = n - 1; i >= 0; --i)
    {
        Frame* frame = stack.Frames()[i];
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
        if (stack.IsEmpty())
        {
            throw std::runtime_error("unhandled exception escaped from main");
        }
        handlingException = true;
        exception = exception_;
        Frame* frame = stack.CurrentFrame();
        std::unique_lock<std::recursive_mutex> lock(GetManagedMemoryPool().AllocationsMutex());
        void* object = GetManagedMemoryPool().GetObject(exception, lock);
        ManagedAllocationHeader* header = GetAllocationHeader(object);
        ObjectHeader* objectHeader = &header->objectHeader;
        exceptionObjectType = objectHeader->GetType();
        FindExceptionBlock(frame);
    }
    catch (const NullReferenceException& ex)
    {
        ThrowNullReferenceException(ex, *stack.CurrentFrame());
    }
}

void Thread::RethrowCurrentException()
{
    Assert(handlingException, "not handling exception");
    Assert(currentExceptionBlock, "current exception block not set");
    Assert(!stack.IsEmpty(), "stack is empty");
    Frame* frame = stack.CurrentFrame();
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
    stack.FreeFrame();
    if (stack.IsEmpty())
    {
        frame = nullptr;
    }
    else
    {
        frame = stack.CurrentFrame();
    }
    FindExceptionBlock(frame);
}

void Thread::EndCatch()
{
    Assert(handlingException, "not handling exception");
    Assert(!exception.IsNull(), "exception is null");
    handlingException = false;
    ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
    std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
    void* object = memoryPool.GetObject(exception, lock);
    ManagedAllocationHeader* header = GetAllocationHeader(object);
    header->Unreference();
    exception = ObjectReference(0);
    Assert(!stack.IsEmpty(), "stack is empty");
    Frame* frame = stack.CurrentFrame();
    Assert(currentExceptionBlock, "got no exception block");
    frame->SetPC(currentExceptionBlock->NextTarget());
}

void Thread::EndFinally()
{
    Assert(!stack.IsEmpty(), "stack is empty");
    Frame* frame = stack.CurrentFrame();
    Assert(handlingException, "end finally called while not handling exception");
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
    stack.FreeFrame();
    if (!stack.IsEmpty())
    {
        frame = stack.CurrentFrame();
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
        stack.FreeFrame();
        if (stack.IsEmpty())
        {
            frame = nullptr;
        }
        else
        {
            frame = stack.CurrentFrame();
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

std::u32string Thread::GetStackTrace() const
{
    std::u32string stackTrace;
    int n = int(stack.Frames().size());
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
        Frame* frame = stack.Frames()[i];
        std::u32string fun = U"at ";
        fun.append(frame->Fun().FullName().Value().AsStringLiteral());
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

int Thread::AllocateDebugContext()
{
    int debugContextId = int(debugContexts.size());
    debugContexts.push_back(std::unique_ptr<DebugContext>(new DebugContext()));
    return debugContextId;
}

DebugContext* Thread::GetDebugContext(int debugContextId)
{
    Assert(debugContextId >= 0 && debugContextId < int(debugContexts.size()), "invalid debug context id");
    return debugContexts[debugContextId].get();
}

void Thread::FreeDebugContext()
{
    debugContexts.pop_back();
}

} } // namespace cminor::machine
