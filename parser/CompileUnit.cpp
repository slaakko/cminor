#include "CompileUnit.hpp"
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>
#include <cminor/parser/Identifier.hpp>
#include <cminor/parser/Function.hpp>
#include <cminor/parser/Class.hpp>
#include <cminor/parser/Interface.hpp>
#include <cminor/parser/Enumeration.hpp>
#include <cminor/parser/Constant.hpp>

namespace cminor { namespace parser {

using namespace cminor::parsing;

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

CompileUnitGrammar::CompileUnitGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("CompileUnitGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

CompileUnitNode* CompileUnitGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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
    stack.push(std::unique_ptr<cminor::parsing::Object>(new ValueObject<ParsingContext*>(ctx)));
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
    CompileUnitNode* result = *static_cast<cminor::parsing::ValueObject<CompileUnitNode*>*>(value.get());
    stack.pop();
    return result;
}

class CompileUnitGrammar::CompileUnitRule : public cminor::parsing::Rule
{
public:
    CompileUnitRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("CompileUnitNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<CompileUnitNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CompileUnitRule>(this, &CompileUnitRule::A0Action));
        cminor::parsing::NonterminalParser* namespaceContentNonterminalParser = GetNonterminal("NamespaceContent");
        namespaceContentNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<CompileUnitRule>(this, &CompileUnitRule::PreNamespaceContent));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new CompileUnitNode(span, fileName);
    }
    void PreNamespaceContent(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<NamespaceNode*>(context.value->GlobalNs())));
    }
private:
    struct Context
    {
        Context(): ctx(), value() {}
        ParsingContext* ctx;
        CompileUnitNode* value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class CompileUnitGrammar::NamespaceContentRule : public cminor::parsing::Rule
{
public:
    NamespaceContentRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("NamespaceNode*", "ns"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ns_value = std::move(stack.top());
        context.ns = *static_cast<cminor::parsing::ValueObject<NamespaceNode*>*>(ns_value.get());
        stack.pop();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::NonterminalParser* usingDirectivesNonterminalParser = GetNonterminal("UsingDirectives");
        usingDirectivesNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<NamespaceContentRule>(this, &NamespaceContentRule::PreUsingDirectives));
        cminor::parsing::NonterminalParser* definitionsNonterminalParser = GetNonterminal("Definitions");
        definitionsNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<NamespaceContentRule>(this, &NamespaceContentRule::PreDefinitions));
    }
    void PreUsingDirectives(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<NamespaceNode*>(context.ns)));
    }
    void PreDefinitions(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<NamespaceNode*>(context.ns)));
    }
private:
    struct Context
    {
        Context(): ctx(), ns() {}
        ParsingContext* ctx;
        NamespaceNode* ns;
    };
    std::stack<Context> contextStack;
    Context context;
};

class CompileUnitGrammar::UsingDirectivesRule : public cminor::parsing::Rule
{
public:
    UsingDirectivesRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("NamespaceNode*", "ns"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ns_value = std::move(stack.top());
        context.ns = *static_cast<cminor::parsing::ValueObject<NamespaceNode*>*>(ns_value.get());
        stack.pop();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::NonterminalParser* usingDirectiveNonterminalParser = GetNonterminal("UsingDirective");
        usingDirectiveNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<UsingDirectivesRule>(this, &UsingDirectivesRule::PreUsingDirective));
    }
    void PreUsingDirective(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<NamespaceNode*>(context.ns)));
    }
private:
    struct Context
    {
        Context(): ctx(), ns() {}
        ParsingContext* ctx;
        NamespaceNode* ns;
    };
    std::stack<Context> contextStack;
    Context context;
};

