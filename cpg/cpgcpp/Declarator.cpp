#include "Declarator.hpp"
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>
#include <cminor/cpg/cpgcpp/Expression.hpp>
#include <cminor/cpg/cpgcpp/Declaration.hpp>
#include <cminor/util/TextUtils.hpp>
#include <cminor/pom/Type.hpp>

namespace cpg { namespace cpp {

using cminor::util::Trim;
using namespace cminor::pom;
using namespace cminor::parsing;

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

DeclaratorGrammar::DeclaratorGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("DeclaratorGrammar", parsingDomain_->GetNamespaceScope("cpg.cpp"), parsingDomain_)
{
    SetOwner(0);
}

cminor::pom::InitDeclaratorList* DeclaratorGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName)
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
    cminor::pom::InitDeclaratorList* result = *static_cast<cminor::parsing::ValueObject<cminor::pom::InitDeclaratorList*>*>(value.get());
    stack.pop();
    return result;
}

class DeclaratorGrammar::InitDeclaratorListRule : public cminor::parsing::Rule
{
public:
    InitDeclaratorListRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::InitDeclaratorList*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<InitDeclaratorList>", "idl"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::InitDeclaratorList*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitDeclaratorListRule>(this, &InitDeclaratorListRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitDeclaratorListRule>(this, &InitDeclaratorListRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitDeclaratorListRule>(this, &InitDeclaratorListRule::A2Action));
        cminor::parsing::NonterminalParser* initDeclaratorNonterminalParser = GetNonterminal("InitDeclarator");
        initDeclaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InitDeclaratorListRule>(this, &InitDeclaratorListRule::PostInitDeclarator));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->idl.reset(new InitDeclaratorList);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->idl.release();
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromInitDeclarator = *static_cast<cminor::parsing::ValueObject<cminor::pom::InitDeclarator*>*>(fromInitDeclarator_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), idl(), fromInitDeclarator() {}
        cminor::pom::InitDeclaratorList* value;
        std::unique_ptr<InitDeclaratorList> idl;
        cminor::pom::InitDeclarator* fromInitDeclarator;
    };
};

class DeclaratorGrammar::InitDeclaratorRule : public cminor::parsing::Rule
{
public:
    InitDeclaratorRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::InitDeclarator*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::InitDeclarator*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitDeclaratorRule>(this, &InitDeclaratorRule::A0Action));
        cminor::parsing::NonterminalParser* declaratorNonterminalParser = GetNonterminal("Declarator");
        declaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InitDeclaratorRule>(this, &InitDeclaratorRule::PostDeclarator));
        cminor::parsing::NonterminalParser* initializerNonterminalParser = GetNonterminal("Initializer");
        initializerNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InitDeclaratorRule>(this, &InitDeclaratorRule::PostInitializer));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromDeclarator = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromDeclarator_value.get());
            stack.pop();
        }
    }
    void PostInitializer(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromInitializer_value = std::move(stack.top());
            context->fromInitializer = *static_cast<cminor::parsing::ValueObject<cminor::pom::Initializer*>*>(fromInitializer_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromDeclarator(), fromInitializer() {}
        cminor::pom::InitDeclarator* value;
        std::string fromDeclarator;
        cminor::pom::Initializer* fromInitializer;
    };
};

class DeclaratorGrammar::DeclaratorRule : public cminor::parsing::Rule
{
public:
    DeclaratorRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("std::string");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::string>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeclaratorRule>(this, &DeclaratorRule::A0Action));
        cminor::parsing::NonterminalParser* directDeclaratorNonterminalParser = GetNonterminal("DirectDeclarator");
        cminor::parsing::NonterminalParser* declaratorNonterminalParser = GetNonterminal("Declarator");
        declaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeclaratorRule>(this, &DeclaratorRule::PostDeclarator));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Trim(std::string(matchBegin, matchEnd));
    }
    void PostDeclarator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDeclarator_value = std::move(stack.top());
            context->fromDeclarator = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromDeclarator_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromDeclarator() {}
        std::string value;
        std::string fromDeclarator;
    };
};

