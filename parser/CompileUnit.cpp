#include "CompileUnit.hpp"
#include <Cm.Parsing/Action.hpp>
#include <Cm.Parsing/Rule.hpp>
#include <Cm.Parsing/ParsingDomain.hpp>
#include <Cm.Parsing/Primitive.hpp>
#include <Cm.Parsing/Composite.hpp>
#include <Cm.Parsing/Nonterminal.hpp>
#include <Cm.Parsing/Exception.hpp>
#include <Cm.Parsing/StdLib.hpp>
#include <Cm.Parsing/XmlLog.hpp>
#include <cminor/parser/Identifier.hpp>
#include <cminor/parser/Function.hpp>

namespace cminor { namespace parser {

using namespace Cm::Parsing;

CompileUnitGrammar* CompileUnitGrammar::Create()
{
    return Create(new Cm::Parsing::ParsingDomain());
}

CompileUnitGrammar* CompileUnitGrammar::Create(Cm::Parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    CompileUnitGrammar* grammar(new CompileUnitGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

CompileUnitGrammar::CompileUnitGrammar(Cm::Parsing::ParsingDomain* parsingDomain_): Cm::Parsing::Grammar("CompileUnitGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

CompileUnitNode* CompileUnitGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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
    stack.push(std::unique_ptr<Cm::Parsing::Object>(new ValueObject<ParsingContext*>(ctx)));
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
    CompileUnitNode* result = *static_cast<Cm::Parsing::ValueObject<CompileUnitNode*>*>(value.get());
    stack.pop();
    return result;
}

class CompileUnitGrammar::CompileUnitRule : public Cm::Parsing::Rule
{
public:
    CompileUnitRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("CompileUnitNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<CompileUnitNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<CompileUnitRule>(this, &CompileUnitRule::A0Action));
        Cm::Parsing::NonterminalParser* namespaceContentNonterminalParser = GetNonterminal("NamespaceContent");
        namespaceContentNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<CompileUnitRule>(this, &CompileUnitRule::PreNamespaceContent));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new CompileUnitNode(span, fileName);
    }
    void PreNamespaceContent(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<CompileUnitNode*>(context.value)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<NamespaceNode*>(context.value->GlobalNs())));
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

class CompileUnitGrammar::NamespaceContentRule : public Cm::Parsing::Rule
{
public:
    NamespaceContentRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("CompileUnitNode*", "compileUnit"));
        AddInheritedAttribute(AttrOrVariable("NamespaceNode*", "ns"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ns_value = std::move(stack.top());
        context.ns = *static_cast<Cm::Parsing::ValueObject<NamespaceNode*>*>(ns_value.get());
        stack.pop();
        std::unique_ptr<Cm::Parsing::Object> compileUnit_value = std::move(stack.top());
        context.compileUnit = *static_cast<Cm::Parsing::ValueObject<CompileUnitNode*>*>(compileUnit_value.get());
        stack.pop();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::NonterminalParser* usingDirectivesNonterminalParser = GetNonterminal("UsingDirectives");
        usingDirectivesNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<NamespaceContentRule>(this, &NamespaceContentRule::PreUsingDirectives));
        Cm::Parsing::NonterminalParser* definitionsNonterminalParser = GetNonterminal("Definitions");
        definitionsNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<NamespaceContentRule>(this, &NamespaceContentRule::PreDefinitions));
    }
    void PreUsingDirectives(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<NamespaceNode*>(context.ns)));
    }
    void PreDefinitions(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<CompileUnitNode*>(context.compileUnit)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<NamespaceNode*>(context.ns)));
    }
private:
    struct Context
    {
        Context(): ctx(), compileUnit(), ns() {}
        ParsingContext* ctx;
        CompileUnitNode* compileUnit;
        NamespaceNode* ns;
    };
    std::stack<Context> contextStack;
    Context context;
};

