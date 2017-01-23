// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/OsInterface.hpp>
#include <iostream>
#include <string>
#include <stdexcept>
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#else
    #include <unistd.h>
    #include <sys/mman.h>
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

void FreeMemory(uint8_t* baseAddress, uint64_t size)
{
    BOOL result = VirtualFree(baseAddress, NULL, MEM_RELEASE);
}

void WriteInGreenToConsole(const std::string& line)
{
    bool written = false;
    HANDLE consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo;
    if (GetConsoleScreenBufferInfo(consoleOutput, &screenBufferInfo))
    {
        WORD oldAttr = screenBufferInfo.wAttributes;
        if (SetConsoleTextAttribute(consoleOutput, FOREGROUND_GREEN))
        {
            std::cout << line << std::endl;
            written = true;
            SetConsoleTextAttribute(consoleOutput, oldAttr);
        }
    }
    if (!written)
    {
        std::cout << line << std::endl;
    }
}

#else

uint64_t GetSystemPageSize()
{
    return getpagesize();
}

uint8_t* ReserveMemory(uint64_t size)
{
    size_t length = size;
    void* baseAddress = mmap(NULL, length, PROT_NONE, MAP_ANONYMOUS, -1, 0);
    if (baseAddress == MAP_FAILED)
    {
        throw std::runtime_error("could not reserve " + std::to_string(size) + " bytes memory from os");
    }
    return static_cast<uint8_t*>(baseAddress);
}

uint8_t* CommitMemory(uint8_t* base, uint64_t size)
{
    size_t length = size;
    int result = mprotect(base, length, PROT_READ | PROT_WRITE);
    if (result != 0)
    {
        throw std::runtime_error("could not commit " + std::to_string(size) + " bytes memory from os");
    }
    return base;
}

void FreeMemory(uint8_t* baseAddress, uint64_t size)
{
    size_t length = size;
    int result = munmap(baseAddress, length);
}

const std::string green("\033[1;32m");
const std::string reset("\033[0m");

void WriteInGreenToConsole(const std::string& line)
{
    if (isatty(1))
    {
        std::cout << green << line << reset << std::endl;
    }
    else
    {
        std::cout << line << std::endl;
    }
}

#endif 

} } // namespace cminor::machine
