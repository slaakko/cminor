#include "Declaration.hpp"
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>
#include <cminor/cpg/cpgcpp/Declarator.hpp>
#include <cminor/cpg/cpgcpp/Identifier.hpp>
#include <cminor/cpg/cpgcpp/Expression.hpp>
#include <cminor/pom/Type.hpp>

namespace cpg { namespace cpp {

using namespace cminor::pom;
using namespace cminor::parsing;

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

DeclarationGrammar::DeclarationGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("DeclarationGrammar", parsingDomain_->GetNamespaceScope("cpg.cpp"), parsingDomain_)
{
    SetOwner(0);
    keywords0.push_back("auto");
    keywords0.push_back("extern");
    keywords0.push_back("mutable");
    keywords0.push_back("register");
    keywords0.push_back("static");
    keywords1.push_back("bool");
    keywords1.push_back("char");
    keywords1.push_back("double");
    keywords1.push_back("float");
    keywords1.push_back("int");
    keywords1.push_back("long");
    keywords1.push_back("short");
    keywords1.push_back("signed");
    keywords1.push_back("unsigned");
    keywords1.push_back("void");
    keywords1.push_back("wchar_t");
}

cminor::pom::CppObject* DeclarationGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName)
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
    cminor::pom::CppObject* result = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(value.get());
    stack.pop();
    return result;
}

class DeclarationGrammar::BlockDeclarationRule : public cminor::parsing::Rule
{
public:
    BlockDeclarationRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::CppObject*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BlockDeclarationRule>(this, &BlockDeclarationRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BlockDeclarationRule>(this, &BlockDeclarationRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BlockDeclarationRule>(this, &BlockDeclarationRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BlockDeclarationRule>(this, &BlockDeclarationRule::A3Action));
        cminor::parsing::NonterminalParser* namespaceAliasDefinitionNonterminalParser = GetNonterminal("NamespaceAliasDefinition");
        namespaceAliasDefinitionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<BlockDeclarationRule>(this, &BlockDeclarationRule::PostNamespaceAliasDefinition));
        cminor::parsing::NonterminalParser* usingDirectiveNonterminalParser = GetNonterminal("UsingDirective");
        usingDirectiveNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<BlockDeclarationRule>(this, &BlockDeclarationRule::PostUsingDirective));
        cminor::parsing::NonterminalParser* usingDeclarationNonterminalParser = GetNonterminal("UsingDeclaration");
        usingDeclarationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<BlockDeclarationRule>(this, &BlockDeclarationRule::PostUsingDeclaration));
        cminor::parsing::NonterminalParser* simpleDeclarationNonterminalParser = GetNonterminal("SimpleDeclaration");
        simpleDeclarationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<BlockDeclarationRule>(this, &BlockDeclarationRule::PostSimpleDeclaration));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromNamespaceAliasDefinition;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromUsingDirective;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromUsingDeclaration;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromNamespaceAliasDefinition = *static_cast<cminor::parsing::ValueObject<cminor::pom::UsingObject*>*>(fromNamespaceAliasDefinition_value.get());
            stack.pop();
        }
    }
    void PostUsingDirective(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromUsingDirective_value = std::move(stack.top());
            context->fromUsingDirective = *static_cast<cminor::parsing::ValueObject<cminor::pom::UsingObject*>*>(fromUsingDirective_value.get());
            stack.pop();
        }
    }
    void PostUsingDeclaration(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromUsingDeclaration_value = std::move(stack.top());
            context->fromUsingDeclaration = *static_cast<cminor::parsing::ValueObject<cminor::pom::UsingObject*>*>(fromUsingDeclaration_value.get());
            stack.pop();
        }
    }
    void PostSimpleDeclaration(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSimpleDeclaration_value = std::move(stack.top());
            context->fromSimpleDeclaration = *static_cast<cminor::parsing::ValueObject<cminor::pom::SimpleDeclaration*>*>(fromSimpleDeclaration_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromNamespaceAliasDefinition(), fromUsingDirective(), fromUsingDeclaration(), fromSimpleDeclaration() {}
        cminor::pom::CppObject* value;
        cminor::pom::UsingObject* fromNamespaceAliasDefinition;
        cminor::pom::UsingObject* fromUsingDirective;
        cminor::pom::UsingObject* fromUsingDeclaration;
        cminor::pom::SimpleDeclaration* fromSimpleDeclaration;
    };
};

