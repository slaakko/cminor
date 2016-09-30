#include "TypeExpr.hpp"
#include <Cm.Parsing/Action.hpp>
#include <Cm.Parsing/Rule.hpp>
#include <Cm.Parsing/ParsingDomain.hpp>
#include <Cm.Parsing/Primitive.hpp>
#include <Cm.Parsing/Composite.hpp>
#include <Cm.Parsing/Nonterminal.hpp>
#include <Cm.Parsing/Exception.hpp>
#include <Cm.Parsing/StdLib.hpp>
#include <Cm.Parsing/XmlLog.hpp>
#include <cminor/parser/BasicType.hpp>
#include <cminor/parser/Identifier.hpp>

namespace cminor { namespace parser {

using namespace Cm::Parsing;

TypeExprGrammar* TypeExprGrammar::Create()
{
    return Create(new Cm::Parsing::ParsingDomain());
}

TypeExprGrammar* TypeExprGrammar::Create(Cm::Parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    TypeExprGrammar* grammar(new TypeExprGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

TypeExprGrammar::TypeExprGrammar(Cm::Parsing::ParsingDomain* parsingDomain_): Cm::Parsing::Grammar("TypeExprGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

Node* TypeExprGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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

class TypeExprGrammar::TypeExprRule : public Cm::Parsing::Rule
{
public:
    TypeExprRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<TypeExprRule>(this, &TypeExprRule::A0Action));
        Cm::Parsing::NonterminalParser* postfixTypeExprNonterminalParser = GetNonterminal("PostfixTypeExpr");
        postfixTypeExprNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<TypeExprRule>(this, &TypeExprRule::PrePostfixTypeExpr));
        postfixTypeExprNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<TypeExprRule>(this, &TypeExprRule::PostPostfixTypeExpr));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromPostfixTypeExpr;
    }
    void PrePostfixTypeExpr(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostPostfixTypeExpr(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromPostfixTypeExpr_value = std::move(stack.top());
            context.fromPostfixTypeExpr = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromPostfixTypeExpr_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromPostfixTypeExpr() {}
        ParsingContext* ctx;
        Node* value;
        Node* fromPostfixTypeExpr;
    };
    std::stack<Context> contextStack;
    Context context;
};

class TypeExprGrammar::PostfixTypeExprRule : public Cm::Parsing::Rule
{
public:
    PostfixTypeExprRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "typeExpr"));
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<PostfixTypeExprRule>(this, &PostfixTypeExprRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<PostfixTypeExprRule>(this, &PostfixTypeExprRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<PostfixTypeExprRule>(this, &PostfixTypeExprRule::A2Action));
        Cm::Parsing::NonterminalParser* primaryTypeExprNonterminalParser = GetNonterminal("PrimaryTypeExpr");
        primaryTypeExprNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<PostfixTypeExprRule>(this, &PostfixTypeExprRule::PrePrimaryTypeExpr));
        primaryTypeExprNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<PostfixTypeExprRule>(this, &PostfixTypeExprRule::PostPrimaryTypeExpr));
        Cm::Parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<PostfixTypeExprRule>(this, &PostfixTypeExprRule::PostIdentifier));
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
    void PrePrimaryTypeExpr(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostPrimaryTypeExpr(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromPrimaryTypeExpr_value = std::move(stack.top());
            context.fromPrimaryTypeExpr = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromPrimaryTypeExpr_value.get());
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
        Context(): ctx(), value(), typeExpr(), s(), fromPrimaryTypeExpr(), fromIdentifier() {}
        ParsingContext* ctx;
        Node* value;
        std::unique_ptr<Node> typeExpr;
        Span s;
        Node* fromPrimaryTypeExpr;
        IdentifierNode* fromIdentifier;
    };
    std::stack<Context> contextStack;
    Context context;
};

class TypeExprGrammar::PrimaryTypeExprRule : public Cm::Parsing::Rule
{
public:
    PrimaryTypeExprRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<PrimaryTypeExprRule>(this, &PrimaryTypeExprRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<PrimaryTypeExprRule>(this, &PrimaryTypeExprRule::A1Action));
        Cm::Parsing::NonterminalParser* basicTypeNonterminalParser = GetNonterminal("BasicType");
        basicTypeNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<PrimaryTypeExprRule>(this, &PrimaryTypeExprRule::PostBasicType));
        Cm::Parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<PrimaryTypeExprRule>(this, &PrimaryTypeExprRule::PostIdentifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromBasicType;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromIdentifier;
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
        Context(): ctx(), value(), fromBasicType(), fromIdentifier() {}
        ParsingContext* ctx;
        Node* value;
        Node* fromBasicType;
        IdentifierNode* fromIdentifier;
    };
    std::stack<Context> contextStack;
    Context context;
};

void TypeExprGrammar::GetReferencedGrammars()
{
    Cm::Parsing::ParsingDomain* pd = GetParsingDomain();
    Cm::Parsing::Grammar* grammar0 = pd->GetGrammar("cminor.parser.IdentifierGrammar");
    if (!grammar0)
    {
        grammar0 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    Cm::Parsing::Grammar* grammar1 = pd->GetGrammar("cminor.parser.BasicTypeGrammar");
    if (!grammar1)
    {
        grammar1 = cminor::parser::BasicTypeGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
}

void TypeExprGrammar::CreateRules()
{
    AddRuleLink(new Cm::Parsing::RuleLink("Identifier", this, "IdentifierGrammar.Identifier"));
    AddRuleLink(new Cm::Parsing::RuleLink("BasicType", this, "BasicTypeGrammar.BasicType"));
    AddRule(new TypeExprRule("TypeExpr", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::NonterminalParser("PostfixTypeExpr", "PostfixTypeExpr", 1))));
    AddRule(new PostfixTypeExprRule("PostfixTypeExpr", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::ActionParser("A1",
                    new Cm::Parsing::NonterminalParser("PrimaryTypeExpr", "PrimaryTypeExpr", 1)),
                new Cm::Parsing::KleeneStarParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::CharParser('.'),
                        new Cm::Parsing::ActionParser("A2",
                            new Cm::Parsing::ExpectationParser(
                                new Cm::Parsing::NonterminalParser("Identifier", "Identifier", 0)))))))));
    AddRule(new PrimaryTypeExprRule("PrimaryTypeExpr", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::ActionParser("A0",
                new Cm::Parsing::NonterminalParser("BasicType", "BasicType", 0)),
            new Cm::Parsing::ActionParser("A1",
                new Cm::Parsing::NonterminalParser("Identifier", "Identifier", 0)))));
}

} } // namespace cminor.parser
