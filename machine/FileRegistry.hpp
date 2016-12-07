// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_FILE_REGISTRY_INCLUDED
#define CMINOR_MACHINE_FILE_REGISTRY_INCLUDED
#include <memory>
#include <string>
#include <vector>

namespace cminor { namespace machine {

class FileRegistry
{
public:
    static void Init();
    static FileRegistry* Instance() { return instance.get(); }
    int RegisterParsedFile(const std::string& filePath);
    const std::string& GetParsedFileName(int parsedFileIndex) const;
    int GetNumberOfParsedFiles() const;
private:
    static std::unique_ptr<FileRegistry> instance;
    std::vector<std::string> parsedFiles;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_FILE_REGISTRY_INCLUDED
