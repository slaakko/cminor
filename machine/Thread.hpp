// =================================
// Copyright (c) 2016 Seppo Laakko
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

struct IntPairHash
{
    size_t operator()(const std::pair<int32_t, int32_t>& x) const
    {
        return size_t(x.first) * 1099511628211 + size_t(x.second);
    }
};

class Thread
{
public:
    Thread(Machine& machine_, Function& fun_);
    Machine& GetMachine() { return machine; }
    OperandStack& OpStack() { return opStack; }
    std::vector<Frame>& Frames() { return frames; }
    const std::vector<Frame>& Frames() const { return frames; }
    void IncInstructionCount() { ++instructionCount;  }
    void Run();
    void Step();
    void Next();
    void RunDebug();
    bool CheckWantToCollectGarbage() { return (instructionCount % checkWantToCollectGarbageCount) == 0; }
    void PauseUntilGarbageCollected();
    void CheckPause();
    void WaitPaused();
    void Sleep();
    bool Sleeping();
private:
    Machine& machine;
    Function& fun;
    OperandStack opStack;
    uint64_t instructionCount;
    uint64_t checkWantToCollectGarbageCount;
    std::vector<Frame> frames;
    std::mutex mtx;
    std::atomic_bool paused;
    std::atomic_bool sleeping;
    std::condition_variable pausedCond;
    std::unordered_set<std::pair<int32_t, int32_t>, IntPairHash> breakPoints;
    void RunToEnd();
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_THREAD_INCLUDED
