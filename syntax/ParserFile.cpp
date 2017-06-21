#include "ParserFile.hpp"
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
#include <cminor/parsing/Namespace.hpp>
#include <cminor/parsing/Grammar.hpp>
#include <cminor/code/Declaration.hpp>
#include <cminor/syntax/Element.hpp>
#include <cminor/syntax/Grammar.hpp>

namespace cminor { namespace syntax {

using namespace cminor::parsing;
using namespace cminor::util;
using namespace cminor::unicode;

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

ParserFileGrammar::ParserFileGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("ParserFileGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.syntax")), parsingDomain_)
{
    SetOwner(0);
}

ParserFileContent* ParserFileGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, int id_, cminor::parsing::ParsingDomain* parsingDomain_)
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
    stack.push(std::unique_ptr<cminor::parsing::Object>(new ValueObject<int>(id_)));
    stack.push(std::unique_ptr<cminor::parsing::Object>(new ValueObject<cminor::parsing::ParsingDomain*>(parsingDomain_)));
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
    ParserFileContent* result = *static_cast<cminor::parsing::ValueObject<ParserFileContent*>*>(value.get());
    stack.pop();
    return result;
}

class ParserFileGrammar::ParserFileRule : public cminor::parsing::Rule
{
public:
    ParserFileRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("int"), ToUtf32("id_")));
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::parsing::ParsingDomain*"), ToUtf32("parsingDomain_")));
        SetValueTypeName(ToUtf32("ParserFileContent*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> parsingDomain__value = std::move(stack.top());
        context->parsingDomain_ = *static_cast<cminor::parsing::ValueObject<cminor::parsing::ParsingDomain*>*>(parsingDomain__value.get());
        stack.pop();
        std::unique_ptr<cminor::parsing::Object> id__value = std::move(stack.top());
        context->id_ = *static_cast<cminor::parsing::ValueObject<int>*>(id__value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParserFileContent*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ParserFileRule>(this, &ParserFileRule::A0Action));
        cminor::parsing::NonterminalParser* includeDirectivesNonterminalParser = GetNonterminal(ToUtf32("IncludeDirectives"));
        includeDirectivesNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ParserFileRule>(this, &ParserFileRule::PreIncludeDirectives));
        cminor::parsing::NonterminalParser* namespaceContentNonterminalParser = GetNonterminal(ToUtf32("NamespaceContent"));
        namespaceContentNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ParserFileRule>(this, &ParserFileRule::PreNamespaceContent));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ParserFileContent(context->id_, context->parsingDomain_);
        context->value->SetFilePath(fileName);
    }
    void PreIncludeDirectives(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParserFileContent*>(context->value)));
    }
    void PreNamespaceContent(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParserFileContent*>(context->value)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): id_(), parsingDomain_(), value() {}
        int id_;
        cminor::parsing::ParsingDomain* parsingDomain_;
        ParserFileContent* value;
    };
};

class ParserFileGrammar::IncludeDirectivesRule : public cminor::parsing::Rule
{
public:
    IncludeDirectivesRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParserFileContent*"), ToUtf32("parserFileContent")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> parserFileContent_value = std::move(stack.top());
        context->parserFileContent = *static_cast<cminor::parsing::ValueObject<ParserFileContent*>*>(parserFileContent_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::NonterminalParser* includeDirectiveNonterminalParser = GetNonterminal(ToUtf32("IncludeDirective"));
        includeDirectiveNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<IncludeDirectivesRule>(this, &IncludeDirectivesRule::PreIncludeDirective));
    }
    void PreIncludeDirective(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParserFileContent*>(context->parserFileContent)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): parserFileContent() {}
        ParserFileContent* parserFileContent;
    };
};