class DeclarationGrammar::SimpleDeclarationRule : public cminor::parsing::Rule
{
public:
    SimpleDeclarationRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::SimpleDeclaration*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<SimpleDeclaration>", "sd"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::SimpleDeclaration*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SimpleDeclarationRule>(this, &SimpleDeclarationRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SimpleDeclarationRule>(this, &SimpleDeclarationRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SimpleDeclarationRule>(this, &SimpleDeclarationRule::A2Action));
        cminor::parsing::NonterminalParser* declSpecifierSeqNonterminalParser = GetNonterminal("DeclSpecifierSeq");
        declSpecifierSeqNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<SimpleDeclarationRule>(this, &SimpleDeclarationRule::PreDeclSpecifierSeq));
        cminor::parsing::NonterminalParser* initDeclaratorListNonterminalParser = GetNonterminal("InitDeclaratorList");
        initDeclaratorListNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<SimpleDeclarationRule>(this, &SimpleDeclarationRule::PostInitDeclaratorList));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->sd.reset(new SimpleDeclaration);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->sd.release();
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->sd->SetInitDeclaratorList(context->fromInitDeclaratorList);
    }
    void PreDeclSpecifierSeq(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::SimpleDeclaration*>(context->sd.get())));
    }
    void PostInitDeclaratorList(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromInitDeclaratorList_value = std::move(stack.top());
            context->fromInitDeclaratorList = *static_cast<cminor::parsing::ValueObject<cminor::pom::InitDeclaratorList*>*>(fromInitDeclaratorList_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), sd(), fromInitDeclaratorList() {}
        cminor::pom::SimpleDeclaration* value;
        std::unique_ptr<SimpleDeclaration> sd;
        cminor::pom::InitDeclaratorList* fromInitDeclaratorList;
    };
};

class DeclarationGrammar::DeclSpecifierSeqRule : public cminor::parsing::Rule
{
public:
    DeclSpecifierSeqRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable("cminor::pom::SimpleDeclaration*", "declaration"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> declaration_value = std::move(stack.top());
        context->declaration = *static_cast<cminor::parsing::ValueObject<cminor::pom::SimpleDeclaration*>*>(declaration_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeclSpecifierSeqRule>(this, &DeclSpecifierSeqRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeclSpecifierSeqRule>(this, &DeclSpecifierSeqRule::A1Action));
        cminor::parsing::NonterminalParser* declSpecifierNonterminalParser = GetNonterminal("DeclSpecifier");
        declSpecifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeclSpecifierSeqRule>(this, &DeclSpecifierSeqRule::PostDeclSpecifier));
        cminor::parsing::NonterminalParser* typeNameNonterminalParser = GetNonterminal("TypeName");
        typeNameNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeclSpecifierSeqRule>(this, &DeclSpecifierSeqRule::PostTypeName));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->declaration->Add(context->fromDeclSpecifier);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromDeclSpecifier = *static_cast<cminor::parsing::ValueObject<cminor::pom::DeclSpecifier*>*>(fromDeclSpecifier_value.get());
            stack.pop();
        }
    }
    void PostTypeName(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeName_value = std::move(stack.top());
            context->fromTypeName = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeName*>*>(fromTypeName_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): declaration(), fromDeclSpecifier(), fromTypeName() {}
        cminor::pom::SimpleDeclaration* declaration;
        cminor::pom::DeclSpecifier* fromDeclSpecifier;
        cminor::pom::TypeName* fromTypeName;
    };
};

