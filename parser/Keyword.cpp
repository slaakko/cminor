#include "Keyword.hpp"
#include <Cm.Parsing/Action.hpp>
#include <Cm.Parsing/Rule.hpp>
#include <Cm.Parsing/ParsingDomain.hpp>
#include <Cm.Parsing/Primitive.hpp>
#include <Cm.Parsing/Composite.hpp>
#include <Cm.Parsing/Nonterminal.hpp>
#include <Cm.Parsing/Exception.hpp>
#include <Cm.Parsing/StdLib.hpp>
#include <Cm.Parsing/XmlLog.hpp>

namespace cminor { namespace parser {

using namespace Cm::Parsing;

KeywordGrammar* KeywordGrammar::Create()
{
    return Create(new Cm::Parsing::ParsingDomain());
}

KeywordGrammar* KeywordGrammar::Create(Cm::Parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    KeywordGrammar* grammar(new KeywordGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

KeywordGrammar::KeywordGrammar(Cm::Parsing::ParsingDomain* parsingDomain_): Cm::Parsing::Grammar("KeywordGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
    keywords0.push_back("bool");
    keywords0.push_back("byte");
    keywords0.push_back("char");
    keywords0.push_back("double");
    keywords0.push_back("float");
    keywords0.push_back("int");
    keywords0.push_back("long");
    keywords0.push_back("sbyte");
    keywords0.push_back("short");
    keywords0.push_back("string");
    keywords0.push_back("uint");
    keywords0.push_back("ulong");
    keywords0.push_back("ushort");
    keywords0.push_back("void");
}

void KeywordGrammar::GetReferencedGrammars()
{
    Cm::Parsing::ParsingDomain* pd = GetParsingDomain();
    Cm::Parsing::Grammar* grammar0 = pd->GetGrammar("Cm.Parsing.stdlib");
    if (!grammar0)
    {
        grammar0 = Cm::Parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar0);
}

void KeywordGrammar::CreateRules()
{
    AddRuleLink(new Cm::Parsing::RuleLink("identifier", this, "Cm.Parsing.stdlib.identifier"));
    AddRule(new Cm::Parsing::Rule("Keyword", GetScope(),
        new Cm::Parsing::KeywordListParser("identifier", keywords0)));
}

} } // namespace cminor.parser
