// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_READER_INCLUDED
#define CMINOR_MACHINE_READER_INCLUDED
#include <cminor/machine/MappedInputFile.hpp>

namespace cminor { namespace machine {

class Machine;
typedef std::basic_string<char32_t> utf32_string;

class Reader
{
public:
    Reader(Machine& machine_, const std::string& fileName_);
    Machine& GetMachine() { return machine; }
    bool GetBool();
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
    char32_t GetChar();
    std::string GetUtf8String();
    utf32_string GetUtf32String();
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
