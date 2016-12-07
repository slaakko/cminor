// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CPG_SYNTAX_COMPILER_INCLUDED
#define CPG_SYNTAX_COMPILER_INCLUDED

#include <string>
#include <vector>

namespace cpg { namespace syntax {

void Compile(const std::string& projectFilePath, const std::vector<std::string>& libraryDirectories);

} } // namespace cpg::syntax

#endif // CPG_SYNTAX_COMPILER_INCLUDED

