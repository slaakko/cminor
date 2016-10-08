// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/build/Build.hpp>
#include <cminor/parser/ProjectFile.hpp>
#include <cminor/parser/CompileUnit.hpp>
#include <cminor/binder/TypeBinderVisitor.hpp>
#include <cminor/binder/StatementBinderVisitor.hpp>
#include <cminor/binder/BoundCompileUnit.hpp>
#include <cminor/symbols/GlobalFlags.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolCreatorVisitor.hpp>
#include <cminor/machine/FileRegistry.hpp>
#include <cminor/machine/MappedInputFile.hpp>
#include <cminor/machine/Machine.hpp>
#include <iostream>

namespace cminor { namespace build {

using namespace cminor::parser;
using namespace cminor::ast;
using namespace cminor::symbols;
using namespace cminor::binder;
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
        std::string s;
        if (sourceFilePaths.size() > 1)
        {
            s = "s";
        }
        std::cout << sourceFilePaths.size() << " source file" << s << " parsed" << std::endl;
    }
    return compileUnits;
}

void BuildSymbolTable(Assembly& assembly, const std::vector<std::unique_ptr<CompileUnitNode>>& compileUnits)
{
    SymbolCreatorVisitor symbolCreatorVisitor(assembly);
    for (const std::unique_ptr<CompileUnitNode>& compileUnit : compileUnits)
    {
        compileUnit->Accept(symbolCreatorVisitor);
    }
}

std::vector<std::unique_ptr<BoundCompileUnit>> BindTypes(Assembly& assembly, const std::vector<std::unique_ptr<CompileUnitNode>>& compileUnits)
{
    std::vector<std::unique_ptr<BoundCompileUnit>> boundCompileUnits;
    for (const std::unique_ptr<CompileUnitNode>& compileUnit : compileUnits)
    {
        std::unique_ptr<BoundCompileUnit> boundCompileUnit(new BoundCompileUnit(assembly, compileUnit.get()));
        TypeBinderVisitor typeBinderVisitor(*boundCompileUnit);
        compileUnit->Accept(typeBinderVisitor);
        boundCompileUnits.push_back(std::move(boundCompileUnit));
    }
    return boundCompileUnits;
}

void BindStatements(std::vector<std::unique_ptr<BoundCompileUnit>>& boundCompileUnits)
{
    for (const std::unique_ptr<BoundCompileUnit>& boundCompileUnit : boundCompileUnits)
    {
        StatementBinderVisitor statementBinderVisitor(*boundCompileUnit);
        boundCompileUnit->GetCompileUnitNode()->Accept(statementBinderVisitor);
    }
}

ProjectGrammar* projectGrammar = nullptr;

void BuildProject(const std::string& projectFilePath)
{
    if (!projectGrammar)
    {
        projectGrammar = ProjectGrammar::Create();
    }
    std::string config = GetConfig();
    MappedInputFile projectFile(projectFilePath);
    std::unique_ptr<Project> project(projectGrammar->Parse(projectFile.Begin(), projectFile.End(), 0, projectFilePath, config));
    project->ResolveDeclarations();
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "Building project '" << project->Name() << "(" << projectFilePath << " using " << config << " configuration." << std::endl;
    }
    std::vector<std::unique_ptr<CompileUnitNode>> compileUnits = ParseSources(project->SourceFilePaths());
    utf32_string assemblyName = ToUtf32(project->Name());
    Machine machine;
    Assembly assembly(assemblyName, project->AssemblyFilePath());
    assembly.ImportAssemblies(machine, project->AssemblyReferences());
    BuildSymbolTable(assembly, compileUnits);
    std::vector<std::unique_ptr<BoundCompileUnit>> boundCompileUnits = BindTypes(assembly, compileUnits);
    BindStatements(boundCompileUnits);
    boost::filesystem::path obp(assembly.FilePath());
    obp.remove_filename();
    boost::filesystem::create_directories(obp);
    SymbolWriter writer(assembly.FilePath());
    assembly.Write(writer);
}

void BuildSolution(const std::string& solutionFilePath)
{
    throw std::runtime_error("not implemented yet");
}

} } // namespace cminor::build
