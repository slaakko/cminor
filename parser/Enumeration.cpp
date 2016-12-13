#include "Enumeration.hpp"
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
#include <cminor/parser/Expression.hpp>

namespace cminor { namespace parser {

using namespace cminor::parsing;

EnumerationGrammar* EnumerationGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

EnumerationGrammar* EnumerationGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    EnumerationGrammar* grammar(new EnumerationGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

EnumerationGrammar::EnumerationGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("EnumerationGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

EnumTypeNode* EnumerationGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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
    stack.push(std::unique_ptr<cminor::parsing::Object>(new ValueObject<ParsingContext*>(ctx)));
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
    std::unique_ptr<cminor::parsing::Object> value = std::move(stack.top());
    EnumTypeNode* result = *static_cast<cminor::parsing::ValueObject<EnumTypeNode*>*>(value.get());
    stack.pop();
    return result;
}

class EnumerationGrammar::EnumTypeRule : public cminor::parsing::Rule
{
public:
    EnumTypeRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("EnumTypeNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<EnumTypeNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EnumTypeRule>(this, &EnumTypeRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EnumTypeRule>(this, &EnumTypeRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EnumTypeRule>(this, &EnumTypeRule::A2Action));
        cminor::parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal("Specifiers");
        specifiersNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<EnumTypeRule>(this, &EnumTypeRule::PostSpecifiers));
        cminor::parsing::NonterminalParser* enumTypeIdNonterminalParser = GetNonterminal("enumTypeId");
        enumTypeIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<EnumTypeRule>(this, &EnumTypeRule::PostenumTypeId));
        cminor::parsing::NonterminalParser* underlyingTypeNonterminalParser = GetNonterminal("UnderlyingType");
        underlyingTypeNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<EnumTypeRule>(this, &EnumTypeRule::PreUnderlyingType));
        underlyingTypeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<EnumTypeRule>(this, &EnumTypeRule::PostUnderlyingType));
        cminor::parsing::NonterminalParser* enumConstantsNonterminalParser = GetNonterminal("EnumConstants");
        enumConstantsNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<EnumTypeRule>(this, &EnumTypeRule::PreEnumConstants));
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
    void PostSpecifiers(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context.fromSpecifiers = *static_cast<cminor::parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
    }
    void PostenumTypeId(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromenumTypeId_value = std::move(stack.top());
            context.fromenumTypeId = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromenumTypeId_value.get());
            stack.pop();
        }
    }
    void PreUnderlyingType(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostUnderlyingType(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromUnderlyingType_value = std::move(stack.top());
            context.fromUnderlyingType = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromUnderlyingType_value.get());
            stack.pop();
        }
    }
    void PreEnumConstants(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<EnumTypeNode*>(context.value)));
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

