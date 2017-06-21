#include "LibraryFile.hpp"
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
#include <cminor/parsing/Rule.hpp>
#include <cminor/parsing/Namespace.hpp>
#include <cminor/syntax/Element.hpp>

namespace cminor { namespace syntax {

using namespace cminor::parsing;
using namespace cminor::util;
using namespace cminor::unicode;

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

LibraryFileGrammar::LibraryFileGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("LibraryFileGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.syntax")), parsingDomain_)
{
    SetOwner(0);
}

void LibraryFileGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, cminor::parsing::ParsingDomain* parsingDomain)
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
    stack.push(std::unique_ptr<cminor::parsing::Object>(new ValueObject<cminor::parsing::ParsingDomain*>(parsingDomain)));
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
}

class LibraryFileGrammar::LibraryFileRule : public cminor::parsing::Rule
{
public:
    LibraryFileRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::parsing::ParsingDomain*"), ToUtf32("parsingDomain")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> parsingDomain_value = std::move(stack.top());
        context->parsingDomain = *static_cast<cminor::parsing::ValueObject<cminor::parsing::ParsingDomain*>*>(parsingDomain_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::NonterminalParser* namespaceContentNonterminalParser = GetNonterminal(ToUtf32("NamespaceContent"));
        namespaceContentNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<LibraryFileRule>(this, &LibraryFileRule::PreNamespaceContent));
    }
    void PreNamespaceContent(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::ParsingDomain*>(context->parsingDomain)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): parsingDomain() {}
        cminor::parsing::ParsingDomain* parsingDomain;
    };
};

class LibraryFileGrammar::NamespaceContentRule : public cminor::parsing::Rule
{
public:
    NamespaceContentRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::parsing::ParsingDomain*"), ToUtf32("parsingDomain")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> parsingDomain_value = std::move(stack.top());
        context->parsingDomain = *static_cast<cminor::parsing::ValueObject<cminor::parsing::ParsingDomain*>*>(parsingDomain_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NamespaceContentRule>(this, &NamespaceContentRule::A0Action));
        cminor::parsing::NonterminalParser* grammarNonterminalParser = GetNonterminal(ToUtf32("Grammar"));
        grammarNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<NamespaceContentRule>(this, &NamespaceContentRule::PreGrammar));
        grammarNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NamespaceContentRule>(this, &NamespaceContentRule::PostGrammar));
        cminor::parsing::NonterminalParser* namespaceNonterminalParser = GetNonterminal(ToUtf32("Namespace"));
        namespaceNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<NamespaceContentRule>(this, &NamespaceContentRule::PreNamespace));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->parsingDomain->AddGrammar(context->fromGrammar);
    }
    void PreGrammar(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context->parsingDomain->CurrentNamespace()->GetScope())));
    }
    void PostGrammar(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromGrammar_value = std::move(stack.top());
            context->fromGrammar = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Grammar*>*>(fromGrammar_value.get());
            stack.pop();
        }
    }
    void PreNamespace(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::ParsingDomain*>(context->parsingDomain)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): parsingDomain(), fromGrammar() {}
        cminor::parsing::ParsingDomain* parsingDomain;
        cminor::parsing::Grammar* fromGrammar;
    };
};

class LibraryFileGrammar::NamespaceRule : public cminor::parsing::Rule
{
public:
    NamespaceRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::parsing::ParsingDomain*"), ToUtf32("parsingDomain")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> parsingDomain_value = std::move(stack.top());
        context->parsingDomain = *static_cast<cminor::parsing::ValueObject<cminor::parsing::ParsingDomain*>*>(parsingDomain_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NamespaceRule>(this, &NamespaceRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NamespaceRule>(this, &NamespaceRule::A1Action));
        cminor::parsing::NonterminalParser* nsNonterminalParser = GetNonterminal(ToUtf32("ns"));
        nsNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NamespaceRule>(this, &NamespaceRule::Postns));
        cminor::parsing::NonterminalParser* namespaceContentNonterminalParser = GetNonterminal(ToUtf32("NamespaceContent"));
        namespaceContentNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<NamespaceRule>(this, &NamespaceRule::PreNamespaceContent));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->parsingDomain->BeginNamespace(context->fromns);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->parsingDomain->EndNamespace();
    }
    void Postns(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromns_value = std::move(stack.top());
            context->fromns = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromns_value.get());
            stack.pop();
        }
    }
    void PreNamespaceContent(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::ParsingDomain*>(context->parsingDomain)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): parsingDomain(), fromns() {}
        cminor::parsing::ParsingDomain* parsingDomain;
        std::u32string fromns;
    };
};

