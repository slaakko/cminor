#include "Element.hpp"
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
#include <cminor/parsing/Rule.hpp>
#include <cminor/codedom/Type.hpp>
#include <cminor/code/Declarator.hpp>

namespace cminor { namespace syntax {

using namespace cminor::parsing;
using namespace cminor::util;
using namespace cminor::unicode;

ElementGrammar* ElementGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

ElementGrammar* ElementGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    ElementGrammar* grammar(new ElementGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

ElementGrammar::ElementGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("ElementGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.syntax")), parsingDomain_)
{
    SetOwner(0);
    keywords0.push_back(ToUtf32("alphabetic"));
    keywords0.push_back(ToUtf32("anychar"));
    keywords0.push_back(ToUtf32("basechar"));
    keywords0.push_back(ToUtf32("cased_letter"));
    keywords0.push_back(ToUtf32("close_punctuation"));
    keywords0.push_back(ToUtf32("connector_punctuation"));
    keywords0.push_back(ToUtf32("control"));
    keywords0.push_back(ToUtf32("currency_symbol"));
    keywords0.push_back(ToUtf32("dash_punctuation"));
    keywords0.push_back(ToUtf32("decimal_number"));
    keywords0.push_back(ToUtf32("digit"));
    keywords0.push_back(ToUtf32("empty"));
    keywords0.push_back(ToUtf32("enclosing_mark"));
    keywords0.push_back(ToUtf32("end"));
    keywords0.push_back(ToUtf32("final_punctuation"));
    keywords0.push_back(ToUtf32("format"));
    keywords0.push_back(ToUtf32("grammar"));
    keywords0.push_back(ToUtf32("graphic"));
    keywords0.push_back(ToUtf32("hexdigit"));
    keywords0.push_back(ToUtf32("idcont"));
    keywords0.push_back(ToUtf32("idstart"));
    keywords0.push_back(ToUtf32("initial_punctuation"));
    keywords0.push_back(ToUtf32("keyword"));
    keywords0.push_back(ToUtf32("keyword_list"));
    keywords0.push_back(ToUtf32("letter"));
    keywords0.push_back(ToUtf32("letter_number"));
    keywords0.push_back(ToUtf32("line_separator"));
    keywords0.push_back(ToUtf32("lower_letter"));
    keywords0.push_back(ToUtf32("mark"));
    keywords0.push_back(ToUtf32("math_symbol"));
    keywords0.push_back(ToUtf32("modifier_letter"));
    keywords0.push_back(ToUtf32("modifier_symbol"));
    keywords0.push_back(ToUtf32("nonspacing_mark"));
    keywords0.push_back(ToUtf32("number"));
    keywords0.push_back(ToUtf32("open_punctuation"));
    keywords0.push_back(ToUtf32("other"));
    keywords0.push_back(ToUtf32("other_letter"));
    keywords0.push_back(ToUtf32("other_number"));
    keywords0.push_back(ToUtf32("other_punctuation"));
    keywords0.push_back(ToUtf32("other_symbol"));
    keywords0.push_back(ToUtf32("paragraph_separator"));
    keywords0.push_back(ToUtf32("private_use"));
    keywords0.push_back(ToUtf32("punctuation"));
    keywords0.push_back(ToUtf32("range"));
    keywords0.push_back(ToUtf32("separator"));
    keywords0.push_back(ToUtf32("skip"));
    keywords0.push_back(ToUtf32("space"));
    keywords0.push_back(ToUtf32("space_separator"));
    keywords0.push_back(ToUtf32("spacing_mark"));
    keywords0.push_back(ToUtf32("start"));
    keywords0.push_back(ToUtf32("surrogate"));
    keywords0.push_back(ToUtf32("symbol"));
    keywords0.push_back(ToUtf32("title_letter"));
    keywords0.push_back(ToUtf32("token"));
    keywords0.push_back(ToUtf32("unassigned"));
    keywords0.push_back(ToUtf32("upper_letter"));
    keywords0.push_back(ToUtf32("using"));
    keywords0.push_back(ToUtf32("var"));
}

void ElementGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, cminor::parsing::Grammar* grammar)
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
    stack.push(std::unique_ptr<cminor::parsing::Object>(new ValueObject<cminor::parsing::Grammar*>(grammar)));
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
}

