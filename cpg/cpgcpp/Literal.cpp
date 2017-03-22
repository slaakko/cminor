#include "Literal.hpp"
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>

namespace cpg { namespace cpp {

using namespace cminor::pom;
using namespace cminor::parsing;

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

LiteralGrammar::LiteralGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("LiteralGrammar", parsingDomain_->GetNamespaceScope("cpg.cpp"), parsingDomain_)
{
    SetOwner(0);
}

cminor::pom::Literal* LiteralGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName)
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
    cminor::pom::Literal* result = *static_cast<cminor::parsing::ValueObject<cminor::pom::Literal*>*>(value.get());
    stack.pop();
    return result;
}

class LiteralGrammar::LiteralRule : public cminor::parsing::Rule
{
public:
    LiteralRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Literal*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Literal*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LiteralRule>(this, &LiteralRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LiteralRule>(this, &LiteralRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LiteralRule>(this, &LiteralRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LiteralRule>(this, &LiteralRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LiteralRule>(this, &LiteralRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LiteralRule>(this, &LiteralRule::A5Action));
        cminor::parsing::NonterminalParser* integerLiteralNonterminalParser = GetNonterminal("IntegerLiteral");
        integerLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostIntegerLiteral));
        cminor::parsing::NonterminalParser* characterLiteralNonterminalParser = GetNonterminal("CharacterLiteral");
        characterLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostCharacterLiteral));
        cminor::parsing::NonterminalParser* floatingLiteralNonterminalParser = GetNonterminal("FloatingLiteral");
        floatingLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostFloatingLiteral));
        cminor::parsing::NonterminalParser* stringLiteralNonterminalParser = GetNonterminal("StringLiteral");
        stringLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostStringLiteral));
        cminor::parsing::NonterminalParser* booleanLiteralNonterminalParser = GetNonterminal("BooleanLiteral");
        booleanLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostBooleanLiteral));
        cminor::parsing::NonterminalParser* pointerLiteralNonterminalParser = GetNonterminal("PointerLiteral");
        pointerLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostPointerLiteral));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromIntegerLiteral;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromCharacterLiteral;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromFloatingLiteral;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromStringLiteral;
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromBooleanLiteral;
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromIntegerLiteral = *static_cast<cminor::parsing::ValueObject<cminor::pom::Literal*>*>(fromIntegerLiteral_value.get());
            stack.pop();
        }
    }
    void PostCharacterLiteral(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCharacterLiteral_value = std::move(stack.top());
            context->fromCharacterLiteral = *static_cast<cminor::parsing::ValueObject<cminor::pom::Literal*>*>(fromCharacterLiteral_value.get());
            stack.pop();
        }
    }
    void PostFloatingLiteral(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFloatingLiteral_value = std::move(stack.top());
            context->fromFloatingLiteral = *static_cast<cminor::parsing::ValueObject<cminor::pom::Literal*>*>(fromFloatingLiteral_value.get());
            stack.pop();
        }
    }
    void PostStringLiteral(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromStringLiteral_value = std::move(stack.top());
            context->fromStringLiteral = *static_cast<cminor::parsing::ValueObject<cminor::pom::Literal*>*>(fromStringLiteral_value.get());
            stack.pop();
        }
    }
    void PostBooleanLiteral(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromBooleanLiteral_value = std::move(stack.top());
            context->fromBooleanLiteral = *static_cast<cminor::parsing::ValueObject<cminor::pom::Literal*>*>(fromBooleanLiteral_value.get());
            stack.pop();
        }
    }
    void PostPointerLiteral(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromPointerLiteral_value = std::move(stack.top());
            context->fromPointerLiteral = *static_cast<cminor::parsing::ValueObject<cminor::pom::Literal*>*>(fromPointerLiteral_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromIntegerLiteral(), fromCharacterLiteral(), fromFloatingLiteral(), fromStringLiteral(), fromBooleanLiteral(), fromPointerLiteral() {}
        cminor::pom::Literal* value;
        cminor::pom::Literal* fromIntegerLiteral;
        cminor::pom::Literal* fromCharacterLiteral;
        cminor::pom::Literal* fromFloatingLiteral;
        cminor::pom::Literal* fromStringLiteral;
        cminor::pom::Literal* fromBooleanLiteral;
        cminor::pom::Literal* fromPointerLiteral;
    };
};

