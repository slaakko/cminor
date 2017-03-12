// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Arena.hpp>
#include <cminor/machine/Thread.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/OsInterface.hpp>
#include <cminor/machine/Log.hpp>
#include <cminor/machine/RunTime.hpp>

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

Segment::Segment(int32_t id_, ArenaId arenaId_, uint64_t pageSize_, uint64_t size_) : 
    id(id_), arenaId(arenaId_), pageSize(pageSize_), size(size_), mem(ReserveMemory(size)), commit(mem), free(mem), end(mem + size), mtx('S')
{
}

Segment::~Segment()
{
    FreeMemory(mem, size);
}

bool Segment::Allocate(uint64_t blockSize, MemPtr& mem)
{
    LockGuard lock(mtx, gc);
    if (free + blockSize > commit)
    {
        uint64_t commitSize = pageSize * ((blockSize - 1) / pageSize + 1);
        if (commit + commitSize <= end)
        {
            uint8_t* commitBase = CommitMemory(commit, commitSize);
            std::memset(commitBase, 0, commitSize);
            commit = commitBase + commitSize;
        }
        else
        {
            return false;
        }
    }
    if (free + blockSize <= commit)
    {
        MemPtr ptr(free);
        free += blockSize;
        mem = ptr;
        return true;
    }
    else
    {
        return false;
    }
}

bool Segment::Allocate(Thread& thread, uint64_t blockSize, MemPtr& mem, bool requestFullCollection)
{
    LockGuard lock(mtx, thread.Owner());
    if (free + blockSize > commit)
    {
        uint64_t commitSize = pageSize * ((blockSize - 1) / pageSize + 1);
        if (commit + commitSize <= end)
        {
            uint8_t* commitBase = CommitMemory(commit, commitSize);
            std::memset(commitBase, 0, commitSize);
            commit = commitBase + commitSize;
        }
        else
        {
            lock.Unlock();
            thread.RequestGc(requestFullCollection);
            return false;
        }
    }
    if (free + blockSize <= commit)
    {
        MemPtr ptr(free);
        free += blockSize;
        mem = ptr;
        return true;
    }
    else
    {
        return false;
    }
}

void Segment::Clear()
{
    LockGuard lock(garbageCollectorMutex, gc);
    free = mem;
    uint64_t n = commit - mem;
    std::memset(mem, 0, n);
}

Arena::Arena(Machine& machine_, ArenaId id_, uint64_t segmentSize_) : machine(machine_), id(id_), pageSize(GetSystemPageSize()), segmentSize(segmentSize_), segmentsMutex('A')
{
    Segment* segment = new Segment(machine.GetNextSegmentId(), id, pageSize, segmentSize);
    machine.AddSegment(segment);
    segments.push_back(std::unique_ptr<Segment>(segment));
}

std::pair<MemPtr, int32_t> Arena::Allocate(uint64_t blockSize)
{
    return Allocate(blockSize, false);
}

void Arena::Clear()
{
    for (const std::unique_ptr<Segment>& segment : segments)
    {
        segment->Clear();
    }
}

void Arena::RemoveSegment(int32_t segmentId)
{
    machine.RemoveSegment(segmentId);
    auto end = segments.end();
    for (auto it = segments.begin(); it != end; ++it)
    {
        Segment* seg = it->get();
        if (seg->Id() == segmentId)
        {
            segments.erase(it);
            break;
        }
    }
}

void Arena::RemoveEmptySegments(const std::unordered_set<int32_t>& liveSegments)
{
    std::vector<int32_t> emptySegmentIds;
    for (const auto& segment : segments)
    {
        if (liveSegments.find(segment->Id()) == liveSegments.cend())
        {
            emptySegmentIds.push_back(segment->Id());
        }
    }
    for (int32_t emptySegmentId : emptySegmentIds)
    {
        RemoveSegment(emptySegmentId);
    }
}

void Arena::Compact()
{
    segments.shrink_to_fit();
}

