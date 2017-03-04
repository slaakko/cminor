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
    id(id_), arenaId(arenaId_), pageSize(pageSize_), size(size_), mem(ReserveMemory(size)), commit(mem), free(mem), end(mem + size)
{
}

Segment::~Segment()
{
    FreeMemory(mem, size);
}

MemPtr Segment::Allocate(uint64_t blockSize)
{
    std::lock_guard<std::mutex> lock(mtx);
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
}

Arena::Arena(Machine& machine_, ArenaId id_, uint64_t segmentSize_) : machine(machine_), id(id_), pageSize(GetSystemPageSize()), segmentSize(segmentSize_)
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

struct Resetter
{
    Resetter(std::atomic_bool& toReset_) : toReset(toReset_) {}
    ~Resetter() { toReset = false; }
    std::atomic_bool& toReset;
};

std::pair<MemPtr, int32_t> GenArena1::Allocate(Thread& thread, uint64_t blockSize)
{
    bool allocating = thread.GetMachine().ThreadAllocating().exchange(true);
    if (allocating)
    {
        if (RunningNativeCode())
        {
            thread.SetFunctionStack(RtGetFunctionStack());
        }
        thread.SetState(ThreadState::paused);
        while (thread.GetMachine().ThreadAllocating())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
        }
        thread.SetState(ThreadState::running);
        thread.GetMachine().GetGarbageCollector().WaitForIdle(thread);
    }
    Resetter resetter(thread.GetMachine().ThreadAllocating());
    if (blockSize == 0)
    {
        throw std::runtime_error("invalid allocation request of zero bytes from arena " + std::to_string(int(Id())));
    }
    else
    {
        MemPtr mem = Segments().back()->Allocate(blockSize);
        if (mem.Value())
        {
            return std::make_pair(mem, Segments().back()->Id());
        }
        else
        {
            if (RunningNativeCode())
            {
                thread.SetFunctionStack(RtGetFunctionStack());
            }
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
            mem = Segments().back()->Allocate(blockSize);
            if (mem.Value())
            {
                return std::make_pair(mem, Segments().back()->Id());
            }
            else
            {
                throw std::runtime_error("could not allocate " + std::to_string(blockSize) + " bytes memory from arena " + std::to_string(int(Id())));
            }
        }
    }
}

GenArena2::GenArena2(Machine& machine_, uint64_t size_) : Arena(machine_, ArenaId::gen2Arena, size_)
{
}

std::pair<MemPtr, int32_t>  GenArena2::Allocate(uint64_t blockSize, bool allocateNewSegment)
{
    if (blockSize > SegmentSize())
    {
        GetMachine().GetGarbageCollector().RequestFullCollection();
        Segment* seg = new Segment(GetMachine().GetNextSegmentId(), ArenaId::gen2Arena, PageSize(), blockSize);
        GetMachine().AddSegment(seg);
        std::unique_ptr<Segment> segment(seg);
        Segments().push_back(std::move(segment));
        MemPtr mem = seg->Allocate(blockSize);
        if (mem.Value())
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
            mem = Segments().back()->Allocate(blockSize);
        }
        if (mem.Value())
        {
            return std::make_pair(mem, Segments().back()->Id());
        }
        else
        {
            GetMachine().GetGarbageCollector().RequestFullCollection();
            Segment* seg = new Segment(GetMachine().GetNextSegmentId(), ArenaId::gen2Arena, PageSize(), SegmentSize());
            GetMachine().AddSegment(seg);
            std::unique_ptr<Segment> segment(seg);
            Segments().push_back(std::move(segment));
            MemPtr mem = seg->Allocate(blockSize);
            if (mem.Value())
            {
                return std::make_pair(mem, seg->Id());
            }
            else
            {
                throw std::runtime_error("could not allocate " + std::to_string(blockSize) + " bytes memory from arena " + std::to_string(int(Id())));
            }
        }
    }
}

std::pair<MemPtr, int32_t> GenArena2::Allocate(Thread& thread, uint64_t blockSize)
{
    bool allocating = thread.GetMachine().ThreadAllocating().exchange(true);
    if (allocating)
    {
        if (RunningNativeCode())
        {
            thread.SetFunctionStack(RtGetFunctionStack());
        }
        thread.SetState(ThreadState::paused);
        while (thread.GetMachine().ThreadAllocating())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
        }
        thread.SetState(ThreadState::running);
        thread.GetMachine().GetGarbageCollector().WaitForIdle(thread);
    }
    Resetter resetter(thread.GetMachine().ThreadAllocating());
    if (blockSize > SegmentSize())
    {
        Segment* seg = new Segment(GetMachine().GetNextSegmentId(), ArenaId::gen2Arena, PageSize(), blockSize);
        GetMachine().AddSegment(seg);
        std::unique_ptr<Segment> segment(seg);
        Segments().push_back(std::move(segment));
        MemPtr mem = seg->Allocate(blockSize);
        if (mem.Value())
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
        MemPtr mem = Segments().back()->Allocate(blockSize);
        if (mem.Value())
        {
            return std::make_pair(mem, Segments().back()->Id());
        }
        else
        {
            if (RunningNativeCode())
            {
                thread.SetFunctionStack(RtGetFunctionStack());
            }
            thread.SetState(ThreadState::paused);
#ifdef GC_LOGGING
            LogMessage(">" + std::to_string(thread.Id()) + " (paused)");
#endif
            GetMachine().GetGarbageCollector().RequestFullCollection(thread);
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
            mem = Segments().back()->Allocate(blockSize);
            if (mem.Value())
            {
                return std::make_pair(mem, Segments().back()->Id());
            }
            else
            {
                Segment* seg = new Segment(GetMachine().GetNextSegmentId(), ArenaId::gen2Arena, PageSize(), SegmentSize());
                GetMachine().AddSegment(seg);
                std::unique_ptr<Segment> segment(seg);
                Segments().push_back(std::move(segment));
                MemPtr mem = seg->Allocate(blockSize);
                if (mem.Value())
                {
                    return std::make_pair(mem, seg->Id());
                }
                else
                {
                    throw std::runtime_error("could not allocate " + std::to_string(blockSize) + " bytes memory from arena " + std::to_string(int(Id())));
                }
            }
        }
    }
}

} } // namespace cminor::machine
