#include "Literal.hpp"
#include <Cm.Parsing/Action.hpp>
#include <Cm.Parsing/Rule.hpp>
#include <Cm.Parsing/ParsingDomain.hpp>
#include <Cm.Parsing/Primitive.hpp>
#include <Cm.Parsing/Composite.hpp>
#include <Cm.Parsing/Nonterminal.hpp>
#include <Cm.Parsing/Exception.hpp>
#include <Cm.Parsing/StdLib.hpp>
#include <Cm.Parsing/XmlLog.hpp>
#include <sstream>

namespace cminor { namespace parser {

using namespace cminor::machine;
using namespace Cm::Parsing;

LiteralGrammar* LiteralGrammar::Create()
{
    return Create(new Cm::Parsing::ParsingDomain());
}

LiteralGrammar* LiteralGrammar::Create(Cm::Parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    LiteralGrammar* grammar(new LiteralGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

LiteralGrammar::LiteralGrammar(Cm::Parsing::ParsingDomain* parsingDomain_): Cm::Parsing::Grammar("LiteralGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

Node* LiteralGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName)
{
    Cm::Parsing::Scanner scanner(start, end, fileName, fileIndex, SkipRule());
    std::unique_ptr<Cm::Parsing::XmlLog> xmlLog;
    if (Log())
    {
        xmlLog.reset(new Cm::Parsing::XmlLog(*Log(), MaxLogLineLength()));
        scanner.SetLog(xmlLog.get());
        xmlLog->WriteBeginRule("parse");
    }
    Cm::Parsing::ObjectStack stack;
    Cm::Parsing::Match match = Cm::Parsing::Grammar::Parse(scanner, stack);
    Cm::Parsing::Span stop = scanner.GetSpan();
    if (Log())
    {
        xmlLog->WriteEndRule("parse");
    }
    if (!match.Hit() || !CC() && stop.Start() != int(end - start))
    {
        if (StartRule())
        {
            throw Cm::Parsing::ExpectationFailure(StartRule()->Info(), fileName, stop, start, end);
        }
        else
        {
            throw Cm::Parsing::ParsingException("grammar '" + Name() + "' has no start rule", fileName, scanner.GetSpan(), start, end);
        }
    }
    std::unique_ptr<Cm::Parsing::Object> value = std::move(stack.top());
    Node* result = *static_cast<Cm::Parsing::ValueObject<Node*>*>(value.get());
    stack.pop();
    return result;
}

class LiteralGrammar::LiteralRule : public Cm::Parsing::Rule
{
public:
    LiteralRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Node*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<LiteralRule>(this, &LiteralRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<LiteralRule>(this, &LiteralRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<LiteralRule>(this, &LiteralRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<LiteralRule>(this, &LiteralRule::A3Action));
        Cm::Parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<LiteralRule>(this, &LiteralRule::A4Action));
        Cm::Parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<LiteralRule>(this, &LiteralRule::A5Action));
        Cm::Parsing::NonterminalParser* booleanLiteralNonterminalParser = GetNonterminal("BooleanLiteral");
        booleanLiteralNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostBooleanLiteral));
        Cm::Parsing::NonterminalParser* floatingLiteralNonterminalParser = GetNonterminal("FloatingLiteral");
        floatingLiteralNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostFloatingLiteral));
        Cm::Parsing::NonterminalParser* integerLiteralNonterminalParser = GetNonterminal("IntegerLiteral");
        integerLiteralNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostIntegerLiteral));
        Cm::Parsing::NonterminalParser* charLiteralNonterminalParser = GetNonterminal("CharLiteral");
        charLiteralNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostCharLiteral));
        Cm::Parsing::NonterminalParser* stringLiteralNonterminalParser = GetNonterminal("StringLiteral");
        stringLiteralNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostStringLiteral));
        Cm::Parsing::NonterminalParser* nullLiteralNonterminalParser = GetNonterminal("NullLiteral");
        nullLiteralNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<LiteralRule>(this, &LiteralRule::PostNullLiteral));
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
    void PostBooleanLiteral(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromBooleanLiteral_value = std::move(stack.top());
            context.fromBooleanLiteral = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromBooleanLiteral_value.get());
            stack.pop();
        }
    }
    void PostFloatingLiteral(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromFloatingLiteral_value = std::move(stack.top());
            context.fromFloatingLiteral = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromFloatingLiteral_value.get());
            stack.pop();
        }
    }
    void PostIntegerLiteral(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromIntegerLiteral_value = std::move(stack.top());
            context.fromIntegerLiteral = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromIntegerLiteral_value.get());
            stack.pop();
        }
    }
    void PostCharLiteral(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromCharLiteral_value = std::move(stack.top());
            context.fromCharLiteral = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromCharLiteral_value.get());
            stack.pop();
        }
    }
    void PostStringLiteral(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromStringLiteral_value = std::move(stack.top());
            context.fromStringLiteral = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromStringLiteral_value.get());
            stack.pop();
        }
    }
    void PostNullLiteral(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromNullLiteral_value = std::move(stack.top());
            context.fromNullLiteral = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromNullLiteral_value.get());
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

