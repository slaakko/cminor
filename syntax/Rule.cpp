#include "Rule.hpp"
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
#include <cminor/syntax/Composite.hpp>

namespace cminor { namespace syntax {

using namespace cminor::parsing;
using namespace cminor::util;
using namespace cminor::unicode;

RuleGrammar* RuleGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

RuleGrammar* RuleGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    RuleGrammar* grammar(new RuleGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

RuleGrammar::RuleGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("RuleGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.syntax")), parsingDomain_)
{
    SetOwner(0);
}

cminor::parsing::Rule* RuleGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, cminor::parsing::Scope* enclosingScope)
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
    cminor::parsing::Rule* result = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Rule*>*>(value.get());
    stack.pop();
    return result;
}

class RuleGrammar::RuleRule : public cminor::parsing::Rule
{
public:
    RuleRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::parsing::Scope*"), ToUtf32("enclosingScope")));
        SetValueTypeName(ToUtf32("cminor::parsing::Rule*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Rule*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RuleRule>(this, &RuleRule::A0Action));
        cminor::parsing::NonterminalParser* ruleHeaderNonterminalParser = GetNonterminal(ToUtf32("RuleHeader"));
        ruleHeaderNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<RuleRule>(this, &RuleRule::PreRuleHeader));
        ruleHeaderNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RuleRule>(this, &RuleRule::PostRuleHeader));
        cminor::parsing::NonterminalParser* ruleBodyNonterminalParser = GetNonterminal(ToUtf32("RuleBody"));
        ruleBodyNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<RuleRule>(this, &RuleRule::PreRuleBody));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromRuleHeader;
    }
    void PreRuleHeader(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context->enclosingScope)));
    }
    void PostRuleHeader(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromRuleHeader_value = std::move(stack.top());
            context->fromRuleHeader = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Rule*>*>(fromRuleHeader_value.get());
            stack.pop();
        }
    }
    void PreRuleBody(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Rule*>(context->value)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): enclosingScope(), value(), fromRuleHeader() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Rule* value;
        cminor::parsing::Rule* fromRuleHeader;
    };
};

class RuleGrammar::RuleHeaderRule : public cminor::parsing::Rule
{
public:
    RuleHeaderRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::parsing::Scope*"), ToUtf32("enclosingScope")));
        SetValueTypeName(ToUtf32("cminor::parsing::Rule*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Rule*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RuleHeaderRule>(this, &RuleHeaderRule::A0Action));
        cminor::parsing::NonterminalParser* nameNonterminalParser = GetNonterminal(ToUtf32("name"));
        nameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RuleHeaderRule>(this, &RuleHeaderRule::Postname));
        cminor::parsing::NonterminalParser* signatureNonterminalParser = GetNonterminal(ToUtf32("Signature"));
        signatureNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<RuleHeaderRule>(this, &RuleHeaderRule::PreSignature));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Rule(context->fromname, context->enclosingScope);
        context->value->SetSpan(span);
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
    void PreSignature(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Rule*>(context->value)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): enclosingScope(), value(), fromname() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Rule* value;
        std::u32string fromname;
    };
};

class RuleGrammar::RuleBodyRule : public cminor::parsing::Rule
{
public:
    RuleBodyRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::parsing::Rule*"), ToUtf32("rule")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> rule_value = std::move(stack.top());
        context->rule = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Rule*>*>(rule_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RuleBodyRule>(this, &RuleBodyRule::A0Action));
        cminor::parsing::NonterminalParser* definitionNonterminalParser = GetNonterminal(ToUtf32("definition"));
        definitionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<RuleBodyRule>(this, &RuleBodyRule::Predefinition));
        definitionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RuleBodyRule>(this, &RuleBodyRule::Postdefinition));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->rule->SetDefinition(context->fromdefinition);
    }
    void Predefinition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context->rule->GetScope())));
    }
    void Postdefinition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromdefinition_value = std::move(stack.top());
            context->fromdefinition = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromdefinition_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): rule(), fromdefinition() {}
        cminor::parsing::Rule* rule;
        cminor::parsing::Parser* fromdefinition;
    };
};

void RuleGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cminor.syntax.CompositeGrammar"));
    if (!grammar0)
    {
        grammar0 = cminor::syntax::CompositeGrammar::Create(pd);
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

void RuleGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Alternative"), this, ToUtf32("CompositeGrammar.Alternative")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Signature"), this, ToUtf32("ElementGrammar.Signature")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Identifier"), this, ToUtf32("ElementGrammar.Identifier")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("string"), this, ToUtf32("cminor.parsing.stdlib.string")));
    AddRule(new RuleRule(ToUtf32("Rule"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::NonterminalParser(ToUtf32("RuleHeader"), ToUtf32("RuleHeader"), 1)),
                new cminor::parsing::StringParser(ToUtf32("::="))),
            new cminor::parsing::NonterminalParser(ToUtf32("RuleBody"), ToUtf32("RuleBody"), 1))));
    AddRule(new RuleHeaderRule(ToUtf32("RuleHeader"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("name"), ToUtf32("Identifier"), 0)),
            new cminor::parsing::NonterminalParser(ToUtf32("Signature"), ToUtf32("Signature"), 1))));
    AddRule(new RuleBodyRule(ToUtf32("RuleBody"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("definition"), ToUtf32("Alternative"), 1)),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
}

} } // namespace cminor.syntax
