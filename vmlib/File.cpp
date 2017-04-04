// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/vmlib/File.hpp>
#include <cminor/machine/Error.hpp>
#include <cminor/machine/OsInterface.hpp>
#include <cminor/machine/Machine.hpp>
#include <cstdio>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <atomic>

namespace cminor { namespace vmlib {

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
    void SeekFile(int32_t fileHandle, int32_t pos, Origin origin);
    int32_t TellFile(int32_t fileHandle);
    bool HandlesSetToBinaryMode() const { return handlesSetToBinaryMode; }
    ~FileTable();
private:
    const int32_t maxNoLockFileHandles = 256;
    static std::unique_ptr<FileTable> instance;
    std::vector<FILE*> files;
    std::vector<std::string> filePaths;
    std::unordered_map<int32_t, FILE*> fileMap;
    std::unordered_map<int32_t, std::string> filePathMap;
    std::atomic<int32_t> nextFileHandle;
    std::mutex mtx;
    bool handlesSetToBinaryMode;
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

FileTable::FileTable() : nextFileHandle(3), handlesSetToBinaryMode(false)
{
    files.resize(maxNoLockFileHandles);
    filePaths.resize(maxNoLockFileHandles);
    SetHandleToBinaryMode(0);
    files[0] = stdin;
    SetHandleToBinaryMode(1);
    files[1] = stdout;
    SetHandleToBinaryMode(2);
    files[2] = stderr;
    handlesSetToBinaryMode = true;
}

FileTable::~FileTable()
{
    try
    {
        if (handlesSetToBinaryMode)
        {
            SetHandleToTextMode(0);
            SetHandleToTextMode(1);
            SetHandleToTextMode(2);
        }
        for (FILE* file : files)
        {
            if (file && file != stdin && file != stdout && file != stderr)
            {
                std::fclose(file);
            }
        }
        for (const auto& p : fileMap)
        {
            FILE* file = p.second;
            if (file)
            {
                std::fclose(p.second);
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
            throw FileSystemError("invalid file access '" + FileAccessStr(access) + "' for file mode '" + FileModeStr(mode));
        }
        if (access == FileAccess::write)
        {
            modeStr = "ab";
        }
        else if (access == FileAccess::read)
        {
            modeStr = "a+b";
        }
    }
    else if (mode == FileMode::create)
    {
        if (access != FileAccess::readWrite && access != FileAccess::write)
        {
            throw FileSystemError("invalid file access '" + FileAccessStr(access) + "' for file mode '" + FileModeStr(mode));
        }
        if (access == FileAccess::write)
        {
            modeStr = "wb";
        }
        else if (access == FileAccess::readWrite)
        {
            modeStr = "w+b";
        }
    }
    else if (mode == FileMode::open)
    {
        if (access == FileAccess::read)
        {
            modeStr = "rb";
        }
        else if (access == FileAccess::readWrite)
        {
            modeStr = "r+b";
        }
    }
    if (modeStr.empty())
    {
        throw FileSystemError("invalid file access '" + FileAccessStr(access) + "' for file mode '" + FileModeStr(mode));
    }
    FILE* file = std::fopen(filePath.c_str(), modeStr.c_str());
    if (!file)
    {
        throw FileSystemError("could not open '" + filePath + "': " + strerror(errno));
    }
    int32_t fileHandle = nextFileHandle++;
    if (fileHandle < maxNoLockFileHandles)
    {
        files[fileHandle] = file;
        filePaths[fileHandle] = filePath;
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        fileMap[fileHandle] = file;
        filePathMap[fileHandle] = filePath;
    }
    return fileHandle;
}

void FileTable::CloseFile(int32_t fileHandle)
{
    if (fileHandle == 0 || fileHandle == 1 || fileHandle == 2) return;
    if (fileHandle < 0)
    {
        throw FileSystemError("invalid file handle " + std::to_string(fileHandle));
    }
    else if (fileHandle < maxNoLockFileHandles)
    {
        FILE* file = files[fileHandle];
        files[fileHandle] = nullptr;
        int result = std::fclose(file);
        if (result != 0)
        {
            std::string filePath = filePaths[fileHandle];
            throw FileSystemError("could not close '" + filePath + "': " + strerror(errno));
        }
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = fileMap.find(fileHandle);
        if (it != fileMap.cend())
        {
            FILE* file = it->second;
            fileMap.erase(fileHandle);
            int result = std::fclose(file);
            if (result != 0)
            {
                std::string filePath = filePathMap[fileHandle];
                throw FileSystemError("could not close '" + filePath + "': " + strerror(errno));
            }
        }
        else
        {
            throw FileSystemError("invalid file handle " + std::to_string(fileHandle));
        }
    }
}

void FileTable::WriteFile(int32_t fileHandle, const uint8_t* buffer, int32_t count)
{
    FILE* file = nullptr;
    if (fileHandle < 0)
    {
        throw FileSystemError("invalid file handle " + std::to_string(fileHandle));
    }
    else if (fileHandle < maxNoLockFileHandles)
    {
        file = files[fileHandle];
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = fileMap.find(fileHandle);
        if (it != fileMap.cend())
        {
            file = it->second;
        }
        else
        {
            throw FileSystemError("invalid file handle " + std::to_string(fileHandle));
        }
    }
    int32_t result = int32_t(std::fwrite(buffer, 1, count, file));
    if (result != count)
    {
        std::string filePath;
        if (fileHandle < maxNoLockFileHandles)
        {
            filePath = filePaths[fileHandle];
        }
        else
        {
            filePath = filePathMap[fileHandle];
        }
        throw FileSystemError("could not write to '" + filePath + "': " + strerror(errno));
    }
}

int32_t FileTable::ReadFile(int32_t fileHandle, uint8_t* buffer, int32_t bufferSize)
{
    FILE* file = nullptr;
    if (fileHandle < 0)
    {
        throw FileSystemError("invalid file handle " + std::to_string(fileHandle));
    }
    else if (fileHandle < maxNoLockFileHandles)
    {
        file = files[fileHandle];
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = fileMap.find(fileHandle);
        if (it != fileMap.cend())
        {
            file = it->second;
        }
        else
        {
            throw FileSystemError("invalid file handle " + std::to_string(fileHandle));
        }
    }
    int32_t result = int32_t(std::fread(buffer, 1, bufferSize, file));
    if (result < bufferSize)
    {
        int error = ferror(file);
        if (error == 0)
        {
            return result;
        }
        std::string filePath;
        if (fileHandle < maxNoLockFileHandles)
        {
            filePath = filePaths[fileHandle];
        }
        else
        {
            filePath = filePathMap[fileHandle];
        }
        throw FileSystemError("could not read from '" + filePath + "': " + strerror(errno));
    }
    return result;
}

void FileTable::SeekFile(int32_t fileHandle, int32_t pos, Origin origin)
{
    FILE* file = nullptr;
    if (fileHandle < 0)
    {
        throw FileSystemError("invalid file handle " + std::to_string(fileHandle));
    }
    else if (fileHandle < maxNoLockFileHandles)
    {
        file = files[fileHandle];
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = fileMap.find(fileHandle);
        if (it != fileMap.cend())
        {
            file = it->second;
        }
        else
        {
            throw FileSystemError("invalid file handle " + std::to_string(fileHandle));
        }
    }
    int orig = 0;
    switch (origin)
    {
        case Origin::seekSet: orig = SEEK_SET; break;
        case Origin::seekCur: orig = SEEK_CUR; break;
        case Origin::seekEnd: orig = SEEK_END; break;
    }
    int result = std::fseek(file, pos, orig);
    if (result != 0)
    {
        std::string filePath;
        if (fileHandle < maxNoLockFileHandles)
        {
            filePath = filePaths[fileHandle];
        }
        else
        {
            filePath = filePathMap[fileHandle];
        }
        throw FileSystemError("could not seek file '" + filePath + "': " + strerror(errno));
    }
}

int32_t FileTable::TellFile(int32_t fileHandle)
{
    FILE* file = nullptr;
    if (fileHandle < 0)
    {
        throw FileSystemError("invalid file handle " + std::to_string(fileHandle));
    }
    else if (fileHandle < maxNoLockFileHandles)
    {
        file = files[fileHandle];
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = fileMap.find(fileHandle);
        if (it != fileMap.cend())
        {
            file = it->second;
        }
        else
        {
            throw FileSystemError("invalid file handle " + std::to_string(fileHandle));
        }
    }
    int32_t result = std::ftell(file);
    if (result == -1)
    {
        std::string filePath;
        if (fileHandle < maxNoLockFileHandles)
        {
            filePath = filePaths[fileHandle];
        }
        else
        {
            filePath = filePathMap[fileHandle];
        }
        throw FileSystemError("could not tell file position for '" + filePath + "': " + strerror(errno));
    }
    return result;
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

void SeekFile(int32_t fileHandle, int32_t pos, Origin origin)
{
    FileTable::Instance().SeekFile(fileHandle, pos, origin);
}

int32_t TellFile(int32_t fileHandle)
{
    return FileTable::Instance().TellFile(fileHandle);
}

bool HandlesSetToBinaryMode()
{
    return FileTable::Instance().HandlesSetToBinaryMode();
}

void FileInit()
{
    FileTable::Init();
}

void FileDone()
{
    FileTable::Done();
}

} } // namespace cminor::vmlib