class ElementGrammar::RuleLinkRule : public cminor::parsing::Rule
{
public:
    RuleLinkRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::parsing::Grammar*"), ToUtf32("grammar")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> grammar_value = std::move(stack.top());
        context->grammar = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Grammar*>*>(grammar_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RuleLinkRule>(this, &RuleLinkRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RuleLinkRule>(this, &RuleLinkRule::A1Action));
        cminor::parsing::NonterminalParser* aliasNameNonterminalParser = GetNonterminal(ToUtf32("aliasName"));
        aliasNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RuleLinkRule>(this, &RuleLinkRule::PostaliasName));
        cminor::parsing::NonterminalParser* ruleNameNonterminalParser = GetNonterminal(ToUtf32("ruleName"));
        ruleNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RuleLinkRule>(this, &RuleLinkRule::PostruleName));
        cminor::parsing::NonterminalParser* qualified_idNonterminalParser = GetNonterminal(ToUtf32("qualified_id"));
        qualified_idNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RuleLinkRule>(this, &RuleLinkRule::Postqualified_id));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        RuleLink * link(new RuleLink(context->fromaliasName, context->grammar, context->fromruleName));
        link->SetSpan(span);
        context->grammar->AddRuleLink(link);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        RuleLink * link(new RuleLink(context->grammar, context->fromqualified_id));
        link->SetSpan(span);
        context->grammar->AddRuleLink(link);
    }
    void PostaliasName(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromaliasName_value = std::move(stack.top());
            context->fromaliasName = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromaliasName_value.get());
            stack.pop();
        }
    }
    void PostruleName(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromruleName_value = std::move(stack.top());
            context->fromruleName = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromruleName_value.get());
            stack.pop();
        }
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
        Context(): grammar(), fromaliasName(), fromruleName(), fromqualified_id() {}
        cminor::parsing::Grammar* grammar;
        std::u32string fromaliasName;
        std::u32string fromruleName;
        std::u32string fromqualified_id;
    };
};

class ElementGrammar::SignatureRule : public cminor::parsing::Rule
{
public:
    SignatureRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::parsing::Rule*"), ToUtf32("rule")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> rule_value = std::move(stack.top());
        context->rule = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Rule*>*>(rule_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::NonterminalParser* parameterListNonterminalParser = GetNonterminal(ToUtf32("ParameterList"));
        parameterListNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<SignatureRule>(this, &SignatureRule::PreParameterList));
        cminor::parsing::NonterminalParser* returnTypeNonterminalParser = GetNonterminal(ToUtf32("ReturnType"));
        returnTypeNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<SignatureRule>(this, &SignatureRule::PreReturnType));
    }
    void PreParameterList(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Rule*>(context->rule)));
    }
    void PreReturnType(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Rule*>(context->rule)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): rule() {}
        cminor::parsing::Rule* rule;
    };
};

class ElementGrammar::ParameterListRule : public cminor::parsing::Rule
{
public:
    ParameterListRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::parsing::Rule*"), ToUtf32("rule")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> rule_value = std::move(stack.top());
        context->rule = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Rule*>*>(rule_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::NonterminalParser* variableNonterminalParser = GetNonterminal(ToUtf32("Variable"));
        variableNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ParameterListRule>(this, &ParameterListRule::PreVariable));
        cminor::parsing::NonterminalParser* parameterNonterminalParser = GetNonterminal(ToUtf32("Parameter"));
        parameterNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ParameterListRule>(this, &ParameterListRule::PreParameter));
    }
    void PreVariable(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Rule*>(context->rule)));
    }
    void PreParameter(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Rule*>(context->rule)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): rule() {}
        cminor::parsing::Rule* rule;
    };
};

class ElementGrammar::VariableRule : public cminor::parsing::Rule
{
public:
    VariableRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::parsing::Rule*"), ToUtf32("rule")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> rule_value = std::move(stack.top());
        context->rule = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Rule*>*>(rule_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<VariableRule>(this, &VariableRule::A0Action));
        cminor::parsing::NonterminalParser* typeIdNonterminalParser = GetNonterminal(ToUtf32("TypeId"));
        typeIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<VariableRule>(this, &VariableRule::PostTypeId));
        cminor::parsing::NonterminalParser* declaratorNonterminalParser = GetNonterminal(ToUtf32("Declarator"));
        declaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<VariableRule>(this, &VariableRule::PostDeclarator));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->rule->AddLocalVariable(AttrOrVariable(context->fromTypeId->ToString(), context->fromDeclarator));
        delete context->fromTypeId;
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
    void PostDeclarator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDeclarator_value = std::move(stack.top());
            context->fromDeclarator = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromDeclarator_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): rule(), fromTypeId(), fromDeclarator() {}
        cminor::parsing::Rule* rule;
        cminor::codedom::TypeId* fromTypeId;
        std::u32string fromDeclarator;
    };
};

