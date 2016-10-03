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

class Function
{
public:
    Function(Constant fullName_);
    void Write(Writer& writer);
    void Read(Reader& reader);
    int32_t NumLocals() const { return numLocals; }
    void SetNumLocals(int32_t numLocals_);
    int NumInsts() const { return int(instructions.size()); }
    Instruction* GetInst(int index) const { return instructions[index].get(); }
    void AddInst(std::unique_ptr<Instruction>&& inst);
    void Dump(CodeFormatter& formatter);
private:
    Constant fullName;
    std::vector<std::unique_ptr<Instruction>> instructions;
    int32_t numLocals;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_FUNCTION_INCLUDED
