// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_FRAME_INCLUDED
#define CMINOR_MACHINE_FRAME_INCLUDED
#include <cminor/machine/OperandStack.hpp>
#include <cminor/machine/LocalVariable.hpp>

namespace cminor { namespace machine {

class Frame
{
public:
    Frame(int32_t numLocals);
    OperandStack& OpStack() { return opStack; }
    LocalVariable& Local(int32_t index) { return locals[index];  }
    int32_t PC() const { return pc; }
private:
    OperandStack opStack;
    LocalVariableVector locals;
    int32_t pc;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_FRAME_INCLUDED