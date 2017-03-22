#include "Composite.hpp"
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/cpg/cpgsyntax/Primary.hpp>

namespace cpg { namespace syntax {

using namespace cminor::parsing;

CompositeGrammar* CompositeGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

CompositeGrammar* CompositeGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    CompositeGrammar* grammar(new CompositeGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

CompositeGrammar::CompositeGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("CompositeGrammar", parsingDomain_->GetNamespaceScope("cpg.syntax"), parsingDomain_)
{
    SetOwner(0);
}

cminor::parsing::Parser* CompositeGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, cminor::parsing::Scope* enclosingScope)
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
    stack.push(std::unique_ptr<cminor::parsing::Object>(new ValueObject<cminor::parsing::Scope*>(enclosingScope)));
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
    cminor::parsing::Parser* result = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(value.get());
    stack.pop();
    return result;
}

class CompositeGrammar::AlternativeRule : public cminor::parsing::Rule
{
public:
    AlternativeRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Scope*", "enclosingScope"));
        SetValueTypeName("cminor::parsing::Parser*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> enclosingScope_value = std::move(stack.top());
        context->enclosingScope = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Scope*>*>(enclosingScope_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AlternativeRule>(this, &AlternativeRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AlternativeRule>(this, &AlternativeRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<AlternativeRule>(this, &AlternativeRule::Preleft));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AlternativeRule>(this, &AlternativeRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<AlternativeRule>(this, &AlternativeRule::Preright));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<AlternativeRule>(this, &AlternativeRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromleft;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new AlternativeParser(context->value, context->fromright);
    }
    void Preleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context->enclosingScope)));
    }
    void Postleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context->fromleft = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context->enclosingScope)));
    }
    void Postright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context->fromright = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): enclosingScope(), value(), fromleft(), fromright() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Parser* value;
        cminor::parsing::Parser* fromleft;
        cminor::parsing::Parser* fromright;
    };
};

class CompositeGrammar::SequenceRule : public cminor::parsing::Rule
{
public:
    SequenceRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Scope*", "enclosingScope"));
        SetValueTypeName("cminor::parsing::Parser*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> enclosingScope_value = std::move(stack.top());
        context->enclosingScope = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Scope*>*>(enclosingScope_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SequenceRule>(this, &SequenceRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SequenceRule>(this, &SequenceRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<SequenceRule>(this, &SequenceRule::Preleft));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SequenceRule>(this, &SequenceRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<SequenceRule>(this, &SequenceRule::Preright));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SequenceRule>(this, &SequenceRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromleft;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new SequenceParser(context->value, context->fromright);
    }
    void Preleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context->enclosingScope)));
    }
    void Postleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context->fromleft = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context->enclosingScope)));
    }
    void Postright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context->fromright = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): enclosingScope(), value(), fromleft(), fromright() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Parser* value;
        cminor::parsing::Parser* fromleft;
        cminor::parsing::Parser* fromright;
    };
};

class CompositeGrammar::DifferenceRule : public cminor::parsing::Rule
{
public:
    DifferenceRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Scope*", "enclosingScope"));
        SetValueTypeName("cminor::parsing::Parser*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> enclosingScope_value = std::move(stack.top());
        context->enclosingScope = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Scope*>*>(enclosingScope_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DifferenceRule>(this, &DifferenceRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DifferenceRule>(this, &DifferenceRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DifferenceRule>(this, &DifferenceRule::Preleft));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DifferenceRule>(this, &DifferenceRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DifferenceRule>(this, &DifferenceRule::Preright));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DifferenceRule>(this, &DifferenceRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromleft;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new DifferenceParser(context->value, context->fromright);
    }
    void Preleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context->enclosingScope)));
    }
    void Postleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context->fromleft = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context->enclosingScope)));
    }
    void Postright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context->fromright = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): enclosingScope(), value(), fromleft(), fromright() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Parser* value;
        cminor::parsing::Parser* fromleft;
        cminor::parsing::Parser* fromright;
    };
};

class CompositeGrammar::ExclusiveOrRule : public cminor::parsing::Rule
{
public:
    ExclusiveOrRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Scope*", "enclosingScope"));
        SetValueTypeName("cminor::parsing::Parser*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> enclosingScope_value = std::move(stack.top());
        context->enclosingScope = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Scope*>*>(enclosingScope_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExclusiveOrRule>(this, &ExclusiveOrRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ExclusiveOrRule>(this, &ExclusiveOrRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ExclusiveOrRule>(this, &ExclusiveOrRule::Preleft));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ExclusiveOrRule>(this, &ExclusiveOrRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ExclusiveOrRule>(this, &ExclusiveOrRule::Preright));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ExclusiveOrRule>(this, &ExclusiveOrRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromleft;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ExclusiveOrParser(context->value, context->fromright);
    }
    void Preleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context->enclosingScope)));
    }
    void Postleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context->fromleft = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context->enclosingScope)));
    }
    void Postright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context->fromright = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): enclosingScope(), value(), fromleft(), fromright() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Parser* value;
        cminor::parsing::Parser* fromleft;
        cminor::parsing::Parser* fromright;
    };
};

