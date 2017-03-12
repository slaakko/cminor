// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_ARENA_INCLUDED
#define CMINOR_MACHINE_ARENA_INCLUDED
#include <cminor/machine/MachineApi.hpp>
#include <cminor/machine/Object.hpp>
#include <atomic>
#include <mutex>

namespace cminor { namespace machine {

class Thread;
struct FunctionStackEntry;

MACHINE_API void SetSegmentSize(uint64_t segmentSize_);
MACHINE_API uint64_t GetSegmentSize();

constexpr uint64_t defaultSegmentSize = static_cast<uint64_t>(16) * 1024 * 1024; // 16 MB
constexpr uint64_t defaultLargeObjectThresholdSize = static_cast<uint64_t>(64) * 1024; // 64 K

const int32_t notGarbageCollectedSegment = -1;

class Segment
{
public:
    Segment(int32_t id_, ArenaId arenaId_, uint64_t pageSize_, uint64_t size_);
    ~Segment();
    int32_t Id() const { return id; }
    ArenaId GetArenaId() const { return arenaId; }
    bool Allocate(uint64_t blockSize, MemPtr& mem);
    bool Allocate(Thread& thread, uint64_t blockSize, MemPtr& mem, bool requestFullCollection);
    void Clear();
private:
    int32_t id;
    ArenaId arenaId;
    uint64_t pageSize;
    uint64_t size;
    uint8_t* mem;
    uint8_t* commit;
    uint8_t* free;
    uint8_t* end;
    std::mutex mtx;
};

class Arena
{
public:
    Arena(Machine& machine_, ArenaId id_, uint64_t segmentSize_);
    Machine& GetMachine() { return machine; }
    ArenaId Id() const { return id; }
    std::pair<MemPtr, int32_t> Allocate(uint64_t blockSize);
    virtual std::pair<MemPtr, int32_t> Allocate(uint64_t blockSize, bool allocateNewSegment) = 0;
    virtual void Allocate(Thread& thread, uint64_t blockSize, MemPtr& memPtr, int32_t& segmentId) = 0;
    void Clear();
    const std::vector<std::unique_ptr<Segment>>& Segments() const { return segments; }
    std::vector<std::unique_ptr<Segment>>& Segments() { return segments; }
    uint64_t PageSize() const { return pageSize; }
    uint64_t SegmentSize() const { return segmentSize; }
    void RemoveSegment(int32_t segmentId);
    void RemoveEmptySegments(const std::unordered_set<int32_t>& liveSegments);
    void Compact();
    std::mutex& SegmentsMutex() { return segmentsMutex; }
private:
    Machine& machine;
    ArenaId id;
    uint64_t pageSize;
    uint64_t segmentSize;
    std::mutex segmentsMutex;
    std::vector<std::unique_ptr<Segment>> segments;
};

class GenArena1 : public Arena
{
public:
    GenArena1(Machine& machine_, uint64_t size_);
    std::pair<MemPtr, int32_t> Allocate(uint64_t blockSize, bool allocateNewSegment) override;
    void Allocate(Thread& thread, uint64_t blockSize, MemPtr& memPtr, int32_t& segmentId) override;
};

class GenArena2 : public Arena
{
public:
    GenArena2(Machine& machine_, uint64_t size_);
    std::pair<MemPtr, int32_t> Allocate(uint64_t blockSize, bool allocateNewSegment) override;
    void Allocate(Thread& thread, uint64_t blockSize, MemPtr& memPtr, int32_t& segmentId) override;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_ARENA_INCLUDED
