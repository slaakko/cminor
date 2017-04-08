// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/CminorException.hpp>
#include <cminor/machine/Runtime.hpp>
#include <cminor/machine/Class.hpp>

namespace cminor { namespace machine {

void* cd = nullptr;

void foo()
{
    uint64_t o = RtCreateObject(cd);
    RtSetClassDataPtr(o, cd);
    RtThrowException(o);
}

void ThrowCatch()
{
    try
    {
        cd = ClassDataTable::GetClassData(U"System.Exception");
        foo();
    }
    catch (const CminorException& ex)
    {
        uint64_t x = ex.ExceptionObjectReference();
        if (RtHandleException(cd))
        {
        }
    }
}

void CatchAll()
{
    try
    {
        foo();
    }
    catch (...)
    {
        throw;
    }
}

} } // namespace cminor::machine
