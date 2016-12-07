#include "Rule.hpp"
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>
#include <cminor/cpg/cpgsyntax/Element.hpp>
#include <cminor/cpg/cpgsyntax/Composite.hpp>

namespace cpg { namespace syntax {

using namespace cminor::parsing;

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

RuleGrammar::RuleGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("RuleGrammar", parsingDomain_->GetNamespaceScope("cpg.syntax"), parsingDomain_)
{
    SetOwner(0);
}

cminor::parsing::Rule* RuleGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, cminor::parsing::Scope* enclosingScope)
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
    cminor::parsing::Rule* result = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Rule*>*>(value.get());
    stack.pop();
    return result;
}

class RuleGrammar::RuleRule : public cminor::parsing::Rule
{
public:
    RuleRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Scope*", "enclosingScope"));
        SetValueTypeName("cminor::parsing::Rule*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Rule*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RuleRule>(this, &RuleRule::A0Action));
        cminor::parsing::NonterminalParser* ruleHeaderNonterminalParser = GetNonterminal("RuleHeader");
        ruleHeaderNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<RuleRule>(this, &RuleRule::PreRuleHeader));
        ruleHeaderNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RuleRule>(this, &RuleRule::PostRuleHeader));
        cminor::parsing::NonterminalParser* ruleBodyNonterminalParser = GetNonterminal("RuleBody");
        ruleBodyNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<RuleRule>(this, &RuleRule::PreRuleBody));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromRuleHeader;
    }
    void PreRuleHeader(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context.enclosingScope)));
    }
    void PostRuleHeader(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromRuleHeader_value = std::move(stack.top());
            context.fromRuleHeader = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Rule*>*>(fromRuleHeader_value.get());
            stack.pop();
        }
    }
    void PreRuleBody(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Rule*>(context.value)));
    }
private:
    struct Context
    {
        Context(): enclosingScope(), value(), fromRuleHeader() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Rule* value;
        cminor::parsing::Rule* fromRuleHeader;
    };
    std::stack<Context> contextStack;
    Context context;
};

class RuleGrammar::RuleHeaderRule : public cminor::parsing::Rule
{
public:
    RuleHeaderRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Scope*", "enclosingScope"));
        SetValueTypeName("cminor::parsing::Rule*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Rule*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RuleHeaderRule>(this, &RuleHeaderRule::A0Action));
        cminor::parsing::NonterminalParser* nameNonterminalParser = GetNonterminal("name");
        nameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RuleHeaderRule>(this, &RuleHeaderRule::Postname));
        cminor::parsing::NonterminalParser* signatureNonterminalParser = GetNonterminal("Signature");
        signatureNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<RuleHeaderRule>(this, &RuleHeaderRule::PreSignature));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new Rule(context.fromname, context.enclosingScope);
        context.value->SetSpan(span);
    }
    void Postname(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromname_value = std::move(stack.top());
            context.fromname = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromname_value.get());
            stack.pop();
        }
    }
    void PreSignature(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Rule*>(context.value)));
    }
private:
    struct Context
    {
        Context(): enclosingScope(), value(), fromname() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Rule* value;
        std::string fromname;
    };
    std::stack<Context> contextStack;
    Context context;
};

class RuleGrammar::RuleBodyRule : public cminor::parsing::Rule
{
public:
    RuleBodyRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Rule*", "rule"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> rule_value = std::move(stack.top());
        context.rule = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Rule*>*>(rule_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RuleBodyRule>(this, &RuleBodyRule::A0Action));
        cminor::parsing::NonterminalParser* definitionNonterminalParser = GetNonterminal("definition");
        definitionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<RuleBodyRule>(this, &RuleBodyRule::Predefinition));
        definitionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RuleBodyRule>(this, &RuleBodyRule::Postdefinition));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.rule->SetDefinition(context.fromdefinition);
    }
    void Predefinition(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context.rule->GetScope())));
    }
    void Postdefinition(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromdefinition_value = std::move(stack.top());
            context.fromdefinition = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromdefinition_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): rule(), fromdefinition() {}
        cminor::parsing::Rule* rule;
        cminor::parsing::Parser* fromdefinition;
    };
    std::stack<Context> contextStack;
    Context context;
};

void RuleGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("cpg.syntax.ElementGrammar");
    if (!grammar0)
    {
        grammar0 = cpg::syntax::ElementGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar("cpg.syntax.CompositeGrammar");
    if (!grammar1)
    {
        grammar1 = cpg::syntax::CompositeGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar("cminor.parsing.stdlib");
    if (!grammar2)
    {
        grammar2 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar2);
}

void RuleGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("Identifier", this, "ElementGrammar.Identifier"));
    AddRuleLink(new cminor::parsing::RuleLink("Signature", this, "ElementGrammar.Signature"));
    AddRuleLink(new cminor::parsing::RuleLink("Alternative", this, "CompositeGrammar.Alternative"));
    AddRuleLink(new cminor::parsing::RuleLink("string", this, "cminor.parsing.stdlib.string"));
    AddRule(new RuleRule("Rule", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::NonterminalParser("RuleHeader", "RuleHeader", 1)),
                new cminor::parsing::StringParser("::=")),
            new cminor::parsing::NonterminalParser("RuleBody", "RuleBody", 1))));
    AddRule(new RuleHeaderRule("RuleHeader", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("name", "Identifier", 0)),
            new cminor::parsing::NonterminalParser("Signature", "Signature", 1))));
    AddRule(new RuleBodyRule("RuleBody", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("definition", "Alternative", 1)),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
}

} } // namespace cpg.syntax
