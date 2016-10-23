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
        Assert(!s.empty(), "operand stack is empty");
        IntegralValue top = s.back();
        s.pop_back();
        return top;
    }
    const std::vector<IntegralValue>& Values() const { return s; }
    IntegralValue GetValue(int32_t index) const { Assert(s.size() - index >= 0 && s.size() - index < s.size(), "invalid get value index"); return s[int32_t(s.size()) - index]; }
    void Dup()
    {
        Assert(!s.empty(), "cannot dup: operand stack is empty");
        s.push_back(s.back());
    }
private:
    std::vector<IntegralValue> s;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_OPERAND_STACK_INCLUDED
