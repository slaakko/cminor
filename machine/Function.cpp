// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Function.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Unicode.hpp>

namespace cminor { namespace machine {

InstIndexRequest::InstIndexRequest() : index(-1)
{
}

Emitter::Emitter() : currentSourceLine(-1)
{
}

Emitter::~Emitter()
{
}

void Emitter::AddIndexRequest(InstIndexRequest* request)
{
    instRequests.push_back(request);
}

void Emitter::SatisfyIndexRequests(int32_t index)
{
    for (InstIndexRequest* request : instRequests)
    {
        request->SetIndex(index);
    }
    instRequests.clear();
}

PCRange::PCRange() : start(-1), end(-1)
{
}

void PCRange::Write(Writer& writer)
{
    writer.Put(start);
    writer.Put(end);
}

void PCRange::Read(Reader& reader)
{
    start = reader.GetInt();
    end = reader.GetInt();
}

CatchBlock::CatchBlock() : exceptionVarClassTypeFullName(), exceptionVarType(nullptr), exceptionVarIndex(-1), catchBlockStart(-1)
{
}

void CatchBlock::Write(Writer& writer)
{
    ConstantId fullNameId = writer.GetConstantPool()->GetIdFor(exceptionVarClassTypeFullName);
    Assert(fullNameId != noConstantId, "got no id");
    fullNameId.Write(writer);
    writer.PutEncodedUInt(exceptionVarIndex);
    writer.Put(catchBlockStart);
}

void CatchBlock::Read(Reader& reader)
{
    ConstantId fullNameId;
    fullNameId.Read(reader);
    exceptionVarClassTypeFullName = reader.GetConstantPool()->GetConstant(fullNameId);
    exceptionVarIndex = reader.GetEncodedUInt();
    catchBlockStart = reader.GetInt();
}

void CatchBlock::ResolveExceptionVarType()
{
    ClassData* classData = ClassDataTable::Instance().GetClassData(exceptionVarClassTypeFullName.Value().AsStringLiteral());
    exceptionVarType = classData->Type();
}

ExceptionBlock::ExceptionBlock(int id_) : id(id_), parentId(-1), finallyStart(-1), nextTarget(-1)
{
}

void ExceptionBlock::Write(Writer& writer)
{
    writer.Put(id);
    writer.Put(parentId);
    uint32_t n = uint32_t(pcRanges.size());
    writer.PutEncodedUInt(n);
    for (uint32_t i = 0; i < n; ++i)
    {
        PCRange& pcRange = pcRanges[i];
        pcRange.Write(writer);
    }
    uint32_t nc = uint32_t(catchBlocks.size());
    writer.PutEncodedUInt(nc);
    for (const std::unique_ptr<CatchBlock>& catchBlock : catchBlocks)
    {
        catchBlock->Write(writer);
    }
    writer.Put(finallyStart);
    writer.Put(nextTarget);
}

void ExceptionBlock::Read(Reader& reader)
{
    id = reader.GetInt();
    parentId = reader.GetInt();
    uint32_t n = reader.GetEncodedUInt();
    for (uint32_t i = 0; i < n; ++i)
    {
        PCRange pcRange;
        pcRange.Read(reader);
        pcRanges.push_back(pcRange);
    }
    uint32_t nc = reader.GetEncodedUInt();
    for (uint32_t i = 0; i < nc; ++i)
    {
        std::unique_ptr<CatchBlock> catchBlock(new CatchBlock());
        catchBlock->Read(reader);
        catchBlocks.push_back(std::move(catchBlock));
    }
    finallyStart = reader.GetInt();
    nextTarget = reader.GetInt();
}

bool ExceptionBlock::Match(int32_t pc) const
{
    for (const PCRange& pcRange : pcRanges)
    {
        if (pcRange.InRange(pc)) return true;
    }
    return false;
}

void ExceptionBlock::AddPCRange(const PCRange& pcRange)
{
    pcRanges.push_back(pcRange);
}

void ExceptionBlock::AddCatchBlock(std::unique_ptr<CatchBlock>&& catchBlock)
{
    catchBlocks.push_back(std::move(catchBlock));
}

void ExceptionBlock::ResolveExceptionVarTypes()
{
    for (const std::unique_ptr<CatchBlock>& catchBlock : catchBlocks)
    {
        catchBlock->ResolveExceptionVarType();
    }
}

Function::Function() : callName(), friendlyName(), sourceFilePath(), id(-1), numLocals(0), numParameters(0), constantPool(nullptr), isMain(false), emitter(nullptr)
{
}

Function::Function(Constant callName_, Constant friendlyName_, uint32_t id_, ConstantPool* constantPool_) : 
    callName(callName_), friendlyName(friendlyName_), sourceFilePath(), id(id_), numLocals(0), numParameters(0), constantPool(constantPool_), isMain(false), emitter(nullptr)
{
}

bool Function::HasSourceFilePath() const
{
    return sourceFilePath.Value().AsStringLiteral() != nullptr;
}

void Function::Write(Writer& writer)
{
    ConstantId callNameId = constantPool->GetIdFor(callName);
    Assert(callNameId != noConstantId, "got no constant id");
    callNameId.Write(writer);
    ConstantId friendlyNameId = constantPool->GetIdFor(friendlyName);
    Assert(friendlyNameId != noConstantId, "got no constant id");
    friendlyNameId.Write(writer);
    bool hasSourceFilePath = sourceFilePath.Value().AsStringLiteral() != nullptr;
    writer.Put(hasSourceFilePath);
    if (hasSourceFilePath)
    {
        ConstantId sourceFilePathId = constantPool->GetIdFor(sourceFilePath);
        Assert(sourceFilePathId != noConstantId, "got no constant id");
        sourceFilePathId.Write(writer);
    }
    writer.PutEncodedUInt(id);
    uint32_t n = static_cast<uint32_t>(instructions.size());
    writer.PutEncodedUInt(n);
    for (uint32_t i = 0; i < n; ++i)
    {
        Instruction* inst = instructions[i].get();
        inst->Encode(writer);
    }
    writer.PutEncodedUInt(numLocals);
    writer.PutEncodedUInt(numParameters);
    uint32_t ne = uint32_t(exceptionBlocks.size());
    writer.PutEncodedUInt(ne);
    for (const std::unique_ptr<ExceptionBlock>& exceptionBlock : exceptionBlocks)
    {
        exceptionBlock->Write(writer);
    }
    uint32_t npc = uint32_t(pcSoureLineMap.size());
    writer.PutEncodedUInt(npc);
    for (const std::pair<uint32_t, uint32_t>& p : pcSoureLineMap)
    {
        writer.PutEncodedUInt(p.first);
        writer.PutEncodedUInt(p.second);
    }
}

void Function::Read(Reader& reader)
{
    constantPool = reader.GetConstantPool();
    ConstantId callNameId;
    callNameId.Read(reader);
    callName = constantPool->GetConstant(callNameId);
    ConstantId friendlyNameId;
    friendlyNameId.Read(reader);
    friendlyName = constantPool->GetConstant(friendlyNameId);
    bool hasSourceFilePath = reader.GetBool();
    if (hasSourceFilePath)
    {
        ConstantId sourceFilePathId;
        sourceFilePathId.Read(reader);
        sourceFilePath = constantPool->GetConstant(sourceFilePathId);
    }
    ConstantId 
    id = reader.GetEncodedUInt();
    uint32_t n = reader.GetEncodedUInt();
    for (uint32_t i = 0; i < n; ++i)
    {
        AddInst(reader.GetMachine().DecodeInst(reader));
    }
    numLocals = reader.GetEncodedUInt();
    numParameters = reader.GetEncodedUInt();
    uint32_t ne = reader.GetEncodedUInt();
    for (uint32_t i = 0; i < ne; ++i)
    {
        std::unique_ptr<ExceptionBlock> exceptionBlock(new ExceptionBlock(int(exceptionBlocks.size())));
        exceptionBlock->Read(reader);
        exceptionBlocks.push_back(std::move(exceptionBlock));
    }
    uint32_t npc = reader.GetEncodedUInt();
    for (uint32_t i = 0; i < npc; ++i)
    {
        uint32_t pc = reader.GetEncodedUInt();
        uint32_t sourceLine = reader.GetEncodedUInt();
        MapPCToSourceLine(pc, sourceLine);
    }
}

void Function::SetNumLocals(uint32_t numLocals_)
{
    numLocals = numLocals_;
}

void Function::SetNumParameters(uint32_t numParameters_)
{
    numParameters = numParameters_;
}

void Function::AddInst(std::unique_ptr<Instruction>&& inst)
{
    if (emitter)
    {
        int32_t instructionIndex = int32_t(instructions.size());
        if (emitter->CreatePCRange())
        {
            emitter->DoCreatePCRange(instructionIndex);
        }
        if (emitter->SetPCRangeEnd())
        {
            emitter->DoSetPCRangeEnd(instructionIndex);
        }
        emitter->SatisfyIndexRequests(instructionIndex);
    }
    instructions.push_back(std::move(inst));
}

void Function::SetInst(int32_t index, std::unique_ptr<Instruction>&& inst)
{
    instructions[index] = std::move(inst);
}

void Function::Dump(CodeFormatter& formatter)
{
    formatter.WriteLine(ToUtf8(callName.Value().AsStringLiteral()) + " [" + ToUtf8(friendlyName.Value().AsStringLiteral()) + "]");
    formatter.WriteLine("{");
    formatter.IncIndent();
    int32_t n = static_cast<int32_t>(instructions.size());
    for (int32_t i = 0; i < n; ++i)
    {
        Instruction* inst = instructions[i].get();
        std::string instructionNumber = std::to_string(i);
        while (instructionNumber.length() < 4)
        {
            instructionNumber.append(1, ' ');
        }
        formatter.Write(instructionNumber + " ");
        inst->Dump(formatter);
        formatter.WriteLine();
    }
    formatter.DecIndent();
    formatter.WriteLine("}");
}

void Function::Dump(CodeFormatter& formatter, int32_t pc)
{
    if (pc == 0)
    {
        formatter.WriteLine(ToUtf8(callName.Value().AsStringLiteral()) + " [" + ToUtf8(friendlyName.Value().AsStringLiteral()) + "]");
        formatter.WriteLine("{");
        formatter.IncIndent();
    }
    else
    {
        formatter.IncIndent();
    }
    int32_t start = std::max(0, pc - 4);
    int32_t end = std::min(NumInsts(), pc + 4);
    for (int32_t i = start; i < end; ++i)
    {
        std::string pcInd = " ";
        if (i == pc)
        {
            pcInd = "*";
        }
        Instruction* inst = instructions[i].get();
        std::string instructionNumber = std::to_string(i);
        while (instructionNumber.length() < 4)
        {
            instructionNumber.append(1, ' ');
        }
        formatter.Write(instructionNumber + pcInd + " ");
        inst->Dump(formatter);
        formatter.WriteLine();
    }
}

void Function::AddExceptionBlock(std::unique_ptr<ExceptionBlock>&& exceptionBlock)
{
    exceptionBlocks.push_back(std::move(exceptionBlock));
}

ExceptionBlock* Function::GetExceptionBlock(int id) const
{
    if (id >= 0 && id < int(exceptionBlocks.size()))
    {
        return exceptionBlocks[id].get();
    }
    return nullptr;
}

ExceptionBlock* Function::FindExceptionBlock(int32_t pc) const
{
    for (const std::unique_ptr<ExceptionBlock>& exceptionBlock : exceptionBlocks)
    {
        if (exceptionBlock->Match(pc)) return exceptionBlock.get();
    }
    return nullptr;
}

void Function::ResolveExceptionVarTypes()
{
    for (const std::unique_ptr<ExceptionBlock>& exceptionBlock : exceptionBlocks)
    {
        exceptionBlock->ResolveExceptionVarTypes();
    }
}

void Function::MapPCToSourceLine(uint32_t pc, uint32_t sourceLine)
{
    if (pc == -1 || sourceLine == -1) return;
    pcSoureLineMap[pc] = sourceLine;
}

uint32_t Function::GetSourceLine(uint32_t pc) const
{
    auto it = pcSoureLineMap.find(pc);
    if (it != pcSoureLineMap.cend())
    {
        return it->second;
    }
    return -1;
}

std::unique_ptr<FunctionTable> FunctionTable::instance;

void FunctionTable::Init()
{
    instance.reset(new FunctionTable());
}

void FunctionTable::Done()
{
    instance.reset();
}

FunctionTable::FunctionTable() : main(nullptr)
{
}

void FunctionTable::AddFunction(Function* fun)
{
    StringPtr functionCallName = fun->CallName().Value().AsStringLiteral();
    auto it = functionMap.find(functionCallName);
    if (it != functionMap.cend())
    {
        throw std::runtime_error("function '" + ToUtf8(functionCallName.Value()) + "' already added to function table");
    }
    else
    {
        functionMap[functionCallName] = fun;
    }
    if (fun->IsMain())
    {
        if (main)
        {
            throw std::runtime_error("already has main function set");
        }
        else
        {
            main = fun;
        }
    }
}

Function* FunctionTable::GetFunction(StringPtr functionCallName) const
{
    auto it = functionMap.find(functionCallName);
    if (it != functionMap.cend())
    {
        return it->second;
    }
    else
    {
        throw std::runtime_error("function '" + ToUtf8(functionCallName.Value()) + "' not found from function table");
    }
}

void FunctionTable::ResolveExceptionVarTypes()
{
    for (const auto& p : functionMap)
    {
        p.second->ResolveExceptionVarTypes();
    }
}

VmFunction::~VmFunction()
{
}

std::unique_ptr<VmFunctionTable> VmFunctionTable::instance;

void VmFunctionTable::Init()
{
    instance.reset(new VmFunctionTable());
}

void VmFunctionTable::Done()
{
    instance.reset();
}

void VmFunctionTable::RegisterVmFunction(VmFunction* vmFunction)
{
    Constant vmFuntionName = vmFunction->Name();
    Assert(vmFuntionName.Value().GetType() == ValueType::stringLiteral, "string literal expected");
    auto it = vmFunctionMap.find(StringPtr(vmFuntionName.Value().AsStringLiteral()));
    if (it != vmFunctionMap.cend())
    {
        throw std::runtime_error("virtual machine function " + ToUtf8(vmFuntionName.Value().AsStringLiteral()) + "' already registered to virtual machine functon table");
    }
    else
    {
        vmFunctionMap[StringPtr(vmFuntionName.Value().AsStringLiteral())] = vmFunction;
    }
}

VmFunction* VmFunctionTable::GetVmFunction(StringPtr vmFuntionName) const
{
    auto it = vmFunctionMap.find(vmFuntionName);
    if (it != vmFunctionMap.cend())
    {
        return it->second;
    }
    else
    {
        throw std::runtime_error("virtual machine function '" + ToUtf8(vmFuntionName.Value()) + "' not found from virtual machine function table");
    }
}

} } // namespace cminor::machine
