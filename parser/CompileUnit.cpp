#include "CompileUnit.hpp"
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
#include <cminor/parser/Identifier.hpp>
#include <cminor/parser/Function.hpp>
#include <cminor/parser/Class.hpp>
#include <cminor/parser/Interface.hpp>
#include <cminor/parser/Enumeration.hpp>
#include <cminor/parser/Constant.hpp>
#include <cminor/parser/Delegate.hpp>

namespace cminor { namespace parser {

using namespace cminor::parsing;
using namespace cminor::util;
using namespace cminor::unicode;

CompileUnitGrammar* CompileUnitGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

CompileUnitGrammar* CompileUnitGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    CompileUnitGrammar* grammar(new CompileUnitGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

CompileUnitGrammar::CompileUnitGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("CompileUnitGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.parser")), parsingDomain_)
{
    SetOwner(0);
}

CompileUnitNode* CompileUnitGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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
    stack.push(std::unique_ptr<cminor::parsing::Object>(new ValueObject<ParsingContext*>(ctx)));
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
    CompileUnitNode* result = *static_cast<cminor::parsing::ValueObject<CompileUnitNode*>*>(value.get());
    stack.pop();
    return result;
}

class CompileUnitGrammar::CompileUnitRule : public cminor::parsing::Rule
{
public:
    CompileUnitRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("CompileUnitNode*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context->ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<CompileUnitNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CompileUnitRule>(this, &CompileUnitRule::A0Action));
        cminor::parsing::NonterminalParser* namespaceContentNonterminalParser = GetNonterminal(ToUtf32("NamespaceContent"));
        namespaceContentNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<CompileUnitRule>(this, &CompileUnitRule::PreNamespaceContent));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new CompileUnitNode(span, fileName);
    }
    void PreNamespaceContent(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<NamespaceNode*>(context->value->GlobalNs())));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value() {}
        ParsingContext* ctx;
        CompileUnitNode* value;
    };
};

class CompileUnitGrammar::NamespaceContentRule : public cminor::parsing::Rule
{
public:
    NamespaceContentRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        AddInheritedAttribute(AttrOrVariable(ToUtf32("NamespaceNode*"), ToUtf32("ns")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> ns_value = std::move(stack.top());
        context->ns = *static_cast<cminor::parsing::ValueObject<NamespaceNode*>*>(ns_value.get());
        stack.pop();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context->ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::NonterminalParser* usingDirectivesNonterminalParser = GetNonterminal(ToUtf32("UsingDirectives"));
        usingDirectivesNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<NamespaceContentRule>(this, &NamespaceContentRule::PreUsingDirectives));
        cminor::parsing::NonterminalParser* definitionsNonterminalParser = GetNonterminal(ToUtf32("Definitions"));
        definitionsNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<NamespaceContentRule>(this, &NamespaceContentRule::PreDefinitions));
    }
    void PreUsingDirectives(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<NamespaceNode*>(context->ns)));
    }
    void PreDefinitions(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<NamespaceNode*>(context->ns)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), ns() {}
        ParsingContext* ctx;
        NamespaceNode* ns;
    };
};

class CompileUnitGrammar::UsingDirectivesRule : public cminor::parsing::Rule
{
public:
    UsingDirectivesRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        AddInheritedAttribute(AttrOrVariable(ToUtf32("NamespaceNode*"), ToUtf32("ns")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> ns_value = std::move(stack.top());
        context->ns = *static_cast<cminor::parsing::ValueObject<NamespaceNode*>*>(ns_value.get());
        stack.pop();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context->ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::NonterminalParser* usingDirectiveNonterminalParser = GetNonterminal(ToUtf32("UsingDirective"));
        usingDirectiveNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<UsingDirectivesRule>(this, &UsingDirectivesRule::PreUsingDirective));
    }
    void PreUsingDirective(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<NamespaceNode*>(context->ns)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), ns() {}
        ParsingContext* ctx;
        NamespaceNode* ns;
    };
};

