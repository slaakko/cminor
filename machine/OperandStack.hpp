// =================================
// Copyright (c) 2017 Seppo Laakko
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
    void SetValue(int32_t index, IntegralValue value) { Assert(s.size() - index >= 0 && s.size() - index < s.size(), "invalid set value index"); s[int32_t(s.size()) - index] = value; }
    void Dup()
    {
        Assert(!s.empty(), "cannot dup: operand stack is empty");
        s.push_back(s.back());
    }
    void Swap()
    {
        Assert(s.size() >= 2, "cannot swap: less than two operands in the operand stack");
        std::swap(s.back(), s[s.size() - 2]);
    }
    void Rotate()
    {
        Assert(s.size() >= 3, "cannot rotate: less than three operands in the operand stack");
        std::swap(s[s.size() - 3], s[s.size() - 2]);
        std::swap(s.back(), s[s.size() - 2]);
    }
    void Insert(int32_t index, IntegralValue value)
    {
        Assert(s.size() - index >= 0 && s.size() - index <= s.size(), "invalid insert index");
        s.insert(s.end() - index, value);
    }
private:
    std::vector<IntegralValue> s;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_OPERAND_STACK_INCLUDED
