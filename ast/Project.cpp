// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Project.hpp>
#include <algorithm>

namespace cminor { namespace ast {

ProjectDeclaration::~ProjectDeclaration()
{
}

AssemblyReferenceDeclaration::AssemblyReferenceDeclaration(const std::string& filePath_) : filePath(filePath_)
{
}

SourceFileDeclaration::SourceFileDeclaration(const std::string& filePath_) : filePath(filePath_)
{
}

Project::Project(const std::string& name_, const std::string& filePath_) : name(name_), filePath(filePath_), basePath(filePath)
{
    basePath.remove_filename();
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
                sfp.replace_extension(".cmn");
            }
            if (!boost::filesystem::exists(sfp))
            {
                throw std::runtime_error("source file path '" + sfp.generic_string() + "' not found");
            }
            if (std::find(sourceFilePaths.cbegin(), sourceFilePaths.cend(), sfp.generic_string()) == sourceFilePaths.cend())
            {
                sourceFilePaths.push_back(sfp.generic_string());
            }
        }
        else
        {
            throw std::runtime_error("unknown project declaration");
        }
    }
}


} } // namespace cminor::ast
