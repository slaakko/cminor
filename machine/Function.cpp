// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Function.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Unicode.hpp>
#include <cminor/machine/Util.hpp>
#include <cminor/machine/OsInterface.hpp>
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
        SourceFileTable* sourceFileTable = SourceFileTable::Instance();
        if (sourceFileTable && HasSourceFilePath() && sourceFilePath.Value().AsStringLiteral() != U"")
        {
            sourceFileTable->MapSourceFileLine(sourceFilePath, sourceLine, this);
        }
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
        if (HasSourceFilePath() && sourceFilePath.Value().AsStringLiteral() != U"")
        {
            std::string sfp = ToUtf8(sourceFilePath.Value().AsStringLiteral());
            WriteInGreenToConsole(sfp + ":");
        }
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
        SourceFileTable* sourceFileTable = SourceFileTable::Instance();
        if (sourceFileTable)
        {
            uint32_t sline = GetSourceLine(i);
            if (sline != -1)
            {
                SourceFile* sourceFile = sourceFileTable->GetSourceFile(sourceFilePath);
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

uint32_t Function::GetSourceLine(uint32_t pc) const
{
    auto it = pcSourceLineMap.find(pc);
    if (it != pcSourceLineMap.cend())
    {
        return it->second;
    }
    return -1;
}

uint32_t Function::GetPC(uint32_t sourceLine) const
{
    auto it = sourceLinePCMap.find(sourceLine);
    if (it != sourceLinePCMap.cend())
    {
        return it->second;
    }
    return -1;
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

void FunctionTable::AddFunction(Function* fun, bool memberOfClassTemplateSpecialization)
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

std::unique_ptr<SourceFileTable> SourceFileTable::instance;

SourceFileTable::SourceFileTable() : nextBp(1)
{
}

SourceFileTable* SourceFileTable::Instance()
{
    return instance.get();
}

void SourceFileTable::Init()
{
    instance.reset(new SourceFileTable());
}

void SourceFileTable::Done()
{
    instance.reset();
}

Constant SourceFileTable::GetSourceFilePath(const std::string& sourceFileName) const
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

SourceFile* SourceFileTable::GetSourceFile(Constant sourceFilePath)
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

SourceFile* SourceFileTable::GetSourceFile(const std::string& sourceFileName)
{
    return GetSourceFile(GetSourceFilePath(sourceFileName));
}

void SourceFileTable::MapSourceFileLine(Constant sourceFilePath, uint32_t sourceLine, Function* function)
{
    LineFunctionTable& lineFunctionTable = sourceFileLineFunctionMap[sourceFilePath];
    lineFunctionTable.MapSourceLineToFunction(sourceLine, function);
}

int SourceFileTable::SetBreakPoint(Constant sourceFilePath, uint32_t sourceLine)
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

int SourceFileTable::SetBreakPoint(const std::string& sourceFileName, uint32_t sourceLine)
{
    Constant sourceFilePath = GetSourceFilePath(sourceFileName);
    return SetBreakPoint(sourceFilePath, sourceLine);
}

void SourceFileTable::RemoveBreakPoint(int bp)
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

const BreakPoint* SourceFileTable::GetBreakPoint(int bp) const
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

} } // namespace cminor::machine
