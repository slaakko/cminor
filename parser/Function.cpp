#include "Function.hpp"
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
#include <cminor/parser/Keyword.hpp>
#include <cminor/parser/Specifier.hpp>
#include <cminor/parser/TypeExpr.hpp>
#include <cminor/parser/Identifier.hpp>
#include <cminor/parser/Parameter.hpp>
#include <cminor/parser/Statement.hpp>

namespace cminor { namespace parser {

using namespace cminor::parsing;
using namespace cminor::util;
using namespace cminor::unicode;

FunctionGrammar* FunctionGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

FunctionGrammar* FunctionGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    FunctionGrammar* grammar(new FunctionGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

FunctionGrammar::FunctionGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("FunctionGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.parser")), parsingDomain_)
{
    SetOwner(0);
}

FunctionNode* FunctionGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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
    stack.push(std::unique_ptr<cminor::parsing::Object>(new ValueObject<ParsingContext*>(ctx)));
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
    FunctionNode* result = *static_cast<cminor::parsing::ValueObject<FunctionNode*>*>(value.get());
    stack.pop();
    return result;
}

class FunctionGrammar::FunctionRule : public cminor::parsing::Rule
{
public:
    FunctionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("FunctionNode*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("AttributeMap"), ToUtf32("attributeMap")));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<FunctionNode>"), ToUtf32("fun")));
        AddLocalVariable(AttrOrVariable(ToUtf32("Span"), ToUtf32("s")));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<IdentifierNode>"), ToUtf32("qid")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context->ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<FunctionNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FunctionRule>(this, &FunctionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FunctionRule>(this, &FunctionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FunctionRule>(this, &FunctionRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FunctionRule>(this, &FunctionRule::A3Action));
        cminor::parsing::NonterminalParser* attributesNonterminalParser = GetNonterminal(ToUtf32("Attributes"));
        attributesNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<FunctionRule>(this, &FunctionRule::PreAttributes));
        cminor::parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal(ToUtf32("Specifiers"));
        specifiersNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<FunctionRule>(this, &FunctionRule::PostSpecifiers));
        cminor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal(ToUtf32("TypeExpr"));
        typeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<FunctionRule>(this, &FunctionRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<FunctionRule>(this, &FunctionRule::PostTypeExpr));
        cminor::parsing::NonterminalParser* functionGroupIdNonterminalParser = GetNonterminal(ToUtf32("FunctionGroupId"));
        functionGroupIdNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<FunctionRule>(this, &FunctionRule::PreFunctionGroupId));
        functionGroupIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<FunctionRule>(this, &FunctionRule::PostFunctionGroupId));
        cminor::parsing::NonterminalParser* parameterListNonterminalParser = GetNonterminal(ToUtf32("ParameterList"));
        parameterListNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<FunctionRule>(this, &FunctionRule::PreParameterList));
        cminor::parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal(ToUtf32("CompoundStatement"));
        compoundStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<FunctionRule>(this, &FunctionRule::PreCompoundStatement));
        compoundStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<FunctionRule>(this, &FunctionRule::PostCompoundStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fun.release();
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->fun.reset(new FunctionNode(span, context->fromSpecifiers, context->fromTypeExpr, context->fromFunctionGroupId));
        context->fun->SetAttributes(context->attributeMap);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->s = span;
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->fun->SetBody(context->fromCompoundStatement);
        context->fun->GetSpan().SetEnd(context->s.End());
    }
    void PreAttributes(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<AttributeMap*>(&context->attributeMap)));
    }
    void PostSpecifiers(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context->fromSpecifiers = *static_cast<cminor::parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
    }
    void PreTypeExpr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostTypeExpr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context->fromTypeExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
            stack.pop();
        }
    }
    void PreFunctionGroupId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostFunctionGroupId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFunctionGroupId_value = std::move(stack.top());
            context->fromFunctionGroupId = *static_cast<cminor::parsing::ValueObject<FunctionGroupIdNode*>*>(fromFunctionGroupId_value.get());
            stack.pop();
        }
    }
    void PreParameterList(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->fun.get())));
    }
    void PreCompoundStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostCompoundStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context->fromCompoundStatement = *static_cast<cminor::parsing::ValueObject<CompoundStatementNode*>*>(fromCompoundStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), attributeMap(), fun(), s(), qid(), fromSpecifiers(), fromTypeExpr(), fromFunctionGroupId(), fromCompoundStatement() {}
        ParsingContext* ctx;
        FunctionNode* value;
        AttributeMap attributeMap;
        std::unique_ptr<FunctionNode> fun;
        Span s;
        std::unique_ptr<IdentifierNode> qid;
        Specifiers fromSpecifiers;
        Node* fromTypeExpr;
        FunctionGroupIdNode* fromFunctionGroupId;
        CompoundStatementNode* fromCompoundStatement;
    };
};

