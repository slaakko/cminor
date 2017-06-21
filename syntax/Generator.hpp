// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYNTAX_COMPILER_INCLUDED
#define CMINOR_SYNTAX_COMPILER_INCLUDED

#include <string>
#include <vector>

namespace cminor { namespace syntax {

void Generate(const std::string& projectFilePath, const std::vector<std::string>& libraryDirectories);

} } // namespace cminor::syntax

#endif // CMINOR_SYNTAX_COMPILER_INCLUDED

