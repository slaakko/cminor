#include "Declarator.hpp"
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
#include <cminor/code/Expression.hpp>
#include <cminor/code/Declaration.hpp>
#include <cminor/util/TextUtils.hpp>
#include <cminor/codedom/Type.hpp>

namespace cminor { namespace code {

using cminor::util::Trim;
using namespace cminor::codedom;
using namespace cminor::parsing;
using namespace cminor::util;
using namespace cminor::unicode;

DeclaratorGrammar* DeclaratorGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

DeclaratorGrammar* DeclaratorGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    DeclaratorGrammar* grammar(new DeclaratorGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

DeclaratorGrammar::DeclaratorGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("DeclaratorGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.code")), parsingDomain_)
{
    SetOwner(0);
}

cminor::codedom::InitDeclaratorList* DeclaratorGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName)
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
    cminor::codedom::InitDeclaratorList* result = *static_cast<cminor::parsing::ValueObject<cminor::codedom::InitDeclaratorList*>*>(value.get());
    stack.pop();
    return result;
}

class DeclaratorGrammar::InitDeclaratorListRule : public cminor::parsing::Rule
{
public:
    InitDeclaratorListRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::InitDeclaratorList*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<InitDeclaratorList>"), ToUtf32("idl")));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::InitDeclaratorList*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitDeclaratorListRule>(this, &InitDeclaratorListRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitDeclaratorListRule>(this, &InitDeclaratorListRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitDeclaratorListRule>(this, &InitDeclaratorListRule::A2Action));
        cminor::parsing::NonterminalParser* initDeclaratorNonterminalParser = GetNonterminal(ToUtf32("InitDeclarator"));
        initDeclaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InitDeclaratorListRule>(this, &InitDeclaratorListRule::PostInitDeclarator));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->idl.reset(new InitDeclaratorList);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->idl.release();
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->idl->Add(context->fromInitDeclarator);
    }
    void PostInitDeclarator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromInitDeclarator_value = std::move(stack.top());
            context->fromInitDeclarator = *static_cast<cminor::parsing::ValueObject<cminor::codedom::InitDeclarator*>*>(fromInitDeclarator_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), idl(), fromInitDeclarator() {}
        cminor::codedom::InitDeclaratorList* value;
        std::unique_ptr<InitDeclaratorList> idl;
        cminor::codedom::InitDeclarator* fromInitDeclarator;
    };
};

class DeclaratorGrammar::InitDeclaratorRule : public cminor::parsing::Rule
{
public:
    InitDeclaratorRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::InitDeclarator*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::InitDeclarator*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitDeclaratorRule>(this, &InitDeclaratorRule::A0Action));
        cminor::parsing::NonterminalParser* declaratorNonterminalParser = GetNonterminal(ToUtf32("Declarator"));
        declaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InitDeclaratorRule>(this, &InitDeclaratorRule::PostDeclarator));
        cminor::parsing::NonterminalParser* initializerNonterminalParser = GetNonterminal(ToUtf32("Initializer"));
        initializerNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InitDeclaratorRule>(this, &InitDeclaratorRule::PostInitializer));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new InitDeclarator(context->fromDeclarator, context->fromInitializer);
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
    void PostInitializer(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromInitializer_value = std::move(stack.top());
            context->fromInitializer = *static_cast<cminor::parsing::ValueObject<cminor::codedom::Initializer*>*>(fromInitializer_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromDeclarator(), fromInitializer() {}
        cminor::codedom::InitDeclarator* value;
        std::u32string fromDeclarator;
        cminor::codedom::Initializer* fromInitializer;
    };
};

class DeclaratorGrammar::DeclaratorRule : public cminor::parsing::Rule
{
public:
    DeclaratorRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeclaratorRule>(this, &DeclaratorRule::A0Action));
        cminor::parsing::NonterminalParser* directDeclaratorNonterminalParser = GetNonterminal(ToUtf32("DirectDeclarator"));
        cminor::parsing::NonterminalParser* declaratorNonterminalParser = GetNonterminal(ToUtf32("Declarator"));
        declaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeclaratorRule>(this, &DeclaratorRule::PostDeclarator));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Trim(std::u32string(matchBegin, matchEnd));
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
        Context(): value(), fromDeclarator() {}
        std::u32string value;
        std::u32string fromDeclarator;
    };
};

