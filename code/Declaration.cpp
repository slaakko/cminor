#include "Declaration.hpp"
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
#include <cminor/code/Declarator.hpp>
#include <cminor/code/Identifier.hpp>
#include <cminor/code/Expression.hpp>
#include <cminor/codedom/Type.hpp>

namespace cminor { namespace code {

using namespace cminor::codedom;
using namespace cminor::parsing;
using namespace cminor::util;
using namespace cminor::unicode;

DeclarationGrammar* DeclarationGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

DeclarationGrammar* DeclarationGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    DeclarationGrammar* grammar(new DeclarationGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

DeclarationGrammar::DeclarationGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("DeclarationGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.code")), parsingDomain_)
{
    SetOwner(0);
    keywords0.push_back(ToUtf32("auto"));
    keywords0.push_back(ToUtf32("extern"));
    keywords0.push_back(ToUtf32("mutable"));
    keywords0.push_back(ToUtf32("register"));
    keywords0.push_back(ToUtf32("static"));
    keywords1.push_back(ToUtf32("bool"));
    keywords1.push_back(ToUtf32("char"));
    keywords1.push_back(ToUtf32("double"));
    keywords1.push_back(ToUtf32("float"));
    keywords1.push_back(ToUtf32("int"));
    keywords1.push_back(ToUtf32("long"));
    keywords1.push_back(ToUtf32("short"));
    keywords1.push_back(ToUtf32("signed"));
    keywords1.push_back(ToUtf32("unsigned"));
    keywords1.push_back(ToUtf32("void"));
    keywords1.push_back(ToUtf32("wchar_t"));
}

cminor::codedom::CppObject* DeclarationGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName)
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
    cminor::codedom::CppObject* result = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(value.get());
    stack.pop();
    return result;
}

class DeclarationGrammar::BlockDeclarationRule : public cminor::parsing::Rule
{
public:
    BlockDeclarationRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BlockDeclarationRule>(this, &BlockDeclarationRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BlockDeclarationRule>(this, &BlockDeclarationRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BlockDeclarationRule>(this, &BlockDeclarationRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BlockDeclarationRule>(this, &BlockDeclarationRule::A3Action));
        cminor::parsing::NonterminalParser* namespaceAliasDefinitionNonterminalParser = GetNonterminal(ToUtf32("NamespaceAliasDefinition"));
        namespaceAliasDefinitionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<BlockDeclarationRule>(this, &BlockDeclarationRule::PostNamespaceAliasDefinition));
        cminor::parsing::NonterminalParser* usingDirectiveNonterminalParser = GetNonterminal(ToUtf32("UsingDirective"));
        usingDirectiveNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<BlockDeclarationRule>(this, &BlockDeclarationRule::PostUsingDirective));
        cminor::parsing::NonterminalParser* usingDeclarationNonterminalParser = GetNonterminal(ToUtf32("UsingDeclaration"));
        usingDeclarationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<BlockDeclarationRule>(this, &BlockDeclarationRule::PostUsingDeclaration));
        cminor::parsing::NonterminalParser* simpleDeclarationNonterminalParser = GetNonterminal(ToUtf32("SimpleDeclaration"));
        simpleDeclarationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<BlockDeclarationRule>(this, &BlockDeclarationRule::PostSimpleDeclaration));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromNamespaceAliasDefinition;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromUsingDirective;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromUsingDeclaration;
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromSimpleDeclaration;
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
    void PostSimpleDeclaration(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSimpleDeclaration_value = std::move(stack.top());
            context->fromSimpleDeclaration = *static_cast<cminor::parsing::ValueObject<cminor::codedom::SimpleDeclaration*>*>(fromSimpleDeclaration_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromNamespaceAliasDefinition(), fromUsingDirective(), fromUsingDeclaration(), fromSimpleDeclaration() {}
        cminor::codedom::CppObject* value;
        cminor::codedom::UsingObject* fromNamespaceAliasDefinition;
        cminor::codedom::UsingObject* fromUsingDirective;
        cminor::codedom::UsingObject* fromUsingDeclaration;
        cminor::codedom::SimpleDeclaration* fromSimpleDeclaration;
    };
};

