// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/parsing/Linking.hpp>
#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Scope.hpp>
#include <cminor/parsing/Rule.hpp>
#include <cminor/parsing/Exception.hpp>
#include <cminor/parsing/Action.hpp>
#include <cminor/parsing/Nonterminal.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/parsing/ParsingDomain.hpp>
#include <cminor/parsing/Namespace.hpp>
#include <cminor/parsing/Primitive.hpp>
#include <cminor/util/Unicode.hpp>

namespace cminor { namespace parsing {

using namespace cminor::util;
using namespace cminor::unicode;

LinkerVisitor::LinkerVisitor(): currentGrammar(0), currentRule(0)
{
}

void LinkerVisitor::BeginVisit(Grammar& grammar)
{
    if (!grammar.IsExternal())
    {
        currentGrammar = &grammar;
    }
    else
    {
        currentGrammar = 0;
    }
}

void LinkerVisitor::EndVisit(Grammar& grammar)
{
    if (currentGrammar)
    {
        currentGrammar->ResolveStartRule();
        currentGrammar->ResolveSkipRule();
    }
}

void LinkerVisitor::Visit(RuleLink& link) 
{
    if (currentGrammar)
    {
        ParsingObject* object = currentGrammar->GetScope()->Get(link.LinkedRuleName());
        if (object)
        {
            if (object->IsRule())
            {
                Rule* rule = static_cast<Rule*>(object);
                link.SetRule(rule);
                currentGrammar->AddGrammarReference(rule->GetGrammar());
                link.AddToScope();
            }
            else
            {
                ThrowException("'" + ToUtf8(link.LinkedRuleName()) + "' is not a rule", link.GetSpan());
            }
        }
        else
        {
            ThrowException("rule '" + ToUtf8(link.LinkedRuleName()) + "' not found", link.GetSpan());
        }
    }
}

void LinkerVisitor::BeginVisit(Rule& rule)
{
    if (currentGrammar)
    {
        currentRule = &rule;
        actionNumber = 0;
    }
}

void LinkerVisitor::EndVisit(Rule& rule)
{
    if (currentGrammar)
    {
        rule.Link();
    }
}

void LinkerVisitor::BeginVisit(ActionParser& parser)
{
    if (currentGrammar)
    {
        if (parser.Name().empty())
        {
            parser.SetName(U"A" + ToUtf32(std::to_string(actionNumber)));
            ++actionNumber;
        }
        currentRule->AddAction(static_cast<ActionParser*>(&parser));
    }
}

void LinkerVisitor::Visit(NonterminalParser& parser)
{
    if (currentGrammar)
    {
        try
        {
            Rule* rule = currentGrammar->GetRule(parser.RuleName());
            parser.SetRule(rule);
            if (parser.Specialized())
            {
                currentRule->AddNonterminal(static_cast<NonterminalParser*>(&parser));
            }
        }
        catch (const ParsingException& ex)
        {
            ThrowException(ex.Message(), parser.GetSpan());
        }
        catch (const std::exception& ex)
        {
            ThrowException(ex.what(), parser.GetSpan());
        }
        if (parser.NumberOfArguments() != parser.GetRule()->NumberOfParameters())
        {
            ThrowException("rule '" + ToUtf8(parser.RuleName()) + "' takes " + std::to_string(parser.GetRule()->NumberOfParameters()) + " parameters (" +
                std::to_string(parser.NumberOfArguments()) + " arguments supplied)", parser.GetSpan());
        }
    }
}

void LinkerVisitor::Visit(KeywordParser& parser)
{
    if (currentGrammar)
    {
        if (!parser.ContinuationRuleName().empty())
        {
            parser.SetContinuationRule(currentGrammar->GetRule(parser.ContinuationRuleName()));
        }
    }
}

void LinkerVisitor::Visit(KeywordListParser& parser)
{
    if (currentGrammar)
    {
        parser.SetSelectorRule(currentGrammar->GetRule(parser.SelectorRuleName()));
    }
}


void Link(ParsingDomain* parsingDomain)
{
    LinkerVisitor visitor;
    parsingDomain->GlobalNamespace()->Accept(visitor);
}

class ExpandCodeVisitor : public Visitor
{
public:
    virtual void BeginVisit(Grammar& grammar)
    {
        if (!grammar.IsExternal())
        {
            const Grammar::RuleVec& rules = grammar.Rules();
            int n = int(rules.size());
            for (int i = 0; i < n; ++i)
            {
                Rule* rule = rules[i];
                rule->ExpandCode();
            }
        }
    }
};

void ExpandCode(ParsingDomain* parsingDomain)
{
    ExpandCodeVisitor visitor;
    parsingDomain->GlobalNamespace()->Accept(visitor);
}

} } // namespace cminor::parsing