class CompileUnitGrammar::UsingDirectivesRule : public Cm::Parsing::Rule
{
public:
    UsingDirectivesRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("NamespaceNode*", "ns"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ns_value = std::move(stack.top());
        context.ns = *static_cast<Cm::Parsing::ValueObject<NamespaceNode*>*>(ns_value.get());
        stack.pop();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::NonterminalParser* usingDirectiveNonterminalParser = GetNonterminal("UsingDirective");
        usingDirectiveNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<UsingDirectivesRule>(this, &UsingDirectivesRule::PreUsingDirective));
    }
    void PreUsingDirective(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<NamespaceNode*>(context.ns)));
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

class CompileUnitGrammar::UsingDirectiveRule : public Cm::Parsing::Rule
{
public:
    UsingDirectiveRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("NamespaceNode*", "ns"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ns_value = std::move(stack.top());
        context.ns = *static_cast<Cm::Parsing::ValueObject<NamespaceNode*>*>(ns_value.get());
        stack.pop();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<UsingDirectiveRule>(this, &UsingDirectiveRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<UsingDirectiveRule>(this, &UsingDirectiveRule::A1Action));
        Cm::Parsing::NonterminalParser* usingAliasDirectiveNonterminalParser = GetNonterminal("UsingAliasDirective");
        usingAliasDirectiveNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<UsingDirectiveRule>(this, &UsingDirectiveRule::PostUsingAliasDirective));
        Cm::Parsing::NonterminalParser* usingNamespaceDirectiveNonterminalParser = GetNonterminal("UsingNamespaceDirective");
        usingNamespaceDirectiveNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<UsingDirectiveRule>(this, &UsingDirectiveRule::PostUsingNamespaceDirective));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ns->AddMember(context.fromUsingAliasDirective);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ns->AddMember(context.fromUsingNamespaceDirective);
    }
    void PostUsingAliasDirective(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromUsingAliasDirective_value = std::move(stack.top());
            context.fromUsingAliasDirective = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromUsingAliasDirective_value.get());
            stack.pop();
        }
    }
    void PostUsingNamespaceDirective(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromUsingNamespaceDirective_value = std::move(stack.top());
            context.fromUsingNamespaceDirective = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromUsingNamespaceDirective_value.get());
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

class CompileUnitGrammar::UsingAliasDirectiveRule : public Cm::Parsing::Rule
{
public:
    UsingAliasDirectiveRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<IdentifierNode>", "id"));
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<UsingAliasDirectiveRule>(this, &UsingAliasDirectiveRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<UsingAliasDirectiveRule>(this, &UsingAliasDirectiveRule::A1Action));
        Cm::Parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<UsingAliasDirectiveRule>(this, &UsingAliasDirectiveRule::PostIdentifier));
        Cm::Parsing::NonterminalParser* qualifiedIdNonterminalParser = GetNonterminal("QualifiedId");
        qualifiedIdNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<UsingAliasDirectiveRule>(this, &UsingAliasDirectiveRule::PostQualifiedId));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new AliasNode(span, context.id.release(), context.fromQualifiedId);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.id.reset(context.fromIdentifier);
    }
    void PostIdentifier(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromIdentifier_value = std::move(stack.top());
            context.fromIdentifier = *static_cast<Cm::Parsing::ValueObject<IdentifierNode*>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
    void PostQualifiedId(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromQualifiedId_value = std::move(stack.top());
            context.fromQualifiedId = *static_cast<Cm::Parsing::ValueObject<IdentifierNode*>*>(fromQualifiedId_value.get());
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

class CompileUnitGrammar::UsingNamespaceDirectiveRule : public Cm::Parsing::Rule
{
public:
    UsingNamespaceDirectiveRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Node*");
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<UsingNamespaceDirectiveRule>(this, &UsingNamespaceDirectiveRule::A0Action));
        Cm::Parsing::NonterminalParser* qualifiedIdNonterminalParser = GetNonterminal("QualifiedId");
        qualifiedIdNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<UsingNamespaceDirectiveRule>(this, &UsingNamespaceDirectiveRule::PostQualifiedId));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new NamespaceImportNode(span, context.fromQualifiedId);
    }
    void PostQualifiedId(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromQualifiedId_value = std::move(stack.top());
            context.fromQualifiedId = *static_cast<Cm::Parsing::ValueObject<IdentifierNode*>*>(fromQualifiedId_value.get());
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

class CompileUnitGrammar::DefinitionsRule : public Cm::Parsing::Rule
{
public:
    DefinitionsRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("CompileUnitNode*", "compileUnit"));
        AddInheritedAttribute(AttrOrVariable("NamespaceNode*", "ns"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ns_value = std::move(stack.top());
        context.ns = *static_cast<Cm::Parsing::ValueObject<NamespaceNode*>*>(ns_value.get());
        stack.pop();
        std::unique_ptr<Cm::Parsing::Object> compileUnit_value = std::move(stack.top());
        context.compileUnit = *static_cast<Cm::Parsing::ValueObject<CompileUnitNode*>*>(compileUnit_value.get());
        stack.pop();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<DefinitionsRule>(this, &DefinitionsRule::A0Action));
        Cm::Parsing::NonterminalParser* definitionNonterminalParser = GetNonterminal("Definition");
        definitionNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<DefinitionsRule>(this, &DefinitionsRule::PreDefinition));
        definitionNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<DefinitionsRule>(this, &DefinitionsRule::PostDefinition));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ns->AddMember(context.fromDefinition);
    }
    void PreDefinition(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<CompileUnitNode*>(context.compileUnit)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<NamespaceNode*>(context.ns)));
    }
    void PostDefinition(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromDefinition_value = std::move(stack.top());
            context.fromDefinition = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromDefinition_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), compileUnit(), ns(), fromDefinition() {}
        ParsingContext* ctx;
        CompileUnitNode* compileUnit;
        NamespaceNode* ns;
        Node* fromDefinition;
    };
    std::stack<Context> contextStack;
    Context context;
};

