// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_CMINOR_EXCEPTION_INCLUDED
#define CMINOR_MACHINE_CMINOR_EXCEPTION_INCLUDED
#include <cminor/machine/MachineApi.hpp>
#include <stdint.h>

namespace cminor { namespace machine {

class MACHINE_API CminorException
{
public:
    CminorException(uint64_t exceptionObjectReference_) : exceptionObjectReference(exceptionObjectReference_) {}
    uint64_t ExceptionObjectReference() const { return exceptionObjectReference; }
private:
    uint64_t exceptionObjectReference;
};

void f();

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_CMINOR_EXCEPTION_INCLUDED
