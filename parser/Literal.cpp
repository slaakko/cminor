#include "Literal.hpp"
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
#include <sstream>

namespace cminor { namespace parser {

using namespace cminor::util;
using namespace cminor::parsing;
using namespace cminor::util;
using namespace cminor::unicode;

LiteralGrammar* LiteralGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

LiteralGrammar* LiteralGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    LiteralGrammar* grammar(new LiteralGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

LiteralGrammar::LiteralGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("LiteralGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.parser")), parsingDomain_)
{
    SetOwner(0);
}

Node* LiteralGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName)
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
    Node* result = *static_cast<cminor::parsing::ValueObject<Node*>*>(value.get());
    stack.pop();
    return result;
}

class LiteralGrammar::LiteralRule : public cminor::parsing::Rule
{
public:
    LiteralRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("Node*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LiteralRule>(this, &LiteralRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LiteralRule>(this, &LiteralRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LiteralRule>(this, &LiteralRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LiteralRule>(this, &LiteralRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction(ToUtf32("A4"));
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LiteralRule>(this, &LiteralRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction(ToUtf32("A5"));
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LiteralRule>(this, &LiteralRule::A5Action));
        cminor::parsing::NonterminalParser* booleanLiteralNonterminalParser = GetNonterminal(ToUtf32("BooleanLiteral"));
        booleanLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostBooleanLiteral));
        cminor::parsing::NonterminalParser* floatingLiteralNonterminalParser = GetNonterminal(ToUtf32("FloatingLiteral"));
        floatingLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostFloatingLiteral));
        cminor::parsing::NonterminalParser* integerLiteralNonterminalParser = GetNonterminal(ToUtf32("IntegerLiteral"));
        integerLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostIntegerLiteral));
        cminor::parsing::NonterminalParser* charLiteralNonterminalParser = GetNonterminal(ToUtf32("CharLiteral"));
        charLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostCharLiteral));
        cminor::parsing::NonterminalParser* stringLiteralNonterminalParser = GetNonterminal(ToUtf32("StringLiteral"));
        stringLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostStringLiteral));
        cminor::parsing::NonterminalParser* nullLiteralNonterminalParser = GetNonterminal(ToUtf32("NullLiteral"));
        nullLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostNullLiteral));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromBooleanLiteral;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromFloatingLiteral;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromIntegerLiteral;
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromCharLiteral;
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromStringLiteral;
    }
    void A5Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromNullLiteral;
    }
    void PostBooleanLiteral(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromBooleanLiteral_value = std::move(stack.top());
            context->fromBooleanLiteral = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromBooleanLiteral_value.get());
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
    void PostNullLiteral(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromNullLiteral_value = std::move(stack.top());
            context->fromNullLiteral = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromNullLiteral_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromBooleanLiteral(), fromFloatingLiteral(), fromIntegerLiteral(), fromCharLiteral(), fromStringLiteral(), fromNullLiteral() {}
        Node* value;
        Node* fromBooleanLiteral;
        Node* fromFloatingLiteral;
        Node* fromIntegerLiteral;
        Node* fromCharLiteral;
        Node* fromStringLiteral;
        Node* fromNullLiteral;
    };
};

class LiteralGrammar::BooleanLiteralRule : public cminor::parsing::Rule
{
public:
    BooleanLiteralRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("Node*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BooleanLiteralRule>(this, &BooleanLiteralRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BooleanLiteralRule>(this, &BooleanLiteralRule::A1Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BooleanLiteralNode(span, true);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BooleanLiteralNode(span, false);
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        Node* value;
    };
};

class LiteralGrammar::FloatingLiteralRule : public cminor::parsing::Rule
{
public:
    FloatingLiteralRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("Node*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("Span"), ToUtf32("s")));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FloatingLiteralRule>(this, &FloatingLiteralRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FloatingLiteralRule>(this, &FloatingLiteralRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FloatingLiteralRule>(this, &FloatingLiteralRule::A2Action));
        cminor::parsing::NonterminalParser* floatingLiteralValueNonterminalParser = GetNonterminal(ToUtf32("FloatingLiteralValue"));
        floatingLiteralValueNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<FloatingLiteralRule>(this, &FloatingLiteralRule::PostFloatingLiteralValue));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->s = span;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->s.SetEnd(span.End());
        context->value = CreateFloatingLiteralNode(context->s, context->fromFloatingLiteralValue, true);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = CreateFloatingLiteralNode(context->s, context->fromFloatingLiteralValue, false);
    }
    void PostFloatingLiteralValue(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFloatingLiteralValue_value = std::move(stack.top());
            context->fromFloatingLiteralValue = *static_cast<cminor::parsing::ValueObject<double>*>(fromFloatingLiteralValue_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), s(), fromFloatingLiteralValue() {}
        Node* value;
        Span s;
        double fromFloatingLiteralValue;
    };
};

