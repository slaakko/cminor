#include "Primary.hpp"
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>
#include <cminor/cpg/cpgcpp/Expression.hpp>
#include <cminor/cpg/cpgcpp/Statement.hpp>
#include <cminor/cpg/cpgsyntax/Primitive.hpp>
#include <cminor/cpg/cpgsyntax/Composite.hpp>
#include <cminor/cpg/cpgsyntax/Element.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pom/Statement.hpp>

namespace cpg { namespace syntax {

using namespace cpg::cpp;
using namespace cminor::parsing;

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

PrimaryGrammar::PrimaryGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("PrimaryGrammar", parsingDomain_->GetNamespaceScope("cpg.syntax"), parsingDomain_)
{
    SetOwner(0);
}

cminor::parsing::Parser* PrimaryGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, cminor::parsing::Scope* enclosingScope)
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
    stack.push(std::unique_ptr<cminor::parsing::Object>(new ValueObject<cminor::parsing::Scope*>(enclosingScope)));
    cminor::parsing::Match match = cminor::parsing::Grammar::Parse(scanner, stack);
    cminor::parsing::Span stop = scanner.GetSpan();
    if (Log())
    {
        xmlLog->WriteEndRule("parse");
    }
    if (!match.Hit() || !CC() && stop.Start() != int(end - start))
    {
        if (StartRule())
        {
            throw cminor::parsing::ExpectationFailure(StartRule()->Info(), fileName, stop, start, end);
        }
        else
        {
            throw cminor::parsing::ParsingException("grammar '" + Name() + "' has no start rule", fileName, scanner.GetSpan(), start, end);
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
    PrimaryRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Scope*", "enclosingScope"));
        SetValueTypeName("cminor::parsing::Parser*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> enclosingScope_value = std::move(stack.top());
        context.enclosingScope = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Scope*>*>(enclosingScope_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction("A6");
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A6Action));
        cminor::parsing::NonterminalParser* ruleCallNonterminalParser = GetNonterminal("RuleCall");
        ruleCallNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostRuleCall));
        cminor::parsing::NonterminalParser* primitiveNonterminalParser = GetNonterminal("Primitive");
        primitiveNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostPrimitive));
        cminor::parsing::NonterminalParser* groupingNonterminalParser = GetNonterminal("Grouping");
        groupingNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PrimaryRule>(this, &PrimaryRule::PreGrouping));
        groupingNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostGrouping));
        cminor::parsing::NonterminalParser* tokenNonterminalParser = GetNonterminal("Token");
        tokenNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PrimaryRule>(this, &PrimaryRule::PreToken));
        tokenNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostToken));
        cminor::parsing::NonterminalParser* expectationNonterminalParser = GetNonterminal("Expectation");
        expectationNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PrimaryRule>(this, &PrimaryRule::PreExpectation));
        expectationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostExpectation));
        cminor::parsing::NonterminalParser* actionNonterminalParser = GetNonterminal("Action");
        actionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PrimaryRule>(this, &PrimaryRule::PreAction));
        actionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostAction));
        cminor::parsing::NonterminalParser* cCOptNonterminalParser = GetNonterminal("CCOpt");
        cCOptNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PrimaryRule>(this, &PrimaryRule::PreCCOpt));
        cCOptNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostCCOpt));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromRuleCall;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromPrimitive;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromGrouping;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromToken;
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromExpectation;
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromAction;
    }
    void A6Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromCCOpt;
    }
    void PostRuleCall(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromRuleCall_value = std::move(stack.top());
            context.fromRuleCall = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromRuleCall_value.get());
            stack.pop();
        }
    }
    void PostPrimitive(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromPrimitive_value = std::move(stack.top());
            context.fromPrimitive = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromPrimitive_value.get());
            stack.pop();
        }
    }
    void PreGrouping(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context.enclosingScope)));
    }
    void PostGrouping(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromGrouping_value = std::move(stack.top());
            context.fromGrouping = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromGrouping_value.get());
            stack.pop();
        }
    }
    void PreToken(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context.enclosingScope)));
    }
    void PostToken(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromToken_value = std::move(stack.top());
            context.fromToken = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromToken_value.get());
            stack.pop();
        }
    }
    void PreExpectation(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context.value)));
    }
    void PostExpectation(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpectation_value = std::move(stack.top());
            context.fromExpectation = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromExpectation_value.get());
            stack.pop();
        }
    }
    void PreAction(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context.value)));
    }
    void PostAction(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAction_value = std::move(stack.top());
            context.fromAction = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromAction_value.get());
            stack.pop();
        }
    }
    void PreCCOpt(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context.value)));
    }
    void PostCCOpt(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCCOpt_value = std::move(stack.top());
            context.fromCCOpt = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromCCOpt_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): enclosingScope(), value(), fromRuleCall(), fromPrimitive(), fromGrouping(), fromToken(), fromExpectation(), fromAction(), fromCCOpt() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Parser* value;
        cminor::parsing::Parser* fromRuleCall;
        cminor::parsing::Parser* fromPrimitive;
        cminor::parsing::Parser* fromGrouping;
        cminor::parsing::Parser* fromToken;
        cminor::parsing::Parser* fromExpectation;
        cminor::parsing::Parser* fromAction;
        cminor::parsing::Parser* fromCCOpt;
    };
    std::stack<Context> contextStack;
    Context context;
};

