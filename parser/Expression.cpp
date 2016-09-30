#include "Expression.hpp"
#include <Cm.Parsing/Action.hpp>
#include <Cm.Parsing/Rule.hpp>
#include <Cm.Parsing/ParsingDomain.hpp>
#include <Cm.Parsing/Primitive.hpp>
#include <Cm.Parsing/Composite.hpp>
#include <Cm.Parsing/Nonterminal.hpp>
#include <Cm.Parsing/Exception.hpp>
#include <Cm.Parsing/StdLib.hpp>
#include <Cm.Parsing/XmlLog.hpp>
#include <cminor/parser/Literal.hpp>
#include <cminor/parser/BasicType.hpp>
#include <cminor/parser/Identifier.hpp>
#include <cminor/parser/Operator.hpp>

namespace cminor { namespace parser {

using namespace Cm::Parsing;

ExpressionGrammar* ExpressionGrammar::Create()
{
    return Create(new Cm::Parsing::ParsingDomain());
}

ExpressionGrammar* ExpressionGrammar::Create(Cm::Parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    ExpressionGrammar* grammar(new ExpressionGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

ExpressionGrammar::ExpressionGrammar(Cm::Parsing::ParsingDomain* parsingDomain_): Cm::Parsing::Grammar("ExpressionGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

Node* ExpressionGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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
    stack.push(std::unique_ptr<Cm::Parsing::Object>(new ValueObject<ParsingContext*>(ctx)));
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

class ExpressionGrammar::ExpressionRule : public Cm::Parsing::Rule
{
public:
    ExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ExpressionRule>(this, &ExpressionRule::A0Action));
        Cm::Parsing::NonterminalParser* disjunctionNonterminalParser = GetNonterminal("Disjunction");
        disjunctionNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ExpressionRule>(this, &ExpressionRule::PreDisjunction));
        disjunctionNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ExpressionRule>(this, &ExpressionRule::PostDisjunction));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromDisjunction;
    }
    void PreDisjunction(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostDisjunction(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromDisjunction_value = std::move(stack.top());
            context.fromDisjunction = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromDisjunction_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromDisjunction() {}
        ParsingContext* ctx;
        Node* value;
        Node* fromDisjunction;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::DisjunctionRule : public Cm::Parsing::Rule
{
public:
    DisjunctionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<DisjunctionRule>(this, &DisjunctionRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<DisjunctionRule>(this, &DisjunctionRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<DisjunctionRule>(this, &DisjunctionRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<DisjunctionRule>(this, &DisjunctionRule::A3Action));
        Cm::Parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<DisjunctionRule>(this, &DisjunctionRule::Preleft));
        leftNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<DisjunctionRule>(this, &DisjunctionRule::Postleft));
        Cm::Parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<DisjunctionRule>(this, &DisjunctionRule::Preright));
        rightNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<DisjunctionRule>(this, &DisjunctionRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.expr.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.expr.reset(context.fromleft);
        context.s = span;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (context.ctx->ParsingLvalue() || context.ctx->ParsingSimpleStatement() && !context.ctx->ParsingArguments()) pass = false;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s.SetEnd(span.End());
        context.expr.reset(new DisjunctionNode(context.s, context.expr.release(), context.fromright));
    }
    void Preleft(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), expr(), s(), fromleft(), fromright() {}
        ParsingContext* ctx;
        Node* value;
        std::unique_ptr<Node> expr;
        Span s;
        Node* fromleft;
        Node* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::ConjunctionRule : public Cm::Parsing::Rule
{
public:
    ConjunctionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ConjunctionRule>(this, &ConjunctionRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ConjunctionRule>(this, &ConjunctionRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ConjunctionRule>(this, &ConjunctionRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ConjunctionRule>(this, &ConjunctionRule::A3Action));
        Cm::Parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ConjunctionRule>(this, &ConjunctionRule::Preleft));
        leftNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ConjunctionRule>(this, &ConjunctionRule::Postleft));
        Cm::Parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ConjunctionRule>(this, &ConjunctionRule::Preright));
        rightNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ConjunctionRule>(this, &ConjunctionRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.expr.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.expr.reset(context.fromleft);
        context.s = span;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (context.ctx->ParsingLvalue() || context.ctx->ParsingSimpleStatement() && !context.ctx->ParsingArguments()) pass = false;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s.SetEnd(span.End());
        context.expr.reset(new ConjunctionNode(context.s, context.expr.release(), context.fromright));
    }
    void Preleft(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), expr(), s(), fromleft(), fromright() {}
        ParsingContext* ctx;
        Node* value;
        std::unique_ptr<Node> expr;
        Span s;
        Node* fromleft;
        Node* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::BitOrRule : public Cm::Parsing::Rule
{
public:
    BitOrRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BitOrRule>(this, &BitOrRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BitOrRule>(this, &BitOrRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BitOrRule>(this, &BitOrRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BitOrRule>(this, &BitOrRule::A3Action));
        Cm::Parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<BitOrRule>(this, &BitOrRule::Preleft));
        leftNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<BitOrRule>(this, &BitOrRule::Postleft));
        Cm::Parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<BitOrRule>(this, &BitOrRule::Preright));
        rightNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<BitOrRule>(this, &BitOrRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.expr.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.expr.reset(context.fromleft);
        context.s = span;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (context.ctx->ParsingLvalue() || context.ctx->ParsingSimpleStatement() && !context.ctx->ParsingArguments()) pass = false;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s.SetEnd(span.End());
        context.expr.reset(new BitOrNode(context.s, context.expr.release(), context.fromright));
    }
    void Preleft(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), expr(), s(), fromleft(), fromright() {}
        ParsingContext* ctx;
        Node* value;
        std::unique_ptr<Node> expr;
        Span s;
        Node* fromleft;
        Node* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::BitXorRule : public Cm::Parsing::Rule
{
public:
    BitXorRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BitXorRule>(this, &BitXorRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BitXorRule>(this, &BitXorRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BitXorRule>(this, &BitXorRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BitXorRule>(this, &BitXorRule::A3Action));
        Cm::Parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<BitXorRule>(this, &BitXorRule::Preleft));
        leftNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<BitXorRule>(this, &BitXorRule::Postleft));
        Cm::Parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<BitXorRule>(this, &BitXorRule::Preright));
        rightNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<BitXorRule>(this, &BitXorRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.expr.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.expr.reset(context.fromleft);
        context.s = span;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (context.ctx->ParsingLvalue() || context.ctx->ParsingSimpleStatement() && !context.ctx->ParsingArguments()) pass = false;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s.SetEnd(span.End());
        context.expr.reset(new BitXorNode(context.s, context.expr.release(), context.fromright));
    }
    void Preleft(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), expr(), s(), fromleft(), fromright() {}
        ParsingContext* ctx;
        Node* value;
        std::unique_ptr<Node> expr;
        Span s;
        Node* fromleft;
        Node* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::BitAndRule : public Cm::Parsing::Rule
{
public:
    BitAndRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BitAndRule>(this, &BitAndRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BitAndRule>(this, &BitAndRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BitAndRule>(this, &BitAndRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BitAndRule>(this, &BitAndRule::A3Action));
        Cm::Parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<BitAndRule>(this, &BitAndRule::Preleft));
        leftNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<BitAndRule>(this, &BitAndRule::Postleft));
        Cm::Parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<BitAndRule>(this, &BitAndRule::Preright));
        rightNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<BitAndRule>(this, &BitAndRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.expr.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.expr.reset(context.fromleft);
        context.s = span;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (context.ctx->ParsingLvalue() || context.ctx->ParsingSimpleStatement() && !context.ctx->ParsingArguments()) pass = false;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s.SetEnd(span.End());
        context.expr.reset(new BitAndNode(context.s, context.expr.release(), context.fromright));
    }
    void Preleft(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), expr(), s(), fromleft(), fromright() {}
        ParsingContext* ctx;
        Node* value;
        std::unique_ptr<Node> expr;
        Span s;
        Node* fromleft;
        Node* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::EqualityRule : public Cm::Parsing::Rule
{
public:
    EqualityRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
        AddLocalVariable(AttrOrVariable("Operator", "op"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<EqualityRule>(this, &EqualityRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<EqualityRule>(this, &EqualityRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<EqualityRule>(this, &EqualityRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<EqualityRule>(this, &EqualityRule::A3Action));
        Cm::Parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<EqualityRule>(this, &EqualityRule::A4Action));
        Cm::Parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<EqualityRule>(this, &EqualityRule::Preleft));
        leftNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<EqualityRule>(this, &EqualityRule::Postleft));
        Cm::Parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<EqualityRule>(this, &EqualityRule::Preright));
        rightNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<EqualityRule>(this, &EqualityRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.expr.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.expr.reset(context.fromleft);
        context.s = span;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (context.ctx->ParsingLvalue() || context.ctx->ParsingSimpleStatement() && !context.ctx->ParsingArguments()) pass = false;
        else context.op = Operator::eq;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (context.ctx->ParsingLvalue() || context.ctx->ParsingSimpleStatement() && !context.ctx->ParsingArguments()) pass = false;
        else context.op = Operator::neq;
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s.SetEnd(span.End());
        switch (context.op)
        {
            case Operator::eq: context.expr.reset(new EqualNode(context.s, context.expr.release(), context.fromright));
            break;
            case Operator::neq: context.expr.reset(new NotEqualNode(context.s, context.expr.release(), context.fromright));
            break;
        }
    }
    void Preleft(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), expr(), s(), op(), fromleft(), fromright() {}
        ParsingContext* ctx;
        Node* value;
        std::unique_ptr<Node> expr;
        Span s;
        Operator op;
        Node* fromleft;
        Node* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::RelationalRule : public Cm::Parsing::Rule
{
public:
    RelationalRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
        AddLocalVariable(AttrOrVariable("Operator", "op"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<RelationalRule>(this, &RelationalRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<RelationalRule>(this, &RelationalRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<RelationalRule>(this, &RelationalRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<RelationalRule>(this, &RelationalRule::A3Action));
        Cm::Parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<RelationalRule>(this, &RelationalRule::A4Action));
        Cm::Parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<RelationalRule>(this, &RelationalRule::A5Action));
        Cm::Parsing::ActionParser* a6ActionParser = GetAction("A6");
        a6ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<RelationalRule>(this, &RelationalRule::A6Action));
        Cm::Parsing::ActionParser* a7ActionParser = GetAction("A7");
        a7ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<RelationalRule>(this, &RelationalRule::A7Action));
        Cm::Parsing::ActionParser* a8ActionParser = GetAction("A8");
        a8ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<RelationalRule>(this, &RelationalRule::A8Action));
        Cm::Parsing::ActionParser* a9ActionParser = GetAction("A9");
        a9ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<RelationalRule>(this, &RelationalRule::A9Action));
        Cm::Parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<RelationalRule>(this, &RelationalRule::Preleft));
        leftNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<RelationalRule>(this, &RelationalRule::Postleft));
        Cm::Parsing::NonterminalParser* isTypeNonterminalParser = GetNonterminal("isType");
        isTypeNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<RelationalRule>(this, &RelationalRule::PreisType));
        isTypeNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<RelationalRule>(this, &RelationalRule::PostisType));
        Cm::Parsing::NonterminalParser* asTypeNonterminalParser = GetNonterminal("asType");
        asTypeNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<RelationalRule>(this, &RelationalRule::PreasType));
        asTypeNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<RelationalRule>(this, &RelationalRule::PostasType));
        Cm::Parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<RelationalRule>(this, &RelationalRule::Preright));
        rightNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<RelationalRule>(this, &RelationalRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.expr.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.expr.reset(context.fromleft);
        context.s = span;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (context.ctx->ParsingLvalue() || context.ctx->ParsingSimpleStatement() && !context.ctx->ParsingArguments()) pass = false;
        else
        {
            context.op = Operator::lessOrEq;
            context.ctx->PushParsingIsOrAs(false);
        }
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (context.ctx->ParsingLvalue() || context.ctx->ParsingSimpleStatement() && !context.ctx->ParsingArguments()) pass = false;
        else
        {
            context.op = Operator::greaterOrEq;
            context.ctx->PushParsingIsOrAs(false);
        }
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (context.ctx->ParsingLvalue() || context.ctx->ParsingSimpleStatement() && !context.ctx->ParsingArguments()) pass = false;
        else
        {
            context.op = Operator::less;
            context.ctx->PushParsingIsOrAs(false);
        }
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (context.ctx->ParsingLvalue() || context.ctx->ParsingSimpleStatement() && !context.ctx->ParsingArguments()) pass = false;
        else
        {
            context.op = Operator::greater;
            context.ctx->PushParsingIsOrAs(false);
        }
    }
    void A6Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (context.ctx->ParsingLvalue() || context.ctx->ParsingSimpleStatement() && !context.ctx->ParsingArguments()) pass = false;
        else
        {
            context.op = Operator::is;
            context.ctx->PushParsingIsOrAs(true);
        }
    }
    void A7Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (context.ctx->ParsingLvalue() || context.ctx->ParsingSimpleStatement() && !context.ctx->ParsingArguments()) pass = false;
        else
        {
            context.op = Operator::as;
            context.ctx->PushParsingIsOrAs(true);
        }
    }
    void A8Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ctx->PopParsingIsOrAs();
        context.s.SetEnd(span.End());
        switch (context.op)
        {
            case Operator::lessOrEq: context.expr.reset(new LessOrEqualNode(context.s, context.expr.release(), context.fromright));
            break;
            case Operator::greaterOrEq: context.expr.reset(new GreaterOrEqualNode(context.s, context.expr.release(), context.fromright));
            break;
            case Operator::less: context.expr.reset(new LessNode(context.s, context.expr.release(), context.fromright));
            break;
            case Operator::greater: context.expr.reset(new GreaterNode(context.s, context.expr.release(), context.fromright));
            break;
            case Operator::is: context.expr.reset(new IsNode(context.s, context.expr.release(), context.fromisType));
            break;
            case Operator::as: context.expr.reset(new AsNode(context.s, context.expr.release(), context.fromasType));
            break;
        }
    }
    void A9Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (!context.ctx->ParsingIsOrAs()) pass = false;
    }
    void Preleft(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void PreisType(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostisType(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromisType_value = std::move(stack.top());
            context.fromisType = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromisType_value.get());
            stack.pop();
        }
    }
    void PreasType(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostasType(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromasType_value = std::move(stack.top());
            context.fromasType = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromasType_value.get());
            stack.pop();
        }
    }
    void Preright(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), expr(), s(), op(), fromleft(), fromisType(), fromasType(), fromright() {}
        ParsingContext* ctx;
        Node* value;
        std::unique_ptr<Node> expr;
        Span s;
        Operator op;
        Node* fromleft;
        Node* fromisType;
        Node* fromasType;
        Node* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::ShiftRule : public Cm::Parsing::Rule
{
public:
    ShiftRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
        AddLocalVariable(AttrOrVariable("Operator", "op"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ShiftRule>(this, &ShiftRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ShiftRule>(this, &ShiftRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ShiftRule>(this, &ShiftRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ShiftRule>(this, &ShiftRule::A3Action));
        Cm::Parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ShiftRule>(this, &ShiftRule::A4Action));
        Cm::Parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ShiftRule>(this, &ShiftRule::A5Action));
        a5ActionParser->SetFailureAction(new Cm::Parsing::MemberFailureAction<ShiftRule>(this, &ShiftRule::A5ActionFail));
        Cm::Parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ShiftRule>(this, &ShiftRule::Preleft));
        leftNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ShiftRule>(this, &ShiftRule::Postleft));
        Cm::Parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ShiftRule>(this, &ShiftRule::Preright));
        rightNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ShiftRule>(this, &ShiftRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.expr.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.expr.reset(context.fromleft);
        context.s = span;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (context.ctx->ParsingLvalue()) pass = false;
        else context.op = Operator::shiftLeft;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (context.ctx->ParsingLvalue()) pass = false;
        else context.op = Operator::shiftRight;
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ctx->BeginParsingArguments();
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s.SetEnd(span.End());
        switch (context.op)
        {
            case Operator::shiftLeft: context.expr.reset(new ShiftLeftNode(context.s, context.expr.release(), context.fromright));
            break;
            case Operator::shiftRight: context.expr.reset(new ShiftRightNode(context.s, context.expr.release(), context.fromright));
            break;
        }
        context.ctx->EndParsingArguments();
    }
    void A5ActionFail()
    {
        context.ctx->EndParsingArguments();
    }
    void Preleft(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), expr(), s(), op(), fromleft(), fromright() {}
        ParsingContext* ctx;
        Node* value;
        std::unique_ptr<Node> expr;
        Span s;
        Operator op;
        Node* fromleft;
        Node* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::AdditiveRule : public Cm::Parsing::Rule
{
public:
    AdditiveRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
        AddLocalVariable(AttrOrVariable("Operator", "op"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<AdditiveRule>(this, &AdditiveRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<AdditiveRule>(this, &AdditiveRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<AdditiveRule>(this, &AdditiveRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<AdditiveRule>(this, &AdditiveRule::A3Action));
        Cm::Parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<AdditiveRule>(this, &AdditiveRule::A4Action));
        Cm::Parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<AdditiveRule>(this, &AdditiveRule::Preleft));
        leftNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<AdditiveRule>(this, &AdditiveRule::Postleft));
        Cm::Parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<AdditiveRule>(this, &AdditiveRule::Preright));
        rightNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<AdditiveRule>(this, &AdditiveRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.expr.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.expr.reset(context.fromleft);
        context.s = span;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (context.ctx->ParsingLvalue() || context.ctx->ParsingSimpleStatement() && !context.ctx->ParsingArguments()) pass = false;
        else context.op = Operator::plus;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (context.ctx->ParsingLvalue() || context.ctx->ParsingSimpleStatement() && !context.ctx->ParsingArguments()) pass = false;
        else context.op = Operator::minus;
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s.SetEnd(span.End());
        switch (context.op)
        {
            case Operator::plus: context.expr.reset(new AddNode(context.s, context.expr.release(), context.fromright));
            break;
            case Operator::minus: context.expr.reset(new SubNode(context.s, context.expr.release(), context.fromright));
            break;
        }
    }
    void Preleft(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), expr(), s(), op(), fromleft(), fromright() {}
        ParsingContext* ctx;
        Node* value;
        std::unique_ptr<Node> expr;
        Span s;
        Operator op;
        Node* fromleft;
        Node* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::MultiplicativeRule : public Cm::Parsing::Rule
{
public:
    MultiplicativeRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
        AddLocalVariable(AttrOrVariable("Operator", "op"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<MultiplicativeRule>(this, &MultiplicativeRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<MultiplicativeRule>(this, &MultiplicativeRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<MultiplicativeRule>(this, &MultiplicativeRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<MultiplicativeRule>(this, &MultiplicativeRule::A3Action));
        Cm::Parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<MultiplicativeRule>(this, &MultiplicativeRule::A4Action));
        Cm::Parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<MultiplicativeRule>(this, &MultiplicativeRule::A5Action));
        Cm::Parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<MultiplicativeRule>(this, &MultiplicativeRule::Preleft));
        leftNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<MultiplicativeRule>(this, &MultiplicativeRule::Postleft));
        Cm::Parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<MultiplicativeRule>(this, &MultiplicativeRule::Preright));
        rightNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<MultiplicativeRule>(this, &MultiplicativeRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.expr.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.expr.reset(context.fromleft);
        context.s = span;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (context.ctx->ParsingLvalue() || context.ctx->ParsingSimpleStatement() && !context.ctx->ParsingArguments()) pass = false;
        else context.op = Operator::mul;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (context.ctx->ParsingLvalue() || context.ctx->ParsingSimpleStatement() && !context.ctx->ParsingArguments()) pass = false;
        else context.op = Operator::div;
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (context.ctx->ParsingLvalue() || context.ctx->ParsingSimpleStatement() && !context.ctx->ParsingArguments()) pass = false;
        else context.op = Operator::rem;
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s.SetEnd(span.End());
        switch (context.op)
        {
            case Operator::mul: context.expr.reset(new MulNode(context.s, context.expr.release(), context.fromright));
            break;
            case Operator::div: context.expr.reset(new DivNode(context.s, context.expr.release(), context.fromright));
            break;
            case Operator::rem: context.expr.reset(new RemNode(context.s, context.expr.release(), context.fromright));
            break;
        }
    }
    void Preleft(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), expr(), s(), op(), fromleft(), fromright() {}
        ParsingContext* ctx;
        Node* value;
        std::unique_ptr<Node> expr;
        Span s;
        Operator op;
        Node* fromleft;
        Node* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::PrefixRule : public Cm::Parsing::Rule
{
public:
    PrefixRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("Span", "s"));
        AddLocalVariable(AttrOrVariable("Operator", "op"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<PrefixRule>(this, &PrefixRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<PrefixRule>(this, &PrefixRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<PrefixRule>(this, &PrefixRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<PrefixRule>(this, &PrefixRule::A3Action));
        Cm::Parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<PrefixRule>(this, &PrefixRule::A4Action));
        Cm::Parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<PrefixRule>(this, &PrefixRule::A5Action));
        Cm::Parsing::NonterminalParser* prefixNonterminalParser = GetNonterminal("prefix");
        prefixNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<PrefixRule>(this, &PrefixRule::Preprefix));
        prefixNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<PrefixRule>(this, &PrefixRule::Postprefix));
        Cm::Parsing::NonterminalParser* postfixNonterminalParser = GetNonterminal("Postfix");
        postfixNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<PrefixRule>(this, &PrefixRule::PrePostfix));
        postfixNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<PrefixRule>(this, &PrefixRule::PostPostfix));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s = span;
        if (context.ctx->ParsingSimpleStatement() && !context.ctx->ParsingArguments()) pass = false;
        else context.op = Operator::minus;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s = span;
        if (context.ctx->ParsingSimpleStatement() && !context.ctx->ParsingArguments()) pass = false;
        else context.op = Operator::plus;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s = span;
        if (context.ctx->ParsingSimpleStatement() && !context.ctx->ParsingArguments()) pass = false;
        else context.op = Operator::not_;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s = span;
        if (context.ctx->ParsingSimpleStatement() && !context.ctx->ParsingArguments()) pass = false;
        else context.op = Operator::complement;
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s.SetEnd(span.End());
        switch (context.op)
        {
            case Operator::minus: context.value = new UnaryMinusNode(context.s, context.fromprefix);
            break;
            case Operator::plus: context.value = new UnaryPlusNode(context.s, context.fromprefix);
            break;
            case Operator::not_: context.value = new NotNode(context.s, context.fromprefix);
            break;
            case Operator::complement: context.value = new ComplementNode(context.s, context.fromprefix);
            break;
        }
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromPostfix;
    }
    void Preprefix(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postprefix(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromprefix_value = std::move(stack.top());
            context.fromprefix = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromprefix_value.get());
            stack.pop();
        }
    }
    void PrePostfix(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostPostfix(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromPostfix_value = std::move(stack.top());
            context.fromPostfix = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromPostfix_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), s(), op(), fromprefix(), fromPostfix() {}
        ParsingContext* ctx;
        Node* value;
        Span s;
        Operator op;
        Node* fromprefix;
        Node* fromPostfix;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::PostfixRule : public Cm::Parsing::Rule
{
public:
    PostfixRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<PostfixRule>(this, &PostfixRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<PostfixRule>(this, &PostfixRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<PostfixRule>(this, &PostfixRule::A2Action));
        Cm::Parsing::NonterminalParser* primaryNonterminalParser = GetNonterminal("Primary");
        primaryNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<PostfixRule>(this, &PostfixRule::PrePrimary));
        primaryNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<PostfixRule>(this, &PostfixRule::PostPrimary));
        Cm::Parsing::NonterminalParser* dotMemberIdNonterminalParser = GetNonterminal("dotMemberId");
        dotMemberIdNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<PostfixRule>(this, &PostfixRule::PostdotMemberId));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.expr.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s = span;
        context.expr.reset(context.fromPrimary);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s.SetEnd(span.End());
        context.expr.reset(new DotNode(context.s, context.expr.release(), context.fromdotMemberId));
    }
    void PrePrimary(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostPrimary(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromPrimary_value = std::move(stack.top());
            context.fromPrimary = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromPrimary_value.get());
            stack.pop();
        }
    }
    void PostdotMemberId(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromdotMemberId_value = std::move(stack.top());
            context.fromdotMemberId = *static_cast<Cm::Parsing::ValueObject<IdentifierNode*>*>(fromdotMemberId_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), expr(), s(), fromPrimary(), fromdotMemberId() {}
        ParsingContext* ctx;
        Node* value;
        std::unique_ptr<Node> expr;
        Span s;
        Node* fromPrimary;
        IdentifierNode* fromdotMemberId;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::PrimaryRule : public Cm::Parsing::Rule
{
public:
    PrimaryRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A3Action));
        Cm::Parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<PrimaryRule>(this, &PrimaryRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostExpression));
        Cm::Parsing::NonterminalParser* literalNonterminalParser = GetNonterminal("Literal");
        literalNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostLiteral));
        Cm::Parsing::NonterminalParser* basicTypeNonterminalParser = GetNonterminal("BasicType");
        basicTypeNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostBasicType));
        Cm::Parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostIdentifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromExpression;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromLiteral;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromBasicType;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromIdentifier;
    }
    void PreExpression(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PostLiteral(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromLiteral_value = std::move(stack.top());
            context.fromLiteral = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromLiteral_value.get());
            stack.pop();
        }
    }
    void PostBasicType(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromBasicType_value = std::move(stack.top());
            context.fromBasicType = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromBasicType_value.get());
            stack.pop();
        }
    }
    void PostIdentifier(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromIdentifier_value = std::move(stack.top());
            context.fromIdentifier = *static_cast<Cm::Parsing::ValueObject<IdentifierNode*>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromExpression(), fromLiteral(), fromBasicType(), fromIdentifier() {}
        ParsingContext* ctx;
        Node* value;
        Node* fromExpression;
        Node* fromLiteral;
        Node* fromBasicType;
        IdentifierNode* fromIdentifier;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::ArgumentListRule : public Cm::Parsing::Rule
{
public:
    ArgumentListRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("Node*", "node"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> node_value = std::move(stack.top());
        context.node = *static_cast<Cm::Parsing::ValueObject<Node*>*>(node_value.get());
        stack.pop();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::NonterminalParser* expressionListNonterminalParser = GetNonterminal("ExpressionList");
        expressionListNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ArgumentListRule>(this, &ArgumentListRule::PreExpressionList));
    }
    void PreExpressionList(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Node*>(context.node)));
    }
