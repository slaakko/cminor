// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Thread.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Function.hpp>

namespace cminor { namespace machine {

Thread::Thread(Machine& machine_, Function& fun_) : 
    machine(machine_), fun(fun_), instructionCount(0), checkWantToCollectGarbageCount(100), paused(), sleeping(), pausedCond()
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

} } // namespace cminor::machine
