// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Function.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Class.hpp>
#include <cminor/util/Unicode.hpp>
#include <cminor/util/Util.hpp>
#include <cminor/util/TextUtils.hpp>
#include <cminor/machine/OsInterface.hpp>
#include <cminor/machine/MachineFunctionVisitor.hpp>
#include <iostream>
#include <iomanip>

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

void PCRange::Adjust(const std::vector<int32_t>& instructionOffsets)
{
    if (start != endOfFunction)
    {
        start = start - instructionOffsets[start];
    }
    if (end != endOfFunction)
    {
        end = end - instructionOffsets[end];
    }
}

void PCRange::Dump(CodeFormatter& formatter)
{
    formatter.WriteLine("[" + std::to_string(start) + ":" + std::to_string(end) + "]");
}

CatchBlock::CatchBlock(int id_) : id(id_), exceptionVarClassTypeFullName(), exceptionVarType(nullptr), exceptionVarIndex(-1), catchBlockStart(-1)
{
}

void CatchBlock::Write(Writer& writer)
{
    writer.Put(id);
    ConstantId fullNameId = writer.GetConstantPool()->GetIdFor(exceptionVarClassTypeFullName);
    Assert(fullNameId != noConstantId, "got no id");
    fullNameId.Write(writer);
    writer.PutEncodedUInt(exceptionVarIndex);
    writer.Put(catchBlockStart);
}

void CatchBlock::Read(Reader& reader)
{
    id = reader.GetInt();
    ConstantId fullNameId;
    fullNameId.Read(reader);
    exceptionVarClassTypeFullName = reader.GetConstantPool()->GetConstant(fullNameId);
    exceptionVarIndex = reader.GetEncodedUInt();
    catchBlockStart = reader.GetInt();
}

void CatchBlock::ResolveExceptionVarType()
{
    ClassData* classData = ClassDataTable::GetClassData(exceptionVarClassTypeFullName.Value().AsStringLiteral());
    exceptionVarType = classData->Type();
}

void CatchBlock::Adjust(const std::vector<int32_t>& instructionOffsets, std::unordered_set<int32_t>& jumpTargets)
{
    if (catchBlockStart != endOfFunction)
    {
        catchBlockStart = catchBlockStart - instructionOffsets[catchBlockStart];
        jumpTargets.insert(catchBlockStart);
    }
}

