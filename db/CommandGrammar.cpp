#include "CommandGrammar.hpp"
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

namespace cminor { namespace db {

using namespace cminor::parsing;
using namespace cminor::util;
using namespace cminor::unicode;

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

CommandGrammar::CommandGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("CommandGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.db")), parsingDomain_)
{
    SetOwner(0);
}

Command* CommandGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName)
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
    Command* result = *static_cast<cminor::parsing::ValueObject<Command*>*>(value.get());
    stack.pop();
    return result;
}

class CommandGrammar::CommandRule : public cminor::parsing::Rule
{
public:
    CommandRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("Command*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Command*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction(ToUtf32("A4"));
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction(ToUtf32("A5"));
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction(ToUtf32("A6"));
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction(ToUtf32("A7"));
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction(ToUtf32("A8"));
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A8Action));
        cminor::parsing::ActionParser* a9ActionParser = GetAction(ToUtf32("A9"));
        a9ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A9Action));
        cminor::parsing::ActionParser* a10ActionParser = GetAction(ToUtf32("A10"));
        a10ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A10Action));
        cminor::parsing::ActionParser* a11ActionParser = GetAction(ToUtf32("A11"));
        a11ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A11Action));
        cminor::parsing::ActionParser* a12ActionParser = GetAction(ToUtf32("A12"));
        a12ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A12Action));
        cminor::parsing::ActionParser* a13ActionParser = GetAction(ToUtf32("A13"));
        a13ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A13Action));
        cminor::parsing::ActionParser* a14ActionParser = GetAction(ToUtf32("A14"));
        a14ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A14Action));
        cminor::parsing::ActionParser* a15ActionParser = GetAction(ToUtf32("A15"));
        a15ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A15Action));
        cminor::parsing::ActionParser* a16ActionParser = GetAction(ToUtf32("A16"));
        a16ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A16Action));
        cminor::parsing::ActionParser* a17ActionParser = GetAction(ToUtf32("A17"));
        a17ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A17Action));
        cminor::parsing::ActionParser* a18ActionParser = GetAction(ToUtf32("A18"));
        a18ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A18Action));
        cminor::parsing::ActionParser* a19ActionParser = GetAction(ToUtf32("A19"));
        a19ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A19Action));
        cminor::parsing::ActionParser* a20ActionParser = GetAction(ToUtf32("A20"));
        a20ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CommandRule>(this, &CommandRule::A20Action));
        cminor::parsing::NonterminalParser* lfpNonterminalParser = GetNonterminal(ToUtf32("lfp"));
        lfpNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postlfp));
        cminor::parsing::NonterminalParser* lineNonterminalParser = GetNonterminal(ToUtf32("line"));
        lineNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postline));
        cminor::parsing::NonterminalParser* llineNonterminalParser = GetNonterminal(ToUtf32("lline"));
        llineNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postlline));
        cminor::parsing::NonterminalParser* nameNonterminalParser = GetNonterminal(ToUtf32("name"));
        nameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postname));
        cminor::parsing::NonterminalParser* lnNonterminalParser = GetNonterminal(ToUtf32("ln"));
        lnNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postln));
        cminor::parsing::NonterminalParser* snNonterminalParser = GetNonterminal(ToUtf32("sn"));
        snNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postsn));
        cminor::parsing::NonterminalParser* ahNonterminalParser = GetNonterminal(ToUtf32("ah"));
        ahNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postah));
        cminor::parsing::NonterminalParser* oNonterminalParser = GetNonterminal(ToUtf32("o"));
        oNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Posto));
        cminor::parsing::NonterminalParser* fNonterminalParser = GetNonterminal(ToUtf32("f"));
        fNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postf));
        cminor::parsing::NonterminalParser* bfpNonterminalParser = GetNonterminal(ToUtf32("bfp"));
        bfpNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postbfp));
        cminor::parsing::NonterminalParser* bplineNonterminalParser = GetNonterminal(ToUtf32("bpline"));
        bplineNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postbpline));
        cminor::parsing::NonterminalParser* bpline2NonterminalParser = GetNonterminal(ToUtf32("bpline2"));
        bpline2NonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postbpline2));
        cminor::parsing::NonterminalParser* bpnumNonterminalParser = GetNonterminal(ToUtf32("bpnum"));
        bpnumNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CommandRule>(this, &CommandRule::Postbpnum));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new StartCommand;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ShowBreakpointsCommand;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new StackCommand;
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new HelpCommand;
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ExitCommand;
    }
    void A5Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ExitCommand;
    }
    void A6Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new StepCommand;
    }
    void A7Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new NextCommand;
    }
    void A8Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new RunCommand;
    }
    void A9Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ListCommand(context->fromlfp, context->fromline);
    }
    void A10Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ListCommand("", context->fromlline);
    }
    void A11Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ListCommand("", -1);
    }
    void A12Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new PrintCommand(context->fromname);
    }
    void A13Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new LocalCommand(context->fromln);
    }
    void A14Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new OperandCommand(context->fromsn);
    }
    void A15Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new AllocationCommand(context->fromah);
    }
    void A16Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new FieldCommand(context->fromo, context->fromf);
    }
    void A17Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BreakCommand(context->frombfp, context->frombpline);
    }
    void A18Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BreakCommand("", context->frombpline2);
    }
    void A19Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ClearCommand(context->frombpnum);
    }
    void A20Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new PrevCommand;
    }
    void Postlfp(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromlfp_value = std::move(stack.top());
            context->fromlfp = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromlfp_value.get());
            stack.pop();
        }
    }
    void Postline(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromline_value = std::move(stack.top());
            context->fromline = *static_cast<cminor::parsing::ValueObject<int32_t>*>(fromline_value.get());
            stack.pop();
        }
    }
    void Postlline(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromlline_value = std::move(stack.top());
            context->fromlline = *static_cast<cminor::parsing::ValueObject<int32_t>*>(fromlline_value.get());
            stack.pop();
        }
    }
    void Postname(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromname_value = std::move(stack.top());
            context->fromname = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromname_value.get());
            stack.pop();
        }
    }
    void Postln(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromln_value = std::move(stack.top());
            context->fromln = *static_cast<cminor::parsing::ValueObject<int32_t>*>(fromln_value.get());
            stack.pop();
        }
    }
    void Postsn(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromsn_value = std::move(stack.top());
            context->fromsn = *static_cast<cminor::parsing::ValueObject<int32_t>*>(fromsn_value.get());
            stack.pop();
        }
    }
    void Postah(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromah_value = std::move(stack.top());
            context->fromah = *static_cast<cminor::parsing::ValueObject<int32_t>*>(fromah_value.get());
            stack.pop();
        }
    }
    void Posto(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromo_value = std::move(stack.top());
            context->fromo = *static_cast<cminor::parsing::ValueObject<int32_t>*>(fromo_value.get());
            stack.pop();
        }
    }
    void Postf(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromf_value = std::move(stack.top());
            context->fromf = *static_cast<cminor::parsing::ValueObject<int32_t>*>(fromf_value.get());
            stack.pop();
        }
    }
    void Postbfp(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frombfp_value = std::move(stack.top());
            context->frombfp = *static_cast<cminor::parsing::ValueObject<std::string>*>(frombfp_value.get());
            stack.pop();
        }
    }
    void Postbpline(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frombpline_value = std::move(stack.top());
            context->frombpline = *static_cast<cminor::parsing::ValueObject<int32_t>*>(frombpline_value.get());
            stack.pop();
        }
    }
    void Postbpline2(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frombpline2_value = std::move(stack.top());
            context->frombpline2 = *static_cast<cminor::parsing::ValueObject<int32_t>*>(frombpline2_value.get());
            stack.pop();
        }
    }
    void Postbpnum(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frombpnum_value = std::move(stack.top());
            context->frombpnum = *static_cast<cminor::parsing::ValueObject<int32_t>*>(frombpnum_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromlfp(), fromline(), fromlline(), fromname(), fromln(), fromsn(), fromah(), fromo(), fromf(), frombfp(), frombpline(), frombpline2(), frombpnum() {}
        Command* value;
        std::string fromlfp;
        int32_t fromline;
        int32_t fromlline;
        std::u32string fromname;
        int32_t fromln;
        int32_t fromsn;
        int32_t fromah;
        int32_t fromo;
        int32_t fromf;
        std::string frombfp;
        int32_t frombpline;
        int32_t frombpline2;
        int32_t frombpnum;
    };
};

