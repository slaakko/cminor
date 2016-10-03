// =================================
// Copyright (c) 2016 Seppo Laakko
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

class Machine;
const uint64_t defaultGarbageCollectionIntervalMs = 250;

class GarbageCollector
{
public:
    GarbageCollector(Machine& machine_, uint64_t garbageCollectionIntervalMs_);
    bool WantToCollectGarbage();
    void SetWantToCollectGarbage();
    void ResetWantToCollectGarbage();
    void WaitForThreadsPaused();
    void WaitUntilGarbageCollected();
    void SetGarbageCollected();
    void ResetGarbageCollected();
    bool CollectingGarbage();
    void SetCollectingGarbage();
    void ResetCollectingGarbage();
    void DoCollectGarbage();
    void DoGarbageCollectArena(ArenaId arenaId);
    void MarkLiveObjects(IntegralValue value, std::unordered_set<ObjectReference, ObjectReferenceHash>& checked);
    void MarkLiveObjects();
    void RequestGarbageCollection();
    void WaitForGarbageCollection();
    void Run();
    bool Started() const { return started; }
private:
    Machine& machine;
    bool started;
    std::atomic_bool wantToCollectGarbage;
    std::atomic_bool collectingGarbage;
    std::atomic_bool garbageCollected;
    std::atomic_bool doGarbageCollection;
    std::mutex garbageCollectedMtx;
    std::condition_variable garbageCollectedCond;
    std::mutex doGarbageCollectionMtx;
    std::condition_variable doGarbageCollectionCond;
    uint64_t garbageCollectionIntervalMs;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_GARBAGE_COLLECTOR_INCLUDED
