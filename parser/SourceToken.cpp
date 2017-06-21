#include "SourceToken.hpp"
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
#include <cminor/parser/Keyword.hpp>
#include <cminor/parser/Identifier.hpp>
#include <cminor/parser/Literal.hpp>

namespace cminor { namespace parser {

using namespace cminor::parsing;
using namespace cminor::util;
using namespace cminor::unicode;

SourceTokenGrammar* SourceTokenGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

SourceTokenGrammar* SourceTokenGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    SourceTokenGrammar* grammar(new SourceTokenGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

SourceTokenGrammar::SourceTokenGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("SourceTokenGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.parser")), parsingDomain_)
{
    SetOwner(0);
}

void SourceTokenGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, SourceTokenFormatter* formatter)
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
    stack.push(std::unique_ptr<cminor::parsing::Object>(new ValueObject<SourceTokenFormatter*>(formatter)));
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

class SourceTokenGrammar::SourceTokensRule : public cminor::parsing::Rule
{
public:
    SourceTokensRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("SourceTokenFormatter*"), ToUtf32("formatter")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> formatter_value = std::move(stack.top());
        context->formatter = *static_cast<cminor::parsing::ValueObject<SourceTokenFormatter*>*>(formatter_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokensRule>(this, &SourceTokensRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokensRule>(this, &SourceTokensRule::A1Action));
        cminor::parsing::NonterminalParser* sourceTokenNonterminalParser = GetNonterminal(ToUtf32("SourceToken"));
        sourceTokenNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<SourceTokensRule>(this, &SourceTokensRule::PreSourceToken));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->formatter->BeginFormat();
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->formatter->EndFormat();
    }
    void PreSourceToken(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<SourceTokenFormatter*>(context->formatter)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): formatter() {}
        SourceTokenFormatter* formatter;
    };
};

