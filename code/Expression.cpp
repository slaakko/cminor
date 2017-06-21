#include "Expression.hpp"
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
#include <cminor/code/Declaration.hpp>
#include <cminor/code/Declarator.hpp>
#include <cminor/code/Literal.hpp>
#include <cminor/code/Identifier.hpp>
#include <cminor/util/TextUtils.hpp>
#include <cminor/codedom/Type.hpp>

namespace cminor { namespace code {

using namespace cminor::codedom;
using cminor::util::Trim;
using namespace cminor::parsing;
using namespace cminor::util;
using namespace cminor::unicode;

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

ExpressionGrammar::ExpressionGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("ExpressionGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.code")), parsingDomain_)
{
    SetOwner(0);
}

cminor::codedom::CppObject* ExpressionGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName)
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
    cminor::codedom::CppObject* result = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(value.get());
    stack.pop();
    return result;
}

class ExpressionGrammar::ExpressionRule : public cminor::parsing::Rule
{
public:
    ExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExpressionRule>(this, &ExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExpressionRule>(this, &ExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal(ToUtf32("left"));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ExpressionRule>(this, &ExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal(ToUtf32("right"));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ExpressionRule>(this, &ExpressionRule::Postright));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromleft;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BinaryOpExpr(context->value, Operator::comma, 0, context->fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context->fromleft = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context->fromright = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromleft(), fromright() {}
        cminor::codedom::CppObject* value;
        cminor::codedom::CppObject* fromleft;
        cminor::codedom::CppObject* fromright;
    };
};

class ExpressionGrammar::ConstantExpressionRule : public cminor::parsing::Rule
{
public:
    ConstantExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConstantExpressionRule>(this, &ConstantExpressionRule::A0Action));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConstantExpressionRule>(this, &ConstantExpressionRule::PostExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromExpression;
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromExpression() {}
        cminor::codedom::CppObject* value;
        cminor::codedom::CppObject* fromExpression;
    };
};

class ExpressionGrammar::AssignmentExpressionRule : public cminor::parsing::Rule
{
public:
    AssignmentExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<CppObject>"), ToUtf32("lor")));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AssignmentExpressionRule>(this, &AssignmentExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AssignmentExpressionRule>(this, &AssignmentExpressionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AssignmentExpressionRule>(this, &AssignmentExpressionRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AssignmentExpressionRule>(this, &AssignmentExpressionRule::A3Action));
        cminor::parsing::NonterminalParser* logicalOrExpressionNonterminalParser = GetNonterminal(ToUtf32("LogicalOrExpression"));
        logicalOrExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AssignmentExpressionRule>(this, &AssignmentExpressionRule::PostLogicalOrExpression));
        cminor::parsing::NonterminalParser* assingmentOpNonterminalParser = GetNonterminal(ToUtf32("AssingmentOp"));
        assingmentOpNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AssignmentExpressionRule>(this, &AssignmentExpressionRule::PostAssingmentOp));
        cminor::parsing::NonterminalParser* assignmentExpressionNonterminalParser = GetNonterminal(ToUtf32("AssignmentExpression"));
        assignmentExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AssignmentExpressionRule>(this, &AssignmentExpressionRule::PostAssignmentExpression));
        cminor::parsing::NonterminalParser* conditionalExpressionNonterminalParser = GetNonterminal(ToUtf32("ConditionalExpression"));
        conditionalExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AssignmentExpressionRule>(this, &AssignmentExpressionRule::PostConditionalExpression));
        cminor::parsing::NonterminalParser* throwExpressionNonterminalParser = GetNonterminal(ToUtf32("ThrowExpression"));
        throwExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AssignmentExpressionRule>(this, &AssignmentExpressionRule::PostThrowExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BinaryOpExpr(context->lor.release(), context->fromAssingmentOp, 1, context->fromAssignmentExpression);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->lor.reset(context->fromLogicalOrExpression);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromConditionalExpression;
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromThrowExpression;
    }
    void PostLogicalOrExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromLogicalOrExpression_value = std::move(stack.top());
            context->fromLogicalOrExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromLogicalOrExpression_value.get());
            stack.pop();
        }
    }
    void PostAssingmentOp(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAssingmentOp_value = std::move(stack.top());
            context->fromAssingmentOp = *static_cast<cminor::parsing::ValueObject<Operator>*>(fromAssingmentOp_value.get());
            stack.pop();
        }
    }
    void PostAssignmentExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAssignmentExpression_value = std::move(stack.top());
            context->fromAssignmentExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromAssignmentExpression_value.get());
            stack.pop();
        }
    }
    void PostConditionalExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromConditionalExpression_value = std::move(stack.top());
            context->fromConditionalExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromConditionalExpression_value.get());
            stack.pop();
        }
    }
    void PostThrowExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromThrowExpression_value = std::move(stack.top());
            context->fromThrowExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromThrowExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), lor(), fromLogicalOrExpression(), fromAssingmentOp(), fromAssignmentExpression(), fromConditionalExpression(), fromThrowExpression() {}
        cminor::codedom::CppObject* value;
        std::unique_ptr<CppObject> lor;
        cminor::codedom::CppObject* fromLogicalOrExpression;
        Operator fromAssingmentOp;
        cminor::codedom::CppObject* fromAssignmentExpression;
        cminor::codedom::CppObject* fromConditionalExpression;
        cminor::codedom::CppObject* fromThrowExpression;
    };
};

class ExpressionGrammar::AssingmentOpRule : public cminor::parsing::Rule
{
public:
    AssingmentOpRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("Operator"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Operator>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AssingmentOpRule>(this, &AssingmentOpRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = GetOperator(std::u32string(matchBegin, matchEnd));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        Operator value;
    };
};

class ExpressionGrammar::ThrowExpressionRule : public cminor::parsing::Rule
{
public:
    ThrowExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ThrowExpressionRule>(this, &ThrowExpressionRule::A0Action));
        cminor::parsing::NonterminalParser* assignmentExpressionNonterminalParser = GetNonterminal(ToUtf32("AssignmentExpression"));
        assignmentExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ThrowExpressionRule>(this, &ThrowExpressionRule::PostAssignmentExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ThrowExpr(context->fromAssignmentExpression);
    }
    void PostAssignmentExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAssignmentExpression_value = std::move(stack.top());
            context->fromAssignmentExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromAssignmentExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromAssignmentExpression() {}
        cminor::codedom::CppObject* value;
        cminor::codedom::CppObject* fromAssignmentExpression;
    };
};

