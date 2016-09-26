// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Assembly.hpp>
#include <cminor/machine/MappedInputFile.hpp>

namespace cminor { namespace machine {

Assembly::Assembly(const std::string& fileName_) : fileName(fileName_)
{
}

void Assembly::Read(Reader& reader)
{
}

} } // namespace cminor::machine
