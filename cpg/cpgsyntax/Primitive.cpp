#include "Primitive.hpp"
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>
#include <cminor/cpg/cpgsyntax/Element.hpp>

namespace cpg { namespace syntax {

using namespace cminor::parsing;

PrimitiveGrammar* PrimitiveGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

PrimitiveGrammar* PrimitiveGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    PrimitiveGrammar* grammar(new PrimitiveGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

PrimitiveGrammar::PrimitiveGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("PrimitiveGrammar", parsingDomain_->GetNamespaceScope("cpg.syntax"), parsingDomain_)
{
    SetOwner(0);
}

cminor::parsing::Parser* PrimitiveGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName)
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
    cminor::parsing::Parser* result = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(value.get());
    stack.pop();
    return result;
}

class PrimitiveGrammar::PrimitiveRule : public cminor::parsing::Rule
{
public:
    PrimitiveRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::parsing::Parser*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction("A6");
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction("A7");
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction("A8");
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A8Action));
        cminor::parsing::ActionParser* a9ActionParser = GetAction("A9");
        a9ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A9Action));
        cminor::parsing::ActionParser* a10ActionParser = GetAction("A10");
        a10ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A10Action));
        cminor::parsing::ActionParser* a11ActionParser = GetAction("A11");
        a11ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A11Action));
        cminor::parsing::NonterminalParser* charNonterminalParser = GetNonterminal("Char");
        charNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostChar));
        cminor::parsing::NonterminalParser* stringNonterminalParser = GetNonterminal("String");
        stringNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostString));
        cminor::parsing::NonterminalParser* charSetNonterminalParser = GetNonterminal("CharSet");
        charSetNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostCharSet));
        cminor::parsing::NonterminalParser* keywordNonterminalParser = GetNonterminal("Keyword");
        keywordNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostKeyword));
        cminor::parsing::NonterminalParser* keywordListNonterminalParser = GetNonterminal("KeywordList");
        keywordListNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostKeywordList));
        cminor::parsing::NonterminalParser* emptyNonterminalParser = GetNonterminal("Empty");
        emptyNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostEmpty));
        cminor::parsing::NonterminalParser* spaceNonterminalParser = GetNonterminal("Space");
        spaceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostSpace));
        cminor::parsing::NonterminalParser* anyCharNonterminalParser = GetNonterminal("AnyChar");
        anyCharNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostAnyChar));
        cminor::parsing::NonterminalParser* letterNonterminalParser = GetNonterminal("Letter");
        letterNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostLetter));
        cminor::parsing::NonterminalParser* digitNonterminalParser = GetNonterminal("Digit");
        digitNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostDigit));
        cminor::parsing::NonterminalParser* hexDigitNonterminalParser = GetNonterminal("HexDigit");
        hexDigitNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostHexDigit));
        cminor::parsing::NonterminalParser* punctuationNonterminalParser = GetNonterminal("Punctuation");
        punctuationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostPunctuation));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromChar;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromString;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromCharSet;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromKeyword;
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromKeywordList;
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromEmpty;
    }
    void A6Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromSpace;
    }
    void A7Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromAnyChar;
    }
    void A8Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromLetter;
    }
    void A9Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromDigit;
    }
    void A10Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromHexDigit;
    }
    void A11Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromPunctuation;
    }
    void PostChar(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromChar_value = std::move(stack.top());
            context->fromChar = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromChar_value.get());
            stack.pop();
        }
    }
    void PostString(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromString_value = std::move(stack.top());
            context->fromString = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromString_value.get());
            stack.pop();
        }
    }
    void PostCharSet(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCharSet_value = std::move(stack.top());
            context->fromCharSet = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromCharSet_value.get());
            stack.pop();
        }
    }
    void PostKeyword(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromKeyword_value = std::move(stack.top());
            context->fromKeyword = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromKeyword_value.get());
            stack.pop();
        }
    }
    void PostKeywordList(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromKeywordList_value = std::move(stack.top());
            context->fromKeywordList = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromKeywordList_value.get());
            stack.pop();
        }
    }
    void PostEmpty(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromEmpty_value = std::move(stack.top());
            context->fromEmpty = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromEmpty_value.get());
            stack.pop();
        }
    }
    void PostSpace(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpace_value = std::move(stack.top());
            context->fromSpace = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromSpace_value.get());
            stack.pop();
        }
    }
    void PostAnyChar(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAnyChar_value = std::move(stack.top());
            context->fromAnyChar = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromAnyChar_value.get());
            stack.pop();
        }
    }
    void PostLetter(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromLetter_value = std::move(stack.top());
            context->fromLetter = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromLetter_value.get());
            stack.pop();
        }
    }
    void PostDigit(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDigit_value = std::move(stack.top());
            context->fromDigit = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromDigit_value.get());
            stack.pop();
        }
    }
    void PostHexDigit(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromHexDigit_value = std::move(stack.top());
            context->fromHexDigit = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromHexDigit_value.get());
            stack.pop();
        }
    }
    void PostPunctuation(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromPunctuation_value = std::move(stack.top());
            context->fromPunctuation = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromPunctuation_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromChar(), fromString(), fromCharSet(), fromKeyword(), fromKeywordList(), fromEmpty(), fromSpace(), fromAnyChar(), fromLetter(), fromDigit(), fromHexDigit(), fromPunctuation() {}
        cminor::parsing::Parser* value;
        cminor::parsing::Parser* fromChar;
        cminor::parsing::Parser* fromString;
        cminor::parsing::Parser* fromCharSet;
        cminor::parsing::Parser* fromKeyword;
        cminor::parsing::Parser* fromKeywordList;
        cminor::parsing::Parser* fromEmpty;
        cminor::parsing::Parser* fromSpace;
        cminor::parsing::Parser* fromAnyChar;
        cminor::parsing::Parser* fromLetter;
        cminor::parsing::Parser* fromDigit;
        cminor::parsing::Parser* fromHexDigit;
        cminor::parsing::Parser* fromPunctuation;
    };
};

