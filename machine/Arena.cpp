// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Arena.hpp>
#include <cminor/machine/Thread.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/OsInterface.hpp>
#include <cminor/machine/Log.hpp>

namespace cminor { namespace machine {

uint64_t segmentSize = defaultSegmentSize;

void SetSegmentSize(uint64_t segmentSize_)
{
    segmentSize = segmentSize_;
}

uint64_t GetSegmentSize()
{
    return segmentSize;
}

Segment::Segment(ArenaId arenaId_, uint64_t pageSize_, uint64_t size_) : arenaId(arenaId_), pageSize(pageSize_), size(size_), mem(ReserveMemory(size)), commit(mem), free(mem), end(mem + size)
{
}

Segment::~Segment()
{
    FreeMemory(mem);
}

MemPtr Segment::Allocate(uint64_t blockSize)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (free + blockSize > commit)
    {
        if (free + blockSize >= end)
        {
            return MemPtr();
        }
        else
        {
            uint64_t commitSize = pageSize * ((blockSize - 1) / pageSize + 1);
            uint8_t* commitBase = CommitMemory(commit, commitSize);
            std::memset(commitBase, 0, commitSize);
            commit = commitBase + commitSize;
        }
    }
    if (free + blockSize <= commit)
    {
        MemPtr ptr(free);
        free += blockSize;
        return ptr;
    }
    else
    {
        return MemPtr();
    }
}

MemPtr Segment::Allocate(Thread& thread, uint64_t blockSize)
{
    std::lock_guard<std::mutex> lock(mtx);
    bool committed = false;
    bool freed = false;
    uint8_t* freeBefore = free;
    uint8_t* freeAfter = nullptr;
    if (free + blockSize > commit)
    {
        uint64_t commitSize = pageSize * ((blockSize - 1) / pageSize + 1);
        if (commit + commitSize <= end)
        {
            uint8_t* commitBase = CommitMemory(commit, commitSize);
            std::memset(commitBase, 0, commitSize);
            commit = commitBase + commitSize;
            committed = true;
        }
        else
        {
            return MemPtr();
        }
    }
    if (free + blockSize <= commit)
    {
        MemPtr ptr(free);
        free += blockSize;
        return ptr;
    }
    else
    {
        return MemPtr();
    }
}

void Segment::Clear()
{
    std::lock_guard<std::mutex> lock(garbageCollectorMutex);
    free = mem;
    uint64_t n = commit - mem;
    std::memset(mem, 0, n);
    cleared = true;
    clearedCond.notify_one();
}

Arena::Arena(ArenaId id_, uint64_t segmentSize_) : id(id_), pageSize(GetSystemPageSize()), segmentSize(segmentSize_)
{
    segments.push_back(std::unique_ptr<Segment>(new Segment(id, pageSize, segmentSize)));
}

void Arena::Clear()
{
    for (const std::unique_ptr<Segment>& segment : segments)
    {
        segment->Clear();
    }
}

GenArena1::GenArena1(uint64_t size_) : Arena(ArenaId::gen1Arena, size_)
{
}

MemPtr GenArena1::Allocate(uint64_t blockSize)
{
    throw std::runtime_error("could not allocate " + std::to_string(blockSize) + " bytes memory from arena " + std::to_string(int(Id())) + ": arena 1 needs a thread");
}

MemPtr GenArena1::Allocate(Thread& thread, uint64_t blockSize)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (blockSize == 0)
    {
        throw std::runtime_error("invalid allocation request of zero bytes from arena " + std::to_string(int(Id())));
    }
    else
    {
        MemPtr mem = Segments().back()->Allocate(thread, blockSize);
        if (mem.Value())
        {
            return mem;
        }
        else
        {
            thread.SetState(ThreadState::paused);
#ifdef GC_LOGGING
            LogMessage(">" + std::to_string(thread.Id()) + " (paused)");
#endif
            thread.GetMachine().GetGarbageCollector().RequestGarbageCollection(thread);
#ifdef GC_LOGGING
            LogMessage(">" + std::to_string(thread.Id()) + " (collection requested)");
#endif
            thread.GetMachine().GetGarbageCollector().WaitUntilGarbageCollected(thread);
#ifdef GC_LOGGING
            LogMessage(">" + std::to_string(thread.Id()) + " (collection ended)");
#endif
            thread.SetState(ThreadState::running);
#ifdef GC_LOGGING
            LogMessage(">" + std::to_string(thread.Id()) + " (running)");
#endif
            thread.GetMachine().GetGarbageCollector().WaitForIdle(thread);
            mem = Segments().back()->Allocate(thread, blockSize);
            if (mem.Value())
            {
                return mem;
            }
            else
            {
                throw std::runtime_error("could not allocate " + std::to_string(blockSize) + " bytes memory from arena " + std::to_string(int(Id())));
            }
        }
    }
}

GenArena2::GenArena2(uint64_t size_) : Arena(ArenaId::gen2Arena, size_)
{
}

MemPtr GenArena2::Allocate(uint64_t blockSize)
{
    if (blockSize > SegmentSize())
    {
        std::unique_ptr<Segment> segment(new Segment(ArenaId::gen2Arena, PageSize(), blockSize));
        Segment* seg = segment.get();
        Segments().push_back(std::move(segment));
        MemPtr mem = seg->Allocate(blockSize);
        if (mem.Value())
        {
            return mem;
        }
        else
        {
            throw std::runtime_error("could not allocate " + std::to_string(blockSize) + " bytes memory from arena " + std::to_string(int(Id())));
        }
    }
    else
    {
        MemPtr mem = Segments().back()->Allocate(blockSize);
        if (mem.Value())
        {
            return mem;
        }
        else
        {
            std::unique_ptr<Segment> segment(new Segment(ArenaId::gen2Arena, PageSize(), SegmentSize()));
            Segment* seg = segment.get();
            Segments().push_back(std::move(segment));
            MemPtr mem = seg->Allocate(blockSize);
            if (mem.Value())
            {
                return mem;
            }
            else
            {
                throw std::runtime_error("could not allocate " + std::to_string(blockSize) + " bytes memory from arena " + std::to_string(int(Id())));
            }
        }
        return MemPtr();
    }
}

MemPtr GenArena2::Allocate(Thread& thread, uint64_t blockSize)
{
    if (blockSize > SegmentSize())
    {
        std::unique_ptr<Segment> segment(new Segment(ArenaId::gen2Arena, PageSize(), blockSize));
        Segment* seg = segment.get();
        Segments().push_back(std::move(segment));
        MemPtr mem = seg->Allocate(blockSize);
        if (mem.Value())
        {
            return mem;
        }
        else
        {
            throw std::runtime_error("could not allocate " + std::to_string(blockSize) + " bytes memory from arena " + std::to_string(int(Id())));
        }
    }
    else
    {
        MemPtr mem = Segments().back()->Allocate(blockSize);
        if (mem.Value())
        {
            return mem;
        }
        else
        {
            std::unique_ptr<Segment> segment(new Segment(ArenaId::gen2Arena, PageSize(), SegmentSize()));
            Segment* seg = segment.get();
            Segments().push_back(std::move(segment));
            MemPtr mem = seg->Allocate(blockSize);
            if (mem.Value())
            {
                return mem;
            }
            else
            {
                throw std::runtime_error("could not allocate " + std::to_string(blockSize) + " bytes memory from arena " + std::to_string(int(Id())));
            }
        }
    }
    return MemPtr();
}

} } // namespace cminor::machine
