// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Arena.hpp>
#include <cminor/machine/Thread.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/OsInterface.hpp>
#include <cminor/machine/Log.hpp>
#include <cminor/machine/Runtime.hpp>

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

uint8_t numAllocationContextPages = defaultNumAllocationContextPages;

MACHINE_API void SetNumAllocationContextPages(uint8_t numPages)
{
    numAllocationContextPages = numPages;
}

MACHINE_API uint8_t GetNumAllocationContextPages()
{
    return numAllocationContextPages;
}

Segment::Segment(int32_t id_, ArenaId arenaId_, uint64_t pageSize_, uint64_t size_) : 
    id(id_), arenaId(arenaId_), pageSize(pageSize_), size(size_), base(ReserveMemory(size)), commit(base), top(base), free(base), end(base + size), mtx('S')
{
}

Segment::~Segment()
{
    FreeMemory(base, size);
}

bool Segment::Allocate(uint64_t blockSize, void*& ptr)
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
            top = commit;
        }
        else
        {
            return false;
        }
    }
    else if (free + blockSize > top)
    {
        uint64_t allocSize = pageSize * ((blockSize - 1) / pageSize + 1);
        if (top + allocSize <= commit)
        {
            top += allocSize;
        }
        else
        {
            return false;
        }
    }
    if (free + blockSize <= top)
    {
        ptr = free;
        free += blockSize;
        return true;
    }
    else
    {
        return false;
    }
}

bool Segment::Allocate(Thread& thread, uint64_t blockSize, void*& ptr, bool requestFullCollection, std::unique_lock<std::recursive_mutex>& allocationLock)
{
    LockGuard segmentLock(mtx, thread.Owner());
    AllocationContext* allocationContext = nullptr;
    if (arenaId == ArenaId::gen1Arena && numAllocationContextPages > 0)
    {
        allocationContext = thread.GetAllocationContext();
    }
    if (free + blockSize > commit)
    {
        uint64_t commitSize = pageSize * ((blockSize - 1) / pageSize + 1);
        if (allocationContext)
        {
            commitSize += pageSize * numAllocationContextPages;
        }
        if (commit + commitSize <= end)
        {
            uint8_t* commitBase = CommitMemory(commit, commitSize);
            std::memset(commitBase, 0, commitSize);
            commit = commitBase + commitSize;
            top = commit;
        }
        else
        {
            if (allocationContext)
            {
                allocationContext->Clear();
            }
            segmentLock.Unlock();
            if (allocationLock.owns_lock())
            {
                allocationLock.unlock();
            }
            thread.RequestGc(requestFullCollection);
            return false;
        }
    }
    else if (free + blockSize > top)
    {
        uint64_t allocSize = pageSize * ((blockSize - 1) / pageSize + 1);
        if (allocationContext)
        {
            allocSize += pageSize * numAllocationContextPages;
        }
        if (top + allocSize <= commit)
        {
            top += allocSize;
        }
        else
        {
            if (allocationContext)
            {
                allocationContext->Clear();
            }
            segmentLock.Unlock();
            if (allocationLock.owns_lock())
            {
                allocationLock.unlock();
            }
            thread.RequestGc(requestFullCollection);
            return false;
        }
    }
    if (free + blockSize <= top)
    {
        ptr = free;
        free += blockSize;
        if (allocationContext)
        {
            allocationContext->SetSegmentId(id);
            allocationContext->SetFree(free);
            allocationContext->SetTop(top);
            free = top;
        }
        return true;
    }
    else
    {
        if (allocationContext)
        {
            allocationContext->Clear();
        }
        return false;
    }
}

void Segment::Clear()
{
    LockGuard lock(garbageCollectorMutex, gc);
    free = base;
    top = base;
    uint64_t n = commit - base;
    std::memset(base, 0, n);
}

AllocationContext::AllocationContext() : segmentId(-1), free(nullptr), top(nullptr)
{
}