class DeclarationGrammar::DeclSpecifierRule : public cminor::parsing::Rule
{
public:
    DeclSpecifierRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::DeclSpecifier*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::DeclSpecifier*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeclSpecifierRule>(this, &DeclSpecifierRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeclSpecifierRule>(this, &DeclSpecifierRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DeclSpecifierRule>(this, &DeclSpecifierRule::A2Action));
        cminor::parsing::NonterminalParser* storageClassSpecifierNonterminalParser = GetNonterminal("StorageClassSpecifier");
        storageClassSpecifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeclSpecifierRule>(this, &DeclSpecifierRule::PostStorageClassSpecifier));
        cminor::parsing::NonterminalParser* typeSpecifierNonterminalParser = GetNonterminal("TypeSpecifier");
        typeSpecifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeclSpecifierRule>(this, &DeclSpecifierRule::PostTypeSpecifier));
        cminor::parsing::NonterminalParser* typedefNonterminalParser = GetNonterminal("Typedef");
        typedefNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DeclSpecifierRule>(this, &DeclSpecifierRule::PostTypedef));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromStorageClassSpecifier;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromTypeSpecifier;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromStorageClassSpecifier = *static_cast<cminor::parsing::ValueObject<cminor::pom::StorageClassSpecifier*>*>(fromStorageClassSpecifier_value.get());
            stack.pop();
        }
    }
    void PostTypeSpecifier(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeSpecifier_value = std::move(stack.top());
            context->fromTypeSpecifier = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeSpecifier*>*>(fromTypeSpecifier_value.get());
            stack.pop();
        }
    }
    void PostTypedef(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypedef_value = std::move(stack.top());
            context->fromTypedef = *static_cast<cminor::parsing::ValueObject<cminor::pom::DeclSpecifier*>*>(fromTypedef_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromStorageClassSpecifier(), fromTypeSpecifier(), fromTypedef() {}
        cminor::pom::DeclSpecifier* value;
        cminor::pom::StorageClassSpecifier* fromStorageClassSpecifier;
        cminor::pom::TypeSpecifier* fromTypeSpecifier;
        cminor::pom::DeclSpecifier* fromTypedef;
    };
};

class DeclarationGrammar::StorageClassSpecifierRule : public cminor::parsing::Rule
{
public:
    StorageClassSpecifierRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::StorageClassSpecifier*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::StorageClassSpecifier*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StorageClassSpecifierRule>(this, &StorageClassSpecifierRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new StorageClassSpecifier(std::string(matchBegin, matchEnd));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::pom::StorageClassSpecifier* value;
    };
};

class DeclarationGrammar::TypeSpecifierRule : public cminor::parsing::Rule
{
public:
    TypeSpecifierRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::TypeSpecifier*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::TypeSpecifier*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeSpecifierRule>(this, &TypeSpecifierRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeSpecifierRule>(this, &TypeSpecifierRule::A1Action));
        cminor::parsing::NonterminalParser* simpleTypeSpecifierNonterminalParser = GetNonterminal("SimpleTypeSpecifier");
        simpleTypeSpecifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TypeSpecifierRule>(this, &TypeSpecifierRule::PostSimpleTypeSpecifier));
        cminor::parsing::NonterminalParser* cVQualifierNonterminalParser = GetNonterminal("CVQualifier");
        cVQualifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TypeSpecifierRule>(this, &TypeSpecifierRule::PostCVQualifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromSimpleTypeSpecifier;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromSimpleTypeSpecifier = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeSpecifier*>*>(fromSimpleTypeSpecifier_value.get());
            stack.pop();
        }
    }
    void PostCVQualifier(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCVQualifier_value = std::move(stack.top());
            context->fromCVQualifier = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeSpecifier*>*>(fromCVQualifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromSimpleTypeSpecifier(), fromCVQualifier() {}
        cminor::pom::TypeSpecifier* value;
        cminor::pom::TypeSpecifier* fromSimpleTypeSpecifier;
        cminor::pom::TypeSpecifier* fromCVQualifier;
    };
};

