// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/CminorException.hpp>
#include <cminor/machine/RunTime.hpp>
#include <cminor/machine/Class.hpp>

namespace cminor { namespace machine {

void* cd = nullptr;

void foo()
{
    uint64_t o = RtCreateObject(cd);
    RtSetClassDataPtr(o, cd);
    RtThrowException(o);
}

MACHINE_API void ThrowCatch()
{
    try
    {
        cd = ClassDataTable::GetClassData(U"System.Exception");
        foo();
    }
    catch (const CminorException&)
    {
        if (RtHandleException(cd))
        {
        }
    }
}

} } // namespace cminor::machine