class CompileUnitGrammar::DefinitionRule : public Cm::Parsing::Rule
{
public:
    DefinitionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("CompileUnitNode*", "compileUnit"));
        AddInheritedAttribute(AttrOrVariable("NamespaceNode*", "ns"));
        SetValueTypeName("Node*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ns_value = std::move(stack.top());
        context.ns = *static_cast<Cm::Parsing::ValueObject<NamespaceNode*>*>(ns_value.get());
        stack.pop();
        std::unique_ptr<Cm::Parsing::Object> compileUnit_value = std::move(stack.top());
        context.compileUnit = *static_cast<Cm::Parsing::ValueObject<CompileUnitNode*>*>(compileUnit_value.get());
        stack.pop();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<DefinitionRule>(this, &DefinitionRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<DefinitionRule>(this, &DefinitionRule::A1Action));
        Cm::Parsing::NonterminalParser* namespaceDefinitionNonterminalParser = GetNonterminal("NamespaceDefinition");
        namespaceDefinitionNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<DefinitionRule>(this, &DefinitionRule::PreNamespaceDefinition));
        namespaceDefinitionNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<DefinitionRule>(this, &DefinitionRule::PostNamespaceDefinition));
        Cm::Parsing::NonterminalParser* functionDefinitionNonterminalParser = GetNonterminal("FunctionDefinition");
        functionDefinitionNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<DefinitionRule>(this, &DefinitionRule::PreFunctionDefinition));
        functionDefinitionNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<DefinitionRule>(this, &DefinitionRule::PostFunctionDefinition));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromNamespaceDefinition;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromFunctionDefinition;
    }
    void PreNamespaceDefinition(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<CompileUnitNode*>(context.compileUnit)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<NamespaceNode*>(context.ns)));
    }
    void PostNamespaceDefinition(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromNamespaceDefinition_value = std::move(stack.top());
            context.fromNamespaceDefinition = *static_cast<Cm::Parsing::ValueObject<NamespaceNode*>*>(fromNamespaceDefinition_value.get());
            stack.pop();
        }
    }
    void PreFunctionDefinition(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<CompileUnitNode*>(context.compileUnit)));
    }
    void PostFunctionDefinition(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromFunctionDefinition_value = std::move(stack.top());
            context.fromFunctionDefinition = *static_cast<Cm::Parsing::ValueObject<FunctionNode*>*>(fromFunctionDefinition_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), compileUnit(), ns(), value(), fromNamespaceDefinition(), fromFunctionDefinition() {}
        ParsingContext* ctx;
        CompileUnitNode* compileUnit;
        NamespaceNode* ns;
        Node* value;
        NamespaceNode* fromNamespaceDefinition;
        FunctionNode* fromFunctionDefinition;
    };
    std::stack<Context> contextStack;
    Context context;
};