class LiteralGrammar::BooleanLiteralRule : public Cm::Parsing::Rule
{
public:
    BooleanLiteralRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Node*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BooleanLiteralRule>(this, &BooleanLiteralRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BooleanLiteralRule>(this, &BooleanLiteralRule::A1Action));
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

class LiteralGrammar::FloatingLiteralRule : public Cm::Parsing::Rule
{
public:
    FloatingLiteralRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("Span", "s"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<FloatingLiteralRule>(this, &FloatingLiteralRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<FloatingLiteralRule>(this, &FloatingLiteralRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<FloatingLiteralRule>(this, &FloatingLiteralRule::A2Action));
        Cm::Parsing::NonterminalParser* floatingLiteralValueNonterminalParser = GetNonterminal("FloatingLiteralValue");
        floatingLiteralValueNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<FloatingLiteralRule>(this, &FloatingLiteralRule::PostFloatingLiteralValue));
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
    void PostFloatingLiteralValue(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromFloatingLiteralValue_value = std::move(stack.top());
            context.fromFloatingLiteralValue = *static_cast<Cm::Parsing::ValueObject<double>*>(fromFloatingLiteralValue_value.get());
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

class LiteralGrammar::FloatingLiteralValueRule : public Cm::Parsing::Rule
{
public:
    FloatingLiteralValueRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("double");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<double>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<FloatingLiteralValueRule>(this, &FloatingLiteralValueRule::A0Action));
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

class LiteralGrammar::IntegerLiteralRule : public Cm::Parsing::Rule
{
public:
    IntegerLiteralRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("Span", "s"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<IntegerLiteralRule>(this, &IntegerLiteralRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<IntegerLiteralRule>(this, &IntegerLiteralRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<IntegerLiteralRule>(this, &IntegerLiteralRule::A2Action));
        Cm::Parsing::NonterminalParser* integerLiteralValueNonterminalParser = GetNonterminal("IntegerLiteralValue");
        integerLiteralValueNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<IntegerLiteralRule>(this, &IntegerLiteralRule::PostIntegerLiteralValue));
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
    void PostIntegerLiteralValue(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromIntegerLiteralValue_value = std::move(stack.top());
            context.fromIntegerLiteralValue = *static_cast<Cm::Parsing::ValueObject<uint64_t>*>(fromIntegerLiteralValue_value.get());
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

class LiteralGrammar::IntegerLiteralValueRule : public Cm::Parsing::Rule
{
public:
    IntegerLiteralValueRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("uint64_t");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<uint64_t>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<IntegerLiteralValueRule>(this, &IntegerLiteralValueRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<IntegerLiteralValueRule>(this, &IntegerLiteralValueRule::A1Action));
        Cm::Parsing::NonterminalParser* hexIntegerLiteralNonterminalParser = GetNonterminal("HexIntegerLiteral");
        hexIntegerLiteralNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<IntegerLiteralValueRule>(this, &IntegerLiteralValueRule::PostHexIntegerLiteral));
        Cm::Parsing::NonterminalParser* decIntegerLiteralNonterminalParser = GetNonterminal("DecIntegerLiteral");
        decIntegerLiteralNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<IntegerLiteralValueRule>(this, &IntegerLiteralValueRule::PostDecIntegerLiteral));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromHexIntegerLiteral;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromDecIntegerLiteral;
    }
    void PostHexIntegerLiteral(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromHexIntegerLiteral_value = std::move(stack.top());
            context.fromHexIntegerLiteral = *static_cast<Cm::Parsing::ValueObject<uint64_t>*>(fromHexIntegerLiteral_value.get());
            stack.pop();
        }
    }
    void PostDecIntegerLiteral(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromDecIntegerLiteral_value = std::move(stack.top());
            context.fromDecIntegerLiteral = *static_cast<Cm::Parsing::ValueObject<uint64_t>*>(fromDecIntegerLiteral_value.get());
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

class LiteralGrammar::HexIntegerLiteralRule : public Cm::Parsing::Rule
{
public:
    HexIntegerLiteralRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("uint64_t");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<uint64_t>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<HexIntegerLiteralRule>(this, &HexIntegerLiteralRule::A0Action));
        Cm::Parsing::NonterminalParser* hexDigitSequenceNonterminalParser = GetNonterminal("HexDigitSequence");
        hexDigitSequenceNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<HexIntegerLiteralRule>(this, &HexIntegerLiteralRule::PostHexDigitSequence));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromHexDigitSequence;
    }
    void PostHexDigitSequence(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromHexDigitSequence_value = std::move(stack.top());
            context.fromHexDigitSequence = *static_cast<Cm::Parsing::ValueObject<uint64_t>*>(fromHexDigitSequence_value.get());
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

class LiteralGrammar::DecIntegerLiteralRule : public Cm::Parsing::Rule
{
public:
    DecIntegerLiteralRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("uint64_t");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<uint64_t>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<DecIntegerLiteralRule>(this, &DecIntegerLiteralRule::A0Action));
        Cm::Parsing::NonterminalParser* decDigitSequenceNonterminalParser = GetNonterminal("DecDigitSequence");
        decDigitSequenceNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<DecIntegerLiteralRule>(this, &DecIntegerLiteralRule::PostDecDigitSequence));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromDecDigitSequence;
    }
    void PostDecDigitSequence(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromDecDigitSequence_value = std::move(stack.top());
            context.fromDecDigitSequence = *static_cast<Cm::Parsing::ValueObject<uint64_t>*>(fromDecDigitSequence_value.get());
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

class LiteralGrammar::CharLiteralRule : public Cm::Parsing::Rule
{
public:
    CharLiteralRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("utf32_char", "litValue"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CharLiteralRule>(this, &CharLiteralRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CharLiteralRule>(this, &CharLiteralRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CharLiteralRule>(this, &CharLiteralRule::A2Action));
        Cm::Parsing::NonterminalParser* charEscapeNonterminalParser = GetNonterminal("CharEscape");
        charEscapeNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<CharLiteralRule>(this, &CharLiteralRule::PostCharEscape));
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
    void PostCharEscape(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromCharEscape_value = std::move(stack.top());
            context.fromCharEscape = *static_cast<Cm::Parsing::ValueObject<utf32_char>*>(fromCharEscape_value.get());
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

class LiteralGrammar::StringLiteralRule : public Cm::Parsing::Rule
{
public:
    StringLiteralRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("utf32_string", "s"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<StringLiteralRule>(this, &StringLiteralRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<StringLiteralRule>(this, &StringLiteralRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<StringLiteralRule>(this, &StringLiteralRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<StringLiteralRule>(this, &StringLiteralRule::A3Action));
        Cm::Parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<StringLiteralRule>(this, &StringLiteralRule::A4Action));
        Cm::Parsing::NonterminalParser* charEscapeNonterminalParser = GetNonterminal("CharEscape");
        charEscapeNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<StringLiteralRule>(this, &StringLiteralRule::PostCharEscape));
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
    void PostCharEscape(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromCharEscape_value = std::move(stack.top());
            context.fromCharEscape = *static_cast<Cm::Parsing::ValueObject<utf32_char>*>(fromCharEscape_value.get());
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

class LiteralGrammar::NullLiteralRule : public Cm::Parsing::Rule
{
public:
    NullLiteralRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Node*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<NullLiteralRule>(this, &NullLiteralRule::A0Action));
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

class LiteralGrammar::CharEscapeRule : public Cm::Parsing::Rule
{
public:
    CharEscapeRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("utf32_char");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<utf32_char>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A3Action));
        Cm::Parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A4Action));
        Cm::Parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A5Action));
        Cm::Parsing::ActionParser* a6ActionParser = GetAction("A6");
        a6ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A6Action));
        Cm::Parsing::ActionParser* a7ActionParser = GetAction("A7");
        a7ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A7Action));
        Cm::Parsing::ActionParser* a8ActionParser = GetAction("A8");
        a8ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A8Action));
        Cm::Parsing::ActionParser* a9ActionParser = GetAction("A9");
        a9ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A9Action));
        Cm::Parsing::ActionParser* a10ActionParser = GetAction("A10");
        a10ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A10Action));
        Cm::Parsing::ActionParser* a11ActionParser = GetAction("A11");
        a11ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A11Action));
        Cm::Parsing::ActionParser* a12ActionParser = GetAction("A12");
        a12ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CharEscapeRule>(this, &CharEscapeRule::A12Action));
        Cm::Parsing::NonterminalParser* hexDigitSequenceNonterminalParser = GetNonterminal("HexDigitSequence");
        hexDigitSequenceNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<CharEscapeRule>(this, &CharEscapeRule::PostHexDigitSequence));
        Cm::Parsing::NonterminalParser* decDigitSequenceNonterminalParser = GetNonterminal("DecDigitSequence");
        decDigitSequenceNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<CharEscapeRule>(this, &CharEscapeRule::PostDecDigitSequence));
        Cm::Parsing::NonterminalParser* octalDigitSequenceNonterminalParser = GetNonterminal("OctalDigitSequence");
        octalDigitSequenceNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<CharEscapeRule>(this, &CharEscapeRule::PostOctalDigitSequence));
        Cm::Parsing::NonterminalParser* hexDigit4NonterminalParser = GetNonterminal("HexDigit4");
        hexDigit4NonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<CharEscapeRule>(this, &CharEscapeRule::PostHexDigit4));
        Cm::Parsing::NonterminalParser* hexDigit8NonterminalParser = GetNonterminal("HexDigit8");
        hexDigit8NonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<CharEscapeRule>(this, &CharEscapeRule::PostHexDigit8));
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
    void PostHexDigitSequence(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromHexDigitSequence_value = std::move(stack.top());
            context.fromHexDigitSequence = *static_cast<Cm::Parsing::ValueObject<uint64_t>*>(fromHexDigitSequence_value.get());
            stack.pop();
        }
    }
    void PostDecDigitSequence(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromDecDigitSequence_value = std::move(stack.top());
            context.fromDecDigitSequence = *static_cast<Cm::Parsing::ValueObject<uint64_t>*>(fromDecDigitSequence_value.get());
            stack.pop();
        }
    }
    void PostOctalDigitSequence(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromOctalDigitSequence_value = std::move(stack.top());
            context.fromOctalDigitSequence = *static_cast<Cm::Parsing::ValueObject<uint64_t>*>(fromOctalDigitSequence_value.get());
            stack.pop();
        }
    }
    void PostHexDigit4(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromHexDigit4_value = std::move(stack.top());
            context.fromHexDigit4 = *static_cast<Cm::Parsing::ValueObject<uint16_t>*>(fromHexDigit4_value.get());
            stack.pop();
        }
    }
    void PostHexDigit8(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromHexDigit8_value = std::move(stack.top());
            context.fromHexDigit8 = *static_cast<Cm::Parsing::ValueObject<uint32_t>*>(fromHexDigit8_value.get());
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

class LiteralGrammar::DecDigitSequenceRule : public Cm::Parsing::Rule
{
public:
    DecDigitSequenceRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("uint64_t");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<uint64_t>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<DecDigitSequenceRule>(this, &DecDigitSequenceRule::A0Action));
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

class LiteralGrammar::HexDigitSequenceRule : public Cm::Parsing::Rule
{
public:
    HexDigitSequenceRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("uint64_t");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<uint64_t>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<HexDigitSequenceRule>(this, &HexDigitSequenceRule::A0Action));
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

class LiteralGrammar::HexDigit4Rule : public Cm::Parsing::Rule
{
public:
    HexDigit4Rule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("uint16_t");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<uint16_t>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<HexDigit4Rule>(this, &HexDigit4Rule::A0Action));
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

class LiteralGrammar::HexDigit8Rule : public Cm::Parsing::Rule
{
public:
    HexDigit8Rule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("uint32_t");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<uint32_t>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<HexDigit8Rule>(this, &HexDigit8Rule::A0Action));
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

class LiteralGrammar::OctalDigitSequenceRule : public Cm::Parsing::Rule
{
public:
    OctalDigitSequenceRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("uint64_t");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<uint64_t>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<OctalDigitSequenceRule>(this, &OctalDigitSequenceRule::A0Action));
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
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::AlternativeParser(
                new Cm::Parsing::AlternativeParser(
                    new Cm::Parsing::AlternativeParser(
                        new Cm::Parsing::AlternativeParser(
                            new Cm::Parsing::ActionParser("A0",
                                new Cm::Parsing::NonterminalParser("BooleanLiteral", "BooleanLiteral", 0)),
                            new Cm::Parsing::ActionParser("A1",
                                new Cm::Parsing::NonterminalParser("FloatingLiteral", "FloatingLiteral", 0))),
                        new Cm::Parsing::ActionParser("A2",
                            new Cm::Parsing::NonterminalParser("IntegerLiteral", "IntegerLiteral", 0))),
                    new Cm::Parsing::ActionParser("A3",
                        new Cm::Parsing::NonterminalParser("CharLiteral", "CharLiteral", 0))),
                new Cm::Parsing::ActionParser("A4",
                    new Cm::Parsing::NonterminalParser("StringLiteral", "StringLiteral", 0))),
            new Cm::Parsing::ActionParser("A5",
                new Cm::Parsing::NonterminalParser("NullLiteral", "NullLiteral", 0)))));
    AddRule(new BooleanLiteralRule("BooleanLiteral", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::ActionParser("A0",
                new Cm::Parsing::KeywordParser("true")),
            new Cm::Parsing::ActionParser("A1",
                new Cm::Parsing::KeywordParser("false")))));
    AddRule(new FloatingLiteralRule("FloatingLiteral", GetScope(),
        new Cm::Parsing::TokenParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::ActionParser("A0",
                    new Cm::Parsing::NonterminalParser("FloatingLiteralValue", "FloatingLiteralValue", 0)),
                new Cm::Parsing::AlternativeParser(
                    new Cm::Parsing::ActionParser("A1",
                        new Cm::Parsing::CharSetParser("fF")),
                    new Cm::Parsing::ActionParser("A2",
                        new Cm::Parsing::EmptyParser()))))));
    AddRule(new FloatingLiteralValueRule("FloatingLiteralValue", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::TokenParser(
                new Cm::Parsing::AlternativeParser(
                    new Cm::Parsing::NonterminalParser("FractionalFloatingLiteral", "FractionalFloatingLiteral", 0),
                    new Cm::Parsing::NonterminalParser("ExponentFloatingLiteral", "ExponentFloatingLiteral", 0))))));
    AddRule(new Cm::Parsing::Rule("FractionalFloatingLiteral", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::TokenParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::OptionalParser(
                                new Cm::Parsing::NonterminalParser("s0", "DecDigitSequence", 0)),
                            new Cm::Parsing::CharParser('.')),
                        new Cm::Parsing::NonterminalParser("m", "DecDigitSequence", 0)),
                    new Cm::Parsing::OptionalParser(
                        new Cm::Parsing::NonterminalParser("ExponentPart", "ExponentPart", 0)))),
            new Cm::Parsing::TokenParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::NonterminalParser("s1", "DecDigitSequence", 0),
                    new Cm::Parsing::CharParser('.'))))));
    AddRule(new Cm::Parsing::Rule("ExponentFloatingLiteral", GetScope(),
        new Cm::Parsing::TokenParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::NonterminalParser("DecDigitSequence", "DecDigitSequence", 0),
                new Cm::Parsing::NonterminalParser("ExponentPart", "ExponentPart", 0)))));
    AddRule(new Cm::Parsing::Rule("ExponentPart", GetScope(),
        new Cm::Parsing::TokenParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::CharSetParser("eE"),
                    new Cm::Parsing::OptionalParser(
                        new Cm::Parsing::NonterminalParser("Sign", "Sign", 0))),
                new Cm::Parsing::NonterminalParser("DecDigitSequence", "DecDigitSequence", 0)))));
    AddRule(new IntegerLiteralRule("IntegerLiteral", GetScope(),
        new Cm::Parsing::TokenParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::ActionParser("A0",
                    new Cm::Parsing::NonterminalParser("IntegerLiteralValue", "IntegerLiteralValue", 0)),
                new Cm::Parsing::AlternativeParser(
                    new Cm::Parsing::ActionParser("A1",
                        new Cm::Parsing::CharSetParser("uU")),
                    new Cm::Parsing::ActionParser("A2",
                        new Cm::Parsing::EmptyParser()))))));
    AddRule(new IntegerLiteralValueRule("IntegerLiteralValue", GetScope(),
        new Cm::Parsing::TokenParser(
            new Cm::Parsing::AlternativeParser(
                new Cm::Parsing::ActionParser("A0",
                    new Cm::Parsing::NonterminalParser("HexIntegerLiteral", "HexIntegerLiteral", 0)),
                new Cm::Parsing::ActionParser("A1",
                    new Cm::Parsing::NonterminalParser("DecIntegerLiteral", "DecIntegerLiteral", 0))))));
    AddRule(new HexIntegerLiteralRule("HexIntegerLiteral", GetScope(),
        new Cm::Parsing::TokenParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::AlternativeParser(
                    new Cm::Parsing::StringParser("0x"),
                    new Cm::Parsing::StringParser("0X")),
                new Cm::Parsing::ActionParser("A0",
                    new Cm::Parsing::NonterminalParser("HexDigitSequence", "HexDigitSequence", 0))))));
    AddRule(new DecIntegerLiteralRule("DecIntegerLiteral", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::NonterminalParser("DecDigitSequence", "DecDigitSequence", 0))));
    AddRule(new CharLiteralRule("CharLiteral", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::TokenParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::CharParser('\''),
                        new Cm::Parsing::AlternativeParser(
                            new Cm::Parsing::ActionParser("A1",
                                new Cm::Parsing::PositiveParser(
                                    new Cm::Parsing::CharSetParser("\'\\r\n", true))),
                            new Cm::Parsing::ActionParser("A2",
                                new Cm::Parsing::NonterminalParser("CharEscape", "CharEscape", 0)))),
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::CharParser('\'')))))));
    AddRule(new StringLiteralRule("StringLiteral", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::CharParser('@'),
                new Cm::Parsing::ActionParser("A0",
                    new Cm::Parsing::TokenParser(
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::SequenceParser(
                                new Cm::Parsing::CharParser('\"'),
                                new Cm::Parsing::ActionParser("A1",
                                    new Cm::Parsing::KleeneStarParser(
                                        new Cm::Parsing::CharSetParser("\"", true)))),
                            new Cm::Parsing::CharParser('\"'))))),
            new Cm::Parsing::ActionParser("A2",
                new Cm::Parsing::TokenParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::CharParser('\"'),
                            new Cm::Parsing::KleeneStarParser(
                                new Cm::Parsing::AlternativeParser(
                                    new Cm::Parsing::ActionParser("A3",
                                        new Cm::Parsing::PositiveParser(
                                            new Cm::Parsing::CharSetParser("\"\\\r\n", true))),
                                    new Cm::Parsing::ActionParser("A4",
                                        new Cm::Parsing::NonterminalParser("CharEscape", "CharEscape", 0))))),
                        new Cm::Parsing::ExpectationParser(
                            new Cm::Parsing::CharParser('\"'))))))));
    AddRule(new NullLiteralRule("NullLiteral", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::KeywordParser("null"))));
    AddRule(new CharEscapeRule("CharEscape", GetScope(),
        new Cm::Parsing::TokenParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::CharParser('\\'),
                new Cm::Parsing::AlternativeParser(
                    new Cm::Parsing::AlternativeParser(
                        new Cm::Parsing::AlternativeParser(
                            new Cm::Parsing::AlternativeParser(
                                new Cm::Parsing::AlternativeParser(
                                    new Cm::Parsing::AlternativeParser(
                                        new Cm::Parsing::AlternativeParser(
                                            new Cm::Parsing::AlternativeParser(
                                                new Cm::Parsing::AlternativeParser(
                                                    new Cm::Parsing::AlternativeParser(
                                                        new Cm::Parsing::AlternativeParser(
                                                            new Cm::Parsing::AlternativeParser(
                                                                new Cm::Parsing::SequenceParser(
                                                                    new Cm::Parsing::CharSetParser("xX"),
                                                                    new Cm::Parsing::ActionParser("A0",
                                                                        new Cm::Parsing::NonterminalParser("HexDigitSequence", "HexDigitSequence", 0))),
                                                                new Cm::Parsing::SequenceParser(
                                                                    new Cm::Parsing::CharSetParser("dD"),
                                                                    new Cm::Parsing::ActionParser("A1",
                                                                        new Cm::Parsing::NonterminalParser("DecDigitSequence", "DecDigitSequence", 0)))),
                                                            new Cm::Parsing::ActionParser("A2",
                                                                new Cm::Parsing::NonterminalParser("OctalDigitSequence", "OctalDigitSequence", 0))),
                                                        new Cm::Parsing::SequenceParser(
                                                            new Cm::Parsing::CharParser('u'),
                                                            new Cm::Parsing::ActionParser("A3",
                                                                new Cm::Parsing::NonterminalParser("HexDigit4", "HexDigit4", 0)))),
                                                    new Cm::Parsing::SequenceParser(
                                                        new Cm::Parsing::CharParser('U'),
                                                        new Cm::Parsing::ActionParser("A4",
                                                            new Cm::Parsing::NonterminalParser("HexDigit8", "HexDigit8", 0)))),
                                                new Cm::Parsing::ActionParser("A5",
                                                    new Cm::Parsing::CharParser('a'))),
                                            new Cm::Parsing::ActionParser("A6",
                                                new Cm::Parsing::CharParser('b'))),
                                        new Cm::Parsing::ActionParser("A7",
                                            new Cm::Parsing::CharParser('f'))),
                                    new Cm::Parsing::ActionParser("A8",
                                        new Cm::Parsing::CharParser('n'))),
                                new Cm::Parsing::ActionParser("A9",
                                    new Cm::Parsing::CharParser('r'))),
                            new Cm::Parsing::ActionParser("A10",
                                new Cm::Parsing::CharParser('t'))),
                        new Cm::Parsing::ActionParser("A11",
                            new Cm::Parsing::CharParser('v'))),
                    new Cm::Parsing::ActionParser("A12",
                        new Cm::Parsing::AnyCharParser()))))));
    AddRule(new DecDigitSequenceRule("DecDigitSequence", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::TokenParser(
                new Cm::Parsing::PositiveParser(
                    new Cm::Parsing::DigitParser())))));
    AddRule(new HexDigitSequenceRule("HexDigitSequence", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::TokenParser(
                new Cm::Parsing::PositiveParser(
                    new Cm::Parsing::HexDigitParser())))));
    AddRule(new HexDigit4Rule("HexDigit4", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::TokenParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::HexDigitParser(),
                            new Cm::Parsing::HexDigitParser()),
                        new Cm::Parsing::HexDigitParser()),
                    new Cm::Parsing::HexDigitParser())))));
    AddRule(new HexDigit8Rule("HexDigit8", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::TokenParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::SequenceParser(
                                new Cm::Parsing::SequenceParser(
                                    new Cm::Parsing::SequenceParser(
                                        new Cm::Parsing::SequenceParser(
                                            new Cm::Parsing::HexDigitParser(),
                                            new Cm::Parsing::HexDigitParser()),
                                        new Cm::Parsing::HexDigitParser()),
                                    new Cm::Parsing::HexDigitParser()),
                                new Cm::Parsing::HexDigitParser()),
                            new Cm::Parsing::HexDigitParser()),
                        new Cm::Parsing::HexDigitParser()),
                    new Cm::Parsing::HexDigitParser())))));
    AddRule(new OctalDigitSequenceRule("OctalDigitSequence", GetScope(),
        new Cm::Parsing::PositiveParser(
            new Cm::Parsing::TokenParser(
                new Cm::Parsing::ActionParser("A0",
                    new Cm::Parsing::CharSetParser("0-7"))))));
    AddRule(new Cm::Parsing::Rule("Sign", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::CharParser('+'),
            new Cm::Parsing::CharParser('-'))));
}

} } // namespace cminor.parser
