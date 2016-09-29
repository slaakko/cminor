// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_FUNCTION_INCLUDED
#define CMINOR_MACHINE_FUNCTION_INCLUDED
#include <cminor/machine/Instruction.hpp>
#include <ostream>

namespace cminor { namespace machine {

class Assembly;

class Function
{
public:
    Function(const std::string& fullName_);
    void Write(Writer& writer);
    void Read(Reader& reader);
    void AddInst(std::unique_ptr<Instruction>&& inst);
    void Dump(CodeFormatter& formatter);
    int32_t NumLocals() const { return numLocals; }
    void SetNumLocals(int32_t numLocals_);
    int NumInsts() const { return int(instructions.size()); }
    Instruction* GetInst(int index) const { return instructions[index].get(); }
    Assembly* GetAssembly() const { assert(assembly, "assembly not set for function '" + fullName + "'"); return assembly; }
    void SetAssembly(Assembly* assembly_) { assembly = assembly_; }
private:
    std::string fullName;
    std::vector<std::unique_ptr<Instruction>> instructions;
    int32_t numLocals;
    Assembly* assembly;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_FUNCTION_INCLUDED
