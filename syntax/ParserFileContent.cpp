// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/syntax/ParserFileContent.hpp>
#include <cminor/parsing/Grammar.hpp>

namespace cminor { namespace syntax {

using namespace cminor::parsing;

ParserFileContent::ParserFileContent(int id_, cminor::parsing::ParsingDomain* parsingDomain_): id(id_), parsingDomain(parsingDomain_)
{
}

void ParserFileContent::AddIncludeDirective(const std::string& includeDirective_, const std::string& fileAttribute_)
{
    includeDirectives.push_back(std::make_pair(includeDirective_, fileAttribute_));
}

void ParserFileContent::AddUsingObject(cminor::codedom::UsingObject* usingObjectSubject)
{
    cminor::parsing::Scope* enclosingScope = parsingDomain->CurrentScope();
    UsingObject* usingObject(new UsingObject(usingObjectSubject, enclosingScope));
    usingObject->SetOwner(id);
    usingObjects.push_back(std::unique_ptr<UsingObject>(usingObject));
}

void ParserFileContent::AddGrammar(cminor::parsing::Grammar* grammar)
{
    grammar->SetOwner(id);
    parsingDomain->AddGrammar(grammar);
}

void ParserFileContent::BeginNamespace(const std::u32string& ns)
{
    parsingDomain->BeginNamespace(ns);
}

void ParserFileContent::EndNamespace()
{
    parsingDomain->EndNamespace();
}

} } // namespace cminor::syntax
