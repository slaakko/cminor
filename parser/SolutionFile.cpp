#include "SolutionFile.hpp"
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

SolutionGrammar* SolutionGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

SolutionGrammar* SolutionGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    SolutionGrammar* grammar(new SolutionGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

SolutionGrammar::SolutionGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("SolutionGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

Solution* SolutionGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName)
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
    Solution* result = *static_cast<cminor::parsing::ValueObject<Solution*>*>(value.get());
    stack.pop();
    return result;
}

class SolutionGrammar::SolutionRule : public cminor::parsing::Rule
{
public:
    SolutionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Solution*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Solution*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SolutionRule>(this, &SolutionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SolutionRule>(this, &SolutionRule::A1Action));
        cminor::parsing::NonterminalParser* qualified_idNonterminalParser = GetNonterminal("qualified_id");
        qualified_idNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SolutionRule>(this, &SolutionRule::Postqualified_id));
        cminor::parsing::NonterminalParser* declarationNonterminalParser = GetNonterminal("Declaration");
        declarationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SolutionRule>(this, &SolutionRule::PostDeclaration));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new Solution(context.fromqualified_id, fileName);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value->AddDeclaration(context.fromDeclaration);
    }
    void Postqualified_id(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromqualified_id_value = std::move(stack.top());
            context.fromqualified_id = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromqualified_id_value.get());
            stack.pop();
        }
    }
    void PostDeclaration(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDeclaration_value = std::move(stack.top());
            context.fromDeclaration = *static_cast<cminor::parsing::ValueObject<SolutionDeclaration*>*>(fromDeclaration_value.get());
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

class SolutionGrammar::DeclarationRule : public cminor::parsing::Rule
{
public:
    DeclarationRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("SolutionDeclaration*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<SolutionDeclaration*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeclarationRule>(this, &DeclarationRule::A0Action));
        cminor::parsing::NonterminalParser* solutionProjectDeclarationNonterminalParser = GetNonterminal("SolutionProjectDeclaration");
        solutionProjectDeclarationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeclarationRule>(this, &DeclarationRule::PostSolutionProjectDeclaration));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromSolutionProjectDeclaration;
    }
    void PostSolutionProjectDeclaration(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSolutionProjectDeclaration_value = std::move(stack.top());
            context.fromSolutionProjectDeclaration = *static_cast<cminor::parsing::ValueObject<SolutionDeclaration*>*>(fromSolutionProjectDeclaration_value.get());
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

class SolutionGrammar::SolutionProjectDeclarationRule : public cminor::parsing::Rule
{
public:
    SolutionProjectDeclarationRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("SolutionDeclaration*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<SolutionDeclaration*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SolutionProjectDeclarationRule>(this, &SolutionProjectDeclarationRule::A0Action));
        cminor::parsing::NonterminalParser* filePathNonterminalParser = GetNonterminal("FilePath");
        filePathNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SolutionProjectDeclarationRule>(this, &SolutionProjectDeclarationRule::PostFilePath));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new SolutionProjectDeclaration(context.fromFilePath);
    }
    void PostFilePath(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFilePath_value = std::move(stack.top());
            context.fromFilePath = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromFilePath_value.get());
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

class SolutionGrammar::FilePathRule : public cminor::parsing::Rule
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
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("Cm.Parsing.stdlib");
    if (!grammar0)
    {
        grammar0 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar0);
}

void SolutionGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("qualified_id", this, "Cm.Parsing.stdlib.qualified_id"));
    AddRuleLink(new cminor::parsing::RuleLink("spaces_and_comments", this, "Cm.Parsing.stdlib.spaces_and_comments"));
    AddRule(new SolutionRule("Solution", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser("solution"),
                    new cminor::parsing::NonterminalParser("qualified_id", "qualified_id", 0)),
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::CharParser(';'))),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("Declaration", "Declaration", 0))))));
    AddRule(new DeclarationRule("Declaration", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::NonterminalParser("SolutionProjectDeclaration", "SolutionProjectDeclaration", 0))));
    AddRule(new SolutionProjectDeclarationRule("SolutionProjectDeclaration", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser("project"),
                    new cminor::parsing::NonterminalParser("FilePath", "FilePath", 0)),
                new cminor::parsing::CharParser(';')))));
    AddRule(new FilePathRule("FilePath", GetScope(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('<'),
                    new cminor::parsing::ActionParser("A0",
                        new cminor::parsing::PositiveParser(
                            new cminor::parsing::CharSetParser(">", true)))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser('>'))))));
    SetSkipRuleName("spaces_and_comments");
}

} } // namespace cminor.parser