class DeclaratorGrammar::DirectDeclaratorRule : public cminor::parsing::Rule
{
public:
    DirectDeclaratorRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddLocalVariable(AttrOrVariable("std::unique_ptr<CppObject>", "o"));
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
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DirectDeclaratorRule>(this, &DirectDeclaratorRule::A0Action));
        cminor::parsing::NonterminalParser* declaratorIdNonterminalParser = GetNonterminal("DeclaratorId");
        cminor::parsing::NonterminalParser* constantExpressionNonterminalParser = GetNonterminal("ConstantExpression");
        constantExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DirectDeclaratorRule>(this, &DirectDeclaratorRule::PostConstantExpression));
        cminor::parsing::NonterminalParser* declaratorNonterminalParser = GetNonterminal("Declarator");
        declaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DirectDeclaratorRule>(this, &DirectDeclaratorRule::PostDeclarator));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromConstantExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromConstantExpression_value.get());
            stack.pop();
        }
    }
    void PostDeclarator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDeclarator_value = std::move(stack.top());
            context->fromDeclarator = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromDeclarator_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): o(), fromConstantExpression(), fromDeclarator() {}
        std::unique_ptr<CppObject> o;
        cminor::pom::CppObject* fromConstantExpression;
        std::string fromDeclarator;
    };
};

class DeclaratorGrammar::DeclaratorIdRule : public cminor::parsing::Rule
{
public:
    DeclaratorIdRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddLocalVariable(AttrOrVariable("std::unique_ptr<CppObject>", "o"));
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
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeclaratorIdRule>(this, &DeclaratorIdRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeclaratorIdRule>(this, &DeclaratorIdRule::A1Action));
        cminor::parsing::NonterminalParser* idExpressionNonterminalParser = GetNonterminal("IdExpression");
        idExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeclaratorIdRule>(this, &DeclaratorIdRule::PostIdExpression));
        cminor::parsing::NonterminalParser* typeNameNonterminalParser = GetNonterminal("TypeName");
        typeNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeclaratorIdRule>(this, &DeclaratorIdRule::PostTypeName));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->o.reset(context->fromIdExpression);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromIdExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::IdExpr*>*>(fromIdExpression_value.get());
            stack.pop();
        }
    }
    void PostTypeName(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeName_value = std::move(stack.top());
            context->fromTypeName = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeName*>*>(fromTypeName_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): o(), fromIdExpression(), fromTypeName() {}
        std::unique_ptr<CppObject> o;
        cminor::pom::IdExpr* fromIdExpression;
        cminor::pom::TypeName* fromTypeName;
    };
};

class DeclaratorGrammar::TypeIdRule : public cminor::parsing::Rule
{
public:
    TypeIdRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::TypeId*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<TypeId>", "ti"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::TypeId*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeIdRule>(this, &TypeIdRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeIdRule>(this, &TypeIdRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeIdRule>(this, &TypeIdRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeIdRule>(this, &TypeIdRule::A3Action));
        cminor::parsing::NonterminalParser* typeSpecifierSeqNonterminalParser = GetNonterminal("TypeSpecifierSeq");
        typeSpecifierSeqNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<TypeIdRule>(this, &TypeIdRule::PreTypeSpecifierSeq));
        cminor::parsing::NonterminalParser* typeNameNonterminalParser = GetNonterminal("TypeName");
        typeNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TypeIdRule>(this, &TypeIdRule::PostTypeName));
        cminor::parsing::NonterminalParser* abstractDeclaratorNonterminalParser = GetNonterminal("AbstractDeclarator");
        abstractDeclaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TypeIdRule>(this, &TypeIdRule::PostAbstractDeclarator));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ti.reset(new TypeId);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->ti.release();
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ti->Add(context->fromTypeName);
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ti->Declarator() = context->fromAbstractDeclarator;
    }
    void PreTypeSpecifierSeq(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::TypeId*>(context->ti.get())));
    }
    void PostTypeName(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeName_value = std::move(stack.top());
            context->fromTypeName = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeName*>*>(fromTypeName_value.get());
            stack.pop();
        }
    }
    void PostAbstractDeclarator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAbstractDeclarator_value = std::move(stack.top());
            context->fromAbstractDeclarator = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromAbstractDeclarator_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), ti(), fromTypeName(), fromAbstractDeclarator() {}
        cminor::pom::TypeId* value;
        std::unique_ptr<TypeId> ti;
        cminor::pom::TypeName* fromTypeName;
        std::string fromAbstractDeclarator;
    };
};

