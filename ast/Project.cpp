// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Project.hpp>
#include <Cm.Util/Path.hpp>
#include <algorithm>

namespace cminor { namespace ast {

using namespace Cm::Util;

ProjectDeclaration::~ProjectDeclaration()
{
}

AssemblyReferenceDeclaration::AssemblyReferenceDeclaration(const std::string& filePath_) : filePath(filePath_)
{
}

SourceFileDeclaration::SourceFileDeclaration(const std::string& filePath_) : filePath(filePath_)
{
}

TargetDeclaration::TargetDeclaration(Target target_) : target(target_)
{
}

Project::Project(const std::string& name_, const std::string& filePath_, const std::string& config_) : name(name_), filePath(filePath_), config(config_), target(Target::program), basePath(filePath)
{
    basePath.remove_filename();
    systemPath = CminorSystemDir();
    boost::filesystem::path afp(filePath);
    boost::filesystem::path fn = afp.filename();
    afp.remove_filename();
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
        if (AssemblyReferenceDeclaration* assemblyReference = dynamic_cast<AssemblyReferenceDeclaration*>(declaration.get()))
        {
            boost::filesystem::path arp(assemblyReference->FilePath());
            boost::filesystem::path fn = arp.filename();
            arp.remove_filename();
            if (arp.is_relative())
            {
                arp = systemPath / arp;
            }
            arp /= config;
            arp /= fn;
            if (!arp.has_extension())
            {
                arp.replace_extension(".cminora");
            }
            if (!boost::filesystem::exists(arp))
            {
                arp = assemblyReference->FilePath();
                arp.remove_filename();
                if (arp.is_relative())
                {
                    arp = basePath / arp;
                }
                arp /= config;
                arp /= fn;
                if (!arp.has_extension())
                {
                    arp.replace_extension(".cminora");
                }
                if (!boost::filesystem::exists(arp))
                {
                    throw std::runtime_error("assembly reference path '" + GetFullPath(arp.generic_string()) + "' not found");
                }
                std::string assemblyReferencePath = GetFullPath(arp.generic_string());
                if (std::find(assemblyReferences.cbegin(), assemblyReferences.cend(), assemblyReferencePath) == assemblyReferences.cend())
                {
                    assemblyReferences.push_back(assemblyReferencePath);
                }
            }
        }
        else if (SourceFileDeclaration* sourceFileDeclaration = dynamic_cast<SourceFileDeclaration*>(declaration.get()))
        {
            boost::filesystem::path sfp(sourceFileDeclaration->FilePath());
            if (sfp.is_relative())
            {
                sfp = basePath / sfp;
            }
            if (!sfp.has_extension())
            {
                sfp.replace_extension(".cminor");
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

std::string CminorRootDir()
{
    char* e = getenv("CMINOR_ROOT");
    if (e == nullptr || !*e)
    {
        throw std::runtime_error("CMINOR_ROOT environment variable not set");
    }
    return std::string(e);
}

std::string CminorSystemDir()
{
    boost::filesystem::path s(CminorRootDir());
    s /= "system";
    return GetFullPath(s.generic_string());
}

std::string CminorSystemAssemblyFilePath(const std::string& config)
{
    boost::filesystem::path safp(CminorSystemDir());
    safp /= config;
    safp /= "system.cminora";
    return GetFullPath(safp.generic_string());
}

} } // namespace cminor::ast
