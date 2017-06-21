// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYNTAX_CODEGENERATOR_INCLUDED
#define CMINOR_SYNTAX_CODEGENERATOR_INCLUDED

#include <cminor/syntax/ParserFileContent.hpp>

namespace cminor { namespace syntax {

void SetForceCodeGen(bool force);
void GenerateCode(const std::vector<std::unique_ptr<ParserFileContent>>& parserFiles);

} } // namespace cminor::syntax

#endif // CMINOR_SYNTAX_CODEGENERATOR_INCLUDED
