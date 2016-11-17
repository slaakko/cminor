#include "Enumeration.hpp"
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
#include <cminor/parser/Expression.hpp>

namespace cminor { namespace parser {

using namespace Cm::Parsing;

EnumerationGrammar* EnumerationGrammar::Create()
{
    return Create(new Cm::Parsing::ParsingDomain());
}

EnumerationGrammar* EnumerationGrammar::Create(Cm::Parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    EnumerationGrammar* grammar(new EnumerationGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

EnumerationGrammar::EnumerationGrammar(Cm::Parsing::ParsingDomain* parsingDomain_): Cm::Parsing::Grammar("EnumerationGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

EnumTypeNode* EnumerationGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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
    EnumTypeNode* result = *static_cast<Cm::Parsing::ValueObject<EnumTypeNode*>*>(value.get());
    stack.pop();
    return result;
}

class EnumerationGrammar::EnumTypeRule : public Cm::Parsing::Rule
{
public:
    EnumTypeRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("EnumTypeNode*");
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<EnumTypeNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<EnumTypeRule>(this, &EnumTypeRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<EnumTypeRule>(this, &EnumTypeRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<EnumTypeRule>(this, &EnumTypeRule::A2Action));
        Cm::Parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal("Specifiers");
        specifiersNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<EnumTypeRule>(this, &EnumTypeRule::PostSpecifiers));
        Cm::Parsing::NonterminalParser* enumTypeIdNonterminalParser = GetNonterminal("enumTypeId");
        enumTypeIdNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<EnumTypeRule>(this, &EnumTypeRule::PostenumTypeId));
        Cm::Parsing::NonterminalParser* underlyingTypeNonterminalParser = GetNonterminal("UnderlyingType");
        underlyingTypeNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<EnumTypeRule>(this, &EnumTypeRule::PreUnderlyingType));
        underlyingTypeNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<EnumTypeRule>(this, &EnumTypeRule::PostUnderlyingType));
        Cm::Parsing::NonterminalParser* enumConstantsNonterminalParser = GetNonterminal("EnumConstants");
        enumConstantsNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<EnumTypeRule>(this, &EnumTypeRule::PreEnumConstants));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new EnumTypeNode(span, context.fromSpecifiers, context.fromenumTypeId);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value->SetUnderlyingType(context.fromUnderlyingType);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value->GetSpan().SetEnd(span.End());
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
    void PostenumTypeId(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromenumTypeId_value = std::move(stack.top());
            context.fromenumTypeId = *static_cast<Cm::Parsing::ValueObject<IdentifierNode*>*>(fromenumTypeId_value.get());
            stack.pop();
        }
    }
    void PreUnderlyingType(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostUnderlyingType(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromUnderlyingType_value = std::move(stack.top());
            context.fromUnderlyingType = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromUnderlyingType_value.get());
            stack.pop();
        }
    }
    void PreEnumConstants(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<EnumTypeNode*>(context.value)));
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromSpecifiers(), fromenumTypeId(), fromUnderlyingType() {}
        ParsingContext* ctx;
        EnumTypeNode* value;
        Specifiers fromSpecifiers;
        IdentifierNode* fromenumTypeId;
        Node* fromUnderlyingType;
    };
    std::stack<Context> contextStack;
    Context context;
};

class EnumerationGrammar::UnderlyingTypeRule : public Cm::Parsing::Rule
{
public:
    UnderlyingTypeRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<UnderlyingTypeRule>(this, &UnderlyingTypeRule::A0Action));
        Cm::Parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<UnderlyingTypeRule>(this, &UnderlyingTypeRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<UnderlyingTypeRule>(this, &UnderlyingTypeRule::PostTypeExpr));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromTypeExpr;
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
        Context(): ctx(), value(), fromTypeExpr() {}
        ParsingContext* ctx;
        Node* value;
        Node* fromTypeExpr;
    };
    std::stack<Context> contextStack;
    Context context;
};

class EnumerationGrammar::EnumConstantsRule : public Cm::Parsing::Rule
{
public:
    EnumConstantsRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("EnumTypeNode*", "enumType"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> enumType_value = std::move(stack.top());
        context.enumType = *static_cast<Cm::Parsing::ValueObject<EnumTypeNode*>*>(enumType_value.get());
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<EnumConstantsRule>(this, &EnumConstantsRule::A0Action));
        Cm::Parsing::NonterminalParser* enumConstantNonterminalParser = GetNonterminal("EnumConstant");
        enumConstantNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<EnumConstantsRule>(this, &EnumConstantsRule::PreEnumConstant));
        enumConstantNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<EnumConstantsRule>(this, &EnumConstantsRule::PostEnumConstant));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.enumType->AddConstant(context.fromEnumConstant);
    }
    void PreEnumConstant(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<EnumTypeNode*>(context.enumType)));
    }
    void PostEnumConstant(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromEnumConstant_value = std::move(stack.top());
            context.fromEnumConstant = *static_cast<Cm::Parsing::ValueObject<EnumConstantNode*>*>(fromEnumConstant_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), enumType(), fromEnumConstant() {}
        ParsingContext* ctx;
        EnumTypeNode* enumType;
        EnumConstantNode* fromEnumConstant;
    };
    std::stack<Context> contextStack;
    Context context;
};

