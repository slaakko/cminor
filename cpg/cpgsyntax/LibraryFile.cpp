#include "LibraryFile.hpp"
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
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/Namespace.hpp>

namespace cpg { namespace syntax {

using namespace cminor::parsing;

LibraryFileGrammar* LibraryFileGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

LibraryFileGrammar* LibraryFileGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    LibraryFileGrammar* grammar(new LibraryFileGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

LibraryFileGrammar::LibraryFileGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("LibraryFileGrammar", parsingDomain_->GetNamespaceScope("Cm.Parsing.Syntax"), parsingDomain_)
{
    SetOwner(0);
}

void LibraryFileGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, cminor::parsing::ParsingDomain* parsingDomain)
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
    stack.push(std::unique_ptr<cminor::parsing::Object>(new ValueObject<cminor::parsing::ParsingDomain*>(parsingDomain)));
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
}

class LibraryFileGrammar::LibraryFileRule : public cminor::parsing::Rule
{
public:
    LibraryFileRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::ParsingDomain*", "parsingDomain"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> parsingDomain_value = std::move(stack.top());
        context.parsingDomain = *static_cast<cminor::parsing::ValueObject<cminor::parsing::ParsingDomain*>*>(parsingDomain_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::NonterminalParser* namespaceContentNonterminalParser = GetNonterminal("NamespaceContent");
        namespaceContentNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<LibraryFileRule>(this, &LibraryFileRule::PreNamespaceContent));
    }
    void PreNamespaceContent(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::ParsingDomain*>(context.parsingDomain)));
    }
private:
    struct Context
    {
        Context(): parsingDomain() {}
        cminor::parsing::ParsingDomain* parsingDomain;
    };
    std::stack<Context> contextStack;
    Context context;
};

class LibraryFileGrammar::NamespaceContentRule : public cminor::parsing::Rule
{
public:
    NamespaceContentRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::ParsingDomain*", "parsingDomain"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> parsingDomain_value = std::move(stack.top());
        context.parsingDomain = *static_cast<cminor::parsing::ValueObject<cminor::parsing::ParsingDomain*>*>(parsingDomain_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NamespaceContentRule>(this, &NamespaceContentRule::A0Action));
        cminor::parsing::NonterminalParser* grammarNonterminalParser = GetNonterminal("Grammar");
        grammarNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<NamespaceContentRule>(this, &NamespaceContentRule::PreGrammar));
        grammarNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NamespaceContentRule>(this, &NamespaceContentRule::PostGrammar));
        cminor::parsing::NonterminalParser* namespaceNonterminalParser = GetNonterminal("Namespace");
        namespaceNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<NamespaceContentRule>(this, &NamespaceContentRule::PreNamespace));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.parsingDomain->AddGrammar(context.fromGrammar);
    }
    void PreGrammar(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context.parsingDomain->CurrentNamespace()->GetScope())));
    }
    void PostGrammar(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromGrammar_value = std::move(stack.top());
            context.fromGrammar = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Grammar*>*>(fromGrammar_value.get());
            stack.pop();
        }
    }
    void PreNamespace(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::ParsingDomain*>(context.parsingDomain)));
    }
private:
    struct Context
    {
        Context(): parsingDomain(), fromGrammar() {}
        cminor::parsing::ParsingDomain* parsingDomain;
        cminor::parsing::Grammar* fromGrammar;
    };
    std::stack<Context> contextStack;
    Context context;
};

class LibraryFileGrammar::NamespaceRule : public cminor::parsing::Rule
{
public:
    NamespaceRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::ParsingDomain*", "parsingDomain"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> parsingDomain_value = std::move(stack.top());
        context.parsingDomain = *static_cast<cminor::parsing::ValueObject<cminor::parsing::ParsingDomain*>*>(parsingDomain_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NamespaceRule>(this, &NamespaceRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NamespaceRule>(this, &NamespaceRule::A1Action));
        cminor::parsing::NonterminalParser* nsNonterminalParser = GetNonterminal("ns");
        nsNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NamespaceRule>(this, &NamespaceRule::Postns));
        cminor::parsing::NonterminalParser* namespaceContentNonterminalParser = GetNonterminal("NamespaceContent");
        namespaceContentNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<NamespaceRule>(this, &NamespaceRule::PreNamespaceContent));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.parsingDomain->BeginNamespace(context.fromns);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.parsingDomain->EndNamespace();
    }
    void Postns(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromns_value = std::move(stack.top());
            context.fromns = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromns_value.get());
            stack.pop();
        }
    }
    void PreNamespaceContent(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::ParsingDomain*>(context.parsingDomain)));
    }
private:
    struct Context
    {
        Context(): parsingDomain(), fromns() {}
        cminor::parsing::ParsingDomain* parsingDomain;
        std::string fromns;
    };
    std::stack<Context> contextStack;
    Context context;
};

