// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_PROJECT_INCLUDED
#define CMINOR_AST_PROJECT_INCLUDED
#include <boost/filesystem.hpp>
#include <string>
#include <vector>
#include <memory>

namespace cminor { namespace ast {

class ProjectDeclaration
{
public:
    virtual ~ProjectDeclaration();
};

class AssemblyReferenceDeclaration : public ProjectDeclaration
{
public:
    AssemblyReferenceDeclaration(const std::string& filePath_);
    const std::string& FilePath() const { return filePath; }
private:
    std::string filePath;
};

class SourceFileDeclaration : public ProjectDeclaration
{
public:
    SourceFileDeclaration(const std::string& filePath_);
    const std::string& FilePath() const { return filePath; }
private:
    std::string filePath;
};

class Project
{
public:
    Project(const std::string& name_, const std::string& filePath_);
    void AddDeclaration(ProjectDeclaration* declaration);
    void ResolveDeclarations();
    const std::string& Name() const { return name; }
    const std::string& FilePath() const { return filePath; }
    const boost::filesystem::path& BasePath() const { return basePath; }
    const std::vector<std::string>& SourceFilePaths() const { return sourceFilePaths; }
private:
    std::string name;
    std::string filePath;
    boost::filesystem::path basePath;
    std::vector<std::unique_ptr<ProjectDeclaration>> declarations;
    std::vector<std::string> sourceFilePaths;
};

} } // namespace cminor::ast

#endif // CMINOR_AST_PROJECT_INCLUDED
