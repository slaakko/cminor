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
#include <sstream>

namespace cminor { namespace parser {

using namespace cminor::util;
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

LiteralGrammar::LiteralGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("LiteralGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

Node* LiteralGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName)
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
    Node* result = *static_cast<cminor::parsing::ValueObject<Node*>*>(value.get());
    stack.pop();
    return result;
}

class LiteralGrammar::LiteralRule : public cminor::parsing::Rule
{
public:
    LiteralRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Node*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
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
        cminor::parsing::NonterminalParser* booleanLiteralNonterminalParser = GetNonterminal("BooleanLiteral");
        booleanLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostBooleanLiteral));
        cminor::parsing::NonterminalParser* floatingLiteralNonterminalParser = GetNonterminal("FloatingLiteral");
        floatingLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostFloatingLiteral));
        cminor::parsing::NonterminalParser* integerLiteralNonterminalParser = GetNonterminal("IntegerLiteral");
        integerLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostIntegerLiteral));
        cminor::parsing::NonterminalParser* charLiteralNonterminalParser = GetNonterminal("CharLiteral");
        charLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostCharLiteral));
        cminor::parsing::NonterminalParser* stringLiteralNonterminalParser = GetNonterminal("StringLiteral");
        stringLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostStringLiteral));
        cminor::parsing::NonterminalParser* nullLiteralNonterminalParser = GetNonterminal("NullLiteral");
        nullLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostNullLiteral));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromBooleanLiteral;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromFloatingLiteral;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromIntegerLiteral;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromCharLiteral;
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromStringLiteral;
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromNullLiteral;
    }
    void PostBooleanLiteral(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromBooleanLiteral_value = std::move(stack.top());
            context.fromBooleanLiteral = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromBooleanLiteral_value.get());
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
    void PostNullLiteral(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromNullLiteral_value = std::move(stack.top());
            context.fromNullLiteral = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromNullLiteral_value.get());
            stack.pop();
        }
    }
private:
    struct Context
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
    std::stack<Context> contextStack;
    Context context;
};

