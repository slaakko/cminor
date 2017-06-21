// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/syntax/Generator.hpp>
#include <cminor/syntax/ProjectFile.hpp>
#include <cminor/syntax/LibraryFile.hpp>
#include <cminor/syntax/ParserFile.hpp>
#include <cminor/syntax/Library.hpp>
#include <cminor/syntax/CodeGenerator.hpp>
#include <cminor/util/TextUtils.hpp>
#include <cminor/util/MappedInputFile.hpp>
#include <cminor/util/Unicode.hpp>
#include <cminor/parsing/Linking.hpp>
#include <cminor/parsing/ParsingDomain.hpp>
#include <cminor/parsing/Exception.hpp>
#include <boost/filesystem.hpp>
#include <iostream>

namespace cminor { namespace syntax {

using namespace cminor::util;
using namespace cminor::unicode;
using namespace boost::filesystem;

std::string ResolveReferenceFilePath(const std::string& relativeReferenceFilePath, const std::string& projectBasePath, const std::vector<std::string>& libraryDirectories)
{
    path base(projectBasePath);
    path rrfp(relativeReferenceFilePath);
    path referenceFilePath = absolute(rrfp, base);
    if (exists(referenceFilePath))
    {
        return referenceFilePath.generic_string();
    }
    int n = int(libraryDirectories.size());
    for (int i = 0; i < n; ++i)
    {
        base = path(libraryDirectories[i]);
        referenceFilePath = absolute(rrfp, base);
        if (exists(referenceFilePath))
        {
            return referenceFilePath.generic_string();
        }
    }
    throw std::runtime_error("library file path '" + relativeReferenceFilePath + "' not found");
}

void Generate(const std::string& projectFilePath, const std::vector<std::string>& libraryDirectories)
{
    std::cout << "Parsing project file " << projectFilePath << "...\n";
    std::unique_ptr<cminor::parsing::ParsingDomain> projectParsingDomain(new cminor::parsing::ParsingDomain());
    projectParsingDomain->SetOwned();
    ProjectFileGrammar* projectFileGrammar = ProjectFileGrammar::Create(projectParsingDomain.get());
    LibraryFileGrammar* libraryFileGrammar = LibraryFileGrammar::Create(projectParsingDomain.get());
    ParserFileGrammar* parserFileGrammar = ParserFileGrammar::Create(projectParsingDomain.get());
    std::u32string projectFileContent = ToUtf32(ReadFile(projectFilePath));
    std::unique_ptr<Project> project(projectFileGrammar->Parse(&projectFileContent[0], &projectFileContent[0] + projectFileContent.length(), 0, projectFilePath));
    std::cout << "Compiling project '" << project->Name() << "'...\n";
    std::unique_ptr<cminor::parsing::ParsingDomain> parsingDomain(new cminor::parsing::ParsingDomain());
    parsingDomain->SetOwned();
    std::cout << "Parsing library files...\n";
    const std::vector<std::string>& referenceFiles = project->ReferenceFiles();
    int nr = int(referenceFiles.size());
    for (int i = 0; i < nr; ++i)
    {
        std::string relativeReferenceFilePath = referenceFiles[i];
        std::string referenceFilePath = ResolveReferenceFilePath(relativeReferenceFilePath, project->BasePath(), libraryDirectories);
        std::cout << "> " << referenceFilePath << "\n";
        std::u32string libraryFileContent = ToUtf32(ReadFile(referenceFilePath));
        libraryFileGrammar->Parse(&libraryFileContent[0], &libraryFileContent[0] + libraryFileContent.length(), i, referenceFilePath, parsingDomain.get());
    }
    std::cout << "Parsing source files...\n";
    std::vector<std::unique_ptr<ParserFileContent>> parserFiles;
    const std::vector<std::string>& sourceFiles = project->SourceFiles();
    cminor::parsing::SetFileInfo(sourceFiles);
    int n = int(sourceFiles.size());
    for (int i = 0; i < n; ++i)
    {
        std::string sourceFilePath = sourceFiles[i];
        std::cout << "> " << sourceFilePath << "\n";
        std::u32string sourceFileContent = ToUtf32(ReadFile(sourceFilePath));
        std::unique_ptr<ParserFileContent> parserFileContent(parserFileGrammar->Parse(&sourceFileContent[0], &sourceFileContent[0] + sourceFileContent.length(), i, sourceFilePath, i, parsingDomain.get()));
        parserFiles.push_back(std::move(parserFileContent));
    }
    std::cout << "Linking...\n";
    cminor::parsing::Link(parsingDomain.get());
    std::cout << "Expanding code...\n";
    cminor::parsing::ExpandCode(parsingDomain.get());
    std::cout << "Generating source code...\n";
    GenerateCode(parserFiles);
    std::cout << "Generating library file...\n";
    GenerateLibraryFile(project.get(), parsingDomain.get());
}

} } // namespace cminor::syntax