class DeclarationGrammar::SimpleDeclarationRule : public cminor::parsing::Rule
{
public:
    SimpleDeclarationRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::SimpleDeclaration*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<SimpleDeclaration>"), ToUtf32("sd")));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::SimpleDeclaration*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SimpleDeclarationRule>(this, &SimpleDeclarationRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SimpleDeclarationRule>(this, &SimpleDeclarationRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SimpleDeclarationRule>(this, &SimpleDeclarationRule::A2Action));
        cminor::parsing::NonterminalParser* declSpecifierSeqNonterminalParser = GetNonterminal(ToUtf32("DeclSpecifierSeq"));
        declSpecifierSeqNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<SimpleDeclarationRule>(this, &SimpleDeclarationRule::PreDeclSpecifierSeq));
        cminor::parsing::NonterminalParser* initDeclaratorListNonterminalParser = GetNonterminal(ToUtf32("InitDeclaratorList"));
        initDeclaratorListNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SimpleDeclarationRule>(this, &SimpleDeclarationRule::PostInitDeclaratorList));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->sd.reset(new SimpleDeclaration);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->sd.release();
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->sd->SetInitDeclaratorList(context->fromInitDeclaratorList);
    }
    void PreDeclSpecifierSeq(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::SimpleDeclaration*>(context->sd.get())));
    }
    void PostInitDeclaratorList(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromInitDeclaratorList_value = std::move(stack.top());
            context->fromInitDeclaratorList = *static_cast<cminor::parsing::ValueObject<cminor::codedom::InitDeclaratorList*>*>(fromInitDeclaratorList_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), sd(), fromInitDeclaratorList() {}
        cminor::codedom::SimpleDeclaration* value;
        std::unique_ptr<SimpleDeclaration> sd;
        cminor::codedom::InitDeclaratorList* fromInitDeclaratorList;
    };
};

class DeclarationGrammar::DeclSpecifierSeqRule : public cminor::parsing::Rule
{
public:
    DeclSpecifierSeqRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::codedom::SimpleDeclaration*"), ToUtf32("declaration")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> declaration_value = std::move(stack.top());
        context->declaration = *static_cast<cminor::parsing::ValueObject<cminor::codedom::SimpleDeclaration*>*>(declaration_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeclSpecifierSeqRule>(this, &DeclSpecifierSeqRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeclSpecifierSeqRule>(this, &DeclSpecifierSeqRule::A1Action));
        cminor::parsing::NonterminalParser* declSpecifierNonterminalParser = GetNonterminal(ToUtf32("DeclSpecifier"));
        declSpecifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeclSpecifierSeqRule>(this, &DeclSpecifierSeqRule::PostDeclSpecifier));
        cminor::parsing::NonterminalParser* typeNameNonterminalParser = GetNonterminal(ToUtf32("TypeName"));
        typeNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeclSpecifierSeqRule>(this, &DeclSpecifierSeqRule::PostTypeName));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->declaration->Add(context->fromDeclSpecifier);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->declaration->Add(context->fromTypeName);
    }
    void PostDeclSpecifier(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDeclSpecifier_value = std::move(stack.top());
            context->fromDeclSpecifier = *static_cast<cminor::parsing::ValueObject<cminor::codedom::DeclSpecifier*>*>(fromDeclSpecifier_value.get());
            stack.pop();
        }
    }
    void PostTypeName(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeName_value = std::move(stack.top());
            context->fromTypeName = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeName*>*>(fromTypeName_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): declaration(), fromDeclSpecifier(), fromTypeName() {}
        cminor::codedom::SimpleDeclaration* declaration;
        cminor::codedom::DeclSpecifier* fromDeclSpecifier;
        cminor::codedom::TypeName* fromTypeName;
    };
};

