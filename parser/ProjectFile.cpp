#include "ProjectFile.hpp"
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

ProjectGrammar* ProjectGrammar::Create()
{
    return Create(new Cm::Parsing::ParsingDomain());
}

ProjectGrammar* ProjectGrammar::Create(Cm::Parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    ProjectGrammar* grammar(new ProjectGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

ProjectGrammar::ProjectGrammar(Cm::Parsing::ParsingDomain* parsingDomain_): Cm::Parsing::Grammar("ProjectGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

Project* ProjectGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName)
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
    Project* result = *static_cast<Cm::Parsing::ValueObject<Project*>*>(value.get());
    stack.pop();
    return result;
}

class ProjectGrammar::ProjectRule : public Cm::Parsing::Rule
{
public:
    ProjectRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Project*");
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Project*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ProjectRule>(this, &ProjectRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ProjectRule>(this, &ProjectRule::A1Action));
        Cm::Parsing::NonterminalParser* qualified_idNonterminalParser = GetNonterminal("qualified_id");
        qualified_idNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ProjectRule>(this, &ProjectRule::Postqualified_id));
        Cm::Parsing::NonterminalParser* declarationNonterminalParser = GetNonterminal("Declaration");
        declarationNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ProjectRule>(this, &ProjectRule::PostDeclaration));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new Project(context.fromqualified_id, fileName);
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
            context.fromDeclaration = *static_cast<Cm::Parsing::ValueObject<ProjectDeclaration*>*>(fromDeclaration_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromqualified_id(), fromDeclaration() {}
        Project* value;
        std::string fromqualified_id;
        ProjectDeclaration* fromDeclaration;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ProjectGrammar::DeclarationRule : public Cm::Parsing::Rule
{
public:
    DeclarationRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("ProjectDeclaration*");
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ProjectDeclaration*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<DeclarationRule>(this, &DeclarationRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<DeclarationRule>(this, &DeclarationRule::A1Action));
        Cm::Parsing::NonterminalParser* assemblyReferenceDeclarationNonterminalParser = GetNonterminal("AssemblyReferenceDeclaration");
        assemblyReferenceDeclarationNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<DeclarationRule>(this, &DeclarationRule::PostAssemblyReferenceDeclaration));
        Cm::Parsing::NonterminalParser* sourceFileDeclarationNonterminalParser = GetNonterminal("SourceFileDeclaration");
        sourceFileDeclarationNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<DeclarationRule>(this, &DeclarationRule::PostSourceFileDeclaration));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromAssemblyReferenceDeclaration;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromSourceFileDeclaration;
    }
    void PostAssemblyReferenceDeclaration(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromAssemblyReferenceDeclaration_value = std::move(stack.top());
            context.fromAssemblyReferenceDeclaration = *static_cast<Cm::Parsing::ValueObject<ProjectDeclaration*>*>(fromAssemblyReferenceDeclaration_value.get());
            stack.pop();
        }
    }
    void PostSourceFileDeclaration(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromSourceFileDeclaration_value = std::move(stack.top());
            context.fromSourceFileDeclaration = *static_cast<Cm::Parsing::ValueObject<ProjectDeclaration*>*>(fromSourceFileDeclaration_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromAssemblyReferenceDeclaration(), fromSourceFileDeclaration() {}
        ProjectDeclaration* value;
        ProjectDeclaration* fromAssemblyReferenceDeclaration;
        ProjectDeclaration* fromSourceFileDeclaration;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ProjectGrammar::AssemblyReferenceDeclarationRule : public Cm::Parsing::Rule
{
public:
    AssemblyReferenceDeclarationRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("ProjectDeclaration*");
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ProjectDeclaration*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<AssemblyReferenceDeclarationRule>(this, &AssemblyReferenceDeclarationRule::A0Action));
        Cm::Parsing::NonterminalParser* filePathNonterminalParser = GetNonterminal("FilePath");
        filePathNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<AssemblyReferenceDeclarationRule>(this, &AssemblyReferenceDeclarationRule::PostFilePath));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new AssemblyReferenceDeclaration(context.fromFilePath);
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
        ProjectDeclaration* value;
        std::string fromFilePath;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ProjectGrammar::SourceFileDeclarationRule : public Cm::Parsing::Rule
{
public:
    SourceFileDeclarationRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("ProjectDeclaration*");
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ProjectDeclaration*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SourceFileDeclarationRule>(this, &SourceFileDeclarationRule::A0Action));
        Cm::Parsing::NonterminalParser* filePathNonterminalParser = GetNonterminal("FilePath");
        filePathNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<SourceFileDeclarationRule>(this, &SourceFileDeclarationRule::PostFilePath));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new SourceFileDeclaration(context.fromFilePath);
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
        ProjectDeclaration* value;
        std::string fromFilePath;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ProjectGrammar::FilePathRule : public Cm::Parsing::Rule
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

void ProjectGrammar::GetReferencedGrammars()
{
    Cm::Parsing::ParsingDomain* pd = GetParsingDomain();
    Cm::Parsing::Grammar* grammar0 = pd->GetGrammar("Cm.Parsing.stdlib");
    if (!grammar0)
    {
        grammar0 = Cm::Parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar0);
}

void ProjectGrammar::CreateRules()
{
    AddRuleLink(new Cm::Parsing::RuleLink("spaces_and_comments", this, "Cm.Parsing.stdlib.spaces_and_comments"));
    AddRuleLink(new Cm::Parsing::RuleLink("qualified_id", this, "Cm.Parsing.stdlib.qualified_id"));
    AddRule(new ProjectRule("Project", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::KeywordParser("project"),
                    new Cm::Parsing::NonterminalParser("qualified_id", "qualified_id", 0)),
                new Cm::Parsing::ActionParser("A0",
                    new Cm::Parsing::CharParser(';'))),
            new Cm::Parsing::KleeneStarParser(
                new Cm::Parsing::ActionParser("A1",
                    new Cm::Parsing::NonterminalParser("Declaration", "Declaration", 0))))));
    AddRule(new DeclarationRule("Declaration", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::ActionParser("A0",
                new Cm::Parsing::NonterminalParser("AssemblyReferenceDeclaration", "AssemblyReferenceDeclaration", 0)),
            new Cm::Parsing::ActionParser("A1",
                new Cm::Parsing::NonterminalParser("SourceFileDeclaration", "SourceFileDeclaration", 0)))));
    AddRule(new AssemblyReferenceDeclarationRule("AssemblyReferenceDeclaration", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::KeywordParser("reference"),
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::NonterminalParser("FilePath", "FilePath", 0))),
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::CharParser(';'))))));
    AddRule(new SourceFileDeclarationRule("SourceFileDeclaration", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::KeywordParser("source"),
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::NonterminalParser("FilePath", "FilePath", 0))),
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::CharParser(';'))))));
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