class ParserFileGrammar::IncludeDirectiveRule : public cminor::parsing::Rule
{
public:
    IncludeDirectiveRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParserFileContent*"), ToUtf32("parserFileContent")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> parserFileContent_value = std::move(stack.top());
        context->parserFileContent = *static_cast<cminor::parsing::ValueObject<ParserFileContent*>*>(parserFileContent_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IncludeDirectiveRule>(this, &IncludeDirectiveRule::A0Action));
        cminor::parsing::NonterminalParser* fileAttributeNonterminalParser = GetNonterminal(ToUtf32("FileAttribute"));
        fileAttributeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IncludeDirectiveRule>(this, &IncludeDirectiveRule::PostFileAttribute));
        cminor::parsing::NonterminalParser* includeFileNameNonterminalParser = GetNonterminal(ToUtf32("IncludeFileName"));
        includeFileNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IncludeDirectiveRule>(this, &IncludeDirectiveRule::PostIncludeFileName));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->parserFileContent->AddIncludeDirective(ToUtf8(context->fromIncludeFileName), ToUtf8(context->fromFileAttribute));
    }
    void PostFileAttribute(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFileAttribute_value = std::move(stack.top());
            context->fromFileAttribute = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromFileAttribute_value.get());
            stack.pop();
        }
    }
    void PostIncludeFileName(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIncludeFileName_value = std::move(stack.top());
            context->fromIncludeFileName = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromIncludeFileName_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): parserFileContent(), fromFileAttribute(), fromIncludeFileName() {}
        ParserFileContent* parserFileContent;
        std::u32string fromFileAttribute;
        std::u32string fromIncludeFileName;
    };
};

class ParserFileGrammar::FileAttributeRule : public cminor::parsing::Rule
{
public:
    FileAttributeRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FileAttributeRule>(this, &FileAttributeRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = std::u32string(matchBegin, matchEnd);
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        std::u32string value;
    };
};

class ParserFileGrammar::IncludeFileNameRule : public cminor::parsing::Rule
{
public:
    IncludeFileNameRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IncludeFileNameRule>(this, &IncludeFileNameRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = std::u32string(matchBegin, matchEnd);
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        std::u32string value;
    };
};

class ParserFileGrammar::NamespaceContentRule : public cminor::parsing::Rule
{
public:
    NamespaceContentRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParserFileContent*"), ToUtf32("parserFileContent")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> parserFileContent_value = std::move(stack.top());
        context->parserFileContent = *static_cast<cminor::parsing::ValueObject<ParserFileContent*>*>(parserFileContent_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NamespaceContentRule>(this, &NamespaceContentRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NamespaceContentRule>(this, &NamespaceContentRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NamespaceContentRule>(this, &NamespaceContentRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NamespaceContentRule>(this, &NamespaceContentRule::A3Action));
        cminor::parsing::NonterminalParser* fileAttributeNonterminalParser = GetNonterminal(ToUtf32("FileAttribute"));
        fileAttributeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NamespaceContentRule>(this, &NamespaceContentRule::PostFileAttribute));
        cminor::parsing::NonterminalParser* usingDeclarationNonterminalParser = GetNonterminal(ToUtf32("UsingDeclaration"));
        usingDeclarationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NamespaceContentRule>(this, &NamespaceContentRule::PostUsingDeclaration));
        cminor::parsing::NonterminalParser* usingDirectiveNonterminalParser = GetNonterminal(ToUtf32("UsingDirective"));
        usingDirectiveNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NamespaceContentRule>(this, &NamespaceContentRule::PostUsingDirective));
        cminor::parsing::NonterminalParser* namespaceAliasDefinitionNonterminalParser = GetNonterminal(ToUtf32("NamespaceAliasDefinition"));
        namespaceAliasDefinitionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NamespaceContentRule>(this, &NamespaceContentRule::PostNamespaceAliasDefinition));
        cminor::parsing::NonterminalParser* grammarNonterminalParser = GetNonterminal(ToUtf32("Grammar"));
        grammarNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<NamespaceContentRule>(this, &NamespaceContentRule::PreGrammar));
        grammarNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NamespaceContentRule>(this, &NamespaceContentRule::PostGrammar));
        cminor::parsing::NonterminalParser* namespaceNonterminalParser = GetNonterminal(ToUtf32("Namespace"));
        namespaceNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<NamespaceContentRule>(this, &NamespaceContentRule::PreNamespace));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->fromUsingDeclaration->SetFileAttr(context->fromFileAttribute);
        context->parserFileContent->AddUsingObject(context->fromUsingDeclaration);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->fromUsingDirective->SetFileAttr(context->fromFileAttribute);
        context->parserFileContent->AddUsingObject(context->fromUsingDirective);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->fromNamespaceAliasDefinition->SetFileAttr(context->fromFileAttribute);
        context->parserFileContent->AddUsingObject(context->fromNamespaceAliasDefinition);
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->parserFileContent->AddGrammar(context->fromGrammar);
    }
    void PostFileAttribute(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFileAttribute_value = std::move(stack.top());
            context->fromFileAttribute = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromFileAttribute_value.get());
            stack.pop();
        }
    }
    void PostUsingDeclaration(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromUsingDeclaration_value = std::move(stack.top());
            context->fromUsingDeclaration = *static_cast<cminor::parsing::ValueObject<cminor::codedom::UsingObject*>*>(fromUsingDeclaration_value.get());
            stack.pop();
        }
    }
    void PostUsingDirective(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromUsingDirective_value = std::move(stack.top());
            context->fromUsingDirective = *static_cast<cminor::parsing::ValueObject<cminor::codedom::UsingObject*>*>(fromUsingDirective_value.get());
            stack.pop();
        }
    }
    void PostNamespaceAliasDefinition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromNamespaceAliasDefinition_value = std::move(stack.top());
            context->fromNamespaceAliasDefinition = *static_cast<cminor::parsing::ValueObject<cminor::codedom::UsingObject*>*>(fromNamespaceAliasDefinition_value.get());
            stack.pop();
        }
    }
    void PreGrammar(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Scope*>(context->parserFileContent->CurrentScope())));
    }
    void PostGrammar(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromGrammar_value = std::move(stack.top());
            context->fromGrammar = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Grammar*>*>(fromGrammar_value.get());
            stack.pop();
        }
    }
    void PreNamespace(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParserFileContent*>(context->parserFileContent)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): parserFileContent(), fromFileAttribute(), fromUsingDeclaration(), fromUsingDirective(), fromNamespaceAliasDefinition(), fromGrammar() {}
        ParserFileContent* parserFileContent;
        std::u32string fromFileAttribute;
        cminor::codedom::UsingObject* fromUsingDeclaration;
        cminor::codedom::UsingObject* fromUsingDirective;
        cminor::codedom::UsingObject* fromNamespaceAliasDefinition;
        cminor::parsing::Grammar* fromGrammar;
    };
};

