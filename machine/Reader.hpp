// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_READER_INCLUDED
#define CMINOR_MACHINE_READER_INCLUDED
#include <cminor/machine/MappedInputFile.hpp>

namespace cminor { namespace machine {

class Machine;

class Reader
{
public:
    Reader(Machine& machine_, const std::string& fileName_);
    Machine& GetMachine() { return machine; }
    uint8_t GetByte();
    int8_t GetSByte();
    uint16_t GetUShort();
    int16_t GetShort();
    uint32_t GetUInt();
    int32_t GetInt();
    uint64_t GetULong();
    int64_t GetLong();
    float GetFloat();
    double GetDouble();
    std::string GetString();
private:
    Machine& machine;
    std::string fileName;
    MappedInputFile file;
    const uint8_t* begin;
    const uint8_t* end;
    void CheckEof();
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_READER_INCLUDED
