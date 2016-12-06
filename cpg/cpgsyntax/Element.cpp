#include "Element.hpp"
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pom/Type.hpp>
#include <cminor/cpg/cpgcpp/Declarator.hpp>

namespace cpg { namespace syntax {

using namespace cminor::parsing;

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

ElementGrammar::ElementGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("ElementGrammar", parsingDomain_->GetNamespaceScope("Cm.Parsing.Syntax"), parsingDomain_)
{
    SetOwner(0);
    keywords0.push_back("anychar");
    keywords0.push_back("cc");
    keywords0.push_back("digit");
    keywords0.push_back("empty");
    keywords0.push_back("end");
    keywords0.push_back("grammar");
    keywords0.push_back("hexdigit");
    keywords0.push_back("keyword");
    keywords0.push_back("keyword_list");
    keywords0.push_back("letter");
    keywords0.push_back("punctuation");
    keywords0.push_back("skip");
    keywords0.push_back("space");
    keywords0.push_back("start");
    keywords0.push_back("token");
    keywords0.push_back("using");
    keywords0.push_back("var");
}

void ElementGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, cminor::parsing::Grammar* grammar)
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
    stack.push(std::unique_ptr<cminor::parsing::Object>(new ValueObject<cminor::parsing::Grammar*>(grammar)));
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
}

class ElementGrammar::RuleLinkRule : public cminor::parsing::Rule
{
public:
    RuleLinkRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Grammar*", "grammar"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> grammar_value = std::move(stack.top());
        context.grammar = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Grammar*>*>(grammar_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RuleLinkRule>(this, &RuleLinkRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RuleLinkRule>(this, &RuleLinkRule::A1Action));
        cminor::parsing::NonterminalParser* aliasNameNonterminalParser = GetNonterminal("aliasName");
        aliasNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RuleLinkRule>(this, &RuleLinkRule::PostaliasName));
        cminor::parsing::NonterminalParser* ruleNameNonterminalParser = GetNonterminal("ruleName");
        ruleNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RuleLinkRule>(this, &RuleLinkRule::PostruleName));
        cminor::parsing::NonterminalParser* qualified_idNonterminalParser = GetNonterminal("qualified_id");
        qualified_idNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RuleLinkRule>(this, &RuleLinkRule::Postqualified_id));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        RuleLink * link(new RuleLink(context.fromaliasName, context.grammar, context.fromruleName));
        link->SetSpan(span);
        context.grammar->AddRuleLink(link);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        RuleLink * link(new RuleLink(context.grammar, context.fromqualified_id));
        link->SetSpan(span);
        context.grammar->AddRuleLink(link);
    }
    void PostaliasName(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromaliasName_value = std::move(stack.top());
            context.fromaliasName = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromaliasName_value.get());
            stack.pop();
        }
    }
    void PostruleName(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromruleName_value = std::move(stack.top());
            context.fromruleName = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromruleName_value.get());
            stack.pop();
        }
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
        Context(): grammar(), fromaliasName(), fromruleName(), fromqualified_id() {}
        cminor::parsing::Grammar* grammar;
        std::string fromaliasName;
        std::string fromruleName;
        std::string fromqualified_id;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ElementGrammar::SignatureRule : public cminor::parsing::Rule
{
public:
    SignatureRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Rule*", "rule"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> rule_value = std::move(stack.top());
        context.rule = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Rule*>*>(rule_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::NonterminalParser* parameterListNonterminalParser = GetNonterminal("ParameterList");
        parameterListNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<SignatureRule>(this, &SignatureRule::PreParameterList));
        cminor::parsing::NonterminalParser* returnTypeNonterminalParser = GetNonterminal("ReturnType");
        returnTypeNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<SignatureRule>(this, &SignatureRule::PreReturnType));
    }
    void PreParameterList(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Rule*>(context.rule)));
    }
    void PreReturnType(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Rule*>(context.rule)));
    }
private:
    struct Context
    {
        Context(): rule() {}
        cminor::parsing::Rule* rule;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ElementGrammar::ParameterListRule : public cminor::parsing::Rule
{
public:
    ParameterListRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Rule*", "rule"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> rule_value = std::move(stack.top());
        context.rule = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Rule*>*>(rule_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::NonterminalParser* variableNonterminalParser = GetNonterminal("Variable");
        variableNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ParameterListRule>(this, &ParameterListRule::PreVariable));
        cminor::parsing::NonterminalParser* parameterNonterminalParser = GetNonterminal("Parameter");
        parameterNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ParameterListRule>(this, &ParameterListRule::PreParameter));
    }
    void PreVariable(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Rule*>(context.rule)));
    }
    void PreParameter(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Rule*>(context.rule)));
    }
