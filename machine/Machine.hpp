// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_MACHINE_INCLUDED
#define CMINOR_MACHINE_MACHINE_INCLUDED
#include <cminor/machine/Instruction.hpp>
#include <unordered_map>

namespace cminor { namespace machine {

class Machine
{
public:
    Machine();
    void AddInst(Instruction* inst);
    Instruction* GetInst(const std::string& instName) const;
private:
    GroupInst rootInst;
    std::unordered_map<std::string, Instruction*> instructionMap;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_MACHINE_INCLUDED