class CompileUnitGrammar::UsingDirectiveRule : public cminor::parsing::Rule
{
public:
    UsingDirectiveRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        AddInheritedAttribute(AttrOrVariable(ToUtf32("NamespaceNode*"), ToUtf32("ns")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> ns_value = std::move(stack.top());
        context->ns = *static_cast<cminor::parsing::ValueObject<NamespaceNode*>*>(ns_value.get());
        stack.pop();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context->ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UsingDirectiveRule>(this, &UsingDirectiveRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UsingDirectiveRule>(this, &UsingDirectiveRule::A1Action));
        cminor::parsing::NonterminalParser* usingAliasDirectiveNonterminalParser = GetNonterminal(ToUtf32("UsingAliasDirective"));
        usingAliasDirectiveNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UsingDirectiveRule>(this, &UsingDirectiveRule::PostUsingAliasDirective));
        cminor::parsing::NonterminalParser* usingNamespaceDirectiveNonterminalParser = GetNonterminal(ToUtf32("UsingNamespaceDirective"));
        usingNamespaceDirectiveNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UsingDirectiveRule>(this, &UsingDirectiveRule::PostUsingNamespaceDirective));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ns->AddMember(context->fromUsingAliasDirective);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ns->AddMember(context->fromUsingNamespaceDirective);
    }
    void PostUsingAliasDirective(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromUsingAliasDirective_value = std::move(stack.top());
            context->fromUsingAliasDirective = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromUsingAliasDirective_value.get());
            stack.pop();
        }
    }
    void PostUsingNamespaceDirective(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromUsingNamespaceDirective_value = std::move(stack.top());
            context->fromUsingNamespaceDirective = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromUsingNamespaceDirective_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), ns(), fromUsingAliasDirective(), fromUsingNamespaceDirective() {}
        ParsingContext* ctx;
        NamespaceNode* ns;
        Node* fromUsingAliasDirective;
        Node* fromUsingNamespaceDirective;
    };
};

class CompileUnitGrammar::UsingAliasDirectiveRule : public cminor::parsing::Rule
{
public:
    UsingAliasDirectiveRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("Node*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<IdentifierNode>"), ToUtf32("id")));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UsingAliasDirectiveRule>(this, &UsingAliasDirectiveRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UsingAliasDirectiveRule>(this, &UsingAliasDirectiveRule::A1Action));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("Identifier"));
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UsingAliasDirectiveRule>(this, &UsingAliasDirectiveRule::PostIdentifier));
        cminor::parsing::NonterminalParser* qualifiedIdNonterminalParser = GetNonterminal(ToUtf32("QualifiedId"));
        qualifiedIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UsingAliasDirectiveRule>(this, &UsingAliasDirectiveRule::PostQualifiedId));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new AliasNode(span, context->id.release(), context->fromQualifiedId);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->id.reset(context->fromIdentifier);
    }
    void PostIdentifier(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIdentifier_value = std::move(stack.top());
            context->fromIdentifier = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
    void PostQualifiedId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromQualifiedId_value = std::move(stack.top());
            context->fromQualifiedId = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromQualifiedId_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), id(), fromIdentifier(), fromQualifiedId() {}
        Node* value;
        std::unique_ptr<IdentifierNode> id;
        IdentifierNode* fromIdentifier;
        IdentifierNode* fromQualifiedId;
    };
};

class CompileUnitGrammar::UsingNamespaceDirectiveRule : public cminor::parsing::Rule
{
public:
    UsingNamespaceDirectiveRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("Node*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UsingNamespaceDirectiveRule>(this, &UsingNamespaceDirectiveRule::A0Action));
        cminor::parsing::NonterminalParser* qualifiedIdNonterminalParser = GetNonterminal(ToUtf32("QualifiedId"));
        qualifiedIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UsingNamespaceDirectiveRule>(this, &UsingNamespaceDirectiveRule::PostQualifiedId));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new NamespaceImportNode(span, context->fromQualifiedId);
    }
    void PostQualifiedId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromQualifiedId_value = std::move(stack.top());
            context->fromQualifiedId = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromQualifiedId_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromQualifiedId() {}
        Node* value;
        IdentifierNode* fromQualifiedId;
    };
};

