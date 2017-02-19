// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BUILD_BUILD_INCLUDED
#define CMINOR_BUILD_BUILD_INCLUDED
#include <string>
#include <set>

namespace cminor { namespace build {

struct AssemblyReferenceInfo
{
    AssemblyReferenceInfo(const std::string& assemblyFilePath_, const std::string& nativeImportLibraryFilePath_, const std::string& nativeSharedLibraryFilePath_, bool isSystemAssembly_) : 
        assemblyFilePath(assemblyFilePath_), nativeImportLibraryFilePath(nativeImportLibraryFilePath_), nativeSharedLibraryFilePath(nativeSharedLibraryFilePath_), isSystemAssembly(isSystemAssembly_) {}
    std::string assemblyFilePath;
    std::string nativeImportLibraryFilePath;
    std::string nativeSharedLibraryFilePath;
    bool isSystemAssembly;
};

inline bool operator<(const AssemblyReferenceInfo& left, const AssemblyReferenceInfo& right)
{
    if (left.assemblyFilePath < right.assemblyFilePath) return true;
    if (right.assemblyFilePath < left.assemblyFilePath) return false;
    if (left.isSystemAssembly && !right.isSystemAssembly) return true;
    return false;
}

void BuildProject(const std::string& projectFilePath, std::set<AssemblyReferenceInfo>& assemblyReferenceInfos);
void BuildSolution(const std::string& solutionFilePath);

} } // namespace cminor::build

#endif // CMINOR_BUILD_BUILD_INCLUDED
