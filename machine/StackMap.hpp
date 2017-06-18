// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_STACK_MAP_INCLUDED
#define CMINOR_MACHINE_STACK_MAP_INCLUDED
#include <cminor/machine/MachineApi.hpp>
#include <cminor/machine/Writer.hpp>
#include <cminor/machine/Reader.hpp>
#include <cminor/util/CodeFormatter.hpp>
#include <vector>
#include <memory>
#include <unordered_map>
#include <stdint.h>

namespace cminor { namespace machine {

using namespace cminor::util;

enum class LocationType : uint8_t
{
    none = 0,
    reg = 1,        // value in register
    direct = 2,     // value in frame register + offset
    indirect = 3,   // value spilled to [register + offset] 
    constant = 4,   // value in offset (small constant)
    constIndex = 5  // value in (large constants)[offset] (large constant)
};

const char* LocationTypeStr(LocationType locationType);

enum class Register : uint16_t
{
    rax = 0, rdx = 1, rcx = 2, rbx = 3, rsi = 4, rdi = 5, rbp = 6, rsp = 7, 
    r8 = 8, r9 = 9, r10 = 10, r11 = 11, r12 = 12, r13 = 13, r14 = 14, r15 = 15, 
    returnAddress = 16, 
    xmm0 = 17, xmm1 = 18, xmm2 = 19, xmm3 = 20, xmm4 = 21, xmm5 = 22, xmm6 = 23, xmm7 = 24, 
    xmm8 = 25, xmm9 = 26, xmm10 = 27, xmm11 = 28, xmm12 = 29, xmm13 = 30, xmm14 = 31, xmm15 = 32, 
    st0 = 33, st1 = 34, st2 = 35, st3 = 36, st4 = 37, st5 = 38, st6 = 39, st7 = 40,
    mm0 = 41, mm1 = 42, mm2 = 43, mm3 = 44, mm4 = 45, mm5 = 46, mm6 = 47, mm7 = 48
};

const char* RegisterStr(Register reg);

class MACHINE_API Location
{
public:
    Location();
    Location(LocationType type_, Register reg_, int32_t offset_);
    LocationType Type() const { return type; }
    Register Reg() const { return reg; }
    int32_t Offset() const { return offset; }
    void Dump(CodeFormatter& formatter);
    void Write(Writer& writer);
    void Read(Reader& reader);
private:
    LocationType type;
    Register reg;
    int32_t offset;
};

class MACHINE_API LiveOut
{
public:
    LiveOut();
    LiveOut(Register reg_, uint8_t size_);
    Register Reg() const { return reg; }
    uint8_t Size() const { return size; }
    void Dump(CodeFormatter& formatter);
    void Write(Writer& writer);
    void Read(Reader& reader);
private:
    Register reg;
    uint8_t size;
};

class MACHINE_API StackMapRecord
{
public:
    StackMapRecord();
    StackMapRecord(uint64_t id_, uint32_t instructionOffset_);
    uint64_t Id() const { return id; }
    uint32_t IstructionOffset() const { return instructionOffset; }
    const std::vector<Location>& Locations() const { return locations; }
    void AddLocation(const Location& location);
    const std::vector<LiveOut>& LiveOuts() const { return liveOuts; }
    void AddLiveOut(const LiveOut& liveOut);
    void Dump(CodeFormatter& formatter);
    void Write(Writer& writer);
    void Read(Reader& reader);
private:
    uint64_t id;
    uint32_t instructionOffset;
    std::vector<Location> locations;
    std::vector<LiveOut> liveOuts;
};

class MACHINE_API StackMapSection
{
public:
    StackMapSection();
    StackMapSection(const StackMapSection&) = delete;
    StackMapSection& operator=(const StackMapSection&) = delete;
    uint32_t NumStackMapRecords() const { return static_cast<uint32_t>(stackMapRecords.size()); }
    const StackMapRecord& GetStackMapRecord(int index) const { return *stackMapRecords[index]; }
    void AddStackMapRecord(StackMapRecord* record);
    StackMapRecord* GetStackMapRecordById(uint64_t id) const;
    void Dump(CodeFormatter& formatter);
    void Write(Writer& writer);
    void Read(Reader& reader);
private:
    std::vector<std::unique_ptr<StackMapRecord>> stackMapRecords;
    std::unordered_map<uint64_t, StackMapRecord*> recordMap;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_STACK_MAP_INCLUDED