class ElementGrammar::ParameterRule : public cminor::parsing::Rule
{
public:
    ParameterRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::parsing::Rule*"), ToUtf32("rule")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> rule_value = std::move(stack.top());
        context->rule = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Rule*>*>(rule_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ParameterRule>(this, &ParameterRule::A0Action));
        cminor::parsing::NonterminalParser* typeIdNonterminalParser = GetNonterminal(ToUtf32("TypeId"));
        typeIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ParameterRule>(this, &ParameterRule::PostTypeId));
        cminor::parsing::NonterminalParser* declaratorNonterminalParser = GetNonterminal(ToUtf32("Declarator"));
        declaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ParameterRule>(this, &ParameterRule::PostDeclarator));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->rule->AddInheritedAttribute(AttrOrVariable(context->fromTypeId->ToString(), context->fromDeclarator));
        delete context->fromTypeId;
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
    void PostDeclarator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDeclarator_value = std::move(stack.top());
            context->fromDeclarator = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromDeclarator_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): rule(), fromTypeId(), fromDeclarator() {}
        cminor::parsing::Rule* rule;
        cminor::codedom::TypeId* fromTypeId;
        std::u32string fromDeclarator;
    };
};

class ElementGrammar::ReturnTypeRule : public cminor::parsing::Rule
{
public:
    ReturnTypeRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::parsing::Rule*"), ToUtf32("rule")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> rule_value = std::move(stack.top());
        context->rule = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Rule*>*>(rule_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ReturnTypeRule>(this, &ReturnTypeRule::A0Action));
        cminor::parsing::NonterminalParser* typeIdNonterminalParser = GetNonterminal(ToUtf32("TypeId"));
        typeIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ReturnTypeRule>(this, &ReturnTypeRule::PostTypeId));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->rule->SetValueTypeName(context->fromTypeId->ToString());
        delete context->fromTypeId;
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
private:
    struct Context : cminor::parsing::Context
    {
        Context(): rule(), fromTypeId() {}
        cminor::parsing::Rule* rule;
        cminor::codedom::TypeId* fromTypeId;
    };
};

class ElementGrammar::IdentifierRule : public cminor::parsing::Rule
{
public:
    IdentifierRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IdentifierRule>(this, &IdentifierRule::A0Action));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("identifier"));
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IdentifierRule>(this, &IdentifierRule::Postidentifier));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = std::u32string(matchBegin, matchEnd);
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
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromidentifier() {}
        std::u32string value;
        std::u32string fromidentifier;
    };
};

class ElementGrammar::QualifiedIdRule : public cminor::parsing::Rule
{
public:
    QualifiedIdRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<QualifiedIdRule>(this, &QualifiedIdRule::A0Action));
        cminor::parsing::NonterminalParser* firstNonterminalParser = GetNonterminal(ToUtf32("first"));
        firstNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<QualifiedIdRule>(this, &QualifiedIdRule::Postfirst));
        cminor::parsing::NonterminalParser* restNonterminalParser = GetNonterminal(ToUtf32("rest"));
        restNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<QualifiedIdRule>(this, &QualifiedIdRule::Postrest));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = std::u32string(matchBegin, matchEnd);
    }
    void Postfirst(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromfirst_value = std::move(stack.top());
            context->fromfirst = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromfirst_value.get());
            stack.pop();
        }
    }
    void Postrest(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromrest_value = std::move(stack.top());
            context->fromrest = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromrest_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromfirst(), fromrest() {}
        std::u32string value;
        std::u32string fromfirst;
        std::u32string fromrest;
    };
};

class ElementGrammar::StringArrayRule : public cminor::parsing::Rule
{
public:
    StringArrayRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("std::vector<std::u32string>*"), ToUtf32("array")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> array_value = std::move(stack.top());
        context->array = *static_cast<cminor::parsing::ValueObject<std::vector<std::u32string>*>*>(array_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StringArrayRule>(this, &StringArrayRule::A0Action));
        cminor::parsing::NonterminalParser* strNonterminalParser = GetNonterminal(ToUtf32("str"));
        strNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StringArrayRule>(this, &StringArrayRule::Poststr));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->array->push_back(context->fromstr);
    }
    void Poststr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromstr_value = std::move(stack.top());
            context->fromstr = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromstr_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): array(), fromstr() {}
        std::vector<std::u32string>* array;
        std::u32string fromstr;
    };
};

void ElementGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cminor.parsing.stdlib"));
    if (!grammar0)
    {
        grammar0 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar(ToUtf32("cminor.code.DeclaratorGrammar"));
    if (!grammar1)
    {
        grammar1 = cminor::code::DeclaratorGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
}

void ElementGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("qualified_id"), this, ToUtf32("cminor.parsing.stdlib.qualified_id")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("identifier"), this, ToUtf32("cminor.parsing.stdlib.identifier")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("string"), this, ToUtf32("cminor.parsing.stdlib.string")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("TypeId"), this, ToUtf32("cminor.code.DeclaratorGrammar.TypeId")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Declarator"), this, ToUtf32("cminor.code.DeclaratorGrammar.Declarator")));
    AddRule(new RuleLinkRule(ToUtf32("RuleLink"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::KeywordParser(ToUtf32("using")),
                                new cminor::parsing::NonterminalParser(ToUtf32("aliasName"), ToUtf32("identifier"), 0)),
                            new cminor::parsing::CharParser('=')),
                        new cminor::parsing::NonterminalParser(ToUtf32("ruleName"), ToUtf32("qualified_id"), 0)),
                    new cminor::parsing::CharParser(';'))),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser(ToUtf32("using")),
                        new cminor::parsing::NonterminalParser(ToUtf32("qualified_id"), ToUtf32("qualified_id"), 0)),
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new SignatureRule(ToUtf32("Signature"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::OptionalParser(
                new cminor::parsing::NonterminalParser(ToUtf32("ParameterList"), ToUtf32("ParameterList"), 1)),
            new cminor::parsing::OptionalParser(
                new cminor::parsing::NonterminalParser(ToUtf32("ReturnType"), ToUtf32("ReturnType"), 1)))));
    AddRule(new ParameterListRule(ToUtf32("ParameterList"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('('),
                new cminor::parsing::ListParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("Variable"), ToUtf32("Variable"), 1),
                        new cminor::parsing::NonterminalParser(ToUtf32("Parameter"), ToUtf32("Parameter"), 1)),
                    new cminor::parsing::CharParser(','))),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::CharParser(')')))));
    AddRule(new VariableRule(ToUtf32("Variable"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser(ToUtf32("var")),
                new cminor::parsing::NonterminalParser(ToUtf32("TypeId"), ToUtf32("TypeId"), 0)),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("Declarator"), ToUtf32("Declarator"), 0)))));
    AddRule(new ParameterRule(ToUtf32("Parameter"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::NonterminalParser(ToUtf32("TypeId"), ToUtf32("TypeId"), 0),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("Declarator"), ToUtf32("Declarator"), 0)))));
    AddRule(new ReturnTypeRule(ToUtf32("ReturnType"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::DifferenceParser(
                new cminor::parsing::CharParser(':'),
                new cminor::parsing::StringParser(ToUtf32("::"))),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("TypeId"), ToUtf32("TypeId"), 0)))));
    AddRule(new cminor::parsing::Rule(ToUtf32("Keyword"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::KeywordListParser(ToUtf32("identifier"), keywords0)));
    AddRule(new IdentifierRule(ToUtf32("Identifier"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::DifferenceParser(
                new cminor::parsing::NonterminalParser(ToUtf32("identifier"), ToUtf32("identifier"), 0),
                new cminor::parsing::NonterminalParser(ToUtf32("Keyword"), ToUtf32("Keyword"), 0)))));
    AddRule(new QualifiedIdRule(ToUtf32("QualifiedId"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("first"), ToUtf32("Identifier"), 0),
                    new cminor::parsing::KleeneStarParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::CharParser('.'),
                            new cminor::parsing::NonterminalParser(ToUtf32("rest"), ToUtf32("Identifier"), 0))))))));
    AddRule(new StringArrayRule(ToUtf32("StringArray"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('['),
                new cminor::parsing::ListParser(
                    new cminor::parsing::ActionParser(ToUtf32("A0"),
                        new cminor::parsing::NonterminalParser(ToUtf32("str"), ToUtf32("string"), 0)),
                    new cminor::parsing::CharParser(','))),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::CharParser(']')))));
}

} } // namespace cminor.syntax