class LiteralGrammar::BooleanLiteralRule : public cminor::parsing::Rule
{
public:
    BooleanLiteralRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Node*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BooleanLiteralRule>(this, &BooleanLiteralRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BooleanLiteralRule>(this, &BooleanLiteralRule::A1Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new BooleanLiteralNode(span, true);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new BooleanLiteralNode(span, false);
    }
private:
    struct Context
    {
        Context(): value() {}
        Node* value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class LiteralGrammar::FloatingLiteralRule : public cminor::parsing::Rule
{
public:
    FloatingLiteralRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("Span", "s"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FloatingLiteralRule>(this, &FloatingLiteralRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FloatingLiteralRule>(this, &FloatingLiteralRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FloatingLiteralRule>(this, &FloatingLiteralRule::A2Action));
        cminor::parsing::NonterminalParser* floatingLiteralValueNonterminalParser = GetNonterminal("FloatingLiteralValue");
        floatingLiteralValueNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<FloatingLiteralRule>(this, &FloatingLiteralRule::PostFloatingLiteralValue));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s = span;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s.SetEnd(span.End());
        context.value = CreateFloatingLiteralNode(context.s, context.fromFloatingLiteralValue, true);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = CreateFloatingLiteralNode(context.s, context.fromFloatingLiteralValue, false);
    }
    void PostFloatingLiteralValue(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFloatingLiteralValue_value = std::move(stack.top());
            context.fromFloatingLiteralValue = *static_cast<cminor::parsing::ValueObject<double>*>(fromFloatingLiteralValue_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), s(), fromFloatingLiteralValue() {}
        Node* value;
        Span s;
        double fromFloatingLiteralValue;
    };
    std::stack<Context> contextStack;
    Context context;
};

class LiteralGrammar::FloatingLiteralValueRule : public cminor::parsing::Rule
{
public:
    FloatingLiteralValueRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("double");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<double>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FloatingLiteralValueRule>(this, &FloatingLiteralValueRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        std::stringstream s;
        s.str(std::string(matchBegin, matchEnd));
        s >> context.value;
    }
private:
    struct Context
    {
        Context(): value() {}
        double value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class LiteralGrammar::IntegerLiteralRule : public cminor::parsing::Rule
{
public:
    IntegerLiteralRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("Span", "s"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IntegerLiteralRule>(this, &IntegerLiteralRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IntegerLiteralRule>(this, &IntegerLiteralRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IntegerLiteralRule>(this, &IntegerLiteralRule::A2Action));
        cminor::parsing::NonterminalParser* integerLiteralValueNonterminalParser = GetNonterminal("IntegerLiteralValue");
        integerLiteralValueNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IntegerLiteralRule>(this, &IntegerLiteralRule::PostIntegerLiteralValue));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s = span;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s.SetEnd(span.End());
        context.value = CreateIntegerLiteralNode(context.s, context.fromIntegerLiteralValue, true);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = CreateIntegerLiteralNode(context.s, context.fromIntegerLiteralValue, false);
    }
    void PostIntegerLiteralValue(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIntegerLiteralValue_value = std::move(stack.top());
            context.fromIntegerLiteralValue = *static_cast<cminor::parsing::ValueObject<uint64_t>*>(fromIntegerLiteralValue_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), s(), fromIntegerLiteralValue() {}
        Node* value;
        Span s;
        uint64_t fromIntegerLiteralValue;
    };
    std::stack<Context> contextStack;
    Context context;
};

class LiteralGrammar::IntegerLiteralValueRule : public cminor::parsing::Rule
{
public:
    IntegerLiteralValueRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("uint64_t");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<uint64_t>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IntegerLiteralValueRule>(this, &IntegerLiteralValueRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IntegerLiteralValueRule>(this, &IntegerLiteralValueRule::A1Action));
        cminor::parsing::NonterminalParser* hexIntegerLiteralNonterminalParser = GetNonterminal("HexIntegerLiteral");
        hexIntegerLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IntegerLiteralValueRule>(this, &IntegerLiteralValueRule::PostHexIntegerLiteral));
        cminor::parsing::NonterminalParser* decIntegerLiteralNonterminalParser = GetNonterminal("DecIntegerLiteral");
        decIntegerLiteralNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IntegerLiteralValueRule>(this, &IntegerLiteralValueRule::PostDecIntegerLiteral));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromHexIntegerLiteral;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromDecIntegerLiteral;
    }
    void PostHexIntegerLiteral(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromHexIntegerLiteral_value = std::move(stack.top());
            context.fromHexIntegerLiteral = *static_cast<cminor::parsing::ValueObject<uint64_t>*>(fromHexIntegerLiteral_value.get());
            stack.pop();
        }
    }
    void PostDecIntegerLiteral(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDecIntegerLiteral_value = std::move(stack.top());
            context.fromDecIntegerLiteral = *static_cast<cminor::parsing::ValueObject<uint64_t>*>(fromDecIntegerLiteral_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromHexIntegerLiteral(), fromDecIntegerLiteral() {}
        uint64_t value;
        uint64_t fromHexIntegerLiteral;
        uint64_t fromDecIntegerLiteral;
    };
    std::stack<Context> contextStack;
    Context context;
};

class LiteralGrammar::HexIntegerLiteralRule : public cminor::parsing::Rule
{
public:
    HexIntegerLiteralRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("uint64_t");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<uint64_t>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<HexIntegerLiteralRule>(this, &HexIntegerLiteralRule::A0Action));
        cminor::parsing::NonterminalParser* hexDigitSequenceNonterminalParser = GetNonterminal("HexDigitSequence");
        hexDigitSequenceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<HexIntegerLiteralRule>(this, &HexIntegerLiteralRule::PostHexDigitSequence));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromHexDigitSequence;
    }
    void PostHexDigitSequence(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromHexDigitSequence_value = std::move(stack.top());
            context.fromHexDigitSequence = *static_cast<cminor::parsing::ValueObject<uint64_t>*>(fromHexDigitSequence_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromHexDigitSequence() {}
        uint64_t value;
        uint64_t fromHexDigitSequence;
    };
    std::stack<Context> contextStack;
    Context context;
};

class LiteralGrammar::DecIntegerLiteralRule : public cminor::parsing::Rule
{
public:
    DecIntegerLiteralRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("uint64_t");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<uint64_t>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DecIntegerLiteralRule>(this, &DecIntegerLiteralRule::A0Action));
        cminor::parsing::NonterminalParser* decDigitSequenceNonterminalParser = GetNonterminal("DecDigitSequence");
        decDigitSequenceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DecIntegerLiteralRule>(this, &DecIntegerLiteralRule::PostDecDigitSequence));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromDecDigitSequence;
    }
    void PostDecDigitSequence(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDecDigitSequence_value = std::move(stack.top());
            context.fromDecDigitSequence = *static_cast<cminor::parsing::ValueObject<uint64_t>*>(fromDecDigitSequence_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromDecDigitSequence() {}
        uint64_t value;
        uint64_t fromDecDigitSequence;
    };
    std::stack<Context> contextStack;
    Context context;
};