private:
    struct Context
    {
        Context(): ctx(), node() {}
        ParsingContext* ctx;
        Node* node;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::ExpressionListRule : public Cm::Parsing::Rule
{
public:
    ExpressionListRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("Node*", "node"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> node_value = std::move(stack.top());
        context.node = *static_cast<Cm::Parsing::ValueObject<Node*>*>(node_value.get());
        stack.pop();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ExpressionListRule>(this, &ExpressionListRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ExpressionListRule>(this, &ExpressionListRule::A1Action));
        a1ActionParser->SetFailureAction(new Cm::Parsing::MemberFailureAction<ExpressionListRule>(this, &ExpressionListRule::A1ActionFail));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ExpressionListRule>(this, &ExpressionListRule::A2Action));
        Cm::Parsing::NonterminalParser* argNonterminalParser = GetNonterminal("arg");
        argNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ExpressionListRule>(this, &ExpressionListRule::Prearg));
        argNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ExpressionListRule>(this, &ExpressionListRule::Postarg));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ctx->BeginParsingArguments();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ctx->EndParsingArguments();
    }
    void A1ActionFail()
    {
        context.ctx->EndParsingArguments();
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.node->AddArgument(context.fromarg);
    }
    void Prearg(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postarg(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromarg_value = std::move(stack.top());
            context.fromarg = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromarg_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), node(), fromarg() {}
        ParsingContext* ctx;
        Node* node;
        Node* fromarg;
    };
    std::stack<Context> contextStack;
    Context context;
};

