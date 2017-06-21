#include "Grammar.hpp"
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
#include <cminor/syntax/Element.hpp>
#include <cminor/syntax/Rule.hpp>

namespace cminor { namespace syntax {

using namespace cminor::parsing;
using namespace cminor::util;
using namespace cminor::unicode;

GrammarGrammar* GrammarGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

GrammarGrammar* GrammarGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    GrammarGrammar* grammar(new GrammarGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

GrammarGrammar::GrammarGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("GrammarGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.syntax")), parsingDomain_)
{
    SetOwner(0);
}

cminor::parsing::Grammar* GrammarGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, cminor::parsing::Scope* enclosingScope)
{
    cminor::parsing::Scanner scanner(start, end, fileName, fileIndex, SkipRule());
    std::unique_ptr<cminor::parsing::XmlLog> xmlLog;
    if (Log())
    {
        xmlLog.reset(new cminor::parsing::XmlLog(*Log(), MaxLogLineLength()));
        scanner.SetLog(xmlLog.get());
        xmlLog->WriteBeginRule("parse");
    }
    cminor::parsing::ObjectStack stack;
    std::unique_ptr<cminor::parsing::ParsingData> parsingData(new cminor::parsing::ParsingData(GetParsingDomain()->GetNumRules()));
    scanner.SetParsingData(parsingData.get());
    stack.push(std::unique_ptr<cminor::parsing::Object>(new ValueObject<cminor::parsing::Scope*>(enclosingScope)));
    cminor::parsing::Match match = cminor::parsing::Grammar::Parse(scanner, stack, parsingData.get());
    cminor::parsing::Span stop = scanner.GetSpan();
    if (Log())
    {
        xmlLog->WriteEndRule("parse");
    }
    if (!match.Hit() || stop.Start() != int(end - start))
    {
        if (StartRule())
        {
            throw cminor::parsing::ExpectationFailure(StartRule()->Info(), fileName, stop, start, end);
        }
        else
        {
            throw cminor::parsing::ParsingException("grammar '" + ToUtf8(Name()) + "' has no start rule", fileName, scanner.GetSpan(), start, end);
        }
    }
    std::unique_ptr<cminor::parsing::Object> value = std::move(stack.top());
    cminor::parsing::Grammar* result = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Grammar*>*>(value.get());
    stack.pop();
    return result;
}

class GrammarGrammar::GrammarRule : public cminor::parsing::Rule
{
public:
    GrammarRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::parsing::Scope*"), ToUtf32("enclosingScope")));
        SetValueTypeName(ToUtf32("cminor::parsing::Grammar*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> enclosingScope_value = std::move(stack.top());
        context->enclosingScope = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Scope*>*>(enclosingScope_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Grammar*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<GrammarRule>(this, &GrammarRule::A0Action));
        cminor::parsing::NonterminalParser* nameNonterminalParser = GetNonterminal(ToUtf32("name"));
        nameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<GrammarRule>(this, &GrammarRule::Postname));
        cminor::parsing::NonterminalParser* grammarContentNonterminalParser = GetNonterminal(ToUtf32("GrammarContent"));
        grammarContentNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<GrammarRule>(this, &GrammarRule::PreGrammarContent));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Grammar(context->fromname, context->enclosingScope);
        context->value->SetSpan(span);
        context->value->SetNs(context->enclosingScope->Ns());
    }
    void Postname(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromname_value = std::move(stack.top());
            context->fromname = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromname_value.get());
            stack.pop();
        }
    }
    void PreGrammarContent(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Grammar*>(context->value)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): enclosingScope(), value(), fromname() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Grammar* value;
        std::u32string fromname;
    };
};

class GrammarGrammar::GrammarContentRule : public cminor::parsing::Rule
{
public:
    GrammarContentRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::parsing::Grammar*"), ToUtf32("grammar")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> grammar_value = std::move(stack.top());
        context->grammar = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Grammar*>*>(grammar_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<GrammarContentRule>(this, &GrammarContentRule::A0Action));
        cminor::parsing::NonterminalParser* startClauseNonterminalParser = GetNonterminal(ToUtf32("StartClause"));
        startClauseNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<GrammarContentRule>(this, &GrammarContentRule::PreStartClause));
        cminor::parsing::NonterminalParser* skipClauseNonterminalParser = GetNonterminal(ToUtf32("SkipClause"));
        skipClauseNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<GrammarContentRule>(this, &GrammarContentRule::PreSkipClause));
        cminor::parsing::NonterminalParser* ruleLinkNonterminalParser = GetNonterminal(ToUtf32("RuleLink"));
        ruleLinkNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<GrammarContentRule>(this, &GrammarContentRule::PreRuleLink));
        cminor::parsing::NonterminalParser* ruleNonterminalParser = GetNonterminal(ToUtf32("Rule"));
        ruleNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<GrammarContentRule>(this, &GrammarContentRule::PreRule));
        ruleNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<GrammarContentRule>(this, &GrammarContentRule::PostRule));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->grammar->AddRule(context->fromRule);
    }
    void PreStartClause(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Grammar*>(context->grammar)));
    }
    void PreSkipClause(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Grammar*>(context->grammar)));
    }
    void PreRuleLink(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Grammar*>(context->grammar)));
    }
    void PreRule(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context->grammar->GetScope())));
    }
    void PostRule(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromRule_value = std::move(stack.top());
            context->fromRule = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Rule*>*>(fromRule_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): grammar(), fromRule() {}
        cminor::parsing::Grammar* grammar;
        cminor::parsing::Rule* fromRule;
    };
};

