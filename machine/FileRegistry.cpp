// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/FileRegistry.hpp>
#include <mutex>

namespace cminor { namespace machine {

std::unique_ptr<FileRegistry> FileRegistry::instance;

void FileRegistry::Init()
{
    instance.reset(new FileRegistry());
}

std::mutex mtx;

int FileRegistry::RegisterParsedFile(const std::string& filePath)
{
    std::lock_guard<std::mutex> lock(mtx);
    int fileIndex = int(parsedFiles.size());
    parsedFiles.push_back(filePath);
    return fileIndex;
}

const std::string& FileRegistry::GetParsedFileName(int parsedFileIndex) const
{
    std::lock_guard<std::mutex> lock(mtx);
    static std::string emptyFileName;
    if (parsedFileIndex >= 0 && parsedFileIndex < int(parsedFiles.size()))
    {
        return parsedFiles[parsedFileIndex];
    }
    return emptyFileName;
}

int FileRegistry::GetNumberOfParsedFiles() const
{
    return int(parsedFiles.size());
}

} } // namespace cminor::machine
