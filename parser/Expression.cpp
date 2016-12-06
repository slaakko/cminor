#include "Expression.hpp"
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>
#include <cminor/parser/Literal.hpp>
#include <cminor/parser/BasicType.hpp>
#include <cminor/parser/Template.hpp>
#include <cminor/parser/Identifier.hpp>
#include <cminor/parser/Operator.hpp>

namespace cminor { namespace parser {

using namespace cminor::parsing;

ExpressionGrammar* ExpressionGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

ExpressionGrammar* ExpressionGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    ExpressionGrammar* grammar(new ExpressionGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

ExpressionGrammar::ExpressionGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("ExpressionGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

Node* ExpressionGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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
    stack.push(std::unique_ptr<cminor::parsing::Object>(new ValueObject<ParsingContext*>(ctx)));
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

class ExpressionGrammar::ExpressionRule : public cminor::parsing::Rule
{
public:
    ExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExpressionRule>(this, &ExpressionRule::A0Action));
        cminor::parsing::NonterminalParser* disjunctionNonterminalParser = GetNonterminal("Disjunction");
        disjunctionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ExpressionRule>(this, &ExpressionRule::PreDisjunction));
        disjunctionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ExpressionRule>(this, &ExpressionRule::PostDisjunction));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromDisjunction;
    }
    void PreDisjunction(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostDisjunction(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDisjunction_value = std::move(stack.top());
            context.fromDisjunction = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromDisjunction_value.get());
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

class ExpressionGrammar::DisjunctionRule : public cminor::parsing::Rule
{
public:
    DisjunctionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DisjunctionRule>(this, &DisjunctionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DisjunctionRule>(this, &DisjunctionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DisjunctionRule>(this, &DisjunctionRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DisjunctionRule>(this, &DisjunctionRule::A3Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DisjunctionRule>(this, &DisjunctionRule::Preleft));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DisjunctionRule>(this, &DisjunctionRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DisjunctionRule>(this, &DisjunctionRule::Preright));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DisjunctionRule>(this, &DisjunctionRule::Postright));
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
    void Preleft(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromright_value.get());
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

class ExpressionGrammar::ConjunctionRule : public cminor::parsing::Rule
{
public:
    ConjunctionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConjunctionRule>(this, &ConjunctionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConjunctionRule>(this, &ConjunctionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConjunctionRule>(this, &ConjunctionRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConjunctionRule>(this, &ConjunctionRule::A3Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ConjunctionRule>(this, &ConjunctionRule::Preleft));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConjunctionRule>(this, &ConjunctionRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ConjunctionRule>(this, &ConjunctionRule::Preright));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConjunctionRule>(this, &ConjunctionRule::Postright));
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
    void Preleft(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromright_value.get());
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

class ExpressionGrammar::BitOrRule : public cminor::parsing::Rule
{
public:
    BitOrRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BitOrRule>(this, &BitOrRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BitOrRule>(this, &BitOrRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BitOrRule>(this, &BitOrRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BitOrRule>(this, &BitOrRule::A3Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<BitOrRule>(this, &BitOrRule::Preleft));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<BitOrRule>(this, &BitOrRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<BitOrRule>(this, &BitOrRule::Preright));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<BitOrRule>(this, &BitOrRule::Postright));
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
    void Preleft(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromright_value.get());
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

class ExpressionGrammar::BitXorRule : public cminor::parsing::Rule
{
public:
    BitXorRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BitXorRule>(this, &BitXorRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BitXorRule>(this, &BitXorRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BitXorRule>(this, &BitXorRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BitXorRule>(this, &BitXorRule::A3Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<BitXorRule>(this, &BitXorRule::Preleft));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<BitXorRule>(this, &BitXorRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<BitXorRule>(this, &BitXorRule::Preright));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<BitXorRule>(this, &BitXorRule::Postright));
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
    void Preleft(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromright_value.get());
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

class ExpressionGrammar::BitAndRule : public cminor::parsing::Rule
{
public:
    BitAndRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BitAndRule>(this, &BitAndRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BitAndRule>(this, &BitAndRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BitAndRule>(this, &BitAndRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BitAndRule>(this, &BitAndRule::A3Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<BitAndRule>(this, &BitAndRule::Preleft));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<BitAndRule>(this, &BitAndRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<BitAndRule>(this, &BitAndRule::Preright));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<BitAndRule>(this, &BitAndRule::Postright));
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
    void Preleft(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromright_value.get());
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

class ExpressionGrammar::EqualityRule : public cminor::parsing::Rule
{
public:
    EqualityRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
        AddLocalVariable(AttrOrVariable("Operator", "op"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EqualityRule>(this, &EqualityRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EqualityRule>(this, &EqualityRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EqualityRule>(this, &EqualityRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EqualityRule>(this, &EqualityRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EqualityRule>(this, &EqualityRule::A4Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<EqualityRule>(this, &EqualityRule::Preleft));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<EqualityRule>(this, &EqualityRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<EqualityRule>(this, &EqualityRule::Preright));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<EqualityRule>(this, &EqualityRule::Postright));
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
    void Preleft(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromright_value.get());
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

class ExpressionGrammar::RelationalRule : public cminor::parsing::Rule
{
public:
    RelationalRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
        AddLocalVariable(AttrOrVariable("Operator", "op"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RelationalRule>(this, &RelationalRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RelationalRule>(this, &RelationalRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RelationalRule>(this, &RelationalRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RelationalRule>(this, &RelationalRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RelationalRule>(this, &RelationalRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RelationalRule>(this, &RelationalRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction("A6");
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RelationalRule>(this, &RelationalRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction("A7");
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RelationalRule>(this, &RelationalRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction("A8");
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RelationalRule>(this, &RelationalRule::A8Action));
        cminor::parsing::ActionParser* a9ActionParser = GetAction("A9");
        a9ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RelationalRule>(this, &RelationalRule::A9Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<RelationalRule>(this, &RelationalRule::Preleft));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RelationalRule>(this, &RelationalRule::Postleft));
        cminor::parsing::NonterminalParser* isTypeNonterminalParser = GetNonterminal("isType");
        isTypeNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<RelationalRule>(this, &RelationalRule::PreisType));
        isTypeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RelationalRule>(this, &RelationalRule::PostisType));
        cminor::parsing::NonterminalParser* asTypeNonterminalParser = GetNonterminal("asType");
        asTypeNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<RelationalRule>(this, &RelationalRule::PreasType));
        asTypeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RelationalRule>(this, &RelationalRule::PostasType));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<RelationalRule>(this, &RelationalRule::Preright));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RelationalRule>(this, &RelationalRule::Postright));
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
    void Preleft(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void PreisType(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostisType(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromisType_value = std::move(stack.top());
            context.fromisType = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromisType_value.get());
            stack.pop();
        }
    }
    void PreasType(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostasType(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromasType_value = std::move(stack.top());
            context.fromasType = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromasType_value.get());
            stack.pop();
        }
    }
    void Preright(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromright_value.get());
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

class ExpressionGrammar::ShiftRule : public cminor::parsing::Rule
{
public:
    ShiftRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
        AddLocalVariable(AttrOrVariable("Operator", "op"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ShiftRule>(this, &ShiftRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ShiftRule>(this, &ShiftRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ShiftRule>(this, &ShiftRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ShiftRule>(this, &ShiftRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ShiftRule>(this, &ShiftRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ShiftRule>(this, &ShiftRule::A5Action));
        a5ActionParser->SetFailureAction(new cminor::parsing::MemberFailureAction<ShiftRule>(this, &ShiftRule::A5ActionFail));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ShiftRule>(this, &ShiftRule::Preleft));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ShiftRule>(this, &ShiftRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ShiftRule>(this, &ShiftRule::Preright));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ShiftRule>(this, &ShiftRule::Postright));
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
    void Preleft(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromright_value.get());
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

class ExpressionGrammar::AdditiveRule : public cminor::parsing::Rule
{
public:
    AdditiveRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
        AddLocalVariable(AttrOrVariable("Operator", "op"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AdditiveRule>(this, &AdditiveRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AdditiveRule>(this, &AdditiveRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AdditiveRule>(this, &AdditiveRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AdditiveRule>(this, &AdditiveRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AdditiveRule>(this, &AdditiveRule::A4Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<AdditiveRule>(this, &AdditiveRule::Preleft));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AdditiveRule>(this, &AdditiveRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<AdditiveRule>(this, &AdditiveRule::Preright));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AdditiveRule>(this, &AdditiveRule::Postright));
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
    void Preleft(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromright_value.get());
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

class ExpressionGrammar::MultiplicativeRule : public cminor::parsing::Rule
{
public:
    MultiplicativeRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
        AddLocalVariable(AttrOrVariable("Operator", "op"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MultiplicativeRule>(this, &MultiplicativeRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MultiplicativeRule>(this, &MultiplicativeRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MultiplicativeRule>(this, &MultiplicativeRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MultiplicativeRule>(this, &MultiplicativeRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MultiplicativeRule>(this, &MultiplicativeRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MultiplicativeRule>(this, &MultiplicativeRule::A5Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<MultiplicativeRule>(this, &MultiplicativeRule::Preleft));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<MultiplicativeRule>(this, &MultiplicativeRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<MultiplicativeRule>(this, &MultiplicativeRule::Preright));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<MultiplicativeRule>(this, &MultiplicativeRule::Postright));
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
    void Preleft(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromright_value.get());
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

class ExpressionGrammar::PrefixRule : public cminor::parsing::Rule
{
public:
    PrefixRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("Span", "s"));
        AddLocalVariable(AttrOrVariable("Operator", "op"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrefixRule>(this, &PrefixRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrefixRule>(this, &PrefixRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrefixRule>(this, &PrefixRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrefixRule>(this, &PrefixRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrefixRule>(this, &PrefixRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrefixRule>(this, &PrefixRule::A5Action));
        cminor::parsing::NonterminalParser* prefixNonterminalParser = GetNonterminal("prefix");
        prefixNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PrefixRule>(this, &PrefixRule::Preprefix));
        prefixNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrefixRule>(this, &PrefixRule::Postprefix));
        cminor::parsing::NonterminalParser* postfixNonterminalParser = GetNonterminal("Postfix");
        postfixNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PrefixRule>(this, &PrefixRule::PrePostfix));
        postfixNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrefixRule>(this, &PrefixRule::PostPostfix));
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
    void Preprefix(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postprefix(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromprefix_value = std::move(stack.top());
            context.fromprefix = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromprefix_value.get());
            stack.pop();
        }
    }
    void PrePostfix(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostPostfix(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromPostfix_value = std::move(stack.top());
            context.fromPostfix = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromPostfix_value.get());
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

class ExpressionGrammar::PostfixRule : public cminor::parsing::Rule
{
public:
    PostfixRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "expr"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixRule>(this, &PostfixRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixRule>(this, &PostfixRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixRule>(this, &PostfixRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixRule>(this, &PostfixRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixRule>(this, &PostfixRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixRule>(this, &PostfixRule::A5Action));
        cminor::parsing::NonterminalParser* primaryNonterminalParser = GetNonterminal("Primary");
        primaryNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PostfixRule>(this, &PostfixRule::PrePrimary));
        primaryNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixRule>(this, &PostfixRule::PostPrimary));
        cminor::parsing::NonterminalParser* dotMemberIdNonterminalParser = GetNonterminal("dotMemberId");
        dotMemberIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixRule>(this, &PostfixRule::PostdotMemberId));
        cminor::parsing::NonterminalParser* indexNonterminalParser = GetNonterminal("index");
        indexNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PostfixRule>(this, &PostfixRule::Preindex));
        indexNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixRule>(this, &PostfixRule::Postindex));
        cminor::parsing::NonterminalParser* argumentListNonterminalParser = GetNonterminal("ArgumentList");
        argumentListNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PostfixRule>(this, &PostfixRule::PreArgumentList));
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
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s.SetEnd(span.End());
        context.expr.reset(new IndexingNode(context.s, context.expr.release(), context.fromindex));
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.expr->GetSpan().SetEnd(span.End());
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.expr.reset(new InvokeNode(context.s, context.expr.release()));
    }
    void PrePrimary(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostPrimary(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromPrimary_value = std::move(stack.top());
            context.fromPrimary = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromPrimary_value.get());
            stack.pop();
        }
    }
    void PostdotMemberId(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromdotMemberId_value = std::move(stack.top());
            context.fromdotMemberId = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromdotMemberId_value.get());
            stack.pop();
        }
    }
    void Preindex(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postindex(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromindex_value = std::move(stack.top());
            context.fromindex = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromindex_value.get());
            stack.pop();
        }
    }
    void PreArgumentList(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.expr.get())));
    }
private:
    struct Context
    {
        Context(): ctx(), value(), expr(), s(), fromPrimary(), fromdotMemberId(), fromindex() {}
        ParsingContext* ctx;
        Node* value;
        std::unique_ptr<Node> expr;
        Span s;
        Node* fromPrimary;
        IdentifierNode* fromdotMemberId;
        Node* fromindex;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::PrimaryRule : public cminor::parsing::Rule
{
public:
    PrimaryRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction("A6");
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction("A7");
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction("A8");
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryRule>(this, &PrimaryRule::A8Action));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PrimaryRule>(this, &PrimaryRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostExpression));
        cminor::parsing::NonterminalParser* literalNonterminalParser = GetNonterminal("Literal");
        literalNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostLiteral));
        cminor::parsing::NonterminalParser* basicTypeNonterminalParser = GetNonterminal("BasicType");
        basicTypeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostBasicType));
        cminor::parsing::NonterminalParser* templateIdNonterminalParser = GetNonterminal("TemplateId");
        templateIdNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PrimaryRule>(this, &PrimaryRule::PreTemplateId));
        templateIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostTemplateId));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostIdentifier));
        cminor::parsing::NonterminalParser* castExprNonterminalParser = GetNonterminal("CastExpr");
        castExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PrimaryRule>(this, &PrimaryRule::PreCastExpr));
        castExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostCastExpr));
        cminor::parsing::NonterminalParser* newExprNonterminalParser = GetNonterminal("NewExpr");
        newExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PrimaryRule>(this, &PrimaryRule::PreNewExpr));
        newExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryRule>(this, &PrimaryRule::PostNewExpr));
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
        context.value = context.fromTemplateId;
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromIdentifier;
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ThisNode(span);
    }
    void A6Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new BaseNode(span);
    }
    void A7Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromCastExpr;
    }
    void A8Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromNewExpr;
    }
    void PreExpression(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PostLiteral(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromLiteral_value = std::move(stack.top());
            context.fromLiteral = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromLiteral_value.get());
            stack.pop();
        }
    }
    void PostBasicType(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromBasicType_value = std::move(stack.top());
            context.fromBasicType = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromBasicType_value.get());
            stack.pop();
        }
    }
    void PreTemplateId(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostTemplateId(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTemplateId_value = std::move(stack.top());
            context.fromTemplateId = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromTemplateId_value.get());
            stack.pop();
        }
    }
    void PostIdentifier(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIdentifier_value = std::move(stack.top());
            context.fromIdentifier = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
    void PreCastExpr(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostCastExpr(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCastExpr_value = std::move(stack.top());
            context.fromCastExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromCastExpr_value.get());
            stack.pop();
        }
    }
    void PreNewExpr(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostNewExpr(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromNewExpr_value = std::move(stack.top());
            context.fromNewExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromNewExpr_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromExpression(), fromLiteral(), fromBasicType(), fromTemplateId(), fromIdentifier(), fromCastExpr(), fromNewExpr() {}
        ParsingContext* ctx;
        Node* value;
        Node* fromExpression;
        Node* fromLiteral;
        Node* fromBasicType;
        Node* fromTemplateId;
        IdentifierNode* fromIdentifier;
        Node* fromCastExpr;
        Node* fromNewExpr;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::CastExprRule : public cminor::parsing::Rule
{
public:
    CastExprRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CastExprRule>(this, &CastExprRule::A0Action));
        cminor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<CastExprRule>(this, &CastExprRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CastExprRule>(this, &CastExprRule::PostTypeExpr));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<CastExprRule>(this, &CastExprRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CastExprRule>(this, &CastExprRule::PostExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new CastNode(span, context.fromTypeExpr, context.fromExpression);
    }
    void PreTypeExpr(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostTypeExpr(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context.fromTypeExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
            stack.pop();
        }
    }
    void PreExpression(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromTypeExpr(), fromExpression() {}
        ParsingContext* ctx;
        Node* value;
        Node* fromTypeExpr;
        Node* fromExpression;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::NewExprRule : public cminor::parsing::Rule
{
public:
    NewExprRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewExprRule>(this, &NewExprRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewExprRule>(this, &NewExprRule::A1Action));
        cminor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<NewExprRule>(this, &NewExprRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NewExprRule>(this, &NewExprRule::PostTypeExpr));
        cminor::parsing::NonterminalParser* argumentListNonterminalParser = GetNonterminal("ArgumentList");
        argumentListNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<NewExprRule>(this, &NewExprRule::PreArgumentList));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new NewNode(span, context.fromTypeExpr);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value->GetSpan().SetEnd(span.End());
    }
    void PreTypeExpr(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostTypeExpr(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context.fromTypeExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
            stack.pop();
        }
    }
    void PreArgumentList(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.value)));
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromTypeExpr() {}
        ParsingContext* ctx;
        Node* value;
        Node* fromTypeExpr;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::ArgumentListRule : public cminor::parsing::Rule
{
public:
    ArgumentListRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("Node*", "node"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> node_value = std::move(stack.top());
        context.node = *static_cast<cminor::parsing::ValueObject<Node*>*>(node_value.get());
        stack.pop();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::NonterminalParser* expressionListNonterminalParser = GetNonterminal("ExpressionList");
        expressionListNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ArgumentListRule>(this, &ArgumentListRule::PreExpressionList));
    }
    void PreExpressionList(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.node)));
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

class ExpressionGrammar::ExpressionListRule : public cminor::parsing::Rule
{
public:
    ExpressionListRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("Node*", "node"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> node_value = std::move(stack.top());
        context.node = *static_cast<cminor::parsing::ValueObject<Node*>*>(node_value.get());
        stack.pop();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExpressionListRule>(this, &ExpressionListRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExpressionListRule>(this, &ExpressionListRule::A1Action));
        a1ActionParser->SetFailureAction(new cminor::parsing::MemberFailureAction<ExpressionListRule>(this, &ExpressionListRule::A1ActionFail));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExpressionListRule>(this, &ExpressionListRule::A2Action));
        cminor::parsing::NonterminalParser* argNonterminalParser = GetNonterminal("arg");
        argNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ExpressionListRule>(this, &ExpressionListRule::Prearg));
        argNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ExpressionListRule>(this, &ExpressionListRule::Postarg));
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
    void Prearg(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postarg(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromarg_value = std::move(stack.top());
            context.fromarg = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromarg_value.get());
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
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("Cm.Parsing.stdlib");
    if (!grammar0)
    {
        grammar0 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar("cminor.parser.TemplateGrammar");
    if (!grammar1)
    {
        grammar1 = cminor::parser::TemplateGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar("cminor.parser.TypeExprGrammar");
    if (!grammar2)
    {
        grammar2 = cminor::parser::TypeExprGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar("cminor.parser.LiteralGrammar");
    if (!grammar3)
    {
        grammar3 = cminor::parser::LiteralGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
    cminor::parsing::Grammar* grammar4 = pd->GetGrammar("cminor.parser.BasicTypeGrammar");
    if (!grammar4)
    {
        grammar4 = cminor::parser::BasicTypeGrammar::Create(pd);
    }
    AddGrammarReference(grammar4);
    cminor::parsing::Grammar* grammar5 = pd->GetGrammar("cminor.parser.IdentifierGrammar");
    if (!grammar5)
    {
        grammar5 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar5);
}

void ExpressionGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("TypeExpr", this, "TypeExprGrammar.TypeExpr"));
    AddRuleLink(new cminor::parsing::RuleLink("Literal", this, "LiteralGrammar.Literal"));
    AddRuleLink(new cminor::parsing::RuleLink("BasicType", this, "BasicTypeGrammar.BasicType"));
    AddRuleLink(new cminor::parsing::RuleLink("TemplateId", this, "TemplateGrammar.TemplateId"));
    AddRuleLink(new cminor::parsing::RuleLink("Identifier", this, "IdentifierGrammar.Identifier"));
    AddRuleLink(new cminor::parsing::RuleLink("identifier", this, "Cm.Parsing.stdlib.identifier"));
    AddRule(new ExpressionRule("Expression", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::NonterminalParser("Disjunction", "Disjunction", 1))));
    AddRule(new DisjunctionRule("Disjunction", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("left", "Conjunction", 1)),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::StringParser("||")),
                        new cminor::parsing::ActionParser("A3",
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("right", "Conjunction", 1)))))))));
    AddRule(new ConjunctionRule("Conjunction", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("left", "BitOr", 1)),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::StringParser("&&")),
                        new cminor::parsing::ActionParser("A3",
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("right", "BitOr", 1)))))))));
    AddRule(new BitOrRule("BitOr", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("left", "BitXor", 1)),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::DifferenceParser(
                                new cminor::parsing::CharParser('|'),
                                new cminor::parsing::StringParser("||"))),
                        new cminor::parsing::ActionParser("A3",
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("right", "BitXor", 1)))))))));
    AddRule(new BitXorRule("BitXor", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("left", "BitAnd", 1)),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::CharParser('^')),
                        new cminor::parsing::ActionParser("A3",
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("right", "BitAnd", 1)))))))));
    AddRule(new BitAndRule("BitAnd", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("left", "Equality", 1)),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::DifferenceParser(
                                new cminor::parsing::CharParser('&'),
                                new cminor::parsing::StringParser("&&"))),
                        new cminor::parsing::ActionParser("A3",
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("right", "Equality", 1)))))))));
    AddRule(new EqualityRule("Equality", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("left", "Relational", 1)),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::ActionParser("A2",
                                new cminor::parsing::StringParser("==")),
                            new cminor::parsing::ActionParser("A3",
                                new cminor::parsing::StringParser("!="))),
                        new cminor::parsing::ActionParser("A4",
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("right", "Relational", 1)))))))));
    AddRule(new RelationalRule("Relational", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("left", "Shift", 1)),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::AlternativeParser(
                                    new cminor::parsing::AlternativeParser(
                                        new cminor::parsing::AlternativeParser(
                                            new cminor::parsing::ActionParser("A2",
                                                new cminor::parsing::DifferenceParser(
                                                    new cminor::parsing::StringParser("<="),
                                                    new cminor::parsing::StringParser("<=>"))),
                                            new cminor::parsing::ActionParser("A3",
                                                new cminor::parsing::StringParser(">="))),
                                        new cminor::parsing::ActionParser("A4",
                                            new cminor::parsing::DifferenceParser(
                                                new cminor::parsing::CharParser('<'),
                                                new cminor::parsing::AlternativeParser(
                                                    new cminor::parsing::StringParser("<<"),
                                                    new cminor::parsing::StringParser("<=>"))))),
                                    new cminor::parsing::ActionParser("A5",
                                        new cminor::parsing::DifferenceParser(
                                            new cminor::parsing::CharParser('>'),
                                            new cminor::parsing::StringParser(">>")))),
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::ActionParser("A6",
                                        new cminor::parsing::KeywordParser("is")),
                                    new cminor::parsing::NonterminalParser("isType", "TypeExpr", 1))),
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::ActionParser("A7",
                                    new cminor::parsing::KeywordParser("as")),
                                new cminor::parsing::NonterminalParser("asType", "TypeExpr", 1))),
                        new cminor::parsing::ActionParser("A8",
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::NonterminalParser("right", "Shift", 1),
                                new cminor::parsing::ActionParser("A9",
                                    new cminor::parsing::EmptyParser())))))))));
    AddRule(new ShiftRule("Shift", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("left", "Additive", 1)),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::ActionParser("A2",
                                    new cminor::parsing::StringParser("<<")),
                                new cminor::parsing::ActionParser("A3",
                                    new cminor::parsing::StringParser(">>"))),
                            new cminor::parsing::ActionParser("A4",
                                new cminor::parsing::EmptyParser())),
                        new cminor::parsing::ActionParser("A5",
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("right", "Additive", 1)))))))));
    AddRule(new AdditiveRule("Additive", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("left", "Multiplicative", 1)),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::ActionParser("A2",
                                new cminor::parsing::CharParser('+')),
                            new cminor::parsing::ActionParser("A3",
                                new cminor::parsing::CharParser('-'))),
                        new cminor::parsing::ActionParser("A4",
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("right", "Multiplicative", 1)))))))));
    AddRule(new MultiplicativeRule("Multiplicative", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("left", "Prefix", 1)),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::ActionParser("A2",
                                    new cminor::parsing::CharParser('*')),
                                new cminor::parsing::ActionParser("A3",
                                    new cminor::parsing::CharParser('/'))),
                            new cminor::parsing::ActionParser("A4",
                                new cminor::parsing::CharParser('%'))),
                        new cminor::parsing::ActionParser("A5",
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("right", "Prefix", 1)))))))));
    AddRule(new PrefixRule("Prefix", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::ActionParser("A0",
                                new cminor::parsing::CharParser('-')),
                            new cminor::parsing::ActionParser("A1",
                                new cminor::parsing::CharParser('+'))),
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::DifferenceParser(
                                new cminor::parsing::CharParser('!'),
                                new cminor::parsing::StringParser("!=")))),
                    new cminor::parsing::ActionParser("A3",
                        new cminor::parsing::CharParser('~'))),
                new cminor::parsing::ActionParser("A4",
                    new cminor::parsing::NonterminalParser("prefix", "Prefix", 1))),
            new cminor::parsing::ActionParser("A5",
                new cminor::parsing::NonterminalParser("Postfix", "Postfix", 1)))));
    AddRule(new PostfixRule("Postfix", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("Primary", "Primary", 1)),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::CharParser('.'),
                                new cminor::parsing::ActionParser("A2",
                                    new cminor::parsing::NonterminalParser("dotMemberId", "Identifier", 0))),
                            new cminor::parsing::ActionParser("A3",
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::CharParser('['),
                                        new cminor::parsing::NonterminalParser("index", "Expression", 1)),
                                    new cminor::parsing::ExpectationParser(
                                        new cminor::parsing::CharParser(']'))))),
                        new cminor::parsing::ActionParser("A4",
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::ActionParser("A5",
                                        new cminor::parsing::CharParser('(')),
                                    new cminor::parsing::NonterminalParser("ArgumentList", "ArgumentList", 2)),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::CharParser(')'))))))))));
    AddRule(new PrimaryRule("Primary", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::AlternativeParser(
                                    new cminor::parsing::AlternativeParser(
                                        new cminor::parsing::ActionParser("A0",
                                            new cminor::parsing::SequenceParser(
                                                new cminor::parsing::SequenceParser(
                                                    new cminor::parsing::CharParser('('),
                                                    new cminor::parsing::NonterminalParser("Expression", "Expression", 1)),
                                                new cminor::parsing::CharParser(')'))),
                                        new cminor::parsing::ActionParser("A1",
                                            new cminor::parsing::NonterminalParser("Literal", "Literal", 0))),
                                    new cminor::parsing::ActionParser("A2",
                                        new cminor::parsing::NonterminalParser("BasicType", "BasicType", 0))),
                                new cminor::parsing::ActionParser("A3",
                                    new cminor::parsing::NonterminalParser("TemplateId", "TemplateId", 1))),
                            new cminor::parsing::ActionParser("A4",
                                new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0))),
                        new cminor::parsing::ActionParser("A5",
                            new cminor::parsing::KeywordParser("this", "identifier"))),
                    new cminor::parsing::ActionParser("A6",
                        new cminor::parsing::KeywordParser("base", "identifier"))),
                new cminor::parsing::ActionParser("A7",
                    new cminor::parsing::NonterminalParser("CastExpr", "CastExpr", 1))),
            new cminor::parsing::ActionParser("A8",
                new cminor::parsing::NonterminalParser("NewExpr", "NewExpr", 1)))));
    AddRule(new CastExprRule("CastExpr", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::KeywordParser("cast"),
                                    new cminor::parsing::ExpectationParser(
                                        new cminor::parsing::CharParser('<'))),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::NonterminalParser("TypeExpr", "TypeExpr", 1))),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::CharParser('>'))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser('('))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("Expression", "Expression", 1))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))))));
    AddRule(new NewExprRule("NewExpr", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser("new"),
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("TypeExpr", "TypeExpr", 1)))),
            new cminor::parsing::OptionalParser(
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::CharParser('('),
                            new cminor::parsing::NonterminalParser("ArgumentList", "ArgumentList", 2)),
                        new cminor::parsing::CharParser(')')))))));
    AddRule(new ArgumentListRule("ArgumentList", GetScope(),
        new cminor::parsing::OptionalParser(
            new cminor::parsing::NonterminalParser("ExpressionList", "ExpressionList", 2))));
    AddRule(new ExpressionListRule("ExpressionList", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::EmptyParser()),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::ListParser(
                    new cminor::parsing::ActionParser("A2",
                        new cminor::parsing::NonterminalParser("arg", "Expression", 1)),
                    new cminor::parsing::CharParser(','))))));
}

} } // namespace cminor.parser
