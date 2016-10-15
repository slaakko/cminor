#include "SolutionFile.hpp"
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

SolutionGrammar* SolutionGrammar::Create()
{
    return Create(new Cm::Parsing::ParsingDomain());
}

SolutionGrammar* SolutionGrammar::Create(Cm::Parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    SolutionGrammar* grammar(new SolutionGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

SolutionGrammar::SolutionGrammar(Cm::Parsing::ParsingDomain* parsingDomain_): Cm::Parsing::Grammar("SolutionGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

Solution* SolutionGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName)
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
    Solution* result = *static_cast<Cm::Parsing::ValueObject<Solution*>*>(value.get());
    stack.pop();
    return result;
}

class SolutionGrammar::SolutionRule : public Cm::Parsing::Rule
{
public:
    SolutionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Solution*");
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Solution*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SolutionRule>(this, &SolutionRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SolutionRule>(this, &SolutionRule::A1Action));
        Cm::Parsing::NonterminalParser* qualified_idNonterminalParser = GetNonterminal("qualified_id");
        qualified_idNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<SolutionRule>(this, &SolutionRule::Postqualified_id));
        Cm::Parsing::NonterminalParser* declarationNonterminalParser = GetNonterminal("Declaration");
        declarationNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<SolutionRule>(this, &SolutionRule::PostDeclaration));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new Solution(context.fromqualified_id, fileName);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value->AddDeclaration(context.fromDeclaration);
    }
    void Postqualified_id(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromqualified_id_value = std::move(stack.top());
            context.fromqualified_id = *static_cast<Cm::Parsing::ValueObject<std::string>*>(fromqualified_id_value.get());
            stack.pop();
        }
    }
    void PostDeclaration(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromDeclaration_value = std::move(stack.top());
            context.fromDeclaration = *static_cast<Cm::Parsing::ValueObject<SolutionDeclaration*>*>(fromDeclaration_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromqualified_id(), fromDeclaration() {}
        Solution* value;
        std::string fromqualified_id;
        SolutionDeclaration* fromDeclaration;
    };
    std::stack<Context> contextStack;
    Context context;
};

class SolutionGrammar::DeclarationRule : public Cm::Parsing::Rule
{
public:
    DeclarationRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("SolutionDeclaration*");
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<SolutionDeclaration*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<DeclarationRule>(this, &DeclarationRule::A0Action));
        Cm::Parsing::NonterminalParser* solutionProjectDeclarationNonterminalParser = GetNonterminal("SolutionProjectDeclaration");
        solutionProjectDeclarationNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<DeclarationRule>(this, &DeclarationRule::PostSolutionProjectDeclaration));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromSolutionProjectDeclaration;
    }
    void PostSolutionProjectDeclaration(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromSolutionProjectDeclaration_value = std::move(stack.top());
            context.fromSolutionProjectDeclaration = *static_cast<Cm::Parsing::ValueObject<SolutionDeclaration*>*>(fromSolutionProjectDeclaration_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromSolutionProjectDeclaration() {}
        SolutionDeclaration* value;
        SolutionDeclaration* fromSolutionProjectDeclaration;
    };
    std::stack<Context> contextStack;
    Context context;
};

class SolutionGrammar::SolutionProjectDeclarationRule : public Cm::Parsing::Rule
{
public:
    SolutionProjectDeclarationRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("SolutionDeclaration*");
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<SolutionDeclaration*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SolutionProjectDeclarationRule>(this, &SolutionProjectDeclarationRule::A0Action));
        Cm::Parsing::NonterminalParser* filePathNonterminalParser = GetNonterminal("FilePath");
        filePathNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<SolutionProjectDeclarationRule>(this, &SolutionProjectDeclarationRule::PostFilePath));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new SolutionProjectDeclaration(context.fromFilePath);
    }
    void PostFilePath(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromFilePath_value = std::move(stack.top());
            context.fromFilePath = *static_cast<Cm::Parsing::ValueObject<std::string>*>(fromFilePath_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromFilePath() {}
        SolutionDeclaration* value;
        std::string fromFilePath;
    };
    std::stack<Context> contextStack;
    Context context;
};

class SolutionGrammar::FilePathRule : public Cm::Parsing::Rule
{
public:
    FilePathRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("std::string");
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<std::string>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<FilePathRule>(this, &FilePathRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
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

void SolutionGrammar::GetReferencedGrammars()
{
    Cm::Parsing::ParsingDomain* pd = GetParsingDomain();
    Cm::Parsing::Grammar* grammar0 = pd->GetGrammar("Cm.Parsing.stdlib");
    if (!grammar0)
    {
        grammar0 = Cm::Parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar0);
}

void SolutionGrammar::CreateRules()
{
    AddRuleLink(new Cm::Parsing::RuleLink("qualified_id", this, "Cm.Parsing.stdlib.qualified_id"));
    AddRuleLink(new Cm::Parsing::RuleLink("spaces_and_comments", this, "Cm.Parsing.stdlib.spaces_and_comments"));
    AddRule(new SolutionRule("Solution", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::KeywordParser("solution"),
                    new Cm::Parsing::NonterminalParser("qualified_id", "qualified_id", 0)),
                new Cm::Parsing::ActionParser("A0",
                    new Cm::Parsing::CharParser(';'))),
            new Cm::Parsing::KleeneStarParser(
                new Cm::Parsing::ActionParser("A1",
                    new Cm::Parsing::NonterminalParser("Declaration", "Declaration", 0))))));
    AddRule(new DeclarationRule("Declaration", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::NonterminalParser("SolutionProjectDeclaration", "SolutionProjectDeclaration", 0))));
    AddRule(new SolutionProjectDeclarationRule("SolutionProjectDeclaration", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::KeywordParser("project"),
                    new Cm::Parsing::NonterminalParser("FilePath", "FilePath", 0)),
                new Cm::Parsing::CharParser(';')))));
    AddRule(new FilePathRule("FilePath", GetScope(),
        new Cm::Parsing::TokenParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::CharParser('<'),
                    new Cm::Parsing::ActionParser("A0",
                        new Cm::Parsing::PositiveParser(
                            new Cm::Parsing::CharSetParser(">", true)))),
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::CharParser('>'))))));
    SetSkipRuleName("spaces_and_comments");
}

} } // namespace cminor.parser