class PrimaryGrammar::RuleCallRule : public cminor::parsing::Rule
{
public:
    RuleCallRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::parsing::Parser*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RuleCallRule>(this, &RuleCallRule::A0Action));
        cminor::parsing::NonterminalParser* ntWithArgsNonterminalParser = GetNonterminal("ntWithArgs");
        ntWithArgsNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RuleCallRule>(this, &RuleCallRule::PostntWithArgs));
        cminor::parsing::NonterminalParser* expressionListNonterminalParser = GetNonterminal("ExpressionList");
        expressionListNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RuleCallRule>(this, &RuleCallRule::PostExpressionList));
        cminor::parsing::NonterminalParser* ntNonterminalParser = GetNonterminal("nt");
        ntNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RuleCallRule>(this, &RuleCallRule::Postnt));
        cminor::parsing::NonterminalParser* aliasNonterminalParser = GetNonterminal("Alias");
        aliasNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RuleCallRule>(this, &RuleCallRule::PostAlias));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        std::string ruleName;
        if (!context.fromntWithArgs.empty())
        {
            ruleName = context.fromntWithArgs;
        }
        else
        {
            ruleName = context.fromnt;
        }
        std::string alias = context.fromAlias;
        std::string name = !alias.empty() ? alias : ruleName;
        NonterminalParser * parser(new NonterminalParser(name, ruleName));
        parser->SetSpan(span);
        if (!context.fromExpressionList.empty())
        {
            parser->SetArguments(context.fromExpressionList);
        }
        context.value = parser;
    }
    void PostntWithArgs(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromntWithArgs_value = std::move(stack.top());
            context.fromntWithArgs = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromntWithArgs_value.get());
            stack.pop();
        }
    }
    void PostExpressionList(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpressionList_value = std::move(stack.top());
            context.fromExpressionList = *static_cast<cminor::parsing::ValueObject<std::vector<cminor::pom::CppObject*>>*>(fromExpressionList_value.get());
            stack.pop();
        }
    }
    void Postnt(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromnt_value = std::move(stack.top());
            context.fromnt = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromnt_value.get());
            stack.pop();
        }
    }
    void PostAlias(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAlias_value = std::move(stack.top());
            context.fromAlias = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromAlias_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromntWithArgs(), fromExpressionList(), fromnt(), fromAlias() {}
        cminor::parsing::Parser* value;
        std::string fromntWithArgs;
        std::vector<cminor::pom::CppObject*> fromExpressionList;
        std::string fromnt;
        std::string fromAlias;
    };
    std::stack<Context> contextStack;
    Context context;
};

class PrimaryGrammar::NonterminalRule : public cminor::parsing::Rule
{
public:
    NonterminalRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("std::string");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::string>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NonterminalRule>(this, &NonterminalRule::A0Action));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NonterminalRule>(this, &NonterminalRule::PostIdentifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromIdentifier;
    }
    void PostIdentifier(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIdentifier_value = std::move(stack.top());
            context.fromIdentifier = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromIdentifier() {}
        std::string value;
        std::string fromIdentifier;
    };
    std::stack<Context> contextStack;
    Context context;
};