class LiteralGrammar::FloatingLiteralValueRule : public cminor::parsing::Rule
{
public:
    FloatingLiteralValueRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("double"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<double>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FloatingLiteralValueRule>(this, &FloatingLiteralValueRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::stringstream s;
        s.str(std::string(matchBegin, matchEnd));
        s >> context->value;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        double value;
    };
};

class LiteralGrammar::IntegerLiteralRule : public cminor::parsing::Rule
{
public:
    IntegerLiteralRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("Node*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("Span"), ToUtf32("s")));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IntegerLiteralRule>(this, &IntegerLiteralRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IntegerLiteralRule>(this, &IntegerLiteralRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IntegerLiteralRule>(this, &IntegerLiteralRule::A2Action));
        cminor::parsing::NonterminalParser* integerLiteralValueNonterminalParser = GetNonterminal(ToUtf32("IntegerLiteralValue"));
        integerLiteralValueNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IntegerLiteralRule>(this, &IntegerLiteralRule::PostIntegerLiteralValue));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->s = span;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->s.SetEnd(span.End());
        context->value = CreateIntegerLiteralNode(context->s, context->fromIntegerLiteralValue, true);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = CreateIntegerLiteralNode(context->s, context->fromIntegerLiteralValue, false);
    }
    void PostIntegerLiteralValue(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIntegerLiteralValue_value = std::move(stack.top());
            context->fromIntegerLiteralValue = *static_cast<cminor::parsing::ValueObject<uint64_t>*>(fromIntegerLiteralValue_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), s(), fromIntegerLiteralValue() {}
        Node* value;
        Span s;
        uint64_t fromIntegerLiteralValue;
    };
};

class LiteralGrammar::IntegerLiteralValueRule : public cminor::parsing::Rule
{
public:
    IntegerLiteralValueRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("uint64_t"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<uint64_t>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IntegerLiteralValueRule>(this, &IntegerLiteralValueRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IntegerLiteralValueRule>(this, &IntegerLiteralValueRule::A1Action));
        cminor::parsing::NonterminalParser* hexIntegerLiteralNonterminalParser = GetNonterminal(ToUtf32("HexIntegerLiteral"));
        hexIntegerLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IntegerLiteralValueRule>(this, &IntegerLiteralValueRule::PostHexIntegerLiteral));
        cminor::parsing::NonterminalParser* decIntegerLiteralNonterminalParser = GetNonterminal(ToUtf32("DecIntegerLiteral"));
        decIntegerLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IntegerLiteralValueRule>(this, &IntegerLiteralValueRule::PostDecIntegerLiteral));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromHexIntegerLiteral;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromDecIntegerLiteral;
    }
    void PostHexIntegerLiteral(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromHexIntegerLiteral_value = std::move(stack.top());
            context->fromHexIntegerLiteral = *static_cast<cminor::parsing::ValueObject<uint64_t>*>(fromHexIntegerLiteral_value.get());
            stack.pop();
        }
    }
    void PostDecIntegerLiteral(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDecIntegerLiteral_value = std::move(stack.top());
            context->fromDecIntegerLiteral = *static_cast<cminor::parsing::ValueObject<uint64_t>*>(fromDecIntegerLiteral_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromHexIntegerLiteral(), fromDecIntegerLiteral() {}
        uint64_t value;
        uint64_t fromHexIntegerLiteral;
        uint64_t fromDecIntegerLiteral;
    };
};

