// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_PARSING_LINKING_INCLUDED
#define CMINOR_PARSING_LINKING_INCLUDED
#include <cminor/pl/Visitor.hpp>

namespace cminor { namespace parsing {

class Grammar;
class Rule;
class ParsingDomain;

class LinkerVisitor : public Visitor
{
public:
    LinkerVisitor();
    virtual void BeginVisit(Grammar& grammar);
    virtual void EndVisit(Grammar& grammar);
    virtual void Visit(RuleLink& link);
    virtual void BeginVisit(Rule& rule);
    virtual void EndVisit(Rule& rule);
    virtual void BeginVisit(ActionParser& parser);
    virtual void Visit(NonterminalParser& parser);
    virtual void Visit(KeywordParser& parser);
    virtual void Visit(KeywordListParser& parser);
    virtual void Visit(CharParser& parser);
private:
    Grammar* currentGrammar;
    Rule* currentRule;
    int actionNumber;
};

void Link(ParsingDomain* parsingDomain);
void ExpandCode(ParsingDomain* parsingDomain);

} } // namespace cminor::parsing

#endif // CMINOR_PARSING_LINKING_INCLUDED