class CompileUnitGrammar::UsingDirectiveRule : public cminor::parsing::Rule
{
public:
    UsingDirectiveRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("NamespaceNode*", "ns"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ns_value = std::move(stack.top());
        context.ns = *static_cast<cminor::parsing::ValueObject<NamespaceNode*>*>(ns_value.get());
        stack.pop();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UsingDirectiveRule>(this, &UsingDirectiveRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UsingDirectiveRule>(this, &UsingDirectiveRule::A1Action));
        cminor::parsing::NonterminalParser* usingAliasDirectiveNonterminalParser = GetNonterminal("UsingAliasDirective");
        usingAliasDirectiveNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UsingDirectiveRule>(this, &UsingDirectiveRule::PostUsingAliasDirective));
        cminor::parsing::NonterminalParser* usingNamespaceDirectiveNonterminalParser = GetNonterminal("UsingNamespaceDirective");
        usingNamespaceDirectiveNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UsingDirectiveRule>(this, &UsingDirectiveRule::PostUsingNamespaceDirective));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ns->AddMember(context.fromUsingAliasDirective);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ns->AddMember(context.fromUsingNamespaceDirective);
    }
    void PostUsingAliasDirective(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromUsingAliasDirective_value = std::move(stack.top());
            context.fromUsingAliasDirective = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromUsingAliasDirective_value.get());
            stack.pop();
        }
    }
    void PostUsingNamespaceDirective(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromUsingNamespaceDirective_value = std::move(stack.top());
            context.fromUsingNamespaceDirective = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromUsingNamespaceDirective_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), ns(), fromUsingAliasDirective(), fromUsingNamespaceDirective() {}
        ParsingContext* ctx;
        NamespaceNode* ns;
        Node* fromUsingAliasDirective;
        Node* fromUsingNamespaceDirective;
    };
    std::stack<Context> contextStack;
    Context context;
};

class CompileUnitGrammar::UsingAliasDirectiveRule : public cminor::parsing::Rule
{
public:
    UsingAliasDirectiveRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<IdentifierNode>", "id"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UsingAliasDirectiveRule>(this, &UsingAliasDirectiveRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UsingAliasDirectiveRule>(this, &UsingAliasDirectiveRule::A1Action));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UsingAliasDirectiveRule>(this, &UsingAliasDirectiveRule::PostIdentifier));
        cminor::parsing::NonterminalParser* qualifiedIdNonterminalParser = GetNonterminal("QualifiedId");
        qualifiedIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UsingAliasDirectiveRule>(this, &UsingAliasDirectiveRule::PostQualifiedId));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new AliasNode(span, context.id.release(), context.fromQualifiedId);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.id.reset(context.fromIdentifier);
    }
    void PostIdentifier(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIdentifier_value = std::move(stack.top());
            context.fromIdentifier = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
    void PostQualifiedId(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromQualifiedId_value = std::move(stack.top());
            context.fromQualifiedId = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromQualifiedId_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), id(), fromIdentifier(), fromQualifiedId() {}
        Node* value;
        std::unique_ptr<IdentifierNode> id;
        IdentifierNode* fromIdentifier;
        IdentifierNode* fromQualifiedId;
    };
    std::stack<Context> contextStack;
    Context context;
};

class CompileUnitGrammar::UsingNamespaceDirectiveRule : public cminor::parsing::Rule
{
public:
    UsingNamespaceDirectiveRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Node*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UsingNamespaceDirectiveRule>(this, &UsingNamespaceDirectiveRule::A0Action));
        cminor::parsing::NonterminalParser* qualifiedIdNonterminalParser = GetNonterminal("QualifiedId");
        qualifiedIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<UsingNamespaceDirectiveRule>(this, &UsingNamespaceDirectiveRule::PostQualifiedId));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new NamespaceImportNode(span, context.fromQualifiedId);
    }
    void PostQualifiedId(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromQualifiedId_value = std::move(stack.top());
            context.fromQualifiedId = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromQualifiedId_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromQualifiedId() {}
        Node* value;
        IdentifierNode* fromQualifiedId;
    };
    std::stack<Context> contextStack;
    Context context;
};

class CompileUnitGrammar::DefinitionsRule : public cminor::parsing::Rule
{
public:
    DefinitionsRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("NamespaceNode*", "ns"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ns_value = std::move(stack.top());
        context.ns = *static_cast<cminor::parsing::ValueObject<NamespaceNode*>*>(ns_value.get());
        stack.pop();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DefinitionsRule>(this, &DefinitionsRule::A0Action));
        cminor::parsing::NonterminalParser* definitionNonterminalParser = GetNonterminal("Definition");
        definitionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DefinitionsRule>(this, &DefinitionsRule::PreDefinition));
        definitionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DefinitionsRule>(this, &DefinitionsRule::PostDefinition));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ns->AddMember(context.fromDefinition);
    }
    void PreDefinition(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<NamespaceNode*>(context.ns)));
    }
    void PostDefinition(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDefinition_value = std::move(stack.top());
            context.fromDefinition = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromDefinition_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), ns(), fromDefinition() {}
        ParsingContext* ctx;
        NamespaceNode* ns;
        Node* fromDefinition;
    };
    std::stack<Context> contextStack;
    Context context;
};

