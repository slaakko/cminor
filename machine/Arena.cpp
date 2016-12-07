// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Arena.hpp>
#include <cminor/machine/Thread.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/OsInterface.hpp>

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

Arena::Arena(ArenaId id_, uint64_t size_) : id(id_), pageSize(GetSystemPageSize()), size(size_), mem(ReserveMemory(size)), commit(mem), free(mem), end(mem + size)
{
}

Arena::~Arena()
{
    FreeMemory(mem);
}

MemPtr Arena::Allocate(uint64_t blockSize)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (blockSize == 0)
    {
        throw std::runtime_error("invalid allocation request of zero bytes from arena " + std::to_string(int(id)));
    }
    else if (free + blockSize > commit)
    {
        if (commit >= end)
        {
            throw std::runtime_error("could not allocate " + std::to_string(blockSize) + " bytes memory from arena " + std::to_string(int(id)));
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
        throw std::runtime_error("could not allocate " + std::to_string(blockSize) + " bytes memory from arena " + std::to_string(int(id)));
    }
}

MemPtr Arena::Allocate(Thread& thread, uint64_t blockSize)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (blockSize == 0)
    {
        throw std::runtime_error("invalid allocation request of zero bytes from arena " + std::to_string(int(id)));
    }
    else if (free + blockSize > commit)
    {
        if (commit >= end)
        {
            thread.GetMachine().GetGarbageCollector().RequestGarbageCollection();
            thread.PauseUntilGarbageCollected();
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
        throw std::runtime_error("could not allocate " + std::to_string(blockSize) + " bytes memory from arena " + std::to_string(int(id)));
    }
}

void Arena::Clear()
{
    free = mem;
    uint64_t n = commit - mem;
    std::memset(mem, 0, n);
}

} } // namespace cminor::machine