class DeclarationGrammar::SimpleTypeSpecifierRule : public cminor::parsing::Rule
{
public:
    SimpleTypeSpecifierRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::TypeSpecifier*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::TypeSpecifier*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SimpleTypeSpecifierRule>(this, &SimpleTypeSpecifierRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new TypeSpecifier(std::string(matchBegin, matchEnd));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::pom::TypeSpecifier* value;
    };
};

class DeclarationGrammar::TypeNameRule : public cminor::parsing::Rule
{
public:
    TypeNameRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::TypeName*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::TypeName*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeNameRule>(this, &TypeNameRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypeNameRule>(this, &TypeNameRule::A1Action));
        cminor::parsing::NonterminalParser* qualifiedIdNonterminalParser = GetNonterminal("QualifiedId");
        qualifiedIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TypeNameRule>(this, &TypeNameRule::PostQualifiedId));
        cminor::parsing::NonterminalParser* templateArgumentListNonterminalParser = GetNonterminal("TemplateArgumentList");
        templateArgumentListNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<TypeNameRule>(this, &TypeNameRule::PreTemplateArgumentList));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new cminor::pom::TypeName(std::string(matchBegin, matchEnd));
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromQualifiedId = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromQualifiedId_value.get());
            stack.pop();
        }
    }
    void PreTemplateArgumentList(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::TypeName*>(context->value)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromQualifiedId() {}
        cminor::pom::TypeName* value;
        std::string fromQualifiedId;
    };
};

class DeclarationGrammar::TemplateArgumentListRule : public cminor::parsing::Rule
{
public:
    TemplateArgumentListRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable("cminor::pom::TypeName*", "typeName"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> typeName_value = std::move(stack.top());
        context->typeName = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeName*>*>(typeName_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TemplateArgumentListRule>(this, &TemplateArgumentListRule::A0Action));
        cminor::parsing::NonterminalParser* templateArgumentNonterminalParser = GetNonterminal("TemplateArgument");
        templateArgumentNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TemplateArgumentListRule>(this, &TemplateArgumentListRule::PostTemplateArgument));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromTemplateArgument = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromTemplateArgument_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): typeName(), fromTemplateArgument() {}
        cminor::pom::TypeName* typeName;
        cminor::pom::CppObject* fromTemplateArgument;
    };
};

class DeclarationGrammar::TemplateArgumentRule : public cminor::parsing::Rule
{
public:
    TemplateArgumentRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::CppObject*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::CppObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TemplateArgumentRule>(this, &TemplateArgumentRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TemplateArgumentRule>(this, &TemplateArgumentRule::A1Action));
        cminor::parsing::NonterminalParser* typeIdNonterminalParser = GetNonterminal("TypeId");
        typeIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TemplateArgumentRule>(this, &TemplateArgumentRule::PostTypeId));
        cminor::parsing::NonterminalParser* assignmentExpressionNonterminalParser = GetNonterminal("AssignmentExpression");
        assignmentExpressionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<TemplateArgumentRule>(this, &TemplateArgumentRule::PostAssignmentExpression));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromTypeId;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromTypeId = *static_cast<cminor::parsing::ValueObject<cminor::pom::TypeId*>*>(fromTypeId_value.get());
            stack.pop();
        }
    }
    void PostAssignmentExpression(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAssignmentExpression_value = std::move(stack.top());
            context->fromAssignmentExpression = *static_cast<cminor::parsing::ValueObject<cminor::pom::CppObject*>*>(fromAssignmentExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromTypeId(), fromAssignmentExpression() {}
        cminor::pom::CppObject* value;
        cminor::pom::TypeId* fromTypeId;
        cminor::pom::CppObject* fromAssignmentExpression;
    };
};

class DeclarationGrammar::TypedefRule : public cminor::parsing::Rule
{
public:
    TypedefRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::DeclSpecifier*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::DeclSpecifier*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TypedefRule>(this, &TypedefRule::A0Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Typedef;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::pom::DeclSpecifier* value;
    };
};

