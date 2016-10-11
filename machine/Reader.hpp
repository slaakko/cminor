// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_READER_INCLUDED
#define CMINOR_MACHINE_READER_INCLUDED
#include <cminor/machine/MappedInputFile.hpp>
#include <cminor/machine/Error.hpp>

namespace cminor { namespace machine {

class Machine;
class CallInst;
class ConstantPool;
typedef std::basic_string<char32_t> utf32_string;

class Reader
{
public:
    Reader(const std::string& fileName_);
    virtual ~Reader();
    Machine& GetMachine() { return *machine; }
    void SetMachine(Machine& machine_) { machine = &machine_; }
    ConstantPool* GetConstantPool() const { return constantPool; }
    void SetConstantPool(ConstantPool* constantPool_) { constantPool = constantPool_; }
    void AddCallInst(CallInst* callInst);
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
    Span GetSpan();
    std::vector<CallInst*> GetCallInstructions() { return std::move(callInstructions); }
    const std::vector<CallInst*>& CallInstructions() const { return callInstructions; }
private:
    Machine* machine;
    std::string fileName;
    MappedInputFile file;
    const uint8_t* begin;
    const uint8_t* end;
    ConstantPool* constantPool;
    std::vector<CallInst*> callInstructions;
    void CheckEof();
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_READER_INCLUDED
