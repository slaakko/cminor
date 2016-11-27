#include "CommandGrammar.hpp"
#include <Cm.Parsing/Action.hpp>
#include <Cm.Parsing/Rule.hpp>
#include <Cm.Parsing/ParsingDomain.hpp>
#include <Cm.Parsing/Primitive.hpp>
#include <Cm.Parsing/Composite.hpp>
#include <Cm.Parsing/Nonterminal.hpp>
#include <Cm.Parsing/Exception.hpp>
#include <Cm.Parsing/StdLib.hpp>
#include <Cm.Parsing/XmlLog.hpp>

namespace cminor { namespace db {

using namespace Cm::Parsing;

CommandGrammar* CommandGrammar::Create()
{
    return Create(new Cm::Parsing::ParsingDomain());
}

CommandGrammar* CommandGrammar::Create(Cm::Parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    CommandGrammar* grammar(new CommandGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

CommandGrammar::CommandGrammar(Cm::Parsing::ParsingDomain* parsingDomain_): Cm::Parsing::Grammar("CommandGrammar", parsingDomain_->GetNamespaceScope("cminor.db"), parsingDomain_)
{
    SetOwner(0);
}

Command* CommandGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName)
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
    Command* result = *static_cast<Cm::Parsing::ValueObject<Command*>*>(value.get());
    stack.pop();
    return result;
}

class CommandGrammar::CommandRule : public Cm::Parsing::Rule
{
public:
    CommandRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Command*");
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Command*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A3Action));
        Cm::Parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A4Action));
        Cm::Parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A5Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new StartCommand;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new HelpCommand;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ExitCommand;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ExitCommand;
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new StepCommand;
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new NextCommand;
    }
private:
    struct Context
    {
        Context(): value() {}
        Command* value;
    };
    std::stack<Context> contextStack;
    Context context;
};

void CommandGrammar::GetReferencedGrammars()
{
}

void CommandGrammar::CreateRules()
{
    AddRule(new CommandRule("Command", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::AlternativeParser(
                new Cm::Parsing::AlternativeParser(
                    new Cm::Parsing::AlternativeParser(
                        new Cm::Parsing::AlternativeParser(
                            new Cm::Parsing::ActionParser("A0",
                                new Cm::Parsing::StringParser("start")),
                            new Cm::Parsing::ActionParser("A1",
                                new Cm::Parsing::AlternativeParser(
                                    new Cm::Parsing::StringParser("help"),
                                    new Cm::Parsing::StringParser("h")))),
                        new Cm::Parsing::ActionParser("A2",
                            new Cm::Parsing::AlternativeParser(
                                new Cm::Parsing::StringParser("exit"),
                                new Cm::Parsing::StringParser("e")))),
                    new Cm::Parsing::ActionParser("A3",
                        new Cm::Parsing::AlternativeParser(
                            new Cm::Parsing::StringParser("quit"),
                            new Cm::Parsing::StringParser("q")))),
                new Cm::Parsing::ActionParser("A4",
                    new Cm::Parsing::AlternativeParser(
                        new Cm::Parsing::StringParser("step"),
                        new Cm::Parsing::StringParser("s")))),
            new Cm::Parsing::ActionParser("A5",
                new Cm::Parsing::AlternativeParser(
                    new Cm::Parsing::StringParser("next"),
                    new Cm::Parsing::StringParser("n"))))));
}

} } // namespace cminor.db
