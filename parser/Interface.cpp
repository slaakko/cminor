#include "Interface.hpp"
#include <Cm.Parsing/Action.hpp>
#include <Cm.Parsing/Rule.hpp>
#include <Cm.Parsing/ParsingDomain.hpp>
#include <Cm.Parsing/Primitive.hpp>
#include <Cm.Parsing/Composite.hpp>
#include <Cm.Parsing/Nonterminal.hpp>
#include <Cm.Parsing/Exception.hpp>
#include <Cm.Parsing/StdLib.hpp>
#include <Cm.Parsing/XmlLog.hpp>
#include <cminor/parser/Specifier.hpp>
#include <cminor/parser/Identifier.hpp>
#include <cminor/parser/TypeExpr.hpp>
#include <cminor/parser/Parameter.hpp>
#include <cminor/parser/Keyword.hpp>
#include <cminor/ast/Class.hpp>

namespace cminor { namespace parser {

using namespace Cm::Parsing;

InterfaceGrammar* InterfaceGrammar::Create()
{
    return Create(new Cm::Parsing::ParsingDomain());
}

InterfaceGrammar* InterfaceGrammar::Create(Cm::Parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    InterfaceGrammar* grammar(new InterfaceGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

InterfaceGrammar::InterfaceGrammar(Cm::Parsing::ParsingDomain* parsingDomain_): Cm::Parsing::Grammar("InterfaceGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

InterfaceNode* InterfaceGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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
    InterfaceNode* result = *static_cast<Cm::Parsing::ValueObject<InterfaceNode*>*>(value.get());
    stack.pop();
    return result;
}

class InterfaceGrammar::InterfaceRule : public Cm::Parsing::Rule
{
public:
    InterfaceRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("InterfaceNode*");
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<InterfaceNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<InterfaceRule>(this, &InterfaceRule::A0Action));
        Cm::Parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal("Specifiers");
        specifiersNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<InterfaceRule>(this, &InterfaceRule::PostSpecifiers));
        Cm::Parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<InterfaceRule>(this, &InterfaceRule::PostIdentifier));
        Cm::Parsing::NonterminalParser* interfaceContentNonterminalParser = GetNonterminal("InterfaceContent");
        interfaceContentNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<InterfaceRule>(this, &InterfaceRule::PreInterfaceContent));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new InterfaceNode(span, context.fromSpecifiers, context.fromIdentifier);
    }
    void PostSpecifiers(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context.fromSpecifiers = *static_cast<Cm::Parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
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
    void PreInterfaceContent(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<InterfaceNode*>(context.value)));
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromSpecifiers(), fromIdentifier() {}
        ParsingContext* ctx;
        InterfaceNode* value;
        Specifiers fromSpecifiers;
        IdentifierNode* fromIdentifier;
    };
    std::stack<Context> contextStack;
    Context context;
};

class InterfaceGrammar::InterfaceContentRule : public Cm::Parsing::Rule
{
public:
    InterfaceContentRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("InterfaceNode*", "intf"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> intf_value = std::move(stack.top());
        context.intf = *static_cast<Cm::Parsing::ValueObject<InterfaceNode*>*>(intf_value.get());
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<InterfaceContentRule>(this, &InterfaceContentRule::A0Action));
        Cm::Parsing::NonterminalParser* interfaceMemFunNonterminalParser = GetNonterminal("InterfaceMemFun");
        interfaceMemFunNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<InterfaceContentRule>(this, &InterfaceContentRule::PreInterfaceMemFun));
        interfaceMemFunNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<InterfaceContentRule>(this, &InterfaceContentRule::PostInterfaceMemFun));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.intf->AddMember(context.fromInterfaceMemFun);
    }
    void PreInterfaceMemFun(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostInterfaceMemFun(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromInterfaceMemFun_value = std::move(stack.top());
            context.fromInterfaceMemFun = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromInterfaceMemFun_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), intf(), fromInterfaceMemFun() {}
        ParsingContext* ctx;
        InterfaceNode* intf;
        Node* fromInterfaceMemFun;
    };
    std::stack<Context> contextStack;
    Context context;
};

class InterfaceGrammar::InterfaceMemFunRule : public Cm::Parsing::Rule
{
public:
    InterfaceMemFunRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<MemberFunctionNode>", "memFun"));
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<InterfaceMemFunRule>(this, &InterfaceMemFunRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<InterfaceMemFunRule>(this, &InterfaceMemFunRule::A1Action));
        Cm::Parsing::NonterminalParser* returnTypeExprNonterminalParser = GetNonterminal("returnTypeExpr");
        returnTypeExprNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<InterfaceMemFunRule>(this, &InterfaceMemFunRule::PrereturnTypeExpr));
        returnTypeExprNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<InterfaceMemFunRule>(this, &InterfaceMemFunRule::PostreturnTypeExpr));
        Cm::Parsing::NonterminalParser* groupIdNonterminalParser = GetNonterminal("groupId");
        groupIdNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<InterfaceMemFunRule>(this, &InterfaceMemFunRule::PostgroupId));
        Cm::Parsing::NonterminalParser* parameterListNonterminalParser = GetNonterminal("ParameterList");
        parameterListNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<InterfaceMemFunRule>(this, &InterfaceMemFunRule::PreParameterList));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.memFun.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.memFun.reset(new MemberFunctionNode(span, Specifiers(), context.fromreturnTypeExpr, context.fromgroupId));
    }
    void PrereturnTypeExpr(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostreturnTypeExpr(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromreturnTypeExpr_value = std::move(stack.top());
            context.fromreturnTypeExpr = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromreturnTypeExpr_value.get());
            stack.pop();
        }
    }
    void PostgroupId(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromgroupId_value = std::move(stack.top());
            context.fromgroupId = *static_cast<Cm::Parsing::ValueObject<FunctionGroupIdNode*>*>(fromgroupId_value.get());
            stack.pop();
        }
    }
    void PreParameterList(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Node*>(context.memFun.get())));
    }
