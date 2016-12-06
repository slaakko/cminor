#include "ParserFile.hpp"
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>
#include <cminor/cpg/cpgcpp/Declaration.hpp>
#include <cminor/cpg/cpgsyntax/Element.hpp>
#include <cminor/cpg/cpgsyntax/Grammar.hpp>
#include <cminor/pl/Namespace.hpp>
#include <cminor/pl/Grammar.hpp>

namespace cpg { namespace syntax {

using namespace cminor::parsing;

ParserFileGrammar* ParserFileGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

ParserFileGrammar* ParserFileGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    ParserFileGrammar* grammar(new ParserFileGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

ParserFileGrammar::ParserFileGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("ParserFileGrammar", parsingDomain_->GetNamespaceScope("Cm.Parsing.Syntax"), parsingDomain_)
{
    SetOwner(0);
}

ParserFileContent* ParserFileGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, int id_, cminor::parsing::ParsingDomain* parsingDomain_)
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
    stack.push(std::unique_ptr<cminor::parsing::Object>(new ValueObject<int>(id_)));
    stack.push(std::unique_ptr<cminor::parsing::Object>(new ValueObject<cminor::parsing::ParsingDomain*>(parsingDomain_)));
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
    ParserFileContent* result = *static_cast<cminor::parsing::ValueObject<ParserFileContent*>*>(value.get());
    stack.pop();
    return result;
}

class ParserFileGrammar::ParserFileRule : public cminor::parsing::Rule
{
public:
    ParserFileRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("int", "id_"));
        AddInheritedAttribute(AttrOrVariable("cminor::parsing::ParsingDomain*", "parsingDomain_"));
        SetValueTypeName("ParserFileContent*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> parsingDomain__value = std::move(stack.top());
        context.parsingDomain_ = *static_cast<cminor::parsing::ValueObject<cminor::parsing::ParsingDomain*>*>(parsingDomain__value.get());
        stack.pop();
        std::unique_ptr<cminor::parsing::Object> id__value = std::move(stack.top());
        context.id_ = *static_cast<cminor::parsing::ValueObject<int>*>(id__value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParserFileContent*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ParserFileRule>(this, &ParserFileRule::A0Action));
        cminor::parsing::NonterminalParser* includeDirectivesNonterminalParser = GetNonterminal("IncludeDirectives");
        includeDirectivesNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ParserFileRule>(this, &ParserFileRule::PreIncludeDirectives));
        cminor::parsing::NonterminalParser* namespaceContentNonterminalParser = GetNonterminal("NamespaceContent");
        namespaceContentNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ParserFileRule>(this, &ParserFileRule::PreNamespaceContent));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ParserFileContent(context.id_, context.parsingDomain_);
        context.value->SetFilePath(fileName);
    }
    void PreIncludeDirectives(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParserFileContent*>(context.value)));
    }
    void PreNamespaceContent(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParserFileContent*>(context.value)));
    }
private:
    struct Context
    {
        Context(): id_(), parsingDomain_(), value() {}
        int id_;
        cminor::parsing::ParsingDomain* parsingDomain_;
        ParserFileContent* value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ParserFileGrammar::IncludeDirectivesRule : public cminor::parsing::Rule
{
public:
    IncludeDirectivesRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParserFileContent*", "parserFileContent"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> parserFileContent_value = std::move(stack.top());
        context.parserFileContent = *static_cast<cminor::parsing::ValueObject<ParserFileContent*>*>(parserFileContent_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::NonterminalParser* includeDirectiveNonterminalParser = GetNonterminal("IncludeDirective");
        includeDirectiveNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<IncludeDirectivesRule>(this, &IncludeDirectivesRule::PreIncludeDirective));
    }
    void PreIncludeDirective(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParserFileContent*>(context.parserFileContent)));
    }
