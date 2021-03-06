// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_THREAD_INCLUDED
#define CMINOR_MACHINE_THREAD_INCLUDED
#include <cminor/machine/MachineApi.hpp>
#include <cminor/machine/Stack.hpp>
#include <cminor/machine/Object.hpp>
#include <cminor/machine/OperandStack.hpp>
#include <cminor/machine/VariableReference.hpp>
#include <cminor/util/Mutex.hpp>
#include <unordered_set>
#include <atomic>
#include <condition_variable>
#include <mutex>

namespace cminor { namespace machine {

class Machine;
class Function;
class ExceptionBlock;
class Thread;
struct FunctionStackEntry;

extern std::atomic<bool> wantToCollectGarbage;

MACHINE_API Thread& GetCurrentThread();
MACHINE_API void SetCurrentThread(Thread* currentThread_);

struct IntPairHash
{
    size_t operator()(const std::pair<int32_t, int32_t>& x) const
    {
        return size_t(x.first) * 1099511628211 + size_t(x.second);
    }
};

enum class ThreadState
{
    paused, running, waiting, exited
};

struct MACHINE_API ThreadExitSetter
{
    ThreadExitSetter(Thread& thread_);
    ~ThreadExitSetter();
    Thread& thread;
};

class DebugContext
{
public:
    DebugContext();
    bool HasBreakpointAt(int32_t pc) const;
    void SetBreakpointAt(int32_t pc);
    void RemoveBreakpointAt(int32_t pc);
private:
    std::unordered_set<int32_t> breakpoints;
};

class AllocationContext;

class MACHINE_API Thread
{
public:
    Thread(int32_t id_, Machine& machine_, Function& fun_);
    ~Thread();
    const Stack& GetStack() const { return stack; }
    Stack& GetStack() { return stack; }
    int32_t Id() const { return id; }
    int32_t NativeId() const { return nativeId; }
    void SetNativeId(int32_t nativeId_) { nativeId = nativeId_; }
    Machine& GetMachine() { return machine; }
    OperandStack& OpStack() { return opStack; }
    void RunMain(bool runWithArgs, const std::vector<std::u32string>& programArguments, ObjectType* argsArrayObjectType);
    void RunUser();
    void Step();
    void Next();
    void RunDebug();
    void PollGc()
    {
        if (wantToCollectGarbage)
        {
            WaitUntilGarbageCollected();
        }
    }
    void RequestGc(bool requestFullCollection);
    void WaitUntilGarbageCollected();
    void WaitPaused();
    void WaitRunning();
    void SetState(ThreadState state_);
    ThreadState GetState() const { return state; }
    void BeginTry();
    void EndTry();
    void HandleException(ObjectReference exception_);
    void RethrowCurrentException();
    void EndCatch();
    void EndFinally();
    std::u32string GetStackTrace() const;
    void MapFrame(Frame* frame);
    void RemoveFrame(int32_t frameId);
    Frame* GetFrame(int32_t frameId) const;
    void AddVariableReference(VariableReference* variableReference);
    VariableReference* GetVariableReference(int32_t variableReferenceId) const;
    void RemoveVariableReference(int32_t variableReferenceId);
    int32_t GetNextVariableReferenceId();
    ObjectReference Exception() const { return exception; }
    int AllocateDebugContext();
    DebugContext* GetDebugContext(int debugContextId);
    void FreeDebugContext();
    uint64_t GetThreadHandle() const { return threadHandle; }
    void SetThreadHandle(uint64_t threadHandle_) { threadHandle = threadHandle_; }
    void SetFunctionStack(FunctionStackEntry* functionStack_) { functionStack = functionStack_; }
    FunctionStackEntry* GetFunctionStack() const { return functionStack; }
    void SetExceptionPtr(std::exception_ptr exceptionPtr_) { exceptionPtr = exceptionPtr_; }
    MutexOwner& Owner() { return owner; }
    Mutex& Mtx() { return mtx; }
    AllocationContext* GetAllocationContext() { return allocationContext.get(); }
    void PushAllocationHandle(AllocationHandle handle) { allocationHandleStack.push(handle); }
    AllocationHandle PopAllocationHandle() { AllocationHandle handle = allocationHandleStack.top();  allocationHandleStack.pop(); return handle; }
    bool HasAllocationHandles() const { return !allocationHandleStack.empty(); }
    void* StackPtr() const { return stackPtr; }
    void SetStackPtr(void* stackPtr_) { stackPtr = stackPtr_; }
    void* FramePtr() const { return framePtr; }
    void SetFramePtr(void* framePtr_) { framePtr = framePtr_; }
    const Function* ThreadMain() const { return &fun; }
private:
    Stack stack;
    int32_t id;
    int32_t nativeId;
    Machine& machine;
    Function& fun;
    OperandStack opStack;
    std::unordered_map<int32_t, Frame*> frameMap;
    std::unordered_map<int32_t, VariableReference*> variableReferenceMap;
    bool handlingException;
    std::stack<bool> handlingExceptionStack;
    ObjectReference exception;
    ExceptionBlock* currentExceptionBlock;
    ObjectType* exceptionObjectType;
    std::atomic<ThreadState> state;
    std::atomic<bool> paused;
    std::condition_variable pausedCond;
    std::atomic<bool> running;
    std::condition_variable runningCond;
    int32_t nextVariableReferenceId;
    std::vector<std::unique_ptr<DebugContext>> debugContexts;
    uint64_t threadHandle;
    std::atomic<FunctionStackEntry*> functionStack;
    std::exception_ptr exceptionPtr;
    MutexOwner owner;
    Mutex mtx;
    std::unique_ptr<AllocationContext> allocationContext;
    std::stack<AllocationHandle> allocationHandleStack;
    void* stackPtr;
    void* framePtr;
    void RunToEnd();
    void FindExceptionBlock(Frame* frame);
    bool DispatchToHandlerOrFinally(Frame* frame);
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_THREAD_INCLUDED
