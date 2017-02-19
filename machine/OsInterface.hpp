// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_OS_INTERFACE_INCLUDED
#define CMINOR_MACHINE_OS_INTERFACE_INCLUDED
#include <cminor/util/Defines.hpp>
#include <cminor/machine/MachineApi.hpp>
#include <string>
#include <stdint.h>

namespace cminor { namespace machine {

uint64_t GetSystemPageSize();
uint8_t* ReserveMemory(uint64_t size);
uint8_t* CommitMemory(uint8_t* base, uint64_t size);
void FreeMemory(uint8_t* baseAddress, uint64_t size);

MACHINE_API void WriteInGreenToConsole(const std::string& line);

void InspectStack(uint64_t threadHandle);
uint64_t GetCurrentThreadHandle();
void CloseThreadHandle(uint64_t threadHandle);

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_OS_INTERFACE_INCLUDED