class PrimitiveGrammar::CharRule : public cminor::parsing::Rule
{
public:
    CharRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::parsing::Parser*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharRule>(this, &CharRule::A0Action));
        cminor::parsing::NonterminalParser* chrNonterminalParser = GetNonterminal("chr");
        chrNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CharRule>(this, &CharRule::Postchr));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new CharParser(context->fromchr);
    }
    void Postchr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromchr_value = std::move(stack.top());
            context->fromchr = *static_cast<cminor::parsing::ValueObject<char>*>(fromchr_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromchr() {}
        cminor::parsing::Parser* value;
        char fromchr;
    };
};

class PrimitiveGrammar::StringRule : public cminor::parsing::Rule
{
public:
    StringRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::parsing::Parser*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StringRule>(this, &StringRule::A0Action));
        cminor::parsing::NonterminalParser* strNonterminalParser = GetNonterminal("str");
        strNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StringRule>(this, &StringRule::Poststr));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new StringParser(context->fromstr);
    }
    void Poststr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromstr_value = std::move(stack.top());
            context->fromstr = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromstr_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromstr() {}
        cminor::parsing::Parser* value;
        std::string fromstr;
    };
};

class PrimitiveGrammar::CharSetRule : public cminor::parsing::Rule
{
public:
    CharSetRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::parsing::Parser*");
        AddLocalVariable(AttrOrVariable("bool", "inverse"));
        AddLocalVariable(AttrOrVariable("std::string", "s"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharSetRule>(this, &CharSetRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharSetRule>(this, &CharSetRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharSetRule>(this, &CharSetRule::A2Action));
        cminor::parsing::NonterminalParser* charSetRangeNonterminalParser = GetNonterminal("CharSetRange");
        charSetRangeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CharSetRule>(this, &CharSetRule::PostCharSetRange));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new CharSetParser(context->s, context->inverse);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->inverse = true;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->s.append(context->fromCharSetRange);
    }
    void PostCharSetRange(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCharSetRange_value = std::move(stack.top());
            context->fromCharSetRange = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromCharSetRange_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), inverse(), s(), fromCharSetRange() {}
        cminor::parsing::Parser* value;
        bool inverse;
        std::string s;
        std::string fromCharSetRange;
    };
};