class PrimaryGrammar::AliasRule : public cminor::parsing::Rule
{
public:
    AliasRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("std::string");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::string>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AliasRule>(this, &AliasRule::A0Action));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AliasRule>(this, &AliasRule::PostIdentifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromIdentifier;
    }
    void PostIdentifier(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIdentifier_value = std::move(stack.top());
            context.fromIdentifier = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromIdentifier() {}
        std::string value;
        std::string fromIdentifier;
    };
    std::stack<Context> contextStack;
    Context context;
};

class PrimaryGrammar::GroupingRule : public cminor::parsing::Rule
{
public:
    GroupingRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Scope*", "enclosingScope"));
        SetValueTypeName("cminor::parsing::Parser*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> enclosingScope_value = std::move(stack.top());
        context.enclosingScope = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Scope*>*>(enclosingScope_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<GroupingRule>(this, &GroupingRule::A0Action));
        cminor::parsing::NonterminalParser* alternativeNonterminalParser = GetNonterminal("Alternative");
        alternativeNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<GroupingRule>(this, &GroupingRule::PreAlternative));
        alternativeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<GroupingRule>(this, &GroupingRule::PostAlternative));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromAlternative;
    }
    void PreAlternative(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context.enclosingScope)));
    }
    void PostAlternative(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAlternative_value = std::move(stack.top());
            context.fromAlternative = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromAlternative_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): enclosingScope(), value(), fromAlternative() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Parser* value;
        cminor::parsing::Parser* fromAlternative;
    };
    std::stack<Context> contextStack;
    Context context;
};

class PrimaryGrammar::TokenRule : public cminor::parsing::Rule
{
public:
    TokenRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Scope*", "enclosingScope"));
        SetValueTypeName("cminor::parsing::Parser*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> enclosingScope_value = std::move(stack.top());
        context.enclosingScope = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Scope*>*>(enclosingScope_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TokenRule>(this, &TokenRule::A0Action));
        cminor::parsing::NonterminalParser* alternativeNonterminalParser = GetNonterminal("Alternative");
        alternativeNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<TokenRule>(this, &TokenRule::PreAlternative));
        alternativeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TokenRule>(this, &TokenRule::PostAlternative));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new TokenParser(context.fromAlternative);
    }
    void PreAlternative(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context.enclosingScope)));
    }
    void PostAlternative(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAlternative_value = std::move(stack.top());
            context.fromAlternative = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromAlternative_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): enclosingScope(), value(), fromAlternative() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Parser* value;
        cminor::parsing::Parser* fromAlternative;
    };
    std::stack<Context> contextStack;
    Context context;
};

