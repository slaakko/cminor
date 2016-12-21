#include "SourceToken.hpp"
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>
#include <cminor/parser/Keyword.hpp>
#include <cminor/parser/Identifier.hpp>
#include <cminor/parser/Literal.hpp>

namespace cminor { namespace parser {

using namespace cminor::parsing;

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

SourceTokenGrammar::SourceTokenGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("SourceTokenGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

void SourceTokenGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, SourceTokenFormatter* formatter)
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
    stack.push(std::unique_ptr<cminor::parsing::Object>(new ValueObject<SourceTokenFormatter*>(formatter)));
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

class SourceTokenGrammar::SourceTokensRule : public cminor::parsing::Rule
{
public:
    SourceTokensRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("SourceTokenFormatter*", "formatter"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> formatter_value = std::move(stack.top());
        context.formatter = *static_cast<cminor::parsing::ValueObject<SourceTokenFormatter*>*>(formatter_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokensRule>(this, &SourceTokensRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokensRule>(this, &SourceTokensRule::A1Action));
        cminor::parsing::NonterminalParser* sourceTokenNonterminalParser = GetNonterminal("SourceToken");
        sourceTokenNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<SourceTokensRule>(this, &SourceTokensRule::PreSourceToken));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.formatter->BeginFormat();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.formatter->EndFormat();
    }
    void PreSourceToken(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<SourceTokenFormatter*>(context.formatter)));
    }
private:
    struct Context
    {
        Context(): formatter() {}
        SourceTokenFormatter* formatter;
    };
    std::stack<Context> contextStack;
    Context context;
};

class SourceTokenGrammar::SourceTokenRule : public cminor::parsing::Rule
{
public:
    SourceTokenRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("SourceTokenFormatter*", "formatter"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> formatter_value = std::move(stack.top());
        context.formatter = *static_cast<cminor::parsing::ValueObject<SourceTokenFormatter*>*>(formatter_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokenRule>(this, &SourceTokenRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokenRule>(this, &SourceTokenRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokenRule>(this, &SourceTokenRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokenRule>(this, &SourceTokenRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokenRule>(this, &SourceTokenRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokenRule>(this, &SourceTokenRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction("A6");
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokenRule>(this, &SourceTokenRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction("A7");
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokenRule>(this, &SourceTokenRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction("A8");
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokenRule>(this, &SourceTokenRule::A8Action));
        cminor::parsing::ActionParser* a9ActionParser = GetAction("A9");
        a9ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceTokenRule>(this, &SourceTokenRule::A9Action));
        cminor::parsing::NonterminalParser* qualifiedIdNonterminalParser = GetNonterminal("QualifiedId");
        qualifiedIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SourceTokenRule>(this, &SourceTokenRule::PostQualifiedId));
        cminor::parsing::NonterminalParser* floatingLiteralNonterminalParser = GetNonterminal("FloatingLiteral");
        floatingLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SourceTokenRule>(this, &SourceTokenRule::PostFloatingLiteral));
        cminor::parsing::NonterminalParser* integerLiteralNonterminalParser = GetNonterminal("IntegerLiteral");
        integerLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SourceTokenRule>(this, &SourceTokenRule::PostIntegerLiteral));
        cminor::parsing::NonterminalParser* charLiteralNonterminalParser = GetNonterminal("CharLiteral");
        charLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SourceTokenRule>(this, &SourceTokenRule::PostCharLiteral));
        cminor::parsing::NonterminalParser* stringLiteralNonterminalParser = GetNonterminal("StringLiteral");
        stringLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SourceTokenRule>(this, &SourceTokenRule::PostStringLiteral));
        cminor::parsing::NonterminalParser* spacesNonterminalParser = GetNonterminal("Spaces");
        spacesNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SourceTokenRule>(this, &SourceTokenRule::PostSpaces));
        cminor::parsing::NonterminalParser* otherNonterminalParser = GetNonterminal("Other");
        otherNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SourceTokenRule>(this, &SourceTokenRule::PostOther));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.formatter->Keyword(std::string(matchBegin, matchEnd));
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.formatter->Identifier(std::string(matchBegin, matchEnd));
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.formatter->Number(std::string(matchBegin, matchEnd));
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.formatter->Char(std::string(matchBegin, matchEnd));
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.formatter->String(std::string(matchBegin, matchEnd));
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.formatter->Spaces(context.fromSpaces);
    }
    void A6Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.formatter->Comment(std::string(matchBegin, matchEnd));
    }
    void A7Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.formatter->Comment(std::string(matchBegin, matchEnd));
    }
    void A8Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.formatter->NewLine(std::string(matchBegin, matchEnd));
    }
    void A9Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.formatter->Other(context.fromOther);
    }
    void PostQualifiedId(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromQualifiedId_value = std::move(stack.top());
            context.fromQualifiedId = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromQualifiedId_value.get());
            stack.pop();
        }
    }
    void PostFloatingLiteral(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFloatingLiteral_value = std::move(stack.top());
            context.fromFloatingLiteral = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromFloatingLiteral_value.get());
            stack.pop();
        }
    }
    void PostIntegerLiteral(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIntegerLiteral_value = std::move(stack.top());
            context.fromIntegerLiteral = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromIntegerLiteral_value.get());
            stack.pop();
        }
    }
    void PostCharLiteral(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCharLiteral_value = std::move(stack.top());
            context.fromCharLiteral = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromCharLiteral_value.get());
            stack.pop();
        }
    }
    void PostStringLiteral(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromStringLiteral_value = std::move(stack.top());
            context.fromStringLiteral = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromStringLiteral_value.get());
            stack.pop();
        }
    }
    void PostSpaces(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpaces_value = std::move(stack.top());
            context.fromSpaces = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromSpaces_value.get());
            stack.pop();
        }
    }
    void PostOther(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromOther_value = std::move(stack.top());
            context.fromOther = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromOther_value.get());
            stack.pop();
        }
    }
