#include "Keyword.hpp"
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>

namespace cminor { namespace parser {

using namespace cminor::parsing;

KeywordGrammar* KeywordGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

KeywordGrammar* KeywordGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    KeywordGrammar* grammar(new KeywordGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

KeywordGrammar::KeywordGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("KeywordGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
    keywords0.push_back("abstract");
    keywords0.push_back("base");
    keywords0.push_back("bool");
    keywords0.push_back("break");
    keywords0.push_back("byte");
    keywords0.push_back("case");
    keywords0.push_back("cast");
    keywords0.push_back("catch");
    keywords0.push_back("char");
    keywords0.push_back("class");
    keywords0.push_back("const");
    keywords0.push_back("continue");
    keywords0.push_back("default");
    keywords0.push_back("delegate");
    keywords0.push_back("do");
    keywords0.push_back("double");
    keywords0.push_back("else");
    keywords0.push_back("enum");
    keywords0.push_back("false");
    keywords0.push_back("finally");
    keywords0.push_back("float");
    keywords0.push_back("for");
    keywords0.push_back("foreach");
    keywords0.push_back("goto");
    keywords0.push_back("if");
    keywords0.push_back("in");
    keywords0.push_back("inline");
    keywords0.push_back("int");
    keywords0.push_back("interface");
    keywords0.push_back("internal");
    keywords0.push_back("long");
    keywords0.push_back("namespace");
    keywords0.push_back("new");
    keywords0.push_back("null");
    keywords0.push_back("object");
    keywords0.push_back("operator");
    keywords0.push_back("override");
    keywords0.push_back("private");
    keywords0.push_back("protected");
    keywords0.push_back("public");
    keywords0.push_back("return");
    keywords0.push_back("sbyte");
    keywords0.push_back("short");
    keywords0.push_back("static");
    keywords0.push_back("string");
    keywords0.push_back("switch");
    keywords0.push_back("this");
    keywords0.push_back("throw");
    keywords0.push_back("true");
    keywords0.push_back("try");
    keywords0.push_back("uint");
    keywords0.push_back("ulong");
    keywords0.push_back("ushort");
    keywords0.push_back("using");
    keywords0.push_back("virtual");
    keywords0.push_back("void");
    keywords0.push_back("while");
}

void KeywordGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("cminor.parsing.stdlib");
    if (!grammar0)
    {
        grammar0 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar0);
}

void KeywordGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("identifier", this, "cminor.parsing.stdlib.identifier"));
    AddRule(new cminor::parsing::Rule("Keyword", GetScope(),
        new cminor::parsing::KeywordListParser("identifier", keywords0)));
}

} } // namespace cminor.parser