class LibraryFileGrammar::GrammarRule : public cminor::parsing::Rule
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
        cminor::parsing::NonterminalParser* grammarNameNonterminalParser = GetNonterminal("grammarName");
        grammarNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<GrammarRule>(this, &GrammarRule::PostgrammarName));
        cminor::parsing::NonterminalParser* grammarContentNonterminalParser = GetNonterminal("GrammarContent");
        grammarContentNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<GrammarRule>(this, &GrammarRule::PreGrammarContent));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new Grammar(context.fromgrammarName, context.enclosingScope);
        context.value->SetNs(context.enclosingScope->Ns());
    }
    void PostgrammarName(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromgrammarName_value = std::move(stack.top());
            context.fromgrammarName = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromgrammarName_value.get());
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
        Context(): enclosingScope(), value(), fromgrammarName() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Grammar* value;
        std::string fromgrammarName;
    };
    std::stack<Context> contextStack;
    Context context;
};

class LibraryFileGrammar::GrammarContentRule : public cminor::parsing::Rule
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
        cminor::parsing::NonterminalParser* ruleNonterminalParser = GetNonterminal("Rule");
        ruleNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<GrammarContentRule>(this, &GrammarContentRule::PreRule));
        ruleNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<GrammarContentRule>(this, &GrammarContentRule::PostRule));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.grammar->AddRule(context.fromRule);
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

class LibraryFileGrammar::RuleRule : public cminor::parsing::Rule
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
        cminor::parsing::NonterminalParser* ruleNameNonterminalParser = GetNonterminal("ruleName");
        ruleNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RuleRule>(this, &RuleRule::PostruleName));
        cminor::parsing::NonterminalParser* signatureNonterminalParser = GetNonterminal("Signature");
        signatureNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<RuleRule>(this, &RuleRule::PreSignature));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new cminor::parsing::Rule(context.fromruleName, context.enclosingScope);
    }
    void PostruleName(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromruleName_value = std::move(stack.top());
            context.fromruleName = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromruleName_value.get());
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
        Context(): enclosingScope(), value(), fromruleName() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Rule* value;
        std::string fromruleName;
    };
    std::stack<Context> contextStack;
    Context context;
};

void LibraryFileGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("Cm.Parsing.stdlib");
    if (!grammar0)
    {
        grammar0 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar("Cm.Parsing.Syntax.ElementGrammar");
    if (!grammar1)
    {
        grammar1 = cpg::syntax::ElementGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
}

void LibraryFileGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("spaces_and_comments", this, "Cm.Parsing.stdlib.spaces_and_comments"));
    AddRuleLink(new cminor::parsing::RuleLink("identifier", this, "Cm.Parsing.stdlib.identifier"));
    AddRuleLink(new cminor::parsing::RuleLink("qualified_id", this, "Cm.Parsing.stdlib.qualified_id"));
    AddRuleLink(new cminor::parsing::RuleLink("Identifier", this, "ElementGrammar.Identifier"));
    AddRuleLink(new cminor::parsing::RuleLink("Signature", this, "ElementGrammar.Signature"));
    AddRule(new LibraryFileRule("LibraryFile", GetScope(),
        new cminor::parsing::NonterminalParser("NamespaceContent", "NamespaceContent", 1)));
    AddRule(new NamespaceContentRule("NamespaceContent", GetScope(),
        new cminor::parsing::KleeneStarParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::NonterminalParser("Grammar", "Grammar", 1)),
                new cminor::parsing::NonterminalParser("Namespace", "Namespace", 1)))));
    AddRule(new NamespaceRule("Namespace", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser("namespace"),
                        new cminor::parsing::ActionParser("A0",
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("ns", "qualified_id", 0)))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('{'))),
                new cminor::parsing::NonterminalParser("NamespaceContent", "NamespaceContent", 1)),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser('}'))))));
    AddRule(new GrammarRule("Grammar", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser("grammar"),
                        new cminor::parsing::ActionParser("A0",
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("grammarName", "Identifier", 0)))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('{'))),
                new cminor::parsing::NonterminalParser("GrammarContent", "GrammarContent", 1)),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::CharParser('}')))));
    AddRule(new GrammarContentRule("GrammarContent", GetScope(),
        new cminor::parsing::KleeneStarParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("Rule", "Rule", 1)))));
    AddRule(new RuleRule("Rule", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::NonterminalParser("ruleName", "Identifier", 0)),
                new cminor::parsing::NonterminalParser("Signature", "Signature", 1)),
            new cminor::parsing::CharParser(';'))));
    SetSkipRuleName("spaces_and_comments");
}

} } // namespace cpg.syntax
