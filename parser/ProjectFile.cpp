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

namespace cminor { namespace parser {

using namespace cminor::parsing;

ProjectGrammar* ProjectGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

ProjectGrammar* ProjectGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    ProjectGrammar* grammar(new ProjectGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

ProjectGrammar::ProjectGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("ProjectGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

Project* ProjectGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, std::string config)
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
    stack.push(std::unique_ptr<cminor::parsing::Object>(new ValueObject<std::string>(config)));
    cminor::parsing::Match match = cminor::parsing::Grammar::Parse(scanner, stack, parsingData.get());
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

class ProjectGrammar::ProjectRule : public cminor::parsing::Rule
{
public:
    ProjectRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable("std::string", "config"));
        SetValueTypeName("Project*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> config_value = std::move(stack.top());
        context->config = *static_cast<cminor::parsing::ValueObject<std::string>*>(config_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Project*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ProjectRule>(this, &ProjectRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ProjectRule>(this, &ProjectRule::A1Action));
        cminor::parsing::NonterminalParser* qualified_idNonterminalParser = GetNonterminal("qualified_id");
        qualified_idNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ProjectRule>(this, &ProjectRule::Postqualified_id));
        cminor::parsing::NonterminalParser* declarationNonterminalParser = GetNonterminal("Declaration");
        declarationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ProjectRule>(this, &ProjectRule::PostDeclaration));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Project(context->fromqualified_id, fileName, context->config);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value->AddDeclaration(context->fromDeclaration);
    }
    void Postqualified_id(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromqualified_id_value = std::move(stack.top());
            context->fromqualified_id = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromqualified_id_value.get());
            stack.pop();
        }
    }
    void PostDeclaration(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDeclaration_value = std::move(stack.top());
            context->fromDeclaration = *static_cast<cminor::parsing::ValueObject<ProjectDeclaration*>*>(fromDeclaration_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): config(), value(), fromqualified_id(), fromDeclaration() {}
        std::string config;
        Project* value;
        std::string fromqualified_id;
        ProjectDeclaration* fromDeclaration;
    };
};

class ProjectGrammar::DeclarationRule : public cminor::parsing::Rule
{
public:
    DeclarationRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("ProjectDeclaration*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ProjectDeclaration*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeclarationRule>(this, &DeclarationRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeclarationRule>(this, &DeclarationRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeclarationRule>(this, &DeclarationRule::A2Action));
        cminor::parsing::NonterminalParser* referenceDeclarationNonterminalParser = GetNonterminal("ReferenceDeclaration");
        referenceDeclarationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeclarationRule>(this, &DeclarationRule::PostReferenceDeclaration));
        cminor::parsing::NonterminalParser* sourceFileDeclarationNonterminalParser = GetNonterminal("SourceFileDeclaration");
        sourceFileDeclarationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeclarationRule>(this, &DeclarationRule::PostSourceFileDeclaration));
        cminor::parsing::NonterminalParser* targetDeclarationNonterminalParser = GetNonterminal("TargetDeclaration");
        targetDeclarationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeclarationRule>(this, &DeclarationRule::PostTargetDeclaration));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromReferenceDeclaration;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromSourceFileDeclaration;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromTargetDeclaration;
    }
    void PostReferenceDeclaration(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromReferenceDeclaration_value = std::move(stack.top());
            context->fromReferenceDeclaration = *static_cast<cminor::parsing::ValueObject<ProjectDeclaration*>*>(fromReferenceDeclaration_value.get());
            stack.pop();
        }
    }
    void PostSourceFileDeclaration(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSourceFileDeclaration_value = std::move(stack.top());
            context->fromSourceFileDeclaration = *static_cast<cminor::parsing::ValueObject<ProjectDeclaration*>*>(fromSourceFileDeclaration_value.get());
            stack.pop();
        }
    }
    void PostTargetDeclaration(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTargetDeclaration_value = std::move(stack.top());
            context->fromTargetDeclaration = *static_cast<cminor::parsing::ValueObject<ProjectDeclaration*>*>(fromTargetDeclaration_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromReferenceDeclaration(), fromSourceFileDeclaration(), fromTargetDeclaration() {}
        ProjectDeclaration* value;
        ProjectDeclaration* fromReferenceDeclaration;
        ProjectDeclaration* fromSourceFileDeclaration;
        ProjectDeclaration* fromTargetDeclaration;
    };
};