class SourceTokenGrammar::SourceTokenRule : public cminor::parsing::Rule
{
public:
    SourceTokenRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("SourceTokenFormatter*"), ToUtf32("formatter")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> formatter_value = std::move(stack.top());
        context->formatter = *static_cast<cminor::parsing::ValueObject<SourceTokenFormatter*>*>(formatter_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokenRule>(this, &SourceTokenRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokenRule>(this, &SourceTokenRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokenRule>(this, &SourceTokenRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokenRule>(this, &SourceTokenRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction(ToUtf32("A4"));
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokenRule>(this, &SourceTokenRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction(ToUtf32("A5"));
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokenRule>(this, &SourceTokenRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction(ToUtf32("A6"));
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokenRule>(this, &SourceTokenRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction(ToUtf32("A7"));
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokenRule>(this, &SourceTokenRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction(ToUtf32("A8"));
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokenRule>(this, &SourceTokenRule::A8Action));
        cminor::parsing::ActionParser* a9ActionParser = GetAction(ToUtf32("A9"));
        a9ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokenRule>(this, &SourceTokenRule::A9Action));
        cminor::parsing::NonterminalParser* qualifiedIdNonterminalParser = GetNonterminal(ToUtf32("QualifiedId"));
        qualifiedIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SourceTokenRule>(this, &SourceTokenRule::PostQualifiedId));
        cminor::parsing::NonterminalParser* floatingLiteralNonterminalParser = GetNonterminal(ToUtf32("FloatingLiteral"));
        floatingLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SourceTokenRule>(this, &SourceTokenRule::PostFloatingLiteral));
        cminor::parsing::NonterminalParser* integerLiteralNonterminalParser = GetNonterminal(ToUtf32("IntegerLiteral"));
        integerLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SourceTokenRule>(this, &SourceTokenRule::PostIntegerLiteral));
        cminor::parsing::NonterminalParser* charLiteralNonterminalParser = GetNonterminal(ToUtf32("CharLiteral"));
        charLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SourceTokenRule>(this, &SourceTokenRule::PostCharLiteral));
        cminor::parsing::NonterminalParser* stringLiteralNonterminalParser = GetNonterminal(ToUtf32("StringLiteral"));
        stringLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SourceTokenRule>(this, &SourceTokenRule::PostStringLiteral));
        cminor::parsing::NonterminalParser* spacesNonterminalParser = GetNonterminal(ToUtf32("Spaces"));
        spacesNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SourceTokenRule>(this, &SourceTokenRule::PostSpaces));
        cminor::parsing::NonterminalParser* otherNonterminalParser = GetNonterminal(ToUtf32("Other"));
        otherNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SourceTokenRule>(this, &SourceTokenRule::PostOther));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->formatter->Keyword(std::string(matchBegin, matchEnd));
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->formatter->Identifier(std::string(matchBegin, matchEnd));
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->formatter->Number(std::string(matchBegin, matchEnd));
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->formatter->Char(std::string(matchBegin, matchEnd));
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->formatter->String(std::string(matchBegin, matchEnd));
    }
    void A5Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->formatter->Spaces(context->fromSpaces);
    }
    void A6Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->formatter->Comment(std::string(matchBegin, matchEnd));
    }
    void A7Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->formatter->Comment(std::string(matchBegin, matchEnd));
    }
    void A8Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->formatter->NewLine(std::string(matchBegin, matchEnd));
    }
    void A9Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->formatter->Other(context->fromOther);
    }
    void PostQualifiedId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromQualifiedId_value = std::move(stack.top());
            context->fromQualifiedId = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromQualifiedId_value.get());
            stack.pop();
        }
    }
    void PostFloatingLiteral(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFloatingLiteral_value = std::move(stack.top());
            context->fromFloatingLiteral = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromFloatingLiteral_value.get());
            stack.pop();
        }
    }
    void PostIntegerLiteral(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIntegerLiteral_value = std::move(stack.top());
            context->fromIntegerLiteral = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromIntegerLiteral_value.get());
            stack.pop();
        }
    }
    void PostCharLiteral(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCharLiteral_value = std::move(stack.top());
            context->fromCharLiteral = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromCharLiteral_value.get());
            stack.pop();
        }
    }
    void PostStringLiteral(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromStringLiteral_value = std::move(stack.top());
            context->fromStringLiteral = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromStringLiteral_value.get());
            stack.pop();
        }
    }
    void PostSpaces(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpaces_value = std::move(stack.top());
            context->fromSpaces = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromSpaces_value.get());
            stack.pop();
        }
    }
    void PostOther(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromOther_value = std::move(stack.top());
            context->fromOther = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromOther_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): formatter(), fromQualifiedId(), fromFloatingLiteral(), fromIntegerLiteral(), fromCharLiteral(), fromStringLiteral(), fromSpaces(), fromOther() {}
        SourceTokenFormatter* formatter;
        IdentifierNode* fromQualifiedId;
        Node* fromFloatingLiteral;
        Node* fromIntegerLiteral;
        Node* fromCharLiteral;
        Node* fromStringLiteral;
        std::string fromSpaces;
        std::string fromOther;
    };
};