class ExpressionGrammar::ConditionalExpressionRule : public cminor::parsing::Rule
{
public:
    ConditionalExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConditionalExpressionRule>(this, &ConditionalExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConditionalExpressionRule>(this, &ConditionalExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal(ToUtf32("left"));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConditionalExpressionRule>(this, &ConditionalExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConditionalExpressionRule>(this, &ConditionalExpressionRule::PostExpression));
        cminor::parsing::NonterminalParser* assignmentExpressionNonterminalParser = GetNonterminal(ToUtf32("AssignmentExpression"));
        assignmentExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConditionalExpressionRule>(this, &ConditionalExpressionRule::PostAssignmentExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromleft;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ConditionalExpr(context->value, context->fromExpression, context->fromAssignmentExpression);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context->fromleft = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PostAssignmentExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAssignmentExpression_value = std::move(stack.top());
            context->fromAssignmentExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromAssignmentExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromleft(), fromExpression(), fromAssignmentExpression() {}
        cminor::codedom::CppObject* value;
        cminor::codedom::CppObject* fromleft;
        cminor::codedom::CppObject* fromExpression;
        cminor::codedom::CppObject* fromAssignmentExpression;
    };
};

class ExpressionGrammar::LogicalOrExpressionRule : public cminor::parsing::Rule
{
public:
    LogicalOrExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LogicalOrExpressionRule>(this, &LogicalOrExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LogicalOrExpressionRule>(this, &LogicalOrExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal(ToUtf32("left"));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LogicalOrExpressionRule>(this, &LogicalOrExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal(ToUtf32("right"));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LogicalOrExpressionRule>(this, &LogicalOrExpressionRule::Postright));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromleft;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BinaryOpExpr(context->value, Operator::or_, 2, context->fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context->fromleft = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context->fromright = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromleft(), fromright() {}
        cminor::codedom::CppObject* value;
        cminor::codedom::CppObject* fromleft;
        cminor::codedom::CppObject* fromright;
    };
};

class ExpressionGrammar::LogicalAndExpressionRule : public cminor::parsing::Rule
{
public:
    LogicalAndExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LogicalAndExpressionRule>(this, &LogicalAndExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LogicalAndExpressionRule>(this, &LogicalAndExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal(ToUtf32("left"));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LogicalAndExpressionRule>(this, &LogicalAndExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal(ToUtf32("right"));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<LogicalAndExpressionRule>(this, &LogicalAndExpressionRule::Postright));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromleft;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BinaryOpExpr(context->value, Operator::and_, 3, context->fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context->fromleft = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context->fromright = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromleft(), fromright() {}
        cminor::codedom::CppObject* value;
        cminor::codedom::CppObject* fromleft;
        cminor::codedom::CppObject* fromright;
    };
};

class ExpressionGrammar::InclusiveOrExpressionRule : public cminor::parsing::Rule
{
public:
    InclusiveOrExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InclusiveOrExpressionRule>(this, &InclusiveOrExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InclusiveOrExpressionRule>(this, &InclusiveOrExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal(ToUtf32("left"));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InclusiveOrExpressionRule>(this, &InclusiveOrExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal(ToUtf32("right"));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InclusiveOrExpressionRule>(this, &InclusiveOrExpressionRule::Postright));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromleft;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BinaryOpExpr(context->value, Operator::bitor_, 4, context->fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context->fromleft = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context->fromright = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromleft(), fromright() {}
        cminor::codedom::CppObject* value;
        cminor::codedom::CppObject* fromleft;
        cminor::codedom::CppObject* fromright;
    };
};

class ExpressionGrammar::ExclusiveOrExpressionRule : public cminor::parsing::Rule
{
public:
    ExclusiveOrExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExclusiveOrExpressionRule>(this, &ExclusiveOrExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExclusiveOrExpressionRule>(this, &ExclusiveOrExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal(ToUtf32("left"));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ExclusiveOrExpressionRule>(this, &ExclusiveOrExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal(ToUtf32("right"));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ExclusiveOrExpressionRule>(this, &ExclusiveOrExpressionRule::Postright));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromleft;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BinaryOpExpr(context->value, Operator::bitxor, 5, context->fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context->fromleft = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context->fromright = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromleft(), fromright() {}
        cminor::codedom::CppObject* value;
        cminor::codedom::CppObject* fromleft;
        cminor::codedom::CppObject* fromright;
    };
};

class ExpressionGrammar::AndExpressionRule : public cminor::parsing::Rule
{
public:
    AndExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AndExpressionRule>(this, &AndExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AndExpressionRule>(this, &AndExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal(ToUtf32("left"));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AndExpressionRule>(this, &AndExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal(ToUtf32("right"));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AndExpressionRule>(this, &AndExpressionRule::Postright));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromleft;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BinaryOpExpr(context->value, Operator::bitand_, 6, context->fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context->fromleft = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context->fromright = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromleft(), fromright() {}
        cminor::codedom::CppObject* value;
        cminor::codedom::CppObject* fromleft;
        cminor::codedom::CppObject* fromright;
    };
};

class ExpressionGrammar::EqualityExpressionRule : public cminor::parsing::Rule
{
public:
    EqualityExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EqualityExpressionRule>(this, &EqualityExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EqualityExpressionRule>(this, &EqualityExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal(ToUtf32("left"));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<EqualityExpressionRule>(this, &EqualityExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* eqOpNonterminalParser = GetNonterminal(ToUtf32("EqOp"));
        eqOpNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<EqualityExpressionRule>(this, &EqualityExpressionRule::PostEqOp));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal(ToUtf32("right"));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<EqualityExpressionRule>(this, &EqualityExpressionRule::Postright));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromleft;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BinaryOpExpr(context->value, context->fromEqOp, 7, context->fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context->fromleft = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void PostEqOp(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromEqOp_value = std::move(stack.top());
            context->fromEqOp = *static_cast<cminor::parsing::ValueObject<Operator>*>(fromEqOp_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context->fromright = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromleft(), fromEqOp(), fromright() {}
        cminor::codedom::CppObject* value;
        cminor::codedom::CppObject* fromleft;
        Operator fromEqOp;
        cminor::codedom::CppObject* fromright;
    };
};

class ExpressionGrammar::EqOpRule : public cminor::parsing::Rule
{
public:
    EqOpRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("Operator"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Operator>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EqOpRule>(this, &EqOpRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EqOpRule>(this, &EqOpRule::A1Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Operator::eq;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Operator::notEq;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        Operator value;
    };
};

class ExpressionGrammar::RelationalExpressionRule : public cminor::parsing::Rule
{
public:
    RelationalExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RelationalExpressionRule>(this, &RelationalExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RelationalExpressionRule>(this, &RelationalExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal(ToUtf32("left"));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RelationalExpressionRule>(this, &RelationalExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* relOpNonterminalParser = GetNonterminal(ToUtf32("RelOp"));
        relOpNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RelationalExpressionRule>(this, &RelationalExpressionRule::PostRelOp));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal(ToUtf32("right"));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RelationalExpressionRule>(this, &RelationalExpressionRule::Postright));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromleft;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BinaryOpExpr(context->value, context->fromRelOp, 8, context->fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context->fromleft = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void PostRelOp(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromRelOp_value = std::move(stack.top());
            context->fromRelOp = *static_cast<cminor::parsing::ValueObject<Operator>*>(fromRelOp_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context->fromright = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromleft(), fromRelOp(), fromright() {}
        cminor::codedom::CppObject* value;
        cminor::codedom::CppObject* fromleft;
        Operator fromRelOp;
        cminor::codedom::CppObject* fromright;
    };
};

class ExpressionGrammar::RelOpRule : public cminor::parsing::Rule
{
public:
    RelOpRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("Operator"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Operator>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RelOpRule>(this, &RelOpRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RelOpRule>(this, &RelOpRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RelOpRule>(this, &RelOpRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RelOpRule>(this, &RelOpRule::A3Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Operator::lessOrEq;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Operator::greaterOrEq;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Operator::less;
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Operator::greater;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        Operator value;
    };
};

class ExpressionGrammar::ShiftExpressionRule : public cminor::parsing::Rule
{
public:
    ShiftExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ShiftExpressionRule>(this, &ShiftExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ShiftExpressionRule>(this, &ShiftExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal(ToUtf32("left"));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ShiftExpressionRule>(this, &ShiftExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* shiftOpNonterminalParser = GetNonterminal(ToUtf32("ShiftOp"));
        shiftOpNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ShiftExpressionRule>(this, &ShiftExpressionRule::PostShiftOp));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal(ToUtf32("right"));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ShiftExpressionRule>(this, &ShiftExpressionRule::Postright));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromleft;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BinaryOpExpr(context->value, context->fromShiftOp, 9, context->fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context->fromleft = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void PostShiftOp(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromShiftOp_value = std::move(stack.top());
            context->fromShiftOp = *static_cast<cminor::parsing::ValueObject<Operator>*>(fromShiftOp_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context->fromright = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromleft(), fromShiftOp(), fromright() {}
        cminor::codedom::CppObject* value;
        cminor::codedom::CppObject* fromleft;
        Operator fromShiftOp;
        cminor::codedom::CppObject* fromright;
    };
};

class ExpressionGrammar::ShiftOpRule : public cminor::parsing::Rule
{
public:
    ShiftOpRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("Operator"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Operator>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ShiftOpRule>(this, &ShiftOpRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ShiftOpRule>(this, &ShiftOpRule::A1Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Operator::shiftLeft;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Operator::shiftRight;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        Operator value;
    };
};

class ExpressionGrammar::AdditiveExpressionRule : public cminor::parsing::Rule
{
public:
    AdditiveExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AdditiveExpressionRule>(this, &AdditiveExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AdditiveExpressionRule>(this, &AdditiveExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal(ToUtf32("left"));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AdditiveExpressionRule>(this, &AdditiveExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* addOpNonterminalParser = GetNonterminal(ToUtf32("AddOp"));
        addOpNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AdditiveExpressionRule>(this, &AdditiveExpressionRule::PostAddOp));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal(ToUtf32("right"));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AdditiveExpressionRule>(this, &AdditiveExpressionRule::Postright));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromleft;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BinaryOpExpr(context->value, context->fromAddOp, 10, context->fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context->fromleft = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void PostAddOp(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAddOp_value = std::move(stack.top());
            context->fromAddOp = *static_cast<cminor::parsing::ValueObject<Operator>*>(fromAddOp_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context->fromright = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromleft(), fromAddOp(), fromright() {}
        cminor::codedom::CppObject* value;
        cminor::codedom::CppObject* fromleft;
        Operator fromAddOp;
        cminor::codedom::CppObject* fromright;
    };
};

class ExpressionGrammar::AddOpRule : public cminor::parsing::Rule
{
public:
    AddOpRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("Operator"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Operator>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AddOpRule>(this, &AddOpRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AddOpRule>(this, &AddOpRule::A1Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Operator::plus;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Operator::minus;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        Operator value;
    };
};

class ExpressionGrammar::MultiplicativeExpressionRule : public cminor::parsing::Rule
{
public:
    MultiplicativeExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MultiplicativeExpressionRule>(this, &MultiplicativeExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MultiplicativeExpressionRule>(this, &MultiplicativeExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal(ToUtf32("left"));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<MultiplicativeExpressionRule>(this, &MultiplicativeExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* mulOpNonterminalParser = GetNonterminal(ToUtf32("MulOp"));
        mulOpNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<MultiplicativeExpressionRule>(this, &MultiplicativeExpressionRule::PostMulOp));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal(ToUtf32("right"));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<MultiplicativeExpressionRule>(this, &MultiplicativeExpressionRule::Postright));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromleft;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BinaryOpExpr(context->value, context->fromMulOp, 11, context->fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context->fromleft = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void PostMulOp(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromMulOp_value = std::move(stack.top());
            context->fromMulOp = *static_cast<cminor::parsing::ValueObject<Operator>*>(fromMulOp_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context->fromright = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromleft(), fromMulOp(), fromright() {}
        cminor::codedom::CppObject* value;
        cminor::codedom::CppObject* fromleft;
        Operator fromMulOp;
        cminor::codedom::CppObject* fromright;
    };
};

class ExpressionGrammar::MulOpRule : public cminor::parsing::Rule
{
public:
    MulOpRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("Operator"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Operator>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MulOpRule>(this, &MulOpRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MulOpRule>(this, &MulOpRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MulOpRule>(this, &MulOpRule::A2Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Operator::mul;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Operator::div;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Operator::rem;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        Operator value;
    };
};

class ExpressionGrammar::PmExpressionRule : public cminor::parsing::Rule
{
public:
    PmExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PmExpressionRule>(this, &PmExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PmExpressionRule>(this, &PmExpressionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal(ToUtf32("left"));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PmExpressionRule>(this, &PmExpressionRule::Postleft));
        cminor::parsing::NonterminalParser* pmOpNonterminalParser = GetNonterminal(ToUtf32("PmOp"));
        pmOpNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PmExpressionRule>(this, &PmExpressionRule::PostPmOp));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal(ToUtf32("right"));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PmExpressionRule>(this, &PmExpressionRule::Postright));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromleft;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BinaryOpExpr(context->value, context->fromPmOp, 12, context->fromright);
    }
    void Postleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context->fromleft = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void PostPmOp(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromPmOp_value = std::move(stack.top());
            context->fromPmOp = *static_cast<cminor::parsing::ValueObject<Operator>*>(fromPmOp_value.get());
            stack.pop();
        }
    }
    void Postright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context->fromright = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromleft(), fromPmOp(), fromright() {}
        cminor::codedom::CppObject* value;
        cminor::codedom::CppObject* fromleft;
        Operator fromPmOp;
        cminor::codedom::CppObject* fromright;
    };
};

class ExpressionGrammar::PmOpRule : public cminor::parsing::Rule
{
public:
    PmOpRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("Operator"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Operator>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PmOpRule>(this, &PmOpRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PmOpRule>(this, &PmOpRule::A1Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Operator::dotStar;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Operator::arrowStar;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        Operator value;
    };
};

class ExpressionGrammar::CastExpressionRule : public cminor::parsing::Rule
{
public:
    CastExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<CppObject>"), ToUtf32("ce")));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<CppObject>"), ToUtf32("ti")));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CastExpressionRule>(this, &CastExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CastExpressionRule>(this, &CastExpressionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CastExpressionRule>(this, &CastExpressionRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CastExpressionRule>(this, &CastExpressionRule::A3Action));
        cminor::parsing::NonterminalParser* typeIdNonterminalParser = GetNonterminal(ToUtf32("TypeId"));
        typeIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CastExpressionRule>(this, &CastExpressionRule::PostTypeId));
        cminor::parsing::NonterminalParser* castExpressionNonterminalParser = GetNonterminal(ToUtf32("CastExpression"));
        castExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CastExpressionRule>(this, &CastExpressionRule::PostCastExpression));
        cminor::parsing::NonterminalParser* unaryExpressionNonterminalParser = GetNonterminal(ToUtf32("UnaryExpression"));
        unaryExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CastExpressionRule>(this, &CastExpressionRule::PostUnaryExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->ce.release();
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ce.reset(new CastExpr(context->ti.release(), context->fromCastExpression));
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ti.reset(context->fromTypeId);
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ce.reset(context->fromUnaryExpression);
    }
    void PostTypeId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeId_value = std::move(stack.top());
            context->fromTypeId = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeId*>*>(fromTypeId_value.get());
            stack.pop();
        }
    }
    void PostCastExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCastExpression_value = std::move(stack.top());
            context->fromCastExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromCastExpression_value.get());
            stack.pop();
        }
    }
    void PostUnaryExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromUnaryExpression_value = std::move(stack.top());
            context->fromUnaryExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromUnaryExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), ce(), ti(), fromTypeId(), fromCastExpression(), fromUnaryExpression() {}
        cminor::codedom::CppObject* value;
        std::unique_ptr<CppObject> ce;
        std::unique_ptr<CppObject> ti;
        cminor::codedom::TypeId* fromTypeId;
        cminor::codedom::CppObject* fromCastExpression;
        cminor::codedom::CppObject* fromUnaryExpression;
    };
};

class ExpressionGrammar::UnaryExpressionRule : public cminor::parsing::Rule
{
public:
    UnaryExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<CppObject>"), ToUtf32("ue")));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryExpressionRule>(this, &UnaryExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryExpressionRule>(this, &UnaryExpressionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryExpressionRule>(this, &UnaryExpressionRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryExpressionRule>(this, &UnaryExpressionRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction(ToUtf32("A4"));
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryExpressionRule>(this, &UnaryExpressionRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction(ToUtf32("A5"));
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryExpressionRule>(this, &UnaryExpressionRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction(ToUtf32("A6"));
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryExpressionRule>(this, &UnaryExpressionRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction(ToUtf32("A7"));
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryExpressionRule>(this, &UnaryExpressionRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction(ToUtf32("A8"));
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryExpressionRule>(this, &UnaryExpressionRule::A8Action));
        cminor::parsing::ActionParser* a9ActionParser = GetAction(ToUtf32("A9"));
        a9ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryExpressionRule>(this, &UnaryExpressionRule::A9Action));
        cminor::parsing::NonterminalParser* postfixExpressionNonterminalParser = GetNonterminal(ToUtf32("PostfixExpression"));
        postfixExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UnaryExpressionRule>(this, &UnaryExpressionRule::PostPostfixExpression));
        cminor::parsing::NonterminalParser* postCastExpressionNonterminalParser = GetNonterminal(ToUtf32("PostCastExpression"));
        postCastExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UnaryExpressionRule>(this, &UnaryExpressionRule::PostPostCastExpression));
        cminor::parsing::NonterminalParser* e1NonterminalParser = GetNonterminal(ToUtf32("e1"));
        e1NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UnaryExpressionRule>(this, &UnaryExpressionRule::Poste1));
        cminor::parsing::NonterminalParser* e2NonterminalParser = GetNonterminal(ToUtf32("e2"));
        e2NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UnaryExpressionRule>(this, &UnaryExpressionRule::Poste2));
        cminor::parsing::NonterminalParser* unaryOperatorNonterminalParser = GetNonterminal(ToUtf32("UnaryOperator"));
        unaryOperatorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UnaryExpressionRule>(this, &UnaryExpressionRule::PostUnaryOperator));
        cminor::parsing::NonterminalParser* castExpressionNonterminalParser = GetNonterminal(ToUtf32("CastExpression"));
        castExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UnaryExpressionRule>(this, &UnaryExpressionRule::PostCastExpression));
        cminor::parsing::NonterminalParser* typeIdNonterminalParser = GetNonterminal(ToUtf32("TypeId"));
        typeIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UnaryExpressionRule>(this, &UnaryExpressionRule::PostTypeId));
        cminor::parsing::NonterminalParser* e3NonterminalParser = GetNonterminal(ToUtf32("e3"));
        e3NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UnaryExpressionRule>(this, &UnaryExpressionRule::Poste3));
        cminor::parsing::NonterminalParser* newExpressionNonterminalParser = GetNonterminal(ToUtf32("NewExpression"));
        newExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UnaryExpressionRule>(this, &UnaryExpressionRule::PostNewExpression));
        cminor::parsing::NonterminalParser* deleteExpressionNonterminalParser = GetNonterminal(ToUtf32("DeleteExpression"));
        deleteExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UnaryExpressionRule>(this, &UnaryExpressionRule::PostDeleteExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->ue.release();
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ue.reset(context->fromPostfixExpression);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ue.reset(context->fromPostCastExpression);
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ue.reset(new PreIncrementExpr(context->frome1));
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ue.reset(new PreDecrementExpr(context->frome2));
    }
    void A5Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ue.reset(new UnaryOpExpr(context->fromUnaryOperator, context->fromCastExpression));
    }
    void A6Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ue.reset(new SizeOfExpr(context->fromTypeId, true));
    }
    void A7Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ue.reset(new SizeOfExpr(context->frome3, false));
    }
    void A8Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ue.reset(context->fromNewExpression);
    }
    void A9Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ue.reset(context->fromDeleteExpression);
    }
    void PostPostfixExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromPostfixExpression_value = std::move(stack.top());
            context->fromPostfixExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromPostfixExpression_value.get());
            stack.pop();
        }
    }
    void PostPostCastExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromPostCastExpression_value = std::move(stack.top());
            context->fromPostCastExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromPostCastExpression_value.get());
            stack.pop();
        }
    }
    void Poste1(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frome1_value = std::move(stack.top());
            context->frome1 = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(frome1_value.get());
            stack.pop();
        }
    }
    void Poste2(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frome2_value = std::move(stack.top());
            context->frome2 = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(frome2_value.get());
            stack.pop();
        }
    }
    void PostUnaryOperator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromUnaryOperator_value = std::move(stack.top());
            context->fromUnaryOperator = *static_cast<cminor::parsing::ValueObject<Operator>*>(fromUnaryOperator_value.get());
            stack.pop();
        }
    }
    void PostCastExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCastExpression_value = std::move(stack.top());
            context->fromCastExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromCastExpression_value.get());
            stack.pop();
        }
    }
    void PostTypeId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeId_value = std::move(stack.top());
            context->fromTypeId = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeId*>*>(fromTypeId_value.get());
            stack.pop();
        }
    }
    void Poste3(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frome3_value = std::move(stack.top());
            context->frome3 = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(frome3_value.get());
            stack.pop();
        }
    }
    void PostNewExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromNewExpression_value = std::move(stack.top());
            context->fromNewExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromNewExpression_value.get());
            stack.pop();
        }
    }
    void PostDeleteExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDeleteExpression_value = std::move(stack.top());
            context->fromDeleteExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromDeleteExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), ue(), fromPostfixExpression(), fromPostCastExpression(), frome1(), frome2(), fromUnaryOperator(), fromCastExpression(), fromTypeId(), frome3(), fromNewExpression(), fromDeleteExpression() {}
        cminor::codedom::CppObject* value;
        std::unique_ptr<CppObject> ue;
        cminor::codedom::CppObject* fromPostfixExpression;
        cminor::codedom::CppObject* fromPostCastExpression;
        cminor::codedom::CppObject* frome1;
        cminor::codedom::CppObject* frome2;
        Operator fromUnaryOperator;
        cminor::codedom::CppObject* fromCastExpression;
        cminor::codedom::TypeId* fromTypeId;
        cminor::codedom::CppObject* frome3;
        cminor::codedom::CppObject* fromNewExpression;
        cminor::codedom::CppObject* fromDeleteExpression;
    };
};

