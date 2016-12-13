#include "Function.hpp"
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>
#include <cminor/parser/Keyword.hpp>
#include <cminor/parser/Specifier.hpp>
#include <cminor/parser/TypeExpr.hpp>
#include <cminor/parser/Identifier.hpp>
#include <cminor/parser/Parameter.hpp>
#include <cminor/parser/Statement.hpp>

namespace cminor { namespace parser {

using namespace cminor::parsing;

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

FunctionGrammar::FunctionGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("FunctionGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

FunctionNode* FunctionGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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
    FunctionNode* result = *static_cast<cminor::parsing::ValueObject<FunctionNode*>*>(value.get());
    stack.pop();
    return result;
}

class FunctionGrammar::FunctionRule : public cminor::parsing::Rule
{
public:
    FunctionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("FunctionNode*");
        AddLocalVariable(AttrOrVariable("AttributeMap", "attributeMap"));
        AddLocalVariable(AttrOrVariable("std::unique_ptr<FunctionNode>", "fun"));
        AddLocalVariable(AttrOrVariable("Span", "s"));
        AddLocalVariable(AttrOrVariable("std::unique_ptr<IdentifierNode>", "qid"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<FunctionNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FunctionRule>(this, &FunctionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FunctionRule>(this, &FunctionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FunctionRule>(this, &FunctionRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FunctionRule>(this, &FunctionRule::A3Action));
        cminor::parsing::NonterminalParser* attributesNonterminalParser = GetNonterminal("Attributes");
        attributesNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<FunctionRule>(this, &FunctionRule::PreAttributes));
        cminor::parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal("Specifiers");
        specifiersNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<FunctionRule>(this, &FunctionRule::PostSpecifiers));
        cminor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<FunctionRule>(this, &FunctionRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<FunctionRule>(this, &FunctionRule::PostTypeExpr));
        cminor::parsing::NonterminalParser* functionGroupIdNonterminalParser = GetNonterminal("FunctionGroupId");
        functionGroupIdNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<FunctionRule>(this, &FunctionRule::PreFunctionGroupId));
        functionGroupIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<FunctionRule>(this, &FunctionRule::PostFunctionGroupId));
        cminor::parsing::NonterminalParser* parameterListNonterminalParser = GetNonterminal("ParameterList");
        parameterListNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<FunctionRule>(this, &FunctionRule::PreParameterList));
        cminor::parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal("CompoundStatement");
        compoundStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<FunctionRule>(this, &FunctionRule::PreCompoundStatement));
        compoundStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<FunctionRule>(this, &FunctionRule::PostCompoundStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fun.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.fun.reset(new FunctionNode(span, context.fromSpecifiers, context.fromTypeExpr, context.fromFunctionGroupId));
        context.fun->SetAttributes(context.attributeMap);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s = span;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.fun->SetBody(context.fromCompoundStatement);
        context.fun->GetSpan().SetEnd(context.s.End());
    }
    void PreAttributes(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<AttributeMap*>(&context.attributeMap)));
    }
    void PostSpecifiers(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context.fromSpecifiers = *static_cast<cminor::parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
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
    void PreFunctionGroupId(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostFunctionGroupId(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFunctionGroupId_value = std::move(stack.top());
            context.fromFunctionGroupId = *static_cast<cminor::parsing::ValueObject<FunctionGroupIdNode*>*>(fromFunctionGroupId_value.get());
            stack.pop();
        }
    }
    void PreParameterList(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.fun.get())));
    }
    void PreCompoundStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostCompoundStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context.fromCompoundStatement = *static_cast<cminor::parsing::ValueObject<CompoundStatementNode*>*>(fromCompoundStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
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
    std::stack<Context> contextStack;
    Context context;
};

