#include "Parameter.hpp"
#include <Cm.Parsing/Action.hpp>
#include <Cm.Parsing/Rule.hpp>
#include <Cm.Parsing/ParsingDomain.hpp>
#include <Cm.Parsing/Primitive.hpp>
#include <Cm.Parsing/Composite.hpp>
#include <Cm.Parsing/Nonterminal.hpp>
#include <Cm.Parsing/Exception.hpp>
#include <Cm.Parsing/StdLib.hpp>
#include <Cm.Parsing/XmlLog.hpp>
#include <cminor/parser/TypeExpr.hpp>
#include <cminor/parser/Identifier.hpp>

namespace cminor { namespace parser {

using namespace Cm::Parsing;

ParameterGrammar* ParameterGrammar::Create()
{
    return Create(new Cm::Parsing::ParsingDomain());
}

ParameterGrammar* ParameterGrammar::Create(Cm::Parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    ParameterGrammar* grammar(new ParameterGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

ParameterGrammar::ParameterGrammar(Cm::Parsing::ParsingDomain* parsingDomain_): Cm::Parsing::Grammar("ParameterGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

void ParameterGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx, Node* owner)
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
    stack.push(std::unique_ptr<Cm::Parsing::Object>(new ValueObject<Node*>(owner)));
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
}

class ParameterGrammar::ParameterListRule : public Cm::Parsing::Rule
{
public:
    ParameterListRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("Node*", "owner"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> owner_value = std::move(stack.top());
        context.owner = *static_cast<Cm::Parsing::ValueObject<Node*>*>(owner_value.get());
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ParameterListRule>(this, &ParameterListRule::A0Action));
        Cm::Parsing::NonterminalParser* parameterNonterminalParser = GetNonterminal("Parameter");
        parameterNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ParameterListRule>(this, &ParameterListRule::PreParameter));
        parameterNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ParameterListRule>(this, &ParameterListRule::PostParameter));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.owner->AddParameter(context.fromParameter);
    }
    void PreParameter(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostParameter(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromParameter_value = std::move(stack.top());
            context.fromParameter = *static_cast<Cm::Parsing::ValueObject<ParameterNode*>*>(fromParameter_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), owner(), fromParameter() {}
        ParsingContext* ctx;
        Node* owner;
        ParameterNode* fromParameter;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ParameterGrammar::ParameterRule : public Cm::Parsing::Rule
{
public:
    ParameterRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("ParameterNode*");
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParameterNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ParameterRule>(this, &ParameterRule::A0Action));
        Cm::Parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ParameterRule>(this, &ParameterRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ParameterRule>(this, &ParameterRule::PostTypeExpr));
        Cm::Parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ParameterRule>(this, &ParameterRule::PostIdentifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ParameterNode(span, context.fromTypeExpr, context.fromIdentifier);
    }
    void PreTypeExpr(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostTypeExpr(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context.fromTypeExpr = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
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
        Context(): ctx(), value(), fromTypeExpr(), fromIdentifier() {}
        ParsingContext* ctx;
        ParameterNode* value;
        Node* fromTypeExpr;
        IdentifierNode* fromIdentifier;
    };
    std::stack<Context> contextStack;
    Context context;
};

void ParameterGrammar::GetReferencedGrammars()
{
    Cm::Parsing::ParsingDomain* pd = GetParsingDomain();
    Cm::Parsing::Grammar* grammar0 = pd->GetGrammar("cminor.parser.TypeExprGrammar");
    if (!grammar0)
    {
        grammar0 = cminor::parser::TypeExprGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    Cm::Parsing::Grammar* grammar1 = pd->GetGrammar("cminor.parser.IdentifierGrammar");
    if (!grammar1)
    {
        grammar1 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
}

void ParameterGrammar::CreateRules()
{
    AddRuleLink(new Cm::Parsing::RuleLink("TypeExpr", this, "TypeExprGrammar.TypeExpr"));
    AddRuleLink(new Cm::Parsing::RuleLink("Identifier", this, "IdentifierGrammar.Identifier"));
    AddRule(new ParameterListRule("ParameterList", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::CharParser('('),
                new Cm::Parsing::OptionalParser(
                    new Cm::Parsing::ListParser(
                        new Cm::Parsing::ActionParser("A0",
                            new Cm::Parsing::NonterminalParser("Parameter", "Parameter", 1)),
                        new Cm::Parsing::CharParser(',')))),
            new Cm::Parsing::ExpectationParser(
                new Cm::Parsing::CharParser(')')))));
    AddRule(new ParameterRule("Parameter", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::NonterminalParser("TypeExpr", "TypeExpr", 1),
                new Cm::Parsing::NonterminalParser("Identifier", "Identifier", 0)))));
}

} } // namespace cminor.parser
