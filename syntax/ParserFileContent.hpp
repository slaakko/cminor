// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYNTAX_PARSERFILECONTENT_INCLUDED
#define CMINOR_SYNTAX_PARSERFILECONTENT_INCLUDED

#include <cminor/parsing/ParsingDomain.hpp>
#include <cminor/parsing/Namespace.hpp>

namespace cminor { namespace syntax {

class ParserFileContent
{
public:
    ParserFileContent(int id_, cminor::parsing::ParsingDomain* parsingDomain_);
    void SetFilePath(const std::string& filePath_) { filePath = filePath_; }
    void AddIncludeDirective(const std::string& includeDirective_, const std::string& fileAttribute_);
    void AddUsingObject(cminor::codedom::UsingObject* usingObjectSubject);
    void AddGrammar(cminor::parsing::Grammar* grammar);
    void BeginNamespace(const std::u32string& ns);
    void EndNamespace();
    int Id() const { return id; }
    const std::string& FilePath() const { return filePath; }
    cminor::parsing::ParsingDomain* ParsingDomain() const { return parsingDomain; }
    const std::vector<std::pair<std::string, std::string>>& IncludeDirectives() const { return includeDirectives; }
    const std::vector<std::unique_ptr<cminor::parsing::UsingObject>>& UsingObjects() const { return usingObjects; }
    cminor::parsing::Scope* CurrentScope() const { return parsingDomain->CurrentScope(); }
private:
    int id;
    std::string filePath;
    cminor::parsing::ParsingDomain* parsingDomain;
    std::vector<std::pair<std::string, std::string>> includeDirectives;
    std::vector<std::unique_ptr<cminor::parsing::UsingObject>> usingObjects;
};

} } // namespace cminor::syntax

#endif // CMINOR_SYNTAX_PARSERFILECONTENT_INCLUDED
