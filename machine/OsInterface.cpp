// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/OsInterface.hpp>
#include <string>
#include <stdexcept>
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#endif

namespace cminor { namespace machine {

#ifdef _WIN32

uint64_t GetSystemPageSize()
{
    SYSTEM_INFO sSysInfo;
    GetSystemInfo(&sSysInfo);
    return sSysInfo.dwPageSize;
}

uint8_t* ReserveMemory(uint64_t size)
{
    void* baseAddress = VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE);
    if (!baseAddress)
    {
        throw std::runtime_error("could not reserve " + std::to_string(size) + " bytes memory from os");
    }
    return static_cast<uint8_t*>(baseAddress);
}

uint8_t* CommitMemory(uint8_t* base, uint64_t size)
{
    void* block = VirtualAlloc(base, size, MEM_COMMIT, PAGE_READWRITE);
    if (!block)
    {
        throw std::runtime_error("could not commit " + std::to_string(size) + " bytes memory from os");
    }
    return static_cast<uint8_t*>(block);
}

void FreeMemory(uint8_t* baseAddress)
{
    BOOL result = VirtualFree(baseAddress, NULL, MEM_RELEASE);
    if (!result)
    {
        int x = 0;
    }
}

#endif 

} } // namespace cminor::machine
