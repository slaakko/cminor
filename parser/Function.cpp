#include "Function.hpp"
#include <Cm.Parsing/Action.hpp>
#include <Cm.Parsing/Rule.hpp>
#include <Cm.Parsing/ParsingDomain.hpp>
#include <Cm.Parsing/Primitive.hpp>
#include <Cm.Parsing/Composite.hpp>
#include <Cm.Parsing/Nonterminal.hpp>
#include <Cm.Parsing/Exception.hpp>
#include <Cm.Parsing/StdLib.hpp>
#include <Cm.Parsing/XmlLog.hpp>
#include <cminor/parser/Keyword.hpp>
#include <cminor/parser/Specifier.hpp>
#include <cminor/parser/TypeExpr.hpp>
#include <cminor/parser/Parameter.hpp>
#include <cminor/parser/Statement.hpp>

namespace cminor { namespace parser {

using namespace Cm::Parsing;

FunctionGrammar* FunctionGrammar::Create()
{
    return Create(new Cm::Parsing::ParsingDomain());
}

FunctionGrammar* FunctionGrammar::Create(Cm::Parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    FunctionGrammar* grammar(new FunctionGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

FunctionGrammar::FunctionGrammar(Cm::Parsing::ParsingDomain* parsingDomain_): Cm::Parsing::Grammar("FunctionGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

FunctionNode* FunctionGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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
    FunctionNode* result = *static_cast<Cm::Parsing::ValueObject<FunctionNode*>*>(value.get());
    stack.pop();
    return result;
}

class FunctionGrammar::FunctionRule : public Cm::Parsing::Rule
{
public:
    FunctionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("FunctionNode*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<FunctionNode>", "fun"));
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<FunctionNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<FunctionRule>(this, &FunctionRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<FunctionRule>(this, &FunctionRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<FunctionRule>(this, &FunctionRule::A2Action));
        Cm::Parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal("Specifiers");
        specifiersNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<FunctionRule>(this, &FunctionRule::PostSpecifiers));
        Cm::Parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<FunctionRule>(this, &FunctionRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<FunctionRule>(this, &FunctionRule::PostTypeExpr));
        Cm::Parsing::NonterminalParser* functionGroupIdNonterminalParser = GetNonterminal("FunctionGroupId");
        functionGroupIdNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<FunctionRule>(this, &FunctionRule::PreFunctionGroupId));
        functionGroupIdNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<FunctionRule>(this, &FunctionRule::PostFunctionGroupId));
        Cm::Parsing::NonterminalParser* parameterListNonterminalParser = GetNonterminal("ParameterList");
        parameterListNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<FunctionRule>(this, &FunctionRule::PreParameterList));
        Cm::Parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal("CompoundStatement");
        compoundStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<FunctionRule>(this, &FunctionRule::PreCompoundStatement));
        compoundStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<FunctionRule>(this, &FunctionRule::PostCompoundStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.fun->SetBody(context.fromCompoundStatement);
        context.fun->GetSpan().SetEnd(context.s.End());
        context.value = context.fun.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.fun.reset(new FunctionNode(span, context.fromSpecifiers, context.fromTypeExpr, context.fromFunctionGroupId));
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s = span;
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
    void PreFunctionGroupId(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostFunctionGroupId(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromFunctionGroupId_value = std::move(stack.top());
            context.fromFunctionGroupId = *static_cast<Cm::Parsing::ValueObject<FunctionGroupIdNode*>*>(fromFunctionGroupId_value.get());
            stack.pop();
        }
    }
    void PreParameterList(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Node*>(context.fun.get())));
    }
    void PreCompoundStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostCompoundStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context.fromCompoundStatement = *static_cast<Cm::Parsing::ValueObject<CompoundStatementNode*>*>(fromCompoundStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fun(), s(), fromSpecifiers(), fromTypeExpr(), fromFunctionGroupId(), fromCompoundStatement() {}
        ParsingContext* ctx;
        FunctionNode* value;
        std::unique_ptr<FunctionNode> fun;
        Span s;
        Specifiers fromSpecifiers;
        Node* fromTypeExpr;
        FunctionGroupIdNode* fromFunctionGroupId;
        CompoundStatementNode* fromCompoundStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

class FunctionGrammar::FunctionGroupIdRule : public Cm::Parsing::Rule
{
public:
    FunctionGroupIdRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("FunctionGroupIdNode*");
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<FunctionGroupIdNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<FunctionGroupIdRule>(this, &FunctionGroupIdRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<FunctionGroupIdRule>(this, &FunctionGroupIdRule::A1Action));
        Cm::Parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("identifier");
        identifierNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<FunctionGroupIdRule>(this, &FunctionGroupIdRule::Postidentifier));
        Cm::Parsing::NonterminalParser* operatorFunctionGroupIdNonterminalParser = GetNonterminal("OperatorFunctionGroupId");
        operatorFunctionGroupIdNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<FunctionGroupIdRule>(this, &FunctionGroupIdRule::PreOperatorFunctionGroupId));
        operatorFunctionGroupIdNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<FunctionGroupIdRule>(this, &FunctionGroupIdRule::PostOperatorFunctionGroupId));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, context.fromidentifier);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromOperatorFunctionGroupId;
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
    void PreOperatorFunctionGroupId(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostOperatorFunctionGroupId(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromOperatorFunctionGroupId_value = std::move(stack.top());
            context.fromOperatorFunctionGroupId = *static_cast<Cm::Parsing::ValueObject<FunctionGroupIdNode*>*>(fromOperatorFunctionGroupId_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromidentifier(), fromOperatorFunctionGroupId() {}
        ParsingContext* ctx;
        FunctionGroupIdNode* value;
        std::string fromidentifier;
        FunctionGroupIdNode* fromOperatorFunctionGroupId;
    };
    std::stack<Context> contextStack;
    Context context;
};

class FunctionGrammar::OperatorFunctionGroupIdRule : public Cm::Parsing::Rule
{
public:
    OperatorFunctionGroupIdRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("FunctionGroupIdNode*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "typeExpr"));
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<FunctionGroupIdNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A3Action));
        Cm::Parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A4Action));
        Cm::Parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A5Action));
        Cm::Parsing::ActionParser* a6ActionParser = GetAction("A6");
        a6ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A6Action));
        Cm::Parsing::ActionParser* a7ActionParser = GetAction("A7");
        a7ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A7Action));
        Cm::Parsing::ActionParser* a8ActionParser = GetAction("A8");
        a8ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A8Action));
        Cm::Parsing::ActionParser* a9ActionParser = GetAction("A9");
        a9ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A9Action));
        Cm::Parsing::ActionParser* a10ActionParser = GetAction("A10");
        a10ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A10Action));
        Cm::Parsing::ActionParser* a11ActionParser = GetAction("A11");
        a11ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::A11Action));
        Cm::Parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<OperatorFunctionGroupIdRule>(this, &OperatorFunctionGroupIdRule::PostTypeExpr));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, "operator<<");
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.typeExpr.reset(context.fromTypeExpr);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, "operator>>");
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, "operator==");
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, "operator<");
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, "operator+");
    }
    void A6Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, "operator-");
    }
    void A7Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, "operator*");
    }
    void A8Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, "operator/");
    }
    void A9Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, "operator^");
    }
    void A10Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, "operator!");
    }
    void A11Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FunctionGroupIdNode(span, "operator()");
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
private:
    struct Context
    {
        Context(): ctx(), value(), typeExpr(), fromTypeExpr() {}
        ParsingContext* ctx;
        FunctionGroupIdNode* value;
        std::unique_ptr<Node> typeExpr;
        Node* fromTypeExpr;
    };
    std::stack<Context> contextStack;
    Context context;
};