class PrimitiveGrammar::CharSetRangeRule : public cminor::parsing::Rule
{
public:
    CharSetRangeRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharSetRangeRule>(this, &CharSetRangeRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharSetRangeRule>(this, &CharSetRangeRule::A1Action));
        cminor::parsing::NonterminalParser* firstNonterminalParser = GetNonterminal("first");
        firstNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CharSetRangeRule>(this, &CharSetRangeRule::Postfirst));
        cminor::parsing::NonterminalParser* lastNonterminalParser = GetNonterminal("last");
        lastNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CharSetRangeRule>(this, &CharSetRangeRule::Postlast));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value.append(1, context->fromfirst);
        context->value.append(1, '-');
        context->value.append(1, context->fromlast);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value.append(1, context->fromfirst);
    }
    void Postfirst(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromfirst_value = std::move(stack.top());
            context->fromfirst = *static_cast<cminor::parsing::ValueObject<char>*>(fromfirst_value.get());
            stack.pop();
        }
    }
    void Postlast(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromlast_value = std::move(stack.top());
            context->fromlast = *static_cast<cminor::parsing::ValueObject<char>*>(fromlast_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromfirst(), fromlast() {}
        std::string value;
        char fromfirst;
        char fromlast;
    };
};

class PrimitiveGrammar::CharSetCharRule : public cminor::parsing::Rule
{
public:
    CharSetCharRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("char");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<char>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharSetCharRule>(this, &CharSetCharRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharSetCharRule>(this, &CharSetCharRule::A1Action));
        cminor::parsing::NonterminalParser* escapeNonterminalParser = GetNonterminal("escape");
        escapeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CharSetCharRule>(this, &CharSetCharRule::Postescape));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = *matchBegin;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromescape;
    }
    void Postescape(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromescape_value = std::move(stack.top());
            context->fromescape = *static_cast<cminor::parsing::ValueObject<char>*>(fromescape_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromescape() {}
        char value;
        char fromescape;
    };
};

class PrimitiveGrammar::KeywordRule : public cminor::parsing::Rule
{
public:
    KeywordRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::parsing::Parser*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<KeywordRule>(this, &KeywordRule::A0Action));
        cminor::parsing::NonterminalParser* keywordBodyNonterminalParser = GetNonterminal("KeywordBody");
        keywordBodyNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<KeywordRule>(this, &KeywordRule::PostKeywordBody));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromKeywordBody;
    }
    void PostKeywordBody(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromKeywordBody_value = std::move(stack.top());
            context->fromKeywordBody = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromKeywordBody_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromKeywordBody() {}
        cminor::parsing::Parser* value;
        cminor::parsing::Parser* fromKeywordBody;
    };
};

class PrimitiveGrammar::KeywordBodyRule : public cminor::parsing::Rule
{
public:
    KeywordBodyRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::parsing::Parser*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<KeywordBodyRule>(this, &KeywordBodyRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<KeywordBodyRule>(this, &KeywordBodyRule::A1Action));
        cminor::parsing::NonterminalParser* strNonterminalParser = GetNonterminal("str");
        strNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<KeywordBodyRule>(this, &KeywordBodyRule::Poststr));
        cminor::parsing::NonterminalParser* continuationNonterminalParser = GetNonterminal("continuation");
        continuationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<KeywordBodyRule>(this, &KeywordBodyRule::Postcontinuation));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new KeywordParser(context->fromstr, context->fromcontinuation);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new KeywordParser(context->fromstr);
    }
    void Poststr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromstr_value = std::move(stack.top());
            context->fromstr = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromstr_value.get());
            stack.pop();
        }
    }
    void Postcontinuation(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromcontinuation_value = std::move(stack.top());
            context->fromcontinuation = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromcontinuation_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromstr(), fromcontinuation() {}
        cminor::parsing::Parser* value;
        std::string fromstr;
        std::string fromcontinuation;
    };
};

