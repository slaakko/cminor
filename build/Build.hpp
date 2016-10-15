// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BUILD_BUILD_INCLUDED
#define CMINOR_BUILD_BUILD_INCLUDED
#include <string>
#include <set>

namespace cminor { namespace build {

struct AssemblyReferenceInfo
{
    AssemblyReferenceInfo(const std::string& filePath_, bool isSystemAssembly_) : filePath(filePath_), isSystemAssembly(isSystemAssembly_) {}
    std::string filePath;
    bool isSystemAssembly;
};

inline bool operator<(const AssemblyReferenceInfo& left, const AssemblyReferenceInfo& right)
{
    if (left.filePath < right.filePath) return true;
    if (right.filePath < left.filePath) return false;
    if (left.isSystemAssembly && !right.isSystemAssembly) return true;
    return false;
}

void BuildProject(const std::string& projectFilePath, std::set<AssemblyReferenceInfo>& assemblyReferenceInfos);
void BuildSolution(const std::string& solutionFilePath);

} } // namespace cminor::build

#endif // CMINOR_BUILD_BUILD_INCLUDED
