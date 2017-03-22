#include "Interface.hpp"
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
#include <cminor/parser/Identifier.hpp>
#include <cminor/parser/TypeExpr.hpp>
#include <cminor/parser/Parameter.hpp>
#include <cminor/parser/Keyword.hpp>
#include <cminor/ast/Class.hpp>

namespace cminor { namespace parser {

using namespace cminor::parsing;

InterfaceGrammar* InterfaceGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

InterfaceGrammar* InterfaceGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    InterfaceGrammar* grammar(new InterfaceGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

InterfaceGrammar::InterfaceGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("InterfaceGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

InterfaceNode* InterfaceGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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
    InterfaceNode* result = *static_cast<cminor::parsing::ValueObject<InterfaceNode*>*>(value.get());
    stack.pop();
    return result;
}

class InterfaceGrammar::InterfaceRule : public cminor::parsing::Rule
{
public:
    InterfaceRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("InterfaceNode*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<InterfaceNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InterfaceRule>(this, &InterfaceRule::A0Action));
        cminor::parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal("Specifiers");
        specifiersNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InterfaceRule>(this, &InterfaceRule::PostSpecifiers));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InterfaceRule>(this, &InterfaceRule::PostIdentifier));
        cminor::parsing::NonterminalParser* interfaceContentNonterminalParser = GetNonterminal("InterfaceContent");
        interfaceContentNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<InterfaceRule>(this, &InterfaceRule::PreInterfaceContent));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new InterfaceNode(span, context->fromSpecifiers, context->fromIdentifier);
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
    void PreInterfaceContent(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<InterfaceNode*>(context->value)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), fromSpecifiers(), fromIdentifier() {}
        ParsingContext* ctx;
        InterfaceNode* value;
        Specifiers fromSpecifiers;
        IdentifierNode* fromIdentifier;
    };
};

class InterfaceGrammar::InterfaceContentRule : public cminor::parsing::Rule
{
public:
    InterfaceContentRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("InterfaceNode*", "intf"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> intf_value = std::move(stack.top());
        context->intf = *static_cast<cminor::parsing::ValueObject<InterfaceNode*>*>(intf_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InterfaceContentRule>(this, &InterfaceContentRule::A0Action));
        cminor::parsing::NonterminalParser* interfaceMemFunNonterminalParser = GetNonterminal("InterfaceMemFun");
        interfaceMemFunNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<InterfaceContentRule>(this, &InterfaceContentRule::PreInterfaceMemFun));
        interfaceMemFunNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InterfaceContentRule>(this, &InterfaceContentRule::PostInterfaceMemFun));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->intf->AddMember(context->fromInterfaceMemFun);
    }
    void PreInterfaceMemFun(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostInterfaceMemFun(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromInterfaceMemFun_value = std::move(stack.top());
            context->fromInterfaceMemFun = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromInterfaceMemFun_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), intf(), fromInterfaceMemFun() {}
        ParsingContext* ctx;
        InterfaceNode* intf;
        Node* fromInterfaceMemFun;
    };
};

class InterfaceGrammar::InterfaceMemFunRule : public cminor::parsing::Rule
{
public:
    InterfaceMemFunRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<MemberFunctionNode>", "memFun"));
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InterfaceMemFunRule>(this, &InterfaceMemFunRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InterfaceMemFunRule>(this, &InterfaceMemFunRule::A1Action));
        cminor::parsing::NonterminalParser* returnTypeExprNonterminalParser = GetNonterminal("returnTypeExpr");
        returnTypeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<InterfaceMemFunRule>(this, &InterfaceMemFunRule::PrereturnTypeExpr));
        returnTypeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InterfaceMemFunRule>(this, &InterfaceMemFunRule::PostreturnTypeExpr));
        cminor::parsing::NonterminalParser* groupIdNonterminalParser = GetNonterminal("groupId");
        groupIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InterfaceMemFunRule>(this, &InterfaceMemFunRule::PostgroupId));
        cminor::parsing::NonterminalParser* parameterListNonterminalParser = GetNonterminal("ParameterList");
        parameterListNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<InterfaceMemFunRule>(this, &InterfaceMemFunRule::PreParameterList));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->memFun.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->memFun.reset(new MemberFunctionNode(span, Specifiers(), context->fromreturnTypeExpr, context->fromgroupId));
    }
    void PrereturnTypeExpr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostreturnTypeExpr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromreturnTypeExpr_value = std::move(stack.top());
            context->fromreturnTypeExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromreturnTypeExpr_value.get());
            stack.pop();
        }
    }
    void PostgroupId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromgroupId_value = std::move(stack.top());
            context->fromgroupId = *static_cast<cminor::parsing::ValueObject<FunctionGroupIdNode*>*>(fromgroupId_value.get());
            stack.pop();
        }
    }
    void PreParameterList(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->memFun.get())));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), memFun(), fromreturnTypeExpr(), fromgroupId() {}
        ParsingContext* ctx;
        Node* value;
        std::unique_ptr<MemberFunctionNode> memFun;
        Node* fromreturnTypeExpr;
        FunctionGroupIdNode* fromgroupId;
    };
};