void ExpressionGrammar::GetReferencedGrammars()
{
    Cm::Parsing::ParsingDomain* pd = GetParsingDomain();
    Cm::Parsing::Grammar* grammar0 = pd->GetGrammar("cminor.parser.LiteralGrammar");
    if (!grammar0)
    {
        grammar0 = cminor::parser::LiteralGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    Cm::Parsing::Grammar* grammar1 = pd->GetGrammar("cminor.parser.TypeExprGrammar");
    if (!grammar1)
    {
        grammar1 = cminor::parser::TypeExprGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    Cm::Parsing::Grammar* grammar2 = pd->GetGrammar("cminor.parser.BasicTypeGrammar");
    if (!grammar2)
    {
        grammar2 = cminor::parser::BasicTypeGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    Cm::Parsing::Grammar* grammar3 = pd->GetGrammar("cminor.parser.IdentifierGrammar");
    if (!grammar3)
    {
        grammar3 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
}

void ExpressionGrammar::CreateRules()
{
    AddRuleLink(new Cm::Parsing::RuleLink("Literal", this, "LiteralGrammar.Literal"));
    AddRuleLink(new Cm::Parsing::RuleLink("BasicType", this, "BasicTypeGrammar.BasicType"));
    AddRuleLink(new Cm::Parsing::RuleLink("Identifier", this, "IdentifierGrammar.Identifier"));
    AddRuleLink(new Cm::Parsing::RuleLink("TypeExpr", this, "TypeExprGrammar.TypeExpr"));
    AddRule(new ExpressionRule("Expression", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::NonterminalParser("Disjunction", "Disjunction", 1))));
    AddRule(new DisjunctionRule("Disjunction", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::ActionParser("A1",
                    new Cm::Parsing::NonterminalParser("left", "Conjunction", 1)),
                new Cm::Parsing::KleeneStarParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::ActionParser("A2",
                            new Cm::Parsing::StringParser("||")),
                        new Cm::Parsing::ActionParser("A3",
                            new Cm::Parsing::ExpectationParser(
                                new Cm::Parsing::NonterminalParser("right", "Conjunction", 1)))))))));
    AddRule(new ConjunctionRule("Conjunction", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::ActionParser("A1",
                    new Cm::Parsing::NonterminalParser("left", "BitOr", 1)),
                new Cm::Parsing::KleeneStarParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::ActionParser("A2",
                            new Cm::Parsing::StringParser("&&")),
                        new Cm::Parsing::ActionParser("A3",
                            new Cm::Parsing::ExpectationParser(
                                new Cm::Parsing::NonterminalParser("right", "BitOr", 1)))))))));
    AddRule(new BitOrRule("BitOr", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::ActionParser("A1",
                    new Cm::Parsing::NonterminalParser("left", "BitXor", 1)),
                new Cm::Parsing::KleeneStarParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::ActionParser("A2",
                            new Cm::Parsing::DifferenceParser(
                                new Cm::Parsing::CharParser('|'),
                                new Cm::Parsing::StringParser("||"))),
                        new Cm::Parsing::ActionParser("A3",
                            new Cm::Parsing::ExpectationParser(
                                new Cm::Parsing::NonterminalParser("right", "BitXor", 1)))))))));
    AddRule(new BitXorRule("BitXor", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::ActionParser("A1",
                    new Cm::Parsing::NonterminalParser("left", "BitAnd", 1)),
                new Cm::Parsing::KleeneStarParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::ActionParser("A2",
                            new Cm::Parsing::CharParser('^')),
                        new Cm::Parsing::ActionParser("A3",
                            new Cm::Parsing::ExpectationParser(
                                new Cm::Parsing::NonterminalParser("right", "BitAnd", 1)))))))));
    AddRule(new BitAndRule("BitAnd", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::ActionParser("A1",
                    new Cm::Parsing::NonterminalParser("left", "Equality", 1)),
                new Cm::Parsing::KleeneStarParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::ActionParser("A2",
                            new Cm::Parsing::DifferenceParser(
                                new Cm::Parsing::CharParser('&'),
                                new Cm::Parsing::StringParser("&&"))),
                        new Cm::Parsing::ActionParser("A3",
                            new Cm::Parsing::ExpectationParser(
                                new Cm::Parsing::NonterminalParser("right", "Equality", 1)))))))));
    AddRule(new EqualityRule("Equality", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::ActionParser("A1",
                    new Cm::Parsing::NonterminalParser("left", "Relational", 1)),
                new Cm::Parsing::KleeneStarParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::AlternativeParser(
                            new Cm::Parsing::ActionParser("A2",
                                new Cm::Parsing::StringParser("==")),
                            new Cm::Parsing::ActionParser("A3",
                                new Cm::Parsing::StringParser("!="))),
                        new Cm::Parsing::ActionParser("A4",
                            new Cm::Parsing::ExpectationParser(
                                new Cm::Parsing::NonterminalParser("right", "Relational", 1)))))))));
    AddRule(new RelationalRule("Relational", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::ActionParser("A1",
                    new Cm::Parsing::NonterminalParser("left", "Shift", 1)),
                new Cm::Parsing::KleeneStarParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::AlternativeParser(
                            new Cm::Parsing::AlternativeParser(
                                new Cm::Parsing::AlternativeParser(
                                    new Cm::Parsing::AlternativeParser(
                                        new Cm::Parsing::AlternativeParser(
                                            new Cm::Parsing::ActionParser("A2",
                                                new Cm::Parsing::DifferenceParser(
                                                    new Cm::Parsing::StringParser("<="),
                                                    new Cm::Parsing::StringParser("<=>"))),
                                            new Cm::Parsing::ActionParser("A3",
                                                new Cm::Parsing::StringParser(">="))),
                                        new Cm::Parsing::ActionParser("A4",
                                            new Cm::Parsing::DifferenceParser(
                                                new Cm::Parsing::CharParser('<'),
                                                new Cm::Parsing::AlternativeParser(
                                                    new Cm::Parsing::StringParser("<<"),
                                                    new Cm::Parsing::StringParser("<=>"))))),
                                    new Cm::Parsing::ActionParser("A5",
                                        new Cm::Parsing::DifferenceParser(
                                            new Cm::Parsing::CharParser('>'),
                                            new Cm::Parsing::StringParser(">>")))),
                                new Cm::Parsing::SequenceParser(
                                    new Cm::Parsing::ActionParser("A6",
                                        new Cm::Parsing::KeywordParser("is")),
                                    new Cm::Parsing::NonterminalParser("isType", "TypeExpr", 1))),
                            new Cm::Parsing::SequenceParser(
                                new Cm::Parsing::ActionParser("A7",
                                    new Cm::Parsing::KeywordParser("as")),
                                new Cm::Parsing::NonterminalParser("asType", "TypeExpr", 1))),
                        new Cm::Parsing::ActionParser("A8",
                            new Cm::Parsing::AlternativeParser(
                                new Cm::Parsing::NonterminalParser("right", "Shift", 1),
                                new Cm::Parsing::ActionParser("A9",
                                    new Cm::Parsing::EmptyParser())))))))));
    AddRule(new ShiftRule("Shift", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::ActionParser("A1",
                    new Cm::Parsing::NonterminalParser("left", "Additive", 1)),
                new Cm::Parsing::KleeneStarParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::AlternativeParser(
                                new Cm::Parsing::ActionParser("A2",
                                    new Cm::Parsing::StringParser("<<")),
                                new Cm::Parsing::ActionParser("A3",
                                    new Cm::Parsing::StringParser(">>"))),
                            new Cm::Parsing::ActionParser("A4",
                                new Cm::Parsing::EmptyParser())),
                        new Cm::Parsing::ActionParser("A5",
                            new Cm::Parsing::ExpectationParser(
                                new Cm::Parsing::NonterminalParser("right", "Additive", 1)))))))));
    AddRule(new AdditiveRule("Additive", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::ActionParser("A1",
                    new Cm::Parsing::NonterminalParser("left", "Multiplicative", 1)),
                new Cm::Parsing::KleeneStarParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::AlternativeParser(
                            new Cm::Parsing::ActionParser("A2",
                                new Cm::Parsing::CharParser('+')),
                            new Cm::Parsing::ActionParser("A3",
                                new Cm::Parsing::CharParser('-'))),
                        new Cm::Parsing::ActionParser("A4",
                            new Cm::Parsing::ExpectationParser(
                                new Cm::Parsing::NonterminalParser("right", "Multiplicative", 1)))))))));
    AddRule(new MultiplicativeRule("Multiplicative", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::ActionParser("A1",
                    new Cm::Parsing::NonterminalParser("left", "Prefix", 1)),
                new Cm::Parsing::KleeneStarParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::AlternativeParser(
                            new Cm::Parsing::AlternativeParser(
                                new Cm::Parsing::ActionParser("A2",
                                    new Cm::Parsing::CharParser('*')),
                                new Cm::Parsing::ActionParser("A3",
                                    new Cm::Parsing::CharParser('/'))),
                            new Cm::Parsing::ActionParser("A4",
                                new Cm::Parsing::CharParser('%'))),
                        new Cm::Parsing::ActionParser("A5",
                            new Cm::Parsing::ExpectationParser(
                                new Cm::Parsing::NonterminalParser("right", "Prefix", 1)))))))));
    AddRule(new PrefixRule("Prefix", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::AlternativeParser(
                    new Cm::Parsing::AlternativeParser(
                        new Cm::Parsing::AlternativeParser(
                            new Cm::Parsing::ActionParser("A0",
                                new Cm::Parsing::CharParser('-')),
                            new Cm::Parsing::ActionParser("A1",
                                new Cm::Parsing::CharParser('+'))),
                        new Cm::Parsing::ActionParser("A2",
                            new Cm::Parsing::DifferenceParser(
                                new Cm::Parsing::CharParser('!'),
                                new Cm::Parsing::StringParser("!=")))),
                    new Cm::Parsing::ActionParser("A3",
                        new Cm::Parsing::CharParser('~'))),
                new Cm::Parsing::ActionParser("A4",
                    new Cm::Parsing::NonterminalParser("prefix", "Prefix", 1))),
            new Cm::Parsing::ActionParser("A5",
                new Cm::Parsing::NonterminalParser("Postfix", "Postfix", 1)))));
    AddRule(new PostfixRule("Postfix", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::ActionParser("A1",
                    new Cm::Parsing::NonterminalParser("Primary", "Primary", 1)),
                new Cm::Parsing::KleeneStarParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::CharParser('.'),
                        new Cm::Parsing::ActionParser("A2",
                            new Cm::Parsing::NonterminalParser("dotMemberId", "Identifier", 0))))))));
    AddRule(new PrimaryRule("Primary", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::AlternativeParser(
                new Cm::Parsing::AlternativeParser(
                    new Cm::Parsing::ActionParser("A0",
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::SequenceParser(
                                new Cm::Parsing::CharParser('('),
                                new Cm::Parsing::NonterminalParser("Expression", "Expression", 1)),
                            new Cm::Parsing::CharParser(')'))),
                    new Cm::Parsing::ActionParser("A1",
                        new Cm::Parsing::NonterminalParser("Literal", "Literal", 0))),
                new Cm::Parsing::ActionParser("A2",
                    new Cm::Parsing::NonterminalParser("BasicType", "BasicType", 0))),
            new Cm::Parsing::ActionParser("A3",
                new Cm::Parsing::NonterminalParser("Identifier", "Identifier", 0)))));
    AddRule(new ArgumentListRule("ArgumentList", GetScope(),
        new Cm::Parsing::OptionalParser(
            new Cm::Parsing::NonterminalParser("ExpressionList", "ExpressionList", 2))));
    AddRule(new ExpressionListRule("ExpressionList", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::ActionParser("A0",
                new Cm::Parsing::EmptyParser()),
            new Cm::Parsing::ActionParser("A1",
                new Cm::Parsing::ListParser(
                    new Cm::Parsing::ActionParser("A2",
                        new Cm::Parsing::NonterminalParser("arg", "Expression", 1)),
                    new Cm::Parsing::CharParser(','))))));
}

} } // namespace cminor.parser
