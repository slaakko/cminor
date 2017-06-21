#include "TypeExpr.hpp"
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
#include <cminor/parser/BasicType.hpp>
#include <cminor/parser/Expression.hpp>
#include <cminor/parser/Template.hpp>
#include <cminor/parser/Identifier.hpp>

namespace cminor { namespace parser {

using namespace cminor::parsing;
using namespace cminor::util;
using namespace cminor::unicode;

TypeExprGrammar* TypeExprGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

TypeExprGrammar* TypeExprGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    TypeExprGrammar* grammar(new TypeExprGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

TypeExprGrammar::TypeExprGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("TypeExprGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.parser")), parsingDomain_)
{
    SetOwner(0);
}

Node* TypeExprGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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
    Node* result = *static_cast<cminor::parsing::ValueObject<Node*>*>(value.get());
    stack.pop();
    return result;
}

class TypeExprGrammar::TypeExprRule : public cminor::parsing::Rule
{
public:
    TypeExprRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("Node*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeExprRule>(this, &TypeExprRule::A0Action));
        cminor::parsing::NonterminalParser* prefixTypeExprNonterminalParser = GetNonterminal(ToUtf32("PrefixTypeExpr"));
        prefixTypeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<TypeExprRule>(this, &TypeExprRule::PrePrefixTypeExpr));
        prefixTypeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TypeExprRule>(this, &TypeExprRule::PostPrefixTypeExpr));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromPrefixTypeExpr;
    }
    void PrePrefixTypeExpr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostPrefixTypeExpr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromPrefixTypeExpr_value = std::move(stack.top());
            context->fromPrefixTypeExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromPrefixTypeExpr_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), fromPrefixTypeExpr() {}
        ParsingContext* ctx;
        Node* value;
        Node* fromPrefixTypeExpr;
    };
};

class TypeExprGrammar::PrefixTypeExprRule : public cminor::parsing::Rule
{
public:
    PrefixTypeExprRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("Node*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrefixTypeExprRule>(this, &PrefixTypeExprRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrefixTypeExprRule>(this, &PrefixTypeExprRule::A1Action));
        cminor::parsing::NonterminalParser* refExprNonterminalParser = GetNonterminal(ToUtf32("refExpr"));
        refExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PrefixTypeExprRule>(this, &PrefixTypeExprRule::PrerefExpr));
        refExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrefixTypeExprRule>(this, &PrefixTypeExprRule::PostrefExpr));
        cminor::parsing::NonterminalParser* pfNonterminalParser = GetNonterminal(ToUtf32("pf"));
        pfNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PrefixTypeExprRule>(this, &PrefixTypeExprRule::Prepf));
        pfNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrefixTypeExprRule>(this, &PrefixTypeExprRule::Postpf));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new RefTypeExprNode(span, context->fromrefExpr);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->frompf;
    }
    void PrerefExpr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostrefExpr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromrefExpr_value = std::move(stack.top());
            context->fromrefExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromrefExpr_value.get());
            stack.pop();
        }
    }
    void Prepf(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void Postpf(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frompf_value = std::move(stack.top());
            context->frompf = *static_cast<cminor::parsing::ValueObject<Node*>*>(frompf_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), fromrefExpr(), frompf() {}
        ParsingContext* ctx;
        Node* value;
        Node* fromrefExpr;
        Node* frompf;
    };
};

class TypeExprGrammar::PostfixTypeExprRule : public cminor::parsing::Rule
{
public:
    PostfixTypeExprRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("Node*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<Node>"), ToUtf32("typeExpr")));
        AddLocalVariable(AttrOrVariable(ToUtf32("Span"), ToUtf32("s")));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixTypeExprRule>(this, &PostfixTypeExprRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixTypeExprRule>(this, &PostfixTypeExprRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixTypeExprRule>(this, &PostfixTypeExprRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixTypeExprRule>(this, &PostfixTypeExprRule::A3Action));
        cminor::parsing::NonterminalParser* primaryTypeExprNonterminalParser = GetNonterminal(ToUtf32("PrimaryTypeExpr"));
        primaryTypeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PostfixTypeExprRule>(this, &PostfixTypeExprRule::PrePrimaryTypeExpr));
        primaryTypeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixTypeExprRule>(this, &PostfixTypeExprRule::PostPrimaryTypeExpr));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("Identifier"));
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixTypeExprRule>(this, &PostfixTypeExprRule::PostIdentifier));
        cminor::parsing::NonterminalParser* sizeNonterminalParser = GetNonterminal(ToUtf32("size"));
        sizeNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PostfixTypeExprRule>(this, &PostfixTypeExprRule::Presize));
        sizeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixTypeExprRule>(this, &PostfixTypeExprRule::Postsize));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->typeExpr.release();
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->typeExpr.reset(context->fromPrimaryTypeExpr);
        context->s = span;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->s.SetEnd(span.End());
        context->typeExpr.reset(new DotNode(context->s, context->typeExpr.release(), context->fromIdentifier));
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->s.SetEnd(span.End());
        context->typeExpr.reset(new ArrayNode(context->s, context->typeExpr.release(), context->fromsize));
    }
    void PrePrimaryTypeExpr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostPrimaryTypeExpr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromPrimaryTypeExpr_value = std::move(stack.top());
            context->fromPrimaryTypeExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromPrimaryTypeExpr_value.get());
            stack.pop();
        }
    }
    void PostIdentifier(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIdentifier_value = std::move(stack.top());
            context->fromIdentifier = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
    void Presize(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void Postsize(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromsize_value = std::move(stack.top());
            context->fromsize = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromsize_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), typeExpr(), s(), fromPrimaryTypeExpr(), fromIdentifier(), fromsize() {}
        ParsingContext* ctx;
        Node* value;
        std::unique_ptr<Node> typeExpr;
        Span s;
        Node* fromPrimaryTypeExpr;
        IdentifierNode* fromIdentifier;
        Node* fromsize;
    };
};

