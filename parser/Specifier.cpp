#include "Specifier.hpp"
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

SpecifierGrammar* SpecifierGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

SpecifierGrammar* SpecifierGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    SpecifierGrammar* grammar(new SpecifierGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

SpecifierGrammar::SpecifierGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("SpecifierGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

Specifiers SpecifierGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName)
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
    cminor::parsing::Match match = cminor::parsing::Grammar::Parse(scanner, stack, parsingData.get());
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
    Specifiers result = *static_cast<cminor::parsing::ValueObject<Specifiers>*>(value.get());
    stack.pop();
    return result;
}

class SpecifierGrammar::SpecifiersRule : public cminor::parsing::Rule
{
public:
    SpecifiersRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("Specifiers");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Specifiers>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SpecifiersRule>(this, &SpecifiersRule::A0Action));
        cminor::parsing::NonterminalParser* specifierNonterminalParser = GetNonterminal("Specifier");
        specifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SpecifiersRule>(this, &SpecifiersRule::PostSpecifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->value | context->fromSpecifier;
    }
    void PostSpecifier(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpecifier_value = std::move(stack.top());
            context->fromSpecifier = *static_cast<cminor::parsing::ValueObject<Specifiers>*>(fromSpecifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromSpecifier() {}
        Specifiers value;
        Specifiers fromSpecifier;
    };
};

class SpecifierGrammar::SpecifierRule : public cminor::parsing::Rule
{
public:
    SpecifierRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("Specifiers");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Specifiers>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction("A6");
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction("A7");
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction("A8");
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A8Action));
        cminor::parsing::ActionParser* a9ActionParser = GetAction("A9");
        a9ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A9Action));
        cminor::parsing::ActionParser* a10ActionParser = GetAction("A10");
        a10ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A10Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Specifiers::public_;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Specifiers::protected_;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Specifiers::private_;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Specifiers::internal_;
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Specifiers::static_;
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Specifiers::virtual_;
    }
    void A6Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Specifiers::override_;
    }
    void A7Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Specifiers::abstract_;
    }
    void A8Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Specifiers::inline_;
    }
    void A9Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Specifiers::external_;
    }
    void A10Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Specifiers::new_;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        Specifiers value;
    };
};

void SpecifierGrammar::GetReferencedGrammars()
{
}

void SpecifierGrammar::CreateRules()
{
    AddRule(new SpecifiersRule("Specifiers", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::KleeneStarParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("Specifier", "Specifier", 0)))));
    AddRule(new SpecifierRule("Specifier", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::AlternativeParser(
                                    new cminor::parsing::AlternativeParser(
                                        new cminor::parsing::AlternativeParser(
                                            new cminor::parsing::AlternativeParser(
                                                new cminor::parsing::ActionParser("A0",
                                                    new cminor::parsing::KeywordParser("public")),
                                                new cminor::parsing::ActionParser("A1",
                                                    new cminor::parsing::KeywordParser("protected"))),
                                            new cminor::parsing::ActionParser("A2",
                                                new cminor::parsing::KeywordParser("private"))),
                                        new cminor::parsing::ActionParser("A3",
                                            new cminor::parsing::KeywordParser("internal"))),
                                    new cminor::parsing::ActionParser("A4",
                                        new cminor::parsing::KeywordParser("static"))),
                                new cminor::parsing::ActionParser("A5",
                                    new cminor::parsing::KeywordParser("virtual"))),
                            new cminor::parsing::ActionParser("A6",
                                new cminor::parsing::KeywordParser("override"))),
                        new cminor::parsing::ActionParser("A7",
                            new cminor::parsing::KeywordParser("abstract"))),
                    new cminor::parsing::ActionParser("A8",
                        new cminor::parsing::KeywordParser("inline"))),
                new cminor::parsing::ActionParser("A9",
                    new cminor::parsing::KeywordParser("extern"))),
            new cminor::parsing::ActionParser("A10",
                new cminor::parsing::KeywordParser("new")))));
}

} } // namespace cminor.parser