class CompositeGrammar::IntersectionRule : public cminor::parsing::Rule
{
public:
    IntersectionRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Scope*", "enclosingScope"));
        SetValueTypeName("cminor::parsing::Parser*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> enclosingScope_value = std::move(stack.top());
        context->enclosingScope = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Scope*>*>(enclosingScope_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IntersectionRule>(this, &IntersectionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IntersectionRule>(this, &IntersectionRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<IntersectionRule>(this, &IntersectionRule::Preleft));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IntersectionRule>(this, &IntersectionRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<IntersectionRule>(this, &IntersectionRule::Preright));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IntersectionRule>(this, &IntersectionRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromleft;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new IntersectionParser(context->value, context->fromright);
    }
    void Preleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context->enclosingScope)));
    }
    void Postleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context->fromleft = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context->enclosingScope)));
    }
    void Postright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context->fromright = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): enclosingScope(), value(), fromleft(), fromright() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Parser* value;
        cminor::parsing::Parser* fromleft;
        cminor::parsing::Parser* fromright;
    };
};

class CompositeGrammar::ListRule : public cminor::parsing::Rule
{
public:
    ListRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Scope*", "enclosingScope"));
        SetValueTypeName("cminor::parsing::Parser*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> enclosingScope_value = std::move(stack.top());
        context->enclosingScope = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Scope*>*>(enclosingScope_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ListRule>(this, &ListRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ListRule>(this, &ListRule::A1Action));
        cminor::parsing::NonterminalParser* leftNonterminalParser = GetNonterminal("left");
        leftNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ListRule>(this, &ListRule::Preleft));
        leftNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ListRule>(this, &ListRule::Postleft));
        cminor::parsing::NonterminalParser* rightNonterminalParser = GetNonterminal("right");
        rightNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ListRule>(this, &ListRule::Preright));
        rightNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ListRule>(this, &ListRule::Postright));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromleft;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ListParser(context->value, context->fromright);
    }
    void Preleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context->enclosingScope)));
    }
    void Postleft(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromleft_value = std::move(stack.top());
            context->fromleft = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromleft_value.get());
            stack.pop();
        }
    }
    void Preright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context->enclosingScope)));
    }
    void Postright(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromright_value = std::move(stack.top());
            context->fromright = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromright_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): enclosingScope(), value(), fromleft(), fromright() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Parser* value;
        cminor::parsing::Parser* fromleft;
        cminor::parsing::Parser* fromright;
    };
};

class CompositeGrammar::PostfixRule : public cminor::parsing::Rule
{
public:
    PostfixRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::Scope*", "enclosingScope"));
        SetValueTypeName("cminor::parsing::Parser*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> enclosingScope_value = std::move(stack.top());
        context->enclosingScope = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Scope*>*>(enclosingScope_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixRule>(this, &PostfixRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixRule>(this, &PostfixRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixRule>(this, &PostfixRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PostfixRule>(this, &PostfixRule::A3Action));
        cminor::parsing::NonterminalParser* primaryNonterminalParser = GetNonterminal("Primary");
        primaryNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PostfixRule>(this, &PostfixRule::PrePrimary));
        primaryNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PostfixRule>(this, &PostfixRule::PostPrimary));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromPrimary;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new KleeneStarParser(context->value);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new PositiveParser(context->value);
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new OptionalParser(context->value);
    }
    void PrePrimary(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context->enclosingScope)));
    }
    void PostPrimary(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromPrimary_value = std::move(stack.top());
            context->fromPrimary = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromPrimary_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): enclosingScope(), value(), fromPrimary() {}
        cminor::parsing::Scope* enclosingScope;
        cminor::parsing::Parser* value;
        cminor::parsing::Parser* fromPrimary;
    };
};

void CompositeGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("cpg.syntax.PrimaryGrammar");
    if (!grammar0)
    {
        grammar0 = cpg::syntax::PrimaryGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
}

void CompositeGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("Primary", this, "PrimaryGrammar.Primary"));
    AddRule(new AlternativeRule("Alternative", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("left", "Sequence", 1)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('|'),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("right", "Sequence", 1))))))));
    AddRule(new SequenceRule("Sequence", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("left", "Difference", 1)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("right", "Difference", 1))))));
    AddRule(new DifferenceRule("Difference", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("left", "ExclusiveOr", 1)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('-'),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("right", "ExclusiveOr", 1))))))));
    AddRule(new ExclusiveOrRule("ExclusiveOr", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("left", "Intersection", 1)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('^'),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("right", "Intersection", 1))))))));
    AddRule(new IntersectionRule("Intersection", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("left", "List", 1)),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('&'),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::NonterminalParser("right", "List", 1)))))));
    AddRule(new ListRule("List", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("left", "Postfix", 1)),
            new cminor::parsing::OptionalParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('%'),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::NonterminalParser("right", "Postfix", 1)))))));
    AddRule(new PostfixRule("Postfix", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("Primary", "Primary", 1)),
            new cminor::parsing::OptionalParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::ActionParser("A1",
                            new cminor::parsing::CharParser('*')),
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::CharParser('+'))),
                    new cminor::parsing::ActionParser("A3",
                        new cminor::parsing::CharParser('?')))))));
}

} } // namespace cpg.syntax