class DeclaratorGrammar::DirectDeclaratorRule : public cminor::parsing::Rule
{
public:
    DirectDeclaratorRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<CppObject>"), ToUtf32("o")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DirectDeclaratorRule>(this, &DirectDeclaratorRule::A0Action));
        cminor::parsing::NonterminalParser* declaratorIdNonterminalParser = GetNonterminal(ToUtf32("DeclaratorId"));
        cminor::parsing::NonterminalParser* constantExpressionNonterminalParser = GetNonterminal(ToUtf32("ConstantExpression"));
        constantExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DirectDeclaratorRule>(this, &DirectDeclaratorRule::PostConstantExpression));
        cminor::parsing::NonterminalParser* declaratorNonterminalParser = GetNonterminal(ToUtf32("Declarator"));
        declaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DirectDeclaratorRule>(this, &DirectDeclaratorRule::PostDeclarator));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->o.reset(context->fromConstantExpression);
    }
    void PostConstantExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromConstantExpression_value = std::move(stack.top());
            context->fromConstantExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromConstantExpression_value.get());
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
        Context(): o(), fromConstantExpression(), fromDeclarator() {}
        std::unique_ptr<CppObject> o;
        cminor::codedom::CppObject* fromConstantExpression;
        std::u32string fromDeclarator;
    };
};

class DeclaratorGrammar::DeclaratorIdRule : public cminor::parsing::Rule
{
public:
    DeclaratorIdRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<CppObject>"), ToUtf32("o")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeclaratorIdRule>(this, &DeclaratorIdRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeclaratorIdRule>(this, &DeclaratorIdRule::A1Action));
        cminor::parsing::NonterminalParser* idExpressionNonterminalParser = GetNonterminal(ToUtf32("IdExpression"));
        idExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeclaratorIdRule>(this, &DeclaratorIdRule::PostIdExpression));
        cminor::parsing::NonterminalParser* typeNameNonterminalParser = GetNonterminal(ToUtf32("TypeName"));
        typeNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeclaratorIdRule>(this, &DeclaratorIdRule::PostTypeName));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->o.reset(context->fromIdExpression);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->o.reset(context->fromTypeName);
    }
    void PostIdExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIdExpression_value = std::move(stack.top());
            context->fromIdExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::IdExpr*>*>(fromIdExpression_value.get());
            stack.pop();
        }
    }
    void PostTypeName(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeName_value = std::move(stack.top());
            context->fromTypeName = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeName*>*>(fromTypeName_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): o(), fromIdExpression(), fromTypeName() {}
        std::unique_ptr<CppObject> o;
        cminor::codedom::IdExpr* fromIdExpression;
        cminor::codedom::TypeName* fromTypeName;
    };
};

class DeclaratorGrammar::TypeIdRule : public cminor::parsing::Rule
{
public:
    TypeIdRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::TypeId*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<TypeId>"), ToUtf32("ti")));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::TypeId*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeIdRule>(this, &TypeIdRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeIdRule>(this, &TypeIdRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeIdRule>(this, &TypeIdRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeIdRule>(this, &TypeIdRule::A3Action));
        cminor::parsing::NonterminalParser* typeSpecifierSeqNonterminalParser = GetNonterminal(ToUtf32("TypeSpecifierSeq"));
        typeSpecifierSeqNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<TypeIdRule>(this, &TypeIdRule::PreTypeSpecifierSeq));
        cminor::parsing::NonterminalParser* typeNameNonterminalParser = GetNonterminal(ToUtf32("TypeName"));
        typeNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TypeIdRule>(this, &TypeIdRule::PostTypeName));
        cminor::parsing::NonterminalParser* abstractDeclaratorNonterminalParser = GetNonterminal(ToUtf32("AbstractDeclarator"));
        abstractDeclaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TypeIdRule>(this, &TypeIdRule::PostAbstractDeclarator));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ti.reset(new TypeId);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->ti.release();
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ti->Add(context->fromTypeName);
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ti->Declarator() = context->fromAbstractDeclarator;
    }
    void PreTypeSpecifierSeq(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::TypeId*>(context->ti.get())));
    }
    void PostTypeName(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeName_value = std::move(stack.top());
            context->fromTypeName = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeName*>*>(fromTypeName_value.get());
            stack.pop();
        }
    }
    void PostAbstractDeclarator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAbstractDeclarator_value = std::move(stack.top());
            context->fromAbstractDeclarator = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromAbstractDeclarator_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), ti(), fromTypeName(), fromAbstractDeclarator() {}
        cminor::codedom::TypeId* value;
        std::unique_ptr<TypeId> ti;
        cminor::codedom::TypeName* fromTypeName;
        std::u32string fromAbstractDeclarator;
    };
};

