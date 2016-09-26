// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/LocalVariable.hpp>

namespace cminor { namespace machine {

LocalVariable::LocalVariable() : value(0)
{
}

LocalVariableVector::LocalVariableVector(int32_t numVars)
{
    variables.resize(numVars);
}

} } // namespace cminor::machine
