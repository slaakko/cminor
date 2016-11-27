// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_VMLIB_VMFUNCTION_INCLUDED
#define CMINOR_VMLIB_VMFUNCTION_INCLUDED
#include <cminor/machine/Constant.hpp>

namespace cminor { namespace vmlib {

using namespace cminor::machine;

void InitVmFunctions(ConstantPool& vmFunctionNamePool);
void DoneVmFunctions();

} } // namespace cminor::vmlib

#endif // CMINOR_VMLIB_VMFUNCTION_INCLUDED