private:
    struct Context
    {
        Context(): rule() {}
        cminor::parsing::Rule* rule;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ElementGrammar::VariableRule : public cminor::parsing::Rule
{
public:
    VariableRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Rule*", "rule"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> rule_value = std::move(stack.top());
        context.rule = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Rule*>*>(rule_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<VariableRule>(this, &VariableRule::A0Action));
        cminor::parsing::NonterminalParser* typeIdNonterminalParser = GetNonterminal("TypeId");
        typeIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<VariableRule>(this, &VariableRule::PostTypeId));
        cminor::parsing::NonterminalParser* declaratorNonterminalParser = GetNonterminal("Declarator");
        declaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<VariableRule>(this, &VariableRule::PostDeclarator));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.rule->AddLocalVariable(AttrOrVariable(context.fromTypeId->ToString(), context.fromDeclarator));
        delete context.fromTypeId;
    }
    void PostTypeId(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeId_value = std::move(stack.top());
            context.fromTypeId = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeId*>*>(fromTypeId_value.get());
            stack.pop();
        }
    }
    void PostDeclarator(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDeclarator_value = std::move(stack.top());
            context.fromDeclarator = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromDeclarator_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): rule(), fromTypeId(), fromDeclarator() {}
        cminor::parsing::Rule* rule;
        cminor::pom::TypeId* fromTypeId;
        std::string fromDeclarator;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ElementGrammar::ParameterRule : public cminor::parsing::Rule
{
public:
    ParameterRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Rule*", "rule"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> rule_value = std::move(stack.top());
        context.rule = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Rule*>*>(rule_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ParameterRule>(this, &ParameterRule::A0Action));
        cminor::parsing::NonterminalParser* typeIdNonterminalParser = GetNonterminal("TypeId");
        typeIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ParameterRule>(this, &ParameterRule::PostTypeId));
        cminor::parsing::NonterminalParser* declaratorNonterminalParser = GetNonterminal("Declarator");
        declaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ParameterRule>(this, &ParameterRule::PostDeclarator));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.rule->AddInheritedAttribute(AttrOrVariable(context.fromTypeId->ToString(), context.fromDeclarator));
        delete context.fromTypeId;
    }
    void PostTypeId(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeId_value = std::move(stack.top());
            context.fromTypeId = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeId*>*>(fromTypeId_value.get());
            stack.pop();
        }
    }
    void PostDeclarator(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDeclarator_value = std::move(stack.top());
            context.fromDeclarator = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromDeclarator_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): rule(), fromTypeId(), fromDeclarator() {}
        cminor::parsing::Rule* rule;
        cminor::pom::TypeId* fromTypeId;
        std::string fromDeclarator;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ElementGrammar::ReturnTypeRule : public cminor::parsing::Rule
{
public:
    ReturnTypeRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Rule*", "rule"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> rule_value = std::move(stack.top());
        context.rule = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Rule*>*>(rule_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ReturnTypeRule>(this, &ReturnTypeRule::A0Action));
        cminor::parsing::NonterminalParser* typeIdNonterminalParser = GetNonterminal("TypeId");
        typeIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ReturnTypeRule>(this, &ReturnTypeRule::PostTypeId));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.rule->SetValueTypeName(context.fromTypeId->ToString());
        delete context.fromTypeId;
    }
    void PostTypeId(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeId_value = std::move(stack.top());
            context.fromTypeId = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeId*>*>(fromTypeId_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): rule(), fromTypeId() {}
        cminor::parsing::Rule* rule;
        cminor::pom::TypeId* fromTypeId;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ElementGrammar::IdentifierRule : public cminor::parsing::Rule
{
public:
    IdentifierRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IdentifierRule>(this, &IdentifierRule::A0Action));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IdentifierRule>(this, &IdentifierRule::Postidentifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = std::string(matchBegin, matchEnd);
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
private:
    struct Context
    {
        Context(): value(), fromidentifier() {}
        std::string value;
        std::string fromidentifier;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ElementGrammar::QualifiedIdRule : public cminor::parsing::Rule
{
public:
    QualifiedIdRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<QualifiedIdRule>(this, &QualifiedIdRule::A0Action));
        cminor::parsing::NonterminalParser* firstNonterminalParser = GetNonterminal("first");
        firstNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<QualifiedIdRule>(this, &QualifiedIdRule::Postfirst));
        cminor::parsing::NonterminalParser* restNonterminalParser = GetNonterminal("rest");
        restNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<QualifiedIdRule>(this, &QualifiedIdRule::Postrest));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = std::string(matchBegin, matchEnd);
    }
    void Postfirst(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromfirst_value = std::move(stack.top());
            context.fromfirst = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromfirst_value.get());
            stack.pop();
        }
    }
    void Postrest(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromrest_value = std::move(stack.top());
            context.fromrest = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromrest_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromfirst(), fromrest() {}
        std::string value;
        std::string fromfirst;
        std::string fromrest;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ElementGrammar::StringArrayRule : public cminor::parsing::Rule
{
public:
    StringArrayRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("std::vector<std::string>*", "array"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> array_value = std::move(stack.top());
        context.array = *static_cast<cminor::parsing::ValueObject<std::vector<std::string>*>*>(array_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StringArrayRule>(this, &StringArrayRule::A0Action));
        cminor::parsing::NonterminalParser* strNonterminalParser = GetNonterminal("str");
        strNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StringArrayRule>(this, &StringArrayRule::Poststr));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.array->push_back(context.fromstr);
    }
    void Poststr(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromstr_value = std::move(stack.top());
            context.fromstr = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromstr_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): array(), fromstr() {}
        std::vector<std::string>* array;
        std::string fromstr;
    };
    std::stack<Context> contextStack;
    Context context;
};

void ElementGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("Cm.Parsing.Cpp.DeclaratorGrammar");
    if (!grammar0)
    {
        grammar0 = cpg::cpp::DeclaratorGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar("Cm.Parsing.stdlib");
    if (!grammar1)
    {
        grammar1 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar1);
}

void ElementGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("Declarator", this, "Cm.Parsing.Cpp.DeclaratorGrammar.Declarator"));
    AddRuleLink(new cminor::parsing::RuleLink("qualified_id", this, "Cm.Parsing.stdlib.qualified_id"));
    AddRuleLink(new cminor::parsing::RuleLink("identifier", this, "Cm.Parsing.stdlib.identifier"));
    AddRuleLink(new cminor::parsing::RuleLink("string", this, "Cm.Parsing.stdlib.string"));
    AddRuleLink(new cminor::parsing::RuleLink("TypeId", this, "Cm.Parsing.Cpp.DeclaratorGrammar.TypeId"));
    AddRule(new RuleLinkRule("RuleLink", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::KeywordParser("using"),
                                new cminor::parsing::NonterminalParser("aliasName", "identifier", 0)),
                            new cminor::parsing::CharParser('=')),
                        new cminor::parsing::NonterminalParser("ruleName", "qualified_id", 0)),
                    new cminor::parsing::CharParser(';'))),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser("using"),
                        new cminor::parsing::NonterminalParser("qualified_id", "qualified_id", 0)),
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new SignatureRule("Signature", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::OptionalParser(
                new cminor::parsing::NonterminalParser("ParameterList", "ParameterList", 1)),
            new cminor::parsing::OptionalParser(
                new cminor::parsing::NonterminalParser("ReturnType", "ReturnType", 1)))));
    AddRule(new ParameterListRule("ParameterList", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('('),
                new cminor::parsing::ListParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::NonterminalParser("Variable", "Variable", 1),
                        new cminor::parsing::NonterminalParser("Parameter", "Parameter", 1)),
                    new cminor::parsing::CharParser(','))),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::CharParser(')')))));
    AddRule(new VariableRule("Variable", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser("var"),
                new cminor::parsing::NonterminalParser("TypeId", "TypeId", 0)),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("Declarator", "Declarator", 0)))));
    AddRule(new ParameterRule("Parameter", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::NonterminalParser("TypeId", "TypeId", 0),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("Declarator", "Declarator", 0)))));
    AddRule(new ReturnTypeRule("ReturnType", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::DifferenceParser(
                new cminor::parsing::CharParser(':'),
                new cminor::parsing::StringParser("::")),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("TypeId", "TypeId", 0)))));
    AddRule(new cminor::parsing::Rule("Keyword", GetScope(),
        new cminor::parsing::KeywordListParser("identifier", keywords0)));
    AddRule(new IdentifierRule("Identifier", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::DifferenceParser(
                new cminor::parsing::NonterminalParser("identifier", "identifier", 0),
                new cminor::parsing::NonterminalParser("Keyword", "Keyword", 0)))));
    AddRule(new QualifiedIdRule("QualifiedId", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser("first", "Identifier", 0),
                    new cminor::parsing::KleeneStarParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::CharParser('.'),
                            new cminor::parsing::NonterminalParser("rest", "Identifier", 0))))))));
    AddRule(new StringArrayRule("StringArray", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('['),
                new cminor::parsing::ListParser(
                    new cminor::parsing::ActionParser("A0",
                        new cminor::parsing::NonterminalParser("str", "string", 0)),
                    new cminor::parsing::CharParser(','))),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::CharParser(']')))));
}

} } // namespace cpg.syntax