class FunctionGrammar::FunctionGroupIdRule : public cminor::parsing::Rule
{
public:
    FunctionGroupIdRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("FunctionGroupIdNode*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<FunctionGroupIdNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FunctionGroupIdRule>(this, &FunctionGroupIdRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FunctionGroupIdRule>(this, &FunctionGroupIdRule::A1Action));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<FunctionGroupIdRule>(this, &FunctionGroupIdRule::Postidentifier));
        cminor::parsing::NonterminalParser* operatorFunctionGroupIdNonterminalParser = GetNonterminal("OperatorFunctionGroupId");
        operatorFunctionGroupIdNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<FunctionGroupIdRule>(this, &FunctionGroupIdRule::PreOperatorFunctionGroupId));
        operatorFunctionGroupIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<FunctionGroupIdRule>(this, &FunctionGroupIdRule::PostOperatorFunctionGroupId));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, context.fromidentifier);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromOperatorFunctionGroupId;
    }
    void Postidentifier(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromidentifier_value = std::move(stack.top());
            context.fromidentifier = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromidentifier_value.get());
            stack.pop();
        }
    }
    void PreOperatorFunctionGroupId(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostOperatorFunctionGroupId(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromOperatorFunctionGroupId_value = std::move(stack.top());
            context.fromOperatorFunctionGroupId = *static_cast<cminor::parsing::ValueObject<FunctionGroupIdNode*>*>(fromOperatorFunctionGroupId_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromidentifier(), fromOperatorFunctionGroupId() {}
        ParsingContext* ctx;
        FunctionGroupIdNode* value;
        std::string fromidentifier;
        FunctionGroupIdNode* fromOperatorFunctionGroupId;
    };
    std::stack<Context> contextStack;
    Context context;
};

class FunctionGrammar::OperatorFunctionGroupIdRule : public cminor::parsing::Rule
{
public:
    OperatorFunctionGroupIdRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("FunctionGroupIdNode*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "typeExpr"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<FunctionGroupIdNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction("A6");
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction("A7");
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction("A8");
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A8Action));
        cminor::parsing::ActionParser* a9ActionParser = GetAction("A9");
        a9ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A9Action));
        cminor::parsing::ActionParser* a10ActionParser = GetAction("A10");
        a10ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A10Action));
        cminor::parsing::ActionParser* a11ActionParser = GetAction("A11");
        a11ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A11Action));
        cminor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::PostTypeExpr));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, "operator<<");
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.typeExpr.reset(context.fromTypeExpr);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, "operator>>");
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, "operator==");
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, "operator<");
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, "operator+");
    }
    void A6Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, "operator-");
    }
    void A7Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, "operator*");
    }
    void A8Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, "operator/");
    }
    void A9Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, "operator^");
    }
    void A10Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, "operator!");
    }
    void A11Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, "operator()");
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
private:
    struct Context
    {
        Context(): ctx(), value(), typeExpr(), fromTypeExpr() {}
        ParsingContext* ctx;
        FunctionGroupIdNode* value;
        std::unique_ptr<Node> typeExpr;
        Node* fromTypeExpr;
    };
    std::stack<Context> contextStack;
    Context context;
};

class FunctionGrammar::AttributesRule : public cminor::parsing::Rule
{
public:
    AttributesRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("AttributeMap*", "attributeMap"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> attributeMap_value = std::move(stack.top());
        context.attributeMap = *static_cast<cminor::parsing::ValueObject<AttributeMap*>*>(attributeMap_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::NonterminalParser* nameValuePairNonterminalParser = GetNonterminal("NameValuePair");
        nameValuePairNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<AttributesRule>(this, &AttributesRule::PreNameValuePair));
    }
    void PreNameValuePair(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<AttributeMap*>(context.attributeMap)));
    }
private:
    struct Context
    {
        Context(): attributeMap() {}
        AttributeMap* attributeMap;
    };
    std::stack<Context> contextStack;
    Context context;
};

class FunctionGrammar::NameValuePairRule : public cminor::parsing::Rule
{
public:
    NameValuePairRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("AttributeMap*", "attributeMap"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> attributeMap_value = std::move(stack.top());
        context.attributeMap = *static_cast<cminor::parsing::ValueObject<AttributeMap*>*>(attributeMap_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NameValuePairRule>(this, &NameValuePairRule::A0Action));
        cminor::parsing::NonterminalParser* nameNonterminalParser = GetNonterminal("Name");
        nameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NameValuePairRule>(this, &NameValuePairRule::PostName));
        cminor::parsing::NonterminalParser* valueNonterminalParser = GetNonterminal("Value");
        valueNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NameValuePairRule>(this, &NameValuePairRule::PostValue));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.attributeMap->AddAttribute(context.fromName, context.fromValue);
    }
    void PostName(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromName_value = std::move(stack.top());
            context.fromName = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromName_value.get());
            stack.pop();
        }
    }
    void PostValue(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromValue_value = std::move(stack.top());
            context.fromValue = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromValue_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): attributeMap(), fromName(), fromValue() {}
        AttributeMap* attributeMap;
        std::string fromName;
        std::string fromValue;
    };
    std::stack<Context> contextStack;
    Context context;
};