class ExpressionGrammar::UnaryOperatorRule : public cminor::parsing::Rule
{
public:
    UnaryOperatorRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("Operator"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Operator>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryOperatorRule>(this, &UnaryOperatorRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryOperatorRule>(this, &UnaryOperatorRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryOperatorRule>(this, &UnaryOperatorRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryOperatorRule>(this, &UnaryOperatorRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction(ToUtf32("A4"));
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryOperatorRule>(this, &UnaryOperatorRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction(ToUtf32("A5"));
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnaryOperatorRule>(this, &UnaryOperatorRule::A5Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Operator::deref;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Operator::addrOf;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Operator::plus;
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Operator::minus;
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Operator::not_;
    }
    void A5Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Operator::neg;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        Operator value;
    };
};

class ExpressionGrammar::NewExpressionRule : public cminor::parsing::Rule
{
public:
    NewExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("bool"), ToUtf32("global")));
        AddLocalVariable(AttrOrVariable(ToUtf32("TypeId*"), ToUtf32("typeId")));
        AddLocalVariable(AttrOrVariable(ToUtf32("bool"), ToUtf32("parens")));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewExpressionRule>(this, &NewExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewExpressionRule>(this, &NewExpressionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewExpressionRule>(this, &NewExpressionRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewExpressionRule>(this, &NewExpressionRule::A3Action));
        cminor::parsing::NonterminalParser* newPlacementNonterminalParser = GetNonterminal(ToUtf32("NewPlacement"));
        newPlacementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NewExpressionRule>(this, &NewExpressionRule::PostNewPlacement));
        cminor::parsing::NonterminalParser* newTypeIdNonterminalParser = GetNonterminal(ToUtf32("NewTypeId"));
        newTypeIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NewExpressionRule>(this, &NewExpressionRule::PostNewTypeId));
        cminor::parsing::NonterminalParser* typeIdNonterminalParser = GetNonterminal(ToUtf32("TypeId"));
        typeIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NewExpressionRule>(this, &NewExpressionRule::PostTypeId));
        cminor::parsing::NonterminalParser* newInitializerNonterminalParser = GetNonterminal(ToUtf32("NewInitializer"));
        newInitializerNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NewExpressionRule>(this, &NewExpressionRule::PostNewInitializer));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new NewExpr(context->global, context->fromNewPlacement, context->typeId, context->parens, context->fromNewInitializer);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->global = true;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->typeId = context->fromNewTypeId;
        context->parens = false;
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->typeId = context->fromTypeId;
        context->parens = true;
    }
    void PostNewPlacement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromNewPlacement_value = std::move(stack.top());
            context->fromNewPlacement = *static_cast<cminor::parsing::ValueObject<std::vector<cminor::codedom::CppObject*>>*>(fromNewPlacement_value.get());
            stack.pop();
        }
    }
    void PostNewTypeId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromNewTypeId_value = std::move(stack.top());
            context->fromNewTypeId = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeId*>*>(fromNewTypeId_value.get());
            stack.pop();
        }
    }
    void PostTypeId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeId_value = std::move(stack.top());
            context->fromTypeId = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeId*>*>(fromTypeId_value.get());
            stack.pop();
        }
    }
    void PostNewInitializer(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromNewInitializer_value = std::move(stack.top());
            context->fromNewInitializer = *static_cast<cminor::parsing::ValueObject<std::vector<cminor::codedom::CppObject*>>*>(fromNewInitializer_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), global(), typeId(), parens(), fromNewPlacement(), fromNewTypeId(), fromTypeId(), fromNewInitializer() {}
        cminor::codedom::CppObject* value;
        bool global;
        TypeId* typeId;
        bool parens;
        std::vector<cminor::codedom::CppObject*> fromNewPlacement;
        cminor::codedom::TypeId* fromNewTypeId;
        cminor::codedom::TypeId* fromTypeId;
        std::vector<cminor::codedom::CppObject*> fromNewInitializer;
    };
};