class DeclarationGrammar::DeclSpecifierRule : public cminor::parsing::Rule
{
public:
    DeclSpecifierRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::DeclSpecifier*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::DeclSpecifier*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeclSpecifierRule>(this, &DeclSpecifierRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeclSpecifierRule>(this, &DeclSpecifierRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeclSpecifierRule>(this, &DeclSpecifierRule::A2Action));
        cminor::parsing::NonterminalParser* storageClassSpecifierNonterminalParser = GetNonterminal(ToUtf32("StorageClassSpecifier"));
        storageClassSpecifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeclSpecifierRule>(this, &DeclSpecifierRule::PostStorageClassSpecifier));
        cminor::parsing::NonterminalParser* typeSpecifierNonterminalParser = GetNonterminal(ToUtf32("TypeSpecifier"));
        typeSpecifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeclSpecifierRule>(this, &DeclSpecifierRule::PostTypeSpecifier));
        cminor::parsing::NonterminalParser* typedefNonterminalParser = GetNonterminal(ToUtf32("Typedef"));
        typedefNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeclSpecifierRule>(this, &DeclSpecifierRule::PostTypedef));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromStorageClassSpecifier;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromTypeSpecifier;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromTypedef;
    }
    void PostStorageClassSpecifier(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromStorageClassSpecifier_value = std::move(stack.top());
            context->fromStorageClassSpecifier = *static_cast<cminor::parsing::ValueObject<cminor::codedom::StorageClassSpecifier*>*>(fromStorageClassSpecifier_value.get());
            stack.pop();
        }
    }
    void PostTypeSpecifier(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeSpecifier_value = std::move(stack.top());
            context->fromTypeSpecifier = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeSpecifier*>*>(fromTypeSpecifier_value.get());
            stack.pop();
        }
    }
    void PostTypedef(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypedef_value = std::move(stack.top());
            context->fromTypedef = *static_cast<cminor::parsing::ValueObject<cminor::codedom::DeclSpecifier*>*>(fromTypedef_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromStorageClassSpecifier(), fromTypeSpecifier(), fromTypedef() {}
        cminor::codedom::DeclSpecifier* value;
        cminor::codedom::StorageClassSpecifier* fromStorageClassSpecifier;
        cminor::codedom::TypeSpecifier* fromTypeSpecifier;
        cminor::codedom::DeclSpecifier* fromTypedef;
    };
};

class DeclarationGrammar::StorageClassSpecifierRule : public cminor::parsing::Rule
{
public:
    StorageClassSpecifierRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::StorageClassSpecifier*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::StorageClassSpecifier*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StorageClassSpecifierRule>(this, &StorageClassSpecifierRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new StorageClassSpecifier(std::u32string(matchBegin, matchEnd));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::codedom::StorageClassSpecifier* value;
    };
};

class DeclarationGrammar::TypeSpecifierRule : public cminor::parsing::Rule
{
public:
    TypeSpecifierRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::TypeSpecifier*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::TypeSpecifier*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeSpecifierRule>(this, &TypeSpecifierRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeSpecifierRule>(this, &TypeSpecifierRule::A1Action));
        cminor::parsing::NonterminalParser* simpleTypeSpecifierNonterminalParser = GetNonterminal(ToUtf32("SimpleTypeSpecifier"));
        simpleTypeSpecifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TypeSpecifierRule>(this, &TypeSpecifierRule::PostSimpleTypeSpecifier));
        cminor::parsing::NonterminalParser* cVQualifierNonterminalParser = GetNonterminal(ToUtf32("CVQualifier"));
        cVQualifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TypeSpecifierRule>(this, &TypeSpecifierRule::PostCVQualifier));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromSimpleTypeSpecifier;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromCVQualifier;
    }
    void PostSimpleTypeSpecifier(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSimpleTypeSpecifier_value = std::move(stack.top());
            context->fromSimpleTypeSpecifier = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeSpecifier*>*>(fromSimpleTypeSpecifier_value.get());
            stack.pop();
        }
    }
    void PostCVQualifier(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCVQualifier_value = std::move(stack.top());
            context->fromCVQualifier = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeSpecifier*>*>(fromCVQualifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromSimpleTypeSpecifier(), fromCVQualifier() {}
        cminor::codedom::TypeSpecifier* value;
        cminor::codedom::TypeSpecifier* fromSimpleTypeSpecifier;
        cminor::codedom::TypeSpecifier* fromCVQualifier;
    };
};

class DeclarationGrammar::SimpleTypeSpecifierRule : public cminor::parsing::Rule
{
public:
    SimpleTypeSpecifierRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::TypeSpecifier*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::TypeSpecifier*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SimpleTypeSpecifierRule>(this, &SimpleTypeSpecifierRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new TypeSpecifier(std::u32string(matchBegin, matchEnd));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::codedom::TypeSpecifier* value;
    };
};