private:
    struct Context
    {
        Context(): parserFileContent() {}
        ParserFileContent* parserFileContent;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ParserFileGrammar::IncludeDirectiveRule : public cminor::parsing::Rule
{
public:
    IncludeDirectiveRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParserFileContent*", "parserFileContent"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> parserFileContent_value = std::move(stack.top());
        context.parserFileContent = *static_cast<cminor::parsing::ValueObject<ParserFileContent*>*>(parserFileContent_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IncludeDirectiveRule>(this, &IncludeDirectiveRule::A0Action));
        cminor::parsing::NonterminalParser* fileAttributeNonterminalParser = GetNonterminal("FileAttribute");
        fileAttributeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IncludeDirectiveRule>(this, &IncludeDirectiveRule::PostFileAttribute));
        cminor::parsing::NonterminalParser* includeFileNameNonterminalParser = GetNonterminal("IncludeFileName");
        includeFileNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IncludeDirectiveRule>(this, &IncludeDirectiveRule::PostIncludeFileName));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.parserFileContent->AddIncludeDirective(context.fromIncludeFileName, context.fromFileAttribute);
    }
    void PostFileAttribute(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFileAttribute_value = std::move(stack.top());
            context.fromFileAttribute = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromFileAttribute_value.get());
            stack.pop();
        }
    }
    void PostIncludeFileName(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIncludeFileName_value = std::move(stack.top());
            context.fromIncludeFileName = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromIncludeFileName_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): parserFileContent(), fromFileAttribute(), fromIncludeFileName() {}
        ParserFileContent* parserFileContent;
        std::string fromFileAttribute;
        std::string fromIncludeFileName;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ParserFileGrammar::FileAttributeRule : public cminor::parsing::Rule
{
public:
    FileAttributeRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FileAttributeRule>(this, &FileAttributeRule::A0Action));
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

class ParserFileGrammar::IncludeFileNameRule : public cminor::parsing::Rule
{
public:
    IncludeFileNameRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IncludeFileNameRule>(this, &IncludeFileNameRule::A0Action));
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

class ParserFileGrammar::NamespaceContentRule : public cminor::parsing::Rule
{
public:
    NamespaceContentRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParserFileContent*", "parserFileContent"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> parserFileContent_value = std::move(stack.top());
        context.parserFileContent = *static_cast<cminor::parsing::ValueObject<ParserFileContent*>*>(parserFileContent_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NamespaceContentRule>(this, &NamespaceContentRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NamespaceContentRule>(this, &NamespaceContentRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NamespaceContentRule>(this, &NamespaceContentRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NamespaceContentRule>(this, &NamespaceContentRule::A3Action));
        cminor::parsing::NonterminalParser* fileAttributeNonterminalParser = GetNonterminal("FileAttribute");
        fileAttributeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NamespaceContentRule>(this, &NamespaceContentRule::PostFileAttribute));
        cminor::parsing::NonterminalParser* usingDeclarationNonterminalParser = GetNonterminal("UsingDeclaration");
        usingDeclarationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NamespaceContentRule>(this, &NamespaceContentRule::PostUsingDeclaration));
        cminor::parsing::NonterminalParser* usingDirectiveNonterminalParser = GetNonterminal("UsingDirective");
        usingDirectiveNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NamespaceContentRule>(this, &NamespaceContentRule::PostUsingDirective));
        cminor::parsing::NonterminalParser* namespaceAliasDefinitionNonterminalParser = GetNonterminal("NamespaceAliasDefinition");
        namespaceAliasDefinitionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NamespaceContentRule>(this, &NamespaceContentRule::PostNamespaceAliasDefinition));
        cminor::parsing::NonterminalParser* grammarNonterminalParser = GetNonterminal("Grammar");
        grammarNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<NamespaceContentRule>(this, &NamespaceContentRule::PreGrammar));
        grammarNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NamespaceContentRule>(this, &NamespaceContentRule::PostGrammar));
        cminor::parsing::NonterminalParser* namespaceNonterminalParser = GetNonterminal("Namespace");
        namespaceNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<NamespaceContentRule>(this, &NamespaceContentRule::PreNamespace));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.fromUsingDeclaration->SetFileAttr(context.fromFileAttribute);
        context.parserFileContent->AddUsingObject(context.fromUsingDeclaration);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.fromUsingDirective->SetFileAttr(context.fromFileAttribute);
        context.parserFileContent->AddUsingObject(context.fromUsingDirective);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.fromNamespaceAliasDefinition->SetFileAttr(context.fromFileAttribute);
        context.parserFileContent->AddUsingObject(context.fromNamespaceAliasDefinition);
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.parserFileContent->AddGrammar(context.fromGrammar);
    }
    void PostFileAttribute(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFileAttribute_value = std::move(stack.top());
            context.fromFileAttribute = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromFileAttribute_value.get());
            stack.pop();
        }
    }
    void PostUsingDeclaration(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromUsingDeclaration_value = std::move(stack.top());
            context.fromUsingDeclaration = *static_cast<cminor::parsing::ValueObject<cminor::pom::UsingObject*>*>(fromUsingDeclaration_value.get());
            stack.pop();
        }
    }
    void PostUsingDirective(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromUsingDirective_value = std::move(stack.top());
            context.fromUsingDirective = *static_cast<cminor::parsing::ValueObject<cminor::pom::UsingObject*>*>(fromUsingDirective_value.get());
            stack.pop();
        }
    }
    void PostNamespaceAliasDefinition(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromNamespaceAliasDefinition_value = std::move(stack.top());
            context.fromNamespaceAliasDefinition = *static_cast<cminor::parsing::ValueObject<cminor::pom::UsingObject*>*>(fromNamespaceAliasDefinition_value.get());
            stack.pop();
        }
    }
    void PreGrammar(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context.parserFileContent->CurrentScope())));
    }
    void PostGrammar(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromGrammar_value = std::move(stack.top());
            context.fromGrammar = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Grammar*>*>(fromGrammar_value.get());
            stack.pop();
        }
    }
    void PreNamespace(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParserFileContent*>(context.parserFileContent)));
    }
