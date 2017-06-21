#include "Enumeration.hpp"
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
#include <cminor/parser/Expression.hpp>

namespace cminor { namespace parser {

using namespace cminor::parsing;
using namespace cminor::util;
using namespace cminor::unicode;

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

EnumerationGrammar::EnumerationGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("EnumerationGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.parser")), parsingDomain_)
{
    SetOwner(0);
}

EnumTypeNode* EnumerationGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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
    EnumTypeNode* result = *static_cast<cminor::parsing::ValueObject<EnumTypeNode*>*>(value.get());
    stack.pop();
    return result;
}

class EnumerationGrammar::EnumTypeRule : public cminor::parsing::Rule
{
public:
    EnumTypeRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("EnumTypeNode*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<EnumTypeNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EnumTypeRule>(this, &EnumTypeRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EnumTypeRule>(this, &EnumTypeRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EnumTypeRule>(this, &EnumTypeRule::A2Action));
        cminor::parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal(ToUtf32("Specifiers"));
        specifiersNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<EnumTypeRule>(this, &EnumTypeRule::PostSpecifiers));
        cminor::parsing::NonterminalParser* enumTypeIdNonterminalParser = GetNonterminal(ToUtf32("enumTypeId"));
        enumTypeIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<EnumTypeRule>(this, &EnumTypeRule::PostenumTypeId));
        cminor::parsing::NonterminalParser* underlyingTypeNonterminalParser = GetNonterminal(ToUtf32("UnderlyingType"));
        underlyingTypeNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<EnumTypeRule>(this, &EnumTypeRule::PreUnderlyingType));
        underlyingTypeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<EnumTypeRule>(this, &EnumTypeRule::PostUnderlyingType));
        cminor::parsing::NonterminalParser* enumConstantsNonterminalParser = GetNonterminal(ToUtf32("EnumConstants"));
        enumConstantsNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<EnumTypeRule>(this, &EnumTypeRule::PreEnumConstants));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new EnumTypeNode(span, context->fromSpecifiers, context->fromenumTypeId);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value->SetUnderlyingType(context->fromUnderlyingType);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value->GetSpan().SetEnd(span.End());
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
    void PostenumTypeId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromenumTypeId_value = std::move(stack.top());
            context->fromenumTypeId = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromenumTypeId_value.get());
            stack.pop();
        }
    }
    void PreUnderlyingType(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostUnderlyingType(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromUnderlyingType_value = std::move(stack.top());
            context->fromUnderlyingType = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromUnderlyingType_value.get());
            stack.pop();
        }
    }
    void PreEnumConstants(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<EnumTypeNode*>(context->value)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), fromSpecifiers(), fromenumTypeId(), fromUnderlyingType() {}
        ParsingContext* ctx;
        EnumTypeNode* value;
        Specifiers fromSpecifiers;
        IdentifierNode* fromenumTypeId;
        Node* fromUnderlyingType;
    };
};

class EnumerationGrammar::UnderlyingTypeRule : public cminor::parsing::Rule
{
public:
    UnderlyingTypeRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("Node*"));
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnderlyingTypeRule>(this, &UnderlyingTypeRule::A0Action));
        cminor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal(ToUtf32("TypeExpr"));
        typeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<UnderlyingTypeRule>(this, &UnderlyingTypeRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UnderlyingTypeRule>(this, &UnderlyingTypeRule::PostTypeExpr));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromTypeExpr;
    }
    void PreTypeExpr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostTypeExpr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context->fromTypeExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), fromTypeExpr() {}
        ParsingContext* ctx;
        Node* value;
        Node* fromTypeExpr;
    };
};

class EnumerationGrammar::EnumConstantsRule : public cminor::parsing::Rule
{
public:
    EnumConstantsRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        AddInheritedAttribute(AttrOrVariable(ToUtf32("EnumTypeNode*"), ToUtf32("enumType")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> enumType_value = std::move(stack.top());
        context->enumType = *static_cast<cminor::parsing::ValueObject<EnumTypeNode*>*>(enumType_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EnumConstantsRule>(this, &EnumConstantsRule::A0Action));
        cminor::parsing::NonterminalParser* enumConstantNonterminalParser = GetNonterminal(ToUtf32("EnumConstant"));
        enumConstantNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<EnumConstantsRule>(this, &EnumConstantsRule::PreEnumConstant));
        enumConstantNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<EnumConstantsRule>(this, &EnumConstantsRule::PostEnumConstant));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->enumType->AddConstant(context->fromEnumConstant);
    }
    void PreEnumConstant(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<EnumTypeNode*>(context->enumType)));
    }
    void PostEnumConstant(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromEnumConstant_value = std::move(stack.top());
            context->fromEnumConstant = *static_cast<cminor::parsing::ValueObject<EnumConstantNode*>*>(fromEnumConstant_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), enumType(), fromEnumConstant() {}
        ParsingContext* ctx;
        EnumTypeNode* enumType;
        EnumConstantNode* fromEnumConstant;
    };
};