class LiteralGrammar::CharLiteralRule : public cminor::parsing::Rule
{
public:
    CharLiteralRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("utf32_char", "litValue"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharLiteralRule>(this, &CharLiteralRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharLiteralRule>(this, &CharLiteralRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharLiteralRule>(this, &CharLiteralRule::A2Action));
        cminor::parsing::NonterminalParser* charEscapeNonterminalParser = GetNonterminal("CharEscape");
        charEscapeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CharLiteralRule>(this, &CharLiteralRule::PostCharEscape));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new CharLiteralNode(span, context.litValue);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        utf32_string s = ToUtf32(std::string(matchBegin, matchEnd));
        if (s.empty()) pass = false;
        else context.litValue = s[0];
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.litValue = context.fromCharEscape;
    }
    void PostCharEscape(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCharEscape_value = std::move(stack.top());
            context.fromCharEscape = *static_cast<cminor::parsing::ValueObject<utf32_char>*>(fromCharEscape_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), litValue(), fromCharEscape() {}
        Node* value;
        utf32_char litValue;
        utf32_char fromCharEscape;
    };
    std::stack<Context> contextStack;
    Context context;
};

class LiteralGrammar::StringLiteralRule : public cminor::parsing::Rule
{
public:
    StringLiteralRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("utf32_string", "s"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StringLiteralRule>(this, &StringLiteralRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StringLiteralRule>(this, &StringLiteralRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StringLiteralRule>(this, &StringLiteralRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StringLiteralRule>(this, &StringLiteralRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StringLiteralRule>(this, &StringLiteralRule::A4Action));
        cminor::parsing::NonterminalParser* charEscapeNonterminalParser = GetNonterminal("CharEscape");
        charEscapeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StringLiteralRule>(this, &StringLiteralRule::PostCharEscape));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new StringLiteralNode(span, context.s);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s = ToUtf32(std::string(matchBegin, matchEnd));
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new StringLiteralNode(span, context.s);
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s.append(ToUtf32(std::string(matchBegin, matchEnd)));
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s.append(1, context.fromCharEscape);
    }
    void PostCharEscape(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCharEscape_value = std::move(stack.top());
            context.fromCharEscape = *static_cast<cminor::parsing::ValueObject<utf32_char>*>(fromCharEscape_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), s(), fromCharEscape() {}
        Node* value;
        utf32_string s;
        utf32_char fromCharEscape;
    };
    std::stack<Context> contextStack;
    Context context;
};

class LiteralGrammar::NullLiteralRule : public cminor::parsing::Rule
{
public:
    NullLiteralRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Node*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NullLiteralRule>(this, &NullLiteralRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new NullLiteralNode(span);
    }
private:
    struct Context
    {
        Context(): value() {}
        Node* value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class LiteralGrammar::CharEscapeRule : public cminor::parsing::Rule
{
public:
    CharEscapeRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("utf32_char");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<utf32_char>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction("A6");
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction("A7");
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction("A8");
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A8Action));
        cminor::parsing::ActionParser* a9ActionParser = GetAction("A9");
        a9ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A9Action));
        cminor::parsing::ActionParser* a10ActionParser = GetAction("A10");
        a10ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A10Action));
        cminor::parsing::ActionParser* a11ActionParser = GetAction("A11");
        a11ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A11Action));
        cminor::parsing::ActionParser* a12ActionParser = GetAction("A12");
        a12ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A12Action));
        cminor::parsing::NonterminalParser* hexDigitSequenceNonterminalParser = GetNonterminal("HexDigitSequence");
        hexDigitSequenceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CharEscapeRule>(this, &CharEscapeRule::PostHexDigitSequence));
        cminor::parsing::NonterminalParser* decDigitSequenceNonterminalParser = GetNonterminal("DecDigitSequence");
        decDigitSequenceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CharEscapeRule>(this, &CharEscapeRule::PostDecDigitSequence));
        cminor::parsing::NonterminalParser* octalDigitSequenceNonterminalParser = GetNonterminal("OctalDigitSequence");
        octalDigitSequenceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CharEscapeRule>(this, &CharEscapeRule::PostOctalDigitSequence));
        cminor::parsing::NonterminalParser* hexDigit4NonterminalParser = GetNonterminal("HexDigit4");
        hexDigit4NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CharEscapeRule>(this, &CharEscapeRule::PostHexDigit4));
        cminor::parsing::NonterminalParser* hexDigit8NonterminalParser = GetNonterminal("HexDigit8");
        hexDigit8NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CharEscapeRule>(this, &CharEscapeRule::PostHexDigit8));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = utf32_char(context.fromHexDigitSequence);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = utf32_char(context.fromDecDigitSequence);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = utf32_char(context.fromOctalDigitSequence);
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = utf32_char(context.fromHexDigit4);
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = utf32_char(context.fromHexDigit8);
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = utf32_char('\a');
    }
    void A6Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = utf32_char('\b');
    }
    void A7Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = utf32_char('\f');
    }
    void A8Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = utf32_char('\n');
    }
    void A9Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = utf32_char('\r');
    }
    void A10Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = utf32_char('\t');
    }
    void A11Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = utf32_char('\v');
    }
    void A12Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = *matchBegin;
    }
    void PostHexDigitSequence(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromHexDigitSequence_value = std::move(stack.top());
            context.fromHexDigitSequence = *static_cast<cminor::parsing::ValueObject<uint64_t>*>(fromHexDigitSequence_value.get());
            stack.pop();
        }
    }
    void PostDecDigitSequence(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDecDigitSequence_value = std::move(stack.top());
            context.fromDecDigitSequence = *static_cast<cminor::parsing::ValueObject<uint64_t>*>(fromDecDigitSequence_value.get());
            stack.pop();
        }
    }
    void PostOctalDigitSequence(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromOctalDigitSequence_value = std::move(stack.top());
            context.fromOctalDigitSequence = *static_cast<cminor::parsing::ValueObject<uint64_t>*>(fromOctalDigitSequence_value.get());
            stack.pop();
        }
    }
    void PostHexDigit4(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromHexDigit4_value = std::move(stack.top());
            context.fromHexDigit4 = *static_cast<cminor::parsing::ValueObject<uint16_t>*>(fromHexDigit4_value.get());
            stack.pop();
        }
    }
    void PostHexDigit8(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromHexDigit8_value = std::move(stack.top());
            context.fromHexDigit8 = *static_cast<cminor::parsing::ValueObject<uint32_t>*>(fromHexDigit8_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromHexDigitSequence(), fromDecDigitSequence(), fromOctalDigitSequence(), fromHexDigit4(), fromHexDigit8() {}
        utf32_char value;
        uint64_t fromHexDigitSequence;
        uint64_t fromDecDigitSequence;
        uint64_t fromOctalDigitSequence;
        uint16_t fromHexDigit4;
        uint32_t fromHexDigit8;
    };
    std::stack<Context> contextStack;
    Context context;
};

