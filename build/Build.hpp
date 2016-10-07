// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BUILD_BUILD_INCLUDED
#define CMINOR_BUILD_BUILD_INCLUDED
#include <string>

namespace cminor { namespace build {

void BuildProject(const std::string& projectFilePath);
void BuildSolution(const std::string& solutionFilePath);

} } // namespace cminor::build

#endif // CMINOR_BUILD_BUILD_INCLUDED