void CatchBlock::Dump(CodeFormatter& formatter)
{
    formatter.WriteLine("catch " + std::to_string(id) + " : " + ToUtf8(exceptionVarClassTypeFullName.Value().AsStringLiteral()) + " : " + std::to_string(catchBlockStart));
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
        std::unique_ptr<CatchBlock> catchBlock(new CatchBlock(i));
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

void ExceptionBlock::Adjust(const std::vector<int32_t>& instructionOffsets, std::unordered_set<int32_t>& jumpTargets)
{
    for (PCRange& pcRange : pcRanges)
    {
        pcRange.Adjust(instructionOffsets);
    }
    for (const std::unique_ptr<CatchBlock>& catchBlock : catchBlocks)
    {
        catchBlock->Adjust(instructionOffsets, jumpTargets);
    }
    if (finallyStart != endOfFunction)
    {
        finallyStart = finallyStart - instructionOffsets[finallyStart];
        jumpTargets.insert(finallyStart);
    }
    if (nextTarget != endOfFunction)
    {
        nextTarget = nextTarget - instructionOffsets[nextTarget];
        jumpTargets.insert(nextTarget);
    }
}

void ExceptionBlock::Dump(CodeFormatter& formatter)
{
    std::string parent;
    if (parentId != -1)
    {
        parent = " (parent=" + std::to_string(parentId) + ")";
    }
    formatter.WriteLine("exception block " + std::to_string(id) + parent);
    formatter.WriteLine("PC ranges:");
    for (PCRange& range : pcRanges)
    {
        range.Dump(formatter);
    }
    for (const std::unique_ptr<CatchBlock>& catchBlock : catchBlocks)
    {
        catchBlock->Dump(formatter);
    }
    if (finallyStart != endOfFunction)
    {
        formatter.WriteLine("finally " + std::to_string(finallyStart));
    }
    if (nextTarget != endOfFunction)
    {
        formatter.WriteLine("next " + std::to_string(nextTarget));
    }
}

std::string FunctionFlagsStr(FunctionFlags flags)
{
    if (flags == FunctionFlags::none)
    {
        return "none";
    }
    else
    {
        std::string s;
        if ((flags & FunctionFlags::exported) != FunctionFlags::none)
        {
            s.append("exported");
        }
        if ((flags & FunctionFlags::canThrow) != FunctionFlags::none)
        {
            if (!s.empty())
            {
                s.append(" ");
            }
            s.append("canThrow");
        }
        if ((flags & FunctionFlags::inlineAttr) != FunctionFlags::none)
        {
            if (!s.empty())
            {
                s.append(" ");
            }
            s.append("inline");
        }
        return s;
    }
}

Function::Function() : groupName(), callName(), fullName(), sourceFilePath(), id(-1), numLocals(0), numParameters(0), constantPool(nullptr), isMain(false), emitter(nullptr), returnsValue(false),
    returnType(ValueType::none), flags(FunctionFlags::none), address(nullptr), assembly(nullptr), functionSymbol(nullptr), alreadyGenerated(false)
{
}

Function::Function(Constant groupName_, Constant callName_, Constant friendlyName_, uint32_t id_, ConstantPool* constantPool_) :
    groupName(groupName_), callName(callName_), fullName(friendlyName_), sourceFilePath(), id(id_), numLocals(0), numParameters(0), constantPool(constantPool_), isMain(false), emitter(nullptr),
    returnsValue(false), returnType(ValueType::none), flags(FunctionFlags::none), address(nullptr), assembly(nullptr), functionSymbol(nullptr), alreadyGenerated(false)
{
}

void Function::SetMangledName(const std::string& mangledName_)
{
    mangledName = mangledName_;
}

void Function::SetMangledInlineName(const std::string& mangledInlineName_)
{
    mangledInlineName = mangledInlineName_;
}

bool Function::HasSourceFilePath() const
{
    return sourceFilePath.Value().AsStringLiteral() != nullptr;
}

void Function::Write(Writer& writer)
{
    writer.Put(uint8_t(flags));
    ConstantId groupNameId = constantPool->GetIdFor(groupName);
    Assert(groupNameId != noConstantId, "got no constant id");
    groupNameId.Write(writer);
    ConstantId callNameId = constantPool->GetIdFor(callName);
    Assert(callNameId != noConstantId, "got no constant id");
    callNameId.Write(writer);
    ConstantId fullNameId = constantPool->GetIdFor(fullName);
    Assert(fullNameId != noConstantId, "got no constant id");
    fullNameId.Write(writer);
    bool hasSourceFilePath = sourceFilePath.Value().AsStringLiteral() != nullptr;
    writer.Put(hasSourceFilePath);
    if (hasSourceFilePath)
    {
        ConstantId sourceFilePathId = constantPool->GetIdFor(sourceFilePath);
        Assert(sourceFilePathId != noConstantId, "got no constant id");
        sourceFilePathId.Write(writer);
    }
    writer.PutEncodedUInt(id);
    writer.Put(mangledName);
    writer.Put(mangledInlineName);
    uint32_t n = static_cast<uint32_t>(instructions.size());
    writer.PutEncodedUInt(n);
    for (uint32_t i = 0; i < n; ++i)
    {
        Instruction* inst = instructions[i].get();
        inst->Encode(writer);
    }
    writer.PutEncodedUInt(numLocals);
    for (uint32_t i = 0; i < numLocals; ++i)
    {
        ValueType valueType = localTypes[i];
        writer.Put(uint8_t(valueType));
    }
    writer.PutEncodedUInt(numParameters);
    for (uint32_t i = 0; i < numParameters; ++i)
    {
        ValueType valueType = parameterTypes[i];
        writer.Put(uint8_t(valueType));
    }
    writer.Put(returnsValue);
    writer.Put(uint8_t(returnType));
    uint32_t ne = uint32_t(exceptionBlocks.size());
    writer.PutEncodedUInt(ne);
    for (const std::unique_ptr<ExceptionBlock>& exceptionBlock : exceptionBlocks)
    {
        exceptionBlock->Write(writer);
    }
    uint32_t npc = uint32_t(pcSourceLineMap.size());
    writer.PutEncodedUInt(npc);
    for (const std::pair<uint32_t, uint32_t>& p : pcSourceLineMap)
    {
        writer.PutEncodedUInt(p.first);
        writer.PutEncodedUInt(p.second);
    }
}

void Function::Read(Reader& reader)
{
    flags = FunctionFlags(reader.GetByte());
    constantPool = reader.GetConstantPool();
    assembly = reader.GetAssemblyAddress();
    ConstantId groupNameId;
    groupNameId.Read(reader);
    groupName = constantPool->GetConstant(groupNameId);
    ConstantId callNameId;
    callNameId.Read(reader);
    callName = constantPool->GetConstant(callNameId);
    ConstantId fullNameId;
    fullNameId.Read(reader);
    fullName = constantPool->GetConstant(fullNameId);
    bool hasSourceFilePath = reader.GetBool();
    if (hasSourceFilePath)
    {
        ConstantId sourceFilePathId;
        sourceFilePathId.Read(reader);
        sourceFilePath = constantPool->GetConstant(sourceFilePathId);
    }
    id = reader.GetEncodedUInt();
    mangledName = reader.GetUtf8String();
    mangledInlineName = reader.GetUtf8String();
    uint32_t n = reader.GetEncodedUInt();
    for (uint32_t i = 0; i < n; ++i)
    {
        AddInst(reader.GetMachine().DecodeInst(reader));
    }
    numLocals = reader.GetEncodedUInt();
    for (uint32_t i = 0; i < numLocals; ++i)
    {
        ValueType valueType = static_cast<ValueType>(reader.GetByte());
        localTypes.push_back(valueType);
    }
    numParameters = reader.GetEncodedUInt();
    for (uint32_t i = 0; i < numParameters; ++i)
    {
        ValueType valueType = static_cast<ValueType>(reader.GetByte());
        parameterTypes.push_back(valueType);
    }
    returnsValue = reader.GetBool();
    returnType = static_cast<ValueType>(reader.GetByte());
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
        if (SourceFileTable::Initialized() && HasSourceFilePath() && sourceFilePath.Value().AsStringLiteral() != U"")
        {
            SourceFileTable::MapSourceFileLine(sourceFilePath, sourceLine, this);
        }
    }
}

