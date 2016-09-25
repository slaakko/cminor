// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_OPERATION_STACK_INCLUDED
#define CMINOR_MACHINE_OPERATION_STACK_INCLUDED
#include <stdint.h>
#include <vector>

class OperationStack
{
public:
    void Push(uint64_t value)
    {
        s.push_back(value);
    }
    uint64_t Pop()
    {
        uint64_t top = s.back();
        s.pop_back();
        return top;
    }
private:
    std::vector<uint64_t> s;
};

#endif // CMINOR_MACHINE_OPERATION_STACK_INCLUDED
