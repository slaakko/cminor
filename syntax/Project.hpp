// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYNTAX_PROJECT_INCLUDED
#define CMINOR_SYNTAX_PROJECT_INCLUDED
#include <boost/filesystem.hpp>
#include <string>
#include <vector>
#include <memory>

namespace cminor { namespace syntax {

class Project
{
public:
    Project(const std::string& name_, const std::string& filePath_);
    const std::string& Name() const { return name; }
    const std::string& FilePath() const { return filePath; }
    std::string BasePath() const;
    void AddSourceFile(const std::string& sourceFilePath);
    void AddReferenceFile(const std::string& referenceFilePath);
    const std::vector<std::string>& ReferenceFiles() const { return referenceFiles; }
    const std::vector<std::string>& SourceFiles() const { return sourceFiles; }
private:
    std::string name;
    std::string filePath;
    std::vector<std::string> sourceFiles;
    std::vector<std::string> referenceFiles;
    boost::filesystem::path base;
};

} } // namespace cminor::syntax

#endif // CMINOR_SYNTAX_PROJECT_INCLUDED