private:
    struct Context
    {
        Context(): ctx(), value(), memFun(), fromreturnTypeExpr(), fromgroupId() {}
        ParsingContext* ctx;
        Node* value;
        std::unique_ptr<MemberFunctionNode> memFun;
        Node* fromreturnTypeExpr;
        FunctionGroupIdNode* fromgroupId;
    };
    std::stack<Context> contextStack;
    Context context;
};

class InterfaceGrammar::InterfaceFunctionGroupIdRule : public Cm::Parsing::Rule
{
public:
    InterfaceFunctionGroupIdRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("FunctionGroupIdNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<FunctionGroupIdNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<InterfaceFunctionGroupIdRule>(this, &InterfaceFunctionGroupIdRule::A0Action));
        Cm::Parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("identifier");
        identifierNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<InterfaceFunctionGroupIdRule>(this, &InterfaceFunctionGroupIdRule::Postidentifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, context.fromidentifier);
    }
    void Postidentifier(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromidentifier_value = std::move(stack.top());
            context.fromidentifier = *static_cast<Cm::Parsing::ValueObject<std::string>*>(fromidentifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromidentifier() {}
        FunctionGroupIdNode* value;
        std::string fromidentifier;
    };
    std::stack<Context> contextStack;
    Context context;
};

void InterfaceGrammar::GetReferencedGrammars()
{
    Cm::Parsing::ParsingDomain* pd = GetParsingDomain();
    Cm::Parsing::Grammar* grammar0 = pd->GetGrammar("cminor.parser.SpecifierGrammar");
    if (!grammar0)
    {
        grammar0 = cminor::parser::SpecifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    Cm::Parsing::Grammar* grammar1 = pd->GetGrammar("Cm.Parsing.stdlib");
    if (!grammar1)
    {
        grammar1 = Cm::Parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar1);
    Cm::Parsing::Grammar* grammar2 = pd->GetGrammar("cminor.parser.KeywordGrammar");
    if (!grammar2)
    {
        grammar2 = cminor::parser::KeywordGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    Cm::Parsing::Grammar* grammar3 = pd->GetGrammar("cminor.parser.TypeExprGrammar");
    if (!grammar3)
    {
        grammar3 = cminor::parser::TypeExprGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
    Cm::Parsing::Grammar* grammar4 = pd->GetGrammar("cminor.parser.IdentifierGrammar");
    if (!grammar4)
    {
        grammar4 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar4);
    Cm::Parsing::Grammar* grammar5 = pd->GetGrammar("cminor.parser.ParameterGrammar");
    if (!grammar5)
    {
        grammar5 = cminor::parser::ParameterGrammar::Create(pd);
    }
    AddGrammarReference(grammar5);
}

void InterfaceGrammar::CreateRules()
{
    AddRuleLink(new Cm::Parsing::RuleLink("Specifiers", this, "SpecifierGrammar.Specifiers"));
    AddRuleLink(new Cm::Parsing::RuleLink("Keyword", this, "KeywordGrammar.Keyword"));
    AddRuleLink(new Cm::Parsing::RuleLink("Identifier", this, "IdentifierGrammar.Identifier"));
    AddRuleLink(new Cm::Parsing::RuleLink("TypeExpr", this, "TypeExprGrammar.TypeExpr"));
    AddRuleLink(new Cm::Parsing::RuleLink("ParameterList", this, "ParameterGrammar.ParameterList"));
    AddRuleLink(new Cm::Parsing::RuleLink("identifier", this, "Cm.Parsing.stdlib.identifier"));
    AddRule(new InterfaceRule("Interface", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::ActionParser("A0",
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::SequenceParser(
                                new Cm::Parsing::NonterminalParser("Specifiers", "Specifiers", 0),
                                new Cm::Parsing::KeywordParser("interface")),
                            new Cm::Parsing::ExpectationParser(
                                new Cm::Parsing::NonterminalParser("Identifier", "Identifier", 0)))),
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::CharParser('{'))),
                new Cm::Parsing::NonterminalParser("InterfaceContent", "InterfaceContent", 2)),
            new Cm::Parsing::ExpectationParser(
                new Cm::Parsing::CharParser('}')))));
    AddRule(new InterfaceContentRule("InterfaceContent", GetScope(),
        new Cm::Parsing::KleeneStarParser(
            new Cm::Parsing::ActionParser("A0",
                new Cm::Parsing::NonterminalParser("InterfaceMemFun", "InterfaceMemFun", 1)))));
    AddRule(new InterfaceMemFunRule("InterfaceMemFun", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::NonterminalParser("returnTypeExpr", "TypeExpr", 1),
                        new Cm::Parsing::ActionParser("A1",
                            new Cm::Parsing::ExpectationParser(
                                new Cm::Parsing::NonterminalParser("groupId", "InterfaceFunctionGroupId", 0)))),
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::NonterminalParser("ParameterList", "ParameterList", 2))),
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::CharParser(';'))))));
    AddRule(new InterfaceFunctionGroupIdRule("InterfaceFunctionGroupId", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::TokenParser(
                new Cm::Parsing::DifferenceParser(
                    new Cm::Parsing::NonterminalParser("identifier", "identifier", 0),
                    new Cm::Parsing::NonterminalParser("Keyword", "Keyword", 0))))));
}

} } // namespace cminor.parser