void Function::SetNumLocals(uint32_t numLocals_)
{
    numLocals = numLocals_;
    localTypes.resize(numLocals);
}

void Function::SetNumParameters(uint32_t numParameters_)
{
    numParameters = numParameters_;
    parameterTypes.resize(numParameters);
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
        emitter->AddingInst(instructionIndex);
    }
    instructions.push_back(std::move(inst));
}

void Function::SetInst(int32_t index, std::unique_ptr<Instruction>&& inst)
{
    instructions[index] = std::move(inst);
}

void Function::Dump(CodeFormatter& formatter)
{
    formatter.WriteLine("FUNCTION #" + std::to_string(id));
    formatter.WriteLine("call name: " + ToUtf8(callName.Value().AsStringLiteral()));
    formatter.WriteLine("group name: " + ToUtf8(groupName.Value().AsStringLiteral()));
    formatter.WriteLine("full name: " + ToUtf8(fullName.Value().AsStringLiteral()));
    if (!mangledName.empty())
    {
        formatter.WriteLine("mangled name: " + mangledName);
    }
    if (!mangledInlineName.empty())
    {
        formatter.WriteLine("mangled inline name: " + mangledInlineName);
    }
    formatter.WriteLine("flags: " + FunctionFlagsStr(flags));
    if (sourceFilePath.Value().AsStringLiteral())
    {
        formatter.WriteLine("source file path: " + ToUtf8(sourceFilePath.Value().AsStringLiteral()));
    }
    formatter.WriteLine(std::to_string(numParameters) + " parameters:");
    int np = int(parameterTypes.size());
    for (int i = 0; i < np; ++i)
    {
        formatter.WriteLine("parameter " + std::to_string(i) + ": " + ValueTypeStr(parameterTypes[i]));
    }
    formatter.WriteLine(std::to_string(numLocals) + " locals:");
    int nl = int(localTypes.size());
    for (int i = 0; i < nl; ++i)
    {
        std::string kind;
        if (i < np)
        {
            kind.append(" for parameter " + std::to_string(i));
        }
        formatter.WriteLine("local " + std::to_string(i) + ": " + ValueTypeStr(localTypes[i]) + kind);
    }
    formatter.WriteLine("returns: " + ValueTypeStr(returnType));
    if (!exceptionBlocks.empty())
    {
        formatter.WriteLine("exception blocks:");
        for (const std::unique_ptr<ExceptionBlock>& exceptionBlock : exceptionBlocks)
        {
            exceptionBlock->Dump(formatter);
        }
    }
    formatter.WriteLine("code:");
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
    formatter.WriteLine();
}