class EnumerationGrammar::UnderlyingTypeRule : public cminor::parsing::Rule
{
public:
    UnderlyingTypeRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnderlyingTypeRule>(this, &UnderlyingTypeRule::A0Action));
        cminor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<UnderlyingTypeRule>(this, &UnderlyingTypeRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UnderlyingTypeRule>(this, &UnderlyingTypeRule::PostTypeExpr));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromTypeExpr;
    }
    void PreTypeExpr(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostTypeExpr(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context.fromTypeExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
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

class EnumerationGrammar::EnumConstantsRule : public cminor::parsing::Rule
{
public:
    EnumConstantsRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("EnumTypeNode*", "enumType"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> enumType_value = std::move(stack.top());
        context.enumType = *static_cast<cminor::parsing::ValueObject<EnumTypeNode*>*>(enumType_value.get());
        stack.pop();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EnumConstantsRule>(this, &EnumConstantsRule::A0Action));
        cminor::parsing::NonterminalParser* enumConstantNonterminalParser = GetNonterminal("EnumConstant");
        enumConstantNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<EnumConstantsRule>(this, &EnumConstantsRule::PreEnumConstant));
        enumConstantNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<EnumConstantsRule>(this, &EnumConstantsRule::PostEnumConstant));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.enumType->AddConstant(context.fromEnumConstant);
    }
    void PreEnumConstant(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<EnumTypeNode*>(context.enumType)));
    }
    void PostEnumConstant(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromEnumConstant_value = std::move(stack.top());
            context.fromEnumConstant = *static_cast<cminor::parsing::ValueObject<EnumConstantNode*>*>(fromEnumConstant_value.get());
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

class EnumerationGrammar::EnumConstantRule : public cminor::parsing::Rule
{
public:
    EnumConstantRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("EnumTypeNode*", "enumType"));
        SetValueTypeName("EnumConstantNode*");
        AddLocalVariable(AttrOrVariable("Span", "s"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> enumType_value = std::move(stack.top());
        context.enumType = *static_cast<cminor::parsing::ValueObject<EnumTypeNode*>*>(enumType_value.get());
        stack.pop();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<EnumConstantNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EnumConstantRule>(this, &EnumConstantRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EnumConstantRule>(this, &EnumConstantRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EnumConstantRule>(this, &EnumConstantRule::A2Action));
        cminor::parsing::NonterminalParser* constantIdNonterminalParser = GetNonterminal("constantId");
        constantIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<EnumConstantRule>(this, &EnumConstantRule::PostconstantId));
        cminor::parsing::NonterminalParser* constantValueNonterminalParser = GetNonterminal("constantValue");
        constantValueNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<EnumConstantRule>(this, &EnumConstantRule::PreconstantValue));
        constantValueNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<EnumConstantRule>(this, &EnumConstantRule::PostconstantValue));
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
    void PostconstantId(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromconstantId_value = std::move(stack.top());
            context.fromconstantId = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromconstantId_value.get());
            stack.pop();
        }
    }
    void PreconstantValue(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostconstantValue(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromconstantValue_value = std::move(stack.top());
            context.fromconstantValue = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromconstantValue_value.get());
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
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("cminor.parser.SpecifierGrammar");
    if (!grammar0)
    {
        grammar0 = cminor::parser::SpecifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar("cminor.parser.IdentifierGrammar");
    if (!grammar1)
    {
        grammar1 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar("cminor.parser.TypeExprGrammar");
    if (!grammar2)
    {
        grammar2 = cminor::parser::TypeExprGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar("cminor.parser.ExpressionGrammar");
    if (!grammar3)
    {
        grammar3 = cminor::parser::ExpressionGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
}

void EnumerationGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("Specifiers", this, "SpecifierGrammar.Specifiers"));
    AddRuleLink(new cminor::parsing::RuleLink("Identifier", this, "IdentifierGrammar.Identifier"));
    AddRuleLink(new cminor::parsing::RuleLink("TypeExpr", this, "TypeExprGrammar.TypeExpr"));
    AddRuleLink(new cminor::parsing::RuleLink("Expression", this, "ExpressionGrammar.Expression"));
    AddRule(new EnumTypeRule("EnumType", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::NonterminalParser("Specifiers", "Specifiers", 0),
                                new cminor::parsing::KeywordParser("enum")),
                            new cminor::parsing::ActionParser("A0",
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::NonterminalParser("enumTypeId", "Identifier", 0)))),
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::ActionParser("A1",
                                new cminor::parsing::NonterminalParser("UnderlyingType", "UnderlyingType", 1)))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('{'))),
                new cminor::parsing::NonterminalParser("EnumConstants", "EnumConstants", 2)),
            new cminor::parsing::ActionParser("A2",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser('}'))))));
    AddRule(new UnderlyingTypeRule("UnderlyingType", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::CharParser(':'),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("TypeExpr", "TypeExpr", 1)))));
    AddRule(new EnumConstantsRule("EnumConstants", GetScope(),
        new cminor::parsing::ListParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("EnumConstant", "EnumConstant", 2))),
            new cminor::parsing::CharParser(','))));
    AddRule(new EnumConstantRule("EnumConstant", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("constantId", "Identifier", 0))),
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('='),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::NonterminalParser("constantValue", "Expression", 1))),
                new cminor::parsing::ActionParser("A2",
                    new cminor::parsing::EmptyParser())))));
}

} } // namespace cminor.parser