class CompileUnitGrammar::DefinitionsRule : public cminor::parsing::Rule
{
public:
    DefinitionsRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        AddInheritedAttribute(AttrOrVariable(ToUtf32("NamespaceNode*"), ToUtf32("ns")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> ns_value = std::move(stack.top());
        context->ns = *static_cast<cminor::parsing::ValueObject<NamespaceNode*>*>(ns_value.get());
        stack.pop();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context->ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DefinitionsRule>(this, &DefinitionsRule::A0Action));
        cminor::parsing::NonterminalParser* definitionNonterminalParser = GetNonterminal(ToUtf32("Definition"));
        definitionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DefinitionsRule>(this, &DefinitionsRule::PreDefinition));
        definitionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DefinitionsRule>(this, &DefinitionsRule::PostDefinition));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ns->AddMember(context->fromDefinition);
    }
    void PreDefinition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<NamespaceNode*>(context->ns)));
    }
    void PostDefinition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDefinition_value = std::move(stack.top());
            context->fromDefinition = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromDefinition_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), ns(), fromDefinition() {}
        ParsingContext* ctx;
        NamespaceNode* ns;
        Node* fromDefinition;
    };
};

class CompileUnitGrammar::DefinitionRule : public cminor::parsing::Rule
{
public:
    DefinitionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        AddInheritedAttribute(AttrOrVariable(ToUtf32("NamespaceNode*"), ToUtf32("ns")));
        SetValueTypeName(ToUtf32("Node*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> ns_value = std::move(stack.top());
        context->ns = *static_cast<cminor::parsing::ValueObject<NamespaceNode*>*>(ns_value.get());
        stack.pop();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context->ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DefinitionRule>(this, &DefinitionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DefinitionRule>(this, &DefinitionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DefinitionRule>(this, &DefinitionRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DefinitionRule>(this, &DefinitionRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction(ToUtf32("A4"));
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DefinitionRule>(this, &DefinitionRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction(ToUtf32("A5"));
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DefinitionRule>(this, &DefinitionRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction(ToUtf32("A6"));
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DefinitionRule>(this, &DefinitionRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction(ToUtf32("A7"));
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DefinitionRule>(this, &DefinitionRule::A7Action));
        cminor::parsing::NonterminalParser* namespaceDefinitionNonterminalParser = GetNonterminal(ToUtf32("NamespaceDefinition"));
        namespaceDefinitionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DefinitionRule>(this, &DefinitionRule::PreNamespaceDefinition));
        namespaceDefinitionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DefinitionRule>(this, &DefinitionRule::PostNamespaceDefinition));
        cminor::parsing::NonterminalParser* functionDefinitionNonterminalParser = GetNonterminal(ToUtf32("FunctionDefinition"));
        functionDefinitionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DefinitionRule>(this, &DefinitionRule::PreFunctionDefinition));
        functionDefinitionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DefinitionRule>(this, &DefinitionRule::PostFunctionDefinition));
        cminor::parsing::NonterminalParser* classDefinitionNonterminalParser = GetNonterminal(ToUtf32("ClassDefinition"));
        classDefinitionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DefinitionRule>(this, &DefinitionRule::PreClassDefinition));
        classDefinitionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DefinitionRule>(this, &DefinitionRule::PostClassDefinition));
        cminor::parsing::NonterminalParser* interfaceDefinitionNonterminalParser = GetNonterminal(ToUtf32("InterfaceDefinition"));
        interfaceDefinitionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DefinitionRule>(this, &DefinitionRule::PreInterfaceDefinition));
        interfaceDefinitionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DefinitionRule>(this, &DefinitionRule::PostInterfaceDefinition));
        cminor::parsing::NonterminalParser* enumTypeDefinitionNonterminalParser = GetNonterminal(ToUtf32("EnumTypeDefinition"));
        enumTypeDefinitionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DefinitionRule>(this, &DefinitionRule::PreEnumTypeDefinition));
        enumTypeDefinitionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DefinitionRule>(this, &DefinitionRule::PostEnumTypeDefinition));
        cminor::parsing::NonterminalParser* constantDefinitionNonterminalParser = GetNonterminal(ToUtf32("ConstantDefinition"));
        constantDefinitionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DefinitionRule>(this, &DefinitionRule::PreConstantDefinition));
        constantDefinitionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DefinitionRule>(this, &DefinitionRule::PostConstantDefinition));
        cminor::parsing::NonterminalParser* delegateDefinitionNonterminalParser = GetNonterminal(ToUtf32("DelegateDefinition"));
        delegateDefinitionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DefinitionRule>(this, &DefinitionRule::PreDelegateDefinition));
        delegateDefinitionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DefinitionRule>(this, &DefinitionRule::PostDelegateDefinition));
        cminor::parsing::NonterminalParser* classDelegateDefinitionNonterminalParser = GetNonterminal(ToUtf32("ClassDelegateDefinition"));
        classDelegateDefinitionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DefinitionRule>(this, &DefinitionRule::PreClassDelegateDefinition));
        classDelegateDefinitionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DefinitionRule>(this, &DefinitionRule::PostClassDelegateDefinition));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromNamespaceDefinition;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromFunctionDefinition;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromClassDefinition;
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromInterfaceDefinition;
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromEnumTypeDefinition;
    }
    void A5Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromConstantDefinition;
    }
    void A6Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromDelegateDefinition;
    }
    void A7Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromClassDelegateDefinition;
    }
    void PreNamespaceDefinition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<NamespaceNode*>(context->ns)));
    }
    void PostNamespaceDefinition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromNamespaceDefinition_value = std::move(stack.top());
            context->fromNamespaceDefinition = *static_cast<cminor::parsing::ValueObject<NamespaceNode*>*>(fromNamespaceDefinition_value.get());
            stack.pop();
        }
    }
    void PreFunctionDefinition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostFunctionDefinition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFunctionDefinition_value = std::move(stack.top());
            context->fromFunctionDefinition = *static_cast<cminor::parsing::ValueObject<FunctionNode*>*>(fromFunctionDefinition_value.get());
            stack.pop();
        }
    }
    void PreClassDefinition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostClassDefinition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromClassDefinition_value = std::move(stack.top());
            context->fromClassDefinition = *static_cast<cminor::parsing::ValueObject<ClassNode*>*>(fromClassDefinition_value.get());
            stack.pop();
        }
    }
    void PreInterfaceDefinition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostInterfaceDefinition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromInterfaceDefinition_value = std::move(stack.top());
            context->fromInterfaceDefinition = *static_cast<cminor::parsing::ValueObject<InterfaceNode*>*>(fromInterfaceDefinition_value.get());
            stack.pop();
        }
    }
    void PreEnumTypeDefinition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostEnumTypeDefinition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromEnumTypeDefinition_value = std::move(stack.top());
            context->fromEnumTypeDefinition = *static_cast<cminor::parsing::ValueObject<EnumTypeNode*>*>(fromEnumTypeDefinition_value.get());
            stack.pop();
        }
    }
    void PreConstantDefinition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostConstantDefinition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromConstantDefinition_value = std::move(stack.top());
            context->fromConstantDefinition = *static_cast<cminor::parsing::ValueObject<ConstantNode*>*>(fromConstantDefinition_value.get());
            stack.pop();
        }
    }
    void PreDelegateDefinition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostDelegateDefinition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDelegateDefinition_value = std::move(stack.top());
            context->fromDelegateDefinition = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromDelegateDefinition_value.get());
            stack.pop();
        }
    }
    void PreClassDelegateDefinition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostClassDelegateDefinition(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromClassDelegateDefinition_value = std::move(stack.top());
            context->fromClassDelegateDefinition = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromClassDelegateDefinition_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), ns(), value(), fromNamespaceDefinition(), fromFunctionDefinition(), fromClassDefinition(), fromInterfaceDefinition(), fromEnumTypeDefinition(), fromConstantDefinition(), fromDelegateDefinition(), fromClassDelegateDefinition() {}
        ParsingContext* ctx;
        NamespaceNode* ns;
        Node* value;
        NamespaceNode* fromNamespaceDefinition;
        FunctionNode* fromFunctionDefinition;
        ClassNode* fromClassDefinition;
        InterfaceNode* fromInterfaceDefinition;
        EnumTypeNode* fromEnumTypeDefinition;
        ConstantNode* fromConstantDefinition;
        Node* fromDelegateDefinition;
        Node* fromClassDelegateDefinition;
    };
};

