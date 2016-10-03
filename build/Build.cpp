// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/build/Build.hpp>
#include <cminor/parser/ProjectFile.hpp>
#include <cminor/parser/CompileUnit.hpp>
#include <cminor/symbols/GlobalFlags.hpp>
#include <cminor/machine/FileRegistry.hpp>
#include <cminor/machine/MappedInputFile.hpp>
#include <iostream>

namespace cminor { namespace build {

using namespace cminor::parser;
using namespace cminor::ast;
using namespace cminor::symbols;
using namespace cminor::machine;

CompileUnitGrammar* compileUnitGrammar = nullptr;

std::vector<std::unique_ptr<CompileUnitNode>> ParseSources(const std::vector<std::string>& sourceFilePaths)
{
    if (!compileUnitGrammar)
    {
        compileUnitGrammar = CompileUnitGrammar::Create();
    }
    std::vector<std::unique_ptr<CompileUnitNode>> compileUnits;
    for (const std::string& sourceFilePath : sourceFilePaths)
    {
        MappedInputFile sourceFile(sourceFilePath);
        int fileIndex = FileRegistry::Instance()->RegisterParsedFile(sourceFilePath);
        ParsingContext parsingContext;
        std::unique_ptr<CompileUnitNode> compileUnit(compileUnitGrammar->Parse(sourceFile.Begin(), sourceFile.End(), fileIndex, sourceFilePath, &parsingContext));
        compileUnits.push_back(std::move(compileUnit));
    }
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "parsed " << sourceFilePaths.size() << " source files" << std::endl;
    }
    return compileUnits;
}

ProjectGrammar* projectGrammar = nullptr;

void BuildProject(const std::string& projectFilePath)
{
    if (!projectGrammar)
    {
        projectGrammar = ProjectGrammar::Create();
    }
    MappedInputFile projectFile(projectFilePath);
    std::unique_ptr<Project> project(projectGrammar->Parse(projectFile.Begin(), projectFile.End(), 0, projectFilePath));
    project->ResolveDeclarations();
    std::vector<std::unique_ptr<CompileUnitNode>> compileUnits = ParseSources(project->SourceFilePaths());
}

} } // namespace cminor::build
