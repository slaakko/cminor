// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_SOLUTION_INCLUDED
#define CMINOR_AST_SOLUTION_INCLUDED
#include <cminor/ast/Project.hpp>
#include <unordered_map>

namespace cminor { namespace ast {

class SolutionDeclaration
{
public:
    virtual ~SolutionDeclaration();
};

class SolutionProjectDeclaration : public SolutionDeclaration
{
public:
    SolutionProjectDeclaration(const std::string& filePath_);
    const std::string& FilePath() const { return filePath; }
private:
    std::string filePath;
};

class ProjectDependencyDeclaration : public SolutionDeclaration
{
public:
    ProjectDependencyDeclaration(const std::u32string& projectName_);
    void AddDependency(const std::u32string& dependsOn);
    const std::u32string& ProjectName() const { return projectName; }
    const std::vector<std::u32string>& DependsOnProjects() const { return dependsOnProjects; }
private:
    std::u32string projectName;
    std::vector<std::u32string> dependsOnProjects;
};

class SolutionFormatter
{
public:
    virtual ~SolutionFormatter();
    virtual void BeginFormat() {}
    virtual void EndFormat() {}
    virtual void FormatName(const std::string& name) {}
    virtual void FormatProjectFilePath(const std::string& projectFilePath) {}
};

class Solution
{
public:
    Solution(const std::u32string& name_, const std::string& filePath_);
    void AddDeclaration(SolutionDeclaration* declaration);
    void ResolveDeclarations();
    const std::u32string& Name() const { return name; }
    const std::string& FilePath() const { return filePath; }
    const boost::filesystem::path& BasePath() const { return basePath; }
    const std::vector<std::string>& ProjectFilePaths() const { return projectFilePaths; }
    void AddProject(std::unique_ptr<Project>&& project);
    std::vector<Project*> CreateBuildOrder();
    void Format(SolutionFormatter& formatter);
private:
    std::u32string name;
    std::string filePath;
    boost::filesystem::path basePath;
    std::vector<std::unique_ptr<SolutionDeclaration>> declarations;
    std::vector<std::unique_ptr<ProjectDependencyDeclaration>> additionalDependencyDeclarations;
    std::vector<std::string> projectFilePaths;
    std::vector<std::unique_ptr<Project>> projects;
    std::unordered_map<std::u32string, ProjectDependencyDeclaration*> dependencyMap;
    void AddDependencies();
};

} } // namespace cminor::ast

#endif // CMINOR_AST_SOLUTION_INCLUDED
