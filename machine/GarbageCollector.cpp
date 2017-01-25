// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/GarbageCollector.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Log.hpp>
#include <iostream>

namespace cminor { namespace machine {

std::mutex garbageCollectorMutex;

GarbageCollector::GarbageCollector(Machine& machine_) : machine(machine_), state(GarbageCollectorState::idle), started(false), collectionRequested(false), fullCollectionRequested(false), error(false)
{
}

bool GarbageCollector::WantToCollectGarbage()
{
    return state == GarbageCollectorState::requested;
}

void GarbageCollector::WaitForIdle(Thread& thread)
{
#ifdef GC_LOGGING
    LogMessage(">" + std::to_string(thread.Id()) + ":WaitForIdle()");
#endif
    while (state != GarbageCollectorState::idle)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
    }
#ifdef GC_LOGGING
    LogMessage("<" + std::to_string(thread.Id()) + ":WaitForIdle()");
#endif
}

void GarbageCollector::WaitForThreadsPaused()
{
#ifdef GC_LOGGING
    LogMessage(">gc:WaitForThreadsPaused()");
#endif
    for (std::unique_ptr<Thread>& thread : machine.Threads())
    {
        thread->WaitPaused();
    }
#ifdef GC_LOGGING
    LogMessage("<gc:WaitForThreadsPaused()");
#endif
}

void GarbageCollector::WaitForThreadsRunning()
{
#ifdef GC_LOGGING
    LogMessage(">gc:WaitForThreadsRunning()");
#endif
    for (std::unique_ptr<Thread>& thread : machine.Threads())
    {
        thread->WaitRunning();
    }
#ifdef GC_LOGGING
    LogMessage("<gc:WaitForThreadsRunning()");
#endif
}

void GarbageCollector::RequestGarbageCollection(Thread& thread)
{
#ifdef GC_LOGGING
    LogMessage(">" + std::to_string(thread.Id()) + ":RequestGarbageCollection()");
#endif
    while (state != GarbageCollectorState::idle)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
    }
    collectionRequested = true;
    collectionRequestedCond.notify_one();
#ifdef GC_LOGGING
    LogMessage("<" + std::to_string(thread.Id()) + ":RequestGarbageCollection()");
#endif
}

void GarbageCollector::RequestFullCollection(Thread& thread)
{
#ifdef GC_LOGGING
    LogMessage(">" + std::to_string(thread.Id()) + ":RequestFullCollection()");
#endif
    while (state != GarbageCollectorState::idle)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
    }
    fullCollectionRequested = true;
    collectionRequested = true;
    collectionRequestedCond.notify_one();
#ifdef GC_LOGGING
    LogMessage("<" + std::to_string(thread.Id()) + ":RequestFullCollection()");
#endif
}

void GarbageCollector::RequestFullCollection()
{
#ifdef GC_LOGGING
    LogMessage(">gc:RequestFullCollection()");
#endif
    fullCollectionRequested = true;
#ifdef GC_LOGGING
    LogMessage("<gc:RequestFullCollection()");
#endif
}

void GarbageCollector::WaitUntilGarbageCollected(Thread& thread)
{
#ifdef GC_LOGGING
    LogMessage(">" + std::to_string(thread.Id()) + ":WaitUntilGarbageCollected()");
#endif
    while (state != GarbageCollectorState::collected)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
    }
#ifdef GC_LOGGING
    LogMessage("<" + std::to_string(thread.Id()) + ":WaitUntilGarbageCollected()");
#endif
}

void GarbageCollector::WaitForGarbageCollection()
{
#ifdef GC_LOGGING
    LogMessage(">gc:WaitForGarbageCollection() (idle)");
#endif
    std::unique_lock<std::mutex> lock(garbageCollectorMutex);
    fullCollectionRequested = false;
    state = GarbageCollectorState::idle;
    collectionRequestedCond.wait(lock);
#ifdef GC_LOGGING
    LogMessage("<gc:WaitForGarbageCollection() (idle)");
#endif
}

