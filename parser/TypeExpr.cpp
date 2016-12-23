#include "TypeExpr.hpp"
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>
#include <cminor/parser/BasicType.hpp>
#include <cminor/parser/Expression.hpp>
#include <cminor/parser/Template.hpp>
#include <cminor/parser/Identifier.hpp>

namespace cminor { namespace parser {

using namespace cminor::parsing;

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

TypeExprGrammar::TypeExprGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("TypeExprGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

Node* TypeExprGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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

class TypeExprGrammar::TypeExprRule : public cminor::parsing::Rule
{
public:
    TypeExprRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeExprRule>(this, &TypeExprRule::A0Action));
        cminor::parsing::NonterminalParser* prefixTypeExprNonterminalParser = GetNonterminal("PrefixTypeExpr");
        prefixTypeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<TypeExprRule>(this, &TypeExprRule::PrePrefixTypeExpr));
        prefixTypeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TypeExprRule>(this, &TypeExprRule::PostPrefixTypeExpr));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromPrefixTypeExpr;
    }
    void PrePrefixTypeExpr(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostPrefixTypeExpr(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromPrefixTypeExpr_value = std::move(stack.top());
            context.fromPrefixTypeExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromPrefixTypeExpr_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromPrefixTypeExpr() {}
        ParsingContext* ctx;
        Node* value;
        Node* fromPrefixTypeExpr;
    };
    std::stack<Context> contextStack;
    Context context;
};

class TypeExprGrammar::PrefixTypeExprRule : public cminor::parsing::Rule
{
public:
    PrefixTypeExprRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrefixTypeExprRule>(this, &PrefixTypeExprRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrefixTypeExprRule>(this, &PrefixTypeExprRule::A1Action));
        cminor::parsing::NonterminalParser* refExprNonterminalParser = GetNonterminal("refExpr");
        refExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PrefixTypeExprRule>(this, &PrefixTypeExprRule::PrerefExpr));
        refExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrefixTypeExprRule>(this, &PrefixTypeExprRule::PostrefExpr));
        cminor::parsing::NonterminalParser* pfNonterminalParser = GetNonterminal("pf");
        pfNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PrefixTypeExprRule>(this, &PrefixTypeExprRule::Prepf));
        pfNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrefixTypeExprRule>(this, &PrefixTypeExprRule::Postpf));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new RefTypeExprNode(span, context.fromrefExpr);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.frompf;
    }
    void PrerefExpr(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostrefExpr(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromrefExpr_value = std::move(stack.top());
            context.fromrefExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromrefExpr_value.get());
            stack.pop();
        }
    }
    void Prepf(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postpf(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frompf_value = std::move(stack.top());
            context.frompf = *static_cast<cminor::parsing::ValueObject<Node*>*>(frompf_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromrefExpr(), frompf() {}
        ParsingContext* ctx;
        Node* value;
        Node* fromrefExpr;
        Node* frompf;
    };
    std::stack<Context> contextStack;
    Context context;
};

class TypeExprGrammar::PostfixTypeExprRule : public cminor::parsing::Rule
{
public:
    PostfixTypeExprRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "typeExpr"));
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixTypeExprRule>(this, &PostfixTypeExprRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixTypeExprRule>(this, &PostfixTypeExprRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixTypeExprRule>(this, &PostfixTypeExprRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixTypeExprRule>(this, &PostfixTypeExprRule::A3Action));
        cminor::parsing::NonterminalParser* primaryTypeExprNonterminalParser = GetNonterminal("PrimaryTypeExpr");
        primaryTypeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PostfixTypeExprRule>(this, &PostfixTypeExprRule::PrePrimaryTypeExpr));
        primaryTypeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixTypeExprRule>(this, &PostfixTypeExprRule::PostPrimaryTypeExpr));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixTypeExprRule>(this, &PostfixTypeExprRule::PostIdentifier));
        cminor::parsing::NonterminalParser* sizeNonterminalParser = GetNonterminal("size");
        sizeNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PostfixTypeExprRule>(this, &PostfixTypeExprRule::Presize));
        sizeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixTypeExprRule>(this, &PostfixTypeExprRule::Postsize));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.typeExpr.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.typeExpr.reset(context.fromPrimaryTypeExpr);
        context.s = span;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s.SetEnd(span.End());
        context.typeExpr.reset(new DotNode(context.s, context.typeExpr.release(), context.fromIdentifier));
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s.SetEnd(span.End());
        context.typeExpr.reset(new ArrayNode(context.s, context.typeExpr.release(), context.fromsize));
    }
    void PrePrimaryTypeExpr(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostPrimaryTypeExpr(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromPrimaryTypeExpr_value = std::move(stack.top());
            context.fromPrimaryTypeExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromPrimaryTypeExpr_value.get());
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
    void Presize(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postsize(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromsize_value = std::move(stack.top());
            context.fromsize = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromsize_value.get());
            stack.pop();
        }
    }
