// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_FRAME_INCLUDED
#define CMINOR_MACHINE_FRAME_INCLUDED
#include <cminor/machine/OperationStack.hpp>

namespace cminor { namespace machine {

class Frame
{
public:
    OperationStack& OpStack() { return opStack; }
private:
    OperationStack opStack;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_FRAME_INCLUDED
