// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_MACHINE_INCLUDED
#define CMINOR_MACHINE_MACHINE_INCLUDED
#include <cminor/machine/Assembly.hpp>
#include <cminor/machine/Instruction.hpp>
#include <unordered_map>

namespace cminor { namespace machine {

class Machine
{
public:
    Machine();
    void AddInst(Instruction* inst);
    std::unique_ptr<Instruction> CreateInst(const std::string& instName) const;
    std::unique_ptr<Instruction> DecodeInst(Reader& reader);
    std::unique_ptr<Assembly> ReadAssemblyFile(const std::string& fileName_);
private:
    GroupInst rootInst;
    std::unordered_map<std::string, Instruction*> instructionMap;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_MACHINE_INCLUDED
