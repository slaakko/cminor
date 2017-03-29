// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/GarbageCollector.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Log.hpp>
#include <cminor/machine/Runtime.hpp>
#include <cminor/machine/Stats.hpp>
#include <iostream>

namespace cminor { namespace machine {

MutexOwner gc('G');

std::atomic_bool wantToCollectGarbage = false;

Mutex garbageCollectorMutex('G');

GarbageCollector::GarbageCollector(Machine& machine_) : machine(machine_), state(GarbageCollectorState::idle), started(false), collectionRequested(false), fullCollectionRequested(false), 
    idle(true), collected(false), error(false), printActions(false)
{
}

void GarbageCollector::WaitForIdle(Thread& thread)
{
#ifdef GC_LOGGING
    LogMessage(">" + std::to_string(thread.Id()) + ":WaitForIdle()");
#endif
    OwnerGuard ownerGuard(garbageCollectorMutex, thread.Owner());
    std::unique_lock<std::mutex> lock(garbageCollectorMutex.Mtx());
    while (!idle)
    {
        idleCond.wait(lock);
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
    OwnerGuard ownerGuard(garbageCollectorMutex, thread.Owner());
    std::unique_lock<std::mutex> lock(garbageCollectorMutex.Mtx());
    while (!idle)
    {
        idleCond.wait(lock);
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
    OwnerGuard ownerGuard(garbageCollectorMutex, thread.Owner());
    std::unique_lock<std::mutex> lock(garbageCollectorMutex.Mtx());
    while (!idle)
    {
        idleCond.wait(lock);
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
    OwnerGuard ownerGuard(garbageCollectorMutex, thread.Owner());
    std::unique_lock<std::mutex> lock(garbageCollectorMutex.Mtx());
    while (!collected)
    {
        collectedCond.wait(lock);
    }
#ifdef GC_LOGGING
    LogMessage("<" + std::to_string(thread.Id()) + ":WaitUntilGarbageCollected()");
#endif
}

void GarbageCollector::SetState(GarbageCollectorState state_)
{
    state = state_;
    switch (state)
    {
        case GarbageCollectorState::idle:
        {
            collected = false;
            idle = true;
            idleCond.notify_all();
            break;
        }
        case GarbageCollectorState::requested:
        {
            OwnerGuard ownerGuard(garbageCollectorMutex, gc);
            std::unique_lock<std::mutex> lock(garbageCollectorMutex.Mtx());
            collectionRequested = false;
            wantToCollectGarbage = true;
            wantToCollectGarbageCond.notify_all();
            break;
        }
        case GarbageCollectorState::collecting:
        {
            idle = false;
            wantToCollectGarbage = false;
            break;
        }
        case GarbageCollectorState::collected:
        {
            OwnerGuard ownerGuard(garbageCollectorMutex, gc);
            std::unique_lock<std::mutex> lock(garbageCollectorMutex.Mtx());
            collected = true;
            collectedCond.notify_all();
            break;
        }
    }
}

void GarbageCollector::WaitForGarbageCollection()
{
#ifdef GC_LOGGING
    LogMessage(">gc:WaitForGarbageCollection() (idle)");
#endif
    OwnerGuard ownerGuard(garbageCollectorMutex, gc);
    std::unique_lock<std::mutex> lock(garbageCollectorMutex.Mtx());
    fullCollectionRequested = false;
    SetState(GarbageCollectorState::idle);
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
            SetState(GarbageCollectorState::requested);
#ifdef GC_LOGGING
            LogMessage(">gc:Run() (requested)");
#endif
            WaitForThreadsPaused();
            SetState(GarbageCollectorState::collecting);
#ifdef GC_LOGGING
            LogMessage(">gc:Run() (collecting)");
#endif
            CollectGarbage();
            SetState(GarbageCollectorState::collected);
#ifdef GC_LOGGING
            LogMessage(">gc:Run() (collected)");
#endif
            WaitForThreadsRunning();
        }
    }
}

void GarbageCollector::CollectGarbage()
{
	if (printActions)
	{
		std::cerr << "[G]";
	}
    auto start = std::chrono::system_clock::now();
    ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
    memoryPool.ResetLiveFlags();
    MarkLiveAllocations();
    memoryPool.MoveLiveAllocationsToArena(ArenaId::gen1Arena, machine.Gen2Arena());
    machine.Gen1Arena().Clear();
    if (fullCollectionRequested)
    {
		if (printActions)
		{
			std::cerr << "[F]";
		}
		memoryPool.MoveLiveAllocationsToNewSegments(machine.Gen2Arena());
    }
    machine.Compact();
    for (const std::unique_ptr<Thread>& thread : machine.Threads())
    {
        AllocationContext* allocationContext = thread->GetAllocationContext();
        if (allocationContext)
        {
            allocationContext->Clear();
        }
    }
#ifdef GC_LOGGING
    std::cout << ".";
#endif
    auto end = std::chrono::system_clock::now();
    auto duration = end - start;
    int64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    AddGcTime(ms, fullCollectionRequested);
}

inline void GarbageCollector::MarkLiveAllocations(ObjectReference objectReference, std::unordered_set<AllocationHandle, AllocationHandleHash>& checked)
{
    if (!objectReference.IsNull() && checked.find(objectReference) == checked.cend())
    {
        checked.insert(objectReference);
        void* object = GetManagedMemoryPool().GetObjectNoThrowNoLock(objectReference);
        if (object)
        {
            ManagedAllocationHeader* header = GetAllocationHeader(object);
            header->SetLive();
            MarkObjectLiveAllocations(object, checked);
        }
    }
}

void GarbageCollector::MarkLiveAllocations()
{
    std::unordered_set<AllocationHandle, AllocationHandleHash> checked;
    ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
    for (const auto& p : ClassDataTable::ClassDataMap())
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
                        ObjectReference gcRoot(value.Value());
                        MarkLiveAllocations(gcRoot, checked);
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
                ObjectReference gcRoot(value.Value());
                MarkLiveAllocations(gcRoot, checked);
            }
        }
        for (Frame* frame : thread->GetStack().Frames())
        {
            int n = frame->NumLocals();
            for (int i = 0; i < n; ++i)
            {
                const LocalVariable& local = frame->Local(i);
                IntegralValue value = local.GetValue();
                if (value.GetType() == ValueType::objectReference)
                {
                    ObjectReference gcRoot(value.Value());
                    MarkLiveAllocations(gcRoot, checked);
                }
            }
        }
        if (RunningNativeCode())
        {
            FunctionStackEntry* functionStackEntry = thread->GetFunctionStack();
            while (functionStackEntry)
            {
                uint64_t** gcEntry = functionStackEntry->gcEntry;
                if (gcEntry)
                {
                    int32_t n = functionStackEntry->numGcRoots;
                    for (int32_t i = 0; i < n; ++i)
                    {
                        uint64_t* gcRootPtr = gcEntry[i];
                        ObjectReference gcRoot(*gcRootPtr);
                        MarkLiveAllocations(gcRoot, checked);
                    }
                }
                functionStackEntry = functionStackEntry->next;
            }
        }
    }
}

} } // namespace cminor::machine