class SourceTokenGrammar::SpacesRule : public cminor::parsing::Rule
{
public:
    SpacesRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("std::string"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::string>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SpacesRule>(this, &SpacesRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = std::string(matchBegin, matchEnd);
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        std::string value;
    };
};

class SourceTokenGrammar::OtherRule : public cminor::parsing::Rule
{
public:
    OtherRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("std::string"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::string>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OtherRule>(this, &OtherRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = std::string(matchBegin, matchEnd);
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        std::string value;
    };
};

void SourceTokenGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cminor.parser.IdentifierGrammar"));
    if (!grammar0)
    {
        grammar0 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar(ToUtf32("cminor.parser.LiteralGrammar"));
    if (!grammar1)
    {
        grammar1 = cminor::parser::LiteralGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar(ToUtf32("cminor.parser.KeywordGrammar"));
    if (!grammar2)
    {
        grammar2 = cminor::parser::KeywordGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar(ToUtf32("cminor.parsing.stdlib"));
    if (!grammar3)
    {
        grammar3 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar3);
}

void SourceTokenGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Keyword"), this, ToUtf32("KeywordGrammar.Keyword")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("block_comment"), this, ToUtf32("cminor.parsing.stdlib.block_comment")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("StringLiteral"), this, ToUtf32("LiteralGrammar.StringLiteral")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("QualifiedId"), this, ToUtf32("IdentifierGrammar.QualifiedId")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("IntegerLiteral"), this, ToUtf32("LiteralGrammar.IntegerLiteral")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("FloatingLiteral"), this, ToUtf32("LiteralGrammar.FloatingLiteral")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("newline"), this, ToUtf32("cminor.parsing.stdlib.newline")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("CharLiteral"), this, ToUtf32("LiteralGrammar.CharLiteral")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("line_comment"), this, ToUtf32("cminor.parsing.stdlib.line_comment")));
    AddRule(new SourceTokensRule(ToUtf32("SourceTokens"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::EmptyParser()),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("SourceToken"), ToUtf32("SourceToken"), 1))),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::EmptyParser()))));
    AddRule(new SourceTokenRule(ToUtf32("SourceToken"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::AlternativeParser(
                                    new cminor::parsing::AlternativeParser(
                                        new cminor::parsing::AlternativeParser(
                                            new cminor::parsing::ActionParser(ToUtf32("A0"),
                                                new cminor::parsing::NonterminalParser(ToUtf32("Keyword"), ToUtf32("Keyword"), 0)),
                                            new cminor::parsing::ActionParser(ToUtf32("A1"),
                                                new cminor::parsing::NonterminalParser(ToUtf32("QualifiedId"), ToUtf32("QualifiedId"), 0))),
                                        new cminor::parsing::ActionParser(ToUtf32("A2"),
                                            new cminor::parsing::AlternativeParser(
                                                new cminor::parsing::NonterminalParser(ToUtf32("FloatingLiteral"), ToUtf32("FloatingLiteral"), 0),
                                                new cminor::parsing::NonterminalParser(ToUtf32("IntegerLiteral"), ToUtf32("IntegerLiteral"), 0)))),
                                    new cminor::parsing::ActionParser(ToUtf32("A3"),
                                        new cminor::parsing::NonterminalParser(ToUtf32("CharLiteral"), ToUtf32("CharLiteral"), 0))),
                                new cminor::parsing::ActionParser(ToUtf32("A4"),
                                    new cminor::parsing::NonterminalParser(ToUtf32("StringLiteral"), ToUtf32("StringLiteral"), 0))),
                            new cminor::parsing::ActionParser(ToUtf32("A5"),
                                new cminor::parsing::NonterminalParser(ToUtf32("Spaces"), ToUtf32("Spaces"), 0))),
                        new cminor::parsing::ActionParser(ToUtf32("A6"),
                            new cminor::parsing::NonterminalParser(ToUtf32("line_comment"), ToUtf32("line_comment"), 0))),
                    new cminor::parsing::ActionParser(ToUtf32("A7"),
                        new cminor::parsing::NonterminalParser(ToUtf32("block_comment"), ToUtf32("block_comment"), 0))),
                new cminor::parsing::ActionParser(ToUtf32("A8"),
                    new cminor::parsing::NonterminalParser(ToUtf32("newline"), ToUtf32("newline"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A9"),
                new cminor::parsing::NonterminalParser(ToUtf32("Other"), ToUtf32("Other"), 0)))));
    AddRule(new SpacesRule(ToUtf32("Spaces"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::PositiveParser(
                new cminor::parsing::CharSetParser(ToUtf32("\t "))))));
    AddRule(new OtherRule(ToUtf32("Other"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::CharSetParser(ToUtf32("\r\n"), true))));
}

} } // namespace cminor.parser
