// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_OPERAND_STACK_INCLUDED
#define CMINOR_MACHINE_OPERAND_STACK_INCLUDED
#include <cminor/machine/Error.hpp>
#include <stdint.h>
#include <vector>

namespace cminor { namespace machine {

class OperandStack
{
public:
    void Push(uint64_t value)
    {
        s.push_back(value);
    }
    uint64_t Pop()
    {
        assert(!s.empty(), "operand stack is empty");
        uint64_t top = s.back();
        s.pop_back();
        return top;
    }
private:
    std::vector<uint64_t> s;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_OPERAND_STACK_INCLUDED
