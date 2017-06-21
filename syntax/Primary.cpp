#include "Primary.hpp"
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
#include <cminor/code/Expression.hpp>
#include <cminor/code/Statement.hpp>
#include <cminor/syntax/Primitive.hpp>
#include <cminor/syntax/Composite.hpp>
#include <cminor/syntax/Element.hpp>
#include <cminor/parsing/Nonterminal.hpp>
#include <cminor/codedom/Statement.hpp>

namespace cminor { namespace syntax {

using namespace cminor::code;
using namespace cminor::parsing;
using namespace cminor::util;
using namespace cminor::unicode;

PrimaryGrammar* PrimaryGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

PrimaryGrammar* PrimaryGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    PrimaryGrammar* grammar(new PrimaryGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

PrimaryGrammar::PrimaryGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("PrimaryGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.syntax")), parsingDomain_)
{
    SetOwner(0);
}

cminor::parsing::Parser* PrimaryGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, cminor::parsing::Scope* enclosingScope)
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
    cminor::parsing::Parser* result = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(value.get());
    stack.pop();
    return result;
}

class PrimaryGrammar::PrimaryRule : public cminor::parsing::Rule
{
public:
    PrimaryRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::parsing::Scope*"), ToUtf32("enclosingScope")));
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction(ToUtf32("A4"));
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction(ToUtf32("A5"));
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A5Action));
        cminor::parsing::NonterminalParser* ruleCallNonterminalParser = GetNonterminal(ToUtf32("RuleCall"));
        ruleCallNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostRuleCall));
        cminor::parsing::NonterminalParser* primitiveNonterminalParser = GetNonterminal(ToUtf32("Primitive"));
        primitiveNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostPrimitive));
        cminor::parsing::NonterminalParser* groupingNonterminalParser = GetNonterminal(ToUtf32("Grouping"));
        groupingNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PrimaryRule>(this, &PrimaryRule::PreGrouping));
        groupingNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostGrouping));
        cminor::parsing::NonterminalParser* tokenNonterminalParser = GetNonterminal(ToUtf32("Token"));
        tokenNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PrimaryRule>(this, &PrimaryRule::PreToken));
        tokenNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostToken));
        cminor::parsing::NonterminalParser* expectationNonterminalParser = GetNonterminal(ToUtf32("Expectation"));
        expectationNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PrimaryRule>(this, &PrimaryRule::PreExpectation));
        expectationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostExpectation));
        cminor::parsing::NonterminalParser* actionNonterminalParser = GetNonterminal(ToUtf32("Action"));
        actionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PrimaryRule>(this, &PrimaryRule::PreAction));
        actionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostAction));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromRuleCall;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromPrimitive;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromGrouping;
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromToken;
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromExpectation;
    }
    void A5Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromAction;
    }
    void PostRuleCall(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromRuleCall_value = std::move(stack.top());
            context->fromRuleCall = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromRuleCall_value.get());
            stack.pop();
        }
    }
    void PostPrimitive(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromPrimitive_value = std::move(stack.top());
            context->fromPrimitive = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromPrimitive_value.get());
            stack.pop();
        }
    }
    void PreGrouping(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context->enclosingScope)));
    }
    void PostGrouping(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromGrouping_value = std::move(stack.top());
            context->fromGrouping = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromGrouping_value.get());
            stack.pop();
        }
    }
    void PreToken(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context->enclosingScope)));
    }
    void PostToken(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromToken_value = std::move(stack.top());
            context->fromToken = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromToken_value.get());
            stack.pop();
        }
    }
    void PreExpectation(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
    }
    void PostExpectation(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpectation_value = std::move(stack.top());
            context->fromExpectation = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromExpectation_value.get());
            stack.pop();
        }
    }
    void PreAction(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
    }
    void PostAction(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAction_value = std::move(stack.top());
            context->fromAction = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromAction_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): enclosingScope(), value(), fromRuleCall(), fromPrimitive(), fromGrouping(), fromToken(), fromExpectation(), fromAction() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Parser* value;
        cminor::parsing::Parser* fromRuleCall;
        cminor::parsing::Parser* fromPrimitive;
        cminor::parsing::Parser* fromGrouping;
        cminor::parsing::Parser* fromToken;
        cminor::parsing::Parser* fromExpectation;
        cminor::parsing::Parser* fromAction;
    };
};

