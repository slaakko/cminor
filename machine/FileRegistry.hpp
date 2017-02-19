// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_FILE_REGISTRY_INCLUDED
#define CMINOR_MACHINE_FILE_REGISTRY_INCLUDED
#include <cminor/machine/MachineApi.hpp>
#include <memory>
#include <string>
#include <vector>

namespace cminor { namespace machine {

class FileRegistry
{
public:
    MACHINE_API static void Init();
    MACHINE_API static int RegisterParsedFile(const std::string& filePath);
    MACHINE_API static const std::string& GetParsedFileName(int parsedFileIndex);
    MACHINE_API static int GetNumberOfParsedFiles();
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_FILE_REGISTRY_INCLUDED
