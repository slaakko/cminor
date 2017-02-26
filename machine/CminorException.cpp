// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/CminorException.hpp>

namespace cminor { namespace machine {

void foo()
{
    throw CminorException(1);
}

void f()
{
    try
    {
        foo();
    }
    catch (const CminorException& ex)
    {
        uint64_t ob = ex.ExceptionObjectReference();
    }
}

} } // namespace cminor::machine