class ExpressionGrammar::NewTypeIdRule : public cminor::parsing::Rule
{
public:
    NewTypeIdRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::TypeId*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<TypeId>"), ToUtf32("ti")));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::TypeId*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewTypeIdRule>(this, &NewTypeIdRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewTypeIdRule>(this, &NewTypeIdRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewTypeIdRule>(this, &NewTypeIdRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewTypeIdRule>(this, &NewTypeIdRule::A3Action));
        cminor::parsing::NonterminalParser* typeSpecifierSeqNonterminalParser = GetNonterminal(ToUtf32("TypeSpecifierSeq"));
        typeSpecifierSeqNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<NewTypeIdRule>(this, &NewTypeIdRule::PreTypeSpecifierSeq));
        cminor::parsing::NonterminalParser* typeNameNonterminalParser = GetNonterminal(ToUtf32("TypeName"));
        typeNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NewTypeIdRule>(this, &NewTypeIdRule::PostTypeName));
        cminor::parsing::NonterminalParser* newDeclaratorNonterminalParser = GetNonterminal(ToUtf32("NewDeclarator"));
        newDeclaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NewTypeIdRule>(this, &NewTypeIdRule::PostNewDeclarator));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ti.reset(new TypeId);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->ti.release();
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ti->Add(context->fromTypeName);
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ti->Declarator() = context->fromNewDeclarator;
    }
    void PreTypeSpecifierSeq(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::TypeId*>(context->ti.get())));
    }
    void PostTypeName(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeName_value = std::move(stack.top());
            context->fromTypeName = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeName*>*>(fromTypeName_value.get());
            stack.pop();
        }
    }
    void PostNewDeclarator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromNewDeclarator_value = std::move(stack.top());
            context->fromNewDeclarator = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromNewDeclarator_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), ti(), fromTypeName(), fromNewDeclarator() {}
        cminor::codedom::TypeId* value;
        std::unique_ptr<TypeId> ti;
        cminor::codedom::TypeName* fromTypeName;
        std::u32string fromNewDeclarator;
    };
};

class ExpressionGrammar::NewDeclaratorRule : public cminor::parsing::Rule
{
public:
    NewDeclaratorRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewDeclaratorRule>(this, &NewDeclaratorRule::A0Action));
        cminor::parsing::NonterminalParser* newDeclaratorNonterminalParser = GetNonterminal(ToUtf32("NewDeclarator"));
        newDeclaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NewDeclaratorRule>(this, &NewDeclaratorRule::PostNewDeclarator));
        cminor::parsing::NonterminalParser* directNewDeclaratorNonterminalParser = GetNonterminal(ToUtf32("DirectNewDeclarator"));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Trim(std::u32string(matchBegin, matchEnd));
    }
    void PostNewDeclarator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromNewDeclarator_value = std::move(stack.top());
            context->fromNewDeclarator = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromNewDeclarator_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromNewDeclarator() {}
        std::u32string value;
        std::u32string fromNewDeclarator;
    };
};

class ExpressionGrammar::DirectNewDeclaratorRule : public cminor::parsing::Rule
{
public:
    DirectNewDeclaratorRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<CppObject>"), ToUtf32("e")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DirectNewDeclaratorRule>(this, &DirectNewDeclaratorRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DirectNewDeclaratorRule>(this, &DirectNewDeclaratorRule::A1Action));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DirectNewDeclaratorRule>(this, &DirectNewDeclaratorRule::PostExpression));
        cminor::parsing::NonterminalParser* constantExpressionNonterminalParser = GetNonterminal(ToUtf32("ConstantExpression"));
        constantExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DirectNewDeclaratorRule>(this, &DirectNewDeclaratorRule::PostConstantExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->e.reset(context->fromExpression);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->e.reset(context->fromConstantExpression);
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PostConstantExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromConstantExpression_value = std::move(stack.top());
            context->fromConstantExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromConstantExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): e(), fromExpression(), fromConstantExpression() {}
        std::unique_ptr<CppObject> e;
        cminor::codedom::CppObject* fromExpression;
        cminor::codedom::CppObject* fromConstantExpression;
    };
};

class ExpressionGrammar::NewPlacementRule : public cminor::parsing::Rule
{
public:
    NewPlacementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("std::vector<cminor::codedom::CppObject*>"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::vector<cminor::codedom::CppObject*>>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewPlacementRule>(this, &NewPlacementRule::A0Action));
        cminor::parsing::NonterminalParser* expressionListNonterminalParser = GetNonterminal(ToUtf32("ExpressionList"));
        expressionListNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NewPlacementRule>(this, &NewPlacementRule::PostExpressionList));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromExpressionList;
    }
    void PostExpressionList(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpressionList_value = std::move(stack.top());
            context->fromExpressionList = *static_cast<cminor::parsing::ValueObject<std::vector<cminor::codedom::CppObject*>>*>(fromExpressionList_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromExpressionList() {}
        std::vector<cminor::codedom::CppObject*> value;
        std::vector<cminor::codedom::CppObject*> fromExpressionList;
    };
};

class ExpressionGrammar::NewInitializerRule : public cminor::parsing::Rule
{
public:
    NewInitializerRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("std::vector<cminor::codedom::CppObject*>"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::vector<cminor::codedom::CppObject*>>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NewInitializerRule>(this, &NewInitializerRule::A0Action));
        cminor::parsing::NonterminalParser* expressionListNonterminalParser = GetNonterminal(ToUtf32("ExpressionList"));
        expressionListNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NewInitializerRule>(this, &NewInitializerRule::PostExpressionList));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromExpressionList;
    }
    void PostExpressionList(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpressionList_value = std::move(stack.top());
            context->fromExpressionList = *static_cast<cminor::parsing::ValueObject<std::vector<cminor::codedom::CppObject*>>*>(fromExpressionList_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromExpressionList() {}
        std::vector<cminor::codedom::CppObject*> value;
        std::vector<cminor::codedom::CppObject*> fromExpressionList;
    };
};

class ExpressionGrammar::DeleteExpressionRule : public cminor::parsing::Rule
{
public:
    DeleteExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("bool"), ToUtf32("global")));
        AddLocalVariable(AttrOrVariable(ToUtf32("bool"), ToUtf32("array")));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeleteExpressionRule>(this, &DeleteExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeleteExpressionRule>(this, &DeleteExpressionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeleteExpressionRule>(this, &DeleteExpressionRule::A2Action));
        cminor::parsing::NonterminalParser* castExpressionNonterminalParser = GetNonterminal(ToUtf32("CastExpression"));
        castExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeleteExpressionRule>(this, &DeleteExpressionRule::PostCastExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new DeleteExpr(context->global, context->array, context->fromCastExpression);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->global = true;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->array = true;
    }
    void PostCastExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCastExpression_value = std::move(stack.top());
            context->fromCastExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromCastExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), global(), array(), fromCastExpression() {}
        cminor::codedom::CppObject* value;
        bool global;
        bool array;
        cminor::codedom::CppObject* fromCastExpression;
    };
};

class ExpressionGrammar::PostfixExpressionRule : public cminor::parsing::Rule
{
public:
    PostfixExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<CppObject>"), ToUtf32("pe")));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixExpressionRule>(this, &PostfixExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixExpressionRule>(this, &PostfixExpressionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixExpressionRule>(this, &PostfixExpressionRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixExpressionRule>(this, &PostfixExpressionRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction(ToUtf32("A4"));
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixExpressionRule>(this, &PostfixExpressionRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction(ToUtf32("A5"));
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixExpressionRule>(this, &PostfixExpressionRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction(ToUtf32("A6"));
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixExpressionRule>(this, &PostfixExpressionRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction(ToUtf32("A7"));
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixExpressionRule>(this, &PostfixExpressionRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction(ToUtf32("A8"));
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixExpressionRule>(this, &PostfixExpressionRule::A8Action));
        cminor::parsing::NonterminalParser* primaryExpressionNonterminalParser = GetNonterminal(ToUtf32("PrimaryExpression"));
        primaryExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixExpressionRule>(this, &PostfixExpressionRule::PostPrimaryExpression));
        cminor::parsing::NonterminalParser* indexNonterminalParser = GetNonterminal(ToUtf32("index"));
        indexNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixExpressionRule>(this, &PostfixExpressionRule::Postindex));
        cminor::parsing::NonterminalParser* invokeNonterminalParser = GetNonterminal(ToUtf32("invoke"));
        invokeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixExpressionRule>(this, &PostfixExpressionRule::Postinvoke));
        cminor::parsing::NonterminalParser* memberNonterminalParser = GetNonterminal(ToUtf32("member"));
        memberNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixExpressionRule>(this, &PostfixExpressionRule::Postmember));
        cminor::parsing::NonterminalParser* ptrmemberNonterminalParser = GetNonterminal(ToUtf32("ptrmember"));
        ptrmemberNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixExpressionRule>(this, &PostfixExpressionRule::Postptrmember));
        cminor::parsing::NonterminalParser* typeSpecifierOrTypeNameNonterminalParser = GetNonterminal(ToUtf32("TypeSpecifierOrTypeName"));
        typeSpecifierOrTypeNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixExpressionRule>(this, &PostfixExpressionRule::PostTypeSpecifierOrTypeName));
        cminor::parsing::NonterminalParser* typeConstructionNonterminalParser = GetNonterminal(ToUtf32("typeConstruction"));
        typeConstructionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixExpressionRule>(this, &PostfixExpressionRule::PosttypeConstruction));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->pe.release();
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->pe.reset(context->fromPrimaryExpression);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->pe.reset(new IndexExpr(context->pe.release(), context->fromindex));
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->pe.reset(new InvokeExpr(context->pe.release(), context->frominvoke));
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->pe.reset(new MemberAccessExpr(context->pe.release(), context->frommember));
    }
    void A5Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->pe.reset(new PtrMemberAccessExpr(context->pe.release(), context->fromptrmember));
    }
    void A6Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->pe.reset(new PostIncrementExpr(context->pe.release()));
    }
    void A7Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->pe.reset(new PostDecrementExpr(context->pe.release()));
    }
    void A8Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->pe.reset(new InvokeExpr(context->fromTypeSpecifierOrTypeName, context->fromtypeConstruction));
    }
    void PostPrimaryExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromPrimaryExpression_value = std::move(stack.top());
            context->fromPrimaryExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromPrimaryExpression_value.get());
            stack.pop();
        }
    }
    void Postindex(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromindex_value = std::move(stack.top());
            context->fromindex = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromindex_value.get());
            stack.pop();
        }
    }
    void Postinvoke(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frominvoke_value = std::move(stack.top());
            context->frominvoke = *static_cast<cminor::parsing::ValueObject<std::vector<cminor::codedom::CppObject*>>*>(frominvoke_value.get());
            stack.pop();
        }
    }
    void Postmember(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frommember_value = std::move(stack.top());
            context->frommember = *static_cast<cminor::parsing::ValueObject<cminor::codedom::IdExpr*>*>(frommember_value.get());
            stack.pop();
        }
    }
    void Postptrmember(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromptrmember_value = std::move(stack.top());
            context->fromptrmember = *static_cast<cminor::parsing::ValueObject<cminor::codedom::IdExpr*>*>(fromptrmember_value.get());
            stack.pop();
        }
    }
    void PostTypeSpecifierOrTypeName(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeSpecifierOrTypeName_value = std::move(stack.top());
            context->fromTypeSpecifierOrTypeName = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromTypeSpecifierOrTypeName_value.get());
            stack.pop();
        }
    }
    void PosttypeConstruction(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromtypeConstruction_value = std::move(stack.top());
            context->fromtypeConstruction = *static_cast<cminor::parsing::ValueObject<std::vector<cminor::codedom::CppObject*>>*>(fromtypeConstruction_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), pe(), fromPrimaryExpression(), fromindex(), frominvoke(), frommember(), fromptrmember(), fromTypeSpecifierOrTypeName(), fromtypeConstruction() {}
        cminor::codedom::CppObject* value;
        std::unique_ptr<CppObject> pe;
        cminor::codedom::CppObject* fromPrimaryExpression;
        cminor::codedom::CppObject* fromindex;
        std::vector<cminor::codedom::CppObject*> frominvoke;
        cminor::codedom::IdExpr* frommember;
        cminor::codedom::IdExpr* fromptrmember;
        cminor::codedom::CppObject* fromTypeSpecifierOrTypeName;
        std::vector<cminor::codedom::CppObject*> fromtypeConstruction;
    };
};

