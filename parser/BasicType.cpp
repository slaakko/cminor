#include "BasicType.hpp"
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

namespace cminor { namespace parser {

using namespace cminor::parsing;
using namespace cminor::util;
using namespace cminor::unicode;

BasicTypeGrammar* BasicTypeGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

BasicTypeGrammar* BasicTypeGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    BasicTypeGrammar* grammar(new BasicTypeGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

BasicTypeGrammar::BasicTypeGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("BasicTypeGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.parser")), parsingDomain_)
{
    SetOwner(0);
}

Node* BasicTypeGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName)
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
    Node* result = *static_cast<cminor::parsing::ValueObject<Node*>*>(value.get());
    stack.pop();
    return result;
}

class BasicTypeGrammar::BasicTypeRule : public cminor::parsing::Rule
{
public:
    BasicTypeRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("Node*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction(ToUtf32("A4"));
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction(ToUtf32("A5"));
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction(ToUtf32("A6"));
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction(ToUtf32("A7"));
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction(ToUtf32("A8"));
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A8Action));
        cminor::parsing::ActionParser* a9ActionParser = GetAction(ToUtf32("A9"));
        a9ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A9Action));
        cminor::parsing::ActionParser* a10ActionParser = GetAction(ToUtf32("A10"));
        a10ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A10Action));
        cminor::parsing::ActionParser* a11ActionParser = GetAction(ToUtf32("A11"));
        a11ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A11Action));
        cminor::parsing::ActionParser* a12ActionParser = GetAction(ToUtf32("A12"));
        a12ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A12Action));
        cminor::parsing::ActionParser* a13ActionParser = GetAction(ToUtf32("A13"));
        a13ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A13Action));
        cminor::parsing::ActionParser* a14ActionParser = GetAction(ToUtf32("A14"));
        a14ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A14Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BoolNode(span);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new SByteNode(span);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ByteNode(span);
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ShortNode(span);
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new UShortNode(span);
    }
    void A5Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new IntNode(span);
    }
    void A6Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new UIntNode(span);
    }
    void A7Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new LongNode(span);
    }
    void A8Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ULongNode(span);
    }
    void A9Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new FloatNode(span);
    }
    void A10Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new DoubleNode(span);
    }
    void A11Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new CharNode(span);
    }
    void A12Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new StringNode(span);
    }
    void A13Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new VoidNode(span);
    }
    void A14Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ObjectNode(span);
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        Node* value;
    };
};

void BasicTypeGrammar::GetReferencedGrammars()
{
}

void BasicTypeGrammar::CreateRules()
{
    AddRule(new BasicTypeRule(ToUtf32("BasicType"), GetScope(), GetParsingDomain()->GetNextRuleId(),
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
                                                    new cminor::parsing::AlternativeParser(
                                                        new cminor::parsing::AlternativeParser(
                                                            new cminor::parsing::AlternativeParser(
                                                                new cminor::parsing::ActionParser(ToUtf32("A0"),
                                                                    new cminor::parsing::KeywordParser(ToUtf32("bool"), ToUtf32("IdentifierChars"))),
                                                                new cminor::parsing::ActionParser(ToUtf32("A1"),
                                                                    new cminor::parsing::KeywordParser(ToUtf32("sbyte"), ToUtf32("IdentifierChars")))),
                                                            new cminor::parsing::ActionParser(ToUtf32("A2"),
                                                                new cminor::parsing::KeywordParser(ToUtf32("byte"), ToUtf32("IdentifierChars")))),
                                                        new cminor::parsing::ActionParser(ToUtf32("A3"),
                                                            new cminor::parsing::KeywordParser(ToUtf32("short"), ToUtf32("IdentifierChars")))),
                                                    new cminor::parsing::ActionParser(ToUtf32("A4"),
                                                        new cminor::parsing::KeywordParser(ToUtf32("ushort"), ToUtf32("IdentifierChars")))),
                                                new cminor::parsing::ActionParser(ToUtf32("A5"),
                                                    new cminor::parsing::KeywordParser(ToUtf32("int"), ToUtf32("IdentifierChars")))),
                                            new cminor::parsing::ActionParser(ToUtf32("A6"),
                                                new cminor::parsing::KeywordParser(ToUtf32("uint"), ToUtf32("IdentifierChars")))),
                                        new cminor::parsing::ActionParser(ToUtf32("A7"),
                                            new cminor::parsing::KeywordParser(ToUtf32("long"), ToUtf32("IdentifierChars")))),
                                    new cminor::parsing::ActionParser(ToUtf32("A8"),
                                        new cminor::parsing::KeywordParser(ToUtf32("ulong"), ToUtf32("IdentifierChars")))),
                                new cminor::parsing::ActionParser(ToUtf32("A9"),
                                    new cminor::parsing::KeywordParser(ToUtf32("float"), ToUtf32("IdentifierChars")))),
                            new cminor::parsing::ActionParser(ToUtf32("A10"),
                                new cminor::parsing::KeywordParser(ToUtf32("double"), ToUtf32("IdentifierChars")))),
                        new cminor::parsing::ActionParser(ToUtf32("A11"),
                            new cminor::parsing::KeywordParser(ToUtf32("char"), ToUtf32("IdentifierChars")))),
                    new cminor::parsing::ActionParser(ToUtf32("A12"),
                        new cminor::parsing::KeywordParser(ToUtf32("string"), ToUtf32("IdentifierChars")))),
                new cminor::parsing::ActionParser(ToUtf32("A13"),
                    new cminor::parsing::KeywordParser(ToUtf32("void"), ToUtf32("IdentifierChars")))),
            new cminor::parsing::ActionParser(ToUtf32("A14"),
                new cminor::parsing::KeywordParser(ToUtf32("object"), ToUtf32("IdentifierChars"))))));
    AddRule(new cminor::parsing::Rule(ToUtf32("IdentifierChars"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::PositiveParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::LetterParser(),
                new cminor::parsing::CharParser('_')))));
}

} } // namespace cminor.parser