private:
    struct Context
    {
        Context(): parserFileContent(), fromFileAttribute(), fromUsingDeclaration(), fromUsingDirective(), fromNamespaceAliasDefinition(), fromGrammar() {}
        ParserFileContent* parserFileContent;
        std::string fromFileAttribute;
        cminor::pom::UsingObject* fromUsingDeclaration;
        cminor::pom::UsingObject* fromUsingDirective;
        cminor::pom::UsingObject* fromNamespaceAliasDefinition;
        cminor::parsing::Grammar* fromGrammar;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ParserFileGrammar::NamespaceRule : public cminor::parsing::Rule
{
public:
    NamespaceRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParserFileContent*", "parserFileContent"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> parserFileContent_value = std::move(stack.top());
        context.parserFileContent = *static_cast<cminor::parsing::ValueObject<ParserFileContent*>*>(parserFileContent_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NamespaceRule>(this, &NamespaceRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NamespaceRule>(this, &NamespaceRule::A1Action));
        cminor::parsing::NonterminalParser* nsNonterminalParser = GetNonterminal("ns");
        nsNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NamespaceRule>(this, &NamespaceRule::Postns));
        cminor::parsing::NonterminalParser* namespaceContentNonterminalParser = GetNonterminal("NamespaceContent");
        namespaceContentNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<NamespaceRule>(this, &NamespaceRule::PreNamespaceContent));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.parserFileContent->BeginNamespace(context.fromns);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.parserFileContent->EndNamespace();
    }
    void Postns(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromns_value = std::move(stack.top());
            context.fromns = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromns_value.get());
            stack.pop();
        }
    }
    void PreNamespaceContent(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParserFileContent*>(context.parserFileContent)));
    }
private:
    struct Context
    {
        Context(): parserFileContent(), fromns() {}
        ParserFileContent* parserFileContent;
        std::string fromns;
    };
    std::stack<Context> contextStack;
    Context context;
};

void ParserFileGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("Cm.Parsing.Cpp.DeclarationGrammar");
    if (!grammar0)
    {
        grammar0 = cpg::cpp::DeclarationGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar("Cm.Parsing.stdlib");
    if (!grammar1)
    {
        grammar1 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar("Cm.Parsing.Syntax.GrammarGrammar");
    if (!grammar2)
    {
        grammar2 = cpg::syntax::GrammarGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
}

void ParserFileGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("newline", this, "Cm.Parsing.stdlib.newline"));
    AddRuleLink(new cminor::parsing::RuleLink("spaces_and_comments", this, "Cm.Parsing.stdlib.spaces_and_comments"));
    AddRuleLink(new cminor::parsing::RuleLink("Grammar", this, "GrammarGrammar.Grammar"));
    AddRuleLink(new cminor::parsing::RuleLink("qualified_id", this, "Cm.Parsing.stdlib.qualified_id"));
    AddRuleLink(new cminor::parsing::RuleLink("UsingDeclaration", this, "Cm.Parsing.Cpp.DeclarationGrammar.UsingDeclaration"));
    AddRuleLink(new cminor::parsing::RuleLink("NamespaceAliasDefinition", this, "Cm.Parsing.Cpp.DeclarationGrammar.NamespaceAliasDefinition"));
    AddRuleLink(new cminor::parsing::RuleLink("UsingDirective", this, "Cm.Parsing.Cpp.DeclarationGrammar.UsingDirective"));
    AddRule(new ParserFileRule("ParserFile", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::EmptyParser()),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::NonterminalParser("IncludeDirectives", "IncludeDirectives", 1))),
            new cminor::parsing::NonterminalParser("NamespaceContent", "NamespaceContent", 1))));
    AddRule(new IncludeDirectivesRule("IncludeDirectives", GetScope(),
        new cminor::parsing::PositiveParser(
            new cminor::parsing::NonterminalParser("IncludeDirective", "IncludeDirective", 1))));
    AddRule(new IncludeDirectiveRule("IncludeDirective", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser("FileAttribute", "FileAttribute", 0)),
                    new cminor::parsing::CharParser('#')),
                new cminor::parsing::KeywordParser("include")),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::TokenParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::NonterminalParser("IncludeFileName", "IncludeFileName", 0),
                            new cminor::parsing::KleeneStarParser(
                                new cminor::parsing::CharSetParser("\r\n", true))),
                        new cminor::parsing::NonterminalParser("newline", "newline", 0)))))));
    AddRule(new FileAttributeRule("FileAttribute", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('['),
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::StringParser("cpp"),
                        new cminor::parsing::StringParser("hpp")))),
            new cminor::parsing::CharParser(']'))));
    AddRule(new IncludeFileNameRule("IncludeFileName", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::CharParser('<'),
                        new cminor::parsing::PositiveParser(
                            new cminor::parsing::CharSetParser(">\r\n", true))),
                    new cminor::parsing::CharParser('>'))))));
    AddRule(new NamespaceContentRule("NamespaceContent", GetScope(),
        new cminor::parsing::KleeneStarParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::NonterminalParser("FileAttribute", "FileAttribute", 0)),
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::ActionParser("A0",
                                    new cminor::parsing::NonterminalParser("UsingDeclaration", "UsingDeclaration", 0)),
                                new cminor::parsing::ActionParser("A1",
                                    new cminor::parsing::NonterminalParser("UsingDirective", "UsingDirective", 0))),
                            new cminor::parsing::ActionParser("A2",
                                new cminor::parsing::NonterminalParser("NamespaceAliasDefinition", "NamespaceAliasDefinition", 0)))),
                    new cminor::parsing::ActionParser("A3",
                        new cminor::parsing::NonterminalParser("Grammar", "Grammar", 1))),
                new cminor::parsing::NonterminalParser("Namespace", "Namespace", 1)))));
    AddRule(new NamespaceRule("Namespace", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser("namespace"),
                        new cminor::parsing::ActionParser("A0",
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("ns", "qualified_id", 0)))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('{'))),
                new cminor::parsing::NonterminalParser("NamespaceContent", "NamespaceContent", 1)),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser('}'))))));
    SetSkipRuleName("spaces_and_comments");
}

} } // namespace cpg.syntax