class CommandGrammar::FilePathRule : public cminor::parsing::Rule
{
public:
    FilePathRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("std::string"));
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
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FilePathRule>(this, &FilePathRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FilePathRule>(this, &FilePathRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FilePathRule>(this, &FilePathRule::A2Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = std::string(matchBegin, matchEnd);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = ToUtf8(std::u32string(matchBegin, matchEnd));
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = ToUtf8(std::u32string(matchBegin, matchEnd));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        std::string value;
    };
};

class CommandGrammar::QualifiedIdRule : public cminor::parsing::Rule
{
public:
    QualifiedIdRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<QualifiedIdRule>(this, &QualifiedIdRule::A0Action));
        cminor::parsing::NonterminalParser* qualified_idNonterminalParser = GetNonterminal(ToUtf32("qualified_id"));
        qualified_idNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<QualifiedIdRule>(this, &QualifiedIdRule::Postqualified_id));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromqualified_id;
    }
    void Postqualified_id(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromqualified_id_value = std::move(stack.top());
            context->fromqualified_id = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromqualified_id_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromqualified_id() {}
        std::u32string value;
        std::u32string fromqualified_id;
    };
};

void CommandGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cminor.parsing.stdlib"));
    if (!grammar0)
    {
        grammar0 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar0);
}

void CommandGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("int"), this, ToUtf32("cminor.parsing.stdlib.int")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("qualified_id"), this, ToUtf32("cminor.parsing.stdlib.qualified_id")));
    AddRule(new CommandRule(ToUtf32("Command"), GetScope(), GetParsingDomain()->GetNextRuleId(),
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
                                                                                    new cminor::parsing::AlternativeParser(
                                                                                        new cminor::parsing::ActionParser(ToUtf32("A0"),
                                                                                            new cminor::parsing::StringParser(ToUtf32("start"))),
                                                                                        new cminor::parsing::ActionParser(ToUtf32("A1"),
                                                                                            new cminor::parsing::SequenceParser(
                                                                                                new cminor::parsing::StringParser(ToUtf32("show")),
                                                                                                new cminor::parsing::StringParser(ToUtf32("breakpoints"))))),
                                                                                    new cminor::parsing::ActionParser(ToUtf32("A2"),
                                                                                        new cminor::parsing::AlternativeParser(
                                                                                            new cminor::parsing::StringParser(ToUtf32("stack")),
                                                                                            new cminor::parsing::StringParser(ToUtf32("k"))))),
                                                                                new cminor::parsing::ActionParser(ToUtf32("A3"),
                                                                                    new cminor::parsing::AlternativeParser(
                                                                                        new cminor::parsing::StringParser(ToUtf32("help")),
                                                                                        new cminor::parsing::StringParser(ToUtf32("h"))))),
                                                                            new cminor::parsing::ActionParser(ToUtf32("A4"),
                                                                                new cminor::parsing::AlternativeParser(
                                                                                    new cminor::parsing::StringParser(ToUtf32("exit")),
                                                                                    new cminor::parsing::StringParser(ToUtf32("e"))))),
                                                                        new cminor::parsing::ActionParser(ToUtf32("A5"),
                                                                            new cminor::parsing::AlternativeParser(
                                                                                new cminor::parsing::StringParser(ToUtf32("quit")),
                                                                                new cminor::parsing::StringParser(ToUtf32("q"))))),
                                                                    new cminor::parsing::ActionParser(ToUtf32("A6"),
                                                                        new cminor::parsing::AlternativeParser(
                                                                            new cminor::parsing::StringParser(ToUtf32("step")),
                                                                            new cminor::parsing::StringParser(ToUtf32("s"))))),
                                                                new cminor::parsing::ActionParser(ToUtf32("A7"),
                                                                    new cminor::parsing::AlternativeParser(
                                                                        new cminor::parsing::StringParser(ToUtf32("next")),
                                                                        new cminor::parsing::StringParser(ToUtf32("n"))))),
                                                            new cminor::parsing::ActionParser(ToUtf32("A8"),
                                                                new cminor::parsing::AlternativeParser(
                                                                    new cminor::parsing::StringParser(ToUtf32("run")),
                                                                    new cminor::parsing::StringParser(ToUtf32("r"))))),
                                                        new cminor::parsing::ActionParser(ToUtf32("A9"),
                                                            new cminor::parsing::SequenceParser(
                                                                new cminor::parsing::AlternativeParser(
                                                                    new cminor::parsing::StringParser(ToUtf32("list")),
                                                                    new cminor::parsing::StringParser(ToUtf32("t"))),
                                                                new cminor::parsing::TokenParser(
                                                                    new cminor::parsing::SequenceParser(
                                                                        new cminor::parsing::SequenceParser(
                                                                            new cminor::parsing::NonterminalParser(ToUtf32("lfp"), ToUtf32("FilePath"), 0),
                                                                            new cminor::parsing::CharParser(':')),
                                                                        new cminor::parsing::NonterminalParser(ToUtf32("line"), ToUtf32("int"), 0)))))),
                                                    new cminor::parsing::ActionParser(ToUtf32("A10"),
                                                        new cminor::parsing::SequenceParser(
                                                            new cminor::parsing::AlternativeParser(
                                                                new cminor::parsing::StringParser(ToUtf32("list")),
                                                                new cminor::parsing::StringParser(ToUtf32("t"))),
                                                            new cminor::parsing::NonterminalParser(ToUtf32("lline"), ToUtf32("int"), 0)))),
                                                new cminor::parsing::ActionParser(ToUtf32("A11"),
                                                    new cminor::parsing::AlternativeParser(
                                                        new cminor::parsing::StringParser(ToUtf32("list")),
                                                        new cminor::parsing::StringParser(ToUtf32("t"))))),
                                            new cminor::parsing::ActionParser(ToUtf32("A12"),
                                                new cminor::parsing::SequenceParser(
                                                    new cminor::parsing::AlternativeParser(
                                                        new cminor::parsing::StringParser(ToUtf32("print")),
                                                        new cminor::parsing::StringParser(ToUtf32("p"))),
                                                    new cminor::parsing::NonterminalParser(ToUtf32("name"), ToUtf32("QualifiedId"), 0)))),
                                        new cminor::parsing::ActionParser(ToUtf32("A13"),
                                            new cminor::parsing::SequenceParser(
                                                new cminor::parsing::AlternativeParser(
                                                    new cminor::parsing::StringParser(ToUtf32("local")),
                                                    new cminor::parsing::StringParser(ToUtf32("l"))),
                                                new cminor::parsing::NonterminalParser(ToUtf32("ln"), ToUtf32("int"), 0)))),
                                    new cminor::parsing::ActionParser(ToUtf32("A14"),
                                        new cminor::parsing::SequenceParser(
                                            new cminor::parsing::AlternativeParser(
                                                new cminor::parsing::StringParser(ToUtf32("operand")),
                                                new cminor::parsing::StringParser(ToUtf32("o"))),
                                            new cminor::parsing::NonterminalParser(ToUtf32("sn"), ToUtf32("int"), 0)))),
                                new cminor::parsing::ActionParser(ToUtf32("A15"),
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::AlternativeParser(
                                            new cminor::parsing::StringParser(ToUtf32("allocation")),
                                            new cminor::parsing::StringParser(ToUtf32("a"))),
                                        new cminor::parsing::NonterminalParser(ToUtf32("ah"), ToUtf32("int"), 0)))),
                            new cminor::parsing::ActionParser(ToUtf32("A16"),
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::AlternativeParser(
                                            new cminor::parsing::StringParser(ToUtf32("field")),
                                            new cminor::parsing::StringParser(ToUtf32("f"))),
                                        new cminor::parsing::NonterminalParser(ToUtf32("o"), ToUtf32("int"), 0)),
                                    new cminor::parsing::NonterminalParser(ToUtf32("f"), ToUtf32("int"), 0)))),
                        new cminor::parsing::ActionParser(ToUtf32("A17"),
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::AlternativeParser(
                                    new cminor::parsing::StringParser(ToUtf32("break")),
                                    new cminor::parsing::StringParser(ToUtf32("b"))),
                                new cminor::parsing::TokenParser(
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::SequenceParser(
                                            new cminor::parsing::NonterminalParser(ToUtf32("bfp"), ToUtf32("FilePath"), 0),
                                            new cminor::parsing::CharParser(':')),
                                        new cminor::parsing::NonterminalParser(ToUtf32("bpline"), ToUtf32("int"), 0)))))),
                    new cminor::parsing::ActionParser(ToUtf32("A18"),
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::StringParser(ToUtf32("break")),
                                new cminor::parsing::StringParser(ToUtf32("b"))),
                            new cminor::parsing::TokenParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("bpline2"), ToUtf32("int"), 0))))),
                new cminor::parsing::ActionParser(ToUtf32("A19"),
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::StringParser(ToUtf32("clear")),
                            new cminor::parsing::StringParser(ToUtf32("c"))),
                        new cminor::parsing::NonterminalParser(ToUtf32("bpnum"), ToUtf32("int"), 0)))),
            new cminor::parsing::ActionParser(ToUtf32("A20"),
                new cminor::parsing::EmptyParser()))));
    AddRule(new cminor::parsing::Rule(ToUtf32("Spaces"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::PositiveParser(
            new cminor::parsing::CharSetParser(ToUtf32(" \t")))));
    AddRule(new FilePathRule(ToUtf32("FilePath"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::ActionParser(ToUtf32("A0"),
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::CharSetParser(ToUtf32("a-zA-Z")),
                                    new cminor::parsing::CharParser(':'))),
                            new cminor::parsing::PositiveParser(
                                new cminor::parsing::CharSetParser(ToUtf32(": "), true)))),
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::CharParser('\"'),
                            new cminor::parsing::ActionParser(ToUtf32("A1"),
                                new cminor::parsing::PositiveParser(
                                    new cminor::parsing::CharSetParser(ToUtf32("\""), true)))),
                        new cminor::parsing::CharParser('\"'))),
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::CharParser('<'),
                        new cminor::parsing::ActionParser(ToUtf32("A2"),
                            new cminor::parsing::PositiveParser(
                                new cminor::parsing::CharSetParser(ToUtf32(">"), true)))),
                    new cminor::parsing::CharParser('>'))))));
    AddRule(new QualifiedIdRule(ToUtf32("QualifiedId"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::NonterminalParser(ToUtf32("qualified_id"), ToUtf32("qualified_id"), 0))));
    SetSkipRuleName(ToUtf32("Spaces"));
}

} } // namespace cminor.db
