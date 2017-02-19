// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Project.hpp>
#include <cminor/util/Path.hpp>
#include <algorithm>

namespace cminor { namespace ast {

using namespace cminor::util;

ProjectFormatter::~ProjectFormatter()
{
}

ProjectDeclaration::~ProjectDeclaration()
{
}

ReferenceDeclaration::ReferenceDeclaration(const std::string& filePath_) : filePath(filePath_)
{
}

SourceFileDeclaration::SourceFileDeclaration(const std::string& filePath_) : filePath(filePath_)
{
}

TargetDeclaration::TargetDeclaration(Target target_) : target(target_)
{
}

Project::Project(const std::string& name_, const std::string& filePath_, const std::string& config_) : 
    name(name_), filePath(filePath_), config(config_), target(Target::program), basePath(filePath), isSystemProject(false)
{
    basePath.remove_filename();
    systemAssemblyDir = CminorSystemAssemblyDir(config);
    boost::filesystem::path afp(filePath);
    boost::filesystem::path fn = afp.filename();
    afp.remove_filename();
    afp /= "assembly";
    afp /= config;
    afp /= fn;
    afp.replace_extension(".cminora");
    assemblyFilePath = GetFullPath(afp.generic_string());
}

void Project::AddDeclaration(ProjectDeclaration* declaration)
{
    declarations.push_back(std::unique_ptr<ProjectDeclaration>(declaration));
}

void Project::ResolveDeclarations()
{
    for (const std::unique_ptr<ProjectDeclaration>& declaration : declarations)
    {
        if (ReferenceDeclaration* reference = dynamic_cast<ReferenceDeclaration*>(declaration.get()))
        {
            boost::filesystem::path rp(reference->FilePath());
            boost::filesystem::path fn = rp.filename();
            rp.remove_filename();
            if (rp.is_relative())
            {
                rp = systemAssemblyDir / rp;
            }
            rp /= fn;
            if (rp.extension() == ".cminorp")
            {
                rp.replace_extension(".cminora");
            }
            if (rp.extension() != ".cminora")
            {
                throw std::runtime_error("invalid reference path extension '" + rp.generic_string() + "' (not .cminorp or .cminora)");
            }
            if (!boost::filesystem::exists(rp))
            {
                rp = reference->FilePath();
                rp.remove_filename();
                if (rp.is_relative())
                {
                    rp = basePath / rp;
                }
                rp /= "assembly";
                rp /= config;
                rp /= fn;
                if (rp.extension() == ".cminorp")
                {
                    rp.replace_extension(".cminora");
                }
                if (rp.extension() != ".cminora")
                {
                    throw std::runtime_error("invalid reference path extension '" + rp.generic_string() + "' (not .cminorp or .cminora)");
                }
            }
            std::string assemblyReferencePath = GetFullPath(rp.generic_string());
            if (std::find(assemblyReferences.cbegin(), assemblyReferences.cend(), assemblyReferencePath) == assemblyReferences.cend())
            {
                assemblyReferences.push_back(assemblyReferencePath);
            }
        }
        else if (SourceFileDeclaration* sourceFileDeclaration = dynamic_cast<SourceFileDeclaration*>(declaration.get()))
        {
            boost::filesystem::path sfp(sourceFileDeclaration->FilePath());
            if (sfp.is_relative())
            {
                sfp = basePath / sfp;
            }
            if (sfp.extension() != ".cminor")
            {
                throw std::runtime_error("invalid source file extension '" + sfp.generic_string() + "' (not .cminor)");
            }
            if (!boost::filesystem::exists(sfp))
            {
                throw std::runtime_error("source file path '" + GetFullPath(sfp.generic_string()) + "' not found");
            }
            std::string sourceFilePath = GetFullPath(sfp.generic_string());
            if (std::find(sourceFilePaths.cbegin(), sourceFilePaths.cend(), sourceFilePath) == sourceFilePaths.cend())
            {
                sourceFilePaths.push_back(sourceFilePath);
            }
        }
        else if (TargetDeclaration* targetDeclaration = dynamic_cast<TargetDeclaration*>(declaration.get()))
        {
            target = targetDeclaration->GetTarget();
        }
        else
        {
            throw std::runtime_error("unknown project declaration");
        }
    }
}

void Project::Format(ProjectFormatter& formatter)
{
    formatter.BeginFormat();
    formatter.FormatName(name);
    for (const std::unique_ptr<ProjectDeclaration>& declaration : declarations)
    {
        if (TargetDeclaration* targetDeclaration = dynamic_cast<TargetDeclaration*>(declaration.get()))
        {
            Target target = targetDeclaration->GetTarget();
            formatter.FormatTarget(target);
        }
        else if (ReferenceDeclaration* reference = dynamic_cast<ReferenceDeclaration*>(declaration.get()))
        {
            formatter.FormatAssemblyReference(reference->FilePath());
        }
        else if (SourceFileDeclaration* sourceFileDeclaration = dynamic_cast<SourceFileDeclaration*>(declaration.get()))
        {
            formatter.FormatSourceFilePath(sourceFileDeclaration->FilePath());
        }
    }
    formatter.EndFormat();
}

bool Project::DependsOn(Project* that) const
{
    return std::find(assemblyReferences.cbegin(), assemblyReferences.cend(), that->assemblyFilePath) != assemblyReferences.cend();
}

std::string CminorRootDir()
{
    char* e = getenv("CMINOR_ROOT");
    if (e == nullptr || !*e)
    {
        throw std::runtime_error("CMINOR_ROOT environment variable not set");
    }
    return std::string(e);
}

std::string CminorLibDir()
{
    boost::filesystem::path s(CminorRootDir());
    s /= "lib";
    return GetFullPath(s.generic_string());
}

std::string CminorSystemDir()
{
    boost::filesystem::path s(CminorRootDir());
    s /= "system";
    return GetFullPath(s.generic_string());
}

std::string CminorSystemAssemblyDir(const std::string& config)
{
    boost::filesystem::path sad(CminorRootDir());
    sad /= "system";
    sad /= "assembly";
    sad /= config;
    return GetFullPath(sad.generic_string());
}

std::string CminorSystemAssemblyFilePath(const std::string& config)
{
    boost::filesystem::path safp(CminorSystemAssemblyDir(config));
    safp /= "System.cminora";
    return GetFullPath(safp.generic_string());
}

std::string CminorSystemCoreAssemblyFilePath(const std::string& config)
{
    boost::filesystem::path scafp(CminorSystemDir());
    scafp /= "System.Core";
    scafp /= "assembly";
    scafp /= config;
    scafp /= "System.Core.cminora";
    return GetFullPath(scafp.generic_string());
}

} } // namespace cminor::ast
