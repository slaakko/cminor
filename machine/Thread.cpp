// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Thread.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/machine/Type.hpp>

namespace cminor { namespace machine {

Thread::Thread(Machine& machine_, Function& fun_) : machine(machine_), fun(fun_), instructionCount(0), checkWantToCollectGarbageCount(100), paused(), sleeping(), pausedCond()
{
    frames.push_back(Frame(machine, fun, fun.GetAssembly()->GetConstantPool()));
}

void Thread::Run()
{
    while (true)
    {
        CheckPause();
        Frame& frame = frames.back();
        Instruction* inst = frame.GetNextInst();
        if (!inst) break;
        inst->Execute(frame);
        IncInstructionCount();
    }
    machine.GetGarbageCollector().DoCollectGarbage();
}

void Thread::CheckPause()
{
    if (CheckWantToCollectGarbage())
    {
        if (GetMachine().GetGarbageCollector().WantToCollectGarbage())
        {
            PauseUntilGarbageCollected();
        }
    }
}

void Thread::PauseUntilGarbageCollected()
{
    std::lock_guard<std::mutex> lock(mtx);
    paused.store(true);
    pausedCond.notify_one();
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