class FunctionGrammar::FunctionGroupIdRule : public cminor::parsing::Rule
{
public:
    FunctionGroupIdRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("FunctionGroupIdNode*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context->ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<FunctionGroupIdNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FunctionGroupIdRule>(this, &FunctionGroupIdRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FunctionGroupIdRule>(this, &FunctionGroupIdRule::A1Action));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("identifier"));
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<FunctionGroupIdRule>(this, &FunctionGroupIdRule::Postidentifier));
        cminor::parsing::NonterminalParser* operatorFunctionGroupIdNonterminalParser = GetNonterminal(ToUtf32("OperatorFunctionGroupId"));
        operatorFunctionGroupIdNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<FunctionGroupIdRule>(this, &FunctionGroupIdRule::PreOperatorFunctionGroupId));
        operatorFunctionGroupIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<FunctionGroupIdRule>(this, &FunctionGroupIdRule::PostOperatorFunctionGroupId));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new FunctionGroupIdNode(span, context->fromidentifier);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromOperatorFunctionGroupId;
    }
    void Postidentifier(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromidentifier_value = std::move(stack.top());
            context->fromidentifier = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromidentifier_value.get());
            stack.pop();
        }
    }
    void PreOperatorFunctionGroupId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostOperatorFunctionGroupId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromOperatorFunctionGroupId_value = std::move(stack.top());
            context->fromOperatorFunctionGroupId = *static_cast<cminor::parsing::ValueObject<FunctionGroupIdNode*>*>(fromOperatorFunctionGroupId_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), fromidentifier(), fromOperatorFunctionGroupId() {}
        ParsingContext* ctx;
        FunctionGroupIdNode* value;
        std::u32string fromidentifier;
        FunctionGroupIdNode* fromOperatorFunctionGroupId;
    };
};

class FunctionGrammar::OperatorFunctionGroupIdRule : public cminor::parsing::Rule
{
public:
    OperatorFunctionGroupIdRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("FunctionGroupIdNode*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<Node>"), ToUtf32("typeExpr")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context->ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<FunctionGroupIdNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction(ToUtf32("A4"));
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction(ToUtf32("A5"));
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction(ToUtf32("A6"));
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction(ToUtf32("A7"));
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction(ToUtf32("A8"));
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A8Action));
        cminor::parsing::ActionParser* a9ActionParser = GetAction(ToUtf32("A9"));
        a9ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A9Action));
        cminor::parsing::ActionParser* a10ActionParser = GetAction(ToUtf32("A10"));
        a10ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A10Action));
        cminor::parsing::ActionParser* a11ActionParser = GetAction(ToUtf32("A11"));
        a11ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A11Action));
        cminor::parsing::ActionParser* a12ActionParser = GetAction(ToUtf32("A12"));
        a12ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A12Action));
        cminor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal(ToUtf32("TypeExpr"));
        typeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::PostTypeExpr));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new FunctionGroupIdNode(span, ToUtf32("operator<<"));
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->typeExpr.reset(context->fromTypeExpr);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new FunctionGroupIdNode(span, ToUtf32("operator>>"));
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new FunctionGroupIdNode(span, ToUtf32("operator=="));
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new FunctionGroupIdNode(span, ToUtf32("operator<"));
    }
    void A5Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new FunctionGroupIdNode(span, ToUtf32("operator+"));
    }
    void A6Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new FunctionGroupIdNode(span, ToUtf32("operator-"));
    }
    void A7Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new FunctionGroupIdNode(span, ToUtf32("operator*"));
    }
    void A8Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new FunctionGroupIdNode(span, ToUtf32("operator/"));
    }
    void A9Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new FunctionGroupIdNode(span, ToUtf32("operator%"));
    }
    void A10Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new FunctionGroupIdNode(span, ToUtf32("operator^"));
    }
    void A11Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new FunctionGroupIdNode(span, ToUtf32("operator!"));
    }
    void A12Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new FunctionGroupIdNode(span, ToUtf32("operator()"));
    }
    void PreTypeExpr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostTypeExpr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context->fromTypeExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), typeExpr(), fromTypeExpr() {}
        ParsingContext* ctx;
        FunctionGroupIdNode* value;
        std::unique_ptr<Node> typeExpr;
        Node* fromTypeExpr;
    };
};

