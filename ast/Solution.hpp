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
    ProjectDependencyDeclaration(const std::string& projectName_);
    void AddDependency(const std::string& dependsOn);
    const std::string& ProjectName() const { return projectName; }
    const std::vector<std::string>& DependsOnProjects() const { return dependsOnProjects; }
private:
    std::string projectName;
    std::vector<std::string> dependsOnProjects;
};

class Solution
{
public:
    Solution(const std::string& name_, const std::string& filePath_);
    void AddDeclaration(SolutionDeclaration* declaration);
    void ResolveDeclarations();
    const std::string& Name() const { return name; }
    const std::string& FilePath() const { return filePath; }
    const boost::filesystem::path& BasePath() const { return basePath; }
    const std::vector<std::string>& ProjectFilePaths() const { return projectFilePaths; }
    void AddProject(std::unique_ptr<Project>&& project);
    std::vector<Project*> CreateBuildOrder();
private:
    std::string name;
    std::string filePath;
    boost::filesystem::path basePath;
    std::vector<std::unique_ptr<SolutionDeclaration>> declarations;
    std::vector<std::unique_ptr<ProjectDependencyDeclaration>> additionalDependencyDeclarations;
    std::vector<std::string> projectFilePaths;
    std::vector<std::unique_ptr<Project>> projects;
    std::unordered_map<std::string, ProjectDependencyDeclaration*> dependencyMap;
    void AddDependencies();
};

} } // namespace cminor::ast

#endif // CMINOR_AST_SOLUTION_INCLUDED
