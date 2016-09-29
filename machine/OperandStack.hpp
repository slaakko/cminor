// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_OPERAND_STACK_INCLUDED
#define CMINOR_MACHINE_OPERAND_STACK_INCLUDED
#include <cminor/machine/Object.hpp>
#include <stdint.h>
#include <vector>

namespace cminor { namespace machine {

class OperandStack
{
public:
    void Push(IntegralValue value)
    {
        s.push_back(value);
    }
    IntegralValue Pop()
    {
        assert(!s.empty(), "operand stack is empty");
        IntegralValue top = s.back();
        s.pop_back();
        return top;
    }
    const std::vector<IntegralValue>& Values() const { return s; }
private:
    std::vector<IntegralValue> s;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_OPERAND_STACK_INCLUDED