private:
    struct Context
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
    std::stack<Context> contextStack;
    Context context;
};

class SourceTokenGrammar::SpacesRule : public cminor::parsing::Rule
{
public:
    SpacesRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SpacesRule>(this, &SpacesRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = std::string(matchBegin, matchEnd);
    }
private:
    struct Context
    {
        Context(): value() {}
        std::string value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class SourceTokenGrammar::OtherRule : public cminor::parsing::Rule
{
public:
    OtherRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OtherRule>(this, &OtherRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = std::string(matchBegin, matchEnd);
    }
private:
    struct Context
    {
        Context(): value() {}
        std::string value;
    };
    std::stack<Context> contextStack;
    Context context;
};

void SourceTokenGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("cminor.parsing.stdlib");
    if (!grammar0)
    {
        grammar0 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar("cminor.parser.LiteralGrammar");
    if (!grammar1)
    {
        grammar1 = cminor::parser::LiteralGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar("cminor.parser.KeywordGrammar");
    if (!grammar2)
    {
        grammar2 = cminor::parser::KeywordGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar("cminor.parser.IdentifierGrammar");
    if (!grammar3)
    {
        grammar3 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
}

void SourceTokenGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("line_comment", this, "cminor.parsing.stdlib.line_comment"));
    AddRuleLink(new cminor::parsing::RuleLink("IntegerLiteral", this, "LiteralGrammar.IntegerLiteral"));
    AddRuleLink(new cminor::parsing::RuleLink("Keyword", this, "KeywordGrammar.Keyword"));
    AddRuleLink(new cminor::parsing::RuleLink("QualifiedId", this, "IdentifierGrammar.QualifiedId"));
    AddRuleLink(new cminor::parsing::RuleLink("newline", this, "cminor.parsing.stdlib.newline"));
    AddRuleLink(new cminor::parsing::RuleLink("block_comment", this, "cminor.parsing.stdlib.block_comment"));
    AddRuleLink(new cminor::parsing::RuleLink("FloatingLiteral", this, "LiteralGrammar.FloatingLiteral"));
    AddRuleLink(new cminor::parsing::RuleLink("CharLiteral", this, "LiteralGrammar.CharLiteral"));
    AddRuleLink(new cminor::parsing::RuleLink("StringLiteral", this, "LiteralGrammar.StringLiteral"));
    AddRule(new SourceTokensRule("SourceTokens", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::EmptyParser()),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::NonterminalParser("SourceToken", "SourceToken", 1))),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::EmptyParser()))));
    AddRule(new SourceTokenRule("SourceToken", GetScope(),
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
                                                new cminor::parsing::NonterminalParser("Keyword", "Keyword", 0)),
                                            new cminor::parsing::ActionParser("A1",
                                                new cminor::parsing::NonterminalParser("QualifiedId", "QualifiedId", 0))),
                                        new cminor::parsing::ActionParser("A2",
                                            new cminor::parsing::AlternativeParser(
                                                new cminor::parsing::NonterminalParser("FloatingLiteral", "FloatingLiteral", 0),
                                                new cminor::parsing::NonterminalParser("IntegerLiteral", "IntegerLiteral", 0)))),
                                    new cminor::parsing::ActionParser("A3",
                                        new cminor::parsing::NonterminalParser("CharLiteral", "CharLiteral", 0))),
                                new cminor::parsing::ActionParser("A4",
                                    new cminor::parsing::NonterminalParser("StringLiteral", "StringLiteral", 0))),
                            new cminor::parsing::ActionParser("A5",
                                new cminor::parsing::NonterminalParser("Spaces", "Spaces", 0))),
                        new cminor::parsing::ActionParser("A6",
                            new cminor::parsing::NonterminalParser("line_comment", "line_comment", 0))),
                    new cminor::parsing::ActionParser("A7",
                        new cminor::parsing::NonterminalParser("block_comment", "block_comment", 0))),
                new cminor::parsing::ActionParser("A8",
                    new cminor::parsing::NonterminalParser("newline", "newline", 0))),
            new cminor::parsing::ActionParser("A9",
                new cminor::parsing::NonterminalParser("Other", "Other", 0)))));
    AddRule(new SpacesRule("Spaces", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::PositiveParser(
                new cminor::parsing::CharSetParser("\t ")))));
    AddRule(new OtherRule("Other", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::CharSetParser("\r\n", true))));
}

} } // namespace cminor.parser