class DeclaratorGrammar::TypeRule : public cminor::parsing::Rule
{
public:
    TypeRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Type*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Type>", "t"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Type*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeRule>(this, &TypeRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeRule>(this, &TypeRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeRule>(this, &TypeRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeRule>(this, &TypeRule::A3Action));
        cminor::parsing::NonterminalParser* typeSpecifierNonterminalParser = GetNonterminal("TypeSpecifier");
        typeSpecifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TypeRule>(this, &TypeRule::PostTypeSpecifier));
        cminor::parsing::NonterminalParser* typeNameNonterminalParser = GetNonterminal("TypeName");
        typeNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TypeRule>(this, &TypeRule::PostTypeName));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->t.reset(new Type);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->t.release();
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->t->Add(context->fromTypeSpecifier);
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromTypeSpecifier = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeSpecifier*>*>(fromTypeSpecifier_value.get());
            stack.pop();
        }
    }
    void PostTypeName(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeName_value = std::move(stack.top());
            context->fromTypeName = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeName*>*>(fromTypeName_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), t(), fromTypeSpecifier(), fromTypeName() {}
        cminor::pom::Type* value;
        std::unique_ptr<Type> t;
        cminor::pom::TypeSpecifier* fromTypeSpecifier;
        cminor::pom::TypeName* fromTypeName;
    };
};

class DeclaratorGrammar::TypeSpecifierSeqRule : public cminor::parsing::Rule
{
public:
    TypeSpecifierSeqRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable("cminor::pom::TypeId*", "typeId"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> typeId_value = std::move(stack.top());
        context->typeId = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeId*>*>(typeId_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeSpecifierSeqRule>(this, &TypeSpecifierSeqRule::A0Action));
        cminor::parsing::NonterminalParser* typeSpecifierNonterminalParser = GetNonterminal("TypeSpecifier");
        typeSpecifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TypeSpecifierSeqRule>(this, &TypeSpecifierSeqRule::PostTypeSpecifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromTypeSpecifier = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeSpecifier*>*>(fromTypeSpecifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): typeId(), fromTypeSpecifier() {}
        cminor::pom::TypeId* typeId;
        cminor::pom::TypeSpecifier* fromTypeSpecifier;
    };
};

class DeclaratorGrammar::AbstractDeclaratorRule : public cminor::parsing::Rule
{
public:
    AbstractDeclaratorRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("std::string");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::string>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AbstractDeclaratorRule>(this, &AbstractDeclaratorRule::A0Action));
        cminor::parsing::NonterminalParser* abstractDeclaratorNonterminalParser = GetNonterminal("AbstractDeclarator");
        abstractDeclaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AbstractDeclaratorRule>(this, &AbstractDeclaratorRule::PostAbstractDeclarator));
        cminor::parsing::NonterminalParser* directAbstractDeclaratorNonterminalParser = GetNonterminal("DirectAbstractDeclarator");
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Trim(std::string(matchBegin, matchEnd));
    }
    void PostAbstractDeclarator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAbstractDeclarator_value = std::move(stack.top());
            context->fromAbstractDeclarator = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromAbstractDeclarator_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromAbstractDeclarator() {}
        std::string value;
        std::string fromAbstractDeclarator;
    };
};

