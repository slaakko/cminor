// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/StackMap.hpp>
#include <cminor/util/TextUtils.hpp>

namespace cminor { namespace machine {

const char* LocationTypeStr(LocationType locationType)
{
    static const char* types[] =
    {
        "", "register", "direct", "indirect", "constant", "constindex"
    };
    return types[static_cast<int>(locationType)];
}

const char* RegisterStr(Register reg)
{
    static const char* regs[] =
    {
        "RAX", "RDX", "RCX", "RBX", "RSI", "RDI", "RBP", "RSP",
        "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15",
        "RA",
        "XMM0", "XMM1", "XMM2", "XMM3", "XMM4", "XMM5", "XMM6", "XMM7",
        "XMM8", "XMM9", "XMM10", "XMM11", "XMM12", "XMM13", "XMM14", "XMM15",
        "ST0", "ST1", "ST2", "ST3", "ST4", "ST5", "ST6", "ST7",
        "MM0", "MM1", "MM2", "MM3", "MM4", "MM5", "MM6", "MM7"
    };
    return regs[static_cast<int>(reg)];
}

Location::Location() : type(LocationType::none), reg(Register::rax), offset(0)
{

}

Location::Location(LocationType type_, Register reg_, int32_t offset_) : type(type_), reg(reg_), offset(offset_)
{
}

void Location::Dump(CodeFormatter& formatter)
{
    formatter.WriteLine("location:");
    formatter.WriteLine("type: " + std::string(LocationTypeStr(type)));
    formatter.WriteLine("reg: " + std::string(RegisterStr(reg)));
    formatter.WriteLine("offset: " + std::to_string(offset));
}

void Location::Write(Writer& writer)
{
    writer.Put(static_cast<uint8_t>(type));
    writer.Put(static_cast<uint16_t>(reg));
    writer.Put(offset);
}

void Location::Read(Reader& reader)
{
    type = static_cast<LocationType>(reader.GetByte());
    reg = static_cast<Register>(reader.GetUShort());
    offset = reader.GetInt();
}

LiveOut::LiveOut() : reg(Register::rax), size(0)
{
}

LiveOut::LiveOut(Register reg_, uint8_t size_) : reg(reg_), size(size_)
{
}

void LiveOut::Dump(CodeFormatter& formatter)
{
    formatter.WriteLine("live out:");
    formatter.WriteLine("reg: " + std::string(RegisterStr(reg)));
    formatter.WriteLine("size: " + std::to_string(static_cast<int>(size)));
}

void LiveOut::Write(Writer& writer)
{
    writer.Put(static_cast<uint16_t>(reg));
    writer.Put(size);
}

void LiveOut::Read(Reader& reader)
{
    reg = static_cast<Register>(reader.GetUShort());
    size = reader.GetByte();
}

StackMapRecord::StackMapRecord() : id(0), instructionOffset(0), locations(), liveOuts()
{
}

StackMapRecord::StackMapRecord(uint64_t id_, uint32_t instructionOffset_) : id(id_), instructionOffset(instructionOffset_), locations(), liveOuts()
{
}

void StackMapRecord::AddLocation(const Location& location)
{
    locations.push_back(location);
}

void StackMapRecord::AddLiveOut(const LiveOut& liveOut)
{
    liveOuts.push_back(liveOut);
}

void StackMapRecord::Dump(CodeFormatter& formatter)
{
    formatter.WriteLine("stack map record:");
    formatter.WriteLine("id: " + ToHexString(id));
    formatter.WriteLine("instruction offset: " + ToHexString(instructionOffset));
    formatter.WriteLine("locations:");
    formatter.IncIndent();
    for (Location& location : locations)
    {
        location.Dump(formatter);
    }
    formatter.DecIndent();
    formatter.WriteLine("live outs:");
    formatter.IncIndent();
    for (LiveOut& liveOut : liveOuts)
    {
        liveOut.Dump(formatter);
    }
    formatter.DecIndent();
}

void StackMapRecord::Write(Writer& writer)
{
    writer.Put(id);
    writer.Put(instructionOffset);
    uint16_t numLocations = static_cast<uint16_t>(locations.size());
    writer.Put(numLocations);
    for (Location& location : locations)
    {
        location.Write(writer);
    }
    uint16_t numLiveOuts = static_cast<uint16_t>(liveOuts.size());
    writer.Put(numLiveOuts);
    for (LiveOut& liveOut : liveOuts)
    {
        liveOut.Write(writer);
    }
}

void StackMapRecord::Read(Reader& reader)
{
    id = reader.GetULong();
    instructionOffset = reader.GetUInt();
    uint16_t numLocations = reader.GetUShort();
    for (uint16_t i = 0; i < numLocations; ++i)
    {
        Location location;
        location.Read(reader);
        locations.push_back(location);
    }
    uint16_t numLiveOuts = reader.GetUShort();
    for (uint16_t i = 0; i < numLiveOuts; ++i)
    {
        LiveOut liveOut;
        liveOut.Read(reader);
        liveOuts.push_back(liveOut);
    }
}

StackMapSection::StackMapSection()
{
}

void StackMapSection::AddStackMapRecord(StackMapRecord* record)
{
    stackMapRecords.push_back(std::unique_ptr<StackMapRecord>(record));
}

StackMapRecord* StackMapSection::GetStackMapRecordById(uint64_t id) const
{
    auto it = recordMap.find(id);
    if (it != recordMap.cend())
    {
        return it->second;
    }
    else
    {
        return nullptr;
    }
}

void StackMapSection::Dump(CodeFormatter& formatter)
{
    formatter.WriteLine("stack map records:");
    for (std::unique_ptr<StackMapRecord>& record : stackMapRecords)
    {
        record->Dump(formatter);
    }
}

void StackMapSection::Write(Writer& writer)
{
    uint32_t numStackMapRecords = static_cast<uint32_t>(stackMapRecords.size());
    writer.Put(numStackMapRecords);
    for (std::unique_ptr<StackMapRecord>& record : stackMapRecords)
    {
        record->Write(writer);
    }
}

void StackMapSection::Read(Reader& reader)
{
    uint32_t numStackMapRecords = reader.GetUInt();
    for (uint32_t i = 0; i < numStackMapRecords; ++i)
    {
        StackMapRecord* record = new StackMapRecord();
        record->Read(reader);
        stackMapRecords.push_back(std::unique_ptr<StackMapRecord>(record));
        recordMap[record->Id()] = record;
    }
}

} } // namespace cminor::machine
