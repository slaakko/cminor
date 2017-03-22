#include "Template.hpp"
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>
#include <cminor/parser/Identifier.hpp>
#include <cminor/parser/TypeExpr.hpp>

namespace cminor { namespace parser {

using namespace cminor::parsing;

TemplateGrammar* TemplateGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

TemplateGrammar* TemplateGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    TemplateGrammar* grammar(new TemplateGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

TemplateGrammar::TemplateGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("TemplateGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

Node* TemplateGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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

class TemplateGrammar::TemplateIdRule : public cminor::parsing::Rule
{
public:
    TemplateIdRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<TemplateIdNode>", "templateId"));
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TemplateIdRule>(this, &TemplateIdRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TemplateIdRule>(this, &TemplateIdRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TemplateIdRule>(this, &TemplateIdRule::A2Action));
        cminor::parsing::NonterminalParser* primaryNonterminalParser = GetNonterminal("primary");
        primaryNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TemplateIdRule>(this, &TemplateIdRule::Postprimary));
        cminor::parsing::NonterminalParser* templateArgNonterminalParser = GetNonterminal("templateArg");
        templateArgNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<TemplateIdRule>(this, &TemplateIdRule::PretemplateArg));
        templateArgNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TemplateIdRule>(this, &TemplateIdRule::PosttemplateArg));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->templateId.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->templateId.reset(new TemplateIdNode(span, context->fromprimary));
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->templateId->AddTemplateArgument(context->fromtemplateArg);
    }
    void Postprimary(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromprimary_value = std::move(stack.top());
            context->fromprimary = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromprimary_value.get());
            stack.pop();
        }
    }
    void PretemplateArg(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PosttemplateArg(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromtemplateArg_value = std::move(stack.top());
            context->fromtemplateArg = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromtemplateArg_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), templateId(), fromprimary(), fromtemplateArg() {}
        ParsingContext* ctx;
        Node* value;
        std::unique_ptr<TemplateIdNode> templateId;
        IdentifierNode* fromprimary;
        Node* fromtemplateArg;
    };
};

class TemplateGrammar::TemplateParameterRule : public cminor::parsing::Rule
{
public:
    TemplateParameterRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("TemplateParameterNode*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<TemplateParameterNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TemplateParameterRule>(this, &TemplateParameterRule::A0Action));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TemplateParameterRule>(this, &TemplateParameterRule::PostIdentifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new TemplateParameterNode(span, context->fromIdentifier);
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
        Context(): ctx(), value(), fromIdentifier() {}
        ParsingContext* ctx;
        TemplateParameterNode* value;
        IdentifierNode* fromIdentifier;
    };
};

class TemplateGrammar::TemplateParameterListRule : public cminor::parsing::Rule
{
public:
    TemplateParameterListRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("Node*", "owner"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> owner_value = std::move(stack.top());
        context->owner = *static_cast<cminor::parsing::ValueObject<Node*>*>(owner_value.get());
        stack.pop();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context->ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TemplateParameterListRule>(this, &TemplateParameterListRule::A0Action));
        cminor::parsing::NonterminalParser* templateParameterNonterminalParser = GetNonterminal("TemplateParameter");
        templateParameterNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<TemplateParameterListRule>(this, &TemplateParameterListRule::PreTemplateParameter));
        templateParameterNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TemplateParameterListRule>(this, &TemplateParameterListRule::PostTemplateParameter));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->owner->AddTemplateParameter(context->fromTemplateParameter);
    }
    void PreTemplateParameter(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostTemplateParameter(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTemplateParameter_value = std::move(stack.top());
            context->fromTemplateParameter = *static_cast<cminor::parsing::ValueObject<TemplateParameterNode*>*>(fromTemplateParameter_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), owner(), fromTemplateParameter() {}
        ParsingContext* ctx;
        Node* owner;
        TemplateParameterNode* fromTemplateParameter;
    };
};

void TemplateGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("cminor.parser.TypeExprGrammar");
    if (!grammar0)
    {
        grammar0 = cminor::parser::TypeExprGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar("cminor.parser.IdentifierGrammar");
    if (!grammar1)
    {
        grammar1 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
}

void TemplateGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("TypeExpr", this, "TypeExprGrammar.TypeExpr"));
    AddRuleLink(new cminor::parsing::RuleLink("QualifiedId", this, "IdentifierGrammar.QualifiedId"));
    AddRuleLink(new cminor::parsing::RuleLink("Identifier", this, "IdentifierGrammar.Identifier"));
    AddRule(new TemplateIdRule("TemplateId", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser("A1",
                            new cminor::parsing::NonterminalParser("primary", "QualifiedId", 0)),
                        new cminor::parsing::CharParser('<')),
                    new cminor::parsing::ListParser(
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::NonterminalParser("templateArg", "TypeExpr", 1)),
                        new cminor::parsing::CharParser(','))),
                new cminor::parsing::CharParser('>')))));
    AddRule(new TemplateParameterRule("TemplateParameter", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0))));
    AddRule(new TemplateParameterListRule("TemplateParameterList", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('<'),
                new cminor::parsing::ListParser(
                    new cminor::parsing::ActionParser("A0",
                        new cminor::parsing::NonterminalParser("TemplateParameter", "TemplateParameter", 1)),
                    new cminor::parsing::CharParser(','))),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::CharParser('>')))));
}

} } // namespace cminor.parser