class LiteralGrammar::HexIntegerLiteralRule : public cminor::parsing::Rule
{
public:
    HexIntegerLiteralRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("uint64_t"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<uint64_t>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<HexIntegerLiteralRule>(this, &HexIntegerLiteralRule::A0Action));
        cminor::parsing::NonterminalParser* hexDigitSequenceNonterminalParser = GetNonterminal(ToUtf32("HexDigitSequence"));
        hexDigitSequenceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<HexIntegerLiteralRule>(this, &HexIntegerLiteralRule::PostHexDigitSequence));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromHexDigitSequence;
    }
    void PostHexDigitSequence(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromHexDigitSequence_value = std::move(stack.top());
            context->fromHexDigitSequence = *static_cast<cminor::parsing::ValueObject<uint64_t>*>(fromHexDigitSequence_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromHexDigitSequence() {}
        uint64_t value;
        uint64_t fromHexDigitSequence;
    };
};

class LiteralGrammar::DecIntegerLiteralRule : public cminor::parsing::Rule
{
public:
    DecIntegerLiteralRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("uint64_t"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<uint64_t>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DecIntegerLiteralRule>(this, &DecIntegerLiteralRule::A0Action));
        cminor::parsing::NonterminalParser* decDigitSequenceNonterminalParser = GetNonterminal(ToUtf32("DecDigitSequence"));
        decDigitSequenceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DecIntegerLiteralRule>(this, &DecIntegerLiteralRule::PostDecDigitSequence));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromDecDigitSequence;
    }
    void PostDecDigitSequence(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDecDigitSequence_value = std::move(stack.top());
            context->fromDecDigitSequence = *static_cast<cminor::parsing::ValueObject<uint64_t>*>(fromDecDigitSequence_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromDecDigitSequence() {}
        uint64_t value;
        uint64_t fromDecDigitSequence;
    };
};

class LiteralGrammar::CharLiteralRule : public cminor::parsing::Rule
{
public:
    CharLiteralRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("Node*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("char32_t"), ToUtf32("litValue")));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharLiteralRule>(this, &CharLiteralRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharLiteralRule>(this, &CharLiteralRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharLiteralRule>(this, &CharLiteralRule::A2Action));
        cminor::parsing::NonterminalParser* charEscapeNonterminalParser = GetNonterminal(ToUtf32("CharEscape"));
        charEscapeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CharLiteralRule>(this, &CharLiteralRule::PostCharEscape));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new CharLiteralNode(span, context->litValue);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::u32string s = ToUtf32(std::string(matchBegin, matchEnd));
        if (s.empty()) pass = false;
        else context->litValue = s[0];
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->litValue = context->fromCharEscape;
    }
    void PostCharEscape(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCharEscape_value = std::move(stack.top());
            context->fromCharEscape = *static_cast<cminor::parsing::ValueObject<char32_t>*>(fromCharEscape_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), litValue(), fromCharEscape() {}
        Node* value;
        char32_t litValue;
        char32_t fromCharEscape;
    };
};

class LiteralGrammar::StringLiteralRule : public cminor::parsing::Rule
{
public:
    StringLiteralRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("Node*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::u32string"), ToUtf32("s")));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StringLiteralRule>(this, &StringLiteralRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StringLiteralRule>(this, &StringLiteralRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StringLiteralRule>(this, &StringLiteralRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StringLiteralRule>(this, &StringLiteralRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction(ToUtf32("A4"));
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StringLiteralRule>(this, &StringLiteralRule::A4Action));
        cminor::parsing::NonterminalParser* charEscapeNonterminalParser = GetNonterminal(ToUtf32("CharEscape"));
        charEscapeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StringLiteralRule>(this, &StringLiteralRule::PostCharEscape));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new StringLiteralNode(span, context->s);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->s = ToUtf32(std::string(matchBegin, matchEnd));
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new StringLiteralNode(span, context->s);
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->s.append(ToUtf32(std::string(matchBegin, matchEnd)));
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->s.append(1, context->fromCharEscape);
    }
    void PostCharEscape(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCharEscape_value = std::move(stack.top());
            context->fromCharEscape = *static_cast<cminor::parsing::ValueObject<char32_t>*>(fromCharEscape_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), s(), fromCharEscape() {}
        Node* value;
        std::u32string s;
        char32_t fromCharEscape;
    };
};

class LiteralGrammar::NullLiteralRule : public cminor::parsing::Rule
{
public:
    NullLiteralRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("Node*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NullLiteralRule>(this, &NullLiteralRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new NullLiteralNode(span);
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        Node* value;
    };
};

