// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_MACHINE_INCLUDED
#define CMINOR_MACHINE_MACHINE_INCLUDED
#include <cminor/machine/Instruction.hpp>
#include <cminor/machine/Arena.hpp>
#include <cminor/machine/GarbageCollector.hpp>
#include <cminor/machine/Thread.hpp>

namespace cminor { namespace machine {

class Machine
{
public:
    Machine();
    ~Machine();
    void Start(const std::vector<utf32_string>& programArguments, ObjectType* argsArrayObjectType);
    void Run(const std::vector<utf32_string>& programArguments, ObjectType* argsArrayObjectType);
    void AddInst(Instruction* inst);
    std::unique_ptr<Instruction> CreateInst(const std::string& instName) const;
    std::unique_ptr<Instruction> CreateInst(const std::string& instGroupName, const std::string& typeName) const;
    std::unique_ptr<Instruction> DecodeInst(Reader& reader);
    ManagedMemoryPool& GetManagedMemoryPool() { return managedMemoryPool; }
    std::vector<std::unique_ptr<Thread>>& Threads() { return threads; }
    const std::vector<std::unique_ptr<Thread>>& Threads() const { return threads; }
    Thread& MainThread() { Assert(!threads.empty(), "no main thread"); return *threads.front().get(); }
    GarbageCollector& GetGarbageCollector() { return garbageCollector; }
    std::pair<ArenaId, MemPtr> AllocateMemory(Thread& thread, uint64_t blockSize);
    void RunGarbageCollector();
    Arena& Gen1Arena() { return gen1Arena; }
    Arena& Gen2Arena() { return gen2Arena; }
    int32_t GetNextFrameId();
    bool Exiting();
    void Exit();
private:
    ContainerInst rootInst;
    std::unordered_map<std::string, Instruction*> instructionMap;
    std::unordered_map<std::string, InstructionTypeGroup> instructionTypeGroupMap;
    ManagedMemoryPool managedMemoryPool;
    std::vector<std::unique_ptr<Thread>> threads;
    GarbageCollector garbageCollector;
    GenArena1 gen1Arena;
    GenArena2 gen2Arena;
    std::atomic_bool exiting;
    std::atomic_bool exited;
    std::thread garbageCollectorThread;
    std::atomic_int32_t nextFrameId;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_MACHINE_INCLUDED