class DeclarationGrammar::TypeNameRule : public cminor::parsing::Rule
{
public:
    TypeNameRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::TypeName*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::TypeName*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeNameRule>(this, &TypeNameRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeNameRule>(this, &TypeNameRule::A1Action));
        cminor::parsing::NonterminalParser* qualifiedIdNonterminalParser = GetNonterminal(ToUtf32("QualifiedId"));
        qualifiedIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TypeNameRule>(this, &TypeNameRule::PostQualifiedId));
        cminor::parsing::NonterminalParser* templateArgumentListNonterminalParser = GetNonterminal(ToUtf32("TemplateArgumentList"));
        templateArgumentListNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<TypeNameRule>(this, &TypeNameRule::PreTemplateArgumentList));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new cminor::codedom::TypeName(std::u32string(matchBegin, matchEnd));
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value->IsTemplate() = true;
    }
    void PostQualifiedId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromQualifiedId_value = std::move(stack.top());
            context->fromQualifiedId = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromQualifiedId_value.get());
            stack.pop();
        }
    }
    void PreTemplateArgumentList(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::TypeName*>(context->value)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromQualifiedId() {}
        cminor::codedom::TypeName* value;
        std::u32string fromQualifiedId;
    };
};

class DeclarationGrammar::TemplateArgumentListRule : public cminor::parsing::Rule
{
public:
    TemplateArgumentListRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("cminor::codedom::TypeName*"), ToUtf32("typeName")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> typeName_value = std::move(stack.top());
        context->typeName = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeName*>*>(typeName_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TemplateArgumentListRule>(this, &TemplateArgumentListRule::A0Action));
        cminor::parsing::NonterminalParser* templateArgumentNonterminalParser = GetNonterminal(ToUtf32("TemplateArgument"));
        templateArgumentNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TemplateArgumentListRule>(this, &TemplateArgumentListRule::PostTemplateArgument));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->typeName->AddTemplateArgument(context->fromTemplateArgument);
    }
    void PostTemplateArgument(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTemplateArgument_value = std::move(stack.top());
            context->fromTemplateArgument = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromTemplateArgument_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): typeName(), fromTemplateArgument() {}
        cminor::codedom::TypeName* typeName;
        cminor::codedom::CppObject* fromTemplateArgument;
    };
};

class DeclarationGrammar::TemplateArgumentRule : public cminor::parsing::Rule
{
public:
    TemplateArgumentRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::CppObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TemplateArgumentRule>(this, &TemplateArgumentRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TemplateArgumentRule>(this, &TemplateArgumentRule::A1Action));
        cminor::parsing::NonterminalParser* typeIdNonterminalParser = GetNonterminal(ToUtf32("TypeId"));
        typeIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TemplateArgumentRule>(this, &TemplateArgumentRule::PostTypeId));
        cminor::parsing::NonterminalParser* assignmentExpressionNonterminalParser = GetNonterminal(ToUtf32("AssignmentExpression"));
        assignmentExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TemplateArgumentRule>(this, &TemplateArgumentRule::PostAssignmentExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromTypeId;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromAssignmentExpression;
    }
    void PostTypeId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeId_value = std::move(stack.top());
            context->fromTypeId = *static_cast<cminor::parsing::ValueObject<cminor::codedom::TypeId*>*>(fromTypeId_value.get());
            stack.pop();
        }
    }
    void PostAssignmentExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAssignmentExpression_value = std::move(stack.top());
            context->fromAssignmentExpression = *static_cast<cminor::parsing::ValueObject<cminor::codedom::CppObject*>*>(fromAssignmentExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromTypeId(), fromAssignmentExpression() {}
        cminor::codedom::CppObject* value;
        cminor::codedom::TypeId* fromTypeId;
        cminor::codedom::CppObject* fromAssignmentExpression;
    };
};

class DeclarationGrammar::TypedefRule : public cminor::parsing::Rule
{
public:
    TypedefRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::DeclSpecifier*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::DeclSpecifier*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypedefRule>(this, &TypedefRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Typedef;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::codedom::DeclSpecifier* value;
    };
};