class DeclarationGrammar::CVQualifierRule : public cminor::parsing::Rule
{
public:
    CVQualifierRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::TypeSpecifier*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::TypeSpecifier*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CVQualifierRule>(this, &CVQualifierRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CVQualifierRule>(this, &CVQualifierRule::A1Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Const;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Volatile;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::pom::TypeSpecifier* value;
    };
};

class DeclarationGrammar::NamespaceAliasDefinitionRule : public cminor::parsing::Rule
{
public:
    NamespaceAliasDefinitionRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::UsingObject*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::UsingObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NamespaceAliasDefinitionRule>(this, &NamespaceAliasDefinitionRule::A0Action));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NamespaceAliasDefinitionRule>(this, &NamespaceAliasDefinitionRule::PostIdentifier));
        cminor::parsing::NonterminalParser* qualifiedIdNonterminalParser = GetNonterminal("QualifiedId");
        qualifiedIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NamespaceAliasDefinitionRule>(this, &NamespaceAliasDefinitionRule::PostQualifiedId));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromIdentifier = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
    void PostQualifiedId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromQualifiedId_value = std::move(stack.top());
            context->fromQualifiedId = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromQualifiedId_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromIdentifier(), fromQualifiedId() {}
        cminor::pom::UsingObject* value;
        std::string fromIdentifier;
        std::string fromQualifiedId;
    };
};

class DeclarationGrammar::UsingDeclarationRule : public cminor::parsing::Rule
{
public:
    UsingDeclarationRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::UsingObject*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::UsingObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UsingDeclarationRule>(this, &UsingDeclarationRule::A0Action));
        cminor::parsing::NonterminalParser* qualifiedIdNonterminalParser = GetNonterminal("QualifiedId");
        qualifiedIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UsingDeclarationRule>(this, &UsingDeclarationRule::PostQualifiedId));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromQualifiedId = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromQualifiedId_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromQualifiedId() {}
        cminor::pom::UsingObject* value;
        std::string fromQualifiedId;
    };
};