class LiteralGrammar::CharEscapeRule : public cminor::parsing::Rule
{
public:
    CharEscapeRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("char32_t"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<char32_t>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction(ToUtf32("A4"));
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction(ToUtf32("A5"));
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction(ToUtf32("A6"));
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction(ToUtf32("A7"));
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction(ToUtf32("A8"));
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A8Action));
        cminor::parsing::ActionParser* a9ActionParser = GetAction(ToUtf32("A9"));
        a9ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A9Action));
        cminor::parsing::ActionParser* a10ActionParser = GetAction(ToUtf32("A10"));
        a10ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A10Action));
        cminor::parsing::ActionParser* a11ActionParser = GetAction(ToUtf32("A11"));
        a11ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A11Action));
        cminor::parsing::ActionParser* a12ActionParser = GetAction(ToUtf32("A12"));
        a12ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A12Action));
        cminor::parsing::NonterminalParser* hexDigitSequenceNonterminalParser = GetNonterminal(ToUtf32("HexDigitSequence"));
        hexDigitSequenceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CharEscapeRule>(this, &CharEscapeRule::PostHexDigitSequence));
        cminor::parsing::NonterminalParser* decDigitSequenceNonterminalParser = GetNonterminal(ToUtf32("DecDigitSequence"));
        decDigitSequenceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CharEscapeRule>(this, &CharEscapeRule::PostDecDigitSequence));
        cminor::parsing::NonterminalParser* octalDigitSequenceNonterminalParser = GetNonterminal(ToUtf32("OctalDigitSequence"));
        octalDigitSequenceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CharEscapeRule>(this, &CharEscapeRule::PostOctalDigitSequence));
        cminor::parsing::NonterminalParser* hexDigit4NonterminalParser = GetNonterminal(ToUtf32("HexDigit4"));
        hexDigit4NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CharEscapeRule>(this, &CharEscapeRule::PostHexDigit4));
        cminor::parsing::NonterminalParser* hexDigit8NonterminalParser = GetNonterminal(ToUtf32("HexDigit8"));
        hexDigit8NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CharEscapeRule>(this, &CharEscapeRule::PostHexDigit8));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = char32_t(context->fromHexDigitSequence);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = char32_t(context->fromDecDigitSequence);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = char32_t(context->fromOctalDigitSequence);
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = char32_t(context->fromHexDigit4);
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = char32_t(context->fromHexDigit8);
    }
    void A5Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = char32_t('\a');
    }
    void A6Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = char32_t('\b');
    }
    void A7Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = char32_t('\f');
    }
    void A8Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = char32_t('\n');
    }
    void A9Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = char32_t('\r');
    }
    void A10Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = char32_t('\t');
    }
    void A11Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = char32_t('\v');
    }
    void A12Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = *matchBegin;
    }
    void PostHexDigitSequence(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromHexDigitSequence_value = std::move(stack.top());
            context->fromHexDigitSequence = *static_cast<cminor::parsing::ValueObject<uint64_t>*>(fromHexDigitSequence_value.get());
            stack.pop();
        }
    }
    void PostDecDigitSequence(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDecDigitSequence_value = std::move(stack.top());
            context->fromDecDigitSequence = *static_cast<cminor::parsing::ValueObject<uint64_t>*>(fromDecDigitSequence_value.get());
            stack.pop();
        }
    }
    void PostOctalDigitSequence(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromOctalDigitSequence_value = std::move(stack.top());
            context->fromOctalDigitSequence = *static_cast<cminor::parsing::ValueObject<uint64_t>*>(fromOctalDigitSequence_value.get());
            stack.pop();
        }
    }
    void PostHexDigit4(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromHexDigit4_value = std::move(stack.top());
            context->fromHexDigit4 = *static_cast<cminor::parsing::ValueObject<uint16_t>*>(fromHexDigit4_value.get());
            stack.pop();
        }
    }
    void PostHexDigit8(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromHexDigit8_value = std::move(stack.top());
            context->fromHexDigit8 = *static_cast<cminor::parsing::ValueObject<uint32_t>*>(fromHexDigit8_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromHexDigitSequence(), fromDecDigitSequence(), fromOctalDigitSequence(), fromHexDigit4(), fromHexDigit8() {}
        char32_t value;
        uint64_t fromHexDigitSequence;
        uint64_t fromDecDigitSequence;
        uint64_t fromOctalDigitSequence;
        uint16_t fromHexDigit4;
        uint32_t fromHexDigit8;
    };
};

