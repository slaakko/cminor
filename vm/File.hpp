// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_VM_FILE_INCLUDED
#define CMINOR_VM_FILE_INCLUDED
#include <string>
#include <stdint.h>
#include <stdexcept>

namespace cminor { namespace vm {

class FileError : public std::runtime_error
{
public:
    FileError(const std::string& errorMessage);
};

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

} } // namespace cminor::vm

#endif // CMINOR_VM_FILE_INCLUDED
