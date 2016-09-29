// =================================
// Copyright (c) 2016 Seppo Laakko
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

class LocalVariableVector
{
public:
    LocalVariableVector(int32_t numVars);
    int32_t Size() const { return int32_t(variables.size()); }
    LocalVariable& operator[](int32_t index) { assert(index >= 0 && index < variables.size(), "invalid local variable index " + std::to_string(index)); return variables[index]; }
    const std::vector<LocalVariable>& Variables() const { return variables; }
private:
    std::vector<LocalVariable> variables;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_LOCAL_VARIABLE_INCLUDED
