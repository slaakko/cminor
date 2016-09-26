// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_LOCAL_VARIABLE_INCLUDED
#define CMINOR_MACHINE_LOCAL_VARIABLE_INCLUDED
#include <cminor/machine/Error.hpp>
#include <string>
#include <stdint.h>
#include <vector>

namespace cminor { namespace machine {

class LocalVariable
{
public:
    LocalVariable();
    uint64_t GetValue() const { return value; }
    void SetValue(uint64_t value_) { value = value_; }
private:
    uint64_t value;
};

class LocalVariableVector
{
public:
    LocalVariableVector(int32_t numVars);
    LocalVariable& operator[](int32_t index) { assert(index >= 0 && index < variables.size(), "invalid local variable index " + std::to_string(index)); return variables[index]; }
private:
    std::vector<LocalVariable> variables;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_LOCAL_VARIABLE_INCLUDED
