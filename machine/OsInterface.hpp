// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_OS_INTERFACE_INCLUDED
#define CMINOR_MACHINE_OS_INTERFACE_INCLUDED
#include <string>
#include <stdint.h>

namespace cminor { namespace machine {

uint64_t GetSystemPageSize();
uint8_t* ReserveMemory(uint64_t size);
uint8_t* CommitMemory(uint8_t* base, uint64_t size);
void FreeMemory(uint8_t* baseAddress, uint64_t size);

#ifdef _WIN32
    #if defined(_WIN64)
        #define BITS_64 1
    #endif
#else
    #if defined(__x86_64__)
        #define BITS_64 1
    #endif 
#endif 

void WriteInGreenToConsole(const std::string& line);

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_OS_INTERFACE_INCLUDED
