#include "BasicType.hpp"
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>

namespace cminor { namespace parser {

using namespace cminor::parsing;

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

BasicTypeGrammar::BasicTypeGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("BasicTypeGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

Node* BasicTypeGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName)
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
    Node* result = *static_cast<cminor::parsing::ValueObject<Node*>*>(value.get());
    stack.pop();
    return result;
}

class BasicTypeGrammar::BasicTypeRule : public cminor::parsing::Rule
{
public:
    BasicTypeRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Node*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction("A6");
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction("A7");
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction("A8");
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A8Action));
        cminor::parsing::ActionParser* a9ActionParser = GetAction("A9");
        a9ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A9Action));
        cminor::parsing::ActionParser* a10ActionParser = GetAction("A10");
        a10ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A10Action));
        cminor::parsing::ActionParser* a11ActionParser = GetAction("A11");
        a11ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A11Action));
        cminor::parsing::ActionParser* a12ActionParser = GetAction("A12");
        a12ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A12Action));
        cminor::parsing::ActionParser* a13ActionParser = GetAction("A13");
        a13ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A13Action));
        cminor::parsing::ActionParser* a14ActionParser = GetAction("A14");
        a14ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A14Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new BoolNode(span);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new SByteNode(span);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ByteNode(span);
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ShortNode(span);
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new UShortNode(span);
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new IntNode(span);
    }
    void A6Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new UIntNode(span);
    }
    void A7Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new LongNode(span);
    }
    void A8Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ULongNode(span);
    }
    void A9Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FloatNode(span);
    }
    void A10Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new DoubleNode(span);
    }
    void A11Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new CharNode(span);
    }
    void A12Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new StringNode(span);
    }
    void A13Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new VoidNode(span);
    }
    void A14Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ObjectNode(span);
    }
private:
    struct Context
    {
        Context(): value() {}
        Node* value;
    };
    std::stack<Context> contextStack;
    Context context;
};

void BasicTypeGrammar::GetReferencedGrammars()
{
}

void BasicTypeGrammar::CreateRules()
{
    AddRule(new BasicTypeRule("BasicType", GetScope(),
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
                                                                new cminor::parsing::ActionParser("A0",
                                                                    new cminor::parsing::KeywordParser("bool")),
                                                                new cminor::parsing::ActionParser("A1",
                                                                    new cminor::parsing::KeywordParser("sbyte"))),
                                                            new cminor::parsing::ActionParser("A2",
                                                                new cminor::parsing::KeywordParser("byte"))),
                                                        new cminor::parsing::ActionParser("A3",
                                                            new cminor::parsing::KeywordParser("short"))),
                                                    new cminor::parsing::ActionParser("A4",
                                                        new cminor::parsing::KeywordParser("ushort"))),
                                                new cminor::parsing::ActionParser("A5",
                                                    new cminor::parsing::KeywordParser("int"))),
                                            new cminor::parsing::ActionParser("A6",
                                                new cminor::parsing::KeywordParser("uint"))),
                                        new cminor::parsing::ActionParser("A7",
                                            new cminor::parsing::KeywordParser("long"))),
                                    new cminor::parsing::ActionParser("A8",
                                        new cminor::parsing::KeywordParser("ulong"))),
                                new cminor::parsing::ActionParser("A9",
                                    new cminor::parsing::KeywordParser("float"))),
                            new cminor::parsing::ActionParser("A10",
                                new cminor::parsing::KeywordParser("double"))),
                        new cminor::parsing::ActionParser("A11",
                            new cminor::parsing::KeywordParser("char"))),
                    new cminor::parsing::ActionParser("A12",
                        new cminor::parsing::KeywordParser("string"))),
                new cminor::parsing::ActionParser("A13",
                    new cminor::parsing::KeywordParser("void"))),
            new cminor::parsing::ActionParser("A14",
                new cminor::parsing::KeywordParser("object")))));
}

} } // namespace cminor.parser
