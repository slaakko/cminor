#include "ProjectFile.hpp"
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>

namespace cpg { namespace syntax {

using namespace cminor::parsing;

ProjectFileGrammar* ProjectFileGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

ProjectFileGrammar* ProjectFileGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    ProjectFileGrammar* grammar(new ProjectFileGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

ProjectFileGrammar::ProjectFileGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("ProjectFileGrammar", parsingDomain_->GetNamespaceScope("Cm.Parsing.Syntax"), parsingDomain_)
{
    SetOwner(0);
}

Project* ProjectFileGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName)
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
    Project* result = *static_cast<cminor::parsing::ValueObject<Project*>*>(value.get());
    stack.pop();
    return result;
}

class ProjectFileGrammar::ProjectFileRule : public cminor::parsing::Rule
{
public:
    ProjectFileRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Project*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Project*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ProjectFileRule>(this, &ProjectFileRule::A0Action));
        cminor::parsing::NonterminalParser* projectNameNonterminalParser = GetNonterminal("projectName");
        projectNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ProjectFileRule>(this, &ProjectFileRule::PostprojectName));
        cminor::parsing::NonterminalParser* projectFileContentNonterminalParser = GetNonterminal("ProjectFileContent");
        projectFileContentNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ProjectFileRule>(this, &ProjectFileRule::PreProjectFileContent));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new Project(context.fromprojectName, fileName);
    }
    void PostprojectName(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromprojectName_value = std::move(stack.top());
            context.fromprojectName = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromprojectName_value.get());
            stack.pop();
        }
    }
    void PreProjectFileContent(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Project*>(context.value)));
    }
private:
    struct Context
    {
        Context(): value(), fromprojectName() {}
        Project* value;
        std::string fromprojectName;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ProjectFileGrammar::ProjectFileContentRule : public cminor::parsing::Rule
{
public:
    ProjectFileContentRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("Project*", "project"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> project_value = std::move(stack.top());
        context.project = *static_cast<cminor::parsing::ValueObject<Project*>*>(project_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ProjectFileContentRule>(this, &ProjectFileContentRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ProjectFileContentRule>(this, &ProjectFileContentRule::A1Action));
        cminor::parsing::NonterminalParser* sourceNonterminalParser = GetNonterminal("Source");
        sourceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ProjectFileContentRule>(this, &ProjectFileContentRule::PostSource));
        cminor::parsing::NonterminalParser* referenceNonterminalParser = GetNonterminal("Reference");
        referenceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ProjectFileContentRule>(this, &ProjectFileContentRule::PostReference));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.project->AddSourceFile(context.fromSource);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.project->AddReferenceFile(context.fromReference);
    }
    void PostSource(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSource_value = std::move(stack.top());
            context.fromSource = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromSource_value.get());
            stack.pop();
        }
    }
    void PostReference(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromReference_value = std::move(stack.top());
            context.fromReference = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromReference_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): project(), fromSource(), fromReference() {}
        Project* project;
        std::string fromSource;
        std::string fromReference;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ProjectFileGrammar::SourceRule : public cminor::parsing::Rule
{
public:
    SourceRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceRule>(this, &SourceRule::A0Action));
        cminor::parsing::NonterminalParser* filePathNonterminalParser = GetNonterminal("FilePath");
        filePathNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SourceRule>(this, &SourceRule::PostFilePath));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromFilePath;
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
        std::string value;
        std::string fromFilePath;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ProjectFileGrammar::ReferenceRule : public cminor::parsing::Rule
{
public:
    ReferenceRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ReferenceRule>(this, &ReferenceRule::A0Action));
        cminor::parsing::NonterminalParser* filePathNonterminalParser = GetNonterminal("FilePath");
        filePathNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ReferenceRule>(this, &ReferenceRule::PostFilePath));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromFilePath;
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
        std::string value;
        std::string fromFilePath;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ProjectFileGrammar::FilePathRule : public cminor::parsing::Rule
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

void ProjectFileGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("Cm.Parsing.stdlib");
    if (!grammar0)
    {
        grammar0 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar0);
}

void ProjectFileGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("qualified_id", this, "Cm.Parsing.stdlib.qualified_id"));
    AddRuleLink(new cminor::parsing::RuleLink("spaces_and_comments", this, "Cm.Parsing.stdlib.spaces_and_comments"));
    AddRule(new ProjectFileRule("ProjectFile", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser("project"),
                    new cminor::parsing::ActionParser("A0",
                        new cminor::parsing::NonterminalParser("projectName", "qualified_id", 0))),
                new cminor::parsing::CharParser(';')),
            new cminor::parsing::NonterminalParser("ProjectFileContent", "ProjectFileContent", 1))));
    AddRule(new ProjectFileContentRule("ProjectFileContent", GetScope(),
        new cminor::parsing::KleeneStarParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::NonterminalParser("Source", "Source", 0)),
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("Reference", "Reference", 0))))));
    AddRule(new SourceRule("Source", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser("source"),
                    new cminor::parsing::NonterminalParser("FilePath", "FilePath", 0)),
                new cminor::parsing::CharParser(';')))));
    AddRule(new ReferenceRule("Reference", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser("reference"),
                    new cminor::parsing::NonterminalParser("FilePath", "FilePath", 0)),
                new cminor::parsing::CharParser(';')))));
    AddRule(new FilePathRule("FilePath", GetScope(),
        new cminor::parsing::TokenParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('<'),
                    new cminor::parsing::ActionParser("A0",
                        new cminor::parsing::PositiveParser(
                            new cminor::parsing::CharSetParser(">\r\n", true)))),
                new cminor::parsing::CharParser('>')))));
    SetSkipRuleName("spaces_and_comments");
}

} } // namespace cpg.syntax
