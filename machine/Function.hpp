// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_FUNCTION_INCLUDED
#define CMINOR_MACHINE_FUNCTION_INCLUDED
#include <cminor/machine/Constant.hpp>
#include <cminor/machine/Instruction.hpp>
#include <ostream>

namespace cminor { namespace machine {

class GenObject;

class InstIndexRequest
{
public:
    InstIndexRequest();
    void SetIndex(int32_t index_) { index = index_; }
    int32_t Index() const { return index; }
private:
    int32_t index;
};

class Emitter
{
public:
    Emitter();
    virtual ~Emitter();
    virtual bool CreatePCRange() const = 0;
    virtual bool SetPCRangeEnd() const = 0;
    virtual void DoCreatePCRange(int32_t start) = 0;
    virtual void DoSetPCRangeEnd(int32_t end) = 0;
    virtual void Visit(GenObject& genObject) = 0;
    void AddIndexRequest(InstIndexRequest* request);
    void SatisfyIndexRequests(int32_t index);
    virtual void BackpatchConDisSet(int32_t index) = 0;
    int32_t CurrentSourceLine() const { return currentSourceLine; }
    void SetCurrentSourceLine(int32_t currentSourceLine_) { currentSourceLine = currentSourceLine_; }
private:
    std::vector<InstIndexRequest*> instRequests;
    int32_t currentSourceLine;
};

class PCRange
{
public:
    PCRange();
    void SetStart(int32_t start_) { start = start_; }
    void SetEnd(int32_t end_) { end = end_; }
    bool InRange(int32_t pc) const { return pc >= start && pc <= end; }
    void Write(Writer& writer);
    void Read(Reader& reader);
private:
    int32_t start;
    int32_t end;
};

class CatchBlock
{
public:
    CatchBlock();
    void Write(Writer& writer);
    void Read(Reader& reader);
    void SetExceptionVarClassTypeFullName(Constant exceptionVarClassTypeFullName_) { exceptionVarClassTypeFullName = exceptionVarClassTypeFullName_; }
    ObjectType* GetExceptionVarType() const { return exceptionVarType; }
    void ResolveExceptionVarType();
    void SetExceptionVarIndex(uint32_t exceptionVarIndex_) { exceptionVarIndex = exceptionVarIndex_; }
    uint32_t GetExceptionVarIndex() const { return exceptionVarIndex; }
    void SetCatchBlockStart(int32_t catchBlockStart_) { catchBlockStart = catchBlockStart_; }
    int32_t CatchBlockStart() const { return catchBlockStart; }
private:
    Constant exceptionVarClassTypeFullName;
    ObjectType* exceptionVarType;
    uint32_t exceptionVarIndex;
    int32_t catchBlockStart;
};

class ExceptionBlock
{
public:
    ExceptionBlock(int id_);
    int Id() const { return id; }
    bool HasParent() const { return parentId != -1; }
    void SetParentId(int parentId_) { parentId = parentId_; }
    int ParentId() const { return parentId; }
    void Write(Writer& writer);
    void Read(Reader& reader);
    bool Match(int32_t pc) const;
    void AddPCRange(const PCRange& pcRange);
    PCRange& GetLastPCRange() { Assert(!pcRanges.empty(), "pc ranges empty"); return pcRanges.back(); }
    void AddCatchBlock(std::unique_ptr<CatchBlock>&& catchBlock);
    const std::vector<std::unique_ptr<CatchBlock>>& CatchBlocks() const { return catchBlocks; }
    void ResolveExceptionVarTypes();
    void SetFinallyStart(int32_t finallyStart_) { finallyStart = finallyStart_; }
    bool HasFinally() const { return finallyStart != -1; }
    int32_t FinallyStart() const { return finallyStart; }
    void SetNextTarget(int32_t nextTarget_) { nextTarget = nextTarget_; }
    int32_t NextTarget() const { return nextTarget; }
private:
    int id;
    int parentId;
    std::vector<PCRange> pcRanges;
    std::vector<std::unique_ptr<CatchBlock>> catchBlocks;
    int32_t finallyStart;
    int32_t nextTarget;
};

class Function
{
public:
    Function();
    Function(Constant callName_, Constant friendlyName_, uint32_t id_, ConstantPool* constantPool_);
    Constant CallName() const { return callName; }
    Constant FriendlyName() const { return friendlyName; }
    Constant SourceFilePath() const { return sourceFilePath; }
    void SetSourceFilePath(Constant sourceFilePath_) { sourceFilePath = sourceFilePath_; }
    bool HasSourceFilePath() const;
    uint32_t Id() const { return id; }
    ConstantPool& GetConstantPool() { Assert(constantPool, "constant pool not set"); return *constantPool; }
    void SetConstantPool(ConstantPool* constantPool_) { constantPool = constantPool_; }
    void Write(Writer& writer);
    void Read(Reader& reader);
    uint32_t NumLocals() const { return numLocals; }
    void SetNumLocals(uint32_t numLocals_);
    uint32_t NumParameters() const { return numParameters; }
    void SetNumParameters(uint32_t numParameters_);
    int NumInsts() const { return int(instructions.size()); }
    Instruction* GetInst(int index) const { return instructions[index].get(); }
    void AddInst(std::unique_ptr<Instruction>&& inst);
    void SetInst(int32_t index, std::unique_ptr<Instruction>&& inst);
    void Dump(CodeFormatter& formatter);
    void Dump(CodeFormatter& formatter, int32_t pc);
    bool IsMain() const { return isMain; }
    void SetMain() { isMain = true; }
    void SetEmitter(Emitter* emitter_) { emitter = emitter_; }
    Emitter* GetEmitter() const { return emitter; }
    int GetNextExeptionBlockId() const { return int(exceptionBlocks.size()); }
    void AddExceptionBlock(std::unique_ptr<ExceptionBlock>&& exceptionBlock);
    ExceptionBlock* GetExceptionBlock(int id) const;
    ExceptionBlock* FindExceptionBlock(int32_t pc) const;
    void ResolveExceptionVarTypes();
    void MapPCToSourceLine(uint32_t pc, uint32_t sourceLine);
    uint32_t GetSourceLine(uint32_t pc) const;
private:
    Constant callName;
    Constant friendlyName;
    Constant sourceFilePath;
    uint32_t id;
    ConstantPool* constantPool;
    std::vector<std::unique_ptr<Instruction>> instructions;
    uint32_t numLocals;
    uint32_t numParameters;
    bool isMain;
    std::vector<std::unique_ptr<ExceptionBlock>> exceptionBlocks;
    std::unordered_map<uint32_t, uint32_t> pcSoureLineMap;
    Emitter* emitter;
};

class FunctionTable
{
public:
    static void Init();
    static void Done();
    FunctionTable();
    static FunctionTable& Instance() { Assert(instance, "function table not initialized"); return *instance; }
    void AddFunction(Function* fun);
    Function* GetFunction(StringPtr functionCallName) const;
    Function* GetMain() const { return main; }
    void ResolveExceptionVarTypes();
private:
    static std::unique_ptr<FunctionTable> instance;
    std::unordered_map<StringPtr, Function*, StringPtrHash> functionMap;
    Function* main;
};

class VmFunction
{
public:
    void SetName(Constant name_) { name = name_; }
    Constant Name() const { return name; }
    virtual ~VmFunction();
    virtual void Execute(Frame& frame) = 0;
private:
    Constant name;
};

class VmFunctionTable
{
public:
    static void Init();
    static void Done();
    static VmFunctionTable& Instance() { Assert(instance, "virtual machine function table not initialized"); return *instance; }
    void RegisterVmFunction(VmFunction* vmFunction);
    VmFunction* GetVmFunction(StringPtr vmFuntionName) const;
private:
    static std::unique_ptr<VmFunctionTable> instance;
    std::unordered_map<StringPtr, VmFunction*, StringPtrHash> vmFunctionMap;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_FUNCTION_INCLUDED
