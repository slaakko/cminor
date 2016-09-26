// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_THREAD_INCLUDED
#define CMINOR_MACHINE_THREAD_INCLUDED
#include <cminor/machine/Frame.hpp>
#include <stack>

namespace cminor { namespace machine {

class Function;

class Thread
{
public:
    Thread(Function& fun_);
private:
    Function& fun;
    std::stack<Frame> frames;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_THREAD_INCLUDED