class CompileUnitGrammar::NamespaceDefinitionRule : public Cm::Parsing::Rule
{
public:
    NamespaceDefinitionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("CompileUnitNode*", "compileUnit"));
        AddInheritedAttribute(AttrOrVariable("NamespaceNode*", "ns"));
        SetValueTypeName("NamespaceNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> ns_value = std::move(stack.top());
        context.ns = *static_cast<Cm::Parsing::ValueObject<NamespaceNode*>*>(ns_value.get());
        stack.pop();
        std::unique_ptr<Cm::Parsing::Object> compileUnit_value = std::move(stack.top());
        context.compileUnit = *static_cast<Cm::Parsing::ValueObject<CompileUnitNode*>*>(compileUnit_value.get());
        stack.pop();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<NamespaceNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<NamespaceDefinitionRule>(this, &NamespaceDefinitionRule::A0Action));
        Cm::Parsing::NonterminalParser* qualifiedIdNonterminalParser = GetNonterminal("QualifiedId");
        qualifiedIdNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<NamespaceDefinitionRule>(this, &NamespaceDefinitionRule::PostQualifiedId));
        Cm::Parsing::NonterminalParser* namespaceContentNonterminalParser = GetNonterminal("NamespaceContent");
        namespaceContentNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<NamespaceDefinitionRule>(this, &NamespaceDefinitionRule::PreNamespaceContent));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new NamespaceNode(span, context.fromQualifiedId);
    }
    void PostQualifiedId(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromQualifiedId_value = std::move(stack.top());
            context.fromQualifiedId = *static_cast<Cm::Parsing::ValueObject<IdentifierNode*>*>(fromQualifiedId_value.get());
            stack.pop();
        }
    }
    void PreNamespaceContent(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<CompileUnitNode*>(context.compileUnit)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<NamespaceNode*>(context.value)));
    }
private:
    struct Context
    {
        Context(): ctx(), compileUnit(), ns(), value(), fromQualifiedId() {}
        ParsingContext* ctx;
        CompileUnitNode* compileUnit;
        NamespaceNode* ns;
        NamespaceNode* value;
        IdentifierNode* fromQualifiedId;
    };
    std::stack<Context> contextStack;
    Context context;
};

class CompileUnitGrammar::FunctionDefinitionRule : public Cm::Parsing::Rule
{
public:
    FunctionDefinitionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("CompileUnitNode*", "compileUnit"));
        SetValueTypeName("FunctionNode*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> compileUnit_value = std::move(stack.top());
        context.compileUnit = *static_cast<Cm::Parsing::ValueObject<CompileUnitNode*>*>(compileUnit_value.get());
        stack.pop();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<FunctionNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<FunctionDefinitionRule>(this, &FunctionDefinitionRule::A0Action));
        Cm::Parsing::NonterminalParser* functionNonterminalParser = GetNonterminal("Function");
        functionNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<FunctionDefinitionRule>(this, &FunctionDefinitionRule::PreFunction));
        functionNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<FunctionDefinitionRule>(this, &FunctionDefinitionRule::PostFunction));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromFunction;
        context.value->SetCompileUnit(context.compileUnit);
    }
    void PreFunction(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostFunction(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromFunction_value = std::move(stack.top());
            context.fromFunction = *static_cast<Cm::Parsing::ValueObject<FunctionNode*>*>(fromFunction_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), compileUnit(), value(), fromFunction() {}
        ParsingContext* ctx;
        CompileUnitNode* compileUnit;
        FunctionNode* value;
        FunctionNode* fromFunction;
    };
    std::stack<Context> contextStack;
    Context context;
};

