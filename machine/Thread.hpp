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
    void RunToEnd();
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_THREAD_INCLUDED
