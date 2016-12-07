#include "Grammar.hpp"
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
#include <cminor/cpg/cpgsyntax/Rule.hpp>

namespace cpg { namespace syntax {

using namespace cminor::parsing;

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

GrammarGrammar::GrammarGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("GrammarGrammar", parsingDomain_->GetNamespaceScope("cpg.syntax"), parsingDomain_)
{
    SetOwner(0);
}

cminor::parsing::Grammar* GrammarGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, cminor::parsing::Scope* enclosingScope)
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
    cminor::parsing::Grammar* result = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Grammar*>*>(value.get());
    stack.pop();
    return result;
}

class GrammarGrammar::GrammarRule : public cminor::parsing::Rule
{
public:
    GrammarRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Scope*", "enclosingScope"));
        SetValueTypeName("cminor::parsing::Grammar*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Grammar*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<GrammarRule>(this, &GrammarRule::A0Action));
        cminor::parsing::NonterminalParser* nameNonterminalParser = GetNonterminal("name");
        nameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<GrammarRule>(this, &GrammarRule::Postname));
        cminor::parsing::NonterminalParser* grammarContentNonterminalParser = GetNonterminal("GrammarContent");
        grammarContentNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<GrammarRule>(this, &GrammarRule::PreGrammarContent));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new Grammar(context.fromname, context.enclosingScope);
        context.value->SetSpan(span);
        context.value->SetNs(context.enclosingScope->Ns());
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
    void PreGrammarContent(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Grammar*>(context.value)));
    }
private:
    struct Context
    {
        Context(): enclosingScope(), value(), fromname() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Grammar* value;
        std::string fromname;
    };
    std::stack<Context> contextStack;
    Context context;
};

class GrammarGrammar::GrammarContentRule : public cminor::parsing::Rule
{
public:
    GrammarContentRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Grammar*", "grammar"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> grammar_value = std::move(stack.top());
        context.grammar = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Grammar*>*>(grammar_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<GrammarContentRule>(this, &GrammarContentRule::A0Action));
        cminor::parsing::NonterminalParser* startClauseNonterminalParser = GetNonterminal("StartClause");
        startClauseNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<GrammarContentRule>(this, &GrammarContentRule::PreStartClause));
        cminor::parsing::NonterminalParser* skipClauseNonterminalParser = GetNonterminal("SkipClause");
        skipClauseNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<GrammarContentRule>(this, &GrammarContentRule::PreSkipClause));
        cminor::parsing::NonterminalParser* cCClauseNonterminalParser = GetNonterminal("CCClause");
        cCClauseNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<GrammarContentRule>(this, &GrammarContentRule::PreCCClause));
        cminor::parsing::NonterminalParser* ruleLinkNonterminalParser = GetNonterminal("RuleLink");
        ruleLinkNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<GrammarContentRule>(this, &GrammarContentRule::PreRuleLink));
        cminor::parsing::NonterminalParser* ruleNonterminalParser = GetNonterminal("Rule");
        ruleNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<GrammarContentRule>(this, &GrammarContentRule::PreRule));
        ruleNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<GrammarContentRule>(this, &GrammarContentRule::PostRule));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.grammar->AddRule(context.fromRule);
    }
    void PreStartClause(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Grammar*>(context.grammar)));
    }
    void PreSkipClause(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Grammar*>(context.grammar)));
    }
    void PreCCClause(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Grammar*>(context.grammar)));
    }
    void PreRuleLink(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Grammar*>(context.grammar)));
    }
    void PreRule(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context.grammar->GetScope())));
    }
    void PostRule(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromRule_value = std::move(stack.top());
            context.fromRule = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Rule*>*>(fromRule_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): grammar(), fromRule() {}
        cminor::parsing::Grammar* grammar;
        cminor::parsing::Rule* fromRule;
    };
    std::stack<Context> contextStack;
    Context context;
};

class GrammarGrammar::StartClauseRule : public cminor::parsing::Rule
{
public:
    StartClauseRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Grammar*", "grammar"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> grammar_value = std::move(stack.top());
        context.grammar = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Grammar*>*>(grammar_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StartClauseRule>(this, &StartClauseRule::A0Action));
        cminor::parsing::NonterminalParser* strtNonterminalParser = GetNonterminal("strt");
        strtNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StartClauseRule>(this, &StartClauseRule::Poststrt));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.grammar->SetStartRuleName(context.fromstrt);
    }
    void Poststrt(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromstrt_value = std::move(stack.top());
            context.fromstrt = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromstrt_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): grammar(), fromstrt() {}
        cminor::parsing::Grammar* grammar;
        std::string fromstrt;
    };
    std::stack<Context> contextStack;
    Context context;
};

class GrammarGrammar::SkipClauseRule : public cminor::parsing::Rule
{
public:
    SkipClauseRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Grammar*", "grammar"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> grammar_value = std::move(stack.top());
        context.grammar = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Grammar*>*>(grammar_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SkipClauseRule>(this, &SkipClauseRule::A0Action));
        cminor::parsing::NonterminalParser* skpNonterminalParser = GetNonterminal("skp");
        skpNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SkipClauseRule>(this, &SkipClauseRule::Postskp));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.grammar->SetSkipRuleName(context.fromskp);
    }
    void Postskp(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromskp_value = std::move(stack.top());
            context.fromskp = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromskp_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): grammar(), fromskp() {}
        cminor::parsing::Grammar* grammar;
        std::string fromskp;
    };
    std::stack<Context> contextStack;
    Context context;
};