class CompileUnitGrammar::NamespaceDefinitionRule : public cminor::parsing::Rule
{
public:
    NamespaceDefinitionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        AddInheritedAttribute(AttrOrVariable(ToUtf32("NamespaceNode*"), ToUtf32("ns")));
        SetValueTypeName(ToUtf32("NamespaceNode*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> ns_value = std::move(stack.top());
        context->ns = *static_cast<cminor::parsing::ValueObject<NamespaceNode*>*>(ns_value.get());
        stack.pop();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context->ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<NamespaceNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NamespaceDefinitionRule>(this, &NamespaceDefinitionRule::A0Action));
        cminor::parsing::NonterminalParser* qualifiedIdNonterminalParser = GetNonterminal(ToUtf32("QualifiedId"));
        qualifiedIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NamespaceDefinitionRule>(this, &NamespaceDefinitionRule::PostQualifiedId));
        cminor::parsing::NonterminalParser* namespaceContentNonterminalParser = GetNonterminal(ToUtf32("NamespaceContent"));
        namespaceContentNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<NamespaceDefinitionRule>(this, &NamespaceDefinitionRule::PreNamespaceContent));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new NamespaceNode(span, context->fromQualifiedId);
    }
    void PostQualifiedId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromQualifiedId_value = std::move(stack.top());
            context->fromQualifiedId = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromQualifiedId_value.get());
            stack.pop();
        }
    }
    void PreNamespaceContent(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<NamespaceNode*>(context->value)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), ns(), value(), fromQualifiedId() {}
        ParsingContext* ctx;
        NamespaceNode* ns;
        NamespaceNode* value;
        IdentifierNode* fromQualifiedId;
    };
};

