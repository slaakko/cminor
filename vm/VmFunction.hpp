// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_VM_FUNCTION_INCLUDED
#define CMINOR_VM_FUNCTION_INCLUDED
#include <cminor/machine/Constant.hpp>

namespace cminor { namespace vm {

using namespace cminor::machine;

void InitVmFunctions(ConstantPool& vmFunctionNamePool);
void DoneVmFunctions();

} } // namespace cminor::vm

#endif // CMINOR_VM_FUNCTION_INCLUDED