class ExpressionGrammar::TypeSpecifierOrTypeNameRule : public cminor::parsing::Rule
{
public:
    TypeSpecifierOrTypeNameRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeSpecifierOrTypeNameRule>(this, &TypeSpecifierOrTypeNameRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeSpecifierOrTypeNameRule>(this, &TypeSpecifierOrTypeNameRule::A1Action));
        cminor::parsing::NonterminalParser* simpleTypeSpecifierNonterminalParser = GetNonterminal(ToUtf32("SimpleTypeSpecifier"));
        simpleTypeSpecifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TypeSpecifierOrTypeNameRule>(this, &TypeSpecifierOrTypeNameRule::PostSimpleTypeSpecifier));
        cminor::parsing::NonterminalParser* typeNameNonterminalParser = GetNonterminal(ToUtf32("TypeName"));
        typeNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TypeSpecifierOrTypeNameRule>(this, &TypeSpecifierOrTypeNameRule::PostTypeName));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromSimpleTypeSpecifier;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromTypeName;
    }
    void PostSimpleTypeSpecifier(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSimpleTypeSpecifier_value = std::move(stack.top());
            context->fromSimpleTypeSpecifier = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeSpecifier*>*>(fromSimpleTypeSpecifier_value.get());
            stack.pop();
        }
    }
    void PostTypeName(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeName_value = std::move(stack.top());
            context->fromTypeName = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeName*>*>(fromTypeName_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromSimpleTypeSpecifier(), fromTypeName() {}
        cminor::codedom::CppObject* value;
        cminor::codedom::TypeSpecifier* fromSimpleTypeSpecifier;
        cminor::codedom::TypeName* fromTypeName;
    };
};

class ExpressionGrammar::PostCastExpressionRule : public cminor::parsing::Rule
{
public:
    PostCastExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostCastExpressionRule>(this, &PostCastExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostCastExpressionRule>(this, &PostCastExpressionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostCastExpressionRule>(this, &PostCastExpressionRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostCastExpressionRule>(this, &PostCastExpressionRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction(ToUtf32("A4"));
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostCastExpressionRule>(this, &PostCastExpressionRule::A4Action));
        cminor::parsing::NonterminalParser* dctNonterminalParser = GetNonterminal(ToUtf32("dct"));
        dctNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostCastExpressionRule>(this, &PostCastExpressionRule::Postdct));
        cminor::parsing::NonterminalParser* dceNonterminalParser = GetNonterminal(ToUtf32("dce"));
        dceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostCastExpressionRule>(this, &PostCastExpressionRule::Postdce));
        cminor::parsing::NonterminalParser* sctNonterminalParser = GetNonterminal(ToUtf32("sct"));
        sctNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostCastExpressionRule>(this, &PostCastExpressionRule::Postsct));
        cminor::parsing::NonterminalParser* sceNonterminalParser = GetNonterminal(ToUtf32("sce"));
        sceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostCastExpressionRule>(this, &PostCastExpressionRule::Postsce));
        cminor::parsing::NonterminalParser* rctNonterminalParser = GetNonterminal(ToUtf32("rct"));
        rctNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostCastExpressionRule>(this, &PostCastExpressionRule::Postrct));
        cminor::parsing::NonterminalParser* rceNonterminalParser = GetNonterminal(ToUtf32("rce"));
        rceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostCastExpressionRule>(this, &PostCastExpressionRule::Postrce));
        cminor::parsing::NonterminalParser* cctNonterminalParser = GetNonterminal(ToUtf32("cct"));
        cctNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostCastExpressionRule>(this, &PostCastExpressionRule::Postcct));
        cminor::parsing::NonterminalParser* cceNonterminalParser = GetNonterminal(ToUtf32("cce"));
        cceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostCastExpressionRule>(this, &PostCastExpressionRule::Postcce));
        cminor::parsing::NonterminalParser* typeidExprNonterminalParser = GetNonterminal(ToUtf32("typeidExpr"));
        typeidExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostCastExpressionRule>(this, &PostCastExpressionRule::PosttypeidExpr));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new PostCastExpr(U"dynamic_cast", context->fromdct, context->fromdce);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new PostCastExpr(U"static_cast", context->fromsct, context->fromsce);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new PostCastExpr(U"reinterpret_cast", context->fromrct, context->fromrce);
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new PostCastExpr(U"const_cast", context->fromcct, context->fromcce);
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new TypeIdExpr(context->fromtypeidExpr);
    }
    void Postdct(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromdct_value = std::move(stack.top());
            context->fromdct = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeId*>*>(fromdct_value.get());
            stack.pop();
        }
    }
    void Postdce(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromdce_value = std::move(stack.top());
            context->fromdce = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromdce_value.get());
            stack.pop();
        }
    }
    void Postsct(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromsct_value = std::move(stack.top());
            context->fromsct = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeId*>*>(fromsct_value.get());
            stack.pop();
        }
    }
    void Postsce(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromsce_value = std::move(stack.top());
            context->fromsce = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromsce_value.get());
            stack.pop();
        }
    }
    void Postrct(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromrct_value = std::move(stack.top());
            context->fromrct = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeId*>*>(fromrct_value.get());
            stack.pop();
        }
    }
    void Postrce(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromrce_value = std::move(stack.top());
            context->fromrce = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromrce_value.get());
            stack.pop();
        }
    }
    void Postcct(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromcct_value = std::move(stack.top());
            context->fromcct = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeId*>*>(fromcct_value.get());
            stack.pop();
        }
    }
    void Postcce(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromcce_value = std::move(stack.top());
            context->fromcce = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromcce_value.get());
            stack.pop();
        }
    }
    void PosttypeidExpr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromtypeidExpr_value = std::move(stack.top());
            context->fromtypeidExpr = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromtypeidExpr_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromdct(), fromdce(), fromsct(), fromsce(), fromrct(), fromrce(), fromcct(), fromcce(), fromtypeidExpr() {}
        cminor::codedom::CppObject* value;
        cminor::codedom::TypeId* fromdct;
        cminor::codedom::CppObject* fromdce;
        cminor::codedom::TypeId* fromsct;
        cminor::codedom::CppObject* fromsce;
        cminor::codedom::TypeId* fromrct;
        cminor::codedom::CppObject* fromrce;
        cminor::codedom::TypeId* fromcct;
        cminor::codedom::CppObject* fromcce;
        cminor::codedom::CppObject* fromtypeidExpr;
    };
};

class ExpressionGrammar::ExpressionListRule : public cminor::parsing::Rule
{
public:
    ExpressionListRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("std::vector<cminor::codedom::CppObject*>"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::vector<cminor::codedom::CppObject*>>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExpressionListRule>(this, &ExpressionListRule::A0Action));
        cminor::parsing::NonterminalParser* assignmentExpressionNonterminalParser = GetNonterminal(ToUtf32("AssignmentExpression"));
        assignmentExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ExpressionListRule>(this, &ExpressionListRule::PostAssignmentExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value.push_back(context->fromAssignmentExpression);
    }
    void PostAssignmentExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAssignmentExpression_value = std::move(stack.top());
            context->fromAssignmentExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromAssignmentExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromAssignmentExpression() {}
        std::vector<cminor::codedom::CppObject*> value;
        cminor::codedom::CppObject* fromAssignmentExpression;
    };
};