class PrimaryGrammar::RuleCallRule : public cminor::parsing::Rule
{
public:
    RuleCallRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RuleCallRule>(this, &RuleCallRule::A0Action));
        cminor::parsing::NonterminalParser* ntWithArgsNonterminalParser = GetNonterminal(ToUtf32("ntWithArgs"));
        ntWithArgsNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RuleCallRule>(this, &RuleCallRule::PostntWithArgs));
        cminor::parsing::NonterminalParser* expressionListNonterminalParser = GetNonterminal(ToUtf32("ExpressionList"));
        expressionListNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RuleCallRule>(this, &RuleCallRule::PostExpressionList));
        cminor::parsing::NonterminalParser* ntNonterminalParser = GetNonterminal(ToUtf32("nt"));
        ntNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RuleCallRule>(this, &RuleCallRule::Postnt));
        cminor::parsing::NonterminalParser* aliasNonterminalParser = GetNonterminal(ToUtf32("Alias"));
        aliasNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RuleCallRule>(this, &RuleCallRule::PostAlias));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::u32string ruleName;
        if (!context->fromntWithArgs.empty())
        {
            ruleName = context->fromntWithArgs;
        }
        else
        {
            ruleName = context->fromnt;
        }
        std::u32string alias = context->fromAlias;
        std::u32string name = !alias.empty() ? alias : ruleName;
        NonterminalParser * parser(new NonterminalParser(name, ruleName));
        parser->SetSpan(span);
        if (!context->fromExpressionList.empty())
        {
            parser->SetArguments(context->fromExpressionList);
        }
        context->value = parser;
    }
    void PostntWithArgs(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromntWithArgs_value = std::move(stack.top());
            context->fromntWithArgs = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromntWithArgs_value.get());
            stack.pop();
        }
    }
    void PostExpressionList(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpressionList_value = std::move(stack.top());
            context->fromExpressionList = *static_cast<cminor::parsing::ValueObject<std::vector<cminor::codedom::CppObject*>>*>(fromExpressionList_value.get());
            stack.pop();
        }
    }
    void Postnt(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromnt_value = std::move(stack.top());
            context->fromnt = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromnt_value.get());
            stack.pop();
        }
    }
    void PostAlias(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAlias_value = std::move(stack.top());
            context->fromAlias = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromAlias_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromntWithArgs(), fromExpressionList(), fromnt(), fromAlias() {}
        cminor::parsing::Parser* value;
        std::u32string fromntWithArgs;
        std::vector<cminor::codedom::CppObject*> fromExpressionList;
        std::u32string fromnt;
        std::u32string fromAlias;
    };
};

class PrimaryGrammar::NonterminalRule : public cminor::parsing::Rule
{
public:
    NonterminalRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("std::u32string"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::u32string>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NonterminalRule>(this, &NonterminalRule::A0Action));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("Identifier"));
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NonterminalRule>(this, &NonterminalRule::PostIdentifier));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromIdentifier;
    }
    void PostIdentifier(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIdentifier_value = std::move(stack.top());
            context->fromIdentifier = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromIdentifier() {}
        std::u32string value;
        std::u32string fromIdentifier;
    };
};

class PrimaryGrammar::AliasRule : public cminor::parsing::Rule
{
public:
    AliasRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("std::u32string"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::u32string>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AliasRule>(this, &AliasRule::A0Action));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("Identifier"));
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AliasRule>(this, &AliasRule::PostIdentifier));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromIdentifier;
    }
    void PostIdentifier(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIdentifier_value = std::move(stack.top());
            context->fromIdentifier = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromIdentifier() {}
        std::u32string value;
        std::u32string fromIdentifier;
    };
};

