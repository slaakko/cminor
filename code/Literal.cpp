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

namespace cminor { namespace code {

using namespace cminor::codedom;
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

LiteralGrammar::LiteralGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("LiteralGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.code")), parsingDomain_)
{
    SetOwner(0);
}

cminor::codedom::Literal* LiteralGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName)
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
    cminor::codedom::Literal* result = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Literal*>*>(value.get());
    stack.pop();
    return result;
}

class LiteralGrammar::LiteralRule : public cminor::parsing::Rule
{
public:
    LiteralRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Literal*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Literal*>(context->value)));
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
        cminor::parsing::NonterminalParser* integerLiteralNonterminalParser = GetNonterminal(ToUtf32("IntegerLiteral"));
        integerLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostIntegerLiteral));
        cminor::parsing::NonterminalParser* characterLiteralNonterminalParser = GetNonterminal(ToUtf32("CharacterLiteral"));
        characterLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostCharacterLiteral));
        cminor::parsing::NonterminalParser* floatingLiteralNonterminalParser = GetNonterminal(ToUtf32("FloatingLiteral"));
        floatingLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostFloatingLiteral));
        cminor::parsing::NonterminalParser* stringLiteralNonterminalParser = GetNonterminal(ToUtf32("StringLiteral"));
        stringLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostStringLiteral));
        cminor::parsing::NonterminalParser* booleanLiteralNonterminalParser = GetNonterminal(ToUtf32("BooleanLiteral"));
        booleanLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostBooleanLiteral));
        cminor::parsing::NonterminalParser* pointerLiteralNonterminalParser = GetNonterminal(ToUtf32("PointerLiteral"));
        pointerLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostPointerLiteral));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromIntegerLiteral;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromCharacterLiteral;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromFloatingLiteral;
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromStringLiteral;
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromBooleanLiteral;
    }
    void A5Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromPointerLiteral;
    }
    void PostIntegerLiteral(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIntegerLiteral_value = std::move(stack.top());
            context->fromIntegerLiteral = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Literal*>*>(fromIntegerLiteral_value.get());
            stack.pop();
        }
    }
    void PostCharacterLiteral(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCharacterLiteral_value = std::move(stack.top());
            context->fromCharacterLiteral = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Literal*>*>(fromCharacterLiteral_value.get());
            stack.pop();
        }
    }
    void PostFloatingLiteral(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFloatingLiteral_value = std::move(stack.top());
            context->fromFloatingLiteral = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Literal*>*>(fromFloatingLiteral_value.get());
            stack.pop();
        }
    }
    void PostStringLiteral(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromStringLiteral_value = std::move(stack.top());
            context->fromStringLiteral = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Literal*>*>(fromStringLiteral_value.get());
            stack.pop();
        }
    }
    void PostBooleanLiteral(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromBooleanLiteral_value = std::move(stack.top());
            context->fromBooleanLiteral = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Literal*>*>(fromBooleanLiteral_value.get());
            stack.pop();
        }
    }
    void PostPointerLiteral(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromPointerLiteral_value = std::move(stack.top());
            context->fromPointerLiteral = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Literal*>*>(fromPointerLiteral_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromIntegerLiteral(), fromCharacterLiteral(), fromFloatingLiteral(), fromStringLiteral(), fromBooleanLiteral(), fromPointerLiteral() {}
        cminor::codedom::Literal* value;
        cminor::codedom::Literal* fromIntegerLiteral;
        cminor::codedom::Literal* fromCharacterLiteral;
        cminor::codedom::Literal* fromFloatingLiteral;
        cminor::codedom::Literal* fromStringLiteral;
        cminor::codedom::Literal* fromBooleanLiteral;
        cminor::codedom::Literal* fromPointerLiteral;
    };
};

class LiteralGrammar::IntegerLiteralRule : public cminor::parsing::Rule
{
public:
    IntegerLiteralRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Literal*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Literal*>(context->value)));
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
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Literal(std::u32string(matchBegin, matchEnd));
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Literal(std::u32string(matchBegin, matchEnd));
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Literal(std::u32string(matchBegin, matchEnd));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::codedom::Literal* value;
    };
};

