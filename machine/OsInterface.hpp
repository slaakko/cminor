// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_OS_INTERFACE_INCLUDED
#define CMINOR_MACHINE_OS_INTERFACE_INCLUDED
#include <stdint.h>

namespace cminor { namespace machine {

uint64_t GetSystemPageSize();
uint8_t* ReserveMemory(uint64_t size);
uint8_t* CommitMemory(uint8_t* base, uint64_t size);
void FreeMemory(uint8_t* baseAddress);

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_OS_INTERFACE_INCLUDED