class PrimaryGrammar::GroupingRule : public cminor::parsing::Rule
{
public:
    GroupingRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::parsing::Scope*"), ToUtf32("enclosingScope")));
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<GroupingRule>(this, &GroupingRule::A0Action));
        cminor::parsing::NonterminalParser* alternativeNonterminalParser = GetNonterminal(ToUtf32("Alternative"));
        alternativeNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<GroupingRule>(this, &GroupingRule::PreAlternative));
        alternativeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<GroupingRule>(this, &GroupingRule::PostAlternative));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromAlternative;
    }
    void PreAlternative(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context->enclosingScope)));
    }
    void PostAlternative(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAlternative_value = std::move(stack.top());
            context->fromAlternative = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromAlternative_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): enclosingScope(), value(), fromAlternative() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Parser* value;
        cminor::parsing::Parser* fromAlternative;
    };
};

class PrimaryGrammar::TokenRule : public cminor::parsing::Rule
{
public:
    TokenRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::parsing::Scope*"), ToUtf32("enclosingScope")));
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TokenRule>(this, &TokenRule::A0Action));
        cminor::parsing::NonterminalParser* alternativeNonterminalParser = GetNonterminal(ToUtf32("Alternative"));
        alternativeNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<TokenRule>(this, &TokenRule::PreAlternative));
        alternativeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TokenRule>(this, &TokenRule::PostAlternative));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new TokenParser(context->fromAlternative);
    }
    void PreAlternative(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context->enclosingScope)));
    }
    void PostAlternative(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAlternative_value = std::move(stack.top());
            context->fromAlternative = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromAlternative_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): enclosingScope(), value(), fromAlternative() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Parser* value;
        cminor::parsing::Parser* fromAlternative;
    };
};

class PrimaryGrammar::ExpectationRule : public cminor::parsing::Rule
{
public:
    ExpectationRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::parsing::Parser*"), ToUtf32("child")));
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> child_value = std::move(stack.top());
        context->child = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(child_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExpectationRule>(this, &ExpectationRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ExpectationParser(context->child);
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): child(), value() {}
        cminor::parsing::Parser* child;
        cminor::parsing::Parser* value;
    };
};

class PrimaryGrammar::ActionRule : public cminor::parsing::Rule
{
public:
    ActionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::parsing::Parser*"), ToUtf32("child")));
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> child_value = std::move(stack.top());
        context->child = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(child_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ActionRule>(this, &ActionRule::A0Action));
        cminor::parsing::NonterminalParser* actionNonterminalParser = GetNonterminal(ToUtf32("action"));
        actionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ActionRule>(this, &ActionRule::Postaction));
        cminor::parsing::NonterminalParser* failureActionNonterminalParser = GetNonterminal(ToUtf32("failureAction"));
        failureActionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ActionRule>(this, &ActionRule::PostfailureAction));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ActionParser(U"", context->fromaction, context->fromfailureAction, context->child);
    }
    void Postaction(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromaction_value = std::move(stack.top());
            context->fromaction = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CompoundStatement*>*>(fromaction_value.get());
            stack.pop();
        }
    }
    void PostfailureAction(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromfailureAction_value = std::move(stack.top());
            context->fromfailureAction = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CompoundStatement*>*>(fromfailureAction_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): child(), value(), fromaction(), fromfailureAction() {}
        cminor::parsing::Parser* child;
        cminor::parsing::Parser* value;
        cminor::codedom::CompoundStatement* fromaction;
        cminor::codedom::CompoundStatement* fromfailureAction;
    };
};

void PrimaryGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cminor.syntax.PrimitiveGrammar"));
    if (!grammar0)
    {
        grammar0 = cminor::syntax::PrimitiveGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar(ToUtf32("cminor.syntax.ElementGrammar"));
    if (!grammar1)
    {
        grammar1 = cminor::syntax::ElementGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar(ToUtf32("cminor.syntax.CompositeGrammar"));
    if (!grammar2)
    {
        grammar2 = cminor::syntax::CompositeGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar(ToUtf32("cminor.code.ExpressionGrammar"));
    if (!grammar3)
    {
        grammar3 = cminor::code::ExpressionGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
    cminor::parsing::Grammar* grammar4 = pd->GetGrammar(ToUtf32("cminor.code.StatementGrammar"));
    if (!grammar4)
    {
        grammar4 = cminor::code::StatementGrammar::Create(pd);
    }
    AddGrammarReference(grammar4);
}

void PrimaryGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Primitive"), this, ToUtf32("PrimitiveGrammar.Primitive")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Identifier"), this, ToUtf32("ElementGrammar.Identifier")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Alternative"), this, ToUtf32("CompositeGrammar.Alternative")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("ExpressionList"), this, ToUtf32("cminor.code.ExpressionGrammar.ExpressionList")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("CompoundStatement"), this, ToUtf32("cminor.code.StatementGrammar.CompoundStatement")));
    AddRule(new PrimaryRule(ToUtf32("Primary"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::ActionParser(ToUtf32("A0"),
                                new cminor::parsing::NonterminalParser(ToUtf32("RuleCall"), ToUtf32("RuleCall"), 0)),
                            new cminor::parsing::ActionParser(ToUtf32("A1"),
                                new cminor::parsing::NonterminalParser(ToUtf32("Primitive"), ToUtf32("Primitive"), 0))),
                        new cminor::parsing::ActionParser(ToUtf32("A2"),
                            new cminor::parsing::NonterminalParser(ToUtf32("Grouping"), ToUtf32("Grouping"), 1))),
                    new cminor::parsing::ActionParser(ToUtf32("A3"),
                        new cminor::parsing::NonterminalParser(ToUtf32("Token"), ToUtf32("Token"), 1))),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::ActionParser(ToUtf32("A4"),
                        new cminor::parsing::NonterminalParser(ToUtf32("Expectation"), ToUtf32("Expectation"), 1)))),
            new cminor::parsing::OptionalParser(
                new cminor::parsing::ActionParser(ToUtf32("A5"),
                    new cminor::parsing::NonterminalParser(ToUtf32("Action"), ToUtf32("Action"), 1))))));
    AddRule(new RuleCallRule(ToUtf32("RuleCall"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::TokenParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::NonterminalParser(ToUtf32("ntWithArgs"), ToUtf32("Nonterminal"), 0),
                                    new cminor::parsing::CharParser('('))),
                            new cminor::parsing::NonterminalParser(ToUtf32("ExpressionList"), ToUtf32("ExpressionList"), 0)),
                        new cminor::parsing::CharParser(')')),
                    new cminor::parsing::NonterminalParser(ToUtf32("nt"), ToUtf32("Nonterminal"), 0)),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("Alias"), ToUtf32("Alias"), 0))))));
    AddRule(new NonterminalRule(ToUtf32("Nonterminal"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::NonterminalParser(ToUtf32("Identifier"), ToUtf32("Identifier"), 0))));
    AddRule(new AliasRule(ToUtf32("Alias"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::CharParser(':'),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("Identifier"), ToUtf32("Identifier"), 0))))));
    AddRule(new GroupingRule(ToUtf32("Grouping"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('('),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("Alternative"), ToUtf32("Alternative"), 1))),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))))));
    AddRule(new TokenRule(ToUtf32("Token"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser(ToUtf32("token")),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('('))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("Alternative"), ToUtf32("Alternative"), 1))),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))))));
    AddRule(new ExpectationRule(ToUtf32("Expectation"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::CharParser('!'))));
    AddRule(new ActionRule(ToUtf32("Action"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::NonterminalParser(ToUtf32("action"), ToUtf32("CompoundStatement"), 0),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::CharParser('/'),
                        new cminor::parsing::NonterminalParser(ToUtf32("failureAction"), ToUtf32("CompoundStatement"), 0)))))));
}

} } // namespace cminor.syntax
