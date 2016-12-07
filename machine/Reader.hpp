// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_READER_INCLUDED
#define CMINOR_MACHINE_READER_INCLUDED
#include <cminor/machine/MappedInputFile.hpp>
#include <cminor/machine/Error.hpp>

namespace cminor { namespace machine {

class Machine;
class CallInst;
class TypeInstruction;
class SetClassDataInst;
class ConstantPool;
typedef std::basic_string<char32_t> utf32_string;

class Reader
{
public:
    Reader(const std::string& filePath_);
    virtual ~Reader();
    const std::string& FilePath() const { return filePath; }
    Machine& GetMachine() { return *machine; }
    void SetMachine(Machine& machine_) { machine = &machine_; }
    ConstantPool* GetConstantPool() const { return constantPool; }
    void SetConstantPool(ConstantPool* constantPool_) { constantPool = constantPool_; }
    void AddCallInst(CallInst* callInst);
    void AddTypeInstruction(TypeInstruction* typeInst);
    void AddSetClassDataInst(SetClassDataInst* setClassDataInst);
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
    uint32_t GetEncodedUInt();
    std::vector<CallInst*> GetCallInstructions() { return std::move(callInstructions); }
    const std::vector<CallInst*>& CallInstructions() const { return callInstructions; }
    std::vector<TypeInstruction*> GetTypeInstructions() { return std::move(typeInstructions); }
    const std::vector<TypeInstruction*>& TypeInstructions() const { return typeInstructions; }
    std::vector<SetClassDataInst*> GetSetClassDataInstructions() { return std::move(setClassDataInstructions); }
    const std::vector<SetClassDataInst*>& SetClassDataInstructions() const { return setClassDataInstructions; }
    uint32_t Pos() const { return pos; }
    void Skip(uint32_t size);
private:
    Machine* machine;
    std::string filePath;
    MappedInputFile file;
    const uint8_t* begin;
    const uint8_t* end;
    ConstantPool* constantPool;
    uint32_t pos;
    std::vector<CallInst*> callInstructions;
    std::vector<TypeInstruction*> typeInstructions;
    std::vector<SetClassDataInst*> setClassDataInstructions;
    void CheckEof();
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_READER_INCLUDED
