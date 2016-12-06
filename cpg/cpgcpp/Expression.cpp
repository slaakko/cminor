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
#include <cminor/cpg/cpgcpp/Declaration.hpp>
#include <cminor/cpg/cpgcpp/Declarator.hpp>
#include <cminor/cpg/cpgcpp/Literal.hpp>
#include <cminor/cpg/cpgcpp/Identifier.hpp>
#include <cminor/machine/TextUtils.hpp>
#include <cminor/pom/Type.hpp>

namespace cpg { namespace cpp {

using namespace cminor::pom;
using cminor::machine::Trim;
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

ExpressionGrammar::ExpressionGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("ExpressionGrammar", parsingDomain_->GetNamespaceScope("Cm.Parsing.Cpp"), parsingDomain_)
{
    SetOwner(0);
}

cminor::pom::CppObject* ExpressionGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName)
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
    if (!match.Hit() || stop.Start() != int(end - start))
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
    cminor::pom::CppObject* result = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(value.get());
    stack.pop();
    return result;
}

class ExpressionGrammar::ExpressionRule : public cminor::parsing::Rule
{
public:
    ExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExpressionRule>(this, &ExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExpressionRule>(this, &ExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ExpressionRule>(this, &ExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ExpressionRule>(this, &ExpressionRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromleft;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new BinaryOpExpr(context.value, Operator::comma, 0, context.fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromleft(), fromright() {}
        cminor::pom::CppObject* value;
        cminor::pom::CppObject* fromleft;
        cminor::pom::CppObject* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::ConstantExpressionRule : public cminor::parsing::Rule
{
public:
    ConstantExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConstantExpressionRule>(this, &ConstantExpressionRule::A0Action));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConstantExpressionRule>(this, &ConstantExpressionRule::PostExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromExpression;
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromExpression() {}
        cminor::pom::CppObject* value;
        cminor::pom::CppObject* fromExpression;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::AssignmentExpressionRule : public cminor::parsing::Rule
{
public:
    AssignmentExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<CppObject>", "lor"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AssignmentExpressionRule>(this, &AssignmentExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AssignmentExpressionRule>(this, &AssignmentExpressionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AssignmentExpressionRule>(this, &AssignmentExpressionRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AssignmentExpressionRule>(this, &AssignmentExpressionRule::A3Action));
        cminor::parsing::NonterminalParser* logicalOrExpressionNonterminalParser = GetNonterminal("LogicalOrExpression");
        logicalOrExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AssignmentExpressionRule>(this, &AssignmentExpressionRule::PostLogicalOrExpression));
        cminor::parsing::NonterminalParser* assingmentOpNonterminalParser = GetNonterminal("AssingmentOp");
        assingmentOpNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AssignmentExpressionRule>(this, &AssignmentExpressionRule::PostAssingmentOp));
        cminor::parsing::NonterminalParser* assignmentExpressionNonterminalParser = GetNonterminal("AssignmentExpression");
        assignmentExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AssignmentExpressionRule>(this, &AssignmentExpressionRule::PostAssignmentExpression));
        cminor::parsing::NonterminalParser* conditionalExpressionNonterminalParser = GetNonterminal("ConditionalExpression");
        conditionalExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AssignmentExpressionRule>(this, &AssignmentExpressionRule::PostConditionalExpression));
        cminor::parsing::NonterminalParser* throwExpressionNonterminalParser = GetNonterminal("ThrowExpression");
        throwExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AssignmentExpressionRule>(this, &AssignmentExpressionRule::PostThrowExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new BinaryOpExpr(context.lor.release(), context.fromAssingmentOp, 1, context.fromAssignmentExpression);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.lor.reset(context.fromLogicalOrExpression);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromConditionalExpression;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromThrowExpression;
    }
    void PostLogicalOrExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromLogicalOrExpression_value = std::move(stack.top());
            context.fromLogicalOrExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromLogicalOrExpression_value.get());
            stack.pop();
        }
    }
    void PostAssingmentOp(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAssingmentOp_value = std::move(stack.top());
            context.fromAssingmentOp = *static_cast<cminor::parsing::ValueObject<Operator>*>(fromAssingmentOp_value.get());
            stack.pop();
        }
    }
    void PostAssignmentExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAssignmentExpression_value = std::move(stack.top());
            context.fromAssignmentExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromAssignmentExpression_value.get());
            stack.pop();
        }
    }
    void PostConditionalExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromConditionalExpression_value = std::move(stack.top());
            context.fromConditionalExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromConditionalExpression_value.get());
            stack.pop();
        }
    }
    void PostThrowExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromThrowExpression_value = std::move(stack.top());
            context.fromThrowExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromThrowExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), lor(), fromLogicalOrExpression(), fromAssingmentOp(), fromAssignmentExpression(), fromConditionalExpression(), fromThrowExpression() {}
        cminor::pom::CppObject* value;
        std::unique_ptr<CppObject> lor;
        cminor::pom::CppObject* fromLogicalOrExpression;
        Operator fromAssingmentOp;
        cminor::pom::CppObject* fromAssignmentExpression;
        cminor::pom::CppObject* fromConditionalExpression;
        cminor::pom::CppObject* fromThrowExpression;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::AssingmentOpRule : public cminor::parsing::Rule
{
public:
    AssingmentOpRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Operator");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Operator>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AssingmentOpRule>(this, &AssingmentOpRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = GetOperator(std::string(matchBegin, matchEnd));
    }