class DeclaratorGrammar::TypeRule : public cminor::parsing::Rule
{
public:
    TypeRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Type*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<Type>"), ToUtf32("t")));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Type*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeRule>(this, &TypeRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeRule>(this, &TypeRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeRule>(this, &TypeRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeRule>(this, &TypeRule::A3Action));
        cminor::parsing::NonterminalParser* typeSpecifierNonterminalParser = GetNonterminal(ToUtf32("TypeSpecifier"));
        typeSpecifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TypeRule>(this, &TypeRule::PostTypeSpecifier));
        cminor::parsing::NonterminalParser* typeNameNonterminalParser = GetNonterminal(ToUtf32("TypeName"));
        typeNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TypeRule>(this, &TypeRule::PostTypeName));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->t.reset(new Type);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->t.release();
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->t->Add(context->fromTypeSpecifier);
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->t->Add(context->fromTypeName);
    }
    void PostTypeSpecifier(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeSpecifier_value = std::move(stack.top());
            context->fromTypeSpecifier = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeSpecifier*>*>(fromTypeSpecifier_value.get());
            stack.pop();
        }
    }
    void PostTypeName(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeName_value = std::move(stack.top());
            context->fromTypeName = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeName*>*>(fromTypeName_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), t(), fromTypeSpecifier(), fromTypeName() {}
        cminor::codedom::Type* value;
        std::unique_ptr<Type> t;
        cminor::codedom::TypeSpecifier* fromTypeSpecifier;
        cminor::codedom::TypeName* fromTypeName;
    };
};

class DeclaratorGrammar::TypeSpecifierSeqRule : public cminor::parsing::Rule
{
public:
    TypeSpecifierSeqRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::codedom::TypeId*"), ToUtf32("typeId")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> typeId_value = std::move(stack.top());
        context->typeId = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeId*>*>(typeId_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeSpecifierSeqRule>(this, &TypeSpecifierSeqRule::A0Action));
        cminor::parsing::NonterminalParser* typeSpecifierNonterminalParser = GetNonterminal(ToUtf32("TypeSpecifier"));
        typeSpecifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TypeSpecifierSeqRule>(this, &TypeSpecifierSeqRule::PostTypeSpecifier));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->typeId->Add(context->fromTypeSpecifier);
    }
    void PostTypeSpecifier(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeSpecifier_value = std::move(stack.top());
            context->fromTypeSpecifier = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeSpecifier*>*>(fromTypeSpecifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): typeId(), fromTypeSpecifier() {}
        cminor::codedom::TypeId* typeId;
        cminor::codedom::TypeSpecifier* fromTypeSpecifier;
    };
};

class DeclaratorGrammar::AbstractDeclaratorRule : public cminor::parsing::Rule
{
public:
    AbstractDeclaratorRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AbstractDeclaratorRule>(this, &AbstractDeclaratorRule::A0Action));
        cminor::parsing::NonterminalParser* abstractDeclaratorNonterminalParser = GetNonterminal(ToUtf32("AbstractDeclarator"));
        abstractDeclaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AbstractDeclaratorRule>(this, &AbstractDeclaratorRule::PostAbstractDeclarator));
        cminor::parsing::NonterminalParser* directAbstractDeclaratorNonterminalParser = GetNonterminal(ToUtf32("DirectAbstractDeclarator"));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Trim(std::u32string(matchBegin, matchEnd));
    }
    void PostAbstractDeclarator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAbstractDeclarator_value = std::move(stack.top());
            context->fromAbstractDeclarator = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromAbstractDeclarator_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromAbstractDeclarator() {}
        std::u32string value;
        std::u32string fromAbstractDeclarator;
    };
};