void CompileUnitGrammar::GetReferencedGrammars()
{
    Cm::Parsing::ParsingDomain* pd = GetParsingDomain();
    Cm::Parsing::Grammar* grammar0 = pd->GetGrammar("Cm.Parsing.stdlib");
    if (!grammar0)
    {
        grammar0 = Cm::Parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar0);
    Cm::Parsing::Grammar* grammar1 = pd->GetGrammar("cminor.parser.IdentifierGrammar");
    if (!grammar1)
    {
        grammar1 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    Cm::Parsing::Grammar* grammar2 = pd->GetGrammar("cminor.parser.FunctionGrammar");
    if (!grammar2)
    {
        grammar2 = cminor::parser::FunctionGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
}

void CompileUnitGrammar::CreateRules()
{
    AddRuleLink(new Cm::Parsing::RuleLink("Identifier", this, "IdentifierGrammar.Identifier"));
    AddRuleLink(new Cm::Parsing::RuleLink("spaces_and_comments", this, "Cm.Parsing.stdlib.spaces_and_comments"));
    AddRuleLink(new Cm::Parsing::RuleLink("QualifiedId", this, "IdentifierGrammar.QualifiedId"));
    AddRuleLink(new Cm::Parsing::RuleLink("Function", this, "FunctionGrammar.Function"));
    AddRule(new CompileUnitRule("CompileUnit", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::ActionParser("A0",
                new Cm::Parsing::EmptyParser()),
            new Cm::Parsing::NonterminalParser("NamespaceContent", "NamespaceContent", 3))));
    AddRule(new NamespaceContentRule("NamespaceContent", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::NonterminalParser("UsingDirectives", "UsingDirectives", 2),
            new Cm::Parsing::NonterminalParser("Definitions", "Definitions", 3))));
    AddRule(new UsingDirectivesRule("UsingDirectives", GetScope(),
        new Cm::Parsing::KleeneStarParser(
            new Cm::Parsing::NonterminalParser("UsingDirective", "UsingDirective", 2))));
    AddRule(new UsingDirectiveRule("UsingDirective", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::ActionParser("A0",
                new Cm::Parsing::NonterminalParser("UsingAliasDirective", "UsingAliasDirective", 0)),
            new Cm::Parsing::ActionParser("A1",
                new Cm::Parsing::NonterminalParser("UsingNamespaceDirective", "UsingNamespaceDirective", 0)))));
    AddRule(new UsingAliasDirectiveRule("UsingAliasDirective", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::KeywordParser("using"),
                            new Cm::Parsing::ActionParser("A1",
                                new Cm::Parsing::NonterminalParser("Identifier", "Identifier", 0))),
                        new Cm::Parsing::CharParser('=')),
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::NonterminalParser("QualifiedId", "QualifiedId", 0))),
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::CharParser(';'))))));
    AddRule(new UsingNamespaceDirectiveRule("UsingNamespaceDirective", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::KeywordParser("using"),
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::NonterminalParser("QualifiedId", "QualifiedId", 0))),
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::CharParser(';'))))));
    AddRule(new DefinitionsRule("Definitions", GetScope(),
        new Cm::Parsing::KleeneStarParser(
            new Cm::Parsing::ActionParser("A0",
                new Cm::Parsing::NonterminalParser("Definition", "Definition", 3)))));
    AddRule(new DefinitionRule("Definition", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::ActionParser("A0",
                new Cm::Parsing::NonterminalParser("NamespaceDefinition", "NamespaceDefinition", 3)),
            new Cm::Parsing::ActionParser("A1",
                new Cm::Parsing::NonterminalParser("FunctionDefinition", "FunctionDefinition", 2)))));
    AddRule(new NamespaceDefinitionRule("NamespaceDefinition", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::ActionParser("A0",
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::KeywordParser("namespace"),
                            new Cm::Parsing::ExpectationParser(
                                new Cm::Parsing::NonterminalParser("QualifiedId", "QualifiedId", 0)))),
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::CharParser('{'))),
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::NonterminalParser("NamespaceContent", "NamespaceContent", 3))),
            new Cm::Parsing::ExpectationParser(
                new Cm::Parsing::CharParser('}')))));
    AddRule(new FunctionDefinitionRule("FunctionDefinition", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::NonterminalParser("Function", "Function", 1))));
    SetSkipRuleName("spaces_and_comments");
}

} } // namespace cminor.parser