class DeclarationGrammar::CVQualifierRule : public cminor::parsing::Rule
{
public:
    CVQualifierRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::TypeSpecifier*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::TypeSpecifier*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CVQualifierRule>(this, &CVQualifierRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CVQualifierRule>(this, &CVQualifierRule::A1Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Const;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Volatile;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::codedom::TypeSpecifier* value;
    };
};

class DeclarationGrammar::NamespaceAliasDefinitionRule : public cminor::parsing::Rule
{
public:
    NamespaceAliasDefinitionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::UsingObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::UsingObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NamespaceAliasDefinitionRule>(this, &NamespaceAliasDefinitionRule::A0Action));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("Identifier"));
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NamespaceAliasDefinitionRule>(this, &NamespaceAliasDefinitionRule::PostIdentifier));
        cminor::parsing::NonterminalParser* qualifiedIdNonterminalParser = GetNonterminal(ToUtf32("QualifiedId"));
        qualifiedIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NamespaceAliasDefinitionRule>(this, &NamespaceAliasDefinitionRule::PostQualifiedId));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new NamespaceAlias(context->fromIdentifier, context->fromQualifiedId);
    }
    void PostIdentifier(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIdentifier_value = std::move(stack.top());
            context->fromIdentifier = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
    void PostQualifiedId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromQualifiedId_value = std::move(stack.top());
            context->fromQualifiedId = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromQualifiedId_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromIdentifier(), fromQualifiedId() {}
        cminor::codedom::UsingObject* value;
        std::u32string fromIdentifier;
        std::u32string fromQualifiedId;
    };
};

class DeclarationGrammar::UsingDeclarationRule : public cminor::parsing::Rule
{
public:
    UsingDeclarationRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::UsingObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::UsingObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UsingDeclarationRule>(this, &UsingDeclarationRule::A0Action));
        cminor::parsing::NonterminalParser* qualifiedIdNonterminalParser = GetNonterminal(ToUtf32("QualifiedId"));
        qualifiedIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UsingDeclarationRule>(this, &UsingDeclarationRule::PostQualifiedId));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new UsingDeclaration(context->fromQualifiedId);
    }
    void PostQualifiedId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromQualifiedId_value = std::move(stack.top());
            context->fromQualifiedId = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromQualifiedId_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromQualifiedId() {}
        cminor::codedom::UsingObject* value;
        std::u32string fromQualifiedId;
    };
};

class DeclarationGrammar::UsingDirectiveRule : public cminor::parsing::Rule
{
public:
    UsingDirectiveRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::codedom::UsingObject*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::codedom::UsingObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UsingDirectiveRule>(this, &UsingDirectiveRule::A0Action));
        cminor::parsing::NonterminalParser* qualifiedIdNonterminalParser = GetNonterminal(ToUtf32("QualifiedId"));
        qualifiedIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UsingDirectiveRule>(this, &UsingDirectiveRule::PostQualifiedId));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new UsingDirective(context->fromQualifiedId);
    }
    void PostQualifiedId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromQualifiedId_value = std::move(stack.top());
            context->fromQualifiedId = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromQualifiedId_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromQualifiedId() {}
        cminor::codedom::UsingObject* value;
        std::u32string fromQualifiedId;
    };
};

void DeclarationGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cminor.code.ExpressionGrammar"));
    if (!grammar0)
    {
        grammar0 = cminor::code::ExpressionGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar(ToUtf32("cminor.parsing.stdlib"));
    if (!grammar1)
    {
        grammar1 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar(ToUtf32("cminor.code.DeclaratorGrammar"));
    if (!grammar2)
    {
        grammar2 = cminor::code::DeclaratorGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar(ToUtf32("cminor.code.IdentifierGrammar"));
    if (!grammar3)
    {
        grammar3 = cminor::code::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
}

void DeclarationGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("AssignmentExpression"), this, ToUtf32("ExpressionGrammar.AssignmentExpression")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("identifier"), this, ToUtf32("cminor.parsing.stdlib.identifier")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("InitDeclaratorList"), this, ToUtf32("DeclaratorGrammar.InitDeclaratorList")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("TypeId"), this, ToUtf32("DeclaratorGrammar.TypeId")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Identifier"), this, ToUtf32("IdentifierGrammar.Identifier")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("QualifiedId"), this, ToUtf32("IdentifierGrammar.QualifiedId")));
    AddRule(new BlockDeclarationRule(ToUtf32("BlockDeclaration"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::ActionParser(ToUtf32("A0"),
                        new cminor::parsing::NonterminalParser(ToUtf32("NamespaceAliasDefinition"), ToUtf32("NamespaceAliasDefinition"), 0)),
                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                        new cminor::parsing::NonterminalParser(ToUtf32("UsingDirective"), ToUtf32("UsingDirective"), 0))),
                new cminor::parsing::ActionParser(ToUtf32("A2"),
                    new cminor::parsing::NonterminalParser(ToUtf32("UsingDeclaration"), ToUtf32("UsingDeclaration"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A3"),
                new cminor::parsing::NonterminalParser(ToUtf32("SimpleDeclaration"), ToUtf32("SimpleDeclaration"), 0)))));
    AddRule(new SimpleDeclarationRule(ToUtf32("SimpleDeclaration"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::EmptyParser()),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("DeclSpecifierSeq"), ToUtf32("DeclSpecifierSeq"), 1)),
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::ActionParser(ToUtf32("A2"),
                                new cminor::parsing::NonterminalParser(ToUtf32("InitDeclaratorList"), ToUtf32("InitDeclaratorList"), 0)))),
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new DeclSpecifierSeqRule(ToUtf32("DeclSpecifierSeq"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::PositiveParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::NonterminalParser(ToUtf32("DeclSpecifier"), ToUtf32("DeclSpecifier"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::NonterminalParser(ToUtf32("TypeName"), ToUtf32("TypeName"), 0)))));
    AddRule(new DeclSpecifierRule(ToUtf32("DeclSpecifier"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::NonterminalParser(ToUtf32("StorageClassSpecifier"), ToUtf32("StorageClassSpecifier"), 0)),
                new cminor::parsing::ActionParser(ToUtf32("A1"),
                    new cminor::parsing::NonterminalParser(ToUtf32("TypeSpecifier"), ToUtf32("TypeSpecifier"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A2"),
                new cminor::parsing::NonterminalParser(ToUtf32("Typedef"), ToUtf32("Typedef"), 0)))));
    AddRule(new StorageClassSpecifierRule(ToUtf32("StorageClassSpecifier"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordListParser(ToUtf32("identifier"), keywords0))));
    AddRule(new TypeSpecifierRule(ToUtf32("TypeSpecifier"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("SimpleTypeSpecifier"), ToUtf32("SimpleTypeSpecifier"), 0)),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::NonterminalParser(ToUtf32("CVQualifier"), ToUtf32("CVQualifier"), 0)))));
    AddRule(new SimpleTypeSpecifierRule(ToUtf32("SimpleTypeSpecifier"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordListParser(ToUtf32("identifier"), keywords1))));
    AddRule(new TypeNameRule(ToUtf32("TypeName"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("QualifiedId"), ToUtf32("QualifiedId"), 0)),
            new cminor::parsing::OptionalParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser(ToUtf32("A1"),
                            new cminor::parsing::CharParser('<')),
                        new cminor::parsing::NonterminalParser(ToUtf32("TemplateArgumentList"), ToUtf32("TemplateArgumentList"), 1)),
                    new cminor::parsing::CharParser('>'))))));
    AddRule(new TemplateArgumentListRule(ToUtf32("TemplateArgumentList"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ListParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("TemplateArgument"), ToUtf32("TemplateArgument"), 0)),
            new cminor::parsing::CharParser(','))));
    AddRule(new TemplateArgumentRule(ToUtf32("TemplateArgument"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("TypeId"), ToUtf32("TypeId"), 0)),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::NonterminalParser(ToUtf32("AssignmentExpression"), ToUtf32("AssignmentExpression"), 0)))));
    AddRule(new TypedefRule(ToUtf32("Typedef"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("typedef")))));
    AddRule(new CVQualifierRule(ToUtf32("CVQualifier"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::KeywordParser(ToUtf32("const"))),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::KeywordParser(ToUtf32("volatile"))))));
    AddRule(new NamespaceAliasDefinitionRule(ToUtf32("NamespaceAliasDefinition"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser(ToUtf32("namespace")),
                        new cminor::parsing::NonterminalParser(ToUtf32("Identifier"), ToUtf32("Identifier"), 0)),
                    new cminor::parsing::CharParser('=')),
                new cminor::parsing::NonterminalParser(ToUtf32("QualifiedId"), ToUtf32("QualifiedId"), 0)),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::CharParser(';')))));
    AddRule(new UsingDeclarationRule(ToUtf32("UsingDeclaration"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser(ToUtf32("using")),
                new cminor::parsing::NonterminalParser(ToUtf32("QualifiedId"), ToUtf32("QualifiedId"), 0)),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::CharParser(';')))));
    AddRule(new UsingDirectiveRule(ToUtf32("UsingDirective"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser(ToUtf32("using")),
                    new cminor::parsing::KeywordParser(ToUtf32("namespace"))),
                new cminor::parsing::NonterminalParser(ToUtf32("QualifiedId"), ToUtf32("QualifiedId"), 0)),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::CharParser(';')))));
}

} } // namespace cminor.code