class DeclaratorGrammar::DirectAbstractDeclaratorRule : public cminor::parsing::Rule
{
public:
    DirectAbstractDeclaratorRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<CppObject>"), ToUtf32("o")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DirectAbstractDeclaratorRule>(this, &DirectAbstractDeclaratorRule::A0Action));
        cminor::parsing::NonterminalParser* constantExpressionNonterminalParser = GetNonterminal(ToUtf32("ConstantExpression"));
        constantExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DirectAbstractDeclaratorRule>(this, &DirectAbstractDeclaratorRule::PostConstantExpression));
        cminor::parsing::NonterminalParser* abstractDeclaratorNonterminalParser = GetNonterminal(ToUtf32("AbstractDeclarator"));
        abstractDeclaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DirectAbstractDeclaratorRule>(this, &DirectAbstractDeclaratorRule::PostAbstractDeclarator));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->o.reset(context->fromConstantExpression);
    }
    void PostConstantExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromConstantExpression_value = std::move(stack.top());
            context->fromConstantExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromConstantExpression_value.get());
            stack.pop();
        }
    }
    void PostAbstractDeclarator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAbstractDeclarator_value = std::move(stack.top());
            context->fromAbstractDeclarator = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromAbstractDeclarator_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): o(), fromConstantExpression(), fromAbstractDeclarator() {}
        std::unique_ptr<CppObject> o;
        cminor::codedom::CppObject* fromConstantExpression;
        std::u32string fromAbstractDeclarator;
    };
};

class DeclaratorGrammar::CVQualifierSeqRule : public cminor::parsing::Rule
{
public:
    CVQualifierSeqRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<CppObject>"), ToUtf32("o")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CVQualifierSeqRule>(this, &CVQualifierSeqRule::A0Action));
        cminor::parsing::NonterminalParser* cVQualifierNonterminalParser = GetNonterminal(ToUtf32("CVQualifier"));
        cVQualifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CVQualifierSeqRule>(this, &CVQualifierSeqRule::PostCVQualifier));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->o.reset(context->fromCVQualifier);
    }
    void PostCVQualifier(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCVQualifier_value = std::move(stack.top());
            context->fromCVQualifier = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeSpecifier*>*>(fromCVQualifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): o(), fromCVQualifier() {}
        std::unique_ptr<CppObject> o;
        cminor::codedom::TypeSpecifier* fromCVQualifier;
    };
};

class DeclaratorGrammar::InitializerRule : public cminor::parsing::Rule
{
public:
    InitializerRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::Initializer*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::Initializer*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitializerRule>(this, &InitializerRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitializerRule>(this, &InitializerRule::A1Action));
        cminor::parsing::NonterminalParser* initializerClauseNonterminalParser = GetNonterminal(ToUtf32("InitializerClause"));
        initializerClauseNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InitializerRule>(this, &InitializerRule::PostInitializerClause));
        cminor::parsing::NonterminalParser* expressionListNonterminalParser = GetNonterminal(ToUtf32("ExpressionList"));
        expressionListNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InitializerRule>(this, &InitializerRule::PostExpressionList));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Initializer(context->fromInitializerClause, GetEmptyVector());
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Initializer(nullptr, context->fromExpressionList);
    }
    void PostInitializerClause(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromInitializerClause_value = std::move(stack.top());
            context->fromInitializerClause = *static_cast<cminor::parsing::ValueObject<cminor::codedom::AssignInit*>*>(fromInitializerClause_value.get());
            stack.pop();
        }
    }
    void PostExpressionList(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpressionList_value = std::move(stack.top());
            context->fromExpressionList = *static_cast<cminor::parsing::ValueObject<std::vector<cminor::codedom::CppObject*>>*>(fromExpressionList_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromInitializerClause(), fromExpressionList() {}
        cminor::codedom::Initializer* value;
        cminor::codedom::AssignInit* fromInitializerClause;
        std::vector<cminor::codedom::CppObject*> fromExpressionList;
    };
};

class DeclaratorGrammar::InitializerClauseRule : public cminor::parsing::Rule
{
public:
    InitializerClauseRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::AssignInit*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::AssignInit*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitializerClauseRule>(this, &InitializerClauseRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitializerClauseRule>(this, &InitializerClauseRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitializerClauseRule>(this, &InitializerClauseRule::A2Action));
        cminor::parsing::NonterminalParser* assignmentExpressionNonterminalParser = GetNonterminal(ToUtf32("AssignmentExpression"));
        assignmentExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InitializerClauseRule>(this, &InitializerClauseRule::PostAssignmentExpression));
        cminor::parsing::NonterminalParser* initializerListNonterminalParser = GetNonterminal(ToUtf32("InitializerList"));
        initializerListNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<InitializerClauseRule>(this, &InitializerClauseRule::PreInitializerList));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new AssignInit(context->fromAssignmentExpression);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new AssignInit(nullptr);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new AssignInit(nullptr);
    }
    void PostAssignmentExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAssignmentExpression_value = std::move(stack.top());
            context->fromAssignmentExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromAssignmentExpression_value.get());
            stack.pop();
        }
    }
    void PreInitializerList(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::AssignInit*>(context->value)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromAssignmentExpression() {}
        cminor::codedom::AssignInit* value;
        cminor::codedom::CppObject* fromAssignmentExpression;
    };
};

