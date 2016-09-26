// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_ASSEMBLY_INCLUDED
#define CMINOR_MACHINE_ASSEMBLY_INCLUDED
#include <cminor/machine/Reader.hpp>

namespace cminor { namespace machine {

class Assembly
{
public:
    Assembly(const std::string& fileName_);
    void Read(Reader& reader);
private:
    std::string fileName;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_ASSEMBLY_INCLUDED
