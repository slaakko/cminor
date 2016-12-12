// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_GARBAGE_COLLECTOR_INCLUDED
#define CMINOR_MACHINE_GARBAGE_COLLECTOR_INCLUDED
#include <cminor/machine/Object.hpp>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <ratio>

namespace cminor { namespace machine {

extern std::mutex garbageCollectorMutex;

class Machine;

enum class GarbageCollectorState
{
    idle, requested, collecting, collected
};

class GarbageCollector
{
public:
    GarbageCollector(Machine& machine_);
    bool WantToCollectGarbage();
    void WaitForIdle(Thread& thread);
    void RequestGarbageCollection(Thread& thread);
    void RequestFullCollection(Thread& thread);
    void RequestFullCollection();
    void WaitUntilGarbageCollected(Thread& thread);
    void Run();
    bool Started() const { return started; }
private:
    Machine& machine;
    std::atomic<GarbageCollectorState> state;
    std::atomic_bool collectionRequested;
    std::condition_variable collectionRequestedCond;
    bool started;
    bool fullCollectionRequested;
    void WaitForGarbageCollection();
    void WaitForThreadsPaused();
    void WaitForThreadsRunning();
    void CollectGarbage();
    void MarkLiveAllocations(ObjectReference objectReference, std::unordered_set<AllocationHandle, AllocationHandleHash>& checked);
    void MarkLiveAllocations();
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_GARBAGE_COLLECTOR_INCLUDED
