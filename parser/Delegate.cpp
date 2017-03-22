#include "Delegate.hpp"
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>
#include <cminor/parser/Specifier.hpp>
#include <cminor/parser/TypeExpr.hpp>
#include <cminor/parser/Identifier.hpp>
#include <cminor/parser/Parameter.hpp>

namespace cminor { namespace parser {

using namespace cminor::parsing;

DelegateGrammar* DelegateGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

DelegateGrammar* DelegateGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    DelegateGrammar* grammar(new DelegateGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

DelegateGrammar::DelegateGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("DelegateGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

Node* DelegateGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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

class DelegateGrammar::DelegateRule : public cminor::parsing::Rule
{
public:
    DelegateRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
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
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DelegateRule>(this, &DelegateRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DelegateRule>(this, &DelegateRule::A1Action));
        cminor::parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal("Specifiers");
        specifiersNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DelegateRule>(this, &DelegateRule::PostSpecifiers));
        cminor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DelegateRule>(this, &DelegateRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DelegateRule>(this, &DelegateRule::PostTypeExpr));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DelegateRule>(this, &DelegateRule::PostIdentifier));
        cminor::parsing::NonterminalParser* parameterListNonterminalParser = GetNonterminal("ParameterList");
        parameterListNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DelegateRule>(this, &DelegateRule::PreParameterList));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new DelegateNode(span, context->fromSpecifiers, context->fromTypeExpr, context->fromIdentifier);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value->GetSpan().SetEnd(span.End());
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
    void PreParameterList(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->value)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), fromSpecifiers(), fromTypeExpr(), fromIdentifier() {}
        ParsingContext* ctx;
        Node* value;
        Specifiers fromSpecifiers;
        Node* fromTypeExpr;
        IdentifierNode* fromIdentifier;
    };
};

class DelegateGrammar::ClassDelegateRule : public cminor::parsing::Rule
{
public:
    ClassDelegateRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
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
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassDelegateRule>(this, &ClassDelegateRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassDelegateRule>(this, &ClassDelegateRule::A1Action));
        cminor::parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal("Specifiers");
        specifiersNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassDelegateRule>(this, &ClassDelegateRule::PostSpecifiers));
        cminor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassDelegateRule>(this, &ClassDelegateRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassDelegateRule>(this, &ClassDelegateRule::PostTypeExpr));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassDelegateRule>(this, &ClassDelegateRule::PostIdentifier));
        cminor::parsing::NonterminalParser* parameterListNonterminalParser = GetNonterminal("ParameterList");
        parameterListNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassDelegateRule>(this, &ClassDelegateRule::PreParameterList));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ClassDelegateNode(span, context->fromSpecifiers, context->fromTypeExpr, context->fromIdentifier);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value->GetSpan().SetEnd(span.End());
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
    void PreParameterList(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->value)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), fromSpecifiers(), fromTypeExpr(), fromIdentifier() {}
        ParsingContext* ctx;
        Node* value;
        Specifiers fromSpecifiers;
        Node* fromTypeExpr;
        IdentifierNode* fromIdentifier;
    };
};

void DelegateGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("cminor.parser.ParameterGrammar");
    if (!grammar0)
    {
        grammar0 = cminor::parser::ParameterGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar("cminor.parser.TypeExprGrammar");
    if (!grammar1)
    {
        grammar1 = cminor::parser::TypeExprGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar("cminor.parser.IdentifierGrammar");
    if (!grammar2)
    {
        grammar2 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar("cminor.parser.SpecifierGrammar");
    if (!grammar3)
    {
        grammar3 = cminor::parser::SpecifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
}

void DelegateGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("ParameterList", this, "ParameterGrammar.ParameterList"));
    AddRuleLink(new cminor::parsing::RuleLink("TypeExpr", this, "TypeExprGrammar.TypeExpr"));
    AddRuleLink(new cminor::parsing::RuleLink("Specifiers", this, "SpecifierGrammar.Specifiers"));
    AddRuleLink(new cminor::parsing::RuleLink("Identifier", this, "IdentifierGrammar.Identifier"));
    AddRule(new DelegateRule("Delegate", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::NonterminalParser("Specifiers", "Specifiers", 0),
                                new cminor::parsing::KeywordParser("delegate")),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("TypeExpr", "TypeExpr", 1))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0)))),
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("ParameterList", "ParameterList", 2)))),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::CharParser(';')))));
    AddRule(new ClassDelegateRule("ClassDelegate", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::NonterminalParser("Specifiers", "Specifiers", 0),
                                    new cminor::parsing::KeywordParser("class")),
                                new cminor::parsing::KeywordParser("delegate")),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("TypeExpr", "TypeExpr", 1))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0)))),
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("ParameterList", "ParameterList", 2)))),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::CharParser(';')))));
}

} } // namespace cminor.parser