Arena::Arena(Machine& machine_, ArenaId id_, uint64_t segmentSize_) : machine(machine_), id(id_), pageSize(GetSystemPageSize()), segmentSize(segmentSize_), segmentsMutex('A')
{
    Segment* segment = new Segment(machine.GetNextSegmentId(), id, pageSize, segmentSize);
    machine.AddSegment(segment);
    segments.push_back(std::unique_ptr<Segment>(segment));
}

void Arena::Allocate(uint64_t blockSize, void*& ptr, int32_t& segmentId)
{
    return Allocate(blockSize, ptr, segmentId, false);
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

void GenArena1::Allocate(uint64_t blockSize, void*& ptr, int32_t& segmentId, bool allocateNewSegment)
{
    throw std::runtime_error("could not allocate " + std::to_string(blockSize) + " bytes memory from arena " + std::to_string(int(Id())) + ": arena 1 needs a thread");
}

void GenArena1::Allocate(Thread& thread, uint64_t blockSize, void*& ptr, int32_t& segmentId, std::unique_lock<std::recursive_mutex>& allocationLock)
{
    if (blockSize == 0)
    {
        throw std::runtime_error("invalid allocation request of zero bytes from arena " + std::to_string(int(Id())));
    }
    else
    {
        Segment* segment = Segments().back().get();
        while (!segment->Allocate(thread, blockSize, ptr, false, allocationLock))
        {
            if (allocationLock.owns_lock())
            {
                allocationLock.unlock();
            }
            thread.WaitUntilGarbageCollected();
        }
        segmentId = segment->Id();
    }
}

GenArena2::GenArena2(Machine& machine_, uint64_t size_) : Arena(machine_, ArenaId::gen2Arena, size_)
{
}

void GenArena2::Allocate(uint64_t blockSize, void*& ptr, int32_t& segmentId, bool allocateNewSegment)
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
        if (seg->Allocate(blockSize, ptr))
        {
            segmentId = seg->Id();
        }
        else
        {
            throw std::runtime_error("could not allocate " + std::to_string(blockSize) + " bytes memory from arena " + std::to_string(int(Id())));
        }
    }
    else
    {
        if (!allocateNewSegment)
        {
            Segment* seg = Segments().back().get();
            if (seg->Allocate(blockSize, ptr))
            {
                segmentId = seg->Id();
                return;
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
        if (seg->Allocate(blockSize, ptr))
        {
            segmentId = seg->Id();
        }
        else
        {
            throw std::runtime_error("could not allocate " + std::to_string(blockSize) + " bytes memory from arena " + std::to_string(int(Id())));
        }
    }
}

void GenArena2::Allocate(Thread& thread, uint64_t blockSize, void*& ptr, int32_t& segmentId, std::unique_lock<std::recursive_mutex>& allocationLock)
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
        if (seg->Allocate(thread, blockSize, ptr, true, allocationLock))
        {
            segmentId = seg->Id();
        }
        else
        {
            throw std::runtime_error("could not allocate " + std::to_string(blockSize) + " bytes memory from arena " + std::to_string(int(Id())));
        }
    }
    else
    {
        Segment* segment = Segments().back().get();
        if (!segment->Allocate(thread, blockSize, ptr, true, allocationLock))
        {
			Segment* seg = new Segment(GetMachine().GetNextSegmentId(), ArenaId::gen2Arena, PageSize(), SegmentSize());
			std::unique_ptr<Segment> segment(seg);
			{
				LockGuard lock(SegmentsMutex(), gc);
				GetMachine().AddSegment(seg);
				Segments().push_back(std::move(segment));
			}
			if (seg->Allocate(blockSize, ptr))
			{
				segmentId = seg->Id();
			}
			else
			{
				throw std::runtime_error("could not allocate " + std::to_string(blockSize) + " bytes memory from arena " + std::to_string(int(Id())));
			}
        }
        segmentId = segment->Id();
    }
}

} } // namespace cminor::machine
