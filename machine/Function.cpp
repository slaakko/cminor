// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Function.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Unicode.hpp>

namespace cminor { namespace machine {

Emitter::~Emitter()
{
}

Function::Function() : callName(), friendlyName(), id(-1), numLocals(0), numParameters(0), constantPool(nullptr), isMain(false), emitter(nullptr)
{
}

Function::Function(Constant callName_, Constant friendlyName_, int32_t id_, ConstantPool* constantPool_) : 
    callName(callName_), friendlyName(friendlyName_), id(id_), numLocals(0), numParameters(0), constantPool(constantPool_), isMain(false), emitter(nullptr)
{
}

void Function::Write(Writer& writer)
{
    ConstantId callNameId = constantPool->GetIdFor(callName);
    Assert(callNameId != noConstantId, "got no constant id");
    callNameId.Write(writer);
    ConstantId friendlyNameId = constantPool->GetIdFor(friendlyName);
    Assert(friendlyNameId != noConstantId, "got no constant id");
    friendlyNameId.Write(writer);
    writer.Put(id);
    int32_t n = static_cast<int32_t>(instructions.size());
    writer.Put(n);
    for (int32_t i = 0; i < n; ++i)
    {
        Instruction* inst = instructions[i].get();
        inst->Encode(writer);
    }
    writer.Put(numLocals);
    writer.Put(numParameters);
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
    id = reader.GetInt();
    int32_t n = reader.GetInt();
    for (int32_t i = 0; i < n; ++i)
    {
        AddInst(reader.GetMachine().DecodeInst(reader));
    }
    numLocals = reader.GetInt();
    numParameters = reader.GetInt();
}

void Function::SetNumLocals(int32_t numLocals_)
{
    numLocals = numLocals_;
}

void Function::SetNumParameters(int32_t numParameters_)
{
    numParameters = numParameters_;
}

void Function::AddInst(std::unique_ptr<Instruction>&& inst)
{
    if (emitter)
    {
        int32_t firstInstIndex = emitter->FistInstIndex();
        if (firstInstIndex == endOfFunction)
        {
            emitter->SetFirstInstIndex(int32_t(instructions.size()));
        }
    }
    instructions.push_back(std::move(inst));
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

std::unique_ptr<FunctionTable> FunctionTable::instance;

void FunctionTable::Init()
{
    instance.reset(new FunctionTable());
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

} } // namespace cminor::machine
