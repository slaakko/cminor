// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_STACK_INCLUDED
#define CMINOR_MACHINE_STACK_INCLUDED
#include <cminor/machine/Error.hpp>
#include <vector>
#include <stdexcept>
#include <stdint.h>

namespace cminor { namespace machine {

constexpr int defaultMaxFrames = 1024;
constexpr uint64_t defaultStackCommitSize = 1 * 1024 * 1024;
constexpr uint64_t defaultStackReserveSize = 16 * 1024 * 1024;
constexpr uint64_t defaultStackGrowSize = 1 * 1024 * 1024;

int GetMaxFrames();
void SetMaxFrames(int maxFrames_);
uint64_t GetStackCommitSize();
void SetStackCommitSize(uint64_t stackCommitSize_);
uint64_t GetStackReserveSize();
void SetStackReserveSize(uint64_t stackReserveSize_);
uint64_t GetStackGrowSize();
void SetStackGrowSize(uint64_t stackGrowSize_);

class StackOverflow : public std::runtime_error
{
public:
    StackOverflow();
};

class Thread;
class Function;
class Frame;

class Stack
{
public:
    Stack(Thread& thread_);
    ~Stack();
    void AllocateFrame(Function& fun);
    void FreeFrame();
    Frame* CurrentFrame() { Assert(!frames.empty(), "no frames"); return frames.back(); }
    bool IsEmpty() const { return frames.empty(); }
    const std::vector<Frame*>& Frames() const { return frames; }
    std::vector<Frame*>& Frames() { return frames; }
private:
    Thread& thread;
    std::vector<Frame*> frames;
    uint64_t pageSize;
    uint64_t size;
    uint64_t growSize;
    uint8_t* mem;
    uint8_t* commit;
    uint8_t* free;
    uint8_t* end;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_STACK_INCLUDED
