// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_MACHINE_INCLUDED
#define CMINOR_MACHINE_MACHINE_INCLUDED
#include <cminor/machine/MachineApi.hpp>
#include <cminor/machine/Instruction.hpp>
#include <cminor/machine/Arena.hpp>
#include <cminor/machine/GarbageCollector.hpp>
#include <cminor/machine/Thread.hpp>
#include <thread>

namespace cminor { namespace machine {

MACHINE_API void SetRunningNativeCode();
MACHINE_API bool RunningNativeCode();

enum class RunThreadKind
{
    function, method, functionWithParam, methodWithParam
};

template <typename T>
struct Identity
{
    const T& operator()(const T& x) const { return x; }
};

template <typename T>
struct BitNot
{
    T operator()(const T& x) const { return ~x; }
};

class MACHINE_API Machine
{
public:
    Machine();
    ~Machine();
    void Start(bool startWithArgs, const std::vector<std::u32string>& programArguments, ObjectType* argsArrayObjectType);
    void RunMain(bool runWithArgs, const std::vector<std::u32string>& programArguments, ObjectType* argsArrayObjectType);
    int StartThread(Function* fun, RunThreadKind runThreadKind, ObjectReference receiver, ObjectReference arg);
    void JoinThread(int threadId);
    void AddInst(Instruction* inst);
    std::unique_ptr<Instruction> CreateInst(const std::string& instName) const;
    std::unique_ptr<Instruction> CreateInst(const std::string& instGroupName, const std::string& typeName) const;
    std::unique_ptr<Instruction> DecodeInst(Reader& reader);
    ManagedMemoryPool& GetManagedMemoryPool() { return managedMemoryPool; }
    std::vector<std::unique_ptr<Thread>>& Threads() { return threads; }
    const std::vector<std::unique_ptr<Thread>>& Threads() const { return threads; }
    Thread& MainThread() { Assert(!threads.empty(), "no main thread"); return *threads.front().get(); }
    GarbageCollector& GetGarbageCollector() { return garbageCollector; }
    void AllocateMemory(Thread& thread, uint64_t blockSize, void*& ptr, int32_t& segmentId, std::unique_lock<std::recursive_mutex>& allocationLock);
    void RunGarbageCollector();
    Arena& Gen1Arena() { return *gen1Arena; }
    Arena& Gen2Arena() { return *gen2Arena; }
    int32_t GetNextFrameId() { return nextFrameId++; }
    int32_t GetNextSegmentId();
    bool Exiting();
    void Exit();
    void AddSegment(Segment* segment);
    void RemoveSegment(int32_t segmentId);
    Segment* GetSegment(int32_t segmentId);
    void Compact();
private:
    ContainerInst rootInst;
    std::unordered_map<std::string, Instruction*> instructionMap;
    std::unordered_map<std::string, InstructionTypeGroup> instructionTypeGroupMap;
    ManagedMemoryPool managedMemoryPool;
    std::vector<std::unique_ptr<Thread>> threads;
    std::vector<std::unique_ptr<std::thread>> userThreads;
    GarbageCollector garbageCollector;
    std::unique_ptr<GenArena1> gen1Arena;
    std::unique_ptr<GenArena2> gen2Arena;
    std::atomic<bool> exiting;
    std::atomic<bool> exited;
    std::thread garbageCollectorThread;
    Mutex threadMutex;
    MutexOwner owner;
    std::atomic<int32_t> nextFrameId;
    std::atomic<int32_t> nextSegmentId;
    std::unordered_map<int32_t, Segment*> segmentMap;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_MACHINE_INCLUDED
