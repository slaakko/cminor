#include "Keyword.hpp"
#include <cminor/parsing/Action.hpp>
#include <cminor/parsing/Rule.hpp>
#include <cminor/parsing/ParsingDomain.hpp>
#include <cminor/parsing/Primitive.hpp>
#include <cminor/parsing/Composite.hpp>
#include <cminor/parsing/Nonterminal.hpp>
#include <cminor/parsing/Exception.hpp>
#include <cminor/parsing/StdLib.hpp>
#include <cminor/parsing/XmlLog.hpp>
#include <cminor/util/Unicode.hpp>

namespace cminor { namespace parser {

using namespace cminor::parsing;
using namespace cminor::util;
using namespace cminor::unicode;

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

KeywordGrammar::KeywordGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("KeywordGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.parser")), parsingDomain_)
{
    SetOwner(0);
    keywords0.push_back(ToUtf32("abstract"));
    keywords0.push_back(ToUtf32("as"));
    keywords0.push_back(ToUtf32("base"));
    keywords0.push_back(ToUtf32("bool"));
    keywords0.push_back(ToUtf32("break"));
    keywords0.push_back(ToUtf32("byte"));
    keywords0.push_back(ToUtf32("case"));
    keywords0.push_back(ToUtf32("cast"));
    keywords0.push_back(ToUtf32("catch"));
    keywords0.push_back(ToUtf32("char"));
    keywords0.push_back(ToUtf32("class"));
    keywords0.push_back(ToUtf32("const"));
    keywords0.push_back(ToUtf32("continue"));
    keywords0.push_back(ToUtf32("default"));
    keywords0.push_back(ToUtf32("delegate"));
    keywords0.push_back(ToUtf32("do"));
    keywords0.push_back(ToUtf32("double"));
    keywords0.push_back(ToUtf32("else"));
    keywords0.push_back(ToUtf32("enum"));
    keywords0.push_back(ToUtf32("false"));
    keywords0.push_back(ToUtf32("finally"));
    keywords0.push_back(ToUtf32("float"));
    keywords0.push_back(ToUtf32("for"));
    keywords0.push_back(ToUtf32("foreach"));
    keywords0.push_back(ToUtf32("goto"));
    keywords0.push_back(ToUtf32("if"));
    keywords0.push_back(ToUtf32("in"));
    keywords0.push_back(ToUtf32("inline"));
    keywords0.push_back(ToUtf32("int"));
    keywords0.push_back(ToUtf32("interface"));
    keywords0.push_back(ToUtf32("internal"));
    keywords0.push_back(ToUtf32("is"));
    keywords0.push_back(ToUtf32("lock"));
    keywords0.push_back(ToUtf32("long"));
    keywords0.push_back(ToUtf32("namespace"));
    keywords0.push_back(ToUtf32("new"));
    keywords0.push_back(ToUtf32("null"));
    keywords0.push_back(ToUtf32("object"));
    keywords0.push_back(ToUtf32("operator"));
    keywords0.push_back(ToUtf32("override"));
    keywords0.push_back(ToUtf32("private"));
    keywords0.push_back(ToUtf32("protected"));
    keywords0.push_back(ToUtf32("public"));
    keywords0.push_back(ToUtf32("ref"));
    keywords0.push_back(ToUtf32("return"));
    keywords0.push_back(ToUtf32("sbyte"));
    keywords0.push_back(ToUtf32("short"));
    keywords0.push_back(ToUtf32("static"));
    keywords0.push_back(ToUtf32("string"));
    keywords0.push_back(ToUtf32("switch"));
    keywords0.push_back(ToUtf32("this"));
    keywords0.push_back(ToUtf32("throw"));
    keywords0.push_back(ToUtf32("true"));
    keywords0.push_back(ToUtf32("try"));
    keywords0.push_back(ToUtf32("uint"));
    keywords0.push_back(ToUtf32("ulong"));
    keywords0.push_back(ToUtf32("ushort"));
    keywords0.push_back(ToUtf32("using"));
    keywords0.push_back(ToUtf32("virtual"));
    keywords0.push_back(ToUtf32("void"));
    keywords0.push_back(ToUtf32("while"));
}

void KeywordGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cminor.parsing.stdlib"));
    if (!grammar0)
    {
        grammar0 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar0);
}

void KeywordGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("identifier"), this, ToUtf32("cminor.parsing.stdlib.identifier")));
    AddRule(new cminor::parsing::Rule(ToUtf32("Keyword"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::KeywordListParser(ToUtf32("identifier"), keywords0)));
}

} } // namespace cminor.parser