class CompileUnitGrammar::FunctionDefinitionRule : public cminor::parsing::Rule
{
public:
    FunctionDefinitionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("FunctionNode*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context->ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<FunctionNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FunctionDefinitionRule>(this, &FunctionDefinitionRule::A0Action));
        cminor::parsing::NonterminalParser* functionNonterminalParser = GetNonterminal(ToUtf32("Function"));
        functionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<FunctionDefinitionRule>(this, &FunctionDefinitionRule::PreFunction));
        functionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<FunctionDefinitionRule>(this, &FunctionDefinitionRule::PostFunction));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromFunction;
    }
    void PreFunction(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostFunction(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFunction_value = std::move(stack.top());
            context->fromFunction = *static_cast<cminor::parsing::ValueObject<FunctionNode*>*>(fromFunction_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), fromFunction() {}
        ParsingContext* ctx;
        FunctionNode* value;
        FunctionNode* fromFunction;
    };
};

class CompileUnitGrammar::ClassDefinitionRule : public cminor::parsing::Rule
{
public:
    ClassDefinitionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("ClassNode*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context->ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ClassNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassDefinitionRule>(this, &ClassDefinitionRule::A0Action));
        cminor::parsing::NonterminalParser* classNonterminalParser = GetNonterminal(ToUtf32("Class"));
        classNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassDefinitionRule>(this, &ClassDefinitionRule::PreClass));
        classNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassDefinitionRule>(this, &ClassDefinitionRule::PostClass));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromClass;
    }
    void PreClass(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostClass(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromClass_value = std::move(stack.top());
            context->fromClass = *static_cast<cminor::parsing::ValueObject<ClassNode*>*>(fromClass_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), fromClass() {}
        ParsingContext* ctx;
        ClassNode* value;
        ClassNode* fromClass;
    };
};

class CompileUnitGrammar::InterfaceDefinitionRule : public cminor::parsing::Rule
{
public:
    InterfaceDefinitionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("InterfaceNode*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context->ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<InterfaceNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InterfaceDefinitionRule>(this, &InterfaceDefinitionRule::A0Action));
        cminor::parsing::NonterminalParser* interfaceNonterminalParser = GetNonterminal(ToUtf32("Interface"));
        interfaceNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<InterfaceDefinitionRule>(this, &InterfaceDefinitionRule::PreInterface));
        interfaceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InterfaceDefinitionRule>(this, &InterfaceDefinitionRule::PostInterface));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromInterface;
    }
    void PreInterface(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostInterface(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromInterface_value = std::move(stack.top());
            context->fromInterface = *static_cast<cminor::parsing::ValueObject<InterfaceNode*>*>(fromInterface_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), fromInterface() {}
        ParsingContext* ctx;
        InterfaceNode* value;
        InterfaceNode* fromInterface;
    };
};

