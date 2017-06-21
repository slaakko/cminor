#include "Interface.hpp"
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
#include <cminor/parser/Specifier.hpp>
#include <cminor/parser/Identifier.hpp>
#include <cminor/parser/TypeExpr.hpp>
#include <cminor/parser/Parameter.hpp>
#include <cminor/parser/Keyword.hpp>
#include <cminor/ast/Class.hpp>

namespace cminor { namespace parser {

using namespace cminor::parsing;
using namespace cminor::util;
using namespace cminor::unicode;

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

InterfaceGrammar::InterfaceGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("InterfaceGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.parser")), parsingDomain_)
{
    SetOwner(0);
}

InterfaceNode* InterfaceGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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
    InterfaceNode* result = *static_cast<cminor::parsing::ValueObject<InterfaceNode*>*>(value.get());
    stack.pop();
    return result;
}

class InterfaceGrammar::InterfaceRule : public cminor::parsing::Rule
{
public:
    InterfaceRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("InterfaceNode*"));
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
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InterfaceRule>(this, &InterfaceRule::A0Action));
        cminor::parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal(ToUtf32("Specifiers"));
        specifiersNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InterfaceRule>(this, &InterfaceRule::PostSpecifiers));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("Identifier"));
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InterfaceRule>(this, &InterfaceRule::PostIdentifier));
        cminor::parsing::NonterminalParser* interfaceContentNonterminalParser = GetNonterminal(ToUtf32("InterfaceContent"));
        interfaceContentNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<InterfaceRule>(this, &InterfaceRule::PreInterfaceContent));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
    InterfaceContentRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        AddInheritedAttribute(AttrOrVariable(ToUtf32("InterfaceNode*"), ToUtf32("intf")));
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
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InterfaceContentRule>(this, &InterfaceContentRule::A0Action));
        cminor::parsing::NonterminalParser* interfaceMemFunNonterminalParser = GetNonterminal(ToUtf32("InterfaceMemFun"));
        interfaceMemFunNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<InterfaceContentRule>(this, &InterfaceContentRule::PreInterfaceMemFun));
        interfaceMemFunNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InterfaceContentRule>(this, &InterfaceContentRule::PostInterfaceMemFun));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
    InterfaceMemFunRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("Node*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<MemberFunctionNode>"), ToUtf32("memFun")));
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InterfaceMemFunRule>(this, &InterfaceMemFunRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InterfaceMemFunRule>(this, &InterfaceMemFunRule::A1Action));
        cminor::parsing::NonterminalParser* returnTypeExprNonterminalParser = GetNonterminal(ToUtf32("returnTypeExpr"));
        returnTypeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<InterfaceMemFunRule>(this, &InterfaceMemFunRule::PrereturnTypeExpr));
        returnTypeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InterfaceMemFunRule>(this, &InterfaceMemFunRule::PostreturnTypeExpr));
        cminor::parsing::NonterminalParser* groupIdNonterminalParser = GetNonterminal(ToUtf32("groupId"));
        groupIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InterfaceMemFunRule>(this, &InterfaceMemFunRule::PostgroupId));
        cminor::parsing::NonterminalParser* parameterListNonterminalParser = GetNonterminal(ToUtf32("ParameterList"));
        parameterListNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<InterfaceMemFunRule>(this, &InterfaceMemFunRule::PreParameterList));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->memFun.release();
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
    InterfaceFunctionGroupIdRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("FunctionGroupIdNode*"));
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
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InterfaceFunctionGroupIdRule>(this, &InterfaceFunctionGroupIdRule::A0Action));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("identifier"));
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InterfaceFunctionGroupIdRule>(this, &InterfaceFunctionGroupIdRule::Postidentifier));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromidentifier = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromidentifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromidentifier() {}
        FunctionGroupIdNode* value;
        std::u32string fromidentifier;
    };
};

void InterfaceGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cminor.parser.TypeExprGrammar"));
    if (!grammar0)
    {
        grammar0 = cminor::parser::TypeExprGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar(ToUtf32("cminor.parser.SpecifierGrammar"));
    if (!grammar1)
    {
        grammar1 = cminor::parser::SpecifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar(ToUtf32("cminor.parsing.stdlib"));
    if (!grammar2)
    {
        grammar2 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar(ToUtf32("cminor.parser.KeywordGrammar"));
    if (!grammar3)
    {
        grammar3 = cminor::parser::KeywordGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
    cminor::parsing::Grammar* grammar4 = pd->GetGrammar(ToUtf32("cminor.parser.IdentifierGrammar"));
    if (!grammar4)
    {
        grammar4 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar4);
    cminor::parsing::Grammar* grammar5 = pd->GetGrammar(ToUtf32("cminor.parser.ParameterGrammar"));
    if (!grammar5)
    {
        grammar5 = cminor::parser::ParameterGrammar::Create(pd);
    }
    AddGrammarReference(grammar5);
}

void InterfaceGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Specifiers"), this, ToUtf32("SpecifierGrammar.Specifiers")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("identifier"), this, ToUtf32("cminor.parsing.stdlib.identifier")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Identifier"), this, ToUtf32("IdentifierGrammar.Identifier")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("TypeExpr"), this, ToUtf32("TypeExprGrammar.TypeExpr")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("ParameterList"), this, ToUtf32("ParameterGrammar.ParameterList")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Keyword"), this, ToUtf32("KeywordGrammar.Keyword")));
    AddRule(new InterfaceRule(ToUtf32("Interface"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::ActionParser(ToUtf32("A0"),
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("Specifiers"), ToUtf32("Specifiers"), 0),
                                new cminor::parsing::KeywordParser(ToUtf32("interface"))),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("Identifier"), ToUtf32("Identifier"), 0)))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('{'))),
                new cminor::parsing::NonterminalParser(ToUtf32("InterfaceContent"), ToUtf32("InterfaceContent"), 2)),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::CharParser('}')))));
    AddRule(new InterfaceContentRule(ToUtf32("InterfaceContent"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::KleeneStarParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("InterfaceMemFun"), ToUtf32("InterfaceMemFun"), 1)))));
    AddRule(new InterfaceMemFunRule(ToUtf32("InterfaceMemFun"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("returnTypeExpr"), ToUtf32("TypeExpr"), 1),
                        new cminor::parsing::ActionParser(ToUtf32("A1"),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("groupId"), ToUtf32("InterfaceFunctionGroupId"), 0)))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("ParameterList"), ToUtf32("ParameterList"), 2))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new InterfaceFunctionGroupIdRule(ToUtf32("InterfaceFunctionGroupId"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::TokenParser(
                new cminor::parsing::DifferenceParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("identifier"), ToUtf32("identifier"), 0),
                    new cminor::parsing::NonterminalParser(ToUtf32("Keyword"), ToUtf32("Keyword"), 0))))));
}

} } // namespace cminor.parser