class EnumerationGrammar::EnumConstantRule : public cminor::parsing::Rule
{
public:
    EnumConstantRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        AddInheritedAttribute(AttrOrVariable(ToUtf32("EnumTypeNode*"), ToUtf32("enumType")));
        SetValueTypeName(ToUtf32("EnumConstantNode*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("Span"), ToUtf32("s")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> enumType_value = std::move(stack.top());
        context->enumType = *static_cast<cminor::parsing::ValueObject<EnumTypeNode*>*>(enumType_value.get());
        stack.pop();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context->ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<EnumConstantNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EnumConstantRule>(this, &EnumConstantRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EnumConstantRule>(this, &EnumConstantRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EnumConstantRule>(this, &EnumConstantRule::A2Action));
        cminor::parsing::NonterminalParser* constantIdNonterminalParser = GetNonterminal(ToUtf32("constantId"));
        constantIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<EnumConstantRule>(this, &EnumConstantRule::PostconstantId));
        cminor::parsing::NonterminalParser* constantValueNonterminalParser = GetNonterminal(ToUtf32("constantValue"));
        constantValueNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<EnumConstantRule>(this, &EnumConstantRule::PreconstantValue));
        constantValueNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<EnumConstantRule>(this, &EnumConstantRule::PostconstantValue));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->s = span;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->s.SetEnd(span.End());
        context->value = new EnumConstantNode(context->s, context->fromconstantId, context->fromconstantValue);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->s.SetEnd(span.End());
        context->value = new EnumConstantNode(context->s, context->fromconstantId, MakeNextEnumConstantValue(context->s, context->enumType));
    }
    void PostconstantId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromconstantId_value = std::move(stack.top());
            context->fromconstantId = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromconstantId_value.get());
            stack.pop();
        }
    }
    void PreconstantValue(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostconstantValue(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromconstantValue_value = std::move(stack.top());
            context->fromconstantValue = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromconstantValue_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), enumType(), value(), s(), fromconstantId(), fromconstantValue() {}
        ParsingContext* ctx;
        EnumTypeNode* enumType;
        EnumConstantNode* value;
        Span s;
        IdentifierNode* fromconstantId;
        Node* fromconstantValue;
    };
};

void EnumerationGrammar::GetReferencedGrammars()
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
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar(ToUtf32("cminor.parser.IdentifierGrammar"));
    if (!grammar2)
    {
        grammar2 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar(ToUtf32("cminor.parser.ExpressionGrammar"));
    if (!grammar3)
    {
        grammar3 = cminor::parser::ExpressionGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
}

void EnumerationGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Specifiers"), this, ToUtf32("SpecifierGrammar.Specifiers")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("TypeExpr"), this, ToUtf32("TypeExprGrammar.TypeExpr")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Identifier"), this, ToUtf32("IdentifierGrammar.Identifier")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Expression"), this, ToUtf32("ExpressionGrammar.Expression")));
    AddRule(new EnumTypeRule(ToUtf32("EnumType"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("Specifiers"), ToUtf32("Specifiers"), 0),
                                new cminor::parsing::KeywordParser(ToUtf32("enum"))),
                            new cminor::parsing::ActionParser(ToUtf32("A0"),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::NonterminalParser(ToUtf32("enumTypeId"), ToUtf32("Identifier"), 0)))),
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::ActionParser(ToUtf32("A1"),
                                new cminor::parsing::NonterminalParser(ToUtf32("UnderlyingType"), ToUtf32("UnderlyingType"), 1)))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('{'))),
                new cminor::parsing::NonterminalParser(ToUtf32("EnumConstants"), ToUtf32("EnumConstants"), 2)),
            new cminor::parsing::ActionParser(ToUtf32("A2"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser('}'))))));
    AddRule(new UnderlyingTypeRule(ToUtf32("UnderlyingType"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::CharParser(':'),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("TypeExpr"), ToUtf32("TypeExpr"), 1)))));
    AddRule(new EnumConstantsRule(ToUtf32("EnumConstants"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ListParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("EnumConstant"), ToUtf32("EnumConstant"), 2))),
            new cminor::parsing::CharParser(','))));
    AddRule(new EnumConstantRule(ToUtf32("EnumConstant"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("constantId"), ToUtf32("Identifier"), 0))),
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('='),
                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                        new cminor::parsing::NonterminalParser(ToUtf32("constantValue"), ToUtf32("Expression"), 1))),
                new cminor::parsing::ActionParser(ToUtf32("A2"),
                    new cminor::parsing::EmptyParser())))));
}

} } // namespace cminor.parser