private:
    struct Context
    {
        Context(): value() {}
        Operator value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::ThrowExpressionRule : public cminor::parsing::Rule
{
public:
    ThrowExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ThrowExpressionRule>(this, &ThrowExpressionRule::A0Action));
        cminor::parsing::NonterminalParser* assignmentExpressionNonterminalParser = GetNonterminal("AssignmentExpression");
        assignmentExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ThrowExpressionRule>(this, &ThrowExpressionRule::PostAssignmentExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ThrowExpr(context.fromAssignmentExpression);
    }
    void PostAssignmentExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAssignmentExpression_value = std::move(stack.top());
            context.fromAssignmentExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromAssignmentExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromAssignmentExpression() {}
        cminor::pom::CppObject* value;
        cminor::pom::CppObject* fromAssignmentExpression;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::ConditionalExpressionRule : public cminor::parsing::Rule
{
public:
    ConditionalExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConditionalExpressionRule>(this, &ConditionalExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConditionalExpressionRule>(this, &ConditionalExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConditionalExpressionRule>(this, &ConditionalExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConditionalExpressionRule>(this, &ConditionalExpressionRule::PostExpression));
        cminor::parsing::NonterminalParser* assignmentExpressionNonterminalParser = GetNonterminal("AssignmentExpression");
        assignmentExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConditionalExpressionRule>(this, &ConditionalExpressionRule::PostAssignmentExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromleft;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ConditionalExpr(context.value, context.fromExpression, context.fromAssignmentExpression);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PostAssignmentExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAssignmentExpression_value = std::move(stack.top());
            context.fromAssignmentExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromAssignmentExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromleft(), fromExpression(), fromAssignmentExpression() {}
        cminor::pom::CppObject* value;
        cminor::pom::CppObject* fromleft;
        cminor::pom::CppObject* fromExpression;
        cminor::pom::CppObject* fromAssignmentExpression;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::LogicalOrExpressionRule : public cminor::parsing::Rule
{
public:
    LogicalOrExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LogicalOrExpressionRule>(this, &LogicalOrExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LogicalOrExpressionRule>(this, &LogicalOrExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LogicalOrExpressionRule>(this, &LogicalOrExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LogicalOrExpressionRule>(this, &LogicalOrExpressionRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromleft;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new BinaryOpExpr(context.value, Operator::or_, 2, context.fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromleft(), fromright() {}
        cminor::pom::CppObject* value;
        cminor::pom::CppObject* fromleft;
        cminor::pom::CppObject* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::LogicalAndExpressionRule : public cminor::parsing::Rule
{
public:
    LogicalAndExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LogicalAndExpressionRule>(this, &LogicalAndExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LogicalAndExpressionRule>(this, &LogicalAndExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LogicalAndExpressionRule>(this, &LogicalAndExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LogicalAndExpressionRule>(this, &LogicalAndExpressionRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromleft;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new BinaryOpExpr(context.value, Operator::and_, 3, context.fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromleft(), fromright() {}
        cminor::pom::CppObject* value;
        cminor::pom::CppObject* fromleft;
        cminor::pom::CppObject* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::InclusiveOrExpressionRule : public cminor::parsing::Rule
{
public:
    InclusiveOrExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InclusiveOrExpressionRule>(this, &InclusiveOrExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InclusiveOrExpressionRule>(this, &InclusiveOrExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InclusiveOrExpressionRule>(this, &InclusiveOrExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InclusiveOrExpressionRule>(this, &InclusiveOrExpressionRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromleft;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new BinaryOpExpr(context.value, Operator::bitor_, 4, context.fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromleft(), fromright() {}
        cminor::pom::CppObject* value;
        cminor::pom::CppObject* fromleft;
        cminor::pom::CppObject* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::ExclusiveOrExpressionRule : public cminor::parsing::Rule
{
public:
    ExclusiveOrExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExclusiveOrExpressionRule>(this, &ExclusiveOrExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExclusiveOrExpressionRule>(this, &ExclusiveOrExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ExclusiveOrExpressionRule>(this, &ExclusiveOrExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ExclusiveOrExpressionRule>(this, &ExclusiveOrExpressionRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromleft;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new BinaryOpExpr(context.value, Operator::bitxor, 5, context.fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromleft(), fromright() {}
        cminor::pom::CppObject* value;
        cminor::pom::CppObject* fromleft;
        cminor::pom::CppObject* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::AndExpressionRule : public cminor::parsing::Rule
{
public:
    AndExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AndExpressionRule>(this, &AndExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AndExpressionRule>(this, &AndExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AndExpressionRule>(this, &AndExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AndExpressionRule>(this, &AndExpressionRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromleft;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new BinaryOpExpr(context.value, Operator::bitand_, 6, context.fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromleft(), fromright() {}
        cminor::pom::CppObject* value;
        cminor::pom::CppObject* fromleft;
        cminor::pom::CppObject* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::EqualityExpressionRule : public cminor::parsing::Rule
{
public:
    EqualityExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EqualityExpressionRule>(this, &EqualityExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EqualityExpressionRule>(this, &EqualityExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<EqualityExpressionRule>(this, &EqualityExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* eqOpNonterminalParser = GetNonterminal("EqOp");
        eqOpNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<EqualityExpressionRule>(this, &EqualityExpressionRule::PostEqOp));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<EqualityExpressionRule>(this, &EqualityExpressionRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromleft;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new BinaryOpExpr(context.value, context.fromEqOp, 7, context.fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void PostEqOp(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromEqOp_value = std::move(stack.top());
            context.fromEqOp = *static_cast<cminor::parsing::ValueObject<Operator>*>(fromEqOp_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromleft(), fromEqOp(), fromright() {}
        cminor::pom::CppObject* value;
        cminor::pom::CppObject* fromleft;
        Operator fromEqOp;
        cminor::pom::CppObject* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::EqOpRule : public cminor::parsing::Rule
{
public:
    EqOpRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Operator");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Operator>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EqOpRule>(this, &EqOpRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EqOpRule>(this, &EqOpRule::A1Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Operator::eq;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Operator::notEq;
    }
private:
    struct Context
    {
        Context(): value() {}
        Operator value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::RelationalExpressionRule : public cminor::parsing::Rule
{
public:
    RelationalExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RelationalExpressionRule>(this, &RelationalExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RelationalExpressionRule>(this, &RelationalExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RelationalExpressionRule>(this, &RelationalExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* relOpNonterminalParser = GetNonterminal("RelOp");
        relOpNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RelationalExpressionRule>(this, &RelationalExpressionRule::PostRelOp));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RelationalExpressionRule>(this, &RelationalExpressionRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromleft;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new BinaryOpExpr(context.value, context.fromRelOp, 8, context.fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void PostRelOp(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromRelOp_value = std::move(stack.top());
            context.fromRelOp = *static_cast<cminor::parsing::ValueObject<Operator>*>(fromRelOp_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromleft(), fromRelOp(), fromright() {}
        cminor::pom::CppObject* value;
        cminor::pom::CppObject* fromleft;
        Operator fromRelOp;
        cminor::pom::CppObject* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::RelOpRule : public cminor::parsing::Rule
{
public:
    RelOpRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Operator");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Operator>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RelOpRule>(this, &RelOpRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RelOpRule>(this, &RelOpRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RelOpRule>(this, &RelOpRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RelOpRule>(this, &RelOpRule::A3Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Operator::lessOrEq;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Operator::greaterOrEq;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Operator::less;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Operator::greater;
    }
private:
    struct Context
    {
        Context(): value() {}
        Operator value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::ShiftExpressionRule : public cminor::parsing::Rule
{
public:
    ShiftExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ShiftExpressionRule>(this, &ShiftExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ShiftExpressionRule>(this, &ShiftExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ShiftExpressionRule>(this, &ShiftExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* shiftOpNonterminalParser = GetNonterminal("ShiftOp");
        shiftOpNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ShiftExpressionRule>(this, &ShiftExpressionRule::PostShiftOp));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ShiftExpressionRule>(this, &ShiftExpressionRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromleft;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new BinaryOpExpr(context.value, context.fromShiftOp, 9, context.fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void PostShiftOp(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromShiftOp_value = std::move(stack.top());
            context.fromShiftOp = *static_cast<cminor::parsing::ValueObject<Operator>*>(fromShiftOp_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromleft(), fromShiftOp(), fromright() {}
        cminor::pom::CppObject* value;
        cminor::pom::CppObject* fromleft;
        Operator fromShiftOp;
        cminor::pom::CppObject* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::ShiftOpRule : public cminor::parsing::Rule
{
public:
    ShiftOpRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Operator");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Operator>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ShiftOpRule>(this, &ShiftOpRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ShiftOpRule>(this, &ShiftOpRule::A1Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Operator::shiftLeft;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Operator::shiftRight;
    }
private:
    struct Context
    {
        Context(): value() {}
        Operator value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::AdditiveExpressionRule : public cminor::parsing::Rule
{
public:
    AdditiveExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AdditiveExpressionRule>(this, &AdditiveExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AdditiveExpressionRule>(this, &AdditiveExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AdditiveExpressionRule>(this, &AdditiveExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* addOpNonterminalParser = GetNonterminal("AddOp");
        addOpNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AdditiveExpressionRule>(this, &AdditiveExpressionRule::PostAddOp));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AdditiveExpressionRule>(this, &AdditiveExpressionRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromleft;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new BinaryOpExpr(context.value, context.fromAddOp, 10, context.fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void PostAddOp(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAddOp_value = std::move(stack.top());
            context.fromAddOp = *static_cast<cminor::parsing::ValueObject<Operator>*>(fromAddOp_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromleft(), fromAddOp(), fromright() {}
        cminor::pom::CppObject* value;
        cminor::pom::CppObject* fromleft;
        Operator fromAddOp;
        cminor::pom::CppObject* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::AddOpRule : public cminor::parsing::Rule
{
public:
    AddOpRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Operator");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Operator>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AddOpRule>(this, &AddOpRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AddOpRule>(this, &AddOpRule::A1Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Operator::plus;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Operator::minus;
    }
private:
    struct Context
    {
        Context(): value() {}
        Operator value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::MultiplicativeExpressionRule : public cminor::parsing::Rule
{
public:
    MultiplicativeExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MultiplicativeExpressionRule>(this, &MultiplicativeExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MultiplicativeExpressionRule>(this, &MultiplicativeExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<MultiplicativeExpressionRule>(this, &MultiplicativeExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* mulOpNonterminalParser = GetNonterminal("MulOp");
        mulOpNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<MultiplicativeExpressionRule>(this, &MultiplicativeExpressionRule::PostMulOp));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<MultiplicativeExpressionRule>(this, &MultiplicativeExpressionRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromleft;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new BinaryOpExpr(context.value, context.fromMulOp, 11, context.fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void PostMulOp(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromMulOp_value = std::move(stack.top());
            context.fromMulOp = *static_cast<cminor::parsing::ValueObject<Operator>*>(fromMulOp_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromleft(), fromMulOp(), fromright() {}
        cminor::pom::CppObject* value;
        cminor::pom::CppObject* fromleft;
        Operator fromMulOp;
        cminor::pom::CppObject* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::MulOpRule : public cminor::parsing::Rule
{
public:
    MulOpRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Operator");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Operator>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MulOpRule>(this, &MulOpRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MulOpRule>(this, &MulOpRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MulOpRule>(this, &MulOpRule::A2Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Operator::mul;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Operator::div;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Operator::rem;
    }
private:
    struct Context
    {
        Context(): value() {}
        Operator value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::PmExpressionRule : public cminor::parsing::Rule
{
public:
    PmExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PmExpressionRule>(this, &PmExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PmExpressionRule>(this, &PmExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PmExpressionRule>(this, &PmExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* pmOpNonterminalParser = GetNonterminal("PmOp");
        pmOpNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PmExpressionRule>(this, &PmExpressionRule::PostPmOp));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PmExpressionRule>(this, &PmExpressionRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromleft;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new BinaryOpExpr(context.value, context.fromPmOp, 12, context.fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context.fromleft = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void PostPmOp(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromPmOp_value = std::move(stack.top());
            context.fromPmOp = *static_cast<cminor::parsing::ValueObject<Operator>*>(fromPmOp_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context.fromright = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromleft(), fromPmOp(), fromright() {}
        cminor::pom::CppObject* value;
        cminor::pom::CppObject* fromleft;
        Operator fromPmOp;
        cminor::pom::CppObject* fromright;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::PmOpRule : public cminor::parsing::Rule
{
public:
    PmOpRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Operator");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Operator>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PmOpRule>(this, &PmOpRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PmOpRule>(this, &PmOpRule::A1Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Operator::dotStar;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Operator::arrowStar;
    }
private:
    struct Context
    {
        Context(): value() {}
        Operator value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::CastExpressionRule : public cminor::parsing::Rule
{
public:
    CastExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<CppObject>", "ce"));
        AddLocalVariable(AttrOrVariable("std::unique_ptr<CppObject>", "ti"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CastExpressionRule>(this, &CastExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CastExpressionRule>(this, &CastExpressionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CastExpressionRule>(this, &CastExpressionRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CastExpressionRule>(this, &CastExpressionRule::A3Action));
        cminor::parsing::NonterminalParser* typeIdNonterminalParser = GetNonterminal("TypeId");
        typeIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CastExpressionRule>(this, &CastExpressionRule::PostTypeId));
        cminor::parsing::NonterminalParser* castExpressionNonterminalParser = GetNonterminal("CastExpression");
        castExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CastExpressionRule>(this, &CastExpressionRule::PostCastExpression));
        cminor::parsing::NonterminalParser* unaryExpressionNonterminalParser = GetNonterminal("UnaryExpression");
        unaryExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CastExpressionRule>(this, &CastExpressionRule::PostUnaryExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.ce.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ce.reset(new CastExpr(context.ti.release(), context.fromCastExpression));
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ti.reset(context.fromTypeId);
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ce.reset(context.fromUnaryExpression);
    }
    void PostTypeId(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeId_value = std::move(stack.top());
            context.fromTypeId = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeId*>*>(fromTypeId_value.get());
            stack.pop();
        }
    }
    void PostCastExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCastExpression_value = std::move(stack.top());
            context.fromCastExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromCastExpression_value.get());
            stack.pop();
        }
    }
    void PostUnaryExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromUnaryExpression_value = std::move(stack.top());
            context.fromUnaryExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromUnaryExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), ce(), ti(), fromTypeId(), fromCastExpression(), fromUnaryExpression() {}
        cminor::pom::CppObject* value;
        std::unique_ptr<CppObject> ce;
        std::unique_ptr<CppObject> ti;
        cminor::pom::TypeId* fromTypeId;
        cminor::pom::CppObject* fromCastExpression;
        cminor::pom::CppObject* fromUnaryExpression;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::UnaryExpressionRule : public cminor::parsing::Rule
{
public:
    UnaryExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<CppObject>", "ue"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryExpressionRule>(this, &UnaryExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryExpressionRule>(this, &UnaryExpressionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryExpressionRule>(this, &UnaryExpressionRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryExpressionRule>(this, &UnaryExpressionRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryExpressionRule>(this, &UnaryExpressionRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryExpressionRule>(this, &UnaryExpressionRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction("A6");
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryExpressionRule>(this, &UnaryExpressionRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction("A7");
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryExpressionRule>(this, &UnaryExpressionRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction("A8");
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryExpressionRule>(this, &UnaryExpressionRule::A8Action));
        cminor::parsing::ActionParser* a9ActionParser = GetAction("A9");
        a9ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryExpressionRule>(this, &UnaryExpressionRule::A9Action));
        cminor::parsing::NonterminalParser* postfixExpressionNonterminalParser = GetNonterminal("PostfixExpression");
        postfixExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UnaryExpressionRule>(this, &UnaryExpressionRule::PostPostfixExpression));
        cminor::parsing::NonterminalParser* postCastExpressionNonterminalParser = GetNonterminal("PostCastExpression");
        postCastExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UnaryExpressionRule>(this, &UnaryExpressionRule::PostPostCastExpression));
        cminor::parsing::NonterminalParser* e1NonterminalParser = GetNonterminal("e1");
        e1NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UnaryExpressionRule>(this, &UnaryExpressionRule::Poste1));
        cminor::parsing::NonterminalParser* e2NonterminalParser = GetNonterminal("e2");
        e2NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UnaryExpressionRule>(this, &UnaryExpressionRule::Poste2));
        cminor::parsing::NonterminalParser* unaryOperatorNonterminalParser = GetNonterminal("UnaryOperator");
        unaryOperatorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UnaryExpressionRule>(this, &UnaryExpressionRule::PostUnaryOperator));
        cminor::parsing::NonterminalParser* castExpressionNonterminalParser = GetNonterminal("CastExpression");
        castExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UnaryExpressionRule>(this, &UnaryExpressionRule::PostCastExpression));
        cminor::parsing::NonterminalParser* typeIdNonterminalParser = GetNonterminal("TypeId");
        typeIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UnaryExpressionRule>(this, &UnaryExpressionRule::PostTypeId));
        cminor::parsing::NonterminalParser* e3NonterminalParser = GetNonterminal("e3");
        e3NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UnaryExpressionRule>(this, &UnaryExpressionRule::Poste3));
        cminor::parsing::NonterminalParser* newExpressionNonterminalParser = GetNonterminal("NewExpression");
        newExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UnaryExpressionRule>(this, &UnaryExpressionRule::PostNewExpression));
        cminor::parsing::NonterminalParser* deleteExpressionNonterminalParser = GetNonterminal("DeleteExpression");
        deleteExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UnaryExpressionRule>(this, &UnaryExpressionRule::PostDeleteExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.ue.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ue.reset(context.fromPostfixExpression);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ue.reset(context.fromPostCastExpression);
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ue.reset(new PreIncrementExpr(context.frome1));
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ue.reset(new PreDecrementExpr(context.frome2));
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ue.reset(new UnaryOpExpr(context.fromUnaryOperator, context.fromCastExpression));
    }
    void A6Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ue.reset(new SizeOfExpr(context.fromTypeId, true));
    }
    void A7Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ue.reset(new SizeOfExpr(context.frome3, false));
    }
    void A8Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ue.reset(context.fromNewExpression);
    }
    void A9Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ue.reset(context.fromDeleteExpression);
    }
    void PostPostfixExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromPostfixExpression_value = std::move(stack.top());
            context.fromPostfixExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromPostfixExpression_value.get());
            stack.pop();
        }
    }
    void PostPostCastExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromPostCastExpression_value = std::move(stack.top());
            context.fromPostCastExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromPostCastExpression_value.get());
            stack.pop();
        }
    }
    void Poste1(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frome1_value = std::move(stack.top());
            context.frome1 = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(frome1_value.get());
            stack.pop();
        }
    }
    void Poste2(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frome2_value = std::move(stack.top());
            context.frome2 = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(frome2_value.get());
            stack.pop();
        }
    }
    void PostUnaryOperator(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromUnaryOperator_value = std::move(stack.top());
            context.fromUnaryOperator = *static_cast<cminor::parsing::ValueObject<Operator>*>(fromUnaryOperator_value.get());
            stack.pop();
        }
    }
    void PostCastExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCastExpression_value = std::move(stack.top());
            context.fromCastExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromCastExpression_value.get());
            stack.pop();
        }
    }
    void PostTypeId(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeId_value = std::move(stack.top());
            context.fromTypeId = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeId*>*>(fromTypeId_value.get());
            stack.pop();
        }
    }
    void Poste3(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frome3_value = std::move(stack.top());
            context.frome3 = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(frome3_value.get());
            stack.pop();
        }
    }
    void PostNewExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromNewExpression_value = std::move(stack.top());
            context.fromNewExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromNewExpression_value.get());
            stack.pop();
        }
    }
    void PostDeleteExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDeleteExpression_value = std::move(stack.top());
            context.fromDeleteExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromDeleteExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), ue(), fromPostfixExpression(), fromPostCastExpression(), frome1(), frome2(), fromUnaryOperator(), fromCastExpression(), fromTypeId(), frome3(), fromNewExpression(), fromDeleteExpression() {}
        cminor::pom::CppObject* value;
        std::unique_ptr<CppObject> ue;
        cminor::pom::CppObject* fromPostfixExpression;
        cminor::pom::CppObject* fromPostCastExpression;
        cminor::pom::CppObject* frome1;
        cminor::pom::CppObject* frome2;
        Operator fromUnaryOperator;
        cminor::pom::CppObject* fromCastExpression;
        cminor::pom::TypeId* fromTypeId;
        cminor::pom::CppObject* frome3;
        cminor::pom::CppObject* fromNewExpression;
        cminor::pom::CppObject* fromDeleteExpression;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::UnaryOperatorRule : public cminor::parsing::Rule
{
public:
    UnaryOperatorRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Operator");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Operator>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryOperatorRule>(this, &UnaryOperatorRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryOperatorRule>(this, &UnaryOperatorRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryOperatorRule>(this, &UnaryOperatorRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryOperatorRule>(this, &UnaryOperatorRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryOperatorRule>(this, &UnaryOperatorRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryOperatorRule>(this, &UnaryOperatorRule::A5Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Operator::deref;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Operator::addrOf;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Operator::plus;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Operator::minus;
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Operator::not_;
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Operator::neg;
    }
private:
    struct Context
    {
        Context(): value() {}
        Operator value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::NewExpressionRule : public cminor::parsing::Rule
{
public:
    NewExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
        AddLocalVariable(AttrOrVariable("bool", "global"));
        AddLocalVariable(AttrOrVariable("TypeId*", "typeId"));
        AddLocalVariable(AttrOrVariable("bool", "parens"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewExpressionRule>(this, &NewExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewExpressionRule>(this, &NewExpressionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewExpressionRule>(this, &NewExpressionRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewExpressionRule>(this, &NewExpressionRule::A3Action));
        cminor::parsing::NonterminalParser* newPlacementNonterminalParser = GetNonterminal("NewPlacement");
        newPlacementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NewExpressionRule>(this, &NewExpressionRule::PostNewPlacement));
        cminor::parsing::NonterminalParser* newTypeIdNonterminalParser = GetNonterminal("NewTypeId");
        newTypeIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NewExpressionRule>(this, &NewExpressionRule::PostNewTypeId));
        cminor::parsing::NonterminalParser* typeIdNonterminalParser = GetNonterminal("TypeId");
        typeIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NewExpressionRule>(this, &NewExpressionRule::PostTypeId));
        cminor::parsing::NonterminalParser* newInitializerNonterminalParser = GetNonterminal("NewInitializer");
        newInitializerNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NewExpressionRule>(this, &NewExpressionRule::PostNewInitializer));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new NewExpr(context.global, context.fromNewPlacement, context.typeId, context.parens, context.fromNewInitializer);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.global = true;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.typeId = context.fromNewTypeId;
        context.parens = false;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.typeId = context.fromTypeId;
        context.parens = true;
    }
    void PostNewPlacement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromNewPlacement_value = std::move(stack.top());
            context.fromNewPlacement = *static_cast<cminor::parsing::ValueObject<std::vector<cminor::pom::CppObject*>>*>(fromNewPlacement_value.get());
            stack.pop();
        }
    }
    void PostNewTypeId(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromNewTypeId_value = std::move(stack.top());
            context.fromNewTypeId = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeId*>*>(fromNewTypeId_value.get());
            stack.pop();
        }
    }
    void PostTypeId(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeId_value = std::move(stack.top());
            context.fromTypeId = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeId*>*>(fromTypeId_value.get());
            stack.pop();
        }
    }
    void PostNewInitializer(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromNewInitializer_value = std::move(stack.top());
            context.fromNewInitializer = *static_cast<cminor::parsing::ValueObject<std::vector<cminor::pom::CppObject*>>*>(fromNewInitializer_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), global(), typeId(), parens(), fromNewPlacement(), fromNewTypeId(), fromTypeId(), fromNewInitializer() {}
        cminor::pom::CppObject* value;
        bool global;
        TypeId* typeId;
        bool parens;
        std::vector<cminor::pom::CppObject*> fromNewPlacement;
        cminor::pom::TypeId* fromNewTypeId;
        cminor::pom::TypeId* fromTypeId;
        std::vector<cminor::pom::CppObject*> fromNewInitializer;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::NewTypeIdRule : public cminor::parsing::Rule
{
public:
    NewTypeIdRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::TypeId*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<TypeId>", "ti"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::TypeId*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewTypeIdRule>(this, &NewTypeIdRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewTypeIdRule>(this, &NewTypeIdRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewTypeIdRule>(this, &NewTypeIdRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewTypeIdRule>(this, &NewTypeIdRule::A3Action));
        cminor::parsing::NonterminalParser* typeSpecifierSeqNonterminalParser = GetNonterminal("TypeSpecifierSeq");
        typeSpecifierSeqNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<NewTypeIdRule>(this, &NewTypeIdRule::PreTypeSpecifierSeq));
        cminor::parsing::NonterminalParser* typeNameNonterminalParser = GetNonterminal("TypeName");
        typeNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NewTypeIdRule>(this, &NewTypeIdRule::PostTypeName));
        cminor::parsing::NonterminalParser* newDeclaratorNonterminalParser = GetNonterminal("NewDeclarator");
        newDeclaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NewTypeIdRule>(this, &NewTypeIdRule::PostNewDeclarator));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ti.reset(new TypeId);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.ti.release();
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ti->Add(context.fromTypeName);
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ti->Declarator() = context.fromNewDeclarator;
    }
    void PreTypeSpecifierSeq(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::TypeId*>(context.ti.get())));
    }
    void PostTypeName(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeName_value = std::move(stack.top());
            context.fromTypeName = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeName*>*>(fromTypeName_value.get());
            stack.pop();
        }
    }
    void PostNewDeclarator(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromNewDeclarator_value = std::move(stack.top());
            context.fromNewDeclarator = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromNewDeclarator_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), ti(), fromTypeName(), fromNewDeclarator() {}
        cminor::pom::TypeId* value;
        std::unique_ptr<TypeId> ti;
        cminor::pom::TypeName* fromTypeName;
        std::string fromNewDeclarator;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::NewDeclaratorRule : public cminor::parsing::Rule
{
public:
    NewDeclaratorRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewDeclaratorRule>(this, &NewDeclaratorRule::A0Action));
        cminor::parsing::NonterminalParser* newDeclaratorNonterminalParser = GetNonterminal("NewDeclarator");
        newDeclaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NewDeclaratorRule>(this, &NewDeclaratorRule::PostNewDeclarator));
        cminor::parsing::NonterminalParser* directNewDeclaratorNonterminalParser = GetNonterminal("DirectNewDeclarator");
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Trim(std::string(matchBegin, matchEnd));
    }
    void PostNewDeclarator(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromNewDeclarator_value = std::move(stack.top());
            context.fromNewDeclarator = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromNewDeclarator_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromNewDeclarator() {}
        std::string value;
        std::string fromNewDeclarator;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::DirectNewDeclaratorRule : public cminor::parsing::Rule
{
public:
    DirectNewDeclaratorRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddLocalVariable(AttrOrVariable("std::unique_ptr<CppObject>", "e"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DirectNewDeclaratorRule>(this, &DirectNewDeclaratorRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DirectNewDeclaratorRule>(this, &DirectNewDeclaratorRule::A1Action));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DirectNewDeclaratorRule>(this, &DirectNewDeclaratorRule::PostExpression));
        cminor::parsing::NonterminalParser* constantExpressionNonterminalParser = GetNonterminal("ConstantExpression");
        constantExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DirectNewDeclaratorRule>(this, &DirectNewDeclaratorRule::PostConstantExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.e.reset(context.fromExpression);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.e.reset(context.fromConstantExpression);
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PostConstantExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromConstantExpression_value = std::move(stack.top());
            context.fromConstantExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromConstantExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): e(), fromExpression(), fromConstantExpression() {}
        std::unique_ptr<CppObject> e;
        cminor::pom::CppObject* fromExpression;
        cminor::pom::CppObject* fromConstantExpression;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::NewPlacementRule : public cminor::parsing::Rule
{
public:
    NewPlacementRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("std::vector<cminor::pom::CppObject*>");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::vector<cminor::pom::CppObject*>>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewPlacementRule>(this, &NewPlacementRule::A0Action));
        cminor::parsing::NonterminalParser* expressionListNonterminalParser = GetNonterminal("ExpressionList");
        expressionListNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NewPlacementRule>(this, &NewPlacementRule::PostExpressionList));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromExpressionList;
    }
    void PostExpressionList(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpressionList_value = std::move(stack.top());
            context.fromExpressionList = *static_cast<cminor::parsing::ValueObject<std::vector<cminor::pom::CppObject*>>*>(fromExpressionList_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromExpressionList() {}
        std::vector<cminor::pom::CppObject*> value;
        std::vector<cminor::pom::CppObject*> fromExpressionList;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::NewInitializerRule : public cminor::parsing::Rule
{
public:
    NewInitializerRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("std::vector<cminor::pom::CppObject*>");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::vector<cminor::pom::CppObject*>>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewInitializerRule>(this, &NewInitializerRule::A0Action));
        cminor::parsing::NonterminalParser* expressionListNonterminalParser = GetNonterminal("ExpressionList");
        expressionListNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NewInitializerRule>(this, &NewInitializerRule::PostExpressionList));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromExpressionList;
    }
    void PostExpressionList(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpressionList_value = std::move(stack.top());
            context.fromExpressionList = *static_cast<cminor::parsing::ValueObject<std::vector<cminor::pom::CppObject*>>*>(fromExpressionList_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromExpressionList() {}
        std::vector<cminor::pom::CppObject*> value;
        std::vector<cminor::pom::CppObject*> fromExpressionList;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::DeleteExpressionRule : public cminor::parsing::Rule
{
public:
    DeleteExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
        AddLocalVariable(AttrOrVariable("bool", "global"));
        AddLocalVariable(AttrOrVariable("bool", "array"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeleteExpressionRule>(this, &DeleteExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeleteExpressionRule>(this, &DeleteExpressionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeleteExpressionRule>(this, &DeleteExpressionRule::A2Action));
        cminor::parsing::NonterminalParser* castExpressionNonterminalParser = GetNonterminal("CastExpression");
        castExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeleteExpressionRule>(this, &DeleteExpressionRule::PostCastExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new DeleteExpr(context.global, context.array, context.fromCastExpression);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.global = true;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.array = true;
    }
    void PostCastExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCastExpression_value = std::move(stack.top());
            context.fromCastExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromCastExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), global(), array(), fromCastExpression() {}
        cminor::pom::CppObject* value;
        bool global;
        bool array;
        cminor::pom::CppObject* fromCastExpression;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::PostfixExpressionRule : public cminor::parsing::Rule
{
public:
    PostfixExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<CppObject>", "pe"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixExpressionRule>(this, &PostfixExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixExpressionRule>(this, &PostfixExpressionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixExpressionRule>(this, &PostfixExpressionRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixExpressionRule>(this, &PostfixExpressionRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixExpressionRule>(this, &PostfixExpressionRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixExpressionRule>(this, &PostfixExpressionRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction("A6");
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixExpressionRule>(this, &PostfixExpressionRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction("A7");
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixExpressionRule>(this, &PostfixExpressionRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction("A8");
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixExpressionRule>(this, &PostfixExpressionRule::A8Action));
        cminor::parsing::NonterminalParser* primaryExpressionNonterminalParser = GetNonterminal("PrimaryExpression");
        primaryExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixExpressionRule>(this, &PostfixExpressionRule::PostPrimaryExpression));
        cminor::parsing::NonterminalParser* indexNonterminalParser = GetNonterminal("index");
        indexNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixExpressionRule>(this, &PostfixExpressionRule::Postindex));
        cminor::parsing::NonterminalParser* invokeNonterminalParser = GetNonterminal("invoke");
        invokeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixExpressionRule>(this, &PostfixExpressionRule::Postinvoke));
        cminor::parsing::NonterminalParser* memberNonterminalParser = GetNonterminal("member");
        memberNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixExpressionRule>(this, &PostfixExpressionRule::Postmember));
        cminor::parsing::NonterminalParser* ptrmemberNonterminalParser = GetNonterminal("ptrmember");
        ptrmemberNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixExpressionRule>(this, &PostfixExpressionRule::Postptrmember));
        cminor::parsing::NonterminalParser* typeSpecifierOrTypeNameNonterminalParser = GetNonterminal("TypeSpecifierOrTypeName");
        typeSpecifierOrTypeNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixExpressionRule>(this, &PostfixExpressionRule::PostTypeSpecifierOrTypeName));
        cminor::parsing::NonterminalParser* typeConstructionNonterminalParser = GetNonterminal("typeConstruction");
        typeConstructionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixExpressionRule>(this, &PostfixExpressionRule::PosttypeConstruction));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.pe.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.pe.reset(context.fromPrimaryExpression);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.pe.reset(new IndexExpr(context.pe.release(), context.fromindex));
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.pe.reset(new InvokeExpr(context.pe.release(), context.frominvoke));
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.pe.reset(new MemberAccessExpr(context.pe.release(), context.frommember));
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.pe.reset(new PtrMemberAccessExpr(context.pe.release(), context.fromptrmember));
    }
    void A6Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.pe.reset(new PostIncrementExpr(context.pe.release()));
    }
    void A7Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.pe.reset(new PostDecrementExpr(context.pe.release()));
    }
    void A8Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.pe.reset(new InvokeExpr(context.fromTypeSpecifierOrTypeName, context.fromtypeConstruction));
    }
    void PostPrimaryExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromPrimaryExpression_value = std::move(stack.top());
            context.fromPrimaryExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromPrimaryExpression_value.get());
            stack.pop();
        }
    }
    void Postindex(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromindex_value = std::move(stack.top());
            context.fromindex = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromindex_value.get());
            stack.pop();
        }
    }
    void Postinvoke(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frominvoke_value = std::move(stack.top());
            context.frominvoke = *static_cast<cminor::parsing::ValueObject<std::vector<cminor::pom::CppObject*>>*>(frominvoke_value.get());
            stack.pop();
        }
    }
    void Postmember(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frommember_value = std::move(stack.top());
            context.frommember = *static_cast<cminor::parsing::ValueObject<cminor::pom::IdExpr*>*>(frommember_value.get());
            stack.pop();
        }
    }
    void Postptrmember(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromptrmember_value = std::move(stack.top());
            context.fromptrmember = *static_cast<cminor::parsing::ValueObject<cminor::pom::IdExpr*>*>(fromptrmember_value.get());
            stack.pop();
        }
    }
    void PostTypeSpecifierOrTypeName(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeSpecifierOrTypeName_value = std::move(stack.top());
            context.fromTypeSpecifierOrTypeName = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromTypeSpecifierOrTypeName_value.get());
            stack.pop();
        }
    }
    void PosttypeConstruction(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromtypeConstruction_value = std::move(stack.top());
            context.fromtypeConstruction = *static_cast<cminor::parsing::ValueObject<std::vector<cminor::pom::CppObject*>>*>(fromtypeConstruction_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), pe(), fromPrimaryExpression(), fromindex(), frominvoke(), frommember(), fromptrmember(), fromTypeSpecifierOrTypeName(), fromtypeConstruction() {}
        cminor::pom::CppObject* value;
        std::unique_ptr<CppObject> pe;
        cminor::pom::CppObject* fromPrimaryExpression;
        cminor::pom::CppObject* fromindex;
        std::vector<cminor::pom::CppObject*> frominvoke;
        cminor::pom::IdExpr* frommember;
        cminor::pom::IdExpr* fromptrmember;
        cminor::pom::CppObject* fromTypeSpecifierOrTypeName;
        std::vector<cminor::pom::CppObject*> fromtypeConstruction;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::TypeSpecifierOrTypeNameRule : public cminor::parsing::Rule
{
public:
    TypeSpecifierOrTypeNameRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeSpecifierOrTypeNameRule>(this, &TypeSpecifierOrTypeNameRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeSpecifierOrTypeNameRule>(this, &TypeSpecifierOrTypeNameRule::A1Action));
        cminor::parsing::NonterminalParser* simpleTypeSpecifierNonterminalParser = GetNonterminal("SimpleTypeSpecifier");
        simpleTypeSpecifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TypeSpecifierOrTypeNameRule>(this, &TypeSpecifierOrTypeNameRule::PostSimpleTypeSpecifier));
        cminor::parsing::NonterminalParser* typeNameNonterminalParser = GetNonterminal("TypeName");
        typeNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TypeSpecifierOrTypeNameRule>(this, &TypeSpecifierOrTypeNameRule::PostTypeName));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromSimpleTypeSpecifier;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromTypeName;
    }
    void PostSimpleTypeSpecifier(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSimpleTypeSpecifier_value = std::move(stack.top());
            context.fromSimpleTypeSpecifier = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeSpecifier*>*>(fromSimpleTypeSpecifier_value.get());
            stack.pop();
        }
    }
    void PostTypeName(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeName_value = std::move(stack.top());
            context.fromTypeName = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeName*>*>(fromTypeName_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromSimpleTypeSpecifier(), fromTypeName() {}
        cminor::pom::CppObject* value;
        cminor::pom::TypeSpecifier* fromSimpleTypeSpecifier;
        cminor::pom::TypeName* fromTypeName;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::PostCastExpressionRule : public cminor::parsing::Rule
{
public:
    PostCastExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostCastExpressionRule>(this, &PostCastExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostCastExpressionRule>(this, &PostCastExpressionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostCastExpressionRule>(this, &PostCastExpressionRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostCastExpressionRule>(this, &PostCastExpressionRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostCastExpressionRule>(this, &PostCastExpressionRule::A4Action));
        cminor::parsing::NonterminalParser* dctNonterminalParser = GetNonterminal("dct");
        dctNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostCastExpressionRule>(this, &PostCastExpressionRule::Postdct));
        cminor::parsing::NonterminalParser* dceNonterminalParser = GetNonterminal("dce");
        dceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostCastExpressionRule>(this, &PostCastExpressionRule::Postdce));
        cminor::parsing::NonterminalParser* sctNonterminalParser = GetNonterminal("sct");
        sctNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostCastExpressionRule>(this, &PostCastExpressionRule::Postsct));
        cminor::parsing::NonterminalParser* sceNonterminalParser = GetNonterminal("sce");
        sceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostCastExpressionRule>(this, &PostCastExpressionRule::Postsce));
        cminor::parsing::NonterminalParser* rctNonterminalParser = GetNonterminal("rct");
        rctNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostCastExpressionRule>(this, &PostCastExpressionRule::Postrct));
        cminor::parsing::NonterminalParser* rceNonterminalParser = GetNonterminal("rce");
        rceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostCastExpressionRule>(this, &PostCastExpressionRule::Postrce));
        cminor::parsing::NonterminalParser* cctNonterminalParser = GetNonterminal("cct");
        cctNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostCastExpressionRule>(this, &PostCastExpressionRule::Postcct));
        cminor::parsing::NonterminalParser* cceNonterminalParser = GetNonterminal("cce");
        cceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostCastExpressionRule>(this, &PostCastExpressionRule::Postcce));
        cminor::parsing::NonterminalParser* typeidExprNonterminalParser = GetNonterminal("typeidExpr");
        typeidExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostCastExpressionRule>(this, &PostCastExpressionRule::PosttypeidExpr));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new PostCastExpr("dynamic_cast", context.fromdct, context.fromdce);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new PostCastExpr("static_cast", context.fromsct, context.fromsce);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new PostCastExpr("reinterpret_cast", context.fromrct, context.fromrce);
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new PostCastExpr("const_cast", context.fromcct, context.fromcce);
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new TypeIdExpr(context.fromtypeidExpr);
    }
    void Postdct(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromdct_value = std::move(stack.top());
            context.fromdct = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeId*>*>(fromdct_value.get());
            stack.pop();
        }
    }
    void Postdce(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromdce_value = std::move(stack.top());
            context.fromdce = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromdce_value.get());
            stack.pop();
        }
    }
    void Postsct(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromsct_value = std::move(stack.top());
            context.fromsct = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeId*>*>(fromsct_value.get());
            stack.pop();
        }
    }
    void Postsce(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromsce_value = std::move(stack.top());
            context.fromsce = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromsce_value.get());
            stack.pop();
        }
    }
    void Postrct(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromrct_value = std::move(stack.top());
            context.fromrct = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeId*>*>(fromrct_value.get());
            stack.pop();
        }
    }
    void Postrce(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromrce_value = std::move(stack.top());
            context.fromrce = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromrce_value.get());
            stack.pop();
        }
    }
    void Postcct(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromcct_value = std::move(stack.top());
            context.fromcct = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeId*>*>(fromcct_value.get());
            stack.pop();
        }
    }
    void Postcce(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromcce_value = std::move(stack.top());
            context.fromcce = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromcce_value.get());
            stack.pop();
        }
    }
    void PosttypeidExpr(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromtypeidExpr_value = std::move(stack.top());
            context.fromtypeidExpr = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromtypeidExpr_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromdct(), fromdce(), fromsct(), fromsce(), fromrct(), fromrce(), fromcct(), fromcce(), fromtypeidExpr() {}
        cminor::pom::CppObject* value;
        cminor::pom::TypeId* fromdct;
        cminor::pom::CppObject* fromdce;
        cminor::pom::TypeId* fromsct;
        cminor::pom::CppObject* fromsce;
        cminor::pom::TypeId* fromrct;
        cminor::pom::CppObject* fromrce;
        cminor::pom::TypeId* fromcct;
        cminor::pom::CppObject* fromcce;
        cminor::pom::CppObject* fromtypeidExpr;
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
        SetValueTypeName("std::vector<cminor::pom::CppObject*>");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::vector<cminor::pom::CppObject*>>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExpressionListRule>(this, &ExpressionListRule::A0Action));
        cminor::parsing::NonterminalParser* assignmentExpressionNonterminalParser = GetNonterminal("AssignmentExpression");
        assignmentExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ExpressionListRule>(this, &ExpressionListRule::PostAssignmentExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value.push_back(context.fromAssignmentExpression);
    }
    void PostAssignmentExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAssignmentExpression_value = std::move(stack.top());
            context.fromAssignmentExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromAssignmentExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromAssignmentExpression() {}
        std::vector<cminor::pom::CppObject*> value;
        cminor::pom::CppObject* fromAssignmentExpression;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::PrimaryExpressionRule : public cminor::parsing::Rule
{
public:
    PrimaryExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::CppObject*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<CppObject>", "pe"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryExpressionRule>(this, &PrimaryExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryExpressionRule>(this, &PrimaryExpressionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryExpressionRule>(this, &PrimaryExpressionRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryExpressionRule>(this, &PrimaryExpressionRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryExpressionRule>(this, &PrimaryExpressionRule::A4Action));
        cminor::parsing::NonterminalParser* literalNonterminalParser = GetNonterminal("Literal");
        literalNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryExpressionRule>(this, &PrimaryExpressionRule::PostLiteral));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal("Expression");
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryExpressionRule>(this, &PrimaryExpressionRule::PostExpression));
        cminor::parsing::NonterminalParser* idExpressionNonterminalParser = GetNonterminal("IdExpression");
        idExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryExpressionRule>(this, &PrimaryExpressionRule::PostIdExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.pe.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.pe.reset(context.fromLiteral);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.pe.reset(new ThisAccess);
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.pe.reset(context.fromExpression);
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.pe.reset(context.fromIdExpression);
    }
    void PostLiteral(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromLiteral_value = std::move(stack.top());
            context.fromLiteral = *static_cast<cminor::parsing::ValueObject<cminor::pom::Literal*>*>(fromLiteral_value.get());
            stack.pop();
        }
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context.fromExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PostIdExpression(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIdExpression_value = std::move(stack.top());
            context.fromIdExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::IdExpr*>*>(fromIdExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), pe(), fromLiteral(), fromExpression(), fromIdExpression() {}
        cminor::pom::CppObject* value;
        std::unique_ptr<CppObject> pe;
        cminor::pom::Literal* fromLiteral;
        cminor::pom::CppObject* fromExpression;
        cminor::pom::IdExpr* fromIdExpression;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ExpressionGrammar::IdExpressionRule : public cminor::parsing::Rule
{
public:
    IdExpressionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("cminor::pom::IdExpr*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::IdExpr*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IdExpressionRule>(this, &IdExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IdExpressionRule>(this, &IdExpressionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IdExpressionRule>(this, &IdExpressionRule::A2Action));
        cminor::parsing::NonterminalParser* id1NonterminalParser = GetNonterminal("id1");
        id1NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IdExpressionRule>(this, &IdExpressionRule::Postid1));
        cminor::parsing::NonterminalParser* id2NonterminalParser = GetNonterminal("id2");
        id2NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IdExpressionRule>(this, &IdExpressionRule::Postid2));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new IdExpr(std::string(matchBegin, matchEnd));
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new IdExpr(std::string(matchBegin, matchEnd));
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new IdExpr(std::string(matchBegin, matchEnd));
    }
    void Postid1(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromid1_value = std::move(stack.top());
            context.fromid1 = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromid1_value.get());
            stack.pop();
        }
    }
    void Postid2(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromid2_value = std::move(stack.top());
            context.fromid2 = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromid2_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromid1(), fromid2() {}
        cminor::pom::IdExpr* value;
        std::string fromid1;
        std::string fromid2;
    };
    std::stack<Context> contextStack;
    Context context;
};

void ExpressionGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("Cm.Parsing.Cpp.LiteralGrammar");
    if (!grammar0)
    {
        grammar0 = cpg::cpp::LiteralGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar("Cm.Parsing.Cpp.DeclarationGrammar");
    if (!grammar1)
    {
        grammar1 = cpg::cpp::DeclarationGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar("Cm.Parsing.Cpp.IdentifierGrammar");
    if (!grammar2)
    {
        grammar2 = cpg::cpp::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar("Cm.Parsing.Cpp.DeclaratorGrammar");
    if (!grammar3)
    {
        grammar3 = cpg::cpp::DeclaratorGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
}

void ExpressionGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("SimpleTypeSpecifier", this, "DeclarationGrammar.SimpleTypeSpecifier"));
    AddRuleLink(new cminor::parsing::RuleLink("Literal", this, "LiteralGrammar.Literal"));
    AddRuleLink(new cminor::parsing::RuleLink("QualifiedId", this, "IdentifierGrammar.QualifiedId"));
    AddRuleLink(new cminor::parsing::RuleLink("TypeId", this, "DeclaratorGrammar.TypeId"));
    AddRuleLink(new cminor::parsing::RuleLink("TypeSpecifierSeq", this, "DeclaratorGrammar.TypeSpecifierSeq"));
    AddRuleLink(new cminor::parsing::RuleLink("PtrOperator", this, "DeclaratorGrammar.PtrOperator"));
    AddRuleLink(new cminor::parsing::RuleLink("TypeName", this, "DeclarationGrammar.TypeName"));
    AddRule(new ExpressionRule("Expression", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("left", "AssignmentExpression", 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser(','),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::NonterminalParser("right", "AssignmentExpression", 0)))))));
    AddRule(new ConstantExpressionRule("ConstantExpression", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::NonterminalParser("Expression", "Expression", 0))));
    AddRule(new AssignmentExpressionRule("AssignmentExpression", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::ActionParser("A1",
                                new cminor::parsing::NonterminalParser("LogicalOrExpression", "LogicalOrExpression", 0)),
                            new cminor::parsing::NonterminalParser("AssingmentOp", "AssingmentOp", 0)),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("AssignmentExpression", "AssignmentExpression", 0)))),
                new cminor::parsing::ActionParser("A2",
                    new cminor::parsing::NonterminalParser("ConditionalExpression", "ConditionalExpression", 0))),
            new cminor::parsing::ActionParser("A3",
                new cminor::parsing::NonterminalParser("ThrowExpression", "ThrowExpression", 0)))));
    AddRule(new AssingmentOpRule("AssingmentOp", GetScope(),
        new cminor::parsing::ActionParser("A0",
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
                                                    new cminor::parsing::CharParser('='),
                                                    new cminor::parsing::StringParser("*=")),
                                                new cminor::parsing::StringParser("/=")),
                                            new cminor::parsing::StringParser("%=")),
                                        new cminor::parsing::StringParser("+=")),
                                    new cminor::parsing::StringParser("-=")),
                                new cminor::parsing::StringParser(">>=")),
                            new cminor::parsing::StringParser("<<=")),
                        new cminor::parsing::StringParser("&=")),
                    new cminor::parsing::StringParser("^=")),
                new cminor::parsing::StringParser("|=")))));
    AddRule(new ThrowExpressionRule("ThrowExpression", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser("throw"),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::NonterminalParser("AssignmentExpression", "AssignmentExpression", 0))))));
    AddRule(new ConditionalExpressionRule("ConditionalExpression", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("left", "LogicalOrExpression", 0)),
            new cminor::parsing::OptionalParser(
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::CharParser('?'),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::NonterminalParser("Expression", "Expression", 0))),
                            new cminor::parsing::CharParser(':')),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("AssignmentExpression", "AssignmentExpression", 0))))))));
    AddRule(new LogicalOrExpressionRule("LogicalOrExpression", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("left", "LogicalAndExpression", 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::StringParser("||"),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("right", "LogicalAndExpression", 0))))))));
    AddRule(new LogicalAndExpressionRule("LogicalAndExpression", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("left", "InclusiveOrExpression", 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::StringParser("&&"),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("right", "InclusiveOrExpression", 0))))))));
    AddRule(new InclusiveOrExpressionRule("InclusiveOrExpression", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("left", "ExclusiveOrExpression", 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::DifferenceParser(
                        new cminor::parsing::CharParser('|'),
                        new cminor::parsing::StringParser("||")),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("right", "ExclusiveOrExpression", 0))))))));
    AddRule(new ExclusiveOrExpressionRule("ExclusiveOrExpression", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("left", "AndExpression", 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('^'),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("right", "AndExpression", 0))))))));
    AddRule(new AndExpressionRule("AndExpression", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("left", "EqualityExpression", 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::DifferenceParser(
                        new cminor::parsing::CharParser('&'),
                        new cminor::parsing::StringParser("&&")),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("right", "EqualityExpression", 0))))))));
    AddRule(new EqualityExpressionRule("EqualityExpression", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("left", "RelationalExpression", 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser("EqOp", "EqOp", 0),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("right", "RelationalExpression", 0))))))));
    AddRule(new EqOpRule("EqOp", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::StringParser("==")),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::StringParser("!=")))));
    AddRule(new RelationalExpressionRule("RelationalExpression", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("left", "ShiftExpression", 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser("RelOp", "RelOp", 0),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("right", "ShiftExpression", 0))))))));
    AddRule(new RelOpRule("RelOp", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::ActionParser("A0",
                        new cminor::parsing::StringParser("<=")),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::StringParser(">="))),
                new cminor::parsing::ActionParser("A2",
                    new cminor::parsing::DifferenceParser(
                        new cminor::parsing::CharParser('<'),
                        new cminor::parsing::StringParser("<<")))),
            new cminor::parsing::ActionParser("A3",
                new cminor::parsing::DifferenceParser(
                    new cminor::parsing::CharParser('>'),
                    new cminor::parsing::StringParser(">>"))))));
    AddRule(new ShiftExpressionRule("ShiftExpression", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("left", "AdditiveExpression", 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser("ShiftOp", "ShiftOp", 0),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("right", "AdditiveExpression", 0))))))));
    AddRule(new ShiftOpRule("ShiftOp", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::DifferenceParser(
                    new cminor::parsing::StringParser("<<"),
                    new cminor::parsing::StringParser("<<="))),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::DifferenceParser(
                    new cminor::parsing::StringParser(">>"),
                    new cminor::parsing::StringParser(">>="))))));
    AddRule(new AdditiveExpressionRule("AdditiveExpression", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("left", "MultiplicativeExpression", 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser("AddOp", "AddOp", 0),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("right", "MultiplicativeExpression", 0))))))));
    AddRule(new AddOpRule("AddOp", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::DifferenceParser(
                    new cminor::parsing::CharParser('+'),
                    new cminor::parsing::StringParser("+="))),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::DifferenceParser(
                    new cminor::parsing::CharParser('-'),
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::StringParser("-="),
                        new cminor::parsing::StringParser("->")))))));
    AddRule(new MultiplicativeExpressionRule("MultiplicativeExpression", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("left", "PmExpression", 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser("MulOp", "MulOp", 0),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("right", "PmExpression", 0))))))));
    AddRule(new MulOpRule("MulOp", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::DifferenceParser(
                        new cminor::parsing::CharParser('*'),
                        new cminor::parsing::StringParser("*="))),
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::DifferenceParser(
                        new cminor::parsing::CharParser('/'),
                        new cminor::parsing::StringParser("/=")))),
            new cminor::parsing::ActionParser("A2",
                new cminor::parsing::DifferenceParser(
                    new cminor::parsing::CharParser('%'),
                    new cminor::parsing::StringParser("%="))))));
    AddRule(new PmExpressionRule("PmExpression", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("left", "CastExpression", 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser("PmOp", "PmOp", 0),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("right", "CastExpression", 0))))))));
    AddRule(new PmOpRule("PmOp", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::StringParser(".*")),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::StringParser("->*")))));
    AddRule(new CastExpressionRule("CastExpression", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::CharParser('('),
                                new cminor::parsing::ActionParser("A2",
                                    new cminor::parsing::NonterminalParser("TypeId", "TypeId", 0))),
                            new cminor::parsing::CharParser(')')),
                        new cminor::parsing::NonterminalParser("CastExpression", "CastExpression", 0))),
                new cminor::parsing::ActionParser("A3",
                    new cminor::parsing::NonterminalParser("UnaryExpression", "UnaryExpression", 0))))));
    AddRule(new UnaryExpressionRule("UnaryExpression", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::AlternativeParser(
                                    new cminor::parsing::AlternativeParser(
                                        new cminor::parsing::AlternativeParser(
                                            new cminor::parsing::ActionParser("A1",
                                                new cminor::parsing::NonterminalParser("PostfixExpression", "PostfixExpression", 0)),
                                            new cminor::parsing::ActionParser("A2",
                                                new cminor::parsing::NonterminalParser("PostCastExpression", "PostCastExpression", 0))),
                                        new cminor::parsing::ActionParser("A3",
                                            new cminor::parsing::SequenceParser(
                                                new cminor::parsing::StringParser("++"),
                                                new cminor::parsing::ExpectationParser(
                                                    new cminor::parsing::NonterminalParser("e1", "UnaryExpression", 0))))),
                                    new cminor::parsing::ActionParser("A4",
                                        new cminor::parsing::SequenceParser(
                                            new cminor::parsing::StringParser("--"),
                                            new cminor::parsing::ExpectationParser(
                                                new cminor::parsing::NonterminalParser("e2", "UnaryExpression", 0))))),
                                new cminor::parsing::ActionParser("A5",
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::NonterminalParser("UnaryOperator", "UnaryOperator", 0),
                                        new cminor::parsing::ExpectationParser(
                                            new cminor::parsing::NonterminalParser("CastExpression", "CastExpression", 0))))),
                            new cminor::parsing::ActionParser("A6",
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::SequenceParser(
                                            new cminor::parsing::KeywordParser("sizeof"),
                                            new cminor::parsing::CharParser('(')),
                                        new cminor::parsing::ExpectationParser(
                                            new cminor::parsing::NonterminalParser("TypeId", "TypeId", 0))),
                                    new cminor::parsing::ExpectationParser(
                                        new cminor::parsing::CharParser(')'))))),
                        new cminor::parsing::ActionParser("A7",
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::KeywordParser("sizeof"),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::NonterminalParser("e3", "UnaryExpression", 0))))),
                    new cminor::parsing::ActionParser("A8",
                        new cminor::parsing::NonterminalParser("NewExpression", "NewExpression", 0))),
                new cminor::parsing::ActionParser("A9",
                    new cminor::parsing::NonterminalParser("DeleteExpression", "DeleteExpression", 0))))));
    AddRule(new UnaryOperatorRule("UnaryOperator", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::ActionParser("A0",
                                new cminor::parsing::DifferenceParser(
                                    new cminor::parsing::CharParser('*'),
                                    new cminor::parsing::StringParser("*="))),
                            new cminor::parsing::ActionParser("A1",
                                new cminor::parsing::DifferenceParser(
                                    new cminor::parsing::CharParser('&'),
                                    new cminor::parsing::StringParser("&=")))),
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::DifferenceParser(
                                new cminor::parsing::CharParser('+'),
                                new cminor::parsing::StringParser("+=")))),
                    new cminor::parsing::ActionParser("A3",
                        new cminor::parsing::DifferenceParser(
                            new cminor::parsing::CharParser('-'),
                            new cminor::parsing::StringParser("-=")))),
                new cminor::parsing::ActionParser("A4",
                    new cminor::parsing::DifferenceParser(
                        new cminor::parsing::CharParser('!'),
                        new cminor::parsing::StringParser("!=")))),
            new cminor::parsing::ActionParser("A5",
                new cminor::parsing::CharParser('~')))));
    AddRule(new NewExpressionRule("NewExpression", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::ActionParser("A1",
                                    new cminor::parsing::StringParser("::"))),
                            new cminor::parsing::KeywordParser("new")),
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::NonterminalParser("NewPlacement", "NewPlacement", 0))),
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::NonterminalParser("NewTypeId", "NewTypeId", 0)),
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::CharParser('('),
                                new cminor::parsing::ActionParser("A3",
                                    new cminor::parsing::NonterminalParser("TypeId", "TypeId", 0))),
                            new cminor::parsing::CharParser(')')))),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::NonterminalParser("NewInitializer", "NewInitializer", 0))))));
    AddRule(new NewTypeIdRule("NewTypeId", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::EmptyParser()),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::NonterminalParser("TypeSpecifierSeq", "TypeSpecifierSeq", 1),
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::NonterminalParser("TypeName", "TypeName", 0))),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::ActionParser("A3",
                            new cminor::parsing::NonterminalParser("NewDeclarator", "NewDeclarator", 0))))))));
    AddRule(new NewDeclaratorRule("NewDeclarator", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser("PtrOperator", "PtrOperator", 0),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser("NewDeclarator", "NewDeclarator", 0))),
                new cminor::parsing::NonterminalParser("DirectNewDeclarator", "DirectNewDeclarator", 0)))));
    AddRule(new DirectNewDeclaratorRule("DirectNewDeclarator", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('['),
                    new cminor::parsing::ActionParser("A0",
                        new cminor::parsing::NonterminalParser("Expression", "Expression", 0))),
                new cminor::parsing::CharParser(']')),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::CharParser('['),
                        new cminor::parsing::ActionParser("A1",
                            new cminor::parsing::NonterminalParser("ConstantExpression", "ConstantExpression", 0))),
                    new cminor::parsing::CharParser(']'))))));
    AddRule(new NewPlacementRule("NewPlacement", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('('),
                    new cminor::parsing::NonterminalParser("ExpressionList", "ExpressionList", 0)),
                new cminor::parsing::CharParser(')')))));
    AddRule(new NewInitializerRule("NewInitializer", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('('),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser("ExpressionList", "ExpressionList", 0))),
                new cminor::parsing::CharParser(')')))));
    AddRule(new DeleteExpressionRule("DeleteExpression", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::ActionParser("A1",
                                new cminor::parsing::StringParser("::"))),
                        new cminor::parsing::KeywordParser("delete")),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::CharParser('['),
                            new cminor::parsing::ActionParser("A2",
                                new cminor::parsing::CharParser(']'))))),
                new cminor::parsing::NonterminalParser("CastExpression", "CastExpression", 0)))));
    AddRule(new PostfixExpressionRule("PostfixExpression", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::NonterminalParser("PrimaryExpression", "PrimaryExpression", 0)),
                    new cminor::parsing::KleeneStarParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::AlternativeParser(
                                    new cminor::parsing::AlternativeParser(
                                        new cminor::parsing::AlternativeParser(
                                            new cminor::parsing::ActionParser("A2",
                                                new cminor::parsing::SequenceParser(
                                                    new cminor::parsing::SequenceParser(
                                                        new cminor::parsing::CharParser('['),
                                                        new cminor::parsing::ExpectationParser(
                                                            new cminor::parsing::NonterminalParser("index", "Expression", 0))),
                                                    new cminor::parsing::ExpectationParser(
                                                        new cminor::parsing::CharParser(']')))),
                                            new cminor::parsing::ActionParser("A3",
                                                new cminor::parsing::SequenceParser(
                                                    new cminor::parsing::SequenceParser(
                                                        new cminor::parsing::CharParser('('),
                                                        new cminor::parsing::OptionalParser(
                                                            new cminor::parsing::NonterminalParser("invoke", "ExpressionList", 0))),
                                                    new cminor::parsing::ExpectationParser(
                                                        new cminor::parsing::CharParser(')'))))),
                                        new cminor::parsing::ActionParser("A4",
                                            new cminor::parsing::SequenceParser(
                                                new cminor::parsing::DifferenceParser(
                                                    new cminor::parsing::CharParser('.'),
                                                    new cminor::parsing::StringParser(".*")),
                                                new cminor::parsing::ExpectationParser(
                                                    new cminor::parsing::NonterminalParser("member", "IdExpression", 0))))),
                                    new cminor::parsing::ActionParser("A5",
                                        new cminor::parsing::SequenceParser(
                                            new cminor::parsing::DifferenceParser(
                                                new cminor::parsing::StringParser("->"),
                                                new cminor::parsing::StringParser("->*")),
                                            new cminor::parsing::ExpectationParser(
                                                new cminor::parsing::NonterminalParser("ptrmember", "IdExpression", 0))))),
                                new cminor::parsing::ActionParser("A6",
                                    new cminor::parsing::StringParser("++"))),
                            new cminor::parsing::ActionParser("A7",
                                new cminor::parsing::StringParser("--"))))),
                new cminor::parsing::ActionParser("A8",
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::NonterminalParser("TypeSpecifierOrTypeName", "TypeSpecifierOrTypeName", 0),
                                new cminor::parsing::CharParser('(')),
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::NonterminalParser("typeConstruction", "ExpressionList", 0))),
                        new cminor::parsing::CharParser(')')))))));
    AddRule(new TypeSpecifierOrTypeNameRule("TypeSpecifierOrTypeName", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("SimpleTypeSpecifier", "SimpleTypeSpecifier", 0)),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::NonterminalParser("TypeName", "TypeName", 0)))));
    AddRule(new PostCastExpressionRule("PostCastExpression", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::ActionParser("A0",
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::SequenceParser(
                                            new cminor::parsing::SequenceParser(
                                                new cminor::parsing::SequenceParser(
                                                    new cminor::parsing::KeywordParser("dynamic_cast"),
                                                    new cminor::parsing::ExpectationParser(
                                                        new cminor::parsing::CharParser('<'))),
                                                new cminor::parsing::ExpectationParser(
                                                    new cminor::parsing::NonterminalParser("dct", "TypeId", 0))),
                                            new cminor::parsing::ExpectationParser(
                                                new cminor::parsing::CharParser('>'))),
                                        new cminor::parsing::ExpectationParser(
                                            new cminor::parsing::CharParser('('))),
                                    new cminor::parsing::ExpectationParser(
                                        new cminor::parsing::NonterminalParser("dce", "Expression", 0))),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::CharParser(')')))),
                        new cminor::parsing::ActionParser("A1",
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::SequenceParser(
                                            new cminor::parsing::SequenceParser(
                                                new cminor::parsing::SequenceParser(
                                                    new cminor::parsing::KeywordParser("static_cast"),
                                                    new cminor::parsing::ExpectationParser(
                                                        new cminor::parsing::CharParser('<'))),
                                                new cminor::parsing::ExpectationParser(
                                                    new cminor::parsing::NonterminalParser("sct", "TypeId", 0))),
                                            new cminor::parsing::ExpectationParser(
                                                new cminor::parsing::CharParser('>'))),
                                        new cminor::parsing::ExpectationParser(
                                            new cminor::parsing::CharParser('('))),
                                    new cminor::parsing::ExpectationParser(
                                        new cminor::parsing::NonterminalParser("sce", "Expression", 0))),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::CharParser(')'))))),
                    new cminor::parsing::ActionParser("A2",
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::SequenceParser(
                                            new cminor::parsing::SequenceParser(
                                                new cminor::parsing::KeywordParser("reinterpret_cast"),
                                                new cminor::parsing::ExpectationParser(
                                                    new cminor::parsing::CharParser('<'))),
                                            new cminor::parsing::ExpectationParser(
                                                new cminor::parsing::NonterminalParser("rct", "TypeId", 0))),
                                        new cminor::parsing::ExpectationParser(
                                            new cminor::parsing::CharParser('>'))),
                                    new cminor::parsing::ExpectationParser(
                                        new cminor::parsing::CharParser('('))),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::NonterminalParser("rce", "Expression", 0))),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::CharParser(')'))))),
                new cminor::parsing::ActionParser("A3",
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::SequenceParser(
                                            new cminor::parsing::KeywordParser("const_cast"),
                                            new cminor::parsing::ExpectationParser(
                                                new cminor::parsing::CharParser('<'))),
                                        new cminor::parsing::ExpectationParser(
                                            new cminor::parsing::NonterminalParser("cct", "TypeId", 0))),
                                    new cminor::parsing::ExpectationParser(
                                        new cminor::parsing::CharParser('>'))),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::CharParser('('))),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("cce", "Expression", 0))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser(')'))))),
            new cminor::parsing::ActionParser("A4",
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::KeywordParser("typeid"),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::CharParser('('))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("typeidExpr", "Expression", 0))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser(')')))))));
    AddRule(new ExpressionListRule("ExpressionList", GetScope(),
        new cminor::parsing::ListParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("AssignmentExpression", "AssignmentExpression", 0)),
            new cminor::parsing::CharParser(','))));
    AddRule(new PrimaryExpressionRule("PrimaryExpression", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::ActionParser("A1",
                            new cminor::parsing::NonterminalParser("Literal", "Literal", 0)),
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::KeywordParser("this"))),
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::CharParser('('),
                            new cminor::parsing::NonterminalParser("Expression", "Expression", 0)),
                        new cminor::parsing::ActionParser("A3",
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::CharParser(')'))))),
                new cminor::parsing::ActionParser("A4",
                    new cminor::parsing::NonterminalParser("IdExpression", "IdExpression", 0))))));
    AddRule(new IdExpressionRule("IdExpression", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::TokenParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::NonterminalParser("id1", "QualifiedId", 0),
                                new cminor::parsing::StringParser("::")),
                            new cminor::parsing::NonterminalParser("OperatorFunctionId", "OperatorFunctionId", 0)))),
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::TokenParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::StringParser("::")),
                            new cminor::parsing::NonterminalParser("OperatorFunctionId", "OperatorFunctionId", 0))))),
            new cminor::parsing::ActionParser("A2",
                new cminor::parsing::NonterminalParser("id2", "QualifiedId", 0)))));
    AddRule(new cminor::parsing::Rule("OperatorFunctionId", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::KeywordParser("operator"),
            new cminor::parsing::NonterminalParser("Operator", "Operator", 0))));
    AddRule(new cminor::parsing::Rule("Operator", GetScope(),
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
                                                                                                                                                        new cminor::parsing::AlternativeParser(
                                                                                                                                                            new cminor::parsing::AlternativeParser(
                                                                                                                                                                new cminor::parsing::AlternativeParser(
                                                                                                                                                                    new cminor::parsing::AlternativeParser(
                                                                                                                                                                        new cminor::parsing::AlternativeParser(
                                                                                                                                                                            new cminor::parsing::SequenceParser(
                                                                                                                                                                                new cminor::parsing::CharParser('['),
                                                                                                                                                                                new cminor::parsing::CharParser(']')),
                                                                                                                                                                            new cminor::parsing::SequenceParser(
                                                                                                                                                                                new cminor::parsing::CharParser('('),
                                                                                                                                                                                new cminor::parsing::CharParser(')'))),
                                                                                                                                                                        new cminor::parsing::StringParser("->*")),
                                                                                                                                                                    new cminor::parsing::StringParser("->")),
                                                                                                                                                                new cminor::parsing::CharParser(',')),
                                                                                                                                                            new cminor::parsing::StringParser("--")),
                                                                                                                                                        new cminor::parsing::StringParser("++")),
                                                                                                                                                    new cminor::parsing::StringParser("||")),
                                                                                                                                                new cminor::parsing::StringParser("&&")),
                                                                                                                                            new cminor::parsing::StringParser("<=")),
                                                                                                                                        new cminor::parsing::StringParser(">=")),
                                                                                                                                    new cminor::parsing::StringParser("!=")),
                                                                                                                                new cminor::parsing::StringParser("==")),
                                                                                                                            new cminor::parsing::StringParser("<<=")),
                                                                                                                        new cminor::parsing::StringParser(">>=")),
                                                                                                                    new cminor::parsing::StringParser("<<")),
                                                                                                                new cminor::parsing::StringParser(">>")),
                                                                                                            new cminor::parsing::StringParser("|=")),
                                                                                                        new cminor::parsing::StringParser("&=")),
                                                                                                    new cminor::parsing::StringParser("^=")),
                                                                                                new cminor::parsing::StringParser("%=")),
                                                                                            new cminor::parsing::StringParser("/=")),
                                                                                        new cminor::parsing::StringParser("*=")),
                                                                                    new cminor::parsing::StringParser("-=")),
                                                                                new cminor::parsing::StringParser("+=")),
                                                                            new cminor::parsing::CharParser('<')),
                                                                        new cminor::parsing::CharParser('>')),
                                                                    new cminor::parsing::CharParser('=')),
                                                                new cminor::parsing::CharParser('!')),
                                                            new cminor::parsing::CharParser('~')),
                                                        new cminor::parsing::CharParser('|')),
                                                    new cminor::parsing::CharParser('&')),
                                                new cminor::parsing::CharParser('^')),
                                            new cminor::parsing::CharParser('%')),
                                        new cminor::parsing::CharParser('/')),
                                    new cminor::parsing::CharParser('*')),
                                new cminor::parsing::CharParser('-')),
                            new cminor::parsing::CharParser('+')),
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::KeywordParser("new"),
                                new cminor::parsing::CharParser('[')),
                            new cminor::parsing::CharParser(']'))),
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::KeywordParser("delete"),
                            new cminor::parsing::CharParser('[')),
                        new cminor::parsing::CharParser(']'))),
                new cminor::parsing::KeywordParser("new")),
            new cminor::parsing::KeywordParser("delete"))));
}

} } // namespace cpg.cpp
