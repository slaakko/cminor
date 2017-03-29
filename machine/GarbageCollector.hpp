// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_GARBAGE_COLLECTOR_INCLUDED
#define CMINOR_MACHINE_GARBAGE_COLLECTOR_INCLUDED
#include <cminor/machine/MachineApi.hpp>
#include <cminor/machine/Object.hpp>
#include <cminor/util/Mutex.hpp>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <ratio>

namespace cminor { namespace machine {

extern MutexOwner gc;

extern Mutex garbageCollectorMutex;

class Machine;

enum class GarbageCollectorState
{
    idle, requested, collecting, collected
};

extern std::atomic_bool wantToCollectGarbage;

class MACHINE_API GarbageCollector
{
public:
    GarbageCollector(Machine& machine_);
    void WaitForIdle(Thread& thread);
    void RequestGarbageCollection(Thread& thread);
    void RequestFullCollection(Thread& thread);
    void RequestFullCollection();
    void WaitUntilGarbageCollected(Thread& thread);
    void Run();
    bool Started() const { return started; }
    bool Error() const { return error; }
    std::exception_ptr Exception() const { return exception; }
    void SetException(std::exception_ptr exception_) { exception = exception_; error = true; }
    std::condition_variable& WantToCollectGarbageCond() { return wantToCollectGarbageCond; }
	bool PrintActions() const { return printActions; }
	void SetPrintActions() { printActions = true; }
private:
    Machine& machine;
    std::atomic<GarbageCollectorState> state;
    std::atomic_bool collectionRequested;
    std::condition_variable collectionRequestedCond;
    std::condition_variable wantToCollectGarbageCond;
    std::atomic_bool idle;
    std::condition_variable idleCond;
    std::atomic_bool collected;
    std::condition_variable collectedCond;
    bool started;
    bool error;
    std::exception_ptr exception;
	bool printActions;
    bool fullCollectionRequested;
    void SetState(GarbageCollectorState state_);
    void WaitForGarbageCollection();
    void WaitForThreadsPaused();
    void WaitForThreadsRunning();
    void CollectGarbage();
    void MarkLiveAllocations(ObjectReference objectReference, std::unordered_set<AllocationHandle, AllocationHandleHash>& checked);
    void MarkLiveAllocations();
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_GARBAGE_COLLECTOR_INCLUDED