class ExpressionGrammar::PrimaryExpressionRule : public cminor::parsing::Rule
{
public:
    PrimaryExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<CppObject>"), ToUtf32("pe")));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryExpressionRule>(this, &PrimaryExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryExpressionRule>(this, &PrimaryExpressionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryExpressionRule>(this, &PrimaryExpressionRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryExpressionRule>(this, &PrimaryExpressionRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction(ToUtf32("A4"));
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryExpressionRule>(this, &PrimaryExpressionRule::A4Action));
        cminor::parsing::NonterminalParser* literalNonterminalParser = GetNonterminal(ToUtf32("Literal"));
        literalNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryExpressionRule>(this, &PrimaryExpressionRule::PostLiteral));
        cminor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryExpressionRule>(this, &PrimaryExpressionRule::PostExpression));
        cminor::parsing::NonterminalParser* idExpressionNonterminalParser = GetNonterminal(ToUtf32("IdExpression"));
        idExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryExpressionRule>(this, &PrimaryExpressionRule::PostIdExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->pe.release();
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->pe.reset(context->fromLiteral);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->pe.reset(new ThisAccess);
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->pe.reset(context->fromExpression);
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->pe.reset(context->fromIdExpression);
    }
    void PostLiteral(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromLiteral_value = std::move(stack.top());
            context->fromLiteral = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Literal*>*>(fromLiteral_value.get());
            stack.pop();
        }
    }
    void PostExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PostIdExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIdExpression_value = std::move(stack.top());
            context->fromIdExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::IdExpr*>*>(fromIdExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), pe(), fromLiteral(), fromExpression(), fromIdExpression() {}
        cminor::codedom::CppObject* value;
        std::unique_ptr<CppObject> pe;
        cminor::codedom::Literal* fromLiteral;
        cminor::codedom::CppObject* fromExpression;
        cminor::codedom::IdExpr* fromIdExpression;
    };
};

class ExpressionGrammar::IdExpressionRule : public cminor::parsing::Rule
{
public:
    IdExpressionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::IdExpr*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::IdExpr*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IdExpressionRule>(this, &IdExpressionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IdExpressionRule>(this, &IdExpressionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IdExpressionRule>(this, &IdExpressionRule::A2Action));
        cminor::parsing::NonterminalParser* id1NonterminalParser = GetNonterminal(ToUtf32("id1"));
        id1NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IdExpressionRule>(this, &IdExpressionRule::Postid1));
        cminor::parsing::NonterminalParser* id2NonterminalParser = GetNonterminal(ToUtf32("id2"));
        id2NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IdExpressionRule>(this, &IdExpressionRule::Postid2));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new IdExpr(std::u32string(matchBegin, matchEnd));
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new IdExpr(std::u32string(matchBegin, matchEnd));
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new IdExpr(std::u32string(matchBegin, matchEnd));
    }
    void Postid1(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromid1_value = std::move(stack.top());
            context->fromid1 = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromid1_value.get());
            stack.pop();
        }
    }
    void Postid2(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromid2_value = std::move(stack.top());
            context->fromid2 = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromid2_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromid1(), fromid2() {}
        cminor::codedom::IdExpr* value;
        std::u32string fromid1;
        std::u32string fromid2;
    };
};

void ExpressionGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cminor.code.IdentifierGrammar"));
    if (!grammar0)
    {
        grammar0 = cminor::code::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar(ToUtf32("cminor.code.LiteralGrammar"));
    if (!grammar1)
    {
        grammar1 = cminor::code::LiteralGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar(ToUtf32("cminor.code.DeclarationGrammar"));
    if (!grammar2)
    {
        grammar2 = cminor::code::DeclarationGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar(ToUtf32("cminor.code.DeclaratorGrammar"));
    if (!grammar3)
    {
        grammar3 = cminor::code::DeclaratorGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
}

void ExpressionGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("QualifiedId"), this, ToUtf32("IdentifierGrammar.QualifiedId")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Literal"), this, ToUtf32("LiteralGrammar.Literal")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("SimpleTypeSpecifier"), this, ToUtf32("DeclarationGrammar.SimpleTypeSpecifier")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("TypeId"), this, ToUtf32("DeclaratorGrammar.TypeId")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("TypeName"), this, ToUtf32("DeclarationGrammar.TypeName")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("TypeSpecifierSeq"), this, ToUtf32("DeclaratorGrammar.TypeSpecifierSeq")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("PtrOperator"), this, ToUtf32("DeclaratorGrammar.PtrOperator")));
    AddRule(new ExpressionRule(ToUtf32("Expression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("left"), ToUtf32("AssignmentExpression"), 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser(','),
                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                        new cminor::parsing::NonterminalParser(ToUtf32("right"), ToUtf32("AssignmentExpression"), 0)))))));
    AddRule(new ConstantExpressionRule(ToUtf32("ConstantExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 0))));
    AddRule(new AssignmentExpressionRule(ToUtf32("AssignmentExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::ActionParser(ToUtf32("A1"),
                                new cminor::parsing::NonterminalParser(ToUtf32("LogicalOrExpression"), ToUtf32("LogicalOrExpression"), 0)),
                            new cminor::parsing::NonterminalParser(ToUtf32("AssingmentOp"), ToUtf32("AssingmentOp"), 0)),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("AssignmentExpression"), ToUtf32("AssignmentExpression"), 0)))),
                new cminor::parsing::ActionParser(ToUtf32("A2"),
                    new cminor::parsing::NonterminalParser(ToUtf32("ConditionalExpression"), ToUtf32("ConditionalExpression"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A3"),
                new cminor::parsing::NonterminalParser(ToUtf32("ThrowExpression"), ToUtf32("ThrowExpression"), 0)))));
    AddRule(new AssingmentOpRule(ToUtf32("AssingmentOp"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
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
                                                    new cminor::parsing::StringParser(ToUtf32("*="))),
                                                new cminor::parsing::StringParser(ToUtf32("/="))),
                                            new cminor::parsing::StringParser(ToUtf32("%="))),
                                        new cminor::parsing::StringParser(ToUtf32("+="))),
                                    new cminor::parsing::StringParser(ToUtf32("-="))),
                                new cminor::parsing::StringParser(ToUtf32(">>="))),
                            new cminor::parsing::StringParser(ToUtf32("<<="))),
                        new cminor::parsing::StringParser(ToUtf32("&="))),
                    new cminor::parsing::StringParser(ToUtf32("^="))),
                new cminor::parsing::StringParser(ToUtf32("|="))))));
    AddRule(new ThrowExpressionRule(ToUtf32("ThrowExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser(ToUtf32("throw")),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("AssignmentExpression"), ToUtf32("AssignmentExpression"), 0))))));
    AddRule(new ConditionalExpressionRule(ToUtf32("ConditionalExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("left"), ToUtf32("LogicalOrExpression"), 0)),
            new cminor::parsing::OptionalParser(
                new cminor::parsing::ActionParser(ToUtf32("A1"),
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::CharParser('?'),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 0))),
                            new cminor::parsing::CharParser(':')),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("AssignmentExpression"), ToUtf32("AssignmentExpression"), 0))))))));
    AddRule(new LogicalOrExpressionRule(ToUtf32("LogicalOrExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("left"), ToUtf32("LogicalAndExpression"), 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::StringParser(ToUtf32("||")),
                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("right"), ToUtf32("LogicalAndExpression"), 0))))))));
    AddRule(new LogicalAndExpressionRule(ToUtf32("LogicalAndExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("left"), ToUtf32("InclusiveOrExpression"), 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::StringParser(ToUtf32("&&")),
                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("right"), ToUtf32("InclusiveOrExpression"), 0))))))));
    AddRule(new InclusiveOrExpressionRule(ToUtf32("InclusiveOrExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("left"), ToUtf32("ExclusiveOrExpression"), 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::DifferenceParser(
                        new cminor::parsing::CharParser('|'),
                        new cminor::parsing::StringParser(ToUtf32("||"))),
                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("right"), ToUtf32("ExclusiveOrExpression"), 0))))))));
    AddRule(new ExclusiveOrExpressionRule(ToUtf32("ExclusiveOrExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("left"), ToUtf32("AndExpression"), 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('^'),
                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("right"), ToUtf32("AndExpression"), 0))))))));
    AddRule(new AndExpressionRule(ToUtf32("AndExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("left"), ToUtf32("EqualityExpression"), 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::DifferenceParser(
                        new cminor::parsing::CharParser('&'),
                        new cminor::parsing::StringParser(ToUtf32("&&"))),
                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("right"), ToUtf32("EqualityExpression"), 0))))))));
    AddRule(new EqualityExpressionRule(ToUtf32("EqualityExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("left"), ToUtf32("RelationalExpression"), 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("EqOp"), ToUtf32("EqOp"), 0),
                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("right"), ToUtf32("RelationalExpression"), 0))))))));
    AddRule(new EqOpRule(ToUtf32("EqOp"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::StringParser(ToUtf32("=="))),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::StringParser(ToUtf32("!="))))));
    AddRule(new RelationalExpressionRule(ToUtf32("RelationalExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("left"), ToUtf32("ShiftExpression"), 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("RelOp"), ToUtf32("RelOp"), 0),
                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("right"), ToUtf32("ShiftExpression"), 0))))))));
    AddRule(new RelOpRule(ToUtf32("RelOp"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::ActionParser(ToUtf32("A0"),
                        new cminor::parsing::StringParser(ToUtf32("<="))),
                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                        new cminor::parsing::StringParser(ToUtf32(">=")))),
                new cminor::parsing::ActionParser(ToUtf32("A2"),
                    new cminor::parsing::DifferenceParser(
                        new cminor::parsing::CharParser('<'),
                        new cminor::parsing::StringParser(ToUtf32("<<"))))),
            new cminor::parsing::ActionParser(ToUtf32("A3"),
                new cminor::parsing::DifferenceParser(
                    new cminor::parsing::CharParser('>'),
                    new cminor::parsing::StringParser(ToUtf32(">>")))))));
    AddRule(new ShiftExpressionRule(ToUtf32("ShiftExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("left"), ToUtf32("AdditiveExpression"), 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("ShiftOp"), ToUtf32("ShiftOp"), 0),
                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("right"), ToUtf32("AdditiveExpression"), 0))))))));
    AddRule(new ShiftOpRule(ToUtf32("ShiftOp"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::DifferenceParser(
                    new cminor::parsing::StringParser(ToUtf32("<<")),
                    new cminor::parsing::StringParser(ToUtf32("<<=")))),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::DifferenceParser(
                    new cminor::parsing::StringParser(ToUtf32(">>")),
                    new cminor::parsing::StringParser(ToUtf32(">>=")))))));
    AddRule(new AdditiveExpressionRule(ToUtf32("AdditiveExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("left"), ToUtf32("MultiplicativeExpression"), 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("AddOp"), ToUtf32("AddOp"), 0),
                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("right"), ToUtf32("MultiplicativeExpression"), 0))))))));
    AddRule(new AddOpRule(ToUtf32("AddOp"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::DifferenceParser(
                    new cminor::parsing::CharParser('+'),
                    new cminor::parsing::StringParser(ToUtf32("+=")))),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::DifferenceParser(
                    new cminor::parsing::CharParser('-'),
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::StringParser(ToUtf32("-=")),
                        new cminor::parsing::StringParser(ToUtf32("->"))))))));
    AddRule(new MultiplicativeExpressionRule(ToUtf32("MultiplicativeExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("left"), ToUtf32("PmExpression"), 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("MulOp"), ToUtf32("MulOp"), 0),
                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("right"), ToUtf32("PmExpression"), 0))))))));
    AddRule(new MulOpRule(ToUtf32("MulOp"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::DifferenceParser(
                        new cminor::parsing::CharParser('*'),
                        new cminor::parsing::StringParser(ToUtf32("*=")))),
                new cminor::parsing::ActionParser(ToUtf32("A1"),
                    new cminor::parsing::DifferenceParser(
                        new cminor::parsing::CharParser('/'),
                        new cminor::parsing::StringParser(ToUtf32("/="))))),
            new cminor::parsing::ActionParser(ToUtf32("A2"),
                new cminor::parsing::DifferenceParser(
                    new cminor::parsing::CharParser('%'),
                    new cminor::parsing::StringParser(ToUtf32("%=")))))));
    AddRule(new PmExpressionRule(ToUtf32("PmExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("left"), ToUtf32("CastExpression"), 0)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("PmOp"), ToUtf32("PmOp"), 0),
                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("right"), ToUtf32("CastExpression"), 0))))))));
    AddRule(new PmOpRule(ToUtf32("PmOp"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::StringParser(ToUtf32(".*"))),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::StringParser(ToUtf32("->*"))))));
    AddRule(new CastExpressionRule(ToUtf32("CastExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser(ToUtf32("A1"),
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::CharParser('('),
                                new cminor::parsing::ActionParser(ToUtf32("A2"),
                                    new cminor::parsing::NonterminalParser(ToUtf32("TypeId"), ToUtf32("TypeId"), 0))),
                            new cminor::parsing::CharParser(')')),
                        new cminor::parsing::NonterminalParser(ToUtf32("CastExpression"), ToUtf32("CastExpression"), 0))),
                new cminor::parsing::ActionParser(ToUtf32("A3"),
                    new cminor::parsing::NonterminalParser(ToUtf32("UnaryExpression"), ToUtf32("UnaryExpression"), 0))))));
    AddRule(new UnaryExpressionRule(ToUtf32("UnaryExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::AlternativeParser(
                                    new cminor::parsing::AlternativeParser(
                                        new cminor::parsing::AlternativeParser(
                                            new cminor::parsing::ActionParser(ToUtf32("A1"),
                                                new cminor::parsing::NonterminalParser(ToUtf32("PostfixExpression"), ToUtf32("PostfixExpression"), 0)),
                                            new cminor::parsing::ActionParser(ToUtf32("A2"),
                                                new cminor::parsing::NonterminalParser(ToUtf32("PostCastExpression"), ToUtf32("PostCastExpression"), 0))),
                                        new cminor::parsing::ActionParser(ToUtf32("A3"),
                                            new cminor::parsing::SequenceParser(
                                                new cminor::parsing::StringParser(ToUtf32("++")),
                                                new cminor::parsing::ExpectationParser(
                                                    new cminor::parsing::NonterminalParser(ToUtf32("e1"), ToUtf32("UnaryExpression"), 0))))),
                                    new cminor::parsing::ActionParser(ToUtf32("A4"),
                                        new cminor::parsing::SequenceParser(
                                            new cminor::parsing::StringParser(ToUtf32("--")),
                                            new cminor::parsing::ExpectationParser(
                                                new cminor::parsing::NonterminalParser(ToUtf32("e2"), ToUtf32("UnaryExpression"), 0))))),
                                new cminor::parsing::ActionParser(ToUtf32("A5"),
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::NonterminalParser(ToUtf32("UnaryOperator"), ToUtf32("UnaryOperator"), 0),
                                        new cminor::parsing::ExpectationParser(
                                            new cminor::parsing::NonterminalParser(ToUtf32("CastExpression"), ToUtf32("CastExpression"), 0))))),
                            new cminor::parsing::ActionParser(ToUtf32("A6"),
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::SequenceParser(
                                            new cminor::parsing::KeywordParser(ToUtf32("sizeof")),
                                            new cminor::parsing::CharParser('(')),
                                        new cminor::parsing::ExpectationParser(
                                            new cminor::parsing::NonterminalParser(ToUtf32("TypeId"), ToUtf32("TypeId"), 0))),
                                    new cminor::parsing::ExpectationParser(
                                        new cminor::parsing::CharParser(')'))))),
                        new cminor::parsing::ActionParser(ToUtf32("A7"),
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::KeywordParser(ToUtf32("sizeof")),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::NonterminalParser(ToUtf32("e3"), ToUtf32("UnaryExpression"), 0))))),
                    new cminor::parsing::ActionParser(ToUtf32("A8"),
                        new cminor::parsing::NonterminalParser(ToUtf32("NewExpression"), ToUtf32("NewExpression"), 0))),
                new cminor::parsing::ActionParser(ToUtf32("A9"),
                    new cminor::parsing::NonterminalParser(ToUtf32("DeleteExpression"), ToUtf32("DeleteExpression"), 0))))));
    AddRule(new UnaryOperatorRule(ToUtf32("UnaryOperator"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::ActionParser(ToUtf32("A0"),
                                new cminor::parsing::DifferenceParser(
                                    new cminor::parsing::CharParser('*'),
                                    new cminor::parsing::StringParser(ToUtf32("*=")))),
                            new cminor::parsing::ActionParser(ToUtf32("A1"),
                                new cminor::parsing::DifferenceParser(
                                    new cminor::parsing::CharParser('&'),
                                    new cminor::parsing::StringParser(ToUtf32("&="))))),
                        new cminor::parsing::ActionParser(ToUtf32("A2"),
                            new cminor::parsing::DifferenceParser(
                                new cminor::parsing::CharParser('+'),
                                new cminor::parsing::StringParser(ToUtf32("+="))))),
                    new cminor::parsing::ActionParser(ToUtf32("A3"),
                        new cminor::parsing::DifferenceParser(
                            new cminor::parsing::CharParser('-'),
                            new cminor::parsing::StringParser(ToUtf32("-="))))),
                new cminor::parsing::ActionParser(ToUtf32("A4"),
                    new cminor::parsing::DifferenceParser(
                        new cminor::parsing::CharParser('!'),
                        new cminor::parsing::StringParser(ToUtf32("!="))))),
            new cminor::parsing::ActionParser(ToUtf32("A5"),
                new cminor::parsing::CharParser('~')))));
    AddRule(new NewExpressionRule(ToUtf32("NewExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::ActionParser(ToUtf32("A1"),
                                    new cminor::parsing::StringParser(ToUtf32("::")))),
                            new cminor::parsing::KeywordParser(ToUtf32("new"))),
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("NewPlacement"), ToUtf32("NewPlacement"), 0))),
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::ActionParser(ToUtf32("A2"),
                            new cminor::parsing::NonterminalParser(ToUtf32("NewTypeId"), ToUtf32("NewTypeId"), 0)),
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::CharParser('('),
                                new cminor::parsing::ActionParser(ToUtf32("A3"),
                                    new cminor::parsing::NonterminalParser(ToUtf32("TypeId"), ToUtf32("TypeId"), 0))),
                            new cminor::parsing::CharParser(')')))),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("NewInitializer"), ToUtf32("NewInitializer"), 0))))));
    AddRule(new NewTypeIdRule(ToUtf32("NewTypeId"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::EmptyParser()),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("TypeSpecifierSeq"), ToUtf32("TypeSpecifierSeq"), 1),
                        new cminor::parsing::ActionParser(ToUtf32("A2"),
                            new cminor::parsing::NonterminalParser(ToUtf32("TypeName"), ToUtf32("TypeName"), 0))),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::ActionParser(ToUtf32("A3"),
                            new cminor::parsing::NonterminalParser(ToUtf32("NewDeclarator"), ToUtf32("NewDeclarator"), 0))))))));
    AddRule(new NewDeclaratorRule(ToUtf32("NewDeclarator"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("PtrOperator"), ToUtf32("PtrOperator"), 0),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("NewDeclarator"), ToUtf32("NewDeclarator"), 0))),
                new cminor::parsing::NonterminalParser(ToUtf32("DirectNewDeclarator"), ToUtf32("DirectNewDeclarator"), 0)))));
    AddRule(new DirectNewDeclaratorRule(ToUtf32("DirectNewDeclarator"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('['),
                    new cminor::parsing::ActionParser(ToUtf32("A0"),
                        new cminor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 0))),
                new cminor::parsing::CharParser(']')),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::CharParser('['),
                        new cminor::parsing::ActionParser(ToUtf32("A1"),
                            new cminor::parsing::NonterminalParser(ToUtf32("ConstantExpression"), ToUtf32("ConstantExpression"), 0))),
                    new cminor::parsing::CharParser(']'))))));
    AddRule(new NewPlacementRule(ToUtf32("NewPlacement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('('),
                    new cminor::parsing::NonterminalParser(ToUtf32("ExpressionList"), ToUtf32("ExpressionList"), 0)),
                new cminor::parsing::CharParser(')')))));
    AddRule(new NewInitializerRule(ToUtf32("NewInitializer"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('('),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("ExpressionList"), ToUtf32("ExpressionList"), 0))),
                new cminor::parsing::CharParser(')')))));
    AddRule(new DeleteExpressionRule(ToUtf32("DeleteExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::ActionParser(ToUtf32("A1"),
                                new cminor::parsing::StringParser(ToUtf32("::")))),
                        new cminor::parsing::KeywordParser(ToUtf32("delete"))),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::CharParser('['),
                            new cminor::parsing::ActionParser(ToUtf32("A2"),
                                new cminor::parsing::CharParser(']'))))),
                new cminor::parsing::NonterminalParser(ToUtf32("CastExpression"), ToUtf32("CastExpression"), 0)))));
    AddRule(new PostfixExpressionRule(ToUtf32("PostfixExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                        new cminor::parsing::NonterminalParser(ToUtf32("PrimaryExpression"), ToUtf32("PrimaryExpression"), 0)),
                    new cminor::parsing::KleeneStarParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::AlternativeParser(
                                    new cminor::parsing::AlternativeParser(
                                        new cminor::parsing::AlternativeParser(
                                            new cminor::parsing::ActionParser(ToUtf32("A2"),
                                                new cminor::parsing::SequenceParser(
                                                    new cminor::parsing::SequenceParser(
                                                        new cminor::parsing::CharParser('['),
                                                        new cminor::parsing::ExpectationParser(
                                                            new cminor::parsing::NonterminalParser(ToUtf32("index"), ToUtf32("Expression"), 0))),
                                                    new cminor::parsing::ExpectationParser(
                                                        new cminor::parsing::CharParser(']')))),
                                            new cminor::parsing::ActionParser(ToUtf32("A3"),
                                                new cminor::parsing::SequenceParser(
                                                    new cminor::parsing::SequenceParser(
                                                        new cminor::parsing::CharParser('('),
                                                        new cminor::parsing::OptionalParser(
                                                            new cminor::parsing::NonterminalParser(ToUtf32("invoke"), ToUtf32("ExpressionList"), 0))),
                                                    new cminor::parsing::ExpectationParser(
                                                        new cminor::parsing::CharParser(')'))))),
                                        new cminor::parsing::ActionParser(ToUtf32("A4"),
                                            new cminor::parsing::SequenceParser(
                                                new cminor::parsing::DifferenceParser(
                                                    new cminor::parsing::CharParser('.'),
                                                    new cminor::parsing::StringParser(ToUtf32(".*"))),
                                                new cminor::parsing::ExpectationParser(
                                                    new cminor::parsing::NonterminalParser(ToUtf32("member"), ToUtf32("IdExpression"), 0))))),
                                    new cminor::parsing::ActionParser(ToUtf32("A5"),
                                        new cminor::parsing::SequenceParser(
                                            new cminor::parsing::DifferenceParser(
                                                new cminor::parsing::StringParser(ToUtf32("->")),
                                                new cminor::parsing::StringParser(ToUtf32("->*"))),
                                            new cminor::parsing::ExpectationParser(
                                                new cminor::parsing::NonterminalParser(ToUtf32("ptrmember"), ToUtf32("IdExpression"), 0))))),
                                new cminor::parsing::ActionParser(ToUtf32("A6"),
                                    new cminor::parsing::StringParser(ToUtf32("++")))),
                            new cminor::parsing::ActionParser(ToUtf32("A7"),
                                new cminor::parsing::StringParser(ToUtf32("--")))))),
                new cminor::parsing::ActionParser(ToUtf32("A8"),
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("TypeSpecifierOrTypeName"), ToUtf32("TypeSpecifierOrTypeName"), 0),
                                new cminor::parsing::CharParser('(')),
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("typeConstruction"), ToUtf32("ExpressionList"), 0))),
                        new cminor::parsing::CharParser(')')))))));
    AddRule(new TypeSpecifierOrTypeNameRule(ToUtf32("TypeSpecifierOrTypeName"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("SimpleTypeSpecifier"), ToUtf32("SimpleTypeSpecifier"), 0)),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::NonterminalParser(ToUtf32("TypeName"), ToUtf32("TypeName"), 0)))));
    AddRule(new PostCastExpressionRule(ToUtf32("PostCastExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::ActionParser(ToUtf32("A0"),
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::SequenceParser(
                                            new cminor::parsing::SequenceParser(
                                                new cminor::parsing::SequenceParser(
                                                    new cminor::parsing::KeywordParser(ToUtf32("dynamic_cast")),
                                                    new cminor::parsing::ExpectationParser(
                                                        new cminor::parsing::CharParser('<'))),
                                                new cminor::parsing::ExpectationParser(
                                                    new cminor::parsing::NonterminalParser(ToUtf32("dct"), ToUtf32("TypeId"), 0))),
                                            new cminor::parsing::ExpectationParser(
                                                new cminor::parsing::CharParser('>'))),
                                        new cminor::parsing::ExpectationParser(
                                            new cminor::parsing::CharParser('('))),
                                    new cminor::parsing::ExpectationParser(
                                        new cminor::parsing::NonterminalParser(ToUtf32("dce"), ToUtf32("Expression"), 0))),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::CharParser(')')))),
                        new cminor::parsing::ActionParser(ToUtf32("A1"),
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::SequenceParser(
                                            new cminor::parsing::SequenceParser(
                                                new cminor::parsing::SequenceParser(
                                                    new cminor::parsing::KeywordParser(ToUtf32("static_cast")),
                                                    new cminor::parsing::ExpectationParser(
                                                        new cminor::parsing::CharParser('<'))),
                                                new cminor::parsing::ExpectationParser(
                                                    new cminor::parsing::NonterminalParser(ToUtf32("sct"), ToUtf32("TypeId"), 0))),
                                            new cminor::parsing::ExpectationParser(
                                                new cminor::parsing::CharParser('>'))),
                                        new cminor::parsing::ExpectationParser(
                                            new cminor::parsing::CharParser('('))),
                                    new cminor::parsing::ExpectationParser(
                                        new cminor::parsing::NonterminalParser(ToUtf32("sce"), ToUtf32("Expression"), 0))),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::CharParser(')'))))),
                    new cminor::parsing::ActionParser(ToUtf32("A2"),
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::SequenceParser(
                                            new cminor::parsing::SequenceParser(
                                                new cminor::parsing::KeywordParser(ToUtf32("reinterpret_cast")),
                                                new cminor::parsing::ExpectationParser(
                                                    new cminor::parsing::CharParser('<'))),
                                            new cminor::parsing::ExpectationParser(
                                                new cminor::parsing::NonterminalParser(ToUtf32("rct"), ToUtf32("TypeId"), 0))),
                                        new cminor::parsing::ExpectationParser(
                                            new cminor::parsing::CharParser('>'))),
                                    new cminor::parsing::ExpectationParser(
                                        new cminor::parsing::CharParser('('))),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::NonterminalParser(ToUtf32("rce"), ToUtf32("Expression"), 0))),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::CharParser(')'))))),
                new cminor::parsing::ActionParser(ToUtf32("A3"),
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::SequenceParser(
                                            new cminor::parsing::KeywordParser(ToUtf32("const_cast")),
                                            new cminor::parsing::ExpectationParser(
                                                new cminor::parsing::CharParser('<'))),
                                        new cminor::parsing::ExpectationParser(
                                            new cminor::parsing::NonterminalParser(ToUtf32("cct"), ToUtf32("TypeId"), 0))),
                                    new cminor::parsing::ExpectationParser(
                                        new cminor::parsing::CharParser('>'))),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::CharParser('('))),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("cce"), ToUtf32("Expression"), 0))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser(')'))))),
            new cminor::parsing::ActionParser(ToUtf32("A4"),
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::KeywordParser(ToUtf32("typeid")),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::CharParser('('))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("typeidExpr"), ToUtf32("Expression"), 0))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser(')')))))));
    AddRule(new ExpressionListRule(ToUtf32("ExpressionList"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ListParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("AssignmentExpression"), ToUtf32("AssignmentExpression"), 0)),
            new cminor::parsing::CharParser(','))));
    AddRule(new PrimaryExpressionRule(ToUtf32("PrimaryExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::ActionParser(ToUtf32("A1"),
                            new cminor::parsing::NonterminalParser(ToUtf32("Literal"), ToUtf32("Literal"), 0)),
                        new cminor::parsing::ActionParser(ToUtf32("A2"),
                            new cminor::parsing::KeywordParser(ToUtf32("this")))),
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::CharParser('('),
                            new cminor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 0)),
                        new cminor::parsing::ActionParser(ToUtf32("A3"),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::CharParser(')'))))),
                new cminor::parsing::ActionParser(ToUtf32("A4"),
                    new cminor::parsing::NonterminalParser(ToUtf32("IdExpression"), ToUtf32("IdExpression"), 0))))));
    AddRule(new IdExpressionRule(ToUtf32("IdExpression"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::TokenParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("id1"), ToUtf32("QualifiedId"), 0),
                                new cminor::parsing::StringParser(ToUtf32("::"))),
                            new cminor::parsing::NonterminalParser(ToUtf32("OperatorFunctionId"), ToUtf32("OperatorFunctionId"), 0)))),
                new cminor::parsing::ActionParser(ToUtf32("A1"),
                    new cminor::parsing::TokenParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::StringParser(ToUtf32("::"))),
                            new cminor::parsing::NonterminalParser(ToUtf32("OperatorFunctionId"), ToUtf32("OperatorFunctionId"), 0))))),
            new cminor::parsing::ActionParser(ToUtf32("A2"),
                new cminor::parsing::NonterminalParser(ToUtf32("id2"), ToUtf32("QualifiedId"), 0)))));
    AddRule(new cminor::parsing::Rule(ToUtf32("OperatorFunctionId"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::KeywordParser(ToUtf32("operator")),
            new cminor::parsing::NonterminalParser(ToUtf32("Operator"), ToUtf32("Operator"), 0))));
    AddRule(new cminor::parsing::Rule(ToUtf32("Operator"), GetScope(), GetParsingDomain()->GetNextRuleId(),
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
                                                                                                                                                                        new cminor::parsing::StringParser(ToUtf32("->*"))),
                                                                                                                                                                    new cminor::parsing::StringParser(ToUtf32("->"))),
                                                                                                                                                                new cminor::parsing::CharParser(',')),
                                                                                                                                                            new cminor::parsing::StringParser(ToUtf32("--"))),
                                                                                                                                                        new cminor::parsing::StringParser(ToUtf32("++"))),
                                                                                                                                                    new cminor::parsing::StringParser(ToUtf32("||"))),
                                                                                                                                                new cminor::parsing::StringParser(ToUtf32("&&"))),
                                                                                                                                            new cminor::parsing::StringParser(ToUtf32("<="))),
                                                                                                                                        new cminor::parsing::StringParser(ToUtf32(">="))),
                                                                                                                                    new cminor::parsing::StringParser(ToUtf32("!="))),
                                                                                                                                new cminor::parsing::StringParser(ToUtf32("=="))),
                                                                                                                            new cminor::parsing::StringParser(ToUtf32("<<="))),
                                                                                                                        new cminor::parsing::StringParser(ToUtf32(">>="))),
                                                                                                                    new cminor::parsing::StringParser(ToUtf32("<<"))),
                                                                                                                new cminor::parsing::StringParser(ToUtf32(">>"))),
                                                                                                            new cminor::parsing::StringParser(ToUtf32("|="))),
                                                                                                        new cminor::parsing::StringParser(ToUtf32("&="))),
                                                                                                    new cminor::parsing::StringParser(ToUtf32("^="))),
                                                                                                new cminor::parsing::StringParser(ToUtf32("%="))),
                                                                                            new cminor::parsing::StringParser(ToUtf32("/="))),
                                                                                        new cminor::parsing::StringParser(ToUtf32("*="))),
                                                                                    new cminor::parsing::StringParser(ToUtf32("-="))),
                                                                                new cminor::parsing::StringParser(ToUtf32("+="))),
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
                                new cminor::parsing::KeywordParser(ToUtf32("new")),
                                new cminor::parsing::CharParser('[')),
                            new cminor::parsing::CharParser(']'))),
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::KeywordParser(ToUtf32("delete")),
                            new cminor::parsing::CharParser('[')),
                        new cminor::parsing::CharParser(']'))),
                new cminor::parsing::KeywordParser(ToUtf32("new"))),
            new cminor::parsing::KeywordParser(ToUtf32("delete")))));
}

} } // namespace cminor.code