void Function::Dump(CodeFormatter& formatter, int32_t pc)
{
    if (pc == 0)
    {
        if (HasSourceFilePath() && sourceFilePath.Value().AsStringLiteral() != U"")
        {
            std::string sfp = ToUtf8(sourceFilePath.Value().AsStringLiteral());
            WriteInGreenToConsole(sfp + ":");
        }
        formatter.WriteLine(ToUtf8(callName.Value().AsStringLiteral()) + " [" + ToUtf8(fullName.Value().AsStringLiteral()) + "]");
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
        if (SourceFileTable::Initialized())
        {
            uint32_t sline = GetSourceLine(i);
            if (sline != -1)
            {
                SourceFile* sourceFile = SourceFileTable::GetSourceFile(sourceFilePath);
                std::string line = sourceFile->GetLine(sline);
                if (!line.empty())
                {
                    WriteInGreenToConsole(line);
                }
            }
        }
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
    if (mappedSourceLines.find(sourceLine) == mappedSourceLines.cend())
    {
        auto pit = pcSourceLineMap.find(pc);
        if (pit == pcSourceLineMap.cend())
        {
            pcSourceLineMap[pc] = sourceLine;
            mappedSourceLines.insert(sourceLine);
        }
    }
    auto sit = sourceLinePCMap.find(sourceLine);
    if (sit == sourceLinePCMap.cend())
    {
        sourceLinePCMap[sourceLine] = pc;
    }
}

uint32_t Function::GetFirstSourceLine() const
{
    for (const std::pair<uint32_t, uint32_t>& p : pcSourceLineMap)
    {
        uint32_t sourceLine = p.second;
        if (sourceLine != uint32_t(-1))
        {
            return sourceLine;
        }
    }
    return uint32_t(-1);
}

uint32_t Function::GetSourceLine(uint32_t pc) const
{
    auto it = pcSourceLineMap.find(pc);
    if (it != pcSourceLineMap.cend())
    {
        return it->second;
    }
    return uint32_t(-1);
}

uint32_t Function::GetPC(uint32_t sourceLine) const
{
    auto it = sourceLinePCMap.find(sourceLine);
    if (it != sourceLinePCMap.cend())
    {
        return it->second;
    }
    return uint32_t(-1);
}

bool Function::HasBreakPointAt(uint32_t pc) const
{
    return breakPoints.find(pc) != breakPoints.cend();
}

void Function::SetBreakPointAt(uint32_t pc)
{
    breakPoints.insert(pc);
}

void Function::RemoveBreakPointAt(uint32_t pc)
{
    breakPoints.erase(pc);
}

enum class JumpTargetType : uint8_t
{
    weak, strong
};

void Function::RemoveUnreachableInstructions(std::unordered_set<int32_t>& jumpTargets)
{
    std::unordered_map<int32_t, JumpTargetType> jumpTargetMap;
    std::unordered_set<int32_t> toBeRemoved;
    for (const std::unique_ptr<ExceptionBlock>& exceptionBlock : exceptionBlocks)
    {
        for (const std::unique_ptr<CatchBlock>& catchBlock : exceptionBlock->CatchBlocks())
        {
            jumpTargetMap[catchBlock->CatchBlockStart()] = JumpTargetType::strong;
        }
        if (exceptionBlock->HasFinally())
        {
            jumpTargetMap[exceptionBlock->FinallyStart()] = JumpTargetType::strong;
        }
        if (exceptionBlock->NextTarget() != endOfFunction)
        {
            jumpTargetMap[exceptionBlock->NextTarget()] = JumpTargetType::strong;
        }
    }
    int32_t n = int32_t(instructions.size());
    for (int32_t i = 0; i < n; ++i)
    {
        Instruction* inst = instructions[i].get();
        if (inst->IsJumpingInst())
        {
            IndexParamInst* indexParamInst = static_cast<IndexParamInst*>(inst);
            int32_t jumpTarget = indexParamInst->Index();
            if (jumpTarget != endOfFunction)
            {
                auto it = jumpTargetMap.find(jumpTarget);
                if (it == jumpTargetMap.cend())
                {
                    jumpTargetMap[jumpTarget] = JumpTargetType::weak;
                }
            }
        }
        else if (inst->IsContinuousSwitchInst())
        {
            ContinuousSwitchInst* cswitch = static_cast<ContinuousSwitchInst*>(inst);
            for (int32_t target : cswitch->Targets())
            {
                jumpTargetMap[target] = JumpTargetType::strong;
            }
            jumpTargetMap[cswitch->DefaultTarget()] = JumpTargetType::strong;
        }
        else if (inst->IsBinarySearchSwitchInst())
        {
            BinarySearchSwitchInst* bswitch = static_cast<BinarySearchSwitchInst*>(inst);
            for (const auto t : bswitch->Targets())
            {
                int32_t target = t.second;
                jumpTargetMap[target] = JumpTargetType::strong;
            }
            jumpTargetMap[bswitch->DefaultTarget()] = JumpTargetType::strong;
        }
    }
    bool prevEndsBasicBlock = false;
    bool prevWasEndEhInst = false;
    bool removeInst = false;
    for (int32_t i = 0; i < n; ++i)
    {
        Instruction* inst = instructions[i].get();
        if (prevWasEndEhInst)
        {
            removeInst = false;
        }
        else if (prevEndsBasicBlock)
        {
            if (jumpTargetMap.find(i) == jumpTargetMap.cend())
            {
                removeInst = true;
            }
        }
        if (removeInst)
        {
            if (inst->IsJumpingInst())
            {
                IndexParamInst* indexParamInst = static_cast<IndexParamInst*>(inst);
                int32_t jumpTarget = indexParamInst->Index();
                if (jumpTarget != endOfFunction)
                {
                    auto it = jumpTargetMap.find(jumpTarget);
                    if (it != jumpTargetMap.cend())
                    {
                        if (it->second == JumpTargetType::weak)
                        {
                            jumpTargetMap.erase(jumpTarget);
                        }
                    }
                }
            }
        }
        else
        {
            if (inst->IsJumpingInst())
            {
                IndexParamInst* indexParamInst = static_cast<IndexParamInst*>(inst);
                int32_t jumpTarget = indexParamInst->Index();
                if (jumpTarget != endOfFunction)
                {
                    jumpTargetMap[jumpTarget] = JumpTargetType::strong;
                }
            }
        }
        if (jumpTargetMap.find(i) != jumpTargetMap.cend())
        {
            removeInst = false;
        }
        if (removeInst && !inst->DontRemove())
        {
            toBeRemoved.insert(i);
        }
        prevEndsBasicBlock = inst->EndsBasicBlock();
        prevWasEndEhInst = inst->IsEndEhInst();
    }
    if (!toBeRemoved.empty())
    {
        std::vector<std::unique_ptr<Instruction>> cleanedInsts;
        std::vector<int32_t> instructionOffsets;
        int32_t offset = 0;
        for (int32_t i = 0; i < n; ++i)
        {
            instructionOffsets.push_back(offset);
            if (toBeRemoved.find(i) != toBeRemoved.cend())
            {
                ++offset;
            }
            else
            {
                cleanedInsts.push_back(std::unique_ptr<Instruction>(instructions[i].release()));
            }
        }
        for (const std::unique_ptr<ExceptionBlock>& exceptionBlock : exceptionBlocks)
        {
            exceptionBlock->Adjust(instructionOffsets, jumpTargets);
        }
        AdjustPCSourceLineMap(instructionOffsets);
        AdjustSourceLinePCMap(instructionOffsets);
        int32_t m = int32_t(cleanedInsts.size());
        for (int32_t i = 0; i < m; ++i)
        {
            Instruction* inst = cleanedInsts[i].get();
            if (inst->IsJumpingInst())
            {
                IndexParamInst* indexParamInst = static_cast<IndexParamInst*>(inst);
                int32_t jumpTarget = indexParamInst->Index();
                if (jumpTarget != endOfFunction)
                {
                    int32_t newTarget = jumpTarget - instructionOffsets[jumpTarget];
                    indexParamInst->SetIndex(newTarget);
                    jumpTargets.insert(newTarget);
                }
            }
            else if (inst->IsContinuousSwitchInst())
            {
                ContinuousSwitchInst* cswitch = static_cast<ContinuousSwitchInst*>(inst);
                std::vector<int32_t>& targets = cswitch->Targets();
                for (int32_t& target : targets)
                {
                    target = target - instructionOffsets[target];
                    jumpTargets.insert(target);
                }
                cswitch->SetDefaultTarget(cswitch->DefaultTarget() - instructionOffsets[cswitch->DefaultTarget()]);
                jumpTargets.insert(cswitch->DefaultTarget());
            }
            else if (inst->IsBinarySearchSwitchInst())
            {
                BinarySearchSwitchInst* bswitch = static_cast<BinarySearchSwitchInst*>(inst);
                std::vector<std::pair<IntegralValue, int32_t>>& targets = bswitch->Targets();
                for (std::pair<IntegralValue, int32_t>& t : targets)
                {
                    int32_t& target = t.second;
                    target = target - instructionOffsets[target];
                    jumpTargets.insert(target);
                }
                bswitch->SetDefaultTarget(bswitch->DefaultTarget() - instructionOffsets[bswitch->DefaultTarget()]);
                jumpTargets.insert(bswitch->DefaultTarget());
            }
        }
        std::swap(instructions, cleanedInsts);
    }
    else
    {
        for (const std::pair<int32_t, JumpTargetType>& p : jumpTargetMap)
        {
            jumpTargets.insert(p.first);
        }
    }
}

void Function::AdjustPCSourceLineMap(const std::vector<int32_t>& instructionOffsets)
{
    std::vector<std::pair<int32_t, int32_t>> pcLine;
    for (const std::pair<int32_t, int32_t>& p : pcSourceLineMap)
    {
        pcLine.push_back(p);
    }
    for (std::pair<int32_t, int32_t>& p : pcLine)
    {
        int32_t& pc = p.first;
        pc = pc - instructionOffsets[pc];
    }
    pcSourceLineMap.clear();
    for (const std::pair<int32_t, int32_t>& p : pcLine)
    {
        pcSourceLineMap[p.first] = p.second;
    }
}

void Function::AdjustSourceLinePCMap(const std::vector<int32_t>& instructionOffsets)
{
    std::vector<std::pair<int32_t, int32_t>> linePC;
    for (const std::pair<int32_t, int32_t>& p : sourceLinePCMap)
    {
        linePC.push_back(p);
    }
    for (std::pair<int32_t, int32_t>& p : linePC)
    {
        int32_t& pc = p.second;
        pc = pc - instructionOffsets[pc];
    }
    sourceLinePCMap.clear();
    for (const std::pair<int32_t, int32_t>& p : linePC)
    {
        sourceLinePCMap[p.first] = p.second;
    }
}

void Function::Accept(MachineFunctionVisitor& visitor)
{
    visitor.BeginVisitFunction(*this);
    int currentInstructionIndex = 0;
    bool prevEndsBasicBlock = false;
    for (const std::unique_ptr<Instruction>& instruction : instructions)
    {
        visitor.SetCurrentInstructionIndex(currentInstructionIndex);
        visitor.BeginVisitInstruction(currentInstructionIndex, prevEndsBasicBlock, instruction.get());
        instruction->Accept(visitor);
        prevEndsBasicBlock = instruction->EndsBasicBlock();
        ++currentInstructionIndex;
    }
    visitor.EndVisitFunction(*this);
}

class FunctionTableImpl
{
public:
    FunctionTableImpl();
    static void Init();
    static void Done();
    static FunctionTableImpl& Instance() { Assert(instance, "function table impl not initialized");  return *instance; }
    void AddFunction(Function* fun, bool memberOfClassTemplateSpecialization);
    Function* GetFunction(StringPtr functionCallName) const;
    Function* GetFunctionNothrow(StringPtr functionCallName) const;
    Function* GetMain() const { return main; }
    void ResolveExceptionVarTypes();
private:
    static std::unique_ptr<FunctionTableImpl> instance;
    std::unordered_map<StringPtr, Function*, StringPtrHash> functionMap;
    Function* main;
};

FunctionTableImpl::FunctionTableImpl() : main(nullptr)
{
}

void FunctionTableImpl::AddFunction(Function* fun, bool memberOfClassTemplateSpecialization)
{
    StringPtr functionCallName = fun->CallName().Value().AsStringLiteral();
    auto it = functionMap.find(functionCallName);
    if (it != functionMap.cend())
    {
        if (!memberOfClassTemplateSpecialization)
        {
            throw std::runtime_error("function '" + ToUtf8(functionCallName.Value()) + "' already added to function table");
        }
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

Function* FunctionTableImpl::GetFunction(StringPtr functionCallName) const
{
    Function* function = GetFunctionNothrow(functionCallName);
    if (function)
    {
        return function;
    }
    else
    {
        throw std::runtime_error("function '" + ToUtf8(functionCallName.Value()) + "' not found from function table");
    }
}

Function* FunctionTableImpl::GetFunctionNothrow(StringPtr functionCallName) const
{
    auto it = functionMap.find(functionCallName);
    if (it != functionMap.cend())
    {
        return it->second;
    }
    else
    {
        return nullptr;
    }
}

void FunctionTableImpl::ResolveExceptionVarTypes()
{
    for (const auto& p : functionMap)
    {
        p.second->ResolveExceptionVarTypes();
    }
}

std::unique_ptr<FunctionTableImpl> FunctionTableImpl::instance;

void FunctionTableImpl::Init()
{
    instance.reset(new FunctionTableImpl());
}

void FunctionTableImpl::Done()
{
    instance.reset();
}

MACHINE_API void FunctionTable::Init()
{
    FunctionTableImpl::Init();
}

MACHINE_API void FunctionTable::Done()
{
    FunctionTableImpl::Done();
}

MACHINE_API void FunctionTable::AddFunction(Function* fun, bool memberOfClassTemplateSpecialization)
{
    FunctionTableImpl::Instance().AddFunction(fun, memberOfClassTemplateSpecialization);
}

MACHINE_API Function* FunctionTable::GetFunction(StringPtr functionCallName)
{
    return FunctionTableImpl::Instance().GetFunction(functionCallName);
}

MACHINE_API Function* FunctionTable::GetFunctionNothrow(StringPtr functionCallName)
{
    return FunctionTableImpl::Instance().GetFunctionNothrow(functionCallName);
}

MACHINE_API Function* FunctionTable::GetMain()
{
    return FunctionTableImpl::Instance().GetMain();
}

MACHINE_API void FunctionTable::ResolveExceptionVarTypes()
{
    FunctionTableImpl::Instance().ResolveExceptionVarTypes();
}

VmFunction::~VmFunction()
{
}

class VmFunctionTableImpl
{
public:
    static void Init();
    static void Done();
    static VmFunctionTableImpl& Instance() { Assert(instance, "virtual machine function table impl not initialized"); return *instance; }
    void RegisterVmFunction(VmFunction* vmFunction);
    VmFunction* GetVmFunction(StringPtr vmFuntionName) const;
private:
    static std::unique_ptr<VmFunctionTableImpl> instance;
    std::unordered_map<StringPtr, VmFunction*, StringPtrHash> vmFunctionMap;
};

void VmFunctionTableImpl::Init()
{
    instance.reset(new VmFunctionTableImpl());
}

void VmFunctionTableImpl::Done()
{
    instance.reset();
}

void VmFunctionTableImpl::RegisterVmFunction(VmFunction* vmFunction)
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

VmFunction* VmFunctionTableImpl::GetVmFunction(StringPtr vmFuntionName) const
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

std::unique_ptr<VmFunctionTableImpl> VmFunctionTableImpl::instance;

MACHINE_API void VmFunctionTable::Init()
{
    VmFunctionTableImpl::Init();
}

MACHINE_API void VmFunctionTable::Done()
{
    VmFunctionTableImpl::Done();
}

MACHINE_API void VmFunctionTable::RegisterVmFunction(VmFunction* vmFunction)
{
    VmFunctionTableImpl::Instance().RegisterVmFunction(vmFunction);
}

MACHINE_API VmFunction* VmFunctionTable::GetVmFunction(StringPtr vmFunctionName)
{
    return VmFunctionTableImpl::Instance().GetVmFunction(vmFunctionName);
}

LineFunctionTable::LineFunctionTable()
{
}

void LineFunctionTable::MapSourceLineToFunction(uint32_t sourceLine, Function* function)
{
    lineFunctionMap[sourceLine] = function;
}

Function* LineFunctionTable::GetFunction(uint32_t sourceLine) const
{
    auto it = lineFunctionMap.find(sourceLine);
    if (it != lineFunctionMap.cend())
    {
        return it->second;
    }
    return nullptr;
}

BreakPoint::BreakPoint() : function(nullptr), line(-1), pc(-1)
{
}

BreakPoint::BreakPoint(Function* function_, uint32_t line_, uint32_t pc_) : function(function_), line(line_), pc(pc_)
{
}

void BreakPoint::Remove()
{
    function->RemoveBreakPointAt(pc);
}

SourceFile::SourceFile(Constant sourceFilePath_) : sourceFilePath(sourceFilePath_)
{
}

void SourceFile::Read()
{
    std::string filePath = ToUtf8(sourceFilePath.Value().AsStringLiteral());
    MappedInputFile mappedFile(filePath);
    std::string line;
    for (const char* p = mappedFile.Begin(); p != mappedFile.End(); ++p)
    {
        char c = *p;
        if (c == '\n')
        {
            lines.push_back(line);
            line.clear();
        }
        else if (c != '\r')
        {
            line.append(1, c);
        }
    }
    if (!line.empty())
    {
        lines.push_back(line);
    }
}

void SourceFile::List(int lineNumber, int numLines)
{
    int start = lineNumber - 1;
    int n = std::min(start + numLines, int(lines.size()));
    for (int i = start; i < n; ++i)
    {
        int ln = lineNumber + i - start;
        std::string line = GetLine(ln);
        WriteInGreenToConsole(line);
    }
}

std::string SourceFile::GetLine(int lineNumber) const
{
    if (lineNumber - 1 < int(lines.size()))
    {
        std::string ln = std::to_string(lineNumber);
        while (ln.length() < 4)
        {
            ln.insert(ln.begin(), ' ');
        }
        ln.append(": ").append(lines[lineNumber - 1]);
        return ln;
    }
    else
    {
        return std::string();
    }
}

class SourceFileTableImpl
{
public:
    static bool Initialized();
    static SourceFileTableImpl* Instance();
    static void Init();
    static void Done();
    Constant GetSourceFilePath(const std::string& sourceFileName) const;
    SourceFile* GetSourceFile(Constant sourceFilePath);
    SourceFile* GetSourceFile(const std::string& sourceFileName);
    void MapSourceFileLine(Constant sourceFilePath, uint32_t sourceLine, Function* function);
    int SetBreakPoint(Constant sourceFilePath, uint32_t sourceLine);
    int SetBreakPoint(const std::string& sourceFileName, uint32_t sourceLine);
    void RemoveBreakPoint(int bp);
    const BreakPoint* GetBreakPoint(int bp) const;
private:
    SourceFileTableImpl();
    static std::unique_ptr<SourceFileTableImpl> instance;
    std::unordered_map<Constant, LineFunctionTable, ConstantHash> sourceFileLineFunctionMap;
    std::unordered_map<Constant, std::unique_ptr<SourceFile>, ConstantHash> sourceFileMap;
    std::unordered_map<int, BreakPoint> breakPoints;
    int nextBp;
};

std::unique_ptr<SourceFileTableImpl> SourceFileTableImpl::instance;

SourceFileTableImpl::SourceFileTableImpl() : nextBp(1)
{
}

bool SourceFileTableImpl::Initialized() 
{ 
    return Instance() != nullptr; 
}

SourceFileTableImpl* SourceFileTableImpl::Instance()
{
    return instance.get();
}

void SourceFileTableImpl::Init()
{
    instance.reset(new SourceFileTableImpl());
}

void SourceFileTableImpl::Done()
{
    instance.reset();
}

Constant SourceFileTableImpl::GetSourceFilePath(const std::string& sourceFileName) const
{
    utf32_string sfp = ToUtf32(sourceFileName);
    std::vector<utf32_string> components = Split(sfp, char32_t('/'));
    int cn = int(components.size());
    std::vector<Constant> sourceFilePaths;
    for (const std::pair<Constant, const LineFunctionTable&>& p : sourceFileLineFunctionMap)
    {
        Constant c = p.first;
        utf32_string s = c.Value().AsStringLiteral();
        std::vector<utf32_string> v = Split(s, char32_t('/'));
        int vn = int(v.size());
        int n = std::min(cn, vn);
        if (n > 0)
        {
            bool match = true;
            for (int i = 0; i < n; ++i)
            {
                if (components[cn - i - 1] != v[vn - i - 1])
                {
                    match = false;
                    break;
                }
            }
            if (match)
            {
                sourceFilePaths.push_back(c);
            }
        }
    }
    if (sourceFilePaths.size() > 1)
    {
        throw std::runtime_error("more than one source file path matched \"" + sourceFileName + "\", specify more directory components");
    }
    else if (sourceFilePaths.empty())
    {
        throw std::runtime_error("source file \"" + sourceFileName + "\" not found");
    }
    else
    {
        Constant sourceFilePath = sourceFilePaths.front();
        return sourceFilePath;
    }
}

SourceFile* SourceFileTableImpl::GetSourceFile(Constant sourceFilePath)
{
    auto it = sourceFileMap.find(sourceFilePath);
    if (it != sourceFileMap.cend())
    {
        return it->second.get();
    }
    std::unique_ptr<SourceFile> sourceFile(new SourceFile(sourceFilePath));
    sourceFile->Read();
    SourceFile* sf = sourceFile.get();
    sourceFileMap[sourceFilePath] = std::move(sourceFile);
    return sf;
}

SourceFile* SourceFileTableImpl::GetSourceFile(const std::string& sourceFileName)
{
    return GetSourceFile(GetSourceFilePath(sourceFileName));
}

void SourceFileTableImpl::MapSourceFileLine(Constant sourceFilePath, uint32_t sourceLine, Function* function)
{
    LineFunctionTable& lineFunctionTable = sourceFileLineFunctionMap[sourceFilePath];
    lineFunctionTable.MapSourceLineToFunction(sourceLine, function);
}

int SourceFileTableImpl::SetBreakPoint(Constant sourceFilePath, uint32_t sourceLine)
{
    const LineFunctionTable& lineFunctionTable = sourceFileLineFunctionMap[sourceFilePath];
    Function* function = lineFunctionTable.GetFunction(sourceLine);
    if (!function)
    {
        throw std::runtime_error("no function matched source line " + std::to_string(sourceLine));
    }
    uint32_t pc = function->GetPC(sourceLine);
    if (pc == -1)
    {
        throw std::runtime_error("no instruction index matched source line " + std::to_string(sourceLine) + " in function '" + ToUtf8(function->CallName().Value().AsStringLiteral()) + "'");
    }
    int bp = nextBp++;
    function->SetBreakPointAt(pc);
    breakPoints[bp] = BreakPoint(function, sourceLine, pc);
    return bp;
}

int SourceFileTableImpl::SetBreakPoint(const std::string& sourceFileName, uint32_t sourceLine)
{
    Constant sourceFilePath = GetSourceFilePath(sourceFileName);
    return SetBreakPoint(sourceFilePath, sourceLine);
}

void SourceFileTableImpl::RemoveBreakPoint(int bp)
{
    auto it = breakPoints.find(bp);
    if (it != breakPoints.cend())
    {
        BreakPoint& breakPoint = it->second;
        breakPoint.Remove();
        breakPoints.erase(bp);
    }
    else
    {
        throw std::runtime_error("breakpoint number " + std::to_string(bp) + " not found");
    }
}

const BreakPoint* SourceFileTableImpl::GetBreakPoint(int bp) const
{
    auto it = breakPoints.find(bp);
    if (it != breakPoints.cend())
    {
        return &it->second;
    }
    else
    {
        throw std::runtime_error("breakpoint number " + std::to_string(bp) + " not found");
    }
}

MACHINE_API bool SourceFileTable::Initialized()
{
    return SourceFileTableImpl::Initialized();
}

MACHINE_API void SourceFileTable::Init()
{
    SourceFileTableImpl::Init();
}

MACHINE_API void SourceFileTable::Done()
{
    SourceFileTableImpl::Done();
}

MACHINE_API Constant SourceFileTable::GetSourceFilePath(const std::string& sourceFileName)
{
    return SourceFileTableImpl::Instance()->GetSourceFilePath(sourceFileName);
}

MACHINE_API SourceFile* SourceFileTable::GetSourceFile(Constant sourceFilePath)
{
    return SourceFileTableImpl::Instance()->GetSourceFile(sourceFilePath);
}

MACHINE_API SourceFile* SourceFileTable::GetSourceFile(const std::string& sourceFileName)
{
    return SourceFileTableImpl::Instance()->GetSourceFile(sourceFileName);
}

MACHINE_API void SourceFileTable::MapSourceFileLine(Constant sourceFilePath, uint32_t sourceLine, Function* function)
{
    SourceFileTableImpl::Instance()->MapSourceFileLine(sourceFilePath, sourceLine, function);
}

MACHINE_API int SourceFileTable::SetBreakPoint(Constant sourceFilePath, uint32_t sourceLine)
{
    return SourceFileTableImpl::Instance()->SetBreakPoint(sourceFilePath, sourceLine);
}

MACHINE_API int SourceFileTable::SetBreakPoint(const std::string& sourceFileName, uint32_t sourceLine)
{
    return SourceFileTableImpl::Instance()->SetBreakPoint(sourceFileName, sourceLine);
}

MACHINE_API void SourceFileTable::RemoveBreakPoint(int bp)
{
    return SourceFileTableImpl::Instance()->RemoveBreakPoint(bp);
}

MACHINE_API const BreakPoint* SourceFileTable::GetBreakPoint(int bp)
{
    return SourceFileTableImpl::Instance()->GetBreakPoint(bp);
}

} } // namespace cminor::machine