class CompileUnitGrammar::DefinitionRule : public cminor::parsing::Rule
{
public:
    DefinitionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("NamespaceNode*", "ns"));
        SetValueTypeName("Node*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ns_value = std::move(stack.top());
        context.ns = *static_cast<cminor::parsing::ValueObject<NamespaceNode*>*>(ns_value.get());
        stack.pop();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DefinitionRule>(this, &DefinitionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DefinitionRule>(this, &DefinitionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DefinitionRule>(this, &DefinitionRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DefinitionRule>(this, &DefinitionRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DefinitionRule>(this, &DefinitionRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DefinitionRule>(this, &DefinitionRule::A5Action));
        cminor::parsing::NonterminalParser* namespaceDefinitionNonterminalParser = GetNonterminal("NamespaceDefinition");
        namespaceDefinitionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DefinitionRule>(this, &DefinitionRule::PreNamespaceDefinition));
        namespaceDefinitionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DefinitionRule>(this, &DefinitionRule::PostNamespaceDefinition));
        cminor::parsing::NonterminalParser* functionDefinitionNonterminalParser = GetNonterminal("FunctionDefinition");
        functionDefinitionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DefinitionRule>(this, &DefinitionRule::PreFunctionDefinition));
        functionDefinitionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DefinitionRule>(this, &DefinitionRule::PostFunctionDefinition));
        cminor::parsing::NonterminalParser* classDefinitionNonterminalParser = GetNonterminal("ClassDefinition");
        classDefinitionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DefinitionRule>(this, &DefinitionRule::PreClassDefinition));
        classDefinitionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DefinitionRule>(this, &DefinitionRule::PostClassDefinition));
        cminor::parsing::NonterminalParser* interfaceDefinitionNonterminalParser = GetNonterminal("InterfaceDefinition");
        interfaceDefinitionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DefinitionRule>(this, &DefinitionRule::PreInterfaceDefinition));
        interfaceDefinitionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DefinitionRule>(this, &DefinitionRule::PostInterfaceDefinition));
        cminor::parsing::NonterminalParser* enumTypeDefinitionNonterminalParser = GetNonterminal("EnumTypeDefinition");
        enumTypeDefinitionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DefinitionRule>(this, &DefinitionRule::PreEnumTypeDefinition));
        enumTypeDefinitionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DefinitionRule>(this, &DefinitionRule::PostEnumTypeDefinition));
        cminor::parsing::NonterminalParser* constantDefinitionNonterminalParser = GetNonterminal("ConstantDefinition");
        constantDefinitionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<DefinitionRule>(this, &DefinitionRule::PreConstantDefinition));
        constantDefinitionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<DefinitionRule>(this, &DefinitionRule::PostConstantDefinition));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromNamespaceDefinition;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromFunctionDefinition;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromClassDefinition;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromInterfaceDefinition;
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromEnumTypeDefinition;
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromConstantDefinition;
    }
    void PreNamespaceDefinition(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<NamespaceNode*>(context.ns)));
    }
    void PostNamespaceDefinition(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromNamespaceDefinition_value = std::move(stack.top());
            context.fromNamespaceDefinition = *static_cast<cminor::parsing::ValueObject<NamespaceNode*>*>(fromNamespaceDefinition_value.get());
            stack.pop();
        }
    }
    void PreFunctionDefinition(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostFunctionDefinition(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFunctionDefinition_value = std::move(stack.top());
            context.fromFunctionDefinition = *static_cast<cminor::parsing::ValueObject<FunctionNode*>*>(fromFunctionDefinition_value.get());
            stack.pop();
        }
    }
    void PreClassDefinition(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostClassDefinition(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromClassDefinition_value = std::move(stack.top());
            context.fromClassDefinition = *static_cast<cminor::parsing::ValueObject<ClassNode*>*>(fromClassDefinition_value.get());
            stack.pop();
        }
    }
    void PreInterfaceDefinition(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostInterfaceDefinition(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromInterfaceDefinition_value = std::move(stack.top());
            context.fromInterfaceDefinition = *static_cast<cminor::parsing::ValueObject<InterfaceNode*>*>(fromInterfaceDefinition_value.get());
            stack.pop();
        }
    }
    void PreEnumTypeDefinition(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostEnumTypeDefinition(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromEnumTypeDefinition_value = std::move(stack.top());
            context.fromEnumTypeDefinition = *static_cast<cminor::parsing::ValueObject<EnumTypeNode*>*>(fromEnumTypeDefinition_value.get());
            stack.pop();
        }
    }
    void PreConstantDefinition(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostConstantDefinition(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromConstantDefinition_value = std::move(stack.top());
            context.fromConstantDefinition = *static_cast<cminor::parsing::ValueObject<ConstantNode*>*>(fromConstantDefinition_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), ns(), value(), fromNamespaceDefinition(), fromFunctionDefinition(), fromClassDefinition(), fromInterfaceDefinition(), fromEnumTypeDefinition(), fromConstantDefinition() {}
        ParsingContext* ctx;
        NamespaceNode* ns;
        Node* value;
        NamespaceNode* fromNamespaceDefinition;
        FunctionNode* fromFunctionDefinition;
        ClassNode* fromClassDefinition;
        InterfaceNode* fromInterfaceDefinition;
        EnumTypeNode* fromEnumTypeDefinition;
        ConstantNode* fromConstantDefinition;
    };
    std::stack<Context> contextStack;
    Context context;
};

