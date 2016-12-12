// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_THREAD_INCLUDED
#define CMINOR_MACHINE_THREAD_INCLUDED
#include <cminor/machine/Frame.hpp>
#include <vector>
#include <atomic>
#include <condition_variable>
#include <mutex>

namespace cminor { namespace machine {

class Machine;
class Function;
class ExceptionBlock;

struct IntPairHash
{
    size_t operator()(const std::pair<int32_t, int32_t>& x) const
    {
        return size_t(x.first) * 1099511628211 + size_t(x.second);
    }
};

enum class ThreadState
{
    paused, running, exited
};

class Thread
{
public:
    Thread(int32_t id_, Machine& machine_, Function& fun_);
    int32_t Id() const { return id; }
    Machine& GetMachine() { return machine; }
    OperandStack& OpStack() { return opStack; }
    std::vector<Frame>& Frames() { return frames; }
    const std::vector<Frame>& Frames() const { return frames; }
    void IncInstructionCount() { ++instructionCount;  }
    void Run(const std::vector<utf32_string>& programArguments, ObjectType* argsArrayObjectType);
    void Step();
    void Next();
    void RunDebug();
    void CheckPause();
    void WaitPaused();
    void WaitRunning();
    void SetState(ThreadState state_);
    void HandleException(ObjectReference exception_);
    void EndCatch();
    void EndFinally();
    void PushExitBlock(int32_t exitBlockNext_);
    void PopExitBlock();
    utf32_string GetStackTrace() const;
private:
    int32_t id;
    Machine& machine;
    Function& fun;
    OperandStack opStack;
    uint64_t instructionCount;
    std::vector<Frame> frames;
    std::unordered_set<std::pair<int32_t, int32_t>, IntPairHash> breakPoints;
    bool handlingException;
    ObjectReference exception;
    ExceptionBlock* currentExceptionBlock;
    ObjectType* exceptionObjectType;
    int32_t exitBlockNext;
    std::stack<int32_t> exitBlockStack;
    std::atomic<ThreadState> state;
    void RunToEnd();
    void FindExceptionBlock(Frame* frame);
    bool DispatchToHandlerOrFinally(Frame* frame);
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_THREAD_INCLUDED