class ParserFileGrammar::NamespaceRule : public cminor::parsing::Rule
{
public:
    NamespaceRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParserFileContent*"), ToUtf32("parserFileContent")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> parserFileContent_value = std::move(stack.top());
        context->parserFileContent = *static_cast<cminor::parsing::ValueObject<ParserFileContent*>*>(parserFileContent_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NamespaceRule>(this, &NamespaceRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NamespaceRule>(this, &NamespaceRule::A1Action));
        cminor::parsing::NonterminalParser* nsNonterminalParser = GetNonterminal(ToUtf32("ns"));
        nsNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NamespaceRule>(this, &NamespaceRule::Postns));
        cminor::parsing::NonterminalParser* namespaceContentNonterminalParser = GetNonterminal(ToUtf32("NamespaceContent"));
        namespaceContentNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<NamespaceRule>(this, &NamespaceRule::PreNamespaceContent));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->parserFileContent->BeginNamespace(context->fromns);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->parserFileContent->EndNamespace();
    }
    void Postns(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromns_value = std::move(stack.top());
            context->fromns = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromns_value.get());
            stack.pop();
        }
    }
    void PreNamespaceContent(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParserFileContent*>(context->parserFileContent)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): parserFileContent(), fromns() {}
        ParserFileContent* parserFileContent;
        std::u32string fromns;
    };
};

void ParserFileGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cminor.syntax.GrammarGrammar"));
    if (!grammar0)
    {
        grammar0 = cminor::syntax::GrammarGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar(ToUtf32("cminor.parsing.stdlib"));
    if (!grammar1)
    {
        grammar1 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar(ToUtf32("cminor.code.DeclarationGrammar"));
    if (!grammar2)
    {
        grammar2 = cminor::code::DeclarationGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
}

void ParserFileGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("spaces_and_comments"), this, ToUtf32("cminor.parsing.stdlib.spaces_and_comments")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("UsingDirective"), this, ToUtf32("cminor.code.DeclarationGrammar.UsingDirective")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("newline"), this, ToUtf32("cminor.parsing.stdlib.newline")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("UsingDeclaration"), this, ToUtf32("cminor.code.DeclarationGrammar.UsingDeclaration")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Grammar"), this, ToUtf32("GrammarGrammar.Grammar")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("qualified_id"), this, ToUtf32("cminor.parsing.stdlib.qualified_id")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("NamespaceAliasDefinition"), this, ToUtf32("cminor.code.DeclarationGrammar.NamespaceAliasDefinition")));
    AddRule(new ParserFileRule(ToUtf32("ParserFile"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::EmptyParser()),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("IncludeDirectives"), ToUtf32("IncludeDirectives"), 1))),
            new cminor::parsing::NonterminalParser(ToUtf32("NamespaceContent"), ToUtf32("NamespaceContent"), 1))));
    AddRule(new IncludeDirectivesRule(ToUtf32("IncludeDirectives"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::PositiveParser(
            new cminor::parsing::NonterminalParser(ToUtf32("IncludeDirective"), ToUtf32("IncludeDirective"), 1))));
    AddRule(new IncludeDirectiveRule(ToUtf32("IncludeDirective"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::OptionalParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("FileAttribute"), ToUtf32("FileAttribute"), 0)),
                    new cminor::parsing::CharParser('#')),
                new cminor::parsing::KeywordParser(ToUtf32("include"))),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::TokenParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("IncludeFileName"), ToUtf32("IncludeFileName"), 0),
                            new cminor::parsing::KleeneStarParser(
                                new cminor::parsing::CharSetParser(ToUtf32("\r\n"), true))),
                        new cminor::parsing::NonterminalParser(ToUtf32("newline"), ToUtf32("newline"), 0)))))));
    AddRule(new FileAttributeRule(ToUtf32("FileAttribute"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::CharParser('['),
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::StringParser(ToUtf32("cpp")),
                        new cminor::parsing::StringParser(ToUtf32("hpp"))))),
            new cminor::parsing::CharParser(']'))));
    AddRule(new IncludeFileNameRule(ToUtf32("IncludeFileName"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::CharParser('<'),
                        new cminor::parsing::PositiveParser(
                            new cminor::parsing::CharSetParser(ToUtf32(">\r\n"), true))),
                    new cminor::parsing::CharParser('>'))))));
    AddRule(new NamespaceContentRule(ToUtf32("NamespaceContent"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::KleeneStarParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("FileAttribute"), ToUtf32("FileAttribute"), 0)),
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::ActionParser(ToUtf32("A0"),
                                    new cminor::parsing::NonterminalParser(ToUtf32("UsingDeclaration"), ToUtf32("UsingDeclaration"), 0)),
                                new cminor::parsing::ActionParser(ToUtf32("A1"),
                                    new cminor::parsing::NonterminalParser(ToUtf32("UsingDirective"), ToUtf32("UsingDirective"), 0))),
                            new cminor::parsing::ActionParser(ToUtf32("A2"),
                                new cminor::parsing::NonterminalParser(ToUtf32("NamespaceAliasDefinition"), ToUtf32("NamespaceAliasDefinition"), 0)))),
                    new cminor::parsing::ActionParser(ToUtf32("A3"),
                        new cminor::parsing::NonterminalParser(ToUtf32("Grammar"), ToUtf32("Grammar"), 1))),
                new cminor::parsing::NonterminalParser(ToUtf32("Namespace"), ToUtf32("Namespace"), 1)))));
    AddRule(new NamespaceRule(ToUtf32("Namespace"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser(ToUtf32("namespace")),
                        new cminor::parsing::ActionParser(ToUtf32("A0"),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("ns"), ToUtf32("qualified_id"), 0)))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('{'))),
                new cminor::parsing::NonterminalParser(ToUtf32("NamespaceContent"), ToUtf32("NamespaceContent"), 1)),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser('}'))))));
    SetSkipRuleName(ToUtf32("spaces_and_comments"));
}

} } // namespace cminor.syntax
