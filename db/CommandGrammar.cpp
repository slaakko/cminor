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
        cminor::parsing::ActionParser* a9ActionParser = GetAction("A9");
        a9ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A9Action));
        cminor::parsing::ActionParser* a10ActionParser = GetAction("A10");
        a10ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A10Action));
        cminor::parsing::ActionParser* a11ActionParser = GetAction("A11");
        a11ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A11Action));
        cminor::parsing::ActionParser* a12ActionParser = GetAction("A12");
        a12ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A12Action));
        cminor::parsing::ActionParser* a13ActionParser = GetAction("A13");
        a13ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A13Action));
        cminor::parsing::ActionParser* a14ActionParser = GetAction("A14");
        a14ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A14Action));
        cminor::parsing::ActionParser* a15ActionParser = GetAction("A15");
        a15ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A15Action));
        cminor::parsing::ActionParser* a16ActionParser = GetAction("A16");
        a16ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A16Action));
        cminor::parsing::ActionParser* a17ActionParser = GetAction("A17");
        a17ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A17Action));
        cminor::parsing::ActionParser* a18ActionParser = GetAction("A18");
        a18ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A18Action));
        cminor::parsing::ActionParser* a19ActionParser = GetAction("A19");
        a19ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A19Action));
        cminor::parsing::NonterminalParser* lfpNonterminalParser = GetNonterminal("lfp");
        lfpNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postlfp));
        cminor::parsing::NonterminalParser* lineNonterminalParser = GetNonterminal("line");
        lineNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postline));
        cminor::parsing::NonterminalParser* llineNonterminalParser = GetNonterminal("lline");
        llineNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postlline));
        cminor::parsing::NonterminalParser* lnNonterminalParser = GetNonterminal("ln");
        lnNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postln));
        cminor::parsing::NonterminalParser* snNonterminalParser = GetNonterminal("sn");
        snNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postsn));
        cminor::parsing::NonterminalParser* ahNonterminalParser = GetNonterminal("ah");
        ahNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postah));
        cminor::parsing::NonterminalParser* oNonterminalParser = GetNonterminal("o");
        oNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Posto));
        cminor::parsing::NonterminalParser* fNonterminalParser = GetNonterminal("f");
        fNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postf));
        cminor::parsing::NonterminalParser* bfpNonterminalParser = GetNonterminal("bfp");
        bfpNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postbfp));
        cminor::parsing::NonterminalParser* bplineNonterminalParser = GetNonterminal("bpline");
        bplineNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postbpline));
        cminor::parsing::NonterminalParser* bpline2NonterminalParser = GetNonterminal("bpline2");
        bpline2NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postbpline2));
        cminor::parsing::NonterminalParser* bpnumNonterminalParser = GetNonterminal("bpnum");
        bpnumNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postbpnum));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new StartCommand;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ShowBreakpointsCommand;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new StackCommand;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new HelpCommand;
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ExitCommand;
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ExitCommand;
    }
    void A6Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new StepCommand;
    }
    void A7Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new NextCommand;
    }
    void A8Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new RunCommand;
    }
    void A9Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ListCommand(context.fromlfp, context.fromline);
    }
    void A10Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ListCommand("", context.fromlline);
    }
    void A11Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ListCommand("", -1);
    }
    void A12Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new LocalCommand(context.fromln);
    }
    void A13Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new OperandCommand(context.fromsn);
    }
    void A14Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new AllocationCommand(context.fromah);
    }
    void A15Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new FieldCommand(context.fromo, context.fromf);
    }
    void A16Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new BreakCommand(context.frombfp, context.frombpline);
    }
    void A17Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new BreakCommand("", context.frombpline2);
    }
    void A18Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ClearCommand(context.frombpnum);
    }
    void A19Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new PrevCommand;
    }
    void Postlfp(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromlfp_value = std::move(stack.top());
            context.fromlfp = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromlfp_value.get());
            stack.pop();
        }
    }
    void Postline(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromline_value = std::move(stack.top());
            context.fromline = *static_cast<cminor::parsing::ValueObject<int>*>(fromline_value.get());
            stack.pop();
        }
    }
    void Postlline(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromlline_value = std::move(stack.top());
            context.fromlline = *static_cast<cminor::parsing::ValueObject<int>*>(fromlline_value.get());
            stack.pop();
        }
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
    void Postah(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromah_value = std::move(stack.top());
            context.fromah = *static_cast<cminor::parsing::ValueObject<int>*>(fromah_value.get());
            stack.pop();
        }
    }
    void Posto(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromo_value = std::move(stack.top());
            context.fromo = *static_cast<cminor::parsing::ValueObject<int>*>(fromo_value.get());
            stack.pop();
        }
    }
    void Postf(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromf_value = std::move(stack.top());
            context.fromf = *static_cast<cminor::parsing::ValueObject<int>*>(fromf_value.get());
            stack.pop();
        }
    }
    void Postbfp(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frombfp_value = std::move(stack.top());
            context.frombfp = *static_cast<cminor::parsing::ValueObject<std::string>*>(frombfp_value.get());
            stack.pop();
        }
    }
    void Postbpline(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frombpline_value = std::move(stack.top());
            context.frombpline = *static_cast<cminor::parsing::ValueObject<int>*>(frombpline_value.get());
            stack.pop();
        }
    }
    void Postbpline2(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frombpline2_value = std::move(stack.top());
            context.frombpline2 = *static_cast<cminor::parsing::ValueObject<int>*>(frombpline2_value.get());
            stack.pop();
        }
    }
    void Postbpnum(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frombpnum_value = std::move(stack.top());
            context.frombpnum = *static_cast<cminor::parsing::ValueObject<int>*>(frombpnum_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromlfp(), fromline(), fromlline(), fromln(), fromsn(), fromah(), fromo(), fromf(), frombfp(), frombpline(), frombpline2(), frombpnum() {}
        Command* value;
        std::string fromlfp;
        int fromline;
        int fromlline;
        int fromln;
        int fromsn;
        int fromah;
        int fromo;
        int fromf;
        std::string frombfp;
        int frombpline;
        int frombpline2;
        int frombpnum;
    };
    std::stack<Context> contextStack;
    Context context;
};