GenArena1::GenArena1(Machine& machine_, uint64_t size_) : Arena(machine_, ArenaId::gen1Arena, size_)
{
}

std::pair<MemPtr, int32_t> GenArena1::Allocate(uint64_t blockSize, bool allocateNewSegment)
{
    throw std::runtime_error("could not allocate " + std::to_string(blockSize) + " bytes memory from arena " + std::to_string(int(Id())) + ": arena 1 needs a thread");
}

void GenArena1::Allocate(Thread& thread, uint64_t blockSize, MemPtr& memPtr, int32_t& segmentId)
{
    if (blockSize == 0)
    {
        throw std::runtime_error("invalid allocation request of zero bytes from arena " + std::to_string(int(Id())));
    }
    else
    {
        Segment* segment = Segments().back().get();
        while (!segment->Allocate(thread, blockSize, memPtr, false))
        {
            thread.WaitUntilGarbageCollected();
        }
        segmentId = segment->Id();
    }
}

GenArena2::GenArena2(Machine& machine_, uint64_t size_) : Arena(machine_, ArenaId::gen2Arena, size_)
{
}

std::pair<MemPtr, int32_t> GenArena2::Allocate(uint64_t blockSize, bool allocateNewSegment)
{
    if (blockSize > SegmentSize())
    {
        GetMachine().GetGarbageCollector().RequestFullCollection();
        Segment* seg = new Segment(GetMachine().GetNextSegmentId(), ArenaId::gen2Arena, PageSize(), blockSize);
        std::unique_ptr<Segment> segment(seg);
        {
            LockGuard lock(SegmentsMutex(), gc);
            GetMachine().AddSegment(seg);
            Segments().push_back(std::move(segment));
        }
        MemPtr mem = MemPtr();
        if (seg->Allocate(blockSize, mem))
        {
            return std::make_pair(mem, seg->Id());
        }
        else
        {
            throw std::runtime_error("could not allocate " + std::to_string(blockSize) + " bytes memory from arena " + std::to_string(int(Id())));
        }
    }
    else
    {
        MemPtr mem = MemPtr();
        if (!allocateNewSegment)
        {
            if (Segments().back()->Allocate(blockSize, mem))
            {
                return std::make_pair(mem, Segments().back()->Id());
            }
        }
        GetMachine().GetGarbageCollector().RequestFullCollection();
        Segment* seg = new Segment(GetMachine().GetNextSegmentId(), ArenaId::gen2Arena, PageSize(), SegmentSize());
        std::unique_ptr<Segment> segment(seg);
        {
            LockGuard lock(SegmentsMutex(), gc);
            GetMachine().AddSegment(seg);
            Segments().push_back(std::move(segment));
        }
        if (seg->Allocate(blockSize, mem))
        {
            return std::make_pair(mem, seg->Id());
        }
        else
        {
            throw std::runtime_error("could not allocate " + std::to_string(blockSize) + " bytes memory from arena " + std::to_string(int(Id())));
        }
    }
}

void GenArena2::Allocate(Thread& thread, uint64_t blockSize, MemPtr& memPtr, int32_t& segmentId)
{
    if (blockSize > SegmentSize())
    {
        Segment* seg = new Segment(GetMachine().GetNextSegmentId(), ArenaId::gen2Arena, PageSize(), blockSize);
        std::unique_ptr<Segment> segment(seg);
        {
            LockGuard lock(SegmentsMutex(), thread.Owner());
            GetMachine().AddSegment(seg);
            Segments().push_back(std::move(segment));
        }
        if (!seg->Allocate(thread, blockSize, memPtr, false))
        {
            throw std::runtime_error("could not allocate " + std::to_string(blockSize) + " bytes memory from arena " + std::to_string(int(Id())));
        }
    }
    else
    {
        Segment* segment = Segments().back().get();
        while (!segment->Allocate(thread, blockSize, memPtr, true))
        {
            thread.WaitUntilGarbageCollected();
        }
        segmentId = segment->Id();
    }
}

} } // namespace cminor::machine
