// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/GarbageCollector.hpp>
#include <cminor/machine/Machine.hpp>

namespace cminor { namespace machine {

GarbageCollector::GarbageCollector(Machine& machine_, uint64_t garbageCollectionIntervalMs_) :
    machine(machine_), wantToCollectGarbage(), collectingGarbage(), garbageCollected(), doGarbageCollection(),
    garbageCollectedCond(), doGarbageCollectionCond(), garbageCollectionIntervalMs(garbageCollectionIntervalMs_)
{
}

bool GarbageCollector::WantToCollectGarbage()
{
    return wantToCollectGarbage.load();
}

void GarbageCollector::SetWantToCollectGarbage()
{
    wantToCollectGarbage.store(true);
}

void GarbageCollector::ResetWantToCollectGarbage()
{
    wantToCollectGarbage.store(false);
}

void GarbageCollector::WaitForThreadsPaused()
{
    for (std::unique_ptr<Thread>& thread : machine.Threads())
    {
        if (!thread->Sleeping())
        {
            thread->WaitPaused();
        }
    }
}

void GarbageCollector::WaitUntilGarbageCollected()
{
    std::unique_lock<std::mutex> lock(garbageCollectedMtx);
    garbageCollectedCond.wait(lock, [this] { return garbageCollected.load(); });
}

void GarbageCollector::SetGarbageCollected()
{
    std::lock_guard<std::mutex> lock(garbageCollectedMtx);
    garbageCollected.store(true);
    garbageCollectedCond.notify_all();
}

void GarbageCollector::ResetGarbageCollected()
{
    garbageCollected.store(false);
}

bool GarbageCollector::CollectingGarbage()
{
    return collectingGarbage.load();
}

void GarbageCollector::SetCollectingGarbage()
{
    collectingGarbage.store(true);
}

void GarbageCollector::ResetCollectingGarbage()
{
    collectingGarbage.store(false);
}

void GarbageCollector::RequestGarbageCollection()
{
    std::lock_guard<std::mutex> lock(doGarbageCollectionMtx);
    doGarbageCollection.store(true);
    doGarbageCollectionCond.notify_one();
}

void GarbageCollector::WaitForGarbageCollection()
{
    std::unique_lock<std::mutex> lock(doGarbageCollectionMtx);
    std::chrono::duration<uint64_t, std::milli> garbageCollectionInterval{ garbageCollectionIntervalMs };
    doGarbageCollectionCond.wait_for(lock, garbageCollectionInterval, [this] { return doGarbageCollection.load(); });
}

void GarbageCollector::Run()
{
    while (!machine.Exiting())
    {
        WaitForGarbageCollection();
        if (machine.Exiting()) break;
        ResetGarbageCollected();
        SetWantToCollectGarbage();
        SetCollectingGarbage();
        WaitForThreadsPaused();
        DoCollectGarbage();
        ResetWantToCollectGarbage();
        ResetCollectingGarbage();
        SetGarbageCollected();
    }
}

void GarbageCollector::DoCollectGarbage()
{
    DoGarbageCollectArena(ArenaId::gen0Arena);
}

void GarbageCollector::DoGarbageCollectArena(ArenaId arenaId)
{
    machine.GetObjectPool().ResetObjectsLiveFlag(arenaId);
    MarkLiveObjects(arenaId);
    if (arenaId == ArenaId::gen0Arena)
    {
        machine.GetObjectPool().MoveLiveObjectsToArena(arenaId, machine.Gen1Arena());
        machine.Gen0Arena().Clear();
    }
    else
    {
        // compact gen1Arena 
    }
}

void GarbageCollector::MarkLiveObjects(ArenaId arenaId)
{
    for (const std::unique_ptr<Thread>& thread : machine.Threads())
    {
        for (const Frame& frame : thread->Frames())
        {
            const OperandStack& operandStack = frame.OpStack();
            for (IntegralValue value : operandStack.Values())
            {
                if (value.GetType() == ValueType::objectReference)
                {
                    ObjectReference objectRef(value.Value());
                    if (!objectRef.IsNull())
                    {
                        Object& object = machine.GetObjectPool().GetObject(objectRef);
                        if (object.GetArenaId() == arenaId)
                        {
                            object.SetLive();
                        }
                    }
                }
            }
            const LocalVariableVector& locals = frame.Locals();
            for (const LocalVariable& local : locals.Variables())
            {
                IntegralValue value = local.GetValue();
                if (value.GetType() == ValueType::objectReference)
                {
                    ObjectReference objectRef(value.Value());
                    if (!objectRef.IsNull())
                    {
                        Object& object = machine.GetObjectPool().GetObject(objectRef);
                        if (object.GetArenaId() == arenaId)
                        {
                            object.SetLive();
                        }
                    }
                }
            }
        }
    }
    // todo: mark also objects referred in static fields
}

} } // namespace cminor::machine