class CommandGrammar::FilePathRule : public cminor::parsing::Rule
{
public:
    FilePathRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("std::string");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::string>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FilePathRule>(this, &FilePathRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FilePathRule>(this, &FilePathRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FilePathRule>(this, &FilePathRule::A2Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = std::string(matchBegin, matchEnd);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = std::string(matchBegin, matchEnd);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = std::string(matchBegin, matchEnd);
    }
private:
    struct Context
    {
        Context(): value() {}
        std::string value;
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
                                                                                        new cminor::parsing::StringParser("start")),
                                                                                    new cminor::parsing::ActionParser("A1",
                                                                                        new cminor::parsing::SequenceParser(
                                                                                            new cminor::parsing::StringParser("show"),
                                                                                            new cminor::parsing::StringParser("breakpoints")))),
                                                                                new cminor::parsing::ActionParser("A2",
                                                                                    new cminor::parsing::AlternativeParser(
                                                                                        new cminor::parsing::StringParser("stack"),
                                                                                        new cminor::parsing::StringParser("k")))),
                                                                            new cminor::parsing::ActionParser("A3",
                                                                                new cminor::parsing::AlternativeParser(
                                                                                    new cminor::parsing::StringParser("help"),
                                                                                    new cminor::parsing::StringParser("h")))),
                                                                        new cminor::parsing::ActionParser("A4",
                                                                            new cminor::parsing::AlternativeParser(
                                                                                new cminor::parsing::StringParser("exit"),
                                                                                new cminor::parsing::StringParser("e")))),
                                                                    new cminor::parsing::ActionParser("A5",
                                                                        new cminor::parsing::AlternativeParser(
                                                                            new cminor::parsing::StringParser("quit"),
                                                                            new cminor::parsing::StringParser("q")))),
                                                                new cminor::parsing::ActionParser("A6",
                                                                    new cminor::parsing::AlternativeParser(
                                                                        new cminor::parsing::StringParser("step"),
                                                                        new cminor::parsing::StringParser("s")))),
                                                            new cminor::parsing::ActionParser("A7",
                                                                new cminor::parsing::AlternativeParser(
                                                                    new cminor::parsing::StringParser("next"),
                                                                    new cminor::parsing::StringParser("n")))),
                                                        new cminor::parsing::ActionParser("A8",
                                                            new cminor::parsing::AlternativeParser(
                                                                new cminor::parsing::StringParser("run"),
                                                                new cminor::parsing::StringParser("r")))),
                                                    new cminor::parsing::ActionParser("A9",
                                                        new cminor::parsing::SequenceParser(
                                                            new cminor::parsing::AlternativeParser(
                                                                new cminor::parsing::StringParser("list"),
                                                                new cminor::parsing::StringParser("t")),
                                                            new cminor::parsing::TokenParser(
                                                                new cminor::parsing::SequenceParser(
                                                                    new cminor::parsing::SequenceParser(
                                                                        new cminor::parsing::NonterminalParser("lfp", "FilePath", 0),
                                                                        new cminor::parsing::CharParser(':')),
                                                                    new cminor::parsing::NonterminalParser("line", "int", 0)))))),
                                                new cminor::parsing::ActionParser("A10",
                                                    new cminor::parsing::SequenceParser(
                                                        new cminor::parsing::AlternativeParser(
                                                            new cminor::parsing::StringParser("list"),
                                                            new cminor::parsing::StringParser("t")),
                                                        new cminor::parsing::NonterminalParser("lline", "int", 0)))),
                                            new cminor::parsing::ActionParser("A11",
                                                new cminor::parsing::AlternativeParser(
                                                    new cminor::parsing::StringParser("list"),
                                                    new cminor::parsing::StringParser("t")))),
                                        new cminor::parsing::ActionParser("A12",
                                            new cminor::parsing::SequenceParser(
                                                new cminor::parsing::AlternativeParser(
                                                    new cminor::parsing::StringParser("local"),
                                                    new cminor::parsing::StringParser("l")),
                                                new cminor::parsing::NonterminalParser("ln", "int", 0)))),
                                    new cminor::parsing::ActionParser("A13",
                                        new cminor::parsing::SequenceParser(
                                            new cminor::parsing::AlternativeParser(
                                                new cminor::parsing::StringParser("operand"),
                                                new cminor::parsing::StringParser("o")),
                                            new cminor::parsing::NonterminalParser("sn", "int", 0)))),
                                new cminor::parsing::ActionParser("A14",
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::AlternativeParser(
                                            new cminor::parsing::StringParser("allocation"),
                                            new cminor::parsing::StringParser("a")),
                                        new cminor::parsing::NonterminalParser("ah", "int", 0)))),
                            new cminor::parsing::ActionParser("A15",
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::AlternativeParser(
                                            new cminor::parsing::StringParser("field"),
                                            new cminor::parsing::StringParser("f")),
                                        new cminor::parsing::NonterminalParser("o", "int", 0)),
                                    new cminor::parsing::NonterminalParser("f", "int", 0)))),
                        new cminor::parsing::ActionParser("A16",
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::AlternativeParser(
                                    new cminor::parsing::StringParser("break"),
                                    new cminor::parsing::StringParser("b")),
                                new cminor::parsing::TokenParser(
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::SequenceParser(
                                            new cminor::parsing::NonterminalParser("bfp", "FilePath", 0),
                                            new cminor::parsing::CharParser(':')),
                                        new cminor::parsing::NonterminalParser("bpline", "int", 0)))))),
                    new cminor::parsing::ActionParser("A17",
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::StringParser("break"),
                                new cminor::parsing::StringParser("b")),
                            new cminor::parsing::TokenParser(
                                new cminor::parsing::NonterminalParser("bpline2", "int", 0))))),
                new cminor::parsing::ActionParser("A18",
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::StringParser("clear"),
                            new cminor::parsing::StringParser("c")),
                        new cminor::parsing::NonterminalParser("bpnum", "int", 0)))),
            new cminor::parsing::ActionParser("A19",
                new cminor::parsing::EmptyParser()))));
    AddRule(new cminor::parsing::Rule("Spaces", GetScope(),
        new cminor::parsing::PositiveParser(
            new cminor::parsing::CharSetParser(" \t"))));
    AddRule(new FilePathRule("FilePath", GetScope(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::ActionParser("A0",
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::CharSetParser("a-zA-Z"),
                                    new cminor::parsing::CharParser(':'))),
                            new cminor::parsing::PositiveParser(
                                new cminor::parsing::CharSetParser(": ", true)))),
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::CharParser('\"'),
                            new cminor::parsing::ActionParser("A1",
                                new cminor::parsing::PositiveParser(
                                    new cminor::parsing::CharSetParser("\"", true)))),
                        new cminor::parsing::CharParser('\"'))),
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::CharParser('<'),
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::PositiveParser(
                                new cminor::parsing::CharSetParser(">", true)))),
                    new cminor::parsing::CharParser('>'))))));
    SetSkipRuleName("Spaces");
}

} } // namespace cminor.db