class CompileUnitGrammar::NamespaceDefinitionRule : public cminor::parsing::Rule
{
public:
    NamespaceDefinitionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("NamespaceNode*", "ns"));
        SetValueTypeName("NamespaceNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ns_value = std::move(stack.top());
        context.ns = *static_cast<cminor::parsing::ValueObject<NamespaceNode*>*>(ns_value.get());
        stack.pop();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<NamespaceNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NamespaceDefinitionRule>(this, &NamespaceDefinitionRule::A0Action));
        cminor::parsing::NonterminalParser* qualifiedIdNonterminalParser = GetNonterminal("QualifiedId");
        qualifiedIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<NamespaceDefinitionRule>(this, &NamespaceDefinitionRule::PostQualifiedId));
        cminor::parsing::NonterminalParser* namespaceContentNonterminalParser = GetNonterminal("NamespaceContent");
        namespaceContentNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<NamespaceDefinitionRule>(this, &NamespaceDefinitionRule::PreNamespaceContent));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new NamespaceNode(span, context.fromQualifiedId);
    }
    void PostQualifiedId(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromQualifiedId_value = std::move(stack.top());
            context.fromQualifiedId = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromQualifiedId_value.get());
            stack.pop();
        }
    }
    void PreNamespaceContent(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<NamespaceNode*>(context.value)));
    }
private:
    struct Context
    {
        Context(): ctx(), ns(), value(), fromQualifiedId() {}
        ParsingContext* ctx;
        NamespaceNode* ns;
        NamespaceNode* value;
        IdentifierNode* fromQualifiedId;
    };
    std::stack<Context> contextStack;
    Context context;
};

