// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Function.hpp>
#include <cminor/machine/Machine.hpp>

namespace cminor { namespace machine {

Function::Function(const std::string& fullName_) : id(-1), fullName(fullName_)
{
}

void Function::Write(Writer& writer)
{
    writer.Put(fullName);
    int32_t n = static_cast<int32_t>(instructions.size());
    writer.Put(n);
    for (int32_t i = 0; i < n; ++i)
    {
        Instruction* inst = instructions[i].get();
        inst->Encode(writer);
    }
}

void Function::Read(Reader& reader)
{
    fullName = reader.GetString();
    int32_t n = reader.GetInt();
    for (int32_t i = 0; i < n; ++i)
    {
        AddInst(reader.GetMachine().DecodeInst(reader));
    }
}

void Function::AddInst(std::unique_ptr<Instruction>&& inst)
{
    instructions.push_back(std::move(inst));
}

void Function::Dump(CodeFormatter& formatter)
{
    formatter.WriteLine(fullName);
    formatter.WriteLine("{");
    formatter.IncIndent();
    int32_t n = static_cast<int32_t>(instructions.size());
    for (int32_t i = 0; i < n; ++i)
    {
        Instruction* inst = instructions[i].get();
        formatter.Write(std::to_string(i) + " ");
        inst->Dump(formatter);
        formatter.WriteLine();
    }
    formatter.DecIndent();
    formatter.WriteLine("}");
}

} } // namespace cminor::machine
