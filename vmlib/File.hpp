// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_VMLIB_FILE_INCLUDED
#define CMINOR_VMLIB_FILE_INCLUDED
#include <cminor/machine/Error.hpp>
#include <string>
#include <stdint.h>
#include <stdexcept>

namespace cminor { namespace vmlib {

using namespace cminor::machine;

enum class FileMode : uint8_t
{
    append, create, open
};

enum class FileAccess : uint8_t
{
    read, readWrite, write
};

int32_t OpenFile(const std::string& filePath, FileMode mode, FileAccess access);
void CloseFile(int32_t fileHandle);
void WriteByteToFile(int32_t fileHandle, uint8_t value);
void WriteFile(int32_t fileHandle, const uint8_t* buffer, int32_t count);
int32_t ReadByteFromFile(int32_t fileHandle);
int32_t ReadFile(int32_t fileHandle, uint8_t* buffer, int32_t bufferSize);
void FileInit();
void FileDone();

} } // namespace cminor::vmlib

#endif // CMINOR_VMLIB_FILE_INCLUDED

