// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_FUNCTION_INCLUDED
#define CMINOR_MACHINE_FUNCTION_INCLUDED
#include <cminor/machine/MachineApi.hpp>
#include <cminor/machine/Constant.hpp>
#include <cminor/machine/Instruction.hpp>
#include <ostream>
#include <map>

namespace cminor { namespace machine {

class GenObject;

class MACHINE_API InstIndexRequest
{
public:
    InstIndexRequest();
    void SetIndex(int32_t index_) { index = index_; }
    int32_t Index() const { return index; }
private:
    int32_t index;
};

class MACHINE_API Emitter
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

class MACHINE_API PCRange
{
public:
    PCRange();
    void SetStart(int32_t start_) { start = start_; }
    void SetEnd(int32_t end_) { end = end_; }
    bool InRange(int32_t pc) const { return pc >= start && pc <= end; }
    void Write(Writer& writer);
    void Read(Reader& reader);
    void Adjust(const std::vector<int32_t>& instructionOffsets);
    void Dump(CodeFormatter& formatter);
private:
    int32_t start;
    int32_t end;
};

class MACHINE_API CatchBlock
{
public:
    CatchBlock(int id_);
    CatchBlock(const CatchBlock&) = delete;
    CatchBlock& operator=(const CatchBlock&) = delete;
    int Id() const { return id; }
    void Write(Writer& writer);
    void Read(Reader& reader);
    void SetExceptionVarClassTypeFullName(Constant exceptionVarClassTypeFullName_) { exceptionVarClassTypeFullName = exceptionVarClassTypeFullName_; }
    ObjectType* GetExceptionVarType() const { return exceptionVarType; }
    void ResolveExceptionVarType();
    void SetExceptionVarIndex(uint32_t exceptionVarIndex_) { exceptionVarIndex = exceptionVarIndex_; }
    uint32_t GetExceptionVarIndex() const { return exceptionVarIndex; }
    void SetCatchBlockStart(int32_t catchBlockStart_) { catchBlockStart = catchBlockStart_; }
    int32_t CatchBlockStart() const { return catchBlockStart; }
    void Adjust(const std::vector<int32_t>& instructionOffsets, std::unordered_set<int32_t>& jumpTargets);
    void Dump(CodeFormatter& formatter);
private:
    int id;
    Constant exceptionVarClassTypeFullName;
    ObjectType* exceptionVarType;
    uint32_t exceptionVarIndex;
    int32_t catchBlockStart;
};

class MACHINE_API ExceptionBlock
{
public:
    ExceptionBlock(int id_);
    ExceptionBlock(const ExceptionBlock&) = delete;
    ExceptionBlock operator=(const ExceptionBlock&) = delete;
    int Id() const { return id; }
    bool HasParent() const { return parentId != -1; }
    void SetParentId(int parentId_) { parentId = parentId_; }
    int ParentId() const { return parentId; }
    void Write(Writer& writer);
    void Read(Reader& reader);
    bool Match(int32_t pc) const;
    void AddPCRange(const PCRange& pcRange);
    PCRange& GetLastPCRange() { Assert(!pcRanges.empty(), "pc ranges empty"); return pcRanges.back(); }
    int GetNextCatchBlockId() const { return int(catchBlocks.size()); }
    CatchBlock* GetCatchBlock(int catchBlockId) { Assert(catchBlockId >= 0 && catchBlockId < int(catchBlocks.size()), "invalid catch block id");  return catchBlocks[catchBlockId].get(); }
    void AddCatchBlock(std::unique_ptr<CatchBlock>&& catchBlock);
    const std::vector<std::unique_ptr<CatchBlock>>& CatchBlocks() const { return catchBlocks; }
    void ResolveExceptionVarTypes();
    void SetFinallyStart(int32_t finallyStart_) { finallyStart = finallyStart_; }
    bool HasFinally() const { return finallyStart != -1; }
    int32_t FinallyStart() const { return finallyStart; }
    void SetNextTarget(int32_t nextTarget_) { nextTarget = nextTarget_; }
    int32_t NextTarget() const { return nextTarget; }
    void Adjust(const std::vector<int32_t>& instructionOffsets, std::unordered_set<int32_t>& jumpTargets);
    void Dump(CodeFormatter& formatter);
private:
    int id;
    int parentId;
    std::vector<PCRange> pcRanges;
    std::vector<std::unique_ptr<CatchBlock>> catchBlocks;
    int32_t finallyStart;
    int32_t nextTarget;
};

class MachineFunctionVisitor;

enum class FunctionFlags : uint8_t
{
    none = 0, 
    exported = 1 << 0, 
    canThrow = 1 << 1
};

inline FunctionFlags operator&(FunctionFlags left, FunctionFlags right)
{
    return FunctionFlags(uint8_t(left) & uint8_t(right));
}

inline FunctionFlags operator|(FunctionFlags left, FunctionFlags right)
{
    return FunctionFlags(uint8_t(left) | uint8_t(right));
}

std::string FunctionFlagsStr(FunctionFlags flags);

class MACHINE_API Function
{
public:
    Function();
    Function(Constant groupName_, Constant callName_, Constant fullName_, uint32_t id_, ConstantPool* constantPool_);
    Function(const Function&) = delete;
    Function& operator=(const Function&) = delete;
    Constant GroupName() const { return groupName; }
    Constant CallName() const { return callName; }
    Constant FullName() const { return fullName; }
    Constant SourceFilePath() const { return sourceFilePath; }
    const std::string& MangledName() const { return mangledName; }
    void SetMangledName(const std::string& mangledName_);
    void SetSourceFilePath(Constant sourceFilePath_) { sourceFilePath = sourceFilePath_; }
    bool HasSourceFilePath() const;
    uint32_t Id() const { return id; }
    ConstantPool& GetConstantPool() { Assert(constantPool, "constant pool not set"); return *constantPool; }
    void SetConstantPool(ConstantPool* constantPool_) { constantPool = constantPool_; }
    void Write(Writer& writer);
    void Read(Reader& reader);
    uint32_t NumLocals() const { return numLocals; }
    void SetNumLocals(uint32_t numLocals_);
    const std::vector<ValueType>& LocalTypes() const { return localTypes; }
    std::vector<ValueType>& LocalTypes() { return localTypes; }
    uint32_t NumParameters() const { return numParameters; }
    void SetNumParameters(uint32_t numParameters_);
    const std::vector<ValueType>& ParameterTypes() const { return parameterTypes; }
    std::vector<ValueType>& ParameterTypes() { return parameterTypes; }
    bool ReturnsValue() const { return returnsValue; }
    void SetReturnsValue() { returnsValue = true; }
    ValueType ReturnType() const { return returnType; }
    void SetReturnType(ValueType returnType_) { returnType = returnType_; }
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
    bool HasExceptionBlocks() const { return !exceptionBlocks.empty(); }
    int NumExceptionBlocks() const { return int(exceptionBlocks.size()); }
    int GetNextExeptionBlockId() const { return int(exceptionBlocks.size()); }
    void AddExceptionBlock(std::unique_ptr<ExceptionBlock>&& exceptionBlock);
    ExceptionBlock* GetExceptionBlock(int id) const;
    ExceptionBlock* FindExceptionBlock(int32_t pc) const;
    void ResolveExceptionVarTypes();
    void MapPCToSourceLine(uint32_t pc, uint32_t sourceLine);
    uint32_t GetFirstSourceLine() const;
    uint32_t GetSourceLine(uint32_t pc) const;
    uint32_t GetPC(uint32_t sourceLine) const;
    bool HasBreakPointAt(uint32_t pc) const;
    void SetBreakPointAt(uint32_t pc);
    void RemoveBreakPointAt(uint32_t pc);
    void RemoveUnreachableInstructions(std::unordered_set<int32_t>& jumpTargets);
    void Accept(MachineFunctionVisitor& visitor);
    bool IsExported() const { return GetFlag(FunctionFlags::exported); }
    void SetExported() { SetFlag(FunctionFlags::exported); }
    bool CanThrow() const { return GetFlag(FunctionFlags::canThrow); }
    void SetCanThrow() { SetFlag(FunctionFlags::canThrow); }
    void* Address() const { return address; }
    void SetAddress(void* address_) { address = address_; }
    void* GetAssembly() const { return assembly; }
    void SetAssembly(void* assembly_) { assembly = assembly_; }
    void* FunctionSymbol() const { return functionSymbol; }
    void SetFunctionSymbol(void* functionSymbol_) { functionSymbol = functionSymbol_; }
    void SetAlreadyGenerated() { alreadyGenerated = true; }
    bool AlreadyGenerated() const { return alreadyGenerated; }
private:
    Constant groupName;
    Constant callName;
    Constant fullName;
    Constant sourceFilePath;
    uint32_t id;
    std::string mangledName;
    ConstantPool* constantPool;
    std::vector<std::unique_ptr<Instruction>> instructions;
    uint32_t numLocals;
    std::vector<ValueType> localTypes;
    uint32_t numParameters;
    std::vector<ValueType> parameterTypes;
    bool returnsValue;
    ValueType returnType;
    bool isMain;
    std::vector<std::unique_ptr<ExceptionBlock>> exceptionBlocks;
    std::map<uint32_t, uint32_t> pcSourceLineMap;
    std::map<uint32_t, uint32_t> sourceLinePCMap;
    std::unordered_set<uint32_t> mappedSourceLines;
    std::unordered_set<uint32_t> breakPoints;
    Emitter* emitter;
    FunctionFlags flags;
    void* address;
    void* assembly;
    void* functionSymbol;
    bool alreadyGenerated;
    bool GetFlag(FunctionFlags flag) const { return (flags & flag) != FunctionFlags::none; }
    void SetFlag(FunctionFlags flag) { flags = flags | flag; }
    void AdjustPCSourceLineMap(const std::vector<int32_t>& instructionOffsets);
    void AdjustSourceLinePCMap(const std::vector<int32_t>& instructionOffsets);
};

class FunctionTable
{
public:
    MACHINE_API static void Init();
    MACHINE_API static void Done();
    MACHINE_API static void AddFunction(Function* fun, bool memberOfClassTemplateSpecialization);
    MACHINE_API static Function* GetFunction(StringPtr functionCallName);
    MACHINE_API static Function* GetFunctionNothrow(StringPtr functionCallName);
    MACHINE_API static Function* GetMain();
    MACHINE_API static void ResolveExceptionVarTypes();
};

class MACHINE_API VmFunction
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
    MACHINE_API static void Init();
    MACHINE_API static void Done();
    MACHINE_API static void RegisterVmFunction(VmFunction* vmFunction);
    MACHINE_API static VmFunction* GetVmFunction(StringPtr vmFunctionName);
};

class MACHINE_API LineFunctionTable
{
public:
    LineFunctionTable();
    void MapSourceLineToFunction(uint32_t sourceLine, Function* function);
    Function* GetFunction(uint32_t sourceLine) const;
private:
    std::unordered_map<uint32_t, Function*> lineFunctionMap;
};

class MACHINE_API BreakPoint
{
public:
    BreakPoint();
    BreakPoint(Function* function_, uint32_t line_, uint32_t pc_);
    void Remove();
    Function* GetFunction() const { return function; }
    uint32_t Line() const { return line; }
    uint32_t PC() const { return pc; }
private:
    Function* function;
    uint32_t line;
    uint32_t pc;
};

class MACHINE_API SourceFile
{
public:
    SourceFile(Constant sourceFilePath_);
    void Read();
    void List(int lineNumber, int numLines);
    Constant SourceFilePath() const { return sourceFilePath; }
    std::string GetLine(int lineNumber) const;
private:
    Constant sourceFilePath;
    std::vector<std::string> lines;
};

class SourceFileTable
{
public:
    MACHINE_API static bool Initialized();
    MACHINE_API static void Init();
    MACHINE_API static void Done();
    MACHINE_API static Constant GetSourceFilePath(const std::string& sourceFileName);
    MACHINE_API static SourceFile* GetSourceFile(Constant sourceFilePath);
    MACHINE_API static SourceFile* GetSourceFile(const std::string& sourceFileName);
    MACHINE_API static void MapSourceFileLine(Constant sourceFilePath, uint32_t sourceLine, Function* function);
    MACHINE_API static int SetBreakPoint(Constant sourceFilePath, uint32_t sourceLine);
    MACHINE_API static int SetBreakPoint(const std::string& sourceFileName, uint32_t sourceLine);
    MACHINE_API static void RemoveBreakPoint(int bp);
    MACHINE_API static const BreakPoint* GetBreakPoint(int bp);
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_FUNCTION_INCLUDED