class CompileUnitGrammar::FunctionDefinitionRule : public cminor::parsing::Rule
{
public:
    FunctionDefinitionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("FunctionNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<FunctionNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FunctionDefinitionRule>(this, &FunctionDefinitionRule::A0Action));
        cminor::parsing::NonterminalParser* functionNonterminalParser = GetNonterminal("Function");
        functionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<FunctionDefinitionRule>(this, &FunctionDefinitionRule::PreFunction));
        functionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<FunctionDefinitionRule>(this, &FunctionDefinitionRule::PostFunction));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromFunction;
    }
    void PreFunction(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostFunction(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFunction_value = std::move(stack.top());
            context.fromFunction = *static_cast<cminor::parsing::ValueObject<FunctionNode*>*>(fromFunction_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromFunction() {}
        ParsingContext* ctx;
        FunctionNode* value;
        FunctionNode* fromFunction;
    };
    std::stack<Context> contextStack;
    Context context;
};

class CompileUnitGrammar::ClassDefinitionRule : public cminor::parsing::Rule
{
public:
    ClassDefinitionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("ClassNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ClassNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassDefinitionRule>(this, &ClassDefinitionRule::A0Action));
        cminor::parsing::NonterminalParser* classNonterminalParser = GetNonterminal("Class");
        classNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassDefinitionRule>(this, &ClassDefinitionRule::PreClass));
        classNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassDefinitionRule>(this, &ClassDefinitionRule::PostClass));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromClass;
    }
    void PreClass(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostClass(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromClass_value = std::move(stack.top());
            context.fromClass = *static_cast<cminor::parsing::ValueObject<ClassNode*>*>(fromClass_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromClass() {}
        ParsingContext* ctx;
        ClassNode* value;
        ClassNode* fromClass;
    };
    std::stack<Context> contextStack;
    Context context;
};

class CompileUnitGrammar::InterfaceDefinitionRule : public cminor::parsing::Rule
{
public:
    InterfaceDefinitionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("InterfaceNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<InterfaceNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InterfaceDefinitionRule>(this, &InterfaceDefinitionRule::A0Action));
        cminor::parsing::NonterminalParser* interfaceNonterminalParser = GetNonterminal("Interface");
        interfaceNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<InterfaceDefinitionRule>(this, &InterfaceDefinitionRule::PreInterface));
        interfaceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InterfaceDefinitionRule>(this, &InterfaceDefinitionRule::PostInterface));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromInterface;
    }
    void PreInterface(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostInterface(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromInterface_value = std::move(stack.top());
            context.fromInterface = *static_cast<cminor::parsing::ValueObject<InterfaceNode*>*>(fromInterface_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromInterface() {}
        ParsingContext* ctx;
        InterfaceNode* value;
        InterfaceNode* fromInterface;
    };
    std::stack<Context> contextStack;
    Context context;
};

class CompileUnitGrammar::EnumTypeDefinitionRule : public cminor::parsing::Rule
{
public:
    EnumTypeDefinitionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("EnumTypeNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<EnumTypeNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EnumTypeDefinitionRule>(this, &EnumTypeDefinitionRule::A0Action));
        cminor::parsing::NonterminalParser* enumTypeNonterminalParser = GetNonterminal("EnumType");
        enumTypeNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<EnumTypeDefinitionRule>(this, &EnumTypeDefinitionRule::PreEnumType));
        enumTypeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<EnumTypeDefinitionRule>(this, &EnumTypeDefinitionRule::PostEnumType));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromEnumType;
    }
    void PreEnumType(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostEnumType(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromEnumType_value = std::move(stack.top());
            context.fromEnumType = *static_cast<cminor::parsing::ValueObject<EnumTypeNode*>*>(fromEnumType_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromEnumType() {}
        ParsingContext* ctx;
        EnumTypeNode* value;
        EnumTypeNode* fromEnumType;
    };
    std::stack<Context> contextStack;
    Context context;
};

class CompileUnitGrammar::ConstantDefinitionRule : public cminor::parsing::Rule
{
public:
    ConstantDefinitionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("ConstantNode*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ConstantNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConstantDefinitionRule>(this, &ConstantDefinitionRule::A0Action));
        cminor::parsing::NonterminalParser* constantNonterminalParser = GetNonterminal("Constant");
        constantNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ConstantDefinitionRule>(this, &ConstantDefinitionRule::PreConstant));
        constantNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConstantDefinitionRule>(this, &ConstantDefinitionRule::PostConstant));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromConstant;
    }
    void PreConstant(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostConstant(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromConstant_value = std::move(stack.top());
            context.fromConstant = *static_cast<cminor::parsing::ValueObject<ConstantNode*>*>(fromConstant_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromConstant() {}
        ParsingContext* ctx;
        ConstantNode* value;
        ConstantNode* fromConstant;
    };
    std::stack<Context> contextStack;
    Context context;
};

void CompileUnitGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("cminor.parser.ClassGrammar");
    if (!grammar0)
    {
        grammar0 = cminor::parser::ClassGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar("cminor.parsing.stdlib");
    if (!grammar1)
    {
        grammar1 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar("cminor.parser.EnumerationGrammar");
    if (!grammar2)
    {
        grammar2 = cminor::parser::EnumerationGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar("cminor.parser.IdentifierGrammar");
    if (!grammar3)
    {
        grammar3 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
    cminor::parsing::Grammar* grammar4 = pd->GetGrammar("cminor.parser.FunctionGrammar");
    if (!grammar4)
    {
        grammar4 = cminor::parser::FunctionGrammar::Create(pd);
    }
    AddGrammarReference(grammar4);
    cminor::parsing::Grammar* grammar5 = pd->GetGrammar("cminor.parser.ConstantGrammar");
    if (!grammar5)
    {
        grammar5 = cminor::parser::ConstantGrammar::Create(pd);
    }
    AddGrammarReference(grammar5);
    cminor::parsing::Grammar* grammar6 = pd->GetGrammar("cminor.parser.InterfaceGrammar");
    if (!grammar6)
    {
        grammar6 = cminor::parser::InterfaceGrammar::Create(pd);
    }
    AddGrammarReference(grammar6);
}

void CompileUnitGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("spaces_and_comments", this, "cminor.parsing.stdlib.spaces_and_comments"));
    AddRuleLink(new cminor::parsing::RuleLink("Identifier", this, "IdentifierGrammar.Identifier"));
    AddRuleLink(new cminor::parsing::RuleLink("QualifiedId", this, "IdentifierGrammar.QualifiedId"));
    AddRuleLink(new cminor::parsing::RuleLink("Function", this, "FunctionGrammar.Function"));
    AddRuleLink(new cminor::parsing::RuleLink("Interface", this, "InterfaceGrammar.Interface"));
    AddRuleLink(new cminor::parsing::RuleLink("Class", this, "ClassGrammar.Class"));
    AddRuleLink(new cminor::parsing::RuleLink("EnumType", this, "EnumerationGrammar.EnumType"));
    AddRuleLink(new cminor::parsing::RuleLink("Constant", this, "ConstantGrammar.Constant"));
    AddRule(new CompileUnitRule("CompileUnit", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::EmptyParser()),
            new cminor::parsing::NonterminalParser("NamespaceContent", "NamespaceContent", 2))));
    AddRule(new NamespaceContentRule("NamespaceContent", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::NonterminalParser("UsingDirectives", "UsingDirectives", 2),
            new cminor::parsing::NonterminalParser("Definitions", "Definitions", 2))));
    AddRule(new UsingDirectivesRule("UsingDirectives", GetScope(),
        new cminor::parsing::KleeneStarParser(
            new cminor::parsing::NonterminalParser("UsingDirective", "UsingDirective", 2))));
    AddRule(new UsingDirectiveRule("UsingDirective", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("UsingAliasDirective", "UsingAliasDirective", 0)),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::NonterminalParser("UsingNamespaceDirective", "UsingNamespaceDirective", 0)))));
    AddRule(new UsingAliasDirectiveRule("UsingAliasDirective", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::KeywordParser("using"),
                            new cminor::parsing::ActionParser("A1",
                                new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0))),
                        new cminor::parsing::CharParser('=')),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("QualifiedId", "QualifiedId", 0))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new UsingNamespaceDirectiveRule("UsingNamespaceDirective", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser("using"),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("QualifiedId", "QualifiedId", 0))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new DefinitionsRule("Definitions", GetScope(),
        new cminor::parsing::KleeneStarParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("Definition", "Definition", 2)))));
    AddRule(new DefinitionRule("Definition", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::ActionParser("A0",
                                new cminor::parsing::NonterminalParser("NamespaceDefinition", "NamespaceDefinition", 2)),
                            new cminor::parsing::ActionParser("A1",
                                new cminor::parsing::NonterminalParser("FunctionDefinition", "FunctionDefinition", 1))),
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::NonterminalParser("ClassDefinition", "ClassDefinition", 1))),
                    new cminor::parsing::ActionParser("A3",
                        new cminor::parsing::NonterminalParser("InterfaceDefinition", "InterfaceDefinition", 1))),
                new cminor::parsing::ActionParser("A4",
                    new cminor::parsing::NonterminalParser("EnumTypeDefinition", "EnumTypeDefinition", 1))),
            new cminor::parsing::ActionParser("A5",
                new cminor::parsing::NonterminalParser("ConstantDefinition", "ConstantDefinition", 1)))));
    AddRule(new NamespaceDefinitionRule("NamespaceDefinition", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::ActionParser("A0",
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::KeywordParser("namespace"),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser("QualifiedId", "QualifiedId", 0)))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('{'))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser("NamespaceContent", "NamespaceContent", 2))),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::CharParser('}')))));
    AddRule(new FunctionDefinitionRule("FunctionDefinition", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::NonterminalParser("Function", "Function", 1))));
    AddRule(new ClassDefinitionRule("ClassDefinition", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::NonterminalParser("Class", "Class", 1))));
    AddRule(new InterfaceDefinitionRule("InterfaceDefinition", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::NonterminalParser("Interface", "Interface", 1))));
    AddRule(new EnumTypeDefinitionRule("EnumTypeDefinition", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::NonterminalParser("EnumType", "EnumType", 1))));
    AddRule(new ConstantDefinitionRule("ConstantDefinition", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::NonterminalParser("Constant", "Constant", 1))));
    SetSkipRuleName("spaces_and_comments");
}

} } // namespace cminor.parser