class LibraryFileGrammar::GrammarRule : public cminor::parsing::Rule
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
        cminor::parsing::NonterminalParser* grammarNameNonterminalParser = GetNonterminal(ToUtf32("grammarName"));
        grammarNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<GrammarRule>(this, &GrammarRule::PostgrammarName));
        cminor::parsing::NonterminalParser* grammarContentNonterminalParser = GetNonterminal(ToUtf32("GrammarContent"));
        grammarContentNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<GrammarRule>(this, &GrammarRule::PreGrammarContent));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Grammar(context->fromgrammarName, context->enclosingScope);
        context->value->SetNs(context->enclosingScope->Ns());
    }
    void PostgrammarName(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromgrammarName_value = std::move(stack.top());
            context->fromgrammarName = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromgrammarName_value.get());
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
        Context(): enclosingScope(), value(), fromgrammarName() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Grammar* value;
        std::u32string fromgrammarName;
    };
};

class LibraryFileGrammar::GrammarContentRule : public cminor::parsing::Rule
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
        cminor::parsing::NonterminalParser* ruleNonterminalParser = GetNonterminal(ToUtf32("Rule"));
        ruleNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<GrammarContentRule>(this, &GrammarContentRule::PreRule));
        ruleNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<GrammarContentRule>(this, &GrammarContentRule::PostRule));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->grammar->AddRule(context->fromRule);
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

class LibraryFileGrammar::RuleRule : public cminor::parsing::Rule
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
        cminor::parsing::NonterminalParser* ruleNameNonterminalParser = GetNonterminal(ToUtf32("ruleName"));
        ruleNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RuleRule>(this, &RuleRule::PostruleName));
        cminor::parsing::NonterminalParser* signatureNonterminalParser = GetNonterminal(ToUtf32("Signature"));
        signatureNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<RuleRule>(this, &RuleRule::PreSignature));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new cminor::parsing::Rule(context->fromruleName, context->enclosingScope);
    }
    void PostruleName(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromruleName_value = std::move(stack.top());
            context->fromruleName = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromruleName_value.get());
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
        Context(): enclosingScope(), value(), fromruleName() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Rule* value;
        std::u32string fromruleName;
    };
};

void LibraryFileGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cminor.parsing.stdlib"));
    if (!grammar0)
    {
        grammar0 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar(ToUtf32("cminor.syntax.ElementGrammar"));
    if (!grammar1)
    {
        grammar1 = cminor::syntax::ElementGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
}

void LibraryFileGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("identifier"), this, ToUtf32("cminor.parsing.stdlib.identifier")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Signature"), this, ToUtf32("ElementGrammar.Signature")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("qualified_id"), this, ToUtf32("cminor.parsing.stdlib.qualified_id")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("spaces_and_comments"), this, ToUtf32("cminor.parsing.stdlib.spaces_and_comments")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Identifier"), this, ToUtf32("ElementGrammar.Identifier")));
    AddRule(new LibraryFileRule(ToUtf32("LibraryFile"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::NonterminalParser(ToUtf32("NamespaceContent"), ToUtf32("NamespaceContent"), 1)));
    AddRule(new NamespaceContentRule(ToUtf32("NamespaceContent"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::KleeneStarParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::NonterminalParser(ToUtf32("Grammar"), ToUtf32("Grammar"), 1)),
                new cminor::parsing::NonterminalParser(ToUtf32("Namespace"), ToUtf32("Namespace"), 1)))));
    AddRule(new NamespaceRule(ToUtf32("Namespace"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser(ToUtf32("namespace")),
                        new cminor::parsing::ActionParser(ToUtf32("A0"),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("ns"), ToUtf32("qualified_id"), 0)))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('{'))),
                new cminor::parsing::NonterminalParser(ToUtf32("NamespaceContent"), ToUtf32("NamespaceContent"), 1)),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser('}'))))));
    AddRule(new GrammarRule(ToUtf32("Grammar"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser(ToUtf32("grammar")),
                        new cminor::parsing::ActionParser(ToUtf32("A0"),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("grammarName"), ToUtf32("Identifier"), 0)))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('{'))),
                new cminor::parsing::NonterminalParser(ToUtf32("GrammarContent"), ToUtf32("GrammarContent"), 1)),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::CharParser('}')))));
    AddRule(new GrammarContentRule(ToUtf32("GrammarContent"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::KleeneStarParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("Rule"), ToUtf32("Rule"), 1)))));
    AddRule(new RuleRule(ToUtf32("Rule"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::NonterminalParser(ToUtf32("ruleName"), ToUtf32("Identifier"), 0)),
                new cminor::parsing::NonterminalParser(ToUtf32("Signature"), ToUtf32("Signature"), 1)),
            new cminor::parsing::CharParser(';'))));
    SetSkipRuleName(ToUtf32("spaces_and_comments"));
}

} } // namespace cminor.syntax