class LiteralGrammar::CharacterLiteralRule : public cminor::parsing::Rule
{
public:
    CharacterLiteralRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Literal*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Literal*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharacterLiteralRule>(this, &CharacterLiteralRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharacterLiteralRule>(this, &CharacterLiteralRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharacterLiteralRule>(this, &CharacterLiteralRule::A2Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Literal(std::u32string(matchBegin, matchEnd));
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Literal(std::u32string(matchBegin, matchEnd));
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Literal(std::u32string(matchBegin, matchEnd));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::codedom::Literal* value;
    };
};

class LiteralGrammar::CCharSequenceRule : public cminor::parsing::Rule
{
public:
    CCharSequenceRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CCharSequenceRule>(this, &CCharSequenceRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = std::u32string(matchBegin, matchEnd);
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        std::u32string value;
    };
};

class LiteralGrammar::FloatingLiteralRule : public cminor::parsing::Rule
{
public:
    FloatingLiteralRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Literal*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Literal*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FloatingLiteralRule>(this, &FloatingLiteralRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FloatingLiteralRule>(this, &FloatingLiteralRule::A1Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Literal(std::u32string(matchBegin, matchEnd));
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Literal(std::u32string(matchBegin, matchEnd));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::codedom::Literal* value;
    };
};

class LiteralGrammar::StringLiteralRule : public cminor::parsing::Rule
{
public:
    StringLiteralRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Literal*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Literal*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StringLiteralRule>(this, &StringLiteralRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Literal(std::u32string(matchBegin, matchEnd));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::codedom::Literal* value;
    };
};

class LiteralGrammar::BooleanLiteralRule : public cminor::parsing::Rule
{
public:
    BooleanLiteralRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Literal*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Literal*>(context->value)));
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
        context->value = new Literal(U"true");
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Literal(U"false");
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::codedom::Literal* value;
    };
};

