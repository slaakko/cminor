// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CPG_SYNTAX_CODEGENERATOR_INCLUDED
#define CPG_SYNTAX_CODEGENERATOR_INCLUDED

#include <cminor/cpg/cpgsyntax/ParserFileContent.hpp>

namespace cpg { namespace syntax {

void SetForceCodeGen(bool force);
void GenerateCode(const std::vector<std::unique_ptr<ParserFileContent>>& parserFiles);

} } // namespace cpg::syntax

#endif // CPG_SYNTAX_CODEGENERATOR_INCLUDED
