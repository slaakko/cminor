// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_FUNCTION_INCLUDED
#define CMINOR_MACHINE_FUNCTION_INCLUDED
#include <cminor/machine/Instruction.hpp>
#include <ostream>

namespace cminor { namespace machine {

class Function
{
public:
    Function(const std::string& fullName_);
    void Write(Writer& writer);
    void Read(Reader& reader);
    void AddInst(std::unique_ptr<Instruction>&& inst);
    void Dump(CodeFormatter& formatter);
private:
    int32_t id;
    std::string fullName;
    std::vector<std::unique_ptr<Instruction>> instructions;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_FUNCTION_INCLUDED