class LiteralGrammar::PointerLiteralRule : public cminor::parsing::Rule
{
public:
    PointerLiteralRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Literal*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Literal*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PointerLiteralRule>(this, &PointerLiteralRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new cminor::codedom::Literal(U"nullptr");
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::codedom::Literal* value;
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
                                new cminor::parsing::NonterminalParser(ToUtf32("IntegerLiteral"), ToUtf32("IntegerLiteral"), 0)),
                            new cminor::parsing::ActionParser(ToUtf32("A1"),
                                new cminor::parsing::NonterminalParser(ToUtf32("CharacterLiteral"), ToUtf32("CharacterLiteral"), 0))),
                        new cminor::parsing::ActionParser(ToUtf32("A2"),
                            new cminor::parsing::NonterminalParser(ToUtf32("FloatingLiteral"), ToUtf32("FloatingLiteral"), 0))),
                    new cminor::parsing::ActionParser(ToUtf32("A3"),
                        new cminor::parsing::NonterminalParser(ToUtf32("StringLiteral"), ToUtf32("StringLiteral"), 0))),
                new cminor::parsing::ActionParser(ToUtf32("A4"),
                    new cminor::parsing::NonterminalParser(ToUtf32("BooleanLiteral"), ToUtf32("BooleanLiteral"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A5"),
                new cminor::parsing::NonterminalParser(ToUtf32("PointerLiteral"), ToUtf32("PointerLiteral"), 0)))));
    AddRule(new IntegerLiteralRule(ToUtf32("IntegerLiteral"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::TokenParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("dl"), ToUtf32("DecimalLiteral"), 0),
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("is1"), ToUtf32("IntegerSuffix"), 0))))),
                new cminor::parsing::ActionParser(ToUtf32("A1"),
                    new cminor::parsing::TokenParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("ol"), ToUtf32("OctalLiteral"), 0),
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("is2"), ToUtf32("IntegerSuffix"), 0)))))),
            new cminor::parsing::ActionParser(ToUtf32("A2"),
                new cminor::parsing::TokenParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("hl"), ToUtf32("HexadecimalLiteral"), 0),
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("is3"), ToUtf32("IntegerSuffix"), 0))))))));
    AddRule(new cminor::parsing::Rule(ToUtf32("DecimalLiteral"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharSetParser(ToUtf32("1-9")),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::DigitParser())))));
    AddRule(new cminor::parsing::Rule(ToUtf32("OctalLiteral"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('0'),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("OctalDigit"), ToUtf32("OctalDigit"), 0))))));
    AddRule(new cminor::parsing::Rule(ToUtf32("OctalDigit"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::CharSetParser(ToUtf32("0-7"))));
    AddRule(new cminor::parsing::Rule(ToUtf32("HexadecimalLiteral"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::StringParser(ToUtf32("0x")),
                    new cminor::parsing::StringParser(ToUtf32("0X"))),
                new cminor::parsing::PositiveParser(
                    new cminor::parsing::HexDigitParser())))));
    AddRule(new cminor::parsing::Rule(ToUtf32("IntegerSuffix"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::TokenParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("UnsignedSuffix"), ToUtf32("UnsignedSuffix"), 0),
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("LongLongSuffix"), ToUtf32("LongLongSuffix"), 0),
                                new cminor::parsing::NonterminalParser(ToUtf32("LongSuffix"), ToUtf32("LongSuffix"), 0))))),
                new cminor::parsing::TokenParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("LongLongSuffix"), ToUtf32("LongLongSuffix"), 0),
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("UnsignedSuffix"), ToUtf32("UnsignedSuffix"), 0))))),
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("LongSuffix"), ToUtf32("LongSuffix"), 0),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("UnsignedSuffix"), ToUtf32("UnsignedSuffix"), 0)))))));
    AddRule(new cminor::parsing::Rule(ToUtf32("UnsignedSuffix"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::CharParser('u'),
            new cminor::parsing::CharParser('U'))));
    AddRule(new cminor::parsing::Rule(ToUtf32("LongLongSuffix"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::StringParser(ToUtf32("ll")),
            new cminor::parsing::StringParser(ToUtf32("LL")))));
    AddRule(new cminor::parsing::Rule(ToUtf32("LongSuffix"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::CharParser('l'),
            new cminor::parsing::CharParser('L'))));
    AddRule(new CharacterLiteralRule(ToUtf32("CharacterLiteral"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::NonterminalParser(ToUtf32("NarrowCharacterLiteral"), ToUtf32("NarrowCharacterLiteral"), 0)),
                new cminor::parsing::ActionParser(ToUtf32("A1"),
                    new cminor::parsing::NonterminalParser(ToUtf32("UniversalCharacterLiteral"), ToUtf32("UniversalCharacterLiteral"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A2"),
                new cminor::parsing::NonterminalParser(ToUtf32("WideCharacterLiteral"), ToUtf32("WideCharacterLiteral"), 0)))));
    AddRule(new cminor::parsing::Rule(ToUtf32("NarrowCharacterLiteral"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('\''),
                    new cminor::parsing::NonterminalParser(ToUtf32("CCharSequence"), ToUtf32("CCharSequence"), 0)),
                new cminor::parsing::CharParser('\'')))));
    AddRule(new cminor::parsing::Rule(ToUtf32("UniversalCharacterLiteral"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::CharParser('u'),
                            new cminor::parsing::CharParser('\'')),
                        new cminor::parsing::NonterminalParser(ToUtf32("cs1"), ToUtf32("CCharSequence"), 0)),
                    new cminor::parsing::CharParser('\''))),
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::CharParser('U'),
                            new cminor::parsing::CharParser('\'')),
                        new cminor::parsing::NonterminalParser(ToUtf32("cs2"), ToUtf32("CCharSequence"), 0)),
                    new cminor::parsing::CharParser('\''))))));
    AddRule(new cminor::parsing::Rule(ToUtf32("WideCharacterLiteral"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::CharParser('L'),
                        new cminor::parsing::CharParser('\'')),
                    new cminor::parsing::NonterminalParser(ToUtf32("CCharSequence"), ToUtf32("CCharSequence"), 0)),
                new cminor::parsing::CharParser('\'')))));
    AddRule(new CCharSequenceRule(ToUtf32("CCharSequence"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::TokenParser(
                new cminor::parsing::PositiveParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("CChar"), ToUtf32("CChar"), 0))))));
    AddRule(new cminor::parsing::Rule(ToUtf32("CChar"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::CharSetParser(ToUtf32("\'\\\n"), true),
            new cminor::parsing::NonterminalParser(ToUtf32("EscapeSequence"), ToUtf32("EscapeSequence"), 0))));
    AddRule(new cminor::parsing::Rule(ToUtf32("EscapeSequence"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::NonterminalParser(ToUtf32("SimpleEscapeSequence"), ToUtf32("SimpleEscapeSequence"), 0),
                new cminor::parsing::NonterminalParser(ToUtf32("OctalEscapeSequence"), ToUtf32("OctalEscapeSequence"), 0)),
            new cminor::parsing::NonterminalParser(ToUtf32("HexadecimalEscapeSequence"), ToUtf32("HexadecimalEscapeSequence"), 0))));
    AddRule(new cminor::parsing::Rule(ToUtf32("SimpleEscapeSequence"), GetScope(), GetParsingDomain()->GetNextRuleId(),
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
                                                new cminor::parsing::StringParser(ToUtf32("\\\'")),
                                                new cminor::parsing::StringParser(ToUtf32("\\\""))),
                                            new cminor::parsing::StringParser(ToUtf32("\\?"))),
                                        new cminor::parsing::StringParser(ToUtf32("\\\\"))),
                                    new cminor::parsing::StringParser(ToUtf32("\\a"))),
                                new cminor::parsing::StringParser(ToUtf32("\\b"))),
                            new cminor::parsing::StringParser(ToUtf32("\\f"))),
                        new cminor::parsing::StringParser(ToUtf32("\\n"))),
                    new cminor::parsing::StringParser(ToUtf32("\\r"))),
                new cminor::parsing::StringParser(ToUtf32("\\t"))),
            new cminor::parsing::StringParser(ToUtf32("\\v")))));
    AddRule(new cminor::parsing::Rule(ToUtf32("OctalEscapeSequence"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('\\'),
                    new cminor::parsing::NonterminalParser(ToUtf32("OctalDigit"), ToUtf32("OctalDigit"), 0)),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("OctalDigit"), ToUtf32("OctalDigit"), 0))),
            new cminor::parsing::OptionalParser(
                new cminor::parsing::NonterminalParser(ToUtf32("OctalDigit"), ToUtf32("OctalDigit"), 0)))));
    AddRule(new cminor::parsing::Rule(ToUtf32("HexadecimalEscapeSequence"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::StringParser(ToUtf32("\\x")),
            new cminor::parsing::PositiveParser(
                new cminor::parsing::HexDigitParser()))));
    AddRule(new FloatingLiteralRule(ToUtf32("FloatingLiteral"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::TokenParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("FractionalConstant"), ToUtf32("FractionalConstant"), 0),
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("ExponentPart"), ToUtf32("ExponentPart"), 0))),
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("FloatingSuffix"), ToUtf32("FloatingSuffix"), 0))))),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::TokenParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("DigitSequence"), ToUtf32("DigitSequence"), 0),
                            new cminor::parsing::NonterminalParser(ToUtf32("ExponentPart"), ToUtf32("ExponentPart"), 0)),
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("FloatingSuffix"), ToUtf32("FloatingSuffix"), 0))))))));
    AddRule(new cminor::parsing::Rule(ToUtf32("FractionalConstant"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("DigitSequence"), ToUtf32("DigitSequence"), 0)),
                    new cminor::parsing::CharParser('.')),
                new cminor::parsing::NonterminalParser(ToUtf32("DigitSequence"), ToUtf32("DigitSequence"), 0)),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::NonterminalParser(ToUtf32("DigitSequence"), ToUtf32("DigitSequence"), 0),
                new cminor::parsing::CharParser('.')))));
    AddRule(new cminor::parsing::Rule(ToUtf32("DigitSequence"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::PositiveParser(
            new cminor::parsing::DigitParser())));
    AddRule(new cminor::parsing::Rule(ToUtf32("ExponentPart"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::CharParser('e'),
                    new cminor::parsing::CharParser('E')),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("Sign"), ToUtf32("Sign"), 0))),
            new cminor::parsing::NonterminalParser(ToUtf32("DigitSequence"), ToUtf32("DigitSequence"), 0))));
    AddRule(new cminor::parsing::Rule(ToUtf32("Sign"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::CharParser('+'),
            new cminor::parsing::CharParser('-'))));
    AddRule(new cminor::parsing::Rule(ToUtf32("FloatingSuffix"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::CharSetParser(ToUtf32("fFlL"))));
    AddRule(new StringLiteralRule(ToUtf32("StringLiteral"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("EncodingPrefix"), ToUtf32("EncodingPrefix"), 0)),
                            new cminor::parsing::CharParser('\"')),
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("SCharSequence"), ToUtf32("SCharSequence"), 0))),
                    new cminor::parsing::CharParser('\"'))))));
    AddRule(new cminor::parsing::Rule(ToUtf32("EncodingPrefix"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::StringParser(ToUtf32("u8")),
                    new cminor::parsing::CharParser('u')),
                new cminor::parsing::CharParser('U')),
            new cminor::parsing::CharParser('L'))));
    AddRule(new cminor::parsing::Rule(ToUtf32("SCharSequence"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::PositiveParser(
            new cminor::parsing::NonterminalParser(ToUtf32("SChar"), ToUtf32("SChar"), 0))));
    AddRule(new cminor::parsing::Rule(ToUtf32("SChar"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::CharSetParser(ToUtf32("\"\\\n"), true),
            new cminor::parsing::NonterminalParser(ToUtf32("EscapeSequence"), ToUtf32("EscapeSequence"), 0))));
    AddRule(new BooleanLiteralRule(ToUtf32("BooleanLiteral"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::KeywordParser(ToUtf32("true"))),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::KeywordParser(ToUtf32("false"))))));
    AddRule(new PointerLiteralRule(ToUtf32("PointerLiteral"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("nullptr")))));
}

} } // namespace cminor.code