class DeclarationGrammar::UsingDirectiveRule : public cminor::parsing::Rule
{
public:
    UsingDirectiveRule(const std::string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName("cminor::pom::UsingObject*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::pom::UsingObject*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UsingDirectiveRule>(this, &UsingDirectiveRule::A0Action));
        cminor::parsing::NonterminalParser* qualifiedIdNonterminalParser = GetNonterminal("QualifiedId");
        qualifiedIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UsingDirectiveRule>(this, &UsingDirectiveRule::PostQualifiedId));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
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
            context->fromQualifiedId = *static_cast<cminor::parsing::ValueObject<std::string>*>(fromQualifiedId_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromQualifiedId() {}
        cminor::pom::UsingObject* value;
        std::string fromQualifiedId;
    };
};

void DeclarationGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("cminor.parsing.stdlib");
    if (!grammar0)
    {
        grammar0 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar("cpg.cpp.DeclaratorGrammar");
    if (!grammar1)
    {
        grammar1 = cpg::cpp::DeclaratorGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar("cpg.cpp.IdentifierGrammar");
    if (!grammar2)
    {
        grammar2 = cpg::cpp::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar("cpg.cpp.ExpressionGrammar");
    if (!grammar3)
    {
        grammar3 = cpg::cpp::ExpressionGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
}

void DeclarationGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("identifier", this, "cminor.parsing.stdlib.identifier"));
    AddRuleLink(new cminor::parsing::RuleLink("InitDeclaratorList", this, "DeclaratorGrammar.InitDeclaratorList"));
    AddRuleLink(new cminor::parsing::RuleLink("TypeId", this, "DeclaratorGrammar.TypeId"));
    AddRuleLink(new cminor::parsing::RuleLink("Identifier", this, "IdentifierGrammar.Identifier"));
    AddRuleLink(new cminor::parsing::RuleLink("QualifiedId", this, "IdentifierGrammar.QualifiedId"));
    AddRuleLink(new cminor::parsing::RuleLink("AssignmentExpression", this, "ExpressionGrammar.AssignmentExpression"));
    AddRule(new BlockDeclarationRule("BlockDeclaration", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::ActionParser("A0",
                        new cminor::parsing::NonterminalParser("NamespaceAliasDefinition", "NamespaceAliasDefinition", 0)),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::NonterminalParser("UsingDirective", "UsingDirective", 0))),
                new cminor::parsing::ActionParser("A2",
                    new cminor::parsing::NonterminalParser("UsingDeclaration", "UsingDeclaration", 0))),
            new cminor::parsing::ActionParser("A3",
                new cminor::parsing::NonterminalParser("SimpleDeclaration", "SimpleDeclaration", 0)))));
    AddRule(new SimpleDeclarationRule("SimpleDeclaration", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::EmptyParser()),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::NonterminalParser("DeclSpecifierSeq", "DeclSpecifierSeq", 1)),
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::ActionParser("A2",
                                new cminor::parsing::NonterminalParser("InitDeclaratorList", "InitDeclaratorList", 0)))),
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new DeclSpecifierSeqRule("DeclSpecifierSeq", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::PositiveParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::NonterminalParser("DeclSpecifier", "DeclSpecifier", 0))),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::NonterminalParser("TypeName", "TypeName", 0)))));
    AddRule(new DeclSpecifierRule("DeclSpecifier", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::NonterminalParser("StorageClassSpecifier", "StorageClassSpecifier", 0)),
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("TypeSpecifier", "TypeSpecifier", 0))),
            new cminor::parsing::ActionParser("A2",
                new cminor::parsing::NonterminalParser("Typedef", "Typedef", 0)))));
    AddRule(new StorageClassSpecifierRule("StorageClassSpecifier", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::KeywordListParser("identifier", keywords0))));
    AddRule(new TypeSpecifierRule("TypeSpecifier", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("SimpleTypeSpecifier", "SimpleTypeSpecifier", 0)),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::NonterminalParser("CVQualifier", "CVQualifier", 0)))));
    AddRule(new SimpleTypeSpecifierRule("SimpleTypeSpecifier", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::KeywordListParser("identifier", keywords1))));
    AddRule(new TypeNameRule("TypeName", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("QualifiedId", "QualifiedId", 0)),
            new cminor::parsing::OptionalParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser("A1",
                            new cminor::parsing::CharParser('<')),
                        new cminor::parsing::NonterminalParser("TemplateArgumentList", "TemplateArgumentList", 1)),
                    new cminor::parsing::CharParser('>'))))));
    AddRule(new TemplateArgumentListRule("TemplateArgumentList", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ListParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("TemplateArgument", "TemplateArgument", 0)),
            new cminor::parsing::CharParser(','))));
    AddRule(new TemplateArgumentRule("TemplateArgument", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("TypeId", "TypeId", 0)),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::NonterminalParser("AssignmentExpression", "AssignmentExpression", 0)))));
    AddRule(new TypedefRule("Typedef", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::KeywordParser("typedef"))));
    AddRule(new CVQualifierRule("CVQualifier", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::KeywordParser("const")),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::KeywordParser("volatile")))));
    AddRule(new NamespaceAliasDefinitionRule("NamespaceAliasDefinition", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::KeywordParser("namespace"),
                        new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0)),
                    new cminor::parsing::CharParser('=')),
                new cminor::parsing::NonterminalParser("QualifiedId", "QualifiedId", 0)),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::CharParser(';')))));
    AddRule(new UsingDeclarationRule("UsingDeclaration", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser("using"),
                new cminor::parsing::NonterminalParser("QualifiedId", "QualifiedId", 0)),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::CharParser(';')))));
    AddRule(new UsingDirectiveRule("UsingDirective", GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser("using"),
                    new cminor::parsing::KeywordParser("namespace")),
                new cminor::parsing::NonterminalParser("QualifiedId", "QualifiedId", 0)),
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::CharParser(';')))));
}

} } // namespace cpg.cpp
