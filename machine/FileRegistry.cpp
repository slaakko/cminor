// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/FileRegistry.hpp>
#include <mutex>

namespace cminor { namespace machine {

class FileRegistryImpl
{
public:
    static void Init();
    static FileRegistryImpl& Instance() { return *instance; }
    int RegisterParsedFile(const std::string& filePath);
    const std::string& GetParsedFileName(int parsedFileIndex) const;
    int GetNumberOfParsedFiles() const;
private:
    static std::unique_ptr<FileRegistryImpl> instance;
    std::vector<std::string> parsedFiles;
};


std::unique_ptr<FileRegistryImpl> FileRegistryImpl::instance;

void FileRegistryImpl::Init()
{
    instance.reset(new FileRegistryImpl());
}

std::mutex mtx;

int FileRegistryImpl::RegisterParsedFile(const std::string& filePath)
{
    std::lock_guard<std::mutex> lock(mtx);
    int fileIndex = int(parsedFiles.size());
    parsedFiles.push_back(filePath);
    return fileIndex;
}

const std::string& FileRegistryImpl::GetParsedFileName(int parsedFileIndex) const
{
    std::lock_guard<std::mutex> lock(mtx);
    static std::string emptyFileName;
    if (parsedFileIndex >= 0 && parsedFileIndex < int(parsedFiles.size()))
    {
        return parsedFiles[parsedFileIndex];
    }
    return emptyFileName;
}

int FileRegistryImpl::GetNumberOfParsedFiles() const
{
    return int(parsedFiles.size());
}

MACHINE_API void FileRegistry::Init()
{
    FileRegistryImpl::Init();
}

MACHINE_API int FileRegistry::RegisterParsedFile(const std::string& filePath)
{
    return FileRegistryImpl::Instance().RegisterParsedFile(filePath);
}

MACHINE_API const std::string& FileRegistry::GetParsedFileName(int parsedFileIndex)
{
    return FileRegistryImpl::Instance().GetParsedFileName(parsedFileIndex);
}

MACHINE_API int FileRegistry::GetNumberOfParsedFiles()
{
    return FileRegistryImpl::Instance().GetNumberOfParsedFiles();
}

} } // namespace cminor::machine