class LiteralGrammar::DecDigitSequenceRule : public cminor::parsing::Rule
{
public:
    DecDigitSequenceRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("uint64_t");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<uint64_t>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DecDigitSequenceRule>(this, &DecDigitSequenceRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        std::stringstream s;
        s.str(std::string(matchBegin, matchEnd));
        s >> context.value;
    }
private:
    struct Context
    {
        Context(): value() {}
        uint64_t value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class LiteralGrammar::HexDigitSequenceRule : public cminor::parsing::Rule
{
public:
    HexDigitSequenceRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("uint64_t");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<uint64_t>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<HexDigitSequenceRule>(this, &HexDigitSequenceRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        std::stringstream s;
        s.str(std::string(matchBegin, matchEnd));
        s >> std::hex >> context.value;
    }
private:
    struct Context
    {
        Context(): value() {}
        uint64_t value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class LiteralGrammar::HexDigit4Rule : public cminor::parsing::Rule
{
public:
    HexDigit4Rule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("uint16_t");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<uint16_t>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<HexDigit4Rule>(this, &HexDigit4Rule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        std::stringstream s;
        s.str(std::string(matchBegin, matchEnd));
        s >> std::hex >> context.value;
    }
private:
    struct Context
    {
        Context(): value() {}
        uint16_t value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class LiteralGrammar::HexDigit8Rule : public cminor::parsing::Rule
{
public:
    HexDigit8Rule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("uint32_t");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<uint32_t>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<HexDigit8Rule>(this, &HexDigit8Rule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        std::stringstream s;
        s.str(std::string(matchBegin, matchEnd));
        s >> std::hex >> context.value;
    }
private:
    struct Context
    {
        Context(): value() {}
        uint32_t value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class LiteralGrammar::OctalDigitSequenceRule : public cminor::parsing::Rule
{
public:
    OctalDigitSequenceRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("uint64_t");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<uint64_t>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OctalDigitSequenceRule>(this, &OctalDigitSequenceRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = 8 * context.value + *matchBegin - '0';
    }
private:
    struct Context
    {
        Context(): value() {}
        uint64_t value;
    };
    std::stack<Context> contextStack;
    Context context;
};

void LiteralGrammar::GetReferencedGrammars()
{
}

void LiteralGrammar::CreateRules()
{
    AddRule(new LiteralRule("Literal", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::ActionParser("A0",
                                new cminor::parsing::NonterminalParser("BooleanLiteral", "BooleanLiteral", 0)),
                            new cminor::parsing::ActionParser("A1",
                                new cminor::parsing::NonterminalParser("FloatingLiteral", "FloatingLiteral", 0))),
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::NonterminalParser("IntegerLiteral", "IntegerLiteral", 0))),
                    new cminor::parsing::ActionParser("A3",
                        new cminor::parsing::NonterminalParser("CharLiteral", "CharLiteral", 0))),
                new cminor::parsing::ActionParser("A4",
                    new cminor::parsing::NonterminalParser("StringLiteral", "StringLiteral", 0))),
            new cminor::parsing::ActionParser("A5",
                new cminor::parsing::NonterminalParser("NullLiteral", "NullLiteral", 0)))));
    AddRule(new BooleanLiteralRule("BooleanLiteral", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::KeywordParser("true")),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::KeywordParser("false")))));
    AddRule(new FloatingLiteralRule("FloatingLiteral", GetScope(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::NonterminalParser("FloatingLiteralValue", "FloatingLiteralValue", 0)),
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::CharSetParser("fF")),
                    new cminor::parsing::ActionParser("A2",
                        new cminor::parsing::EmptyParser()))))));
    AddRule(new FloatingLiteralValueRule("FloatingLiteralValue", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::TokenParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::NonterminalParser("FractionalFloatingLiteral", "FractionalFloatingLiteral", 0),
                    new cminor::parsing::NonterminalParser("ExponentFloatingLiteral", "ExponentFloatingLiteral", 0))))));
    AddRule(new cminor::parsing::Rule("FractionalFloatingLiteral", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::NonterminalParser("s0", "DecDigitSequence", 0)),
                            new cminor::parsing::CharParser('.')),
                        new cminor::parsing::NonterminalParser("m", "DecDigitSequence", 0)),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser("ExponentPart", "ExponentPart", 0)))),
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser("s1", "DecDigitSequence", 0),
                    new cminor::parsing::CharParser('.'))))));
    AddRule(new cminor::parsing::Rule("ExponentFloatingLiteral", GetScope(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::NonterminalParser("DecDigitSequence", "DecDigitSequence", 0),
                new cminor::parsing::NonterminalParser("ExponentPart", "ExponentPart", 0)))));
    AddRule(new cminor::parsing::Rule("ExponentPart", GetScope(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharSetParser("eE"),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser("Sign", "Sign", 0))),
                new cminor::parsing::NonterminalParser("DecDigitSequence", "DecDigitSequence", 0)))));
    AddRule(new IntegerLiteralRule("IntegerLiteral", GetScope(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::NonterminalParser("IntegerLiteralValue", "IntegerLiteralValue", 0)),
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::CharSetParser("uU")),
                    new cminor::parsing::ActionParser("A2",
                        new cminor::parsing::EmptyParser()))))));
    AddRule(new IntegerLiteralValueRule("IntegerLiteralValue", GetScope(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::NonterminalParser("HexIntegerLiteral", "HexIntegerLiteral", 0)),
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("DecIntegerLiteral", "DecIntegerLiteral", 0))))));
    AddRule(new HexIntegerLiteralRule("HexIntegerLiteral", GetScope(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::StringParser("0x"),
                    new cminor::parsing::StringParser("0X")),
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::NonterminalParser("HexDigitSequence", "HexDigitSequence", 0))))));
    AddRule(new DecIntegerLiteralRule("DecIntegerLiteral", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::NonterminalParser("DecDigitSequence", "DecDigitSequence", 0))));
    AddRule(new CharLiteralRule("CharLiteral", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::CharParser('\''),
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::ActionParser("A1",
                                new cminor::parsing::PositiveParser(
                                    new cminor::parsing::CharSetParser("\'\\\r\n", true))),
                            new cminor::parsing::ActionParser("A2",
                                new cminor::parsing::NonterminalParser("CharEscape", "CharEscape", 0)))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('\'')))))));
    AddRule(new StringLiteralRule("StringLiteral", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('@'),
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::TokenParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::CharParser('\"'),
                                new cminor::parsing::ActionParser("A1",
                                    new cminor::parsing::KleeneStarParser(
                                        new cminor::parsing::CharSetParser("\"", true)))),
                            new cminor::parsing::CharParser('\"'))))),
            new cminor::parsing::ActionParser("A2",
                new cminor::parsing::TokenParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::CharParser('\"'),
                            new cminor::parsing::KleeneStarParser(
                                new cminor::parsing::AlternativeParser(
                                    new cminor::parsing::ActionParser("A3",
                                        new cminor::parsing::PositiveParser(
                                            new cminor::parsing::CharSetParser("\"\\\r\n", true))),
                                    new cminor::parsing::ActionParser("A4",
                                        new cminor::parsing::NonterminalParser("CharEscape", "CharEscape", 0))))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser('\"'))))))));
    AddRule(new NullLiteralRule("NullLiteral", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::KeywordParser("null"))));
    AddRule(new CharEscapeRule("CharEscape", GetScope(),
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
                                                                    new cminor::parsing::CharSetParser("xX"),
                                                                    new cminor::parsing::ActionParser("A0",
                                                                        new cminor::parsing::NonterminalParser("HexDigitSequence", "HexDigitSequence", 0))),
                                                                new cminor::parsing::SequenceParser(
                                                                    new cminor::parsing::CharSetParser("dD"),
                                                                    new cminor::parsing::ActionParser("A1",
                                                                        new cminor::parsing::NonterminalParser("DecDigitSequence", "DecDigitSequence", 0)))),
                                                            new cminor::parsing::ActionParser("A2",
                                                                new cminor::parsing::NonterminalParser("OctalDigitSequence", "OctalDigitSequence", 0))),
                                                        new cminor::parsing::SequenceParser(
                                                            new cminor::parsing::CharParser('u'),
                                                            new cminor::parsing::ActionParser("A3",
                                                                new cminor::parsing::NonterminalParser("HexDigit4", "HexDigit4", 0)))),
                                                    new cminor::parsing::SequenceParser(
                                                        new cminor::parsing::CharParser('U'),
                                                        new cminor::parsing::ActionParser("A4",
                                                            new cminor::parsing::NonterminalParser("HexDigit8", "HexDigit8", 0)))),
                                                new cminor::parsing::ActionParser("A5",
                                                    new cminor::parsing::CharParser('a'))),
                                            new cminor::parsing::ActionParser("A6",
                                                new cminor::parsing::CharParser('b'))),
                                        new cminor::parsing::ActionParser("A7",
                                            new cminor::parsing::CharParser('f'))),
                                    new cminor::parsing::ActionParser("A8",
                                        new cminor::parsing::CharParser('n'))),
                                new cminor::parsing::ActionParser("A9",
                                    new cminor::parsing::CharParser('r'))),
                            new cminor::parsing::ActionParser("A10",
                                new cminor::parsing::CharParser('t'))),
                        new cminor::parsing::ActionParser("A11",
                            new cminor::parsing::CharParser('v'))),
                    new cminor::parsing::ActionParser("A12",
                        new cminor::parsing::AnyCharParser()))))));
    AddRule(new DecDigitSequenceRule("DecDigitSequence", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::TokenParser(
                new cminor::parsing::PositiveParser(
                    new cminor::parsing::DigitParser())))));
    AddRule(new HexDigitSequenceRule("HexDigitSequence", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::TokenParser(
                new cminor::parsing::PositiveParser(
                    new cminor::parsing::HexDigitParser())))));
    AddRule(new HexDigit4Rule("HexDigit4", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::HexDigitParser(),
                            new cminor::parsing::HexDigitParser()),
                        new cminor::parsing::HexDigitParser()),
                    new cminor::parsing::HexDigitParser())))));
    AddRule(new HexDigit8Rule("HexDigit8", GetScope(),
        new cminor::parsing::ActionParser("A0",
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
    AddRule(new OctalDigitSequenceRule("OctalDigitSequence", GetScope(),
        new cminor::parsing::PositiveParser(
            new cminor::parsing::TokenParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::CharSetParser("0-7"))))));
    AddRule(new cminor::parsing::Rule("Sign", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::CharParser('+'),
            new cminor::parsing::CharParser('-'))));
}

} } // namespace cminor.parser