class PrimaryGrammar::ExpectationRule : public cminor::parsing::Rule
{
public:
    ExpectationRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Parser*", "child"));
        SetValueTypeName("cminor::parsing::Parser*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> child_value = std::move(stack.top());
        context.child = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(child_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExpectationRule>(this, &ExpectationRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ExpectationParser(context.child);
    }
private:
    struct Context
    {
        Context(): child(), value() {}
        cminor::parsing::Parser* child;
        cminor::parsing::Parser* value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class PrimaryGrammar::ActionRule : public cminor::parsing::Rule
{
public:
    ActionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Parser*", "child"));
        SetValueTypeName("cminor::parsing::Parser*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> child_value = std::move(stack.top());
        context.child = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(child_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ActionRule>(this, &ActionRule::A0Action));
        cminor::parsing::NonterminalParser* actionNonterminalParser = GetNonterminal("action");
        actionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ActionRule>(this, &ActionRule::Postaction));
        cminor::parsing::NonterminalParser* failureActionNonterminalParser = GetNonterminal("failureAction");
        failureActionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ActionRule>(this, &ActionRule::PostfailureAction));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ActionParser("", context.fromaction, context.fromfailureAction, context.child);
    }
    void Postaction(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromaction_value = std::move(stack.top());
            context.fromaction = *static_cast<cminor::parsing::ValueObject<cminor::pom::CompoundStatement*>*>(fromaction_value.get());
            stack.pop();
        }
    }
    void PostfailureAction(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromfailureAction_value = std::move(stack.top());
            context.fromfailureAction = *static_cast<cminor::parsing::ValueObject<cminor::pom::CompoundStatement*>*>(fromfailureAction_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): child(), value(), fromaction(), fromfailureAction() {}
        cminor::parsing::Parser* child;
        cminor::parsing::Parser* value;
        cminor::pom::CompoundStatement* fromaction;
        cminor::pom::CompoundStatement* fromfailureAction;
    };
    std::stack<Context> contextStack;
    Context context;
};

class PrimaryGrammar::CCOptRule : public cminor::parsing::Rule
{
public:
    CCOptRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Parser*", "child"));
        SetValueTypeName("cminor::parsing::Parser*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> child_value = std::move(stack.top());
        context.child = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(child_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CCOptRule>(this, &CCOptRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new CCOptParser(context.child);
    }
private:
    struct Context
    {
        Context(): child(), value() {}
        cminor::parsing::Parser* child;
        cminor::parsing::Parser* value;
    };
    std::stack<Context> contextStack;
    Context context;
};

void PrimaryGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("cpg.syntax.PrimitiveGrammar");
    if (!grammar0)
    {
        grammar0 = cpg::syntax::PrimitiveGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar("cpg.cpp.ExpressionGrammar");
    if (!grammar1)
    {
        grammar1 = cpg::cpp::ExpressionGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar("cpg.cpp.StatementGrammar");
    if (!grammar2)
    {
        grammar2 = cpg::cpp::StatementGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar("cpg.syntax.ElementGrammar");
    if (!grammar3)
    {
        grammar3 = cpg::syntax::ElementGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
    cminor::parsing::Grammar* grammar4 = pd->GetGrammar("cpg.syntax.CompositeGrammar");
    if (!grammar4)
    {
        grammar4 = cpg::syntax::CompositeGrammar::Create(pd);
    }
    AddGrammarReference(grammar4);
}

void PrimaryGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("Primitive", this, "PrimitiveGrammar.Primitive"));
    AddRuleLink(new cminor::parsing::RuleLink("CompoundStatement", this, "cpg.cpp.StatementGrammar.CompoundStatement"));
    AddRuleLink(new cminor::parsing::RuleLink("Identifier", this, "ElementGrammar.Identifier"));
    AddRuleLink(new cminor::parsing::RuleLink("ExpressionList", this, "cpg.cpp.ExpressionGrammar.ExpressionList"));
    AddRuleLink(new cminor::parsing::RuleLink("Alternative", this, "CompositeGrammar.Alternative"));
    AddRule(new PrimaryRule("Primary", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::ActionParser("A0",
                                    new cminor::parsing::NonterminalParser("RuleCall", "RuleCall", 0)),
                                new cminor::parsing::ActionParser("A1",
                                    new cminor::parsing::NonterminalParser("Primitive", "Primitive", 0))),
                            new cminor::parsing::ActionParser("A2",
                                new cminor::parsing::NonterminalParser("Grouping", "Grouping", 1))),
                        new cminor::parsing::ActionParser("A3",
                            new cminor::parsing::NonterminalParser("Token", "Token", 1))),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::ActionParser("A4",
                            new cminor::parsing::NonterminalParser("Expectation", "Expectation", 1)))),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::ActionParser("A5",
                        new cminor::parsing::NonterminalParser("Action", "Action", 1)))),
            new cminor::parsing::OptionalParser(
                new cminor::parsing::ActionParser("A6",
                    new cminor::parsing::NonterminalParser("CCOpt", "CCOpt", 1))))));
    AddRule(new RuleCallRule("RuleCall", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::TokenParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::NonterminalParser("ntWithArgs", "Nonterminal", 0),
                                    new cminor::parsing::CharParser('('))),
                            new cminor::parsing::NonterminalParser("ExpressionList", "ExpressionList", 0)),
                        new cminor::parsing::CharParser(')')),
                    new cminor::parsing::NonterminalParser("nt", "Nonterminal", 0)),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::NonterminalParser("Alias", "Alias", 0))))));
    AddRule(new NonterminalRule("Nonterminal", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0))));
    AddRule(new AliasRule("Alias", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::CharParser(':'),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0))))));
    AddRule(new GroupingRule("Grouping", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('('),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("Alternative", "Alternative", 1))),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))))));
    AddRule(new TokenRule("Token", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser("token"),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('('))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("Alternative", "Alternative", 1))),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))))));
    AddRule(new ExpectationRule("Expectation", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::CharParser('!'))));
    AddRule(new ActionRule("Action", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::NonterminalParser("action", "CompoundStatement", 0),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::CharParser('/'),
                        new cminor::parsing::NonterminalParser("failureAction", "CompoundStatement", 0)))))));
    AddRule(new CCOptRule("CCOpt", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::CharParser('$'))));
}

} } // namespace cpg.syntax
