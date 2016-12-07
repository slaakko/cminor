#include "CommandGrammar.hpp"
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>

namespace cminor { namespace db {

using namespace cminor::parsing;

CommandGrammar* CommandGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

CommandGrammar* CommandGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    CommandGrammar* grammar(new CommandGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

CommandGrammar::CommandGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("CommandGrammar", parsingDomain_->GetNamespaceScope("cminor.db"), parsingDomain_)
{
    SetOwner(0);
}

Command* CommandGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName)
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
    Command* result = *static_cast<cminor::parsing::ValueObject<Command*>*>(value.get());
    stack.pop();
    return result;
}

class CommandGrammar::CommandRule : public cminor::parsing::Rule
{
public:
    CommandRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Command*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Command*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction("A6");
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction("A7");
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction("A8");
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A8Action));
        cminor::parsing::NonterminalParser* lnNonterminalParser = GetNonterminal("ln");
        lnNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postln));
        cminor::parsing::NonterminalParser* snNonterminalParser = GetNonterminal("sn");
        snNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postsn));
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
    void A6Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new LocalCommand(context.fromln);
    }
    void A7Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new StackCommand(context.fromsn);
    }
    void A8Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new PrevCommand;
    }
    void Postln(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromln_value = std::move(stack.top());
            context.fromln = *static_cast<cminor::parsing::ValueObject<int>*>(fromln_value.get());
            stack.pop();
        }
    }
    void Postsn(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromsn_value = std::move(stack.top());
            context.fromsn = *static_cast<cminor::parsing::ValueObject<int>*>(fromsn_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromln(), fromsn() {}
        Command* value;
        int fromln;
        int fromsn;
    };
    std::stack<Context> contextStack;
    Context context;
};

void CommandGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("cminor.parsing.stdlib");
    if (!grammar0)
    {
        grammar0 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar0);
}

void CommandGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("int", this, "cminor.parsing.stdlib.int"));
    AddRule(new CommandRule("Command", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::AlternativeParser(
                                    new cminor::parsing::AlternativeParser(
                                        new cminor::parsing::ActionParser("A0",
                                            new cminor::parsing::StringParser("start")),
                                        new cminor::parsing::ActionParser("A1",
                                            new cminor::parsing::AlternativeParser(
                                                new cminor::parsing::StringParser("help"),
                                                new cminor::parsing::StringParser("h")))),
                                    new cminor::parsing::ActionParser("A2",
                                        new cminor::parsing::AlternativeParser(
                                            new cminor::parsing::StringParser("exit"),
                                            new cminor::parsing::StringParser("e")))),
                                new cminor::parsing::ActionParser("A3",
                                    new cminor::parsing::AlternativeParser(
                                        new cminor::parsing::StringParser("quit"),
                                        new cminor::parsing::StringParser("q")))),
                            new cminor::parsing::ActionParser("A4",
                                new cminor::parsing::AlternativeParser(
                                    new cminor::parsing::StringParser("step"),
                                    new cminor::parsing::StringParser("s")))),
                        new cminor::parsing::ActionParser("A5",
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::StringParser("next"),
                                new cminor::parsing::StringParser("n")))),
                    new cminor::parsing::ActionParser("A6",
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::StringParser("local"),
                                new cminor::parsing::StringParser("l")),
                            new cminor::parsing::NonterminalParser("ln", "int", 0)))),
                new cminor::parsing::ActionParser("A7",
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::StringParser("stack"),
                            new cminor::parsing::StringParser("k")),
                        new cminor::parsing::NonterminalParser("sn", "int", 0)))),
            new cminor::parsing::ActionParser("A8",
                new cminor::parsing::EmptyParser()))));
    AddRule(new cminor::parsing::Rule("Spaces", GetScope(),
        new cminor::parsing::PositiveParser(
            new cminor::parsing::CharSetParser(" \t"))));
    SetSkipRuleName("Spaces");
}

} } // namespace cminor.db
