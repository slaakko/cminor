// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Stack.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/machine/Thread.hpp>
#include <cminor/util/Util.hpp>
#include <cminor/machine/OsInterface.hpp>

namespace cminor { namespace machine {

int maxFrames = defaultMaxFrames;
uint64_t stackCommitSize = defaultStackCommitSize;
uint64_t stackReserveSize = defaultStackReserveSize;
uint64_t stackGrowSize = defaultStackGrowSize;

int GetMaxFrames()
{
    return maxFrames;
}

void SetMaxFrames(int maxFrames_)
{
    maxFrames = maxFrames_;
}

uint64_t GetStackCommitSize()
{
    return stackCommitSize;
}

void SetStackCommitSize(uint64_t stackCommitSize_)
{
    stackCommitSize = stackCommitSize_;
}

uint64_t GetStackReserveSize()
{
    return stackReserveSize;
}

void SetStackReserveSize(uint64_t stackReserveSize_)
{
    stackReserveSize = stackReserveSize_;
}

uint64_t GetStackGrowSize()
{
    return stackGrowSize;
}

void SetStackGrowSize(uint64_t stackGrowSize_)
{
    stackGrowSize = stackGrowSize_;
}

Stack::Stack(Thread& thread_) : 
    thread(thread_), pageSize(GetSystemPageSize()), size(GetStackReserveSize()), mem(ReserveMemory(size)), free(mem), end(mem + size)
{
    uint64_t stackGrowSize = GetStackGrowSize();
    growSize = stackGrowSize * ((pageSize - 1) / stackGrowSize + 1);
    uint8_t* commitBase = CommitMemory(mem, growSize);
    commit = commitBase + growSize;
    frames.reserve(GetMaxFrames());
}

Stack::~Stack()
{
    FreeMemory(mem, size);
}

void Stack::AllocateFrame(Function& fun)
{
    int numLocals = fun.NumLocals();
    size_t sizeOfLocals = numLocals * Align(sizeof(LocalVariable), 8);
    uint64_t frameSize = Align(sizeof(Frame), 8) + sizeOfLocals;
    if (free + frameSize > commit)
    {
        uint64_t commitSize = growSize * ((frameSize - 1) / growSize + 1);
        if (commit + commitSize <= end)
        {
            uint8_t* commitBase = CommitMemory(commit, commitSize);
            commit = commitBase + commitSize;
        }
        else
        {
            throw StackOverflowException();
        }
    }
    if (free + frameSize <= commit)
    {
        void* ptr = free;
        free += frameSize;
        Frame* frame = new (ptr) Frame(frameSize, thread, fun);
        std::memset(frame->Locals(), 0, sizeOfLocals);
        frames.push_back(frame);
        thread.MapFrame(frame);
    }
    else
    {
        throw StackOverflowException();
    }
}

void Stack::FreeFrame()
{
    Assert(!frames.empty(), "no frames");
    Frame* last = frames.back();
    thread.RemoveFrame(last->Id());
    uint64_t frameSize = last->Size();
    last->~Frame();
    frames.pop_back();
    free -= frameSize;
}

} } // namespace cminor::machine