class ProjectGrammar::ReferenceDeclarationRule : public cminor::parsing::Rule
{
public:
    ReferenceDeclarationRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("ProjectDeclaration*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ProjectDeclaration*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ReferenceDeclarationRule>(this, &ReferenceDeclarationRule::A0Action));
        cminor::parsing::NonterminalParser* filePathNonterminalParser = GetNonterminal("FilePath");
        filePathNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ReferenceDeclarationRule>(this, &ReferenceDeclarationRule::PostFilePath));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ReferenceDeclaration(context->fromFilePath);
    }
    void PostFilePath(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFilePath_value = std::move(stack.top());
            context->fromFilePath = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromFilePath_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromFilePath() {}
        ProjectDeclaration* value;
        std::string fromFilePath;
    };
};

class ProjectGrammar::SourceFileDeclarationRule : public cminor::parsing::Rule
{
public:
    SourceFileDeclarationRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("ProjectDeclaration*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ProjectDeclaration*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SourceFileDeclarationRule>(this, &SourceFileDeclarationRule::A0Action));
        cminor::parsing::NonterminalParser* filePathNonterminalParser = GetNonterminal("FilePath");
        filePathNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SourceFileDeclarationRule>(this, &SourceFileDeclarationRule::PostFilePath));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new SourceFileDeclaration(context->fromFilePath);
    }
    void PostFilePath(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFilePath_value = std::move(stack.top());
            context->fromFilePath = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromFilePath_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromFilePath() {}
        ProjectDeclaration* value;
        std::string fromFilePath;
    };
};

class ProjectGrammar::TargetDeclarationRule : public cminor::parsing::Rule
{
public:
    TargetDeclarationRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("ProjectDeclaration*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ProjectDeclaration*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TargetDeclarationRule>(this, &TargetDeclarationRule::A0Action));
        cminor::parsing::NonterminalParser* targetNonterminalParser = GetNonterminal("Target");
        targetNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TargetDeclarationRule>(this, &TargetDeclarationRule::PostTarget));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new TargetDeclaration(context->fromTarget);
    }
    void PostTarget(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTarget_value = std::move(stack.top());
            context->fromTarget = *static_cast<cminor::parsing::ValueObject<Target>*>(fromTarget_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromTarget() {}
        ProjectDeclaration* value;
        Target fromTarget;
    };
};

class ProjectGrammar::TargetRule : public cminor::parsing::Rule
{
public:
    TargetRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("Target");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Target>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TargetRule>(this, &TargetRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TargetRule>(this, &TargetRule::A1Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Target::program;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = Target::library;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        Target value;
    };
};

class ProjectGrammar::FilePathRule : public cminor::parsing::Rule
{
public:
    FilePathRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("std::string");
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
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FilePathRule>(this, &FilePathRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = std::string(matchBegin, matchEnd);
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        std::string value;
    };
};

void ProjectGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("cminor.parsing.stdlib");
    if (!grammar0)
    {
        grammar0 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar0);
}

void ProjectGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("qualified_id", this, "cminor.parsing.stdlib.qualified_id"));
    AddRuleLink(new cminor::parsing::RuleLink("spaces_and_comments", this, "cminor.parsing.stdlib.spaces_and_comments"));
    AddRule(new ProjectRule("Project", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser("project"),
                    new cminor::parsing::NonterminalParser("qualified_id", "qualified_id", 0)),
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::CharParser(';'))),
            new cminor::parsing::KleeneStarParser(
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("Declaration", "Declaration", 0))))));
    AddRule(new DeclarationRule("Declaration", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::NonterminalParser("ReferenceDeclaration", "ReferenceDeclaration", 0)),
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("SourceFileDeclaration", "SourceFileDeclaration", 0))),
            new cminor::parsing::ActionParser("A2",
                new cminor::parsing::NonterminalParser("TargetDeclaration", "TargetDeclaration", 0)))));
    AddRule(new ReferenceDeclarationRule("ReferenceDeclaration", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser("reference"),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("FilePath", "FilePath", 0))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new SourceFileDeclarationRule("SourceFileDeclaration", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser("source"),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("FilePath", "FilePath", 0))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new TargetDeclarationRule("TargetDeclaration", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser("target"),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser('='))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("Target", "Target", 0))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new TargetRule("Target", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::KeywordParser("program")),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::KeywordParser("library")))));
    AddRule(new FilePathRule("FilePath", GetScope(), GetParsingDomain()->GetNextRuleId(),
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