void FunctionGrammar::GetReferencedGrammars()
{
    Cm::Parsing::ParsingDomain* pd = GetParsingDomain();
    Cm::Parsing::Grammar* grammar0 = pd->GetGrammar("cminor.parser.SpecifierGrammar");
    if (!grammar0)
    {
        grammar0 = cminor::parser::SpecifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    Cm::Parsing::Grammar* grammar1 = pd->GetGrammar("cminor.parser.ParameterGrammar");
    if (!grammar1)
    {
        grammar1 = cminor::parser::ParameterGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    Cm::Parsing::Grammar* grammar2 = pd->GetGrammar("Cm.Parsing.stdlib");
    if (!grammar2)
    {
        grammar2 = Cm::Parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar2);
    Cm::Parsing::Grammar* grammar3 = pd->GetGrammar("cminor.parser.TypeExprGrammar");
    if (!grammar3)
    {
        grammar3 = cminor::parser::TypeExprGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
    Cm::Parsing::Grammar* grammar4 = pd->GetGrammar("cminor.parser.KeywordGrammar");
    if (!grammar4)
    {
        grammar4 = cminor::parser::KeywordGrammar::Create(pd);
    }
    AddGrammarReference(grammar4);
    Cm::Parsing::Grammar* grammar5 = pd->GetGrammar("cminor.parser.StatementGrammar");
    if (!grammar5)
    {
        grammar5 = cminor::parser::StatementGrammar::Create(pd);
    }
    AddGrammarReference(grammar5);
}

void FunctionGrammar::CreateRules()
{
    AddRuleLink(new Cm::Parsing::RuleLink("Specifiers", this, "SpecifierGrammar.Specifiers"));
    AddRuleLink(new Cm::Parsing::RuleLink("identifier", this, "Cm.Parsing.stdlib.identifier"));
    AddRuleLink(new Cm::Parsing::RuleLink("ParameterList", this, "ParameterGrammar.ParameterList"));
    AddRuleLink(new Cm::Parsing::RuleLink("TypeExpr", this, "TypeExprGrammar.TypeExpr"));
    AddRuleLink(new Cm::Parsing::RuleLink("Keyword", this, "KeywordGrammar.Keyword"));
    AddRuleLink(new Cm::Parsing::RuleLink("CompoundStatement", this, "StatementGrammar.CompoundStatement"));
    AddRule(new FunctionRule("Function", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::NonterminalParser("Specifiers", "Specifiers", 0),
                            new Cm::Parsing::NonterminalParser("TypeExpr", "TypeExpr", 1)),
                        new Cm::Parsing::ActionParser("A1",
                            new Cm::Parsing::NonterminalParser("FunctionGroupId", "FunctionGroupId", 1))),
                    new Cm::Parsing::ActionParser("A2",
                        new Cm::Parsing::NonterminalParser("ParameterList", "ParameterList", 2))),
                new Cm::Parsing::NonterminalParser("CompoundStatement", "CompoundStatement", 1)))));
    AddRule(new FunctionGroupIdRule("FunctionGroupId", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::ActionParser("A0",
                new Cm::Parsing::DifferenceParser(
                    new Cm::Parsing::NonterminalParser("identifier", "identifier", 0),
                    new Cm::Parsing::NonterminalParser("Keyword", "Keyword", 0))),
            new Cm::Parsing::ActionParser("A1",
                new Cm::Parsing::NonterminalParser("OperatorFunctionGroupId", "OperatorFunctionGroupId", 1)))));
    AddRule(new OperatorFunctionGroupIdRule("OperatorFunctionGroupId", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::KeywordParser("operator"),
            new Cm::Parsing::AlternativeParser(
                new Cm::Parsing::AlternativeParser(
                    new Cm::Parsing::AlternativeParser(
                        new Cm::Parsing::AlternativeParser(
                            new Cm::Parsing::AlternativeParser(
                                new Cm::Parsing::AlternativeParser(
                                    new Cm::Parsing::AlternativeParser(
                                        new Cm::Parsing::AlternativeParser(
                                            new Cm::Parsing::AlternativeParser(
                                                new Cm::Parsing::AlternativeParser(
                                                    new Cm::Parsing::ActionParser("A0",
                                                        new Cm::Parsing::DifferenceParser(
                                                            new Cm::Parsing::StringParser("<<"),
                                                            new Cm::Parsing::SequenceParser(
                                                                new Cm::Parsing::SequenceParser(
                                                                    new Cm::Parsing::SequenceParser(
                                                                        new Cm::Parsing::CharParser('<'),
                                                                        new Cm::Parsing::CharParser('<')),
                                                                    new Cm::Parsing::ListParser(
                                                                        new Cm::Parsing::ActionParser("A1",
                                                                            new Cm::Parsing::NonterminalParser("TypeExpr", "TypeExpr", 1)),
                                                                        new Cm::Parsing::CharParser(','))),
                                                                new Cm::Parsing::CharParser('>')))),
                                                    new Cm::Parsing::ActionParser("A2",
                                                        new Cm::Parsing::StringParser(">>"))),
                                                new Cm::Parsing::ActionParser("A3",
                                                    new Cm::Parsing::StringParser("=="))),
                                            new Cm::Parsing::ActionParser("A4",
                                                new Cm::Parsing::CharParser('<'))),
                                        new Cm::Parsing::ActionParser("A5",
                                            new Cm::Parsing::CharParser('+'))),
                                    new Cm::Parsing::ActionParser("A6",
                                        new Cm::Parsing::CharParser('-'))),
                                new Cm::Parsing::ActionParser("A7",
                                    new Cm::Parsing::CharParser('*'))),
                            new Cm::Parsing::ActionParser("A8",
                                new Cm::Parsing::CharParser('/'))),
                        new Cm::Parsing::ActionParser("A9",
                            new Cm::Parsing::CharParser('^'))),
                    new Cm::Parsing::ActionParser("A10",
                        new Cm::Parsing::CharParser('!'))),
                new Cm::Parsing::ActionParser("A11",
                    new Cm::Parsing::StringParser("()"))))));
}

} } // namespace cminor.parser
