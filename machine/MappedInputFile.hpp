// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_MAPPED_INPUT_FILE_INCLUDED
#define CMINOR_MACHINE_MAPPED_INPUT_FILE_INCLUDED
#include <string>
#include <stdint.h>

namespace cminor { namespace machine {

class MappedInputFileImpl;

class MappedInputFile
{
public:
    MappedInputFile(const std::string& fileName_);
    ~MappedInputFile();
    const char* Begin() const;
    const char* End() const;
private:
    MappedInputFileImpl* impl;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_MAPPED_INPUT_FILE_INCLUDED
