// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_ARENA_INCLUDED
#define CMINOR_MACHINE_ARENA_INCLUDED
#include <cminor/machine/Object.hpp>
#include <mutex>

namespace cminor { namespace machine {

class Thread;

//const uint64_t defaultArenaSize = static_cast<uint64_t>(256) * 1024 * 1024; // 256 MB
const uint64_t defaultArenaSize = static_cast<uint64_t>(4) * 1024; // 4 K
const uint64_t defaultLargeObjectThresholdSize = static_cast<uint64_t>(64) * 1024; // 64 K

class Arena
{
public:
    Arena(ArenaId id_, uint64_t size_);
    ~Arena();
    ArenaId Id() const { return id; }
    MemPtr Allocate(uint64_t blockSize);
    MemPtr Allocate(Thread& thread, uint64_t blockSize);
    void Clear();
private:
    ArenaId id;
    uint64_t pageSize;
    uint64_t size;
    uint8_t* mem;
    uint8_t* commit;
    uint8_t* free;
    uint8_t* end;
    std::mutex mtx;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_ARENA_INCLUDED
