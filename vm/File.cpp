// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/vm/File.hpp>
#include <cminor/machine/Error.hpp>
#include <stdio.h>
#include <unordered_map>
#include <memory>

namespace cminor { namespace vm {

FileError::FileError(const std::string& errorMessage) : std::runtime_error(errorMessage)
{
}

std::string FileModeStr(FileMode mode)
{
    switch (mode)
    {
        case FileMode::append: return "append";
        case FileMode::create: return "create";
        case FileMode::open: return "open";
    }
    return std::string();
}

std::string FileAccessStr(FileAccess access)
{
    switch (access)
    {
        case FileAccess::read: return "read";
        case FileAccess::readWrite: return "read/write";
        case FileAccess::write: return "write";
    }
    return std::string();
}

class FileTable
{
public:
    static void Init();
    static void Done();
    static FileTable& Instance();
    int32_t OpenFile(const std::string& filePath, FileMode mode, FileAccess access);
    void CloseFile(int32_t fileHandle);
    void WriteFile(int32_t fileHandle, const uint8_t* buffer, int32_t count);
    int32_t ReadFile(int32_t fileHandle, uint8_t* buffer, int32_t bufferSize);
    ~FileTable();
private:
    static std::unique_ptr<FileTable> instance;
    std::unordered_map<int32_t, FILE*> files;
    std::unordered_map<int32_t, std::string> filePaths;
    int32_t nextFileHandle;
    FileTable();
};

std::unique_ptr<FileTable> FileTable::instance;

void FileTable::Init()
{
    instance.reset(new FileTable());
}

void FileTable::Done()
{
    instance.reset();
}

FileTable& FileTable::Instance()
{
    Assert(instance, "file table not initialized");
    return *instance;
}

FileTable::FileTable() : nextFileHandle(3)
{
    files[0] = stdin;
    files[1] = stdout;
    files[2] = stderr;
}

FileTable::~FileTable()
{
    try
    {
        for (const auto& p : files)
        {
            int32_t fileHandle = p.first;
            FILE* file = p.second;
            if (file != stdin && file != stdout && file != stderr)
            {
                fclose(file);
            }
        }
    }
    catch (...)
    {
    }
}

int32_t FileTable::OpenFile(const std::string& filePath, FileMode mode, FileAccess access)
{
    std::string modeStr;
    if (mode == FileMode::append)
    {
        if (access != FileAccess::write && access != FileAccess::read)
        {
            throw FileError("invalid file access '" + FileAccessStr(access) + "' for file mode '" + FileModeStr(mode));
        }
        if (access == FileAccess::write)
        {
            modeStr = "a";
        }
        else if (access == FileAccess::read)
        {
            modeStr = "a+";
        }
    }
    else if (mode == FileMode::create)
    {
        if (access != FileAccess::readWrite && access != FileAccess::write)
        {
            throw FileError("invalid file access '" + FileAccessStr(access) + "' for file mode '" + FileModeStr(mode));
        }
        if (access == FileAccess::write)
        {
            modeStr = "w";
        }
        else if (access == FileAccess::readWrite)
        {
            modeStr = "w+";
        }
    }
    else if (mode == FileMode::open)
    {
        if (access == FileAccess::read)
        {
            modeStr = "r";
        }
        else if (access == FileAccess::readWrite)
        {
            modeStr = "r+";
        }
    }
    if (modeStr.empty())
    {
        throw FileError("invalid file access '" + FileAccessStr(access) + "' for file mode '" + FileModeStr(mode));
    }
    FILE* file = fopen(filePath.c_str(), modeStr.c_str());
    if (!file)
    {
        throw FileError("could not open '" + filePath + "': " + strerror(errno));
    }
    int32_t fileHandle = nextFileHandle++;
    files[fileHandle] = file;
    filePaths[fileHandle] = filePath;
    return fileHandle;
}

void FileTable::CloseFile(int32_t fileHandle)
{
    if (fileHandle == 0 || fileHandle == 1 || fileHandle == 2) return;
    auto it = files.find(fileHandle);
    if (it != files.cend())
    {
        FILE* file = it->second;
        files.erase(fileHandle);
        int result = std::fclose(file);
        if (result != 0)
        {
            std::string filePath = filePaths[fileHandle];
            throw FileError("could not close '" + filePath + "': " + strerror(errno));
        }
    }
    else
    {
        throw FileError("invalid file handle " + std::to_string(fileHandle));
    }
}

void FileTable::WriteFile(int32_t fileHandle, const uint8_t* buffer, int32_t count)
{
    auto it = files.find(fileHandle);
    if (it != files.cend())
    {
        FILE* file = it->second;
        int32_t result = int32_t(std::fwrite(buffer, 1, count, file));
        if (result != count)
        {
            std::string filePath = filePaths[fileHandle];
            throw FileError("could not write to '" + filePath + "': " + strerror(errno));
        }
    }
    else
    {
        throw FileError("invalid file handle " + std::to_string(fileHandle));
    }
}

int32_t FileTable::ReadFile(int32_t fileHandle, uint8_t* buffer, int32_t bufferSize)
{
    auto it = files.find(fileHandle);
    if (it != files.cend())
    {
        FILE* file = it->second;
        int32_t result = int32_t(std::fread(buffer, 1, bufferSize, file));
        if (result < bufferSize)
        {
            int error = ferror(file);
            if (error == 0)
            {
                return result;
            }
            std::string filePath = filePaths[fileHandle];
            throw FileError("could not read from '" + filePath + "': " + strerror(errno));
        }
        return result;
    }
    else
    {
        throw FileError("invalid file handle " + std::to_string(fileHandle));
    }
}

int32_t OpenFile(const std::string& filePath, FileMode mode, FileAccess access)
{
    return FileTable::Instance().OpenFile(filePath, mode, access);
}

void CloseFile(int32_t fileHandle)
{
    FileTable::Instance().CloseFile(fileHandle);
}

void WriteFile(int32_t fileHandle, const uint8_t* buffer, int32_t count)
{
    FileTable::Instance().WriteFile(fileHandle, buffer, count);
}

void WriteByteToFile(int32_t fileHandle, uint8_t value)
{
    uint8_t x = value;
    WriteFile(fileHandle, &x, 1);
}

int32_t ReadFile(int32_t fileHandle, uint8_t* buffer, int32_t bufferSize)
{
    return FileTable::Instance().ReadFile(fileHandle, buffer, bufferSize);
}

int32_t ReadByteFromFile(int32_t fileHandle)
{
    uint8_t value;
    int32_t bytesRead = ReadFile(fileHandle, &value, 1);
    if (bytesRead == 0)
    {
        return -1;
    }
    return value;
}

void FileInit()
{
    FileTable::Init();
}

void FileDone()
{
    FileTable::Done();
}

} } // namespace cminor::vm