void GarbageCollector::Run()
{
    started = true;
    while (!machine.Exiting())
    {
        WaitForGarbageCollection();
        if (machine.Exiting()) break;
        if (collectionRequested)
        {
            collectionRequested = false;
            state = GarbageCollectorState::requested;
#ifdef GC_LOGGING
            LogMessage(">gc:Run() (requested)");
#endif
            WaitForThreadsPaused();
            state = GarbageCollectorState::collecting;
#ifdef GC_LOGGING
            LogMessage(">gc:Run() (collecting)");
#endif
            CollectGarbage();
            state = GarbageCollectorState::collected;
#ifdef GC_LOGGING
            LogMessage(">gc:Run() (collected)");
#endif
            WaitForThreadsRunning();
        }
    }
}

void GarbageCollector::CollectGarbage()
{
    //auto start = std::chrono::system_clock::now();
    machine.GetManagedMemoryPool().ResetLiveFlags();
    MarkLiveAllocations();
    machine.GetManagedMemoryPool().MoveLiveAllocationsToArena(ArenaId::gen1Arena, machine.Gen2Arena());
    machine.Gen1Arena().Clear();
    if (fullCollectionRequested)
    {
        machine.GetManagedMemoryPool().MoveLiveAllocationsToNewSegments(machine.Gen2Arena());
    }
    machine.Compact();
#ifdef GC_LOGGING
    std::cout << ".";
#endif
    //auto end = std::chrono::system_clock::now();
    //auto duration = end - start;
    //long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    //std::cout << "gc " << ms << " " << fullCollectionRequested << std::endl;
}

inline void GarbageCollector::MarkLiveAllocations(ObjectReference objectReference, std::unordered_set<AllocationHandle, AllocationHandleHash>& checked)
{
    if (!objectReference.IsNull() && checked.find(objectReference) == checked.cend())
    {
        checked.insert(objectReference);
        Object* object = machine.GetManagedMemoryPool().GetObjectNothrow(objectReference);
        if (object)
        {
            object->SetLive();
            object->MarkLiveAllocations(checked, machine.GetManagedMemoryPool());
        }
    }
}

void GarbageCollector::MarkLiveAllocations()
{
    std::unordered_set<AllocationHandle, AllocationHandleHash> checked;
    AllocationHandle handle = machine.GetManagedMemoryPool().PoolRoot();
    if (handle.Value() != 0)
    {
        ManagedAllocation* allocation = machine.GetManagedMemoryPool().GetAllocationNothrow(handle);
        if (allocation)
        {
            allocation->SetLive();
            allocation->MarkLiveAllocations(checked, machine.GetManagedMemoryPool());
        }
    }
    for (const auto& p : ClassDataTable::Instance().ClassDataMap())
    {
        ClassData* classData = p.second;
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (staticClassData)
        {
            if (staticClassData->HasStaticData())
            {
                Layout& staticLayout = staticClassData->StaticLayout();
                int n = staticLayout.FieldCount();
                for (int i = 0; i < n; ++i)
                {
                    Field field = staticLayout.GetField(i);
                    ValueType valueType = field.GetType();
                    if (valueType == ValueType::objectReference)
                    {
                        IntegralValue value = staticClassData->GetStaticField(i);
                        Assert(value.GetType() == ValueType::objectReference, "object reference expected");
                        ObjectReference objectReference(value.Value());
                        MarkLiveAllocations(objectReference, checked);
                    }
                }
            }
        }
    }
    for (const std::unique_ptr<Thread>& thread : machine.Threads())
    {
        ObjectReference exceptionReference = thread->Exception();
        MarkLiveAllocations(exceptionReference, checked);
        const OperandStack& operandStack = thread->OpStack();
        for (IntegralValue value : operandStack.Values())
        {
            if (value.GetType() == ValueType::objectReference)
            {
                ObjectReference objectReference(value.Value());
                MarkLiveAllocations(objectReference, checked);
            }
        }
        for (const std::unique_ptr<Frame>& frame : thread->Frames())
        {
            const LocalVariableVector& locals = frame->Locals();
            for (const LocalVariable& local : locals.Variables())
            {
                IntegralValue value = local.GetValue();
                if (value.GetType() == ValueType::objectReference)
                {
                    ObjectReference objectReference(value.Value());
                    MarkLiveAllocations(objectReference, checked);
                }
            }
        }
    }
}

} } // namespace cminor::machine
