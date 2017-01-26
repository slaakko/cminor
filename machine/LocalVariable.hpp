// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_LOCAL_VARIABLE_INCLUDED
#define CMINOR_MACHINE_LOCAL_VARIABLE_INCLUDED
#include <cminor/machine/Object.hpp>
#include <string>
#include <stdint.h>
#include <vector>

namespace cminor { namespace machine {

class LocalVariable
{
public:
    LocalVariable();
    IntegralValue GetValue() const { return value; }
    void SetValue(IntegralValue value_) { value = value_; }
private:
    IntegralValue value;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_LOCAL_VARIABLE_INCLUDED