class LiteralGrammar::DecDigitSequenceRule : public cminor::parsing::Rule
{
public:
    DecDigitSequenceRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("uint64_t"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<uint64_t>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DecDigitSequenceRule>(this, &DecDigitSequenceRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::stringstream s;
        s.str(std::string(matchBegin, matchEnd));
        s >> context->value;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        uint64_t value;
    };
};

class LiteralGrammar::HexDigitSequenceRule : public cminor::parsing::Rule
{
public:
    HexDigitSequenceRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("uint64_t"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<uint64_t>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<HexDigitSequenceRule>(this, &HexDigitSequenceRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::stringstream s;
        s.str(std::string(matchBegin, matchEnd));
        s >> std::hex >> context->value;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        uint64_t value;
    };
};

class LiteralGrammar::HexDigit4Rule : public cminor::parsing::Rule
{
public:
    HexDigit4Rule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("uint16_t"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<uint16_t>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<HexDigit4Rule>(this, &HexDigit4Rule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::stringstream s;
        s.str(std::string(matchBegin, matchEnd));
        s >> std::hex >> context->value;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        uint16_t value;
    };
};

class LiteralGrammar::HexDigit8Rule : public cminor::parsing::Rule
{
public:
    HexDigit8Rule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("uint32_t"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<uint32_t>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<HexDigit8Rule>(this, &HexDigit8Rule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::stringstream s;
        s.str(std::string(matchBegin, matchEnd));
        s >> std::hex >> context->value;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        uint32_t value;
    };
};

class LiteralGrammar::OctalDigitSequenceRule : public cminor::parsing::Rule
{
public:
    OctalDigitSequenceRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("uint64_t"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<uint64_t>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OctalDigitSequenceRule>(this, &OctalDigitSequenceRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = 8 * context->value + *matchBegin - '0';
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        uint64_t value;
    };
};

void LiteralGrammar::GetReferencedGrammars()
{
}

void LiteralGrammar::CreateRules()
{
    AddRule(new LiteralRule(ToUtf32("Literal"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::ActionParser(ToUtf32("A0"),
                                new cminor::parsing::NonterminalParser(ToUtf32("BooleanLiteral"), ToUtf32("BooleanLiteral"), 0)),
                            new cminor::parsing::ActionParser(ToUtf32("A1"),
                                new cminor::parsing::NonterminalParser(ToUtf32("FloatingLiteral"), ToUtf32("FloatingLiteral"), 0))),
                        new cminor::parsing::ActionParser(ToUtf32("A2"),
                            new cminor::parsing::NonterminalParser(ToUtf32("IntegerLiteral"), ToUtf32("IntegerLiteral"), 0))),
                    new cminor::parsing::ActionParser(ToUtf32("A3"),
                        new cminor::parsing::NonterminalParser(ToUtf32("CharLiteral"), ToUtf32("CharLiteral"), 0))),
                new cminor::parsing::ActionParser(ToUtf32("A4"),
                    new cminor::parsing::NonterminalParser(ToUtf32("StringLiteral"), ToUtf32("StringLiteral"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A5"),
                new cminor::parsing::NonterminalParser(ToUtf32("NullLiteral"), ToUtf32("NullLiteral"), 0)))));
    AddRule(new BooleanLiteralRule(ToUtf32("BooleanLiteral"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::KeywordParser(ToUtf32("true"))),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::KeywordParser(ToUtf32("false"))))));
    AddRule(new FloatingLiteralRule(ToUtf32("FloatingLiteral"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::NonterminalParser(ToUtf32("FloatingLiteralValue"), ToUtf32("FloatingLiteralValue"), 0)),
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                        new cminor::parsing::CharSetParser(ToUtf32("fF"))),
                    new cminor::parsing::ActionParser(ToUtf32("A2"),
                        new cminor::parsing::EmptyParser()))))));
    AddRule(new FloatingLiteralValueRule(ToUtf32("FloatingLiteralValue"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::TokenParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("FractionalFloatingLiteral"), ToUtf32("FractionalFloatingLiteral"), 0),
                    new cminor::parsing::NonterminalParser(ToUtf32("ExponentFloatingLiteral"), ToUtf32("ExponentFloatingLiteral"), 0))))));
    AddRule(new cminor::parsing::Rule(ToUtf32("FractionalFloatingLiteral"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("s0"), ToUtf32("DecDigitSequence"), 0)),
                            new cminor::parsing::CharParser('.')),
                        new cminor::parsing::NonterminalParser(ToUtf32("m"), ToUtf32("DecDigitSequence"), 0)),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("ExponentPart"), ToUtf32("ExponentPart"), 0)))),
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("s1"), ToUtf32("DecDigitSequence"), 0),
                    new cminor::parsing::CharParser('.'))))));
    AddRule(new cminor::parsing::Rule(ToUtf32("ExponentFloatingLiteral"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::NonterminalParser(ToUtf32("DecDigitSequence"), ToUtf32("DecDigitSequence"), 0),
                new cminor::parsing::NonterminalParser(ToUtf32("ExponentPart"), ToUtf32("ExponentPart"), 0)))));
    AddRule(new cminor::parsing::Rule(ToUtf32("ExponentPart"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharSetParser(ToUtf32("eE")),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("Sign"), ToUtf32("Sign"), 0))),
                new cminor::parsing::NonterminalParser(ToUtf32("DecDigitSequence"), ToUtf32("DecDigitSequence"), 0)))));
    AddRule(new IntegerLiteralRule(ToUtf32("IntegerLiteral"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::NonterminalParser(ToUtf32("IntegerLiteralValue"), ToUtf32("IntegerLiteralValue"), 0)),
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                        new cminor::parsing::CharSetParser(ToUtf32("uU"))),
                    new cminor::parsing::ActionParser(ToUtf32("A2"),
                        new cminor::parsing::EmptyParser()))))));
    AddRule(new IntegerLiteralValueRule(ToUtf32("IntegerLiteralValue"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::NonterminalParser(ToUtf32("HexIntegerLiteral"), ToUtf32("HexIntegerLiteral"), 0)),
                new cminor::parsing::ActionParser(ToUtf32("A1"),
                    new cminor::parsing::NonterminalParser(ToUtf32("DecIntegerLiteral"), ToUtf32("DecIntegerLiteral"), 0))))));
    AddRule(new HexIntegerLiteralRule(ToUtf32("HexIntegerLiteral"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::StringParser(ToUtf32("0x")),
                    new cminor::parsing::StringParser(ToUtf32("0X"))),
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::NonterminalParser(ToUtf32("HexDigitSequence"), ToUtf32("HexDigitSequence"), 0))))));
    AddRule(new DecIntegerLiteralRule(ToUtf32("DecIntegerLiteral"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::NonterminalParser(ToUtf32("DecDigitSequence"), ToUtf32("DecDigitSequence"), 0))));
    AddRule(new CharLiteralRule(ToUtf32("CharLiteral"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::CharParser('\''),
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::ActionParser(ToUtf32("A1"),
                                new cminor::parsing::PositiveParser(
                                    new cminor::parsing::CharSetParser(ToUtf32("\'\\\r\n"), true))),
                            new cminor::parsing::ActionParser(ToUtf32("A2"),
                                new cminor::parsing::NonterminalParser(ToUtf32("CharEscape"), ToUtf32("CharEscape"), 0)))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('\'')))))));
    AddRule(new StringLiteralRule(ToUtf32("StringLiteral"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('@'),
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::TokenParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::CharParser('\"'),
                                new cminor::parsing::ActionParser(ToUtf32("A1"),
                                    new cminor::parsing::KleeneStarParser(
                                        new cminor::parsing::CharSetParser(ToUtf32("\""), true)))),
                            new cminor::parsing::CharParser('\"'))))),
            new cminor::parsing::ActionParser(ToUtf32("A2"),
                new cminor::parsing::TokenParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::CharParser('\"'),
                            new cminor::parsing::KleeneStarParser(
                                new cminor::parsing::AlternativeParser(
                                    new cminor::parsing::ActionParser(ToUtf32("A3"),
                                        new cminor::parsing::PositiveParser(
                                            new cminor::parsing::CharSetParser(ToUtf32("\"\\\r\n"), true))),
                                    new cminor::parsing::ActionParser(ToUtf32("A4"),
                                        new cminor::parsing::NonterminalParser(ToUtf32("CharEscape"), ToUtf32("CharEscape"), 0))))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser('\"'))))))));
    AddRule(new NullLiteralRule(ToUtf32("NullLiteral"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("null")))));
    AddRule(new CharEscapeRule(ToUtf32("CharEscape"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('\\'),
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
                                                        new cminor::parsing::AlternativeParser(
                                                            new cminor::parsing::AlternativeParser(
                                                                new cminor::parsing::SequenceParser(
                                                                    new cminor::parsing::CharSetParser(ToUtf32("xX")),
                                                                    new cminor::parsing::ActionParser(ToUtf32("A0"),
                                                                        new cminor::parsing::NonterminalParser(ToUtf32("HexDigitSequence"), ToUtf32("HexDigitSequence"), 0))),
                                                                new cminor::parsing::SequenceParser(
                                                                    new cminor::parsing::CharSetParser(ToUtf32("dD")),
                                                                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                                                                        new cminor::parsing::NonterminalParser(ToUtf32("DecDigitSequence"), ToUtf32("DecDigitSequence"), 0)))),
                                                            new cminor::parsing::ActionParser(ToUtf32("A2"),
                                                                new cminor::parsing::NonterminalParser(ToUtf32("OctalDigitSequence"), ToUtf32("OctalDigitSequence"), 0))),
                                                        new cminor::parsing::SequenceParser(
                                                            new cminor::parsing::CharParser('u'),
                                                            new cminor::parsing::ActionParser(ToUtf32("A3"),
                                                                new cminor::parsing::NonterminalParser(ToUtf32("HexDigit4"), ToUtf32("HexDigit4"), 0)))),
                                                    new cminor::parsing::SequenceParser(
                                                        new cminor::parsing::CharParser('U'),
                                                        new cminor::parsing::ActionParser(ToUtf32("A4"),
                                                            new cminor::parsing::NonterminalParser(ToUtf32("HexDigit8"), ToUtf32("HexDigit8"), 0)))),
                                                new cminor::parsing::ActionParser(ToUtf32("A5"),
                                                    new cminor::parsing::CharParser('a'))),
                                            new cminor::parsing::ActionParser(ToUtf32("A6"),
                                                new cminor::parsing::CharParser('b'))),
                                        new cminor::parsing::ActionParser(ToUtf32("A7"),
                                            new cminor::parsing::CharParser('f'))),
                                    new cminor::parsing::ActionParser(ToUtf32("A8"),
                                        new cminor::parsing::CharParser('n'))),
                                new cminor::parsing::ActionParser(ToUtf32("A9"),
                                    new cminor::parsing::CharParser('r'))),
                            new cminor::parsing::ActionParser(ToUtf32("A10"),
                                new cminor::parsing::CharParser('t'))),
                        new cminor::parsing::ActionParser(ToUtf32("A11"),
                            new cminor::parsing::CharParser('v'))),
                    new cminor::parsing::ActionParser(ToUtf32("A12"),
                        new cminor::parsing::AnyCharParser()))))));
    AddRule(new DecDigitSequenceRule(ToUtf32("DecDigitSequence"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::TokenParser(
                new cminor::parsing::PositiveParser(
                    new cminor::parsing::DigitParser())))));
    AddRule(new HexDigitSequenceRule(ToUtf32("HexDigitSequence"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::TokenParser(
                new cminor::parsing::PositiveParser(
                    new cminor::parsing::HexDigitParser())))));
    AddRule(new HexDigit4Rule(ToUtf32("HexDigit4"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::HexDigitParser(),
                            new cminor::parsing::HexDigitParser()),
                        new cminor::parsing::HexDigitParser()),
                    new cminor::parsing::HexDigitParser())))));
    AddRule(new HexDigit8Rule(ToUtf32("HexDigit8"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::SequenceParser(
                                            new cminor::parsing::HexDigitParser(),
                                            new cminor::parsing::HexDigitParser()),
                                        new cminor::parsing::HexDigitParser()),
                                    new cminor::parsing::HexDigitParser()),
                                new cminor::parsing::HexDigitParser()),
                            new cminor::parsing::HexDigitParser()),
                        new cminor::parsing::HexDigitParser()),
                    new cminor::parsing::HexDigitParser())))));
    AddRule(new OctalDigitSequenceRule(ToUtf32("OctalDigitSequence"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::PositiveParser(
            new cminor::parsing::TokenParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::CharSetParser(ToUtf32("0-7")))))));
    AddRule(new cminor::parsing::Rule(ToUtf32("Sign"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::CharParser('+'),
            new cminor::parsing::CharParser('-'))));
}

} } // namespace cminor.parser