class GrammarGrammar::CCClauseRule : public cminor::parsing::Rule
{
public:
    CCClauseRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Grammar*", "grammar"));
        AddLocalVariable(AttrOrVariable("bool", "doSkip"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> grammar_value = std::move(stack.top());
        context.grammar = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Grammar*>*>(grammar_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CCClauseRule>(this, &CCClauseRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CCClauseRule>(this, &CCClauseRule::A1Action));
        cminor::parsing::NonterminalParser* ccRuleNonterminalParser = GetNonterminal("ccRule");
        ccRuleNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CCClauseRule>(this, &CCClauseRule::PostccRule));
        cminor::parsing::NonterminalParser* ccStartNonterminalParser = GetNonterminal("ccStart");
        ccStartNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CCClauseRule>(this, &CCClauseRule::PostccStart));
        cminor::parsing::NonterminalParser* ccEndNonterminalParser = GetNonterminal("ccEnd");
        ccEndNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CCClauseRule>(this, &CCClauseRule::PostccEnd));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.grammar->SetCCRuleData(context.fromccRule, context.fromccStart, context.fromccEnd, context.doSkip);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.doSkip = true;
    }
    void PostccRule(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromccRule_value = std::move(stack.top());
            context.fromccRule = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromccRule_value.get());
            stack.pop();
        }
    }
    void PostccStart(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromccStart_value = std::move(stack.top());
            context.fromccStart = *static_cast<cminor::parsing::ValueObject<char>*>(fromccStart_value.get());
            stack.pop();
        }
    }
    void PostccEnd(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromccEnd_value = std::move(stack.top());
            context.fromccEnd = *static_cast<cminor::parsing::ValueObject<char>*>(fromccEnd_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): grammar(), doSkip(), fromccRule(), fromccStart(), fromccEnd() {}
        cminor::parsing::Grammar* grammar;
        bool doSkip;
        std::string fromccRule;
        char fromccStart;
        char fromccEnd;
    };
    std::stack<Context> contextStack;
    Context context;
};

void GrammarGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("cpg.syntax.ElementGrammar");
    if (!grammar0)
    {
        grammar0 = cpg::syntax::ElementGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar("cpg.syntax.RuleGrammar");
    if (!grammar1)
    {
        grammar1 = cpg::syntax::RuleGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar("cminor.parsing.stdlib");
    if (!grammar2)
    {
        grammar2 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar2);
}

void GrammarGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("Identifier", this, "ElementGrammar.Identifier"));
    AddRuleLink(new cminor::parsing::RuleLink("Rule", this, "RuleGrammar.Rule"));
    AddRuleLink(new cminor::parsing::RuleLink("Signature", this, "ElementGrammar.Signature"));
    AddRuleLink(new cminor::parsing::RuleLink("QualifiedId", this, "ElementGrammar.QualifiedId"));
    AddRuleLink(new cminor::parsing::RuleLink("RuleLink", this, "ElementGrammar.RuleLink"));
    AddRuleLink(new cminor::parsing::RuleLink("char", this, "cminor.parsing.stdlib.char"));
    AddRule(new GrammarRule("Grammar", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser("grammar"),
                        new cminor::parsing::ActionParser("A0",
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("name", "Identifier", 0)))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('{'))),
                new cminor::parsing::NonterminalParser("GrammarContent", "GrammarContent", 1)),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::CharParser('}')))));
    AddRule(new GrammarContentRule("GrammarContent", GetScope(),
        new cminor::parsing::KleeneStarParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::NonterminalParser("StartClause", "StartClause", 1),
                            new cminor::parsing::NonterminalParser("SkipClause", "SkipClause", 1)),
                        new cminor::parsing::NonterminalParser("CCClause", "CCClause", 1)),
                    new cminor::parsing::NonterminalParser("RuleLink", "RuleLink", 1)),
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::NonterminalParser("Rule", "Rule", 1))))));
    AddRule(new StartClauseRule("StartClause", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser("start"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("strt", "Identifier", 0))),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::CharParser(';')))));
    AddRule(new SkipClauseRule("SkipClause", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser("skip"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("skp", "QualifiedId", 0))),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::CharParser(';')))));
    AddRule(new CCClauseRule("CCClause", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::SequenceParser(
                                            new cminor::parsing::SequenceParser(
                                                new cminor::parsing::SequenceParser(
                                                    new cminor::parsing::SequenceParser(
                                                        new cminor::parsing::SequenceParser(
                                                            new cminor::parsing::KeywordParser("cc"),
                                                            new cminor::parsing::ExpectationParser(
                                                                new cminor::parsing::NonterminalParser("ccRule", "Identifier", 0))),
                                                        new cminor::parsing::ExpectationParser(
                                                            new cminor::parsing::CharParser('('))),
                                                    new cminor::parsing::ExpectationParser(
                                                        new cminor::parsing::KeywordParser("start"))),
                                                new cminor::parsing::ExpectationParser(
                                                    new cminor::parsing::CharParser('='))),
                                            new cminor::parsing::ExpectationParser(
                                                new cminor::parsing::NonterminalParser("ccStart", "char", 0))),
                                        new cminor::parsing::ExpectationParser(
                                            new cminor::parsing::CharParser(','))),
                                    new cminor::parsing::ExpectationParser(
                                        new cminor::parsing::KeywordParser("end"))),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::CharParser('='))),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("ccEnd", "char", 0))),
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::CharParser(','),
                                new cminor::parsing::ActionParser("A1",
                                    new cminor::parsing::KeywordParser("skip"))),
                            new cminor::parsing::EmptyParser())),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser(')'))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
}

} } // namespace cpg.syntax