class LiteralGrammar::IntegerLiteralRule : public cminor::parsing::Rule
{
public:
    IntegerLiteralRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Literal*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Literal*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IntegerLiteralRule>(this, &IntegerLiteralRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IntegerLiteralRule>(this, &IntegerLiteralRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IntegerLiteralRule>(this, &IntegerLiteralRule::A2Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Literal(std::string(matchBegin, matchEnd));
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Literal(std::string(matchBegin, matchEnd));
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Literal(std::string(matchBegin, matchEnd));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::pom::Literal* value;
    };
};

class LiteralGrammar::CharacterLiteralRule : public cminor::parsing::Rule
{
public:
    CharacterLiteralRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Literal*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Literal*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharacterLiteralRule>(this, &CharacterLiteralRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharacterLiteralRule>(this, &CharacterLiteralRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharacterLiteralRule>(this, &CharacterLiteralRule::A2Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Literal(std::string(matchBegin, matchEnd));
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Literal(std::string(matchBegin, matchEnd));
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Literal(std::string(matchBegin, matchEnd));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::pom::Literal* value;
    };
};

class LiteralGrammar::CCharSequenceRule : public cminor::parsing::Rule
{
public:
    CCharSequenceRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("std::string");
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
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CCharSequenceRule>(this, &CCharSequenceRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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

class LiteralGrammar::FloatingLiteralRule : public cminor::parsing::Rule
{
public:
    FloatingLiteralRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Literal*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Literal*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FloatingLiteralRule>(this, &FloatingLiteralRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FloatingLiteralRule>(this, &FloatingLiteralRule::A1Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Literal(std::string(matchBegin, matchEnd));
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Literal(std::string(matchBegin, matchEnd));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::pom::Literal* value;
    };
};

class LiteralGrammar::StringLiteralRule : public cminor::parsing::Rule
{
public:
    StringLiteralRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Literal*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Literal*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StringLiteralRule>(this, &StringLiteralRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Literal(std::string(matchBegin, matchEnd));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::pom::Literal* value;
    };
};

class LiteralGrammar::BooleanLiteralRule : public cminor::parsing::Rule
{
public:
    BooleanLiteralRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Literal*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Literal*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BooleanLiteralRule>(this, &BooleanLiteralRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BooleanLiteralRule>(this, &BooleanLiteralRule::A1Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Literal("true");
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Literal("false");
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::pom::Literal* value;
    };
};

class LiteralGrammar::PointerLiteralRule : public cminor::parsing::Rule
{
public:
    PointerLiteralRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Literal*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Literal*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PointerLiteralRule>(this, &PointerLiteralRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new cminor::pom::Literal("nullptr");
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::pom::Literal* value;
    };
};

void LiteralGrammar::GetReferencedGrammars()
{
}

void LiteralGrammar::CreateRules()
{
    AddRule(new LiteralRule("Literal", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::ActionParser("A0",
                                new cminor::parsing::NonterminalParser("IntegerLiteral", "IntegerLiteral", 0)),
                            new cminor::parsing::ActionParser("A1",
                                new cminor::parsing::NonterminalParser("CharacterLiteral", "CharacterLiteral", 0))),
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::NonterminalParser("FloatingLiteral", "FloatingLiteral", 0))),
                    new cminor::parsing::ActionParser("A3",
                        new cminor::parsing::NonterminalParser("StringLiteral", "StringLiteral", 0))),
                new cminor::parsing::ActionParser("A4",
                    new cminor::parsing::NonterminalParser("BooleanLiteral", "BooleanLiteral", 0))),
            new cminor::parsing::ActionParser("A5",
                new cminor::parsing::NonterminalParser("PointerLiteral", "PointerLiteral", 0)))));
    AddRule(new IntegerLiteralRule("IntegerLiteral", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::TokenParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::NonterminalParser("dl", "DecimalLiteral", 0),
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::NonterminalParser("is1", "IntegerSuffix", 0))))),
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::TokenParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::NonterminalParser("ol", "OctalLiteral", 0),
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::NonterminalParser("is2", "IntegerSuffix", 0)))))),
            new cminor::parsing::ActionParser("A2",
                new cminor::parsing::TokenParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::NonterminalParser("hl", "HexadecimalLiteral", 0),
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::NonterminalParser("is3", "IntegerSuffix", 0))))))));
    AddRule(new cminor::parsing::Rule("DecimalLiteral", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharSetParser("1-9"),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::DigitParser())))));
    AddRule(new cminor::parsing::Rule("OctalLiteral", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('0'),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::NonterminalParser("OctalDigit", "OctalDigit", 0))))));
    AddRule(new cminor::parsing::Rule("OctalDigit", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::CharSetParser("0-7")));
    AddRule(new cminor::parsing::Rule("HexadecimalLiteral", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::StringParser("0x"),
                    new cminor::parsing::StringParser("0X")),
                new cminor::parsing::PositiveParser(
                    new cminor::parsing::HexDigitParser())))));
    AddRule(new cminor::parsing::Rule("IntegerSuffix", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::TokenParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::NonterminalParser("UnsignedSuffix", "UnsignedSuffix", 0),
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::NonterminalParser("LongLongSuffix", "LongLongSuffix", 0),
                                new cminor::parsing::NonterminalParser("LongSuffix", "LongSuffix", 0))))),
                new cminor::parsing::TokenParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::NonterminalParser("LongLongSuffix", "LongLongSuffix", 0),
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::NonterminalParser("UnsignedSuffix", "UnsignedSuffix", 0))))),
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser("LongSuffix", "LongSuffix", 0),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser("UnsignedSuffix", "UnsignedSuffix", 0)))))));
    AddRule(new cminor::parsing::Rule("UnsignedSuffix", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::CharParser('u'),
            new cminor::parsing::CharParser('U'))));
    AddRule(new cminor::parsing::Rule("LongLongSuffix", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::StringParser("ll"),
            new cminor::parsing::StringParser("LL"))));
    AddRule(new cminor::parsing::Rule("LongSuffix", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::CharParser('l'),
            new cminor::parsing::CharParser('L'))));
    AddRule(new CharacterLiteralRule("CharacterLiteral", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::NonterminalParser("NarrowCharacterLiteral", "NarrowCharacterLiteral", 0)),
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("UniversalCharacterLiteral", "UniversalCharacterLiteral", 0))),
            new cminor::parsing::ActionParser("A2",
                new cminor::parsing::NonterminalParser("WideCharacterLiteral", "WideCharacterLiteral", 0)))));
    AddRule(new cminor::parsing::Rule("NarrowCharacterLiteral", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('\''),
                    new cminor::parsing::NonterminalParser("CCharSequence", "CCharSequence", 0)),
                new cminor::parsing::CharParser('\'')))));
    AddRule(new cminor::parsing::Rule("UniversalCharacterLiteral", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::CharParser('u'),
                            new cminor::parsing::CharParser('\'')),
                        new cminor::parsing::NonterminalParser("cs1", "CCharSequence", 0)),
                    new cminor::parsing::CharParser('\''))),
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::CharParser('U'),
                            new cminor::parsing::CharParser('\'')),
                        new cminor::parsing::NonterminalParser("cs2", "CCharSequence", 0)),
                    new cminor::parsing::CharParser('\''))))));
    AddRule(new cminor::parsing::Rule("WideCharacterLiteral", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::CharParser('L'),
                        new cminor::parsing::CharParser('\'')),
                    new cminor::parsing::NonterminalParser("CCharSequence", "CCharSequence", 0)),
                new cminor::parsing::CharParser('\'')))));
    AddRule(new CCharSequenceRule("CCharSequence", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::TokenParser(
                new cminor::parsing::PositiveParser(
                    new cminor::parsing::NonterminalParser("CChar", "CChar", 0))))));
    AddRule(new cminor::parsing::Rule("CChar", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::CharSetParser("\'\\\n", true),
            new cminor::parsing::NonterminalParser("EscapeSequence", "EscapeSequence", 0))));
    AddRule(new cminor::parsing::Rule("EscapeSequence", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::NonterminalParser("SimpleEscapeSequence", "SimpleEscapeSequence", 0),
                new cminor::parsing::NonterminalParser("OctalEscapeSequence", "OctalEscapeSequence", 0)),
            new cminor::parsing::NonterminalParser("HexadecimalEscapeSequence", "HexadecimalEscapeSequence", 0))));
    AddRule(new cminor::parsing::Rule("SimpleEscapeSequence", GetScope(), GetParsingDomain()->GetNextRuleId(),
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
                                                new cminor::parsing::StringParser("\\\'"),
                                                new cminor::parsing::StringParser("\\\"")),
                                            new cminor::parsing::StringParser("\\?")),
                                        new cminor::parsing::StringParser("\\\\")),
                                    new cminor::parsing::StringParser("\\a")),
                                new cminor::parsing::StringParser("\\b")),
                            new cminor::parsing::StringParser("\\f")),
                        new cminor::parsing::StringParser("\\n")),
                    new cminor::parsing::StringParser("\\r")),
                new cminor::parsing::StringParser("\\t")),
            new cminor::parsing::StringParser("\\v"))));
    AddRule(new cminor::parsing::Rule("OctalEscapeSequence", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('\\'),
                    new cminor::parsing::NonterminalParser("OctalDigit", "OctalDigit", 0)),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::NonterminalParser("OctalDigit", "OctalDigit", 0))),
            new cminor::parsing::OptionalParser(
                new cminor::parsing::NonterminalParser("OctalDigit", "OctalDigit", 0)))));
    AddRule(new cminor::parsing::Rule("HexadecimalEscapeSequence", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::StringParser("\\x"),
            new cminor::parsing::PositiveParser(
                new cminor::parsing::HexDigitParser()))));
    AddRule(new FloatingLiteralRule("FloatingLiteral", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::TokenParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::NonterminalParser("FractionalConstant", "FractionalConstant", 0),
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::NonterminalParser("ExponentPart", "ExponentPart", 0))),
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::NonterminalParser("FloatingSuffix", "FloatingSuffix", 0))))),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::TokenParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::NonterminalParser("DigitSequence", "DigitSequence", 0),
                            new cminor::parsing::NonterminalParser("ExponentPart", "ExponentPart", 0)),
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::NonterminalParser("FloatingSuffix", "FloatingSuffix", 0))))))));
    AddRule(new cminor::parsing::Rule("FractionalConstant", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser("DigitSequence", "DigitSequence", 0)),
                    new cminor::parsing::CharParser('.')),
                new cminor::parsing::NonterminalParser("DigitSequence", "DigitSequence", 0)),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::NonterminalParser("DigitSequence", "DigitSequence", 0),
                new cminor::parsing::CharParser('.')))));
    AddRule(new cminor::parsing::Rule("DigitSequence", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::PositiveParser(
            new cminor::parsing::DigitParser())));
    AddRule(new cminor::parsing::Rule("ExponentPart", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::CharParser('e'),
                    new cminor::parsing::CharParser('E')),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::NonterminalParser("Sign", "Sign", 0))),
            new cminor::parsing::NonterminalParser("DigitSequence", "DigitSequence", 0))));
    AddRule(new cminor::parsing::Rule("Sign", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::CharParser('+'),
            new cminor::parsing::CharParser('-'))));
    AddRule(new cminor::parsing::Rule("FloatingSuffix", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::CharSetParser("fFlL")));
    AddRule(new StringLiteralRule("StringLiteral", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::NonterminalParser("EncodingPrefix", "EncodingPrefix", 0)),
                            new cminor::parsing::CharParser('\"')),
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::NonterminalParser("SCharSequence", "SCharSequence", 0))),
                    new cminor::parsing::CharParser('\"'))))));
    AddRule(new cminor::parsing::Rule("EncodingPrefix", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::StringParser("u8"),
                    new cminor::parsing::CharParser('u')),
                new cminor::parsing::CharParser('U')),
            new cminor::parsing::CharParser('L'))));
    AddRule(new cminor::parsing::Rule("SCharSequence", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::PositiveParser(
            new cminor::parsing::NonterminalParser("SChar", "SChar", 0))));
    AddRule(new cminor::parsing::Rule("SChar", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::CharSetParser("\"\\\n", true),
            new cminor::parsing::NonterminalParser("EscapeSequence", "EscapeSequence", 0))));
    AddRule(new BooleanLiteralRule("BooleanLiteral", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::KeywordParser("true")),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::KeywordParser("false")))));
    AddRule(new PointerLiteralRule("PointerLiteral", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::KeywordParser("nullptr"))));
}

} } // namespace cpg.cpp