private:
    struct Context
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
    std::stack<Context> contextStack;
    Context context;
};

class TypeExprGrammar::PrimaryTypeExprRule : public cminor::parsing::Rule
{
public:
    PrimaryTypeExprRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryTypeExprRule>(this, &PrimaryTypeExprRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryTypeExprRule>(this, &PrimaryTypeExprRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimaryTypeExprRule>(this, &PrimaryTypeExprRule::A2Action));
        cminor::parsing::NonterminalParser* basicTypeNonterminalParser = GetNonterminal("BasicType");
        basicTypeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryTypeExprRule>(this, &PrimaryTypeExprRule::PostBasicType));
        cminor::parsing::NonterminalParser* templateIdNonterminalParser = GetNonterminal("TemplateId");
        templateIdNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PrimaryTypeExprRule>(this, &PrimaryTypeExprRule::PreTemplateId));
        templateIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryTypeExprRule>(this, &PrimaryTypeExprRule::PostTemplateId));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimaryTypeExprRule>(this, &PrimaryTypeExprRule::PostIdentifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromBasicType;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromTemplateId;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromIdentifier;
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
private:
    struct Context
    {
        Context(): ctx(), value(), fromBasicType(), fromTemplateId(), fromIdentifier() {}
        ParsingContext* ctx;
        Node* value;
        Node* fromBasicType;
        Node* fromTemplateId;
        IdentifierNode* fromIdentifier;
    };
    std::stack<Context> contextStack;
    Context context;
};

void TypeExprGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("cminor.parser.IdentifierGrammar");
    if (!grammar0)
    {
        grammar0 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar("cminor.parser.TemplateGrammar");
    if (!grammar1)
    {
        grammar1 = cminor::parser::TemplateGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar("cminor.parser.BasicTypeGrammar");
    if (!grammar2)
    {
        grammar2 = cminor::parser::BasicTypeGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar("cminor.parser.ExpressionGrammar");
    if (!grammar3)
    {
        grammar3 = cminor::parser::ExpressionGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
}

void TypeExprGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("TemplateId", this, "TemplateGrammar.TemplateId"));
    AddRuleLink(new cminor::parsing::RuleLink("Identifier", this, "IdentifierGrammar.Identifier"));
    AddRuleLink(new cminor::parsing::RuleLink("BasicType", this, "BasicTypeGrammar.BasicType"));
    AddRuleLink(new cminor::parsing::RuleLink("Expression", this, "ExpressionGrammar.Expression"));
    AddRule(new TypeExprRule("TypeExpr", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::NonterminalParser("PrefixTypeExpr", "PrefixTypeExpr", 1))));
    AddRule(new PrefixTypeExprRule("PrefixTypeExpr", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser("ref"),
                    new cminor::parsing::NonterminalParser("refExpr", "PostfixTypeExpr", 1))),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::NonterminalParser("pf", "PostfixTypeExpr", 1)))));
    AddRule(new PostfixTypeExprRule("PostfixTypeExpr", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("PrimaryTypeExpr", "PrimaryTypeExpr", 1)),
                new cminor::parsing::KleeneStarParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::CharParser('.'),
                            new cminor::parsing::ActionParser("A2",
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0)))),
                        new cminor::parsing::ActionParser("A3",
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::CharParser('['),
                                    new cminor::parsing::OptionalParser(
                                        new cminor::parsing::NonterminalParser("size", "Expression", 1))),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::CharParser(']'))))))))));
    AddRule(new PrimaryTypeExprRule("PrimaryTypeExpr", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::NonterminalParser("BasicType", "BasicType", 0)),
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("TemplateId", "TemplateId", 1))),
            new cminor::parsing::ActionParser("A2",
                new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0)))));
}

} } // namespace cminor.parser