class FunctionGrammar::NameRule : public cminor::parsing::Rule
{
public:
    NameRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NameRule>(this, &NameRule::A0Action));
        cminor::parsing::NonterminalParser* qualified_idNonterminalParser = GetNonterminal("qualified_id");
        qualified_idNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NameRule>(this, &NameRule::Postqualified_id));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromqualified_id;
    }
    void Postqualified_id(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromqualified_id_value = std::move(stack.top());
            context.fromqualified_id = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromqualified_id_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromqualified_id() {}
        std::string value;
        std::string fromqualified_id;
    };
    std::stack<Context> contextStack;
    Context context;
};

class FunctionGrammar::ValueRule : public cminor::parsing::Rule
{
public:
    ValueRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ValueRule>(this, &ValueRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ValueRule>(this, &ValueRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ValueRule>(this, &ValueRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ValueRule>(this, &ValueRule::A3Action));
        cminor::parsing::NonterminalParser* qualified_idNonterminalParser = GetNonterminal("qualified_id");
        qualified_idNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ValueRule>(this, &ValueRule::Postqualified_id));
        cminor::parsing::NonterminalParser* stringNonterminalParser = GetNonterminal("string");
        stringNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ValueRule>(this, &ValueRule::Poststring));
        cminor::parsing::NonterminalParser* longNonterminalParser = GetNonterminal("long");
        longNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ValueRule>(this, &ValueRule::Postlong));
        cminor::parsing::NonterminalParser* boolNonterminalParser = GetNonterminal("bool");
        boolNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ValueRule>(this, &ValueRule::Postbool));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromqualified_id;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromstring;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = std::string(matchBegin, matchEnd);
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = std::string(matchBegin, matchEnd);
    }
    void Postqualified_id(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromqualified_id_value = std::move(stack.top());
            context.fromqualified_id = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromqualified_id_value.get());
            stack.pop();
        }
    }
    void Poststring(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromstring_value = std::move(stack.top());
            context.fromstring = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromstring_value.get());
            stack.pop();
        }
    }
    void Postlong(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromlong_value = std::move(stack.top());
            context.fromlong = *static_cast<cminor::parsing::ValueObject<int64_t>*>(fromlong_value.get());
            stack.pop();
        }
    }
    void Postbool(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frombool_value = std::move(stack.top());
            context.frombool = *static_cast<cminor::parsing::ValueObject<bool>*>(frombool_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromqualified_id(), fromstring(), fromlong(), frombool() {}
        std::string value;
        std::string fromqualified_id;
        std::string fromstring;
        int64_t fromlong;
        bool frombool;
    };
    std::stack<Context> contextStack;
    Context context;
};

void FunctionGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("cminor.parser.StatementGrammar");
    if (!grammar0)
    {
        grammar0 = cminor::parser::StatementGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar("cminor.parser.IdentifierGrammar");
    if (!grammar1)
    {
        grammar1 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar("cminor.parser.SpecifierGrammar");
    if (!grammar2)
    {
        grammar2 = cminor::parser::SpecifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar("cminor.parser.TypeExprGrammar");
    if (!grammar3)
    {
        grammar3 = cminor::parser::TypeExprGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
    cminor::parsing::Grammar* grammar4 = pd->GetGrammar("cminor.parsing.stdlib");
    if (!grammar4)
    {
        grammar4 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar4);
    cminor::parsing::Grammar* grammar5 = pd->GetGrammar("cminor.parser.KeywordGrammar");
    if (!grammar5)
    {
        grammar5 = cminor::parser::KeywordGrammar::Create(pd);
    }
    AddGrammarReference(grammar5);
    cminor::parsing::Grammar* grammar6 = pd->GetGrammar("cminor.parser.ParameterGrammar");
    if (!grammar6)
    {
        grammar6 = cminor::parser::ParameterGrammar::Create(pd);
    }
    AddGrammarReference(grammar6);
}

void FunctionGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("QualifiedId", this, "IdentifierGrammar.QualifiedId"));
    AddRuleLink(new cminor::parsing::RuleLink("Specifiers", this, "SpecifierGrammar.Specifiers"));
    AddRuleLink(new cminor::parsing::RuleLink("TypeExpr", this, "TypeExprGrammar.TypeExpr"));
    AddRuleLink(new cminor::parsing::RuleLink("qualified_id", this, "cminor.parsing.stdlib.qualified_id"));
    AddRuleLink(new cminor::parsing::RuleLink("ParameterList", this, "ParameterGrammar.ParameterList"));
    AddRuleLink(new cminor::parsing::RuleLink("Keyword", this, "KeywordGrammar.Keyword"));
    AddRuleLink(new cminor::parsing::RuleLink("CompoundStatement", this, "StatementGrammar.CompoundStatement"));
    AddRuleLink(new cminor::parsing::RuleLink("identifier", this, "cminor.parsing.stdlib.identifier"));
    AddRuleLink(new cminor::parsing::RuleLink("string", this, "cminor.parsing.stdlib.string"));
    AddRuleLink(new cminor::parsing::RuleLink("long", this, "cminor.parsing.stdlib.long"));
    AddRuleLink(new cminor::parsing::RuleLink("bool", this, "cminor.parsing.stdlib.bool"));
    AddRule(new FunctionRule("Function", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::OptionalParser(
                                    new cminor::parsing::NonterminalParser("Attributes", "Attributes", 1)),
                                new cminor::parsing::NonterminalParser("Specifiers", "Specifiers", 0)),
                            new cminor::parsing::NonterminalParser("TypeExpr", "TypeExpr", 1)),
                        new cminor::parsing::ActionParser("A1",
                            new cminor::parsing::NonterminalParser("FunctionGroupId", "FunctionGroupId", 1))),
                    new cminor::parsing::ActionParser("A2",
                        new cminor::parsing::NonterminalParser("ParameterList", "ParameterList", 2))),
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::ActionParser("A3",
                        new cminor::parsing::NonterminalParser("CompoundStatement", "CompoundStatement", 1)),
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new FunctionGroupIdRule("FunctionGroupId", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::DifferenceParser(
                    new cminor::parsing::NonterminalParser("identifier", "identifier", 0),
                    new cminor::parsing::NonterminalParser("Keyword", "Keyword", 0))),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::NonterminalParser("OperatorFunctionGroupId", "OperatorFunctionGroupId", 1)))));
    AddRule(new OperatorFunctionGroupIdRule("OperatorFunctionGroupId", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::KeywordParser("operator"),
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
                                                    new cminor::parsing::ActionParser("A0",
                                                        new cminor::parsing::DifferenceParser(
                                                            new cminor::parsing::StringParser("<<"),
                                                            new cminor::parsing::SequenceParser(
                                                                new cminor::parsing::SequenceParser(
                                                                    new cminor::parsing::SequenceParser(
                                                                        new cminor::parsing::CharParser('<'),
                                                                        new cminor::parsing::CharParser('<')),
                                                                    new cminor::parsing::ListParser(
                                                                        new cminor::parsing::ActionParser("A1",
                                                                            new cminor::parsing::NonterminalParser("TypeExpr", "TypeExpr", 1)),
                                                                        new cminor::parsing::CharParser(','))),
                                                                new cminor::parsing::CharParser('>')))),
                                                    new cminor::parsing::ActionParser("A2",
                                                        new cminor::parsing::StringParser(">>"))),
                                                new cminor::parsing::ActionParser("A3",
                                                    new cminor::parsing::StringParser("=="))),
                                            new cminor::parsing::ActionParser("A4",
                                                new cminor::parsing::CharParser('<'))),
                                        new cminor::parsing::ActionParser("A5",
                                            new cminor::parsing::CharParser('+'))),
                                    new cminor::parsing::ActionParser("A6",
                                        new cminor::parsing::CharParser('-'))),
                                new cminor::parsing::ActionParser("A7",
                                    new cminor::parsing::CharParser('*'))),
                            new cminor::parsing::ActionParser("A8",
                                new cminor::parsing::CharParser('/'))),
                        new cminor::parsing::ActionParser("A9",
                            new cminor::parsing::CharParser('^'))),
                    new cminor::parsing::ActionParser("A10",
                        new cminor::parsing::CharParser('!'))),
                new cminor::parsing::ActionParser("A11",
                    new cminor::parsing::StringParser("()"))))));
    AddRule(new AttributesRule("Attributes", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('['),
                new cminor::parsing::ListParser(
                    new cminor::parsing::NonterminalParser("NameValuePair", "NameValuePair", 1),
                    new cminor::parsing::CharParser(','))),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::CharParser(']')))));
    AddRule(new NameValuePairRule("NameValuePair", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser("Name", "Name", 0),
                    new cminor::parsing::CharParser('=')),
                new cminor::parsing::NonterminalParser("Value", "Value", 0)))));
    AddRule(new NameRule("Name", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::NonterminalParser("qualified_id", "qualified_id", 0))));
    AddRule(new ValueRule("Value", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::ActionParser("A0",
                        new cminor::parsing::NonterminalParser("qualified_id", "qualified_id", 0)),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::NonterminalParser("string", "string", 0))),
                new cminor::parsing::ActionParser("A2",
                    new cminor::parsing::NonterminalParser("long", "long", 0))),
            new cminor::parsing::ActionParser("A3",
                new cminor::parsing::NonterminalParser("bool", "bool", 0)))));
}

} } // namespace cminor.parser
