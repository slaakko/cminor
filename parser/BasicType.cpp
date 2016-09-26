#include "BasicType.hpp"
#include <Cm.Parsing/Action.hpp>
#include <Cm.Parsing/Rule.hpp>
#include <Cm.Parsing/ParsingDomain.hpp>
#include <Cm.Parsing/Primitive.hpp>
#include <Cm.Parsing/Composite.hpp>
#include <Cm.Parsing/Nonterminal.hpp>
#include <Cm.Parsing/Exception.hpp>
#include <Cm.Parsing/StdLib.hpp>
#include <Cm.Parsing/XmlLog.hpp>

namespace cminor { namespace parser {

using namespace Cm::Parsing;

BasicTypeGrammar* BasicTypeGrammar::Create()
{
    return Create(new Cm::Parsing::ParsingDomain());
}

BasicTypeGrammar* BasicTypeGrammar::Create(Cm::Parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    BasicTypeGrammar* grammar(new BasicTypeGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

BasicTypeGrammar::BasicTypeGrammar(Cm::Parsing::ParsingDomain* parsingDomain_): Cm::Parsing::Grammar("BasicTypeGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

Node* BasicTypeGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName)
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
    Node* result = *static_cast<Cm::Parsing::ValueObject<Node*>*>(value.get());
    stack.pop();
    return result;
}

class BasicTypeGrammar::BasicTypeRule : public Cm::Parsing::Rule
{
public:
    BasicTypeRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Node*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A3Action));
        Cm::Parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A4Action));
        Cm::Parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A5Action));
        Cm::Parsing::ActionParser* a6ActionParser = GetAction("A6");
        a6ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A6Action));
        Cm::Parsing::ActionParser* a7ActionParser = GetAction("A7");
        a7ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A7Action));
        Cm::Parsing::ActionParser* a8ActionParser = GetAction("A8");
        a8ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A8Action));
        Cm::Parsing::ActionParser* a9ActionParser = GetAction("A9");
        a9ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A9Action));
        Cm::Parsing::ActionParser* a10ActionParser = GetAction("A10");
        a10ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A10Action));
        Cm::Parsing::ActionParser* a11ActionParser = GetAction("A11");
        a11ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A11Action));
        Cm::Parsing::ActionParser* a12ActionParser = GetAction("A12");
        a12ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A12Action));
        Cm::Parsing::ActionParser* a13ActionParser = GetAction("A13");
        a13ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BasicTypeRule>(this, &BasicTypeRule::A13Action));
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
                                                new Cm::Parsing::AlternativeParser(
                                                    new Cm::Parsing::AlternativeParser(
                                                        new Cm::Parsing::AlternativeParser(
                                                            new Cm::Parsing::ActionParser("A0",
                                                                new Cm::Parsing::KeywordParser("bool")),
                                                            new Cm::Parsing::ActionParser("A1",
                                                                new Cm::Parsing::KeywordParser("sbyte"))),
                                                        new Cm::Parsing::ActionParser("A2",
                                                            new Cm::Parsing::KeywordParser("byte"))),
                                                    new Cm::Parsing::ActionParser("A3",
                                                        new Cm::Parsing::KeywordParser("short"))),
                                                new Cm::Parsing::ActionParser("A4",
                                                    new Cm::Parsing::KeywordParser("ushort"))),
                                            new Cm::Parsing::ActionParser("A5",
                                                new Cm::Parsing::KeywordParser("int"))),
                                        new Cm::Parsing::ActionParser("A6",
                                            new Cm::Parsing::KeywordParser("uint"))),
                                    new Cm::Parsing::ActionParser("A7",
                                        new Cm::Parsing::KeywordParser("long"))),
                                new Cm::Parsing::ActionParser("A8",
                                    new Cm::Parsing::KeywordParser("ulong"))),
                            new Cm::Parsing::ActionParser("A9",
                                new Cm::Parsing::KeywordParser("float"))),
                        new Cm::Parsing::ActionParser("A10",
                            new Cm::Parsing::KeywordParser("double"))),
                    new Cm::Parsing::ActionParser("A11",
                        new Cm::Parsing::KeywordParser("char"))),
                new Cm::Parsing::ActionParser("A12",
                    new Cm::Parsing::KeywordParser("string"))),
            new Cm::Parsing::ActionParser("A13",
                new Cm::Parsing::KeywordParser("void")))));
}

} } // namespace cminor.parser
