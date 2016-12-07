// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Scope.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/Visitor.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/XmlLog.hpp>
#include <cminor/pl/Linking.hpp>
#include <cminor/pl/ParsingDomain.hpp>

namespace cminor { namespace parsing {

Grammar::Grammar(const std::string& name_, Scope* enclosingScope_): ParsingObject(name_, enclosingScope_), parsingDomain(new ParsingDomain()), ns(nullptr),
    linking(false), linked(false), contentParser(nullptr), startRule(nullptr), skipRule(nullptr), ccStart(), ccEnd(), ccSkip(false), log(0), maxLogLineLength(256)
{
    RegisterParsingDomain(parsingDomain);
    SetScope(new Scope(Name(), EnclosingScope()));
}

Grammar::Grammar(const std::string& name_, Scope* enclosingScope_, ParsingDomain* parsingDomain_): ParsingObject(name_, enclosingScope_), parsingDomain(parsingDomain_), ns(nullptr), 
    linking(false), linked(false), contentParser(nullptr), startRule(nullptr), skipRule(nullptr), ccStart(), ccEnd(), ccSkip(false), log(0), maxLogLineLength(256)
{
    SetScope(new Scope(Name(), EnclosingScope()));
}

void Grammar::SetCCRuleData(const std::string& ccRuleName_, char ccStart_, char ccEnd_, bool ccSkip_)
{
    ccRuleName = ccRuleName_;
    ccStart = ccStart_;
    ccEnd = ccEnd_;
    ccSkip = ccSkip_;
}

void Grammar::AddRule(Rule* rule)
{
    Own(rule);
    rule->SetGrammar(this);
    rule->GetScope()->SetEnclosingScope(GetScope());
    rule->SetEnclosingScope(GetScope());
    rules.push_back(rule);
    try
    {
        GetScope()->Add(rule);
    }
    catch (std::exception& ex)
    {
        ThrowException(ex.what(), GetSpan());
    }
}

Rule* Grammar::GetRule(const std::string& ruleName) const
{
    ParsingObject* object = GetScope()->Get(ruleName);
    if (!object)
    {
        ThrowException("rule '" + ruleName + "' not found in grammar '" + FullName() + "'", GetSpan());
    }
    if (object->IsRule())
    {
        return static_cast<Rule*>(object);
    }
    else if (object->IsRuleLink())
    {
        RuleLink* ruleLink = static_cast<RuleLink*>(object);
        Rule* rule = ruleLink->GetRule();
        if (rule)
        {
            return rule;
        }
        else
        {
            ThrowException("rule link '" + ruleName + "' not bound in grammar '" + FullName() + "'", GetSpan());
        }
    }
    else
    {
        ThrowException("'" + ruleName + "' is not a rule or rule link", GetSpan());
    }
    return nullptr;
}

void Grammar::Accept(Visitor& visitor)
{
    visitor.BeginVisit(*this);
    RuleLinkSetIt e = ruleLinks.cend();
    for (RuleLinkSetIt i = ruleLinks.cbegin(); i != e; ++i)
    {
        RuleLink* ruleLink = *i;
        ruleLink->Accept(visitor);
    }
    int n = int(rules.size());
    for (int i = 0; i < n; ++i)
    {
        Rule* rule = rules[i];
        rule->Accept(visitor);
    }
    visitor.EndVisit(*this);
}

void Grammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName)
{
    Scanner scanner(start, end, fileName, fileIndex, skipRule);
    std::unique_ptr<XmlLog> xmlLog;
    if (log)
    {
        xmlLog.reset(new XmlLog(*log, maxLogLineLength));
        scanner.SetLog(xmlLog.get());
        xmlLog->WriteBeginRule("parse");
    }
    ObjectStack stack;
    Match match = Parse(scanner, stack);
    Span stop = scanner.GetSpan();
    if (log)
    {
        xmlLog->WriteEndRule("parse");
    }
    if (!match.Hit() || !CC() && stop.Start() != int(end - start))
    {
        if (startRule)
        {
            throw ExpectationFailure(startRule->Info(), fileName, stop, start, end);
        }
        else
        {
            throw ParsingException("grammar '" + FullName() + "' has no start rule", fileName, stop, start, end);
        }
    }
}

Match Grammar::Parse(Scanner& scanner, ObjectStack& stack)
{
    if (startRule)
    {
        if (!contentParser)
        {
            if (skipRule)
            {
                contentParser = new SequenceParser(new SequenceParser(new OptionalParser(skipRule), startRule), new OptionalParser(skipRule));
                Own(contentParser);
            }
            else
            {
                contentParser = startRule;
            }
        }
        return contentParser->Parse(scanner, stack);
    }
    return Match::Nothing();
}

void Grammar::ResolveStartRule()
{
    if (startRuleName.empty())
    {
        if (!rules.empty())
        {
            startRule = rules.front();
        }
        else
        {
            ThrowException("cannot resolve start rule because grammar '" + FullName() + "' is empty", GetSpan());
        }
    }
    else
    {
        startRule = GetRule(startRuleName);
    }
}

void Grammar::ResolveSkipRule()
{
    if (!skipRuleName.empty())
    {
        skipRule = GetRule(skipRuleName);
    }
}

void Grammar::Link()
{
    if (!linked && !linking)
    {
        linking = true;
        GetReferencedGrammars();
        GrammarSetIt e = grammarReferences.end();
        for (GrammarSetIt i = grammarReferences.begin(); i != e; ++i)
        {
            Grammar* grammarReference = *i;
            grammarReference->Link();
        }
        LinkerVisitor linkerVisitor;
        Accept(linkerVisitor);
        linking = false;
        linked = true;
    }
}

void Grammar::AddGrammarReference(Grammar* grammarReference)
{
    Own(grammarReference);
    grammarReferences.insert(grammarReference);
}

void Grammar::AddRuleLink(RuleLink* ruleLink)
{
    Own(ruleLink);
    ruleLinks.insert(ruleLink);
}

} } // namespace cminor::parsing