class TypeExprGrammar::PrimaryTypeExprRule : public cminor::parsing::Rule
{
public:
    PrimaryTypeExprRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("Node*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryTypeExprRule>(this, &PrimaryTypeExprRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryTypeExprRule>(this, &PrimaryTypeExprRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryTypeExprRule>(this, &PrimaryTypeExprRule::A2Action));
        cminor::parsing::NonterminalParser* basicTypeNonterminalParser = GetNonterminal(ToUtf32("BasicType"));
        basicTypeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryTypeExprRule>(this, &PrimaryTypeExprRule::PostBasicType));
        cminor::parsing::NonterminalParser* templateIdNonterminalParser = GetNonterminal(ToUtf32("TemplateId"));
        templateIdNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PrimaryTypeExprRule>(this, &PrimaryTypeExprRule::PreTemplateId));
        templateIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryTypeExprRule>(this, &PrimaryTypeExprRule::PostTemplateId));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("Identifier"));
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryTypeExprRule>(this, &PrimaryTypeExprRule::PostIdentifier));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromBasicType;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromTemplateId;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromIdentifier;
    }
    void PostBasicType(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromBasicType_value = std::move(stack.top());
            context->fromBasicType = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromBasicType_value.get());
            stack.pop();
        }
    }
    void PreTemplateId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostTemplateId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTemplateId_value = std::move(stack.top());
            context->fromTemplateId = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromTemplateId_value.get());
            stack.pop();
        }
    }
    void PostIdentifier(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIdentifier_value = std::move(stack.top());
            context->fromIdentifier = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), fromBasicType(), fromTemplateId(), fromIdentifier() {}
        ParsingContext* ctx;
        Node* value;
        Node* fromBasicType;
        Node* fromTemplateId;
        IdentifierNode* fromIdentifier;
    };
};

void TypeExprGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cminor.parser.ExpressionGrammar"));
    if (!grammar0)
    {
        grammar0 = cminor::parser::ExpressionGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar(ToUtf32("cminor.parser.IdentifierGrammar"));
    if (!grammar1)
    {
        grammar1 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar(ToUtf32("cminor.parser.TemplateGrammar"));
    if (!grammar2)
    {
        grammar2 = cminor::parser::TemplateGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar(ToUtf32("cminor.parser.BasicTypeGrammar"));
    if (!grammar3)
    {
        grammar3 = cminor::parser::BasicTypeGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
}

void TypeExprGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Expression"), this, ToUtf32("ExpressionGrammar.Expression")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("TemplateId"), this, ToUtf32("TemplateGrammar.TemplateId")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Identifier"), this, ToUtf32("IdentifierGrammar.Identifier")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("BasicType"), this, ToUtf32("BasicTypeGrammar.BasicType")));
    AddRule(new TypeExprRule(ToUtf32("TypeExpr"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::NonterminalParser(ToUtf32("PrefixTypeExpr"), ToUtf32("PrefixTypeExpr"), 1))));
    AddRule(new PrefixTypeExprRule(ToUtf32("PrefixTypeExpr"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser(ToUtf32("ref")),
                    new cminor::parsing::NonterminalParser(ToUtf32("refExpr"), ToUtf32("PostfixTypeExpr"), 1))),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::NonterminalParser(ToUtf32("pf"), ToUtf32("PostfixTypeExpr"), 1)))));
    AddRule(new PostfixTypeExprRule(ToUtf32("PostfixTypeExpr"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser(ToUtf32("A1"),
                    new cminor::parsing::NonterminalParser(ToUtf32("PrimaryTypeExpr"), ToUtf32("PrimaryTypeExpr"), 1)),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::CharParser('.'),
                            new cminor::parsing::ActionParser(ToUtf32("A2"),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::NonterminalParser(ToUtf32("Identifier"), ToUtf32("Identifier"), 0)))),
                        new cminor::parsing::ActionParser(ToUtf32("A3"),
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::CharParser('['),
                                    new cminor::parsing::OptionalParser(
                                        new cminor::parsing::NonterminalParser(ToUtf32("size"), ToUtf32("Expression"), 1))),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::CharParser(']'))))))))));
    AddRule(new PrimaryTypeExprRule(ToUtf32("PrimaryTypeExpr"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::NonterminalParser(ToUtf32("BasicType"), ToUtf32("BasicType"), 0)),
                new cminor::parsing::ActionParser(ToUtf32("A1"),
                    new cminor::parsing::NonterminalParser(ToUtf32("TemplateId"), ToUtf32("TemplateId"), 1))),
            new cminor::parsing::ActionParser(ToUtf32("A2"),
                new cminor::parsing::NonterminalParser(ToUtf32("Identifier"), ToUtf32("Identifier"), 0)))));
}

} } // namespace cminor.parser
