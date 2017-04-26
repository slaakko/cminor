// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/OsInterface.hpp>
#include <iostream>
#include <string>
#include <stdexcept>
#include <cstring>
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
    #include <winnt.h>
#include <io.h>
#include <fcntl.h>
#else
    #include <unistd.h>
    #include <sys/mman.h>
    #include <dlfcn.h>
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

MACHINE_API void WriteInGreenToConsole(const std::string& line)
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

void InspectStack(uint64_t threadHandle)
{
    HANDLE thread = reinterpret_cast<HANDLE>(threadHandle);
    DWORD suspendResult = SuspendThread(thread);
    if (suspendResult == (DWORD)-1)
    {
        throw std::runtime_error("suspend thread failed");
    }
    CONTEXT context;
    context.ContextFlags = CONTEXT_INTEGER;
    BOOL getThreadContextResult = GetThreadContext(thread, &context);
    if (!getThreadContextResult)
    {
        DWORD resumeResult = ResumeThread(thread);
        if (resumeResult == (DWORD)-1)
        {
            // resume thread failed
        }
        throw std::runtime_error("get thread context failed");
    }
    DWORD resumeResult = ResumeThread(thread);
    if (resumeResult == (DWORD)-1)
    {
        // resume thread failed
    }
}

uint64_t GetCurrentThreadHandle()
{
    HANDLE threadPseudoHandle = GetCurrentThread();
    HANDLE threadHandle;
    HANDLE processHandle = GetCurrentProcess();
    BOOL duplicateHandleResult = DuplicateHandle(processHandle, threadPseudoHandle, processHandle, &threadHandle, 0, false, DUPLICATE_SAME_ACCESS);
    if (!duplicateHandleResult)
    {
        throw std::runtime_error("duplicate handle failed");
    }
    return reinterpret_cast<uint64_t>(threadHandle);
}

void CloseThreadHandle(uint64_t threadHandle)
{
    HANDLE thread = reinterpret_cast<HANDLE>(threadHandle);
    CloseHandle(thread);
}

MACHINE_API void* LoadSharedLibrary(const std::string& sharedLibraryFilePath)
{
    HMODULE handle = LoadLibrary(sharedLibraryFilePath.c_str());
    if (handle == NULL)
    {
        DWORD errorCode = GetLastError();
        LPTSTR buffer = nullptr;
        DWORD result = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorCode, 0, buffer, 0, nullptr);
        if (result != 0)
        {
            std::string errorMessage(buffer);
            LocalFree(buffer);
            throw std::runtime_error("error loading library '" + sharedLibraryFilePath + "': " + errorMessage);
        }
        else
        {
            throw std::runtime_error("error loading library '" + sharedLibraryFilePath + "': error code = " + std::to_string(errorCode));
        }
    }
    return static_cast<void*>(handle);
}

MACHINE_API void FreeSharedLibrary(void* sharedLibraryHandle)
{
    HMODULE handle = static_cast<HMODULE>(sharedLibraryHandle);
    FreeLibrary(handle);
}

MACHINE_API void* ResolveSymbolAddress(void* sharedLibraryHandle, const std::string& sharedLibraryFilePath, const std::string& symbolName)
{
    HMODULE handle = static_cast<HMODULE>(sharedLibraryHandle);
    void* symbolAddress = GetProcAddress(handle, symbolName.c_str());
    if (!symbolAddress)
    {
        DWORD errorCode = GetLastError();
        LPTSTR buffer = nullptr;
        DWORD result = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorCode, 0, buffer, 0, nullptr);
        if (result != 0)
        {
            std::string errorMessage(buffer);
            LocalFree(buffer);
            throw std::runtime_error("error resolving address of symbol '" + symbolName + "' in library '" + sharedLibraryFilePath + "': " + errorMessage);
        }
        else
        {
            throw std::runtime_error("error resolving address of symbol '" + symbolName + "' in library '" + sharedLibraryFilePath + "': error code = " + std::to_string(errorCode));
        }
    }
    return symbolAddress;
}

MACHINE_API void SetHandleToBinaryMode(int handle)
{
    _setmode(handle, _O_BINARY);
}

MACHINE_API void SetHandleToTextMode(int handle)
{
    _setmode(handle, _O_TEXT);
}

#else

uint64_t GetSystemPageSize()
{
    return getpagesize();
}

uint8_t* ReserveMemory(uint64_t size)
{
    size_t length = size;
    void* baseAddress = mmap(NULL, length, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (baseAddress == MAP_FAILED)
    {
        throw std::runtime_error("could not reserve " + std::to_string(size) + " bytes memory from os: " + std::string(strerror(errno)));
    }
    return static_cast<uint8_t*>(baseAddress);
}

uint8_t* CommitMemory(uint8_t* base, uint64_t size)
{
    size_t length = size;
    int result = mprotect(base, length, PROT_READ | PROT_WRITE);
    if (result != 0)
    {
        throw std::runtime_error("could not commit " + std::to_string(size) + " bytes memory from os: " + std::string(strerror(errno)));
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

uint64_t GetCurrentThreadHandle()
{
    return 0;
}

void CloseThreadHandle(uint64_t threadHandle)
{
}

void* LoadSharedLibrary(const std::string& sharedLibraryFilePath)
{
    void* libraryHandle = dlopen(sharedLibraryFilePath.c_str(), RTLD_LAZY);
    if (libraryHandle == nullptr)
    {
        throw std::runtime_error("error loading library '" + sharedLibraryFilePath + "': " + std::string(dlerror()));
    }
    return libraryHandle;
}

void FreeSharedLibrary(void* sharedLibraryHandle)
{
    int result = dlclose(sharedLibraryHandle);
    if (result != 0)
    {
        throw std::runtime_error("error closing library: " + std::string(dlerror()));
    }
}

void* ResolveSymbolAddress(void* sharedLibraryHandle, const std::string& sharedLibraryFilePath, const std::string& symbolName)
{
    void* address = dlsym(sharedLibraryHandle, symbolName.c_str());
    if (address == nullptr)
    {
        throw std::runtime_error("error resolving address of symbol '" + symbolName + "' in library '" + sharedLibraryFilePath + "': " + std::string(dlerror()));
    }
}

void SetHandleToBinaryMode(int handle)
{
}

void SetHandleToTextMode(int handle)
{
}

#endif 

} } // namespace cminor::machine