class InterfaceGrammar::InterfaceFunctionGroupIdRule : public cminor::parsing::Rule
{
public:
    InterfaceFunctionGroupIdRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("FunctionGroupIdNode*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<FunctionGroupIdNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InterfaceFunctionGroupIdRule>(this, &InterfaceFunctionGroupIdRule::A0Action));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InterfaceFunctionGroupIdRule>(this, &InterfaceFunctionGroupIdRule::Postidentifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new FunctionGroupIdNode(span, context->fromidentifier);
    }
    void Postidentifier(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromidentifier_value = std::move(stack.top());
            context->fromidentifier = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromidentifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromidentifier() {}
        FunctionGroupIdNode* value;
        std::string fromidentifier;
    };
};

void InterfaceGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("cminor.parser.IdentifierGrammar");
    if (!grammar0)
    {
        grammar0 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar("cminor.parser.SpecifierGrammar");
    if (!grammar1)
    {
        grammar1 = cminor::parser::SpecifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar("cminor.parser.KeywordGrammar");
    if (!grammar2)
    {
        grammar2 = cminor::parser::KeywordGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar("cminor.parser.TypeExprGrammar");
    if (!grammar3)
    {
        grammar3 = cminor::parser::TypeExprGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
    cminor::parsing::Grammar* grammar4 = pd->GetGrammar("cminor.parser.ParameterGrammar");
    if (!grammar4)
    {
        grammar4 = cminor::parser::ParameterGrammar::Create(pd);
    }
    AddGrammarReference(grammar4);
    cminor::parsing::Grammar* grammar5 = pd->GetGrammar("cminor.parsing.stdlib");
    if (!grammar5)
    {
        grammar5 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar5);
}

void InterfaceGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("Specifiers", this, "SpecifierGrammar.Specifiers"));
    AddRuleLink(new cminor::parsing::RuleLink("TypeExpr", this, "TypeExprGrammar.TypeExpr"));
    AddRuleLink(new cminor::parsing::RuleLink("Identifier", this, "IdentifierGrammar.Identifier"));
    AddRuleLink(new cminor::parsing::RuleLink("ParameterList", this, "ParameterGrammar.ParameterList"));
    AddRuleLink(new cminor::parsing::RuleLink("Keyword", this, "KeywordGrammar.Keyword"));
    AddRuleLink(new cminor::parsing::RuleLink("identifier", this, "cminor.parsing.stdlib.identifier"));
    AddRule(new InterfaceRule("Interface", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::ActionParser("A0",
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::NonterminalParser("Specifiers", "Specifiers", 0),
                                new cminor::parsing::KeywordParser("interface")),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0)))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('{'))),
                new cminor::parsing::NonterminalParser("InterfaceContent", "InterfaceContent", 2)),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::CharParser('}')))));
    AddRule(new InterfaceContentRule("InterfaceContent", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::KleeneStarParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("InterfaceMemFun", "InterfaceMemFun", 1)))));
    AddRule(new InterfaceMemFunRule("InterfaceMemFun", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::NonterminalParser("returnTypeExpr", "TypeExpr", 1),
                        new cminor::parsing::ActionParser("A1",
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("groupId", "InterfaceFunctionGroupId", 0)))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("ParameterList", "ParameterList", 2))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new InterfaceFunctionGroupIdRule("InterfaceFunctionGroupId", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::TokenParser(
                new cminor::parsing::DifferenceParser(
                    new cminor::parsing::NonterminalParser("identifier", "identifier", 0),
                    new cminor::parsing::NonterminalParser("Keyword", "Keyword", 0))))));
}

} } // namespace cminor.parser