class CompileUnitGrammar::EnumTypeDefinitionRule : public cminor::parsing::Rule
{
public:
    EnumTypeDefinitionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("EnumTypeNode*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context->ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<EnumTypeNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EnumTypeDefinitionRule>(this, &EnumTypeDefinitionRule::A0Action));
        cminor::parsing::NonterminalParser* enumTypeNonterminalParser = GetNonterminal(ToUtf32("EnumType"));
        enumTypeNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<EnumTypeDefinitionRule>(this, &EnumTypeDefinitionRule::PreEnumType));
        enumTypeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<EnumTypeDefinitionRule>(this, &EnumTypeDefinitionRule::PostEnumType));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromEnumType;
    }
    void PreEnumType(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostEnumType(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromEnumType_value = std::move(stack.top());
            context->fromEnumType = *static_cast<cminor::parsing::ValueObject<EnumTypeNode*>*>(fromEnumType_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), fromEnumType() {}
        ParsingContext* ctx;
        EnumTypeNode* value;
        EnumTypeNode* fromEnumType;
    };
};

class CompileUnitGrammar::ConstantDefinitionRule : public cminor::parsing::Rule
{
public:
    ConstantDefinitionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("ConstantNode*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context->ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ConstantNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConstantDefinitionRule>(this, &ConstantDefinitionRule::A0Action));
        cminor::parsing::NonterminalParser* constantNonterminalParser = GetNonterminal(ToUtf32("Constant"));
        constantNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ConstantDefinitionRule>(this, &ConstantDefinitionRule::PreConstant));
        constantNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConstantDefinitionRule>(this, &ConstantDefinitionRule::PostConstant));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromConstant;
    }
    void PreConstant(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostConstant(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromConstant_value = std::move(stack.top());
            context->fromConstant = *static_cast<cminor::parsing::ValueObject<ConstantNode*>*>(fromConstant_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), fromConstant() {}
        ParsingContext* ctx;
        ConstantNode* value;
        ConstantNode* fromConstant;
    };
};

class CompileUnitGrammar::DelegateDefinitionRule : public cminor::parsing::Rule
{
public:
    DelegateDefinitionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("Node*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context->ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DelegateDefinitionRule>(this, &DelegateDefinitionRule::A0Action));
        cminor::parsing::NonterminalParser* delegateNonterminalParser = GetNonterminal(ToUtf32("Delegate"));
        delegateNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DelegateDefinitionRule>(this, &DelegateDefinitionRule::PreDelegate));
        delegateNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DelegateDefinitionRule>(this, &DelegateDefinitionRule::PostDelegate));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromDelegate;
    }
    void PreDelegate(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostDelegate(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDelegate_value = std::move(stack.top());
            context->fromDelegate = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromDelegate_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), fromDelegate() {}
        ParsingContext* ctx;
        Node* value;
        Node* fromDelegate;
    };
};

class CompileUnitGrammar::ClassDelegateDefinitionRule : public cminor::parsing::Rule
{
public:
    ClassDelegateDefinitionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("Node*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context->ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassDelegateDefinitionRule>(this, &ClassDelegateDefinitionRule::A0Action));
        cminor::parsing::NonterminalParser* classDelegateNonterminalParser = GetNonterminal(ToUtf32("ClassDelegate"));
        classDelegateNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassDelegateDefinitionRule>(this, &ClassDelegateDefinitionRule::PreClassDelegate));
        classDelegateNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassDelegateDefinitionRule>(this, &ClassDelegateDefinitionRule::PostClassDelegate));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromClassDelegate;
    }
    void PreClassDelegate(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostClassDelegate(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromClassDelegate_value = std::move(stack.top());
            context->fromClassDelegate = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromClassDelegate_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), fromClassDelegate() {}
        ParsingContext* ctx;
        Node* value;
        Node* fromClassDelegate;
    };
};

void CompileUnitGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cminor.parser.DelegateGrammar"));
    if (!grammar0)
    {
        grammar0 = cminor::parser::DelegateGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar(ToUtf32("cminor.parser.EnumerationGrammar"));
    if (!grammar1)
    {
        grammar1 = cminor::parser::EnumerationGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar(ToUtf32("cminor.parser.IdentifierGrammar"));
    if (!grammar2)
    {
        grammar2 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar(ToUtf32("cminor.parser.ClassGrammar"));
    if (!grammar3)
    {
        grammar3 = cminor::parser::ClassGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
    cminor::parsing::Grammar* grammar4 = pd->GetGrammar(ToUtf32("cminor.parsing.stdlib"));
    if (!grammar4)
    {
        grammar4 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar4);
    cminor::parsing::Grammar* grammar5 = pd->GetGrammar(ToUtf32("cminor.parser.FunctionGrammar"));
    if (!grammar5)
    {
        grammar5 = cminor::parser::FunctionGrammar::Create(pd);
    }
    AddGrammarReference(grammar5);
    cminor::parsing::Grammar* grammar6 = pd->GetGrammar(ToUtf32("cminor.parser.InterfaceGrammar"));
    if (!grammar6)
    {
        grammar6 = cminor::parser::InterfaceGrammar::Create(pd);
    }
    AddGrammarReference(grammar6);
    cminor::parsing::Grammar* grammar7 = pd->GetGrammar(ToUtf32("cminor.parser.ConstantGrammar"));
    if (!grammar7)
    {
        grammar7 = cminor::parser::ConstantGrammar::Create(pd);
    }
    AddGrammarReference(grammar7);
}

void CompileUnitGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("EnumType"), this, ToUtf32("EnumerationGrammar.EnumType")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Class"), this, ToUtf32("ClassGrammar.Class")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("spaces_and_comments"), this, ToUtf32("cminor.parsing.stdlib.spaces_and_comments")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Interface"), this, ToUtf32("InterfaceGrammar.Interface")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("QualifiedId"), this, ToUtf32("IdentifierGrammar.QualifiedId")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("ClassDelegate"), this, ToUtf32("DelegateGrammar.ClassDelegate")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Identifier"), this, ToUtf32("IdentifierGrammar.Identifier")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Function"), this, ToUtf32("FunctionGrammar.Function")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Delegate"), this, ToUtf32("DelegateGrammar.Delegate")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Constant"), this, ToUtf32("ConstantGrammar.Constant")));
    AddRule(new CompileUnitRule(ToUtf32("CompileUnit"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::EmptyParser()),
            new cminor::parsing::NonterminalParser(ToUtf32("NamespaceContent"), ToUtf32("NamespaceContent"), 2))));
    AddRule(new NamespaceContentRule(ToUtf32("NamespaceContent"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::NonterminalParser(ToUtf32("UsingDirectives"), ToUtf32("UsingDirectives"), 2),
            new cminor::parsing::NonterminalParser(ToUtf32("Definitions"), ToUtf32("Definitions"), 2))));
    AddRule(new UsingDirectivesRule(ToUtf32("UsingDirectives"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::KleeneStarParser(
            new cminor::parsing::NonterminalParser(ToUtf32("UsingDirective"), ToUtf32("UsingDirective"), 2))));
    AddRule(new UsingDirectiveRule(ToUtf32("UsingDirective"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("UsingAliasDirective"), ToUtf32("UsingAliasDirective"), 0)),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::NonterminalParser(ToUtf32("UsingNamespaceDirective"), ToUtf32("UsingNamespaceDirective"), 0)))));
    AddRule(new UsingAliasDirectiveRule(ToUtf32("UsingAliasDirective"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::KeywordParser(ToUtf32("using")),
                            new cminor::parsing::ActionParser(ToUtf32("A1"),
                                new cminor::parsing::NonterminalParser(ToUtf32("Identifier"), ToUtf32("Identifier"), 0))),
                        new cminor::parsing::CharParser('=')),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("QualifiedId"), ToUtf32("QualifiedId"), 0))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new UsingNamespaceDirectiveRule(ToUtf32("UsingNamespaceDirective"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser(ToUtf32("using")),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("QualifiedId"), ToUtf32("QualifiedId"), 0))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new DefinitionsRule(ToUtf32("Definitions"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::KleeneStarParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("Definition"), ToUtf32("Definition"), 2)))));
    AddRule(new DefinitionRule(ToUtf32("Definition"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::AlternativeParser(
                                    new cminor::parsing::ActionParser(ToUtf32("A0"),
                                        new cminor::parsing::NonterminalParser(ToUtf32("NamespaceDefinition"), ToUtf32("NamespaceDefinition"), 2)),
                                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                                        new cminor::parsing::NonterminalParser(ToUtf32("FunctionDefinition"), ToUtf32("FunctionDefinition"), 1))),
                                new cminor::parsing::ActionParser(ToUtf32("A2"),
                                    new cminor::parsing::NonterminalParser(ToUtf32("ClassDefinition"), ToUtf32("ClassDefinition"), 1))),
                            new cminor::parsing::ActionParser(ToUtf32("A3"),
                                new cminor::parsing::NonterminalParser(ToUtf32("InterfaceDefinition"), ToUtf32("InterfaceDefinition"), 1))),
                        new cminor::parsing::ActionParser(ToUtf32("A4"),
                            new cminor::parsing::NonterminalParser(ToUtf32("EnumTypeDefinition"), ToUtf32("EnumTypeDefinition"), 1))),
                    new cminor::parsing::ActionParser(ToUtf32("A5"),
                        new cminor::parsing::NonterminalParser(ToUtf32("ConstantDefinition"), ToUtf32("ConstantDefinition"), 1))),
                new cminor::parsing::ActionParser(ToUtf32("A6"),
                    new cminor::parsing::NonterminalParser(ToUtf32("DelegateDefinition"), ToUtf32("DelegateDefinition"), 1))),
            new cminor::parsing::ActionParser(ToUtf32("A7"),
                new cminor::parsing::NonterminalParser(ToUtf32("ClassDelegateDefinition"), ToUtf32("ClassDelegateDefinition"), 1)))));
    AddRule(new NamespaceDefinitionRule(ToUtf32("NamespaceDefinition"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::ActionParser(ToUtf32("A0"),
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::KeywordParser(ToUtf32("namespace")),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("QualifiedId"), ToUtf32("QualifiedId"), 0)))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('{'))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("NamespaceContent"), ToUtf32("NamespaceContent"), 2))),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::CharParser('}')))));
    AddRule(new FunctionDefinitionRule(ToUtf32("FunctionDefinition"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::NonterminalParser(ToUtf32("Function"), ToUtf32("Function"), 1))));
    AddRule(new ClassDefinitionRule(ToUtf32("ClassDefinition"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::NonterminalParser(ToUtf32("Class"), ToUtf32("Class"), 1))));
    AddRule(new InterfaceDefinitionRule(ToUtf32("InterfaceDefinition"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::NonterminalParser(ToUtf32("Interface"), ToUtf32("Interface"), 1))));
    AddRule(new EnumTypeDefinitionRule(ToUtf32("EnumTypeDefinition"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::NonterminalParser(ToUtf32("EnumType"), ToUtf32("EnumType"), 1))));
    AddRule(new ConstantDefinitionRule(ToUtf32("ConstantDefinition"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::NonterminalParser(ToUtf32("Constant"), ToUtf32("Constant"), 1))));
    AddRule(new DelegateDefinitionRule(ToUtf32("DelegateDefinition"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::NonterminalParser(ToUtf32("Delegate"), ToUtf32("Delegate"), 1))));
    AddRule(new ClassDelegateDefinitionRule(ToUtf32("ClassDelegateDefinition"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::NonterminalParser(ToUtf32("ClassDelegate"), ToUtf32("ClassDelegate"), 1))));
    SetSkipRuleName(ToUtf32("spaces_and_comments"));
}

} } // namespace cminor.parser