class GrammarGrammar::StartClauseRule : public cminor::parsing::Rule
{
public:
    StartClauseRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::parsing::Grammar*"), ToUtf32("grammar")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> grammar_value = std::move(stack.top());
        context->grammar = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Grammar*>*>(grammar_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StartClauseRule>(this, &StartClauseRule::A0Action));
        cminor::parsing::NonterminalParser* strtNonterminalParser = GetNonterminal(ToUtf32("strt"));
        strtNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StartClauseRule>(this, &StartClauseRule::Poststrt));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->grammar->SetStartRuleName(context->fromstrt);
    }
    void Poststrt(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromstrt_value = std::move(stack.top());
            context->fromstrt = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromstrt_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): grammar(), fromstrt() {}
        cminor::parsing::Grammar* grammar;
        std::u32string fromstrt;
    };
};

class GrammarGrammar::SkipClauseRule : public cminor::parsing::Rule
{
public:
    SkipClauseRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::parsing::Grammar*"), ToUtf32("grammar")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> grammar_value = std::move(stack.top());
        context->grammar = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Grammar*>*>(grammar_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SkipClauseRule>(this, &SkipClauseRule::A0Action));
        cminor::parsing::NonterminalParser* skpNonterminalParser = GetNonterminal(ToUtf32("skp"));
        skpNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SkipClauseRule>(this, &SkipClauseRule::Postskp));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->grammar->SetSkipRuleName(context->fromskp);
    }
    void Postskp(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromskp_value = std::move(stack.top());
            context->fromskp = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromskp_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): grammar(), fromskp() {}
        cminor::parsing::Grammar* grammar;
        std::u32string fromskp;
    };
};

void GrammarGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cminor.syntax.RuleGrammar"));
    if (!grammar0)
    {
        grammar0 = cminor::syntax::RuleGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar(ToUtf32("cminor.syntax.ElementGrammar"));
    if (!grammar1)
    {
        grammar1 = cminor::syntax::ElementGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar(ToUtf32("cminor.parsing.stdlib"));
    if (!grammar2)
    {
        grammar2 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar2);
}

void GrammarGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Identifier"), this, ToUtf32("ElementGrammar.Identifier")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("QualifiedId"), this, ToUtf32("ElementGrammar.QualifiedId")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("char"), this, ToUtf32("cminor.parsing.stdlib.char")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Signature"), this, ToUtf32("ElementGrammar.Signature")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("RuleLink"), this, ToUtf32("ElementGrammar.RuleLink")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Rule"), this, ToUtf32("RuleGrammar.Rule")));
    AddRule(new GrammarRule(ToUtf32("Grammar"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser(ToUtf32("grammar")),
                        new cminor::parsing::ActionParser(ToUtf32("A0"),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("name"), ToUtf32("Identifier"), 0)))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('{'))),
                new cminor::parsing::NonterminalParser(ToUtf32("GrammarContent"), ToUtf32("GrammarContent"), 1)),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::CharParser('}')))));
    AddRule(new GrammarContentRule(ToUtf32("GrammarContent"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::KleeneStarParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("StartClause"), ToUtf32("StartClause"), 1),
                        new cminor::parsing::NonterminalParser(ToUtf32("SkipClause"), ToUtf32("SkipClause"), 1)),
                    new cminor::parsing::NonterminalParser(ToUtf32("RuleLink"), ToUtf32("RuleLink"), 1)),
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::NonterminalParser(ToUtf32("Rule"), ToUtf32("Rule"), 1))))));
    AddRule(new StartClauseRule(ToUtf32("StartClause"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser(ToUtf32("start")),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("strt"), ToUtf32("Identifier"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::CharParser(';')))));
    AddRule(new SkipClauseRule(ToUtf32("SkipClause"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser(ToUtf32("skip")),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("skp"), ToUtf32("QualifiedId"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::CharParser(';')))));
}

} } // namespace cminor.syntax
