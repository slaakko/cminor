// =================================
// Copyright (c) 2016 Seppo Laakko
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
    void AddInst(Instruction* inst);
    std::unique_ptr<Instruction> CreateInst(const std::string& instName) const;
    std::unique_ptr<Instruction> CreateInst(const std::string& instGroupName, const std::string& typeName) const;
    std::unique_ptr<Instruction> DecodeInst(Reader& reader);
    ObjectPool& GetObjectPool() { return objectPool; }
    std::vector<std::unique_ptr<Thread>>& Threads() { return threads; }
    const std::vector<std::unique_ptr<Thread>>& Threads() const { return threads; }
    Thread& MainThread() { Assert(!threads.empty(), "no main thread"); return *threads.front().get(); }
    GarbageCollector& GetGarbageCollector() { return garbageCollector; }
    std::pair<ArenaId, ObjectMemPtr> AllocateMemory(Thread& thread, uint64_t blockSize);
    void RunGarbageCollector();
    Arena& Gen1Arena() { return gen1Arena; }
    Arena& Gen2Arena() { return gen2Arena; }
    bool Exiting();
    void Exit();
private:
    ContainerInst rootInst;
    std::unordered_map<std::string, Instruction*> instructionMap;
    std::unordered_map<std::string, InstructionTypeGroup> instructionTypeGroupMap;
    ObjectPool objectPool;
    std::vector<std::unique_ptr<Thread>> threads;
    GarbageCollector garbageCollector;
    Arena gen1Arena;
    Arena gen2Arena;
    std::atomic_bool exiting;
    std::atomic_bool exited;
    std::thread garbageCollectorThread;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_MACHINE_INCLUDED