class FunctionGrammar::AttributesRule : public cminor::parsing::Rule
{
public:
    AttributesRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("AttributeMap*"), ToUtf32("attributeMap")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> attributeMap_value = std::move(stack.top());
        context->attributeMap = *static_cast<cminor::parsing::ValueObject<AttributeMap*>*>(attributeMap_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::NonterminalParser* nameValuePairNonterminalParser = GetNonterminal(ToUtf32("NameValuePair"));
        nameValuePairNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<AttributesRule>(this, &AttributesRule::PreNameValuePair));
    }
    void PreNameValuePair(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<AttributeMap*>(context->attributeMap)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): attributeMap() {}
        AttributeMap* attributeMap;
    };
};

class FunctionGrammar::NameValuePairRule : public cminor::parsing::Rule
{
public:
    NameValuePairRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("AttributeMap*"), ToUtf32("attributeMap")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> attributeMap_value = std::move(stack.top());
        context->attributeMap = *static_cast<cminor::parsing::ValueObject<AttributeMap*>*>(attributeMap_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NameValuePairRule>(this, &NameValuePairRule::A0Action));
        cminor::parsing::NonterminalParser* nameNonterminalParser = GetNonterminal(ToUtf32("Name"));
        nameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NameValuePairRule>(this, &NameValuePairRule::PostName));
        cminor::parsing::NonterminalParser* valueNonterminalParser = GetNonterminal(ToUtf32("Value"));
        valueNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NameValuePairRule>(this, &NameValuePairRule::PostValue));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->attributeMap->AddAttribute(context->fromName, context->fromValue);
    }
    void PostName(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromName_value = std::move(stack.top());
            context->fromName = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromName_value.get());
            stack.pop();
        }
    }
    void PostValue(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromValue_value = std::move(stack.top());
            context->fromValue = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromValue_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): attributeMap(), fromName(), fromValue() {}
        AttributeMap* attributeMap;
        std::u32string fromName;
        std::u32string fromValue;
    };
};

class FunctionGrammar::NameRule : public cminor::parsing::Rule
{
public:
    NameRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NameRule>(this, &NameRule::A0Action));
        cminor::parsing::NonterminalParser* qualified_idNonterminalParser = GetNonterminal(ToUtf32("qualified_id"));
        qualified_idNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NameRule>(this, &NameRule::Postqualified_id));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromqualified_id;
    }
    void Postqualified_id(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromqualified_id_value = std::move(stack.top());
            context->fromqualified_id = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromqualified_id_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromqualified_id() {}
        std::u32string value;
        std::u32string fromqualified_id;
    };
};

class FunctionGrammar::ValueRule : public cminor::parsing::Rule
{
public:
    ValueRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ValueRule>(this, &ValueRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ValueRule>(this, &ValueRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ValueRule>(this, &ValueRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ValueRule>(this, &ValueRule::A3Action));
        cminor::parsing::NonterminalParser* qualified_idNonterminalParser = GetNonterminal(ToUtf32("qualified_id"));
        qualified_idNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ValueRule>(this, &ValueRule::Postqualified_id));
        cminor::parsing::NonterminalParser* stringNonterminalParser = GetNonterminal(ToUtf32("string"));
        stringNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ValueRule>(this, &ValueRule::Poststring));
        cminor::parsing::NonterminalParser* longNonterminalParser = GetNonterminal(ToUtf32("long"));
        longNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ValueRule>(this, &ValueRule::Postlong));
        cminor::parsing::NonterminalParser* boolNonterminalParser = GetNonterminal(ToUtf32("bool"));
        boolNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ValueRule>(this, &ValueRule::Postbool));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromqualified_id;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromstring;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = std::u32string(matchBegin, matchEnd);
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = std::u32string(matchBegin, matchEnd);
    }
    void Postqualified_id(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromqualified_id_value = std::move(stack.top());
            context->fromqualified_id = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromqualified_id_value.get());
            stack.pop();
        }
    }
    void Poststring(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromstring_value = std::move(stack.top());
            context->fromstring = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromstring_value.get());
            stack.pop();
        }
    }
    void Postlong(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromlong_value = std::move(stack.top());
            context->fromlong = *static_cast<cminor::parsing::ValueObject<int64_t>*>(fromlong_value.get());
            stack.pop();
        }
    }
    void Postbool(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frombool_value = std::move(stack.top());
            context->frombool = *static_cast<cminor::parsing::ValueObject<bool>*>(frombool_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromqualified_id(), fromstring(), fromlong(), frombool() {}
        std::u32string value;
        std::u32string fromqualified_id;
        std::u32string fromstring;
        int64_t fromlong;
        bool frombool;
    };
};

void FunctionGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cminor.parsing.stdlib"));
    if (!grammar0)
    {
        grammar0 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar(ToUtf32("cminor.parser.KeywordGrammar"));
    if (!grammar1)
    {
        grammar1 = cminor::parser::KeywordGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar(ToUtf32("cminor.parser.ParameterGrammar"));
    if (!grammar2)
    {
        grammar2 = cminor::parser::ParameterGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar(ToUtf32("cminor.parser.SpecifierGrammar"));
    if (!grammar3)
    {
        grammar3 = cminor::parser::SpecifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
    cminor::parsing::Grammar* grammar4 = pd->GetGrammar(ToUtf32("cminor.parser.TypeExprGrammar"));
    if (!grammar4)
    {
        grammar4 = cminor::parser::TypeExprGrammar::Create(pd);
    }
    AddGrammarReference(grammar4);
    cminor::parsing::Grammar* grammar5 = pd->GetGrammar(ToUtf32("cminor.parser.StatementGrammar"));
    if (!grammar5)
    {
        grammar5 = cminor::parser::StatementGrammar::Create(pd);
    }
    AddGrammarReference(grammar5);
    cminor::parsing::Grammar* grammar6 = pd->GetGrammar(ToUtf32("cminor.parser.IdentifierGrammar"));
    if (!grammar6)
    {
        grammar6 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar6);
}

void FunctionGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("bool"), this, ToUtf32("cminor.parsing.stdlib.bool")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Specifiers"), this, ToUtf32("SpecifierGrammar.Specifiers")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("qualified_id"), this, ToUtf32("cminor.parsing.stdlib.qualified_id")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("ParameterList"), this, ToUtf32("ParameterGrammar.ParameterList")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("TypeExpr"), this, ToUtf32("TypeExprGrammar.TypeExpr")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("string"), this, ToUtf32("cminor.parsing.stdlib.string")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("long"), this, ToUtf32("cminor.parsing.stdlib.long")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Keyword"), this, ToUtf32("KeywordGrammar.Keyword")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("CompoundStatement"), this, ToUtf32("StatementGrammar.CompoundStatement")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("QualifiedId"), this, ToUtf32("IdentifierGrammar.QualifiedId")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("identifier"), this, ToUtf32("cminor.parsing.stdlib.identifier")));
    AddRule(new FunctionRule(ToUtf32("Function"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::OptionalParser(
                                    new cminor::parsing::NonterminalParser(ToUtf32("Attributes"), ToUtf32("Attributes"), 1)),
                                new cminor::parsing::NonterminalParser(ToUtf32("Specifiers"), ToUtf32("Specifiers"), 0)),
                            new cminor::parsing::NonterminalParser(ToUtf32("TypeExpr"), ToUtf32("TypeExpr"), 1)),
                        new cminor::parsing::ActionParser(ToUtf32("A1"),
                            new cminor::parsing::NonterminalParser(ToUtf32("FunctionGroupId"), ToUtf32("FunctionGroupId"), 1))),
                    new cminor::parsing::ActionParser(ToUtf32("A2"),
                        new cminor::parsing::NonterminalParser(ToUtf32("ParameterList"), ToUtf32("ParameterList"), 2))),
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::ActionParser(ToUtf32("A3"),
                        new cminor::parsing::NonterminalParser(ToUtf32("CompoundStatement"), ToUtf32("CompoundStatement"), 1)),
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new FunctionGroupIdRule(ToUtf32("FunctionGroupId"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::DifferenceParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("identifier"), ToUtf32("identifier"), 0),
                    new cminor::parsing::NonterminalParser(ToUtf32("Keyword"), ToUtf32("Keyword"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::NonterminalParser(ToUtf32("OperatorFunctionGroupId"), ToUtf32("OperatorFunctionGroupId"), 1)))));
    AddRule(new OperatorFunctionGroupIdRule(ToUtf32("OperatorFunctionGroupId"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::KeywordParser(ToUtf32("operator")),
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
                                                        new cminor::parsing::ActionParser(ToUtf32("A0"),
                                                            new cminor::parsing::DifferenceParser(
                                                                new cminor::parsing::StringParser(ToUtf32("<<")),
                                                                new cminor::parsing::SequenceParser(
                                                                    new cminor::parsing::SequenceParser(
                                                                        new cminor::parsing::SequenceParser(
                                                                            new cminor::parsing::CharParser('<'),
                                                                            new cminor::parsing::CharParser('<')),
                                                                        new cminor::parsing::ListParser(
                                                                            new cminor::parsing::ActionParser(ToUtf32("A1"),
                                                                                new cminor::parsing::NonterminalParser(ToUtf32("TypeExpr"), ToUtf32("TypeExpr"), 1)),
                                                                            new cminor::parsing::CharParser(','))),
                                                                    new cminor::parsing::CharParser('>')))),
                                                        new cminor::parsing::ActionParser(ToUtf32("A2"),
                                                            new cminor::parsing::StringParser(ToUtf32(">>")))),
                                                    new cminor::parsing::ActionParser(ToUtf32("A3"),
                                                        new cminor::parsing::StringParser(ToUtf32("==")))),
                                                new cminor::parsing::ActionParser(ToUtf32("A4"),
                                                    new cminor::parsing::CharParser('<'))),
                                            new cminor::parsing::ActionParser(ToUtf32("A5"),
                                                new cminor::parsing::CharParser('+'))),
                                        new cminor::parsing::ActionParser(ToUtf32("A6"),
                                            new cminor::parsing::CharParser('-'))),
                                    new cminor::parsing::ActionParser(ToUtf32("A7"),
                                        new cminor::parsing::CharParser('*'))),
                                new cminor::parsing::ActionParser(ToUtf32("A8"),
                                    new cminor::parsing::CharParser('/'))),
                            new cminor::parsing::ActionParser(ToUtf32("A9"),
                                new cminor::parsing::CharParser('%'))),
                        new cminor::parsing::ActionParser(ToUtf32("A10"),
                            new cminor::parsing::CharParser('^'))),
                    new cminor::parsing::ActionParser(ToUtf32("A11"),
                        new cminor::parsing::CharParser('!'))),
                new cminor::parsing::ActionParser(ToUtf32("A12"),
                    new cminor::parsing::StringParser(ToUtf32("()")))))));
    AddRule(new AttributesRule(ToUtf32("Attributes"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('['),
                new cminor::parsing::ListParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("NameValuePair"), ToUtf32("NameValuePair"), 1),
                    new cminor::parsing::CharParser(','))),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::CharParser(']')))));
    AddRule(new NameValuePairRule(ToUtf32("NameValuePair"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("Name"), ToUtf32("Name"), 0),
                    new cminor::parsing::CharParser('=')),
                new cminor::parsing::NonterminalParser(ToUtf32("Value"), ToUtf32("Value"), 0)))));
    AddRule(new NameRule(ToUtf32("Name"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::NonterminalParser(ToUtf32("qualified_id"), ToUtf32("qualified_id"), 0))));
    AddRule(new ValueRule(ToUtf32("Value"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::ActionParser(ToUtf32("A0"),
                        new cminor::parsing::NonterminalParser(ToUtf32("qualified_id"), ToUtf32("qualified_id"), 0)),
                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                        new cminor::parsing::NonterminalParser(ToUtf32("string"), ToUtf32("string"), 0))),
                new cminor::parsing::ActionParser(ToUtf32("A2"),
                    new cminor::parsing::NonterminalParser(ToUtf32("long"), ToUtf32("long"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A3"),
                new cminor::parsing::NonterminalParser(ToUtf32("bool"), ToUtf32("bool"), 0)))));
}

} } // namespace cminor.parser