class PrimitiveGrammar::KeywordListRule : public cminor::parsing::Rule
{
public:
    KeywordListRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::parsing::Parser*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<KeywordListRule>(this, &KeywordListRule::A0Action));
        cminor::parsing::NonterminalParser* keywordListBodyNonterminalParser = GetNonterminal("KeywordListBody");
        keywordListBodyNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<KeywordListRule>(this, &KeywordListRule::PostKeywordListBody));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromKeywordListBody;
    }
    void PostKeywordListBody(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromKeywordListBody_value = std::move(stack.top());
            context->fromKeywordListBody = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromKeywordListBody_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromKeywordListBody() {}
        cminor::parsing::Parser* value;
        cminor::parsing::Parser* fromKeywordListBody;
    };
};

class PrimitiveGrammar::KeywordListBodyRule : public cminor::parsing::Rule
{
public:
    KeywordListBodyRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::parsing::Parser*");
        AddLocalVariable(AttrOrVariable("std::vector<std::string>", "keywords"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<KeywordListBodyRule>(this, &KeywordListBodyRule::A0Action));
        cminor::parsing::NonterminalParser* selectorNonterminalParser = GetNonterminal("selector");
        selectorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<KeywordListBodyRule>(this, &KeywordListBodyRule::Postselector));
        cminor::parsing::NonterminalParser* stringArrayNonterminalParser = GetNonterminal("StringArray");
        stringArrayNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<KeywordListBodyRule>(this, &KeywordListBodyRule::PreStringArray));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new KeywordListParser(context->fromselector, context->keywords);
    }
    void Postselector(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromselector_value = std::move(stack.top());
            context->fromselector = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromselector_value.get());
            stack.pop();
        }
    }
    void PreStringArray(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::vector<std::string>*>(&context->keywords)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), keywords(), fromselector() {}
        cminor::parsing::Parser* value;
        std::vector<std::string> keywords;
        std::string fromselector;
    };
};

