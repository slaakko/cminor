// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_ARENA_INCLUDED
#define CMINOR_MACHINE_ARENA_INCLUDED
#include <cminor/machine/MachineApi.hpp>
#include <cminor/machine/Object.hpp>
#include <cminor/util/Mutex.hpp>
#include <atomic>

namespace cminor { namespace machine {

class Thread;
struct FunctionStackEntry;

MACHINE_API void SetSegmentSize(uint64_t segmentSize_);
MACHINE_API uint64_t GetSegmentSize();

constexpr uint64_t defaultSegmentSize = static_cast<uint64_t>(16) * 1024 * 1024; // 16 MB
constexpr uint64_t defaultLargeObjectThresholdSize = static_cast<uint64_t>(72) * 1024; // 64 K + 8 K

const int32_t notGarbageCollectedSegment = -1;

class AllocationContext;

class Segment
{
public:
    Segment(int32_t id_, ArenaId arenaId_, uint64_t pageSize_, uint64_t size_);
    ~Segment();
    int32_t Id() const { return id; }
    ArenaId GetArenaId() const { return arenaId; }
    bool Allocate(uint64_t blockSize, void*& ptr);
    bool Allocate(Thread& thread, uint64_t blockSize, void*& ptr, bool requestFullCollection, std::unique_lock<std::recursive_mutex>& allocationLock);
    void Clear();
private:
    int32_t id;
    ArenaId arenaId;
    uint64_t pageSize;
    uint64_t size;
    uint8_t* base;
    uint8_t* commit;
    uint8_t* free;
    uint8_t* top;
    uint8_t* end;
    Mutex mtx;
};

const uint8_t defaultNumAllocationContextPages = 2;
MACHINE_API void SetNumAllocationContextPages(uint8_t numPages);
MACHINE_API uint8_t GetNumAllocationContextPages();

class AllocationContext
{
public:
    AllocationContext();
    void SetSegmentId(int32_t segmentId_) { segmentId = segmentId_; }
    void SetFree(uint8_t* free_) { free = free_; }
    void SetTop(uint8_t* top_) { top = top_; }
    bool Allocate(uint64_t blockSize, void*& ptr, int32_t& segId)
    {
        if (!free) return false;
        if (!top) return false;
        if (free + blockSize <= top)
        {
            ptr = free;
            free += blockSize;
            segId = segmentId;
            return true;
        }
        return false;
    }
    void Clear()
    {
        segmentId = -1;
        free = nullptr;
        top = nullptr;
    }
private:
    int32_t segmentId;
    uint8_t* free;
    uint8_t* top;
};

class Arena
{
public:
    Arena(Machine& machine_, ArenaId id_, uint64_t segmentSize_);
    Machine& GetMachine() { return machine; }
    ArenaId Id() const { return id; }
    void Allocate(uint64_t blockSize, void*& ptr, int32_t& segmentId);
    virtual void Allocate(uint64_t blockSize, void*& ptr, int32_t& segmentId, bool allocateNewSegment) = 0;
    virtual void Allocate(Thread& thread, uint64_t blockSize, void*& ptr, int32_t& segmentId, std::unique_lock<std::recursive_mutex>& allocationLock) = 0;
    void Clear();
    const std::vector<std::unique_ptr<Segment>>& Segments() const { return segments; }
    std::vector<std::unique_ptr<Segment>>& Segments() { return segments; }
    uint64_t PageSize() const { return pageSize; }
    uint64_t SegmentSize() const { return segmentSize; }
    void RemoveSegment(int32_t segmentId);
    void RemoveEmptySegments(const std::unordered_set<int32_t>& liveSegments);
    void Compact();
    Mutex& SegmentsMutex() { return segmentsMutex; }
private:
    Machine& machine;
    ArenaId id;
    uint64_t pageSize;
    uint64_t segmentSize;
    Mutex segmentsMutex;
    std::vector<std::unique_ptr<Segment>> segments;
};

class GenArena1 : public Arena
{
public:
    GenArena1(Machine& machine_, uint64_t size_);
    void Allocate(uint64_t blockSize, void*& ptr, int32_t& segmentId, bool allocateNewSegment) override;
    void Allocate(Thread& thread, uint64_t blockSize, void*& ptr, int32_t& segmentId, std::unique_lock<std::recursive_mutex>& allocationLock) override;
};

class GenArena2 : public Arena
{
public:
    GenArena2(Machine& machine_, uint64_t size_);
    void Allocate(uint64_t blockSize, void*& ptr, int32_t& segmentId, bool allocateNewSegment) override;
    void Allocate(Thread& thread, uint64_t blockSize, void*& ptr, int32_t& segmentId, std::unique_lock<std::recursive_mutex>& allocationLock) override;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_ARENA_INCLUDED
