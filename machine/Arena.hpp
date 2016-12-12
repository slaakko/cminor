// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_ARENA_INCLUDED
#define CMINOR_MACHINE_ARENA_INCLUDED
#include <cminor/machine/Object.hpp>
#include <atomic>
#include <mutex>

namespace cminor { namespace machine {

class Thread;

void SetSegmentSize(uint64_t segmentSize_);
uint64_t GetSegmentSize();

const uint64_t defaultSegmentSize = static_cast<uint64_t>(16) * 1024 * 1024; // 16 MB
const uint64_t defaultLargeObjectThresholdSize = static_cast<uint64_t>(64) * 1024; // 64 K

class Segment
{
public:
    Segment(ArenaId arenaId_, uint64_t pageSize_, uint64_t size_);
    ~Segment();
    MemPtr Allocate(uint64_t blockSize);
    MemPtr Allocate(Thread& thread, uint64_t blockSize);
    void Clear();
private:
    ArenaId arenaId;
    uint64_t pageSize;
    uint64_t size;
    uint8_t* mem;
    uint8_t* commit;
    uint8_t* free;
    uint8_t* end;
    std::mutex mtx;
    bool cleared;
    std::condition_variable clearedCond;
};

class Arena
{
public:
    Arena(ArenaId id_, uint64_t segmentSize_);
    ArenaId Id() const { return id; }
    virtual MemPtr Allocate(uint64_t blockSize) = 0;
    virtual MemPtr Allocate(Thread& thread, uint64_t blockSize) = 0;
    void Clear();
    const std::vector<std::unique_ptr<Segment>>& Segments() const { return segments; }
    std::vector<std::unique_ptr<Segment>>& Segments() { return segments; }
    uint64_t PageSize() const { return pageSize; }
    uint64_t SegmentSize() const { return segmentSize; }
private:
    ArenaId id;
    uint64_t pageSize;
    uint64_t segmentSize;
    std::vector<std::unique_ptr<Segment>> segments;
};

class GenArena1 : public Arena
{
public:
    GenArena1(uint64_t size_);
    MemPtr Allocate(uint64_t blockSize);
    MemPtr Allocate(Thread& thread, uint64_t blockSize);
private:
    std::mutex mtx;
};

class GenArena2 : public Arena
{
public:
    GenArena2(uint64_t size_);
    MemPtr Allocate(uint64_t blockSize);
    MemPtr Allocate(Thread& thread, uint64_t blockSize);
private:
    std::mutex mtx;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_ARENA_INCLUDED
