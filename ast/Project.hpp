// =================================
// Copyright (c) 2017 Seppo Laakko
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

class ReferenceDeclaration : public ProjectDeclaration
{
public:
    ReferenceDeclaration(const std::string& filePath_);
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

enum class Target
{
    program, library
};

class TargetDeclaration : public ProjectDeclaration
{ 
public:
    TargetDeclaration(Target target_);
    Target GetTarget() const { return target; }
private:
    Target target;
};

class ProjectFormatter
{
public:
    virtual ~ProjectFormatter();
    virtual void BeginFormat() {}
    virtual void EndFormat() {}
    virtual void FormatName(const std::string& name) {}
    virtual void FormatTarget(Target target) {}
    virtual void FormatAssemblyReference(const std::string& assemblyReference) {}
    virtual void FormatSourceFilePath(const std::string& sourceFilePath) {}
};

class Project
{
public:
    Project(const std::string& name_, const std::string& filePath_, const std::string& config_);
    void AddDeclaration(ProjectDeclaration* declaration);
    void ResolveDeclarations();
    const std::string& Name() const { return name; }
    const std::string& FilePath() const { return filePath; }
    const boost::filesystem::path& BasePath() const { return basePath; }
    const std::string& AssemblyFilePath() const { return assemblyFilePath; }
    const std::vector<std::string>& AssemblyReferences() const { return assemblyReferences; }
    const std::vector<std::string>& SourceFilePaths() const { return sourceFilePaths; }
    Target GetTarget() const { return target; }
    bool DependsOn(Project* that) const;
    bool IsSystemProject() const { return isSystemProject; }
    void SetSystemProject() { isSystemProject = true; }
    void Format(ProjectFormatter& formatter);
private:
    std::string name;
    std::string filePath;
    std::string config;
    Target target;
    boost::filesystem::path basePath;
    boost::filesystem::path systemAssemblyDir;
    std::vector<std::unique_ptr<ProjectDeclaration>> declarations;
    std::string assemblyFilePath;
    std::vector<std::string> assemblyReferences;
    std::vector<std::string> sourceFilePaths;
    bool isSystemProject;
};

std::string CminorRootDir();
std::string CminorSystemDir();
std::string CminorSystemAssemblyDir(const std::string& config);
std::string CminorSystemAssemblyFilePath(const std::string& config);
std::string CminorSystemCoreAssemblyFilePath(const std::string& config);


} } // namespace cminor::ast

#endif // CMINOR_AST_PROJECT_INCLUDED