class PrimitiveGrammar::EmptyRule : public cminor::parsing::Rule
{
public:
    EmptyRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::parsing::Parser*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EmptyRule>(this, &EmptyRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new EmptyParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::SpaceRule : public cminor::parsing::Rule
{
public:
    SpaceRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::parsing::Parser*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SpaceRule>(this, &SpaceRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new SpaceParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::AnyCharRule : public cminor::parsing::Rule
{
public:
    AnyCharRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::parsing::Parser*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AnyCharRule>(this, &AnyCharRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new AnyCharParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::LetterRule : public cminor::parsing::Rule
{
public:
    LetterRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::parsing::Parser*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LetterRule>(this, &LetterRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new LetterParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::DigitRule : public cminor::parsing::Rule
{
public:
    DigitRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::parsing::Parser*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DigitRule>(this, &DigitRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new DigitParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::HexDigitRule : public cminor::parsing::Rule
{
public:
    HexDigitRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::parsing::Parser*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<HexDigitRule>(this, &HexDigitRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new HexDigitParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::PunctuationRule : public cminor::parsing::Rule
{
public:
    PunctuationRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::parsing::Parser*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PunctuationRule>(this, &PunctuationRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new PunctuationParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

void PrimitiveGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("cpg.syntax.ElementGrammar");
    if (!grammar0)
    {
        grammar0 = cpg::syntax::ElementGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar("cminor.parsing.stdlib");
    if (!grammar1)
    {
        grammar1 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar1);
}

void PrimitiveGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("char", this, "cminor.parsing.stdlib.char"));
    AddRuleLink(new cminor::parsing::RuleLink("StringArray", this, "ElementGrammar.StringArray"));
    AddRuleLink(new cminor::parsing::RuleLink("escape", this, "cminor.parsing.stdlib.escape"));
    AddRuleLink(new cminor::parsing::RuleLink("string", this, "cminor.parsing.stdlib.string"));
    AddRuleLink(new cminor::parsing::RuleLink("QualifiedId", this, "ElementGrammar.QualifiedId"));
    AddRule(new PrimitiveRule("Primitive", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::AlternativeParser(
                                    new cminor::parsing::AlternativeParser(
                                        new cminor::parsing::AlternativeParser(
                                            new cminor::parsing::AlternativeParser(
                                                new cminor::parsing::AlternativeParser(
                                                    new cminor::parsing::ActionParser("A0",
                                                        new cminor::parsing::NonterminalParser("Char", "Char", 0)),
                                                    new cminor::parsing::ActionParser("A1",
                                                        new cminor::parsing::NonterminalParser("String", "String", 0))),
                                                new cminor::parsing::ActionParser("A2",
                                                    new cminor::parsing::NonterminalParser("CharSet", "CharSet", 0))),
                                            new cminor::parsing::ActionParser("A3",
                                                new cminor::parsing::NonterminalParser("Keyword", "Keyword", 0))),
                                        new cminor::parsing::ActionParser("A4",
                                            new cminor::parsing::NonterminalParser("KeywordList", "KeywordList", 0))),
                                    new cminor::parsing::ActionParser("A5",
                                        new cminor::parsing::NonterminalParser("Empty", "Empty", 0))),
                                new cminor::parsing::ActionParser("A6",
                                    new cminor::parsing::NonterminalParser("Space", "Space", 0))),
                            new cminor::parsing::ActionParser("A7",
                                new cminor::parsing::NonterminalParser("AnyChar", "AnyChar", 0))),
                        new cminor::parsing::ActionParser("A8",
                            new cminor::parsing::NonterminalParser("Letter", "Letter", 0))),
                    new cminor::parsing::ActionParser("A9",
                        new cminor::parsing::NonterminalParser("Digit", "Digit", 0))),
                new cminor::parsing::ActionParser("A10",
                    new cminor::parsing::NonterminalParser("HexDigit", "HexDigit", 0))),
            new cminor::parsing::ActionParser("A11",
                new cminor::parsing::NonterminalParser("Punctuation", "Punctuation", 0)))));
    AddRule(new CharRule("Char", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::NonterminalParser("chr", "char", 0))));
    AddRule(new StringRule("String", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::NonterminalParser("str", "string", 0))));
    AddRule(new CharSetRule("CharSet", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::CharParser('['),
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::ActionParser("A1",
                                    new cminor::parsing::CharParser('^')))),
                        new cminor::parsing::KleeneStarParser(
                            new cminor::parsing::ActionParser("A2",
                                new cminor::parsing::NonterminalParser("CharSetRange", "CharSetRange", 0)))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser(']')))))));
    AddRule(new CharSetRangeRule("CharSetRange", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::NonterminalParser("first", "CharSetChar", 0),
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('-'),
                    new cminor::parsing::ActionParser("A0",
                        new cminor::parsing::NonterminalParser("last", "CharSetChar", 0))),
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::EmptyParser())))));
    AddRule(new CharSetCharRule("CharSetChar", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::CharSetParser("\\]", true)),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::NonterminalParser("escape", "escape", 0)))));
    AddRule(new KeywordRule("Keyword", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser("keyword"),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('('))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("KeywordBody", "KeywordBody", 0))),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))))));
    AddRule(new KeywordBodyRule("KeywordBody", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::NonterminalParser("str", "string", 0),
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser(','),
                    new cminor::parsing::ActionParser("A0",
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser("continuation", "QualifiedId", 0)))),
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::EmptyParser())))));
    AddRule(new KeywordListRule("KeywordList", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser("keyword_list"),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('('))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("KeywordListBody", "KeywordListBody", 0))),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))))));
    AddRule(new KeywordListBodyRule("KeywordListBody", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("selector", "QualifiedId", 0)),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(','))),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("StringArray", "StringArray", 1))))));
    AddRule(new EmptyRule("Empty", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::KeywordParser("empty"))));
    AddRule(new SpaceRule("Space", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::KeywordParser("space"))));
    AddRule(new AnyCharRule("AnyChar", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::KeywordParser("anychar"))));
    AddRule(new LetterRule("Letter", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::KeywordParser("letter"))));
    AddRule(new DigitRule("Digit", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::KeywordParser("digit"))));
    AddRule(new HexDigitRule("HexDigit", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::KeywordParser("hexdigit"))));
    AddRule(new PunctuationRule("Punctuation", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::KeywordParser("punctuation"))));
}

} } // namespace cpg.syntax