class EnumerationGrammar::EnumConstantRule : public Cm::Parsing::Rule
{
public:
    EnumConstantRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("EnumTypeNode*", "enumType"));
        SetValueTypeName("EnumConstantNode*");
        AddLocalVariable(AttrOrVariable("Span", "s"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> enumType_value = std::move(stack.top());
        context.enumType = *static_cast<Cm::Parsing::ValueObject<EnumTypeNode*>*>(enumType_value.get());
        stack.pop();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<EnumConstantNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<EnumConstantRule>(this, &EnumConstantRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<EnumConstantRule>(this, &EnumConstantRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<EnumConstantRule>(this, &EnumConstantRule::A2Action));
        Cm::Parsing::NonterminalParser* constantIdNonterminalParser = GetNonterminal("constantId");
        constantIdNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<EnumConstantRule>(this, &EnumConstantRule::PostconstantId));
        Cm::Parsing::NonterminalParser* constantValueNonterminalParser = GetNonterminal("constantValue");
        constantValueNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<EnumConstantRule>(this, &EnumConstantRule::PreconstantValue));
        constantValueNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<EnumConstantRule>(this, &EnumConstantRule::PostconstantValue));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s = span;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s.SetEnd(span.End());
        context.value = new EnumConstantNode(context.s, context.fromconstantId, context.fromconstantValue);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.s.SetEnd(span.End());
        context.value = new EnumConstantNode(context.s, context.fromconstantId, MakeNextEnumConstantValue(context.s, context.enumType));
    }
    void PostconstantId(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromconstantId_value = std::move(stack.top());
            context.fromconstantId = *static_cast<Cm::Parsing::ValueObject<IdentifierNode*>*>(fromconstantId_value.get());
            stack.pop();
        }
    }
    void PreconstantValue(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostconstantValue(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromconstantValue_value = std::move(stack.top());
            context.fromconstantValue = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromconstantValue_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), enumType(), value(), s(), fromconstantId(), fromconstantValue() {}
        ParsingContext* ctx;
        EnumTypeNode* enumType;
        EnumConstantNode* value;
        Span s;
        IdentifierNode* fromconstantId;
        Node* fromconstantValue;
    };
    std::stack<Context> contextStack;
    Context context;
};

void EnumerationGrammar::GetReferencedGrammars()
{
    Cm::Parsing::ParsingDomain* pd = GetParsingDomain();
    Cm::Parsing::Grammar* grammar0 = pd->GetGrammar("cminor.parser.IdentifierGrammar");
    if (!grammar0)
    {
        grammar0 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    Cm::Parsing::Grammar* grammar1 = pd->GetGrammar("cminor.parser.SpecifierGrammar");
    if (!grammar1)
    {
        grammar1 = cminor::parser::SpecifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    Cm::Parsing::Grammar* grammar2 = pd->GetGrammar("cminor.parser.TypeExprGrammar");
    if (!grammar2)
    {
        grammar2 = cminor::parser::TypeExprGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    Cm::Parsing::Grammar* grammar3 = pd->GetGrammar("cminor.parser.ExpressionGrammar");
    if (!grammar3)
    {
        grammar3 = cminor::parser::ExpressionGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
}

void EnumerationGrammar::CreateRules()
{
    AddRuleLink(new Cm::Parsing::RuleLink("Identifier", this, "IdentifierGrammar.Identifier"));
    AddRuleLink(new Cm::Parsing::RuleLink("Specifiers", this, "SpecifierGrammar.Specifiers"));
    AddRuleLink(new Cm::Parsing::RuleLink("TypeExpr", this, "TypeExprGrammar.TypeExpr"));
    AddRuleLink(new Cm::Parsing::RuleLink("Expression", this, "ExpressionGrammar.Expression"));
    AddRule(new EnumTypeRule("EnumType", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::SequenceParser(
                                new Cm::Parsing::NonterminalParser("Specifiers", "Specifiers", 0),
                                new Cm::Parsing::KeywordParser("enum")),
                            new Cm::Parsing::ActionParser("A0",
                                new Cm::Parsing::ExpectationParser(
                                    new Cm::Parsing::NonterminalParser("enumTypeId", "Identifier", 0)))),
                        new Cm::Parsing::OptionalParser(
                            new Cm::Parsing::ActionParser("A1",
                                new Cm::Parsing::NonterminalParser("UnderlyingType", "UnderlyingType", 1)))),
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::CharParser('{'))),
                new Cm::Parsing::NonterminalParser("EnumConstants", "EnumConstants", 2)),
            new Cm::Parsing::ActionParser("A2",
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::CharParser('}'))))));
    AddRule(new UnderlyingTypeRule("UnderlyingType", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::CharParser(':'),
            new Cm::Parsing::ActionParser("A0",
                new Cm::Parsing::NonterminalParser("TypeExpr", "TypeExpr", 1)))));
    AddRule(new EnumConstantsRule("EnumConstants", GetScope(),
        new Cm::Parsing::ListParser(
            new Cm::Parsing::ActionParser("A0",
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::NonterminalParser("EnumConstant", "EnumConstant", 2))),
            new Cm::Parsing::CharParser(','))));
    AddRule(new EnumConstantRule("EnumConstant", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::ActionParser("A0",
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::NonterminalParser("constantId", "Identifier", 0))),
            new Cm::Parsing::AlternativeParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::CharParser('='),
                    new Cm::Parsing::ActionParser("A1",
                        new Cm::Parsing::NonterminalParser("constantValue", "Expression", 1))),
                new Cm::Parsing::ActionParser("A2",
                    new Cm::Parsing::EmptyParser())))));
}

} } // namespace cminor.parser
