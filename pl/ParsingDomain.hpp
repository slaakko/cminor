// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_PARSING_PARSING_DOMAIN_INCLUDED
#define CMINOR_PARSING_PARSING_DOMAIN_INCLUDED
#include <cminor/pl/ParsingObject.hpp>
#include <string>
#include <stack>
#include <unordered_map>

namespace cminor { namespace parsing {

class Scope;
class Grammar;
class Namespace;

class ParsingDomain: public ParsingObject
{
public:
    ParsingDomain();
    Scope* GetNamespaceScope(const std::string& fullNamespaceName);
    Grammar* GetGrammar(const std::string& grammarName);
    void AddGrammar(Grammar* grammar);
    void BeginNamespace(const std::string& ns);
    void EndNamespace();
    Namespace* GlobalNamespace() const { return globalNamespace; }
    Namespace* CurrentNamespace() const { return currentNamespace; }
    Scope* CurrentScope() const;
    virtual void Accept(Visitor& visitor);
    int GetNextRuleId() { return nextRuleId++; }
    int GetNumRules() const { return nextRuleId; }
private:
    typedef std::unordered_map<std::string, Grammar*> GrammarMap;
    typedef GrammarMap::const_iterator GrammarMapIt;
    GrammarMap grammarMap;
    Namespace* globalNamespace;
    Scope* globalScope;
    typedef std::unordered_map<std::string, Namespace*> NamespaceMap;
    typedef NamespaceMap::const_iterator NamespaceMapIt;
    NamespaceMap namespaceMap;
    std::stack<Namespace*> namespaceStack;
    Namespace* currentNamespace;
    int nextRuleId;
};

void RegisterParsingDomain(ParsingDomain* parsingDomain);
void ParsingDomainInit();
void ParsingDomainDone();

} } // namespace cminor::parsing

#endif // CMINOR_PARSING_PARSING_DOMAIN_INCLUDED