class DeclaratorGrammar::InitializerListRule : public cminor::parsing::Rule
{
public:
    InitializerListRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::codedom::AssignInit*"), ToUtf32("init")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> init_value = std::move(stack.top());
        context->init = *static_cast<cminor::parsing::ValueObject<cminor::codedom::AssignInit*>*>(init_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitializerListRule>(this, &InitializerListRule::A0Action));
        cminor::parsing::NonterminalParser* initializerClauseNonterminalParser = GetNonterminal(ToUtf32("InitializerClause"));
        initializerClauseNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InitializerListRule>(this, &InitializerListRule::PostInitializerClause));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->init->Add(context->fromInitializerClause);
    }
    void PostInitializerClause(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromInitializerClause_value = std::move(stack.top());
            context->fromInitializerClause = *static_cast<cminor::parsing::ValueObject<cminor::codedom::AssignInit*>*>(fromInitializerClause_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): init(), fromInitializerClause() {}
        cminor::codedom::AssignInit* init;
        cminor::codedom::AssignInit* fromInitializerClause;
    };
};

void DeclaratorGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cminor.code.ExpressionGrammar"));
    if (!grammar0)
    {
        grammar0 = cminor::code::ExpressionGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar(ToUtf32("cminor.code.DeclarationGrammar"));
    if (!grammar1)
    {
        grammar1 = cminor::code::DeclarationGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
}

void DeclaratorGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("AssignmentExpression"), this, ToUtf32("ExpressionGrammar.AssignmentExpression")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("ConstantExpression"), this, ToUtf32("ExpressionGrammar.ConstantExpression")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("IdExpression"), this, ToUtf32("ExpressionGrammar.IdExpression")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("TypeSpecifier"), this, ToUtf32("DeclarationGrammar.TypeSpecifier")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("ExpressionList"), this, ToUtf32("ExpressionGrammar.ExpressionList")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("TypeName"), this, ToUtf32("DeclarationGrammar.TypeName")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("CVQualifier"), this, ToUtf32("DeclarationGrammar.CVQualifier")));
    AddRule(new InitDeclaratorListRule(ToUtf32("InitDeclaratorList"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::EmptyParser()),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::ListParser(
                    new cminor::parsing::ActionParser(ToUtf32("A2"),
                        new cminor::parsing::NonterminalParser(ToUtf32("InitDeclarator"), ToUtf32("InitDeclarator"), 0)),
                    new cminor::parsing::CharParser(','))))));
    AddRule(new InitDeclaratorRule(ToUtf32("InitDeclarator"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::NonterminalParser(ToUtf32("Declarator"), ToUtf32("Declarator"), 0),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("Initializer"), ToUtf32("Initializer"), 0))))));
    AddRule(new DeclaratorRule(ToUtf32("Declarator"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::NonterminalParser(ToUtf32("DirectDeclarator"), ToUtf32("DirectDeclarator"), 0),
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("PtrOperator"), ToUtf32("PtrOperator"), 0),
                    new cminor::parsing::NonterminalParser(ToUtf32("Declarator"), ToUtf32("Declarator"), 0))))));
    AddRule(new DirectDeclaratorRule(ToUtf32("DirectDeclarator"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::NonterminalParser(ToUtf32("DeclaratorId"), ToUtf32("DeclaratorId"), 0),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::CharParser('['),
                                    new cminor::parsing::OptionalParser(
                                        new cminor::parsing::ActionParser(ToUtf32("A0"),
                                            new cminor::parsing::NonterminalParser(ToUtf32("ConstantExpression"), ToUtf32("ConstantExpression"), 0)))),
                                new cminor::parsing::CharParser(']')),
                            new cminor::parsing::CharParser('(')),
                        new cminor::parsing::NonterminalParser(ToUtf32("Declarator"), ToUtf32("Declarator"), 0)),
                    new cminor::parsing::CharParser(')'))))));
    AddRule(new DeclaratorIdRule(ToUtf32("DeclaratorId"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("IdExpression"), ToUtf32("IdExpression"), 0)),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::NonterminalParser(ToUtf32("TypeName"), ToUtf32("TypeName"), 0)))));
    AddRule(new TypeIdRule(ToUtf32("TypeId"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::EmptyParser()),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("TypeSpecifierSeq"), ToUtf32("TypeSpecifierSeq"), 1),
                        new cminor::parsing::ActionParser(ToUtf32("A2"),
                            new cminor::parsing::NonterminalParser(ToUtf32("TypeName"), ToUtf32("TypeName"), 0))),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::ActionParser(ToUtf32("A3"),
                            new cminor::parsing::NonterminalParser(ToUtf32("AbstractDeclarator"), ToUtf32("AbstractDeclarator"), 0))))))));
    AddRule(new TypeRule(ToUtf32("Type"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::EmptyParser()),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::PositiveParser(
                        new cminor::parsing::ActionParser(ToUtf32("A2"),
                            new cminor::parsing::NonterminalParser(ToUtf32("TypeSpecifier"), ToUtf32("TypeSpecifier"), 0))),
                    new cminor::parsing::ActionParser(ToUtf32("A3"),
                        new cminor::parsing::NonterminalParser(ToUtf32("TypeName"), ToUtf32("TypeName"), 0)))))));
    AddRule(new TypeSpecifierSeqRule(ToUtf32("TypeSpecifierSeq"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::PositiveParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("TypeSpecifier"), ToUtf32("TypeSpecifier"), 0)))));
    AddRule(new AbstractDeclaratorRule(ToUtf32("AbstractDeclarator"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("PtrOperator"), ToUtf32("PtrOperator"), 0),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("AbstractDeclarator"), ToUtf32("AbstractDeclarator"), 0))),
                new cminor::parsing::NonterminalParser(ToUtf32("DirectAbstractDeclarator"), ToUtf32("DirectAbstractDeclarator"), 0)))));
    AddRule(new DirectAbstractDeclaratorRule(ToUtf32("DirectAbstractDeclarator"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::PositiveParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::CharParser('['),
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::ActionParser(ToUtf32("A0"),
                                new cminor::parsing::NonterminalParser(ToUtf32("ConstantExpression"), ToUtf32("ConstantExpression"), 0)))),
                    new cminor::parsing::CharParser(']')),
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::CharParser('('),
                        new cminor::parsing::NonterminalParser(ToUtf32("AbstractDeclarator"), ToUtf32("AbstractDeclarator"), 0)),
                    new cminor::parsing::CharParser(')'))))));
    AddRule(new cminor::parsing::Rule(ToUtf32("PtrOperator"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('*'),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("CVQualifierSeq"), ToUtf32("CVQualifierSeq"), 0))),
            new cminor::parsing::CharParser('&'))));
    AddRule(new CVQualifierSeqRule(ToUtf32("CVQualifierSeq"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::PositiveParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("CVQualifier"), ToUtf32("CVQualifier"), 0)))));
    AddRule(new InitializerRule(ToUtf32("Initializer"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('='),
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::NonterminalParser(ToUtf32("InitializerClause"), ToUtf32("InitializerClause"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::CharParser('('),
                        new cminor::parsing::NonterminalParser(ToUtf32("ExpressionList"), ToUtf32("ExpressionList"), 0)),
                    new cminor::parsing::CharParser(')'))))));
    AddRule(new InitializerClauseRule(ToUtf32("InitializerClause"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::NonterminalParser(ToUtf32("AssignmentExpression"), ToUtf32("AssignmentExpression"), 0)),
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser(ToUtf32("A1"),
                            new cminor::parsing::CharParser('{')),
                        new cminor::parsing::NonterminalParser(ToUtf32("InitializerList"), ToUtf32("InitializerList"), 1)),
                    new cminor::parsing::CharParser('}'))),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('{'),
                new cminor::parsing::ActionParser(ToUtf32("A2"),
                    new cminor::parsing::CharParser('}'))))));
    AddRule(new InitializerListRule(ToUtf32("InitializerList"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ListParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("InitializerClause"), ToUtf32("InitializerClause"), 0)),
            new cminor::parsing::CharParser(','))));
}

} } // namespace cminor.code
