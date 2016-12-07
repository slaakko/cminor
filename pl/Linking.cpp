// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/pl/Linking.hpp>
#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Scope.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Namespace.hpp>
#include <cminor/pl/Primitive.hpp>

namespace cminor { namespace parsing {

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
                ThrowException("'" + link.LinkedRuleName() + "' is not a rule", link.GetSpan());
            }
        }
        else
        {
            ThrowException("rule '" + link.LinkedRuleName() + "' not found", link.GetSpan());
        }
    }
}

void LinkerVisitor::BeginVisit(Rule& rule)
{
    if (currentGrammar)
    {
        currentRule = &rule;
        actionNumber = 0;
        if (!currentGrammar->CCRuleName().empty())
        {
            if (rule.Name() == currentGrammar->CCRuleName())
            {
                rule.SetCCRule();
                rule.SetCCStart(currentGrammar->CCStart());
                rule.SetCCEnd(currentGrammar->CCEnd());
                if (currentGrammar->CCSkip())
                {
                    rule.SetCCSkip();
                }
            }
        }
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
            parser.SetName("A" + std::to_string(actionNumber));
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
            ThrowException("rule '" + parser.RuleName() + "' takes " + std::to_string(parser.GetRule()->NumberOfParameters()) + " parameters (" +
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


void LinkerVisitor::Visit(CharParser& parser)
{
    if (currentRule && currentRule->IsCCRule() && (currentRule->CCStart() == parser.GetChar() || currentRule->CCEnd() == parser.GetChar()))
    {
        parser.SetCCRule(currentRule);
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