class DeclaratorGrammar::DirectAbstractDeclaratorRule : public cminor::parsing::Rule
{
public:
    DirectAbstractDeclaratorRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddLocalVariable(AttrOrVariable("std::unique_ptr<CppObject>", "o"));
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
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DirectAbstractDeclaratorRule>(this, &DirectAbstractDeclaratorRule::A0Action));
        cminor::parsing::NonterminalParser* constantExpressionNonterminalParser = GetNonterminal("ConstantExpression");
        constantExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DirectAbstractDeclaratorRule>(this, &DirectAbstractDeclaratorRule::PostConstantExpression));
        cminor::parsing::NonterminalParser* abstractDeclaratorNonterminalParser = GetNonterminal("AbstractDeclarator");
        abstractDeclaratorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DirectAbstractDeclaratorRule>(this, &DirectAbstractDeclaratorRule::PostAbstractDeclarator));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromConstantExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromConstantExpression_value.get());
            stack.pop();
        }
    }
    void PostAbstractDeclarator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAbstractDeclarator_value = std::move(stack.top());
            context->fromAbstractDeclarator = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromAbstractDeclarator_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): o(), fromConstantExpression(), fromAbstractDeclarator() {}
        std::unique_ptr<CppObject> o;
        cminor::pom::CppObject* fromConstantExpression;
        std::string fromAbstractDeclarator;
    };
};

class DeclaratorGrammar::CVQualifierSeqRule : public cminor::parsing::Rule
{
public:
    CVQualifierSeqRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddLocalVariable(AttrOrVariable("std::unique_ptr<CppObject>", "o"));
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
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CVQualifierSeqRule>(this, &CVQualifierSeqRule::A0Action));
        cminor::parsing::NonterminalParser* cVQualifierNonterminalParser = GetNonterminal("CVQualifier");
        cVQualifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CVQualifierSeqRule>(this, &CVQualifierSeqRule::PostCVQualifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromCVQualifier = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeSpecifier*>*>(fromCVQualifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): o(), fromCVQualifier() {}
        std::unique_ptr<CppObject> o;
        cminor::pom::TypeSpecifier* fromCVQualifier;
    };
};

class DeclaratorGrammar::InitializerRule : public cminor::parsing::Rule
{
public:
    InitializerRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::Initializer*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::Initializer*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitializerRule>(this, &InitializerRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitializerRule>(this, &InitializerRule::A1Action));
        cminor::parsing::NonterminalParser* initializerClauseNonterminalParser = GetNonterminal("InitializerClause");
        initializerClauseNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InitializerRule>(this, &InitializerRule::PostInitializerClause));
        cminor::parsing::NonterminalParser* expressionListNonterminalParser = GetNonterminal("ExpressionList");
        expressionListNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InitializerRule>(this, &InitializerRule::PostExpressionList));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Initializer(context->fromInitializerClause, GetEmptyVector());
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromInitializerClause = *static_cast<cminor::parsing::ValueObject<cminor::pom::AssignInit*>*>(fromInitializerClause_value.get());
            stack.pop();
        }
    }
    void PostExpressionList(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromExpressionList_value = std::move(stack.top());
            context->fromExpressionList = *static_cast<cminor::parsing::ValueObject<std::vector<cminor::pom::CppObject*>>*>(fromExpressionList_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromInitializerClause(), fromExpressionList() {}
        cminor::pom::Initializer* value;
        cminor::pom::AssignInit* fromInitializerClause;
        std::vector<cminor::pom::CppObject*> fromExpressionList;
    };
};

