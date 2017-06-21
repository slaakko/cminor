#include "Template.hpp"
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
#include <cminor/parser/Identifier.hpp>
#include <cminor/parser/TypeExpr.hpp>

namespace cminor { namespace parser {

using namespace cminor::parsing;
using namespace cminor::util;
using namespace cminor::unicode;

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

TemplateGrammar::TemplateGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("TemplateGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.parser")), parsingDomain_)
{
    SetOwner(0);
}

Node* TemplateGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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

class TemplateGrammar::TemplateIdRule : public cminor::parsing::Rule
{
public:
    TemplateIdRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("Node*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<TemplateIdNode>"), ToUtf32("templateId")));
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TemplateIdRule>(this, &TemplateIdRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TemplateIdRule>(this, &TemplateIdRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TemplateIdRule>(this, &TemplateIdRule::A2Action));
        cminor::parsing::NonterminalParser* primaryNonterminalParser = GetNonterminal(ToUtf32("primary"));
        primaryNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TemplateIdRule>(this, &TemplateIdRule::Postprimary));
        cminor::parsing::NonterminalParser* templateArgNonterminalParser = GetNonterminal(ToUtf32("templateArg"));
        templateArgNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<TemplateIdRule>(this, &TemplateIdRule::PretemplateArg));
        templateArgNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TemplateIdRule>(this, &TemplateIdRule::PosttemplateArg));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->templateId.release();
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->templateId.reset(new TemplateIdNode(span, context->fromprimary));
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
    TemplateParameterRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("TemplateParameterNode*"));
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
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TemplateParameterRule>(this, &TemplateParameterRule::A0Action));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("Identifier"));
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TemplateParameterRule>(this, &TemplateParameterRule::PostIdentifier));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
    TemplateParameterListRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        AddInheritedAttribute(AttrOrVariable(ToUtf32("Node*"), ToUtf32("owner")));
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
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TemplateParameterListRule>(this, &TemplateParameterListRule::A0Action));
        cminor::parsing::NonterminalParser* templateParameterNonterminalParser = GetNonterminal(ToUtf32("TemplateParameter"));
        templateParameterNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<TemplateParameterListRule>(this, &TemplateParameterListRule::PreTemplateParameter));
        templateParameterNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TemplateParameterListRule>(this, &TemplateParameterListRule::PostTemplateParameter));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cminor.parser.IdentifierGrammar"));
    if (!grammar0)
    {
        grammar0 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar(ToUtf32("cminor.parser.TypeExprGrammar"));
    if (!grammar1)
    {
        grammar1 = cminor::parser::TypeExprGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
}

void TemplateGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("QualifiedId"), this, ToUtf32("IdentifierGrammar.QualifiedId")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Identifier"), this, ToUtf32("IdentifierGrammar.Identifier")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("TypeExpr"), this, ToUtf32("TypeExprGrammar.TypeExpr")));
    AddRule(new TemplateIdRule(ToUtf32("TemplateId"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser(ToUtf32("A1"),
                            new cminor::parsing::NonterminalParser(ToUtf32("primary"), ToUtf32("QualifiedId"), 0)),
                        new cminor::parsing::CharParser('<')),
                    new cminor::parsing::ListParser(
                        new cminor::parsing::ActionParser(ToUtf32("A2"),
                            new cminor::parsing::NonterminalParser(ToUtf32("templateArg"), ToUtf32("TypeExpr"), 1)),
                        new cminor::parsing::CharParser(','))),
                new cminor::parsing::CharParser('>')))));
    AddRule(new TemplateParameterRule(ToUtf32("TemplateParameter"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::NonterminalParser(ToUtf32("Identifier"), ToUtf32("Identifier"), 0))));
    AddRule(new TemplateParameterListRule(ToUtf32("TemplateParameterList"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('<'),
                new cminor::parsing::ListParser(
                    new cminor::parsing::ActionParser(ToUtf32("A0"),
                        new cminor::parsing::NonterminalParser(ToUtf32("TemplateParameter"), ToUtf32("TemplateParameter"), 1)),
                    new cminor::parsing::CharParser(','))),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::CharParser('>')))));
}

} } // namespace cminor.parser
