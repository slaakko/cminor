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

Function::Function() : fullName(), id(-1), numLocals(0), numParameters(0), constantPool(nullptr), isMain(false), emitter(nullptr)
{
}

Function::Function(Constant fullName_, int32_t id_, ConstantPool* constantPool_) : 
    fullName(fullName_), id(id_), numLocals(0), numParameters(0), constantPool(constantPool_), isMain(false), emitter(nullptr)
{
}

void Function::Write(Writer& writer)
{
    ConstantId fullNameId = writer.GetConstantPool()->GetIdFor(fullName);
    Assert(fullNameId != noConstantId, "got no constant id");
    fullNameId.Write(writer);
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
    ConstantId fullNameId;
    fullNameId.Read(reader);
    fullName = constantPool->GetConstant(fullNameId);
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
    formatter.WriteLine(ToUtf8(fullName.Value().AsStringLiteral()));
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
    StringPtr functionFullName = fun->FullName().Value().AsStringLiteral();
    auto it = functionMap.find(functionFullName);
    if (it != functionMap.cend())
    {
        throw std::runtime_error("function '" + ToUtf8(functionFullName.Value()) + "' already added to function table");
    }
    else
    {
        functionMap[functionFullName] = fun;
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

Function* FunctionTable::GetFunction(StringPtr functionFullName) const
{
    auto it = functionMap.find(functionFullName);
    if (it != functionMap.cend())
    {
        return it->second;
    }
    else
    {
        throw std::runtime_error("function '" + ToUtf8(functionFullName.Value()) + "' not found from function table");
    }
}

} } // namespace cminor::machine