class DeclaratorGrammar::InitializerClauseRule : public cminor::parsing::Rule
{
public:
    InitializerClauseRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::AssignInit*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::AssignInit*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitializerClauseRule>(this, &InitializerClauseRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitializerClauseRule>(this, &InitializerClauseRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitializerClauseRule>(this, &InitializerClauseRule::A2Action));
        cminor::parsing::NonterminalParser* assignmentExpressionNonterminalParser = GetNonterminal("AssignmentExpression");
        assignmentExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InitializerClauseRule>(this, &InitializerClauseRule::PostAssignmentExpression));
        cminor::parsing::NonterminalParser* initializerListNonterminalParser = GetNonterminal("InitializerList");
        initializerListNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<InitializerClauseRule>(this, &InitializerClauseRule::PreInitializerList));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new AssignInit(context->fromAssignmentExpression);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new AssignInit(nullptr);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromAssignmentExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromAssignmentExpression_value.get());
            stack.pop();
        }
    }
    void PreInitializerList(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::AssignInit*>(context->value)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromAssignmentExpression() {}
        cminor::pom::AssignInit* value;
        cminor::pom::CppObject* fromAssignmentExpression;
    };
};

class DeclaratorGrammar::InitializerListRule : public cminor::parsing::Rule
{
public:
    InitializerListRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable("cminor::pom::AssignInit*", "init"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> init_value = std::move(stack.top());
        context->init = *static_cast<cminor::parsing::ValueObject<cminor::pom::AssignInit*>*>(init_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitializerListRule>(this, &InitializerListRule::A0Action));
        cminor::parsing::NonterminalParser* initializerClauseNonterminalParser = GetNonterminal("InitializerClause");
        initializerClauseNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InitializerListRule>(this, &InitializerListRule::PostInitializerClause));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromInitializerClause = *static_cast<cminor::parsing::ValueObject<cminor::pom::AssignInit*>*>(fromInitializerClause_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): init(), fromInitializerClause() {}
        cminor::pom::AssignInit* init;
        cminor::pom::AssignInit* fromInitializerClause;
    };
};

void DeclaratorGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("cpg.cpp.ExpressionGrammar");
    if (!grammar0)
    {
        grammar0 = cpg::cpp::ExpressionGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar("cpg.cpp.DeclarationGrammar");
    if (!grammar1)
    {
        grammar1 = cpg::cpp::DeclarationGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
}

void DeclaratorGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("ConstantExpression", this, "ExpressionGrammar.ConstantExpression"));
    AddRuleLink(new cminor::parsing::RuleLink("IdExpression", this, "ExpressionGrammar.IdExpression"));
    AddRuleLink(new cminor::parsing::RuleLink("TypeSpecifier", this, "DeclarationGrammar.TypeSpecifier"));
    AddRuleLink(new cminor::parsing::RuleLink("ExpressionList", this, "ExpressionGrammar.ExpressionList"));
    AddRuleLink(new cminor::parsing::RuleLink("AssignmentExpression", this, "ExpressionGrammar.AssignmentExpression"));
    AddRuleLink(new cminor::parsing::RuleLink("TypeName", this, "DeclarationGrammar.TypeName"));
    AddRuleLink(new cminor::parsing::RuleLink("CVQualifier", this, "DeclarationGrammar.CVQualifier"));
    AddRule(new InitDeclaratorListRule("InitDeclaratorList", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::EmptyParser()),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::ListParser(
                    new cminor::parsing::ActionParser("A2",
                        new cminor::parsing::NonterminalParser("InitDeclarator", "InitDeclarator", 0)),
                    new cminor::parsing::CharParser(','))))));
    AddRule(new InitDeclaratorRule("InitDeclarator", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::NonterminalParser("Declarator", "Declarator", 0),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::NonterminalParser("Initializer", "Initializer", 0))))));
    AddRule(new DeclaratorRule("Declarator", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::NonterminalParser("DirectDeclarator", "DirectDeclarator", 0),
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser("PtrOperator", "PtrOperator", 0),
                    new cminor::parsing::NonterminalParser("Declarator", "Declarator", 0))))));
    AddRule(new DirectDeclaratorRule("DirectDeclarator", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::NonterminalParser("DeclaratorId", "DeclaratorId", 0),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::CharParser('['),
                                    new cminor::parsing::OptionalParser(
                                        new cminor::parsing::ActionParser("A0",
                                            new cminor::parsing::NonterminalParser("ConstantExpression", "ConstantExpression", 0)))),
                                new cminor::parsing::CharParser(']')),
                            new cminor::parsing::CharParser('(')),
                        new cminor::parsing::NonterminalParser("Declarator", "Declarator", 0)),
                    new cminor::parsing::CharParser(')'))))));
    AddRule(new DeclaratorIdRule("DeclaratorId", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("IdExpression", "IdExpression", 0)),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::NonterminalParser("TypeName", "TypeName", 0)))));
    AddRule(new TypeIdRule("TypeId", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::EmptyParser()),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::NonterminalParser("TypeSpecifierSeq", "TypeSpecifierSeq", 1),
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::NonterminalParser("TypeName", "TypeName", 0))),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::ActionParser("A3",
                            new cminor::parsing::NonterminalParser("AbstractDeclarator", "AbstractDeclarator", 0))))))));
    AddRule(new TypeRule("Type", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::EmptyParser()),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::PositiveParser(
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::NonterminalParser("TypeSpecifier", "TypeSpecifier", 0))),
                    new cminor::parsing::ActionParser("A3",
                        new cminor::parsing::NonterminalParser("TypeName", "TypeName", 0)))))));
    AddRule(new TypeSpecifierSeqRule("TypeSpecifierSeq", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::PositiveParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("TypeSpecifier", "TypeSpecifier", 0)))));
    AddRule(new AbstractDeclaratorRule("AbstractDeclarator", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::NonterminalParser("PtrOperator", "PtrOperator", 0),
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser("AbstractDeclarator", "AbstractDeclarator", 0))),
                new cminor::parsing::NonterminalParser("DirectAbstractDeclarator", "DirectAbstractDeclarator", 0)))));
    AddRule(new DirectAbstractDeclaratorRule("DirectAbstractDeclarator", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::PositiveParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::CharParser('['),
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::ActionParser("A0",
                                new cminor::parsing::NonterminalParser("ConstantExpression", "ConstantExpression", 0)))),
                    new cminor::parsing::CharParser(']')),
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::CharParser('('),
                        new cminor::parsing::NonterminalParser("AbstractDeclarator", "AbstractDeclarator", 0)),
                    new cminor::parsing::CharParser(')'))))));
    AddRule(new cminor::parsing::Rule("PtrOperator", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('*'),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::NonterminalParser("CVQualifierSeq", "CVQualifierSeq", 0))),
            new cminor::parsing::CharParser('&'))));
    AddRule(new CVQualifierSeqRule("CVQualifierSeq", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::PositiveParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("CVQualifier", "CVQualifier", 0)))));
    AddRule(new InitializerRule("Initializer", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('='),
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::NonterminalParser("InitializerClause", "InitializerClause", 0))),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::CharParser('('),
                        new cminor::parsing::NonterminalParser("ExpressionList", "ExpressionList", 0)),
                    new cminor::parsing::CharParser(')'))))));
    AddRule(new InitializerClauseRule("InitializerClause", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::NonterminalParser("AssignmentExpression", "AssignmentExpression", 0)),
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser("A1",
                            new cminor::parsing::CharParser('{')),
                        new cminor::parsing::NonterminalParser("InitializerList", "InitializerList", 1)),
                    new cminor::parsing::CharParser('}'))),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('{'),
                new cminor::parsing::ActionParser("A2",
                    new cminor::parsing::CharParser('}'))))));
    AddRule(new InitializerListRule("InitializerList", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ListParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("InitializerClause", "InitializerClause", 0)),
            new cminor::parsing::CharParser(','))));
}

} } // namespace cpg.cpp
