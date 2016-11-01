#include "Class.hpp"
#include <Cm.Parsing/Action.hpp>
#include <Cm.Parsing/Rule.hpp>
#include <Cm.Parsing/ParsingDomain.hpp>
#include <Cm.Parsing/Primitive.hpp>
#include <Cm.Parsing/Composite.hpp>
#include <Cm.Parsing/Nonterminal.hpp>
#include <Cm.Parsing/Exception.hpp>
#include <Cm.Parsing/StdLib.hpp>
#include <Cm.Parsing/XmlLog.hpp>
#include <cminor/parser/Specifier.hpp>
#include <cminor/parser/Identifier.hpp>
#include <cminor/parser/TypeExpr.hpp>
#include <cminor/parser/Expression.hpp>
#include <cminor/parser/Statement.hpp>
#include <cminor/parser/Parameter.hpp>
#include <cminor/parser/Function.hpp>

namespace cminor { namespace parser {

using namespace Cm::Parsing;

ClassGrammar* ClassGrammar::Create()
{
    return Create(new Cm::Parsing::ParsingDomain());
}

ClassGrammar* ClassGrammar::Create(Cm::Parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    ClassGrammar* grammar(new ClassGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

ClassGrammar::ClassGrammar(Cm::Parsing::ParsingDomain* parsingDomain_): Cm::Parsing::Grammar("ClassGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

ClassNode* ClassGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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
    ClassNode* result = *static_cast<Cm::Parsing::ValueObject<ClassNode*>*>(value.get());
    stack.pop();
    return result;
}

class ClassGrammar::ClassRule : public Cm::Parsing::Rule
{
public:
    ClassRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("ClassNode*");
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ClassNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ClassRule>(this, &ClassRule::A0Action));
        Cm::Parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal("Specifiers");
        specifiersNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ClassRule>(this, &ClassRule::PostSpecifiers));
        Cm::Parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ClassRule>(this, &ClassRule::PostIdentifier));
        Cm::Parsing::NonterminalParser* inheritanceAndInterfacesNonterminalParser = GetNonterminal("InheritanceAndInterfaces");
        inheritanceAndInterfacesNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ClassRule>(this, &ClassRule::PreInheritanceAndInterfaces));
        Cm::Parsing::NonterminalParser* classContentNonterminalParser = GetNonterminal("ClassContent");
        classContentNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ClassRule>(this, &ClassRule::PreClassContent));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ClassNode(span, context.fromSpecifiers, context.fromIdentifier);
    }
    void PostSpecifiers(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context.fromSpecifiers = *static_cast<Cm::Parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
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
    void PreInheritanceAndInterfaces(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ClassNode*>(context.value)));
    }
    void PreClassContent(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ClassNode*>(context.value)));
    }
private:
    struct Context
    {
        Context(): ctx(), value(), fromSpecifiers(), fromIdentifier() {}
        ParsingContext* ctx;
        ClassNode* value;
        Specifiers fromSpecifiers;
        IdentifierNode* fromIdentifier;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ClassGrammar::InheritanceAndInterfacesRule : public Cm::Parsing::Rule
{
public:
    InheritanceAndInterfacesRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("ClassNode*", "classNode"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> classNode_value = std::move(stack.top());
        context.classNode = *static_cast<Cm::Parsing::ValueObject<ClassNode*>*>(classNode_value.get());
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<InheritanceAndInterfacesRule>(this, &InheritanceAndInterfacesRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<InheritanceAndInterfacesRule>(this, &InheritanceAndInterfacesRule::A1Action));
        a1ActionParser->SetFailureAction(new Cm::Parsing::MemberFailureAction<InheritanceAndInterfacesRule>(this, &InheritanceAndInterfacesRule::A1ActionFail));
        Cm::Parsing::NonterminalParser* baseClassOrInterfaceNonterminalParser = GetNonterminal("BaseClassOrInterface");
        baseClassOrInterfaceNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<InheritanceAndInterfacesRule>(this, &InheritanceAndInterfacesRule::PreBaseClassOrInterface));
        baseClassOrInterfaceNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<InheritanceAndInterfacesRule>(this, &InheritanceAndInterfacesRule::PostBaseClassOrInterface));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ctx->BeginParsingTypeExpr();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.classNode->AddBaseClassOrInterface(context.fromBaseClassOrInterface);
        context.ctx->EndParsingTypeExpr();
    }
    void A1ActionFail()
    {
        context.ctx->EndParsingTypeExpr();
    }
    void PreBaseClassOrInterface(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostBaseClassOrInterface(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromBaseClassOrInterface_value = std::move(stack.top());
            context.fromBaseClassOrInterface = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromBaseClassOrInterface_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), classNode(), fromBaseClassOrInterface() {}
        ParsingContext* ctx;
        ClassNode* classNode;
        Node* fromBaseClassOrInterface;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ClassGrammar::BaseClassOrInterfaceRule : public Cm::Parsing::Rule
{
public:
    BaseClassOrInterfaceRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<BaseClassOrInterfaceRule>(this, &BaseClassOrInterfaceRule::A0Action));
        Cm::Parsing::NonterminalParser* qualifiedIdNonterminalParser = GetNonterminal("QualifiedId");
        qualifiedIdNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<BaseClassOrInterfaceRule>(this, &BaseClassOrInterfaceRule::PostQualifiedId));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromQualifiedId;
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
        Context(): ctx(), value(), fromQualifiedId() {}
        ParsingContext* ctx;
        Node* value;
        IdentifierNode* fromQualifiedId;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ClassGrammar::ClassContentRule : public Cm::Parsing::Rule
{
public:
    ClassContentRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("ClassNode*", "classNode"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> classNode_value = std::move(stack.top());
        context.classNode = *static_cast<Cm::Parsing::ValueObject<ClassNode*>*>(classNode_value.get());
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ClassContentRule>(this, &ClassContentRule::A0Action));
        Cm::Parsing::NonterminalParser* classMemberNonterminalParser = GetNonterminal("ClassMember");
        classMemberNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ClassContentRule>(this, &ClassContentRule::PreClassMember));
        classMemberNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ClassContentRule>(this, &ClassContentRule::PostClassMember));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.classNode->AddMember(context.fromClassMember);
    }
    void PreClassMember(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ClassNode*>(context.classNode)));
    }
    void PostClassMember(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromClassMember_value = std::move(stack.top());
            context.fromClassMember = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromClassMember_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), classNode(), fromClassMember() {}
        ParsingContext* ctx;
        ClassNode* classNode;
        Node* fromClassMember;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ClassGrammar::ClassMemberRule : public Cm::Parsing::Rule
{
public:
    ClassMemberRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("ClassNode*", "classNode"));
        SetValueTypeName("Node*");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> classNode_value = std::move(stack.top());
        context.classNode = *static_cast<Cm::Parsing::ValueObject<ClassNode*>*>(classNode_value.get());
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A3Action));
        Cm::Parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A4Action));
        Cm::Parsing::NonterminalParser* staticConstructorNonterminalParser = GetNonterminal("StaticConstructor");
        staticConstructorNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreStaticConstructor));
        staticConstructorNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostStaticConstructor));
        Cm::Parsing::NonterminalParser* constructorNonterminalParser = GetNonterminal("Constructor");
        constructorNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreConstructor));
        constructorNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostConstructor));
        Cm::Parsing::NonterminalParser* memberFunctionNonterminalParser = GetNonterminal("MemberFunction");
        memberFunctionNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreMemberFunction));
        memberFunctionNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostMemberFunction));
        Cm::Parsing::NonterminalParser* memberVariableNonterminalParser = GetNonterminal("MemberVariable");
        memberVariableNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreMemberVariable));
        memberVariableNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostMemberVariable));
        Cm::Parsing::NonterminalParser* propertyNonterminalParser = GetNonterminal("Property");
        propertyNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreProperty));
        propertyNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostProperty));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromStaticConstructor;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromConstructor;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromMemberFunction;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromMemberVariable;
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromProperty;
    }
    void PreStaticConstructor(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ClassNode*>(context.classNode)));
    }
    void PostStaticConstructor(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromStaticConstructor_value = std::move(stack.top());
            context.fromStaticConstructor = *static_cast<Cm::Parsing::ValueObject<StaticConstructorNode*>*>(fromStaticConstructor_value.get());
            stack.pop();
        }
    }
    void PreConstructor(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ClassNode*>(context.classNode)));
    }
    void PostConstructor(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromConstructor_value = std::move(stack.top());
            context.fromConstructor = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromConstructor_value.get());
            stack.pop();
        }
    }
    void PreMemberFunction(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ClassNode*>(context.classNode)));
    }
    void PostMemberFunction(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromMemberFunction_value = std::move(stack.top());
            context.fromMemberFunction = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromMemberFunction_value.get());
            stack.pop();
        }
    }
    void PreMemberVariable(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostMemberVariable(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromMemberVariable_value = std::move(stack.top());
            context.fromMemberVariable = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromMemberVariable_value.get());
            stack.pop();
        }
    }
    void PreProperty(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostProperty(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromProperty_value = std::move(stack.top());
            context.fromProperty = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromProperty_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), classNode(), value(), fromStaticConstructor(), fromConstructor(), fromMemberFunction(), fromMemberVariable(), fromProperty() {}
        ParsingContext* ctx;
        ClassNode* classNode;
        Node* value;
        StaticConstructorNode* fromStaticConstructor;
        Node* fromConstructor;
        Node* fromMemberFunction;
        Node* fromMemberVariable;
        Node* fromProperty;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ClassGrammar::StaticConstructorRule : public Cm::Parsing::Rule
{
public:
    StaticConstructorRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("ClassNode*", "classNode"));
        SetValueTypeName("StaticConstructorNode*");
        AddLocalVariable(AttrOrVariable("AttributeMap", "attributeMap"));
        AddLocalVariable(AttrOrVariable("std::unique_ptr<IdentifierNode>", "id"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> classNode_value = std::move(stack.top());
        context.classNode = *static_cast<Cm::Parsing::ValueObject<ClassNode*>*>(classNode_value.get());
        stack.pop();
        std::unique_ptr<Cm::Parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<Cm::Parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<StaticConstructorNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<StaticConstructorRule>(this, &StaticConstructorRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<StaticConstructorRule>(this, &StaticConstructorRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<StaticConstructorRule>(this, &StaticConstructorRule::A2Action));
        Cm::Parsing::NonterminalParser* attributesNonterminalParser = GetNonterminal("Attributes");
        attributesNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<StaticConstructorRule>(this, &StaticConstructorRule::PreAttributes));
        Cm::Parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal("Specifiers");
        specifiersNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<StaticConstructorRule>(this, &StaticConstructorRule::PostSpecifiers));
        Cm::Parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<StaticConstructorRule>(this, &StaticConstructorRule::PostIdentifier));
        Cm::Parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal("CompoundStatement");
        compoundStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<StaticConstructorRule>(this, &StaticConstructorRule::PreCompoundStatement));
        compoundStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<StaticConstructorRule>(this, &StaticConstructorRule::PostCompoundStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (context.id->Str() != context.classNode->Id()->Str()) pass = false;
        else if (!HasStaticSpecifier(context.fromSpecifiers)) pass = false;
        else
        {
            context.value = new StaticConstructorNode(span, context.fromSpecifiers);
            context.value->SetAttributes(context.attributeMap);
        }
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.id.reset(context.fromIdentifier);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value->SetBody(context.fromCompoundStatement);
    }
    void PreAttributes(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<AttributeMap*>(&context.attributeMap)));
    }
    void PostSpecifiers(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context.fromSpecifiers = *static_cast<Cm::Parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
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
    void PreCompoundStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostCompoundStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context.fromCompoundStatement = *static_cast<Cm::Parsing::ValueObject<CompoundStatementNode*>*>(fromCompoundStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), classNode(), value(), attributeMap(), id(), fromSpecifiers(), fromIdentifier(), fromCompoundStatement() {}
        ParsingContext* ctx;
        ClassNode* classNode;
        StaticConstructorNode* value;
        AttributeMap attributeMap;
        std::unique_ptr<IdentifierNode> id;
        Specifiers fromSpecifiers;
        IdentifierNode* fromIdentifier;
        CompoundStatementNode* fromCompoundStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ClassGrammar::ConstructorRule : public Cm::Parsing::Rule
{
public:
    ConstructorRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("ClassNode*", "classNode"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("AttributeMap", "attributeMap"));
        AddLocalVariable(AttrOrVariable("std::unique_ptr<IdentifierNode>", "id"));
        AddLocalVariable(AttrOrVariable("std::unique_ptr<ConstructorNode>", "ctor"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> classNode_value = std::move(stack.top());
        context.classNode = *static_cast<Cm::Parsing::ValueObject<ClassNode*>*>(classNode_value.get());
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ConstructorRule>(this, &ConstructorRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ConstructorRule>(this, &ConstructorRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ConstructorRule>(this, &ConstructorRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<ConstructorRule>(this, &ConstructorRule::A3Action));
        Cm::Parsing::NonterminalParser* attributesNonterminalParser = GetNonterminal("Attributes");
        attributesNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ConstructorRule>(this, &ConstructorRule::PreAttributes));
        Cm::Parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal("Specifiers");
        specifiersNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ConstructorRule>(this, &ConstructorRule::PostSpecifiers));
        Cm::Parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ConstructorRule>(this, &ConstructorRule::PostIdentifier));
        Cm::Parsing::NonterminalParser* parameterListNonterminalParser = GetNonterminal("ParameterList");
        parameterListNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ConstructorRule>(this, &ConstructorRule::PreParameterList));
        Cm::Parsing::NonterminalParser* initializerNonterminalParser = GetNonterminal("Initializer");
        initializerNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ConstructorRule>(this, &ConstructorRule::PreInitializer));
        initializerNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ConstructorRule>(this, &ConstructorRule::PostInitializer));
        Cm::Parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal("CompoundStatement");
        compoundStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<ConstructorRule>(this, &ConstructorRule::PreCompoundStatement));
        compoundStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<ConstructorRule>(this, &ConstructorRule::PostCompoundStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        if (context.id->Str() != context.classNode->Id()->Str()) pass = false;
        else
        {
            context.ctor.reset(new ConstructorNode(span, context.fromSpecifiers));
            context.ctor->SetAttributes(context.attributeMap);
        }
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.id.reset(context.fromIdentifier);
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ctor->SetInitializer(context.fromInitializer);
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ctor->SetBody(context.fromCompoundStatement);
        context.value = context.ctor.release();
    }
    void PreAttributes(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<AttributeMap*>(&context.attributeMap)));
    }
    void PostSpecifiers(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context.fromSpecifiers = *static_cast<Cm::Parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
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
    void PreParameterList(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Node*>(context.ctor.get())));
    }
    void PreInitializer(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostInitializer(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromInitializer_value = std::move(stack.top());
            context.fromInitializer = *static_cast<Cm::Parsing::ValueObject<InitializerNode*>*>(fromInitializer_value.get());
            stack.pop();
        }
    }
    void PreCompoundStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostCompoundStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context.fromCompoundStatement = *static_cast<Cm::Parsing::ValueObject<CompoundStatementNode*>*>(fromCompoundStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), classNode(), value(), attributeMap(), id(), ctor(), fromSpecifiers(), fromIdentifier(), fromInitializer(), fromCompoundStatement() {}
        ParsingContext* ctx;
        ClassNode* classNode;
        Node* value;
        AttributeMap attributeMap;
        std::unique_ptr<IdentifierNode> id;
        std::unique_ptr<ConstructorNode> ctor;
        Specifiers fromSpecifiers;
        IdentifierNode* fromIdentifier;
        InitializerNode* fromInitializer;
        CompoundStatementNode* fromCompoundStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ClassGrammar::InitializerRule : public Cm::Parsing::Rule
{
public:
    InitializerRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("InitializerNode*");
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<InitializerNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<InitializerRule>(this, &InitializerRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<InitializerRule>(this, &InitializerRule::A1Action));
        Cm::Parsing::NonterminalParser* thisArgsNonterminalParser = GetNonterminal("thisArgs");
        thisArgsNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<InitializerRule>(this, &InitializerRule::PrethisArgs));
        Cm::Parsing::NonterminalParser* baseArgsNonterminalParser = GetNonterminal("baseArgs");
        baseArgsNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<InitializerRule>(this, &InitializerRule::PrebaseArgs));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ThisInitializerNode(span);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new BaseInitializerNode(span);
    }
    void PrethisArgs(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Node*>(context.value)));
    }
    void PrebaseArgs(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Node*>(context.value)));
    }
private:
    struct Context
    {
        Context(): ctx(), value() {}
        ParsingContext* ctx;
        InitializerNode* value;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ClassGrammar::MemberFunctionRule : public Cm::Parsing::Rule
{
public:
    MemberFunctionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("ClassNode*", "classNode"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("AttributeMap", "attributeMap"));
        AddLocalVariable(AttrOrVariable("std::unique_ptr<MemberFunctionNode>", "memFun"));
        AddLocalVariable(AttrOrVariable("std::unique_ptr<IdentifierNode>", "qid"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> classNode_value = std::move(stack.top());
        context.classNode = *static_cast<Cm::Parsing::ValueObject<ClassNode*>*>(classNode_value.get());
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<MemberFunctionRule>(this, &MemberFunctionRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<MemberFunctionRule>(this, &MemberFunctionRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<MemberFunctionRule>(this, &MemberFunctionRule::A2Action));
        Cm::Parsing::NonterminalParser* attributesNonterminalParser = GetNonterminal("Attributes");
        attributesNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<MemberFunctionRule>(this, &MemberFunctionRule::PreAttributes));
        Cm::Parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal("Specifiers");
        specifiersNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<MemberFunctionRule>(this, &MemberFunctionRule::PostSpecifiers));
        Cm::Parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<MemberFunctionRule>(this, &MemberFunctionRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<MemberFunctionRule>(this, &MemberFunctionRule::PostTypeExpr));
        Cm::Parsing::NonterminalParser* functionGroupIdNonterminalParser = GetNonterminal("FunctionGroupId");
        functionGroupIdNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<MemberFunctionRule>(this, &MemberFunctionRule::PreFunctionGroupId));
        functionGroupIdNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<MemberFunctionRule>(this, &MemberFunctionRule::PostFunctionGroupId));
        Cm::Parsing::NonterminalParser* parameterListNonterminalParser = GetNonterminal("ParameterList");
        parameterListNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<MemberFunctionRule>(this, &MemberFunctionRule::PreParameterList));
        Cm::Parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal("CompoundStatement");
        compoundStatementNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<MemberFunctionRule>(this, &MemberFunctionRule::PreCompoundStatement));
        compoundStatementNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<MemberFunctionRule>(this, &MemberFunctionRule::PostCompoundStatement));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.memFun.reset(new MemberFunctionNode(span, context.fromSpecifiers, context.fromTypeExpr, context.fromFunctionGroupId));
        context.memFun->SetAttributes(context.attributeMap);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.memFun.release();
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.memFun->SetBody(context.fromCompoundStatement);
    }
    void PreAttributes(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<AttributeMap*>(&context.attributeMap)));
    }
    void PostSpecifiers(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context.fromSpecifiers = *static_cast<Cm::Parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
    }
    void PreTypeExpr(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostTypeExpr(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context.fromTypeExpr = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
            stack.pop();
        }
    }
    void PreFunctionGroupId(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostFunctionGroupId(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromFunctionGroupId_value = std::move(stack.top());
            context.fromFunctionGroupId = *static_cast<Cm::Parsing::ValueObject<FunctionGroupIdNode*>*>(fromFunctionGroupId_value.get());
            stack.pop();
        }
    }
    void PreParameterList(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Node*>(context.memFun.get())));
    }
    void PreCompoundStatement(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostCompoundStatement(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context.fromCompoundStatement = *static_cast<Cm::Parsing::ValueObject<CompoundStatementNode*>*>(fromCompoundStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), classNode(), value(), attributeMap(), memFun(), qid(), fromSpecifiers(), fromTypeExpr(), fromFunctionGroupId(), fromCompoundStatement() {}
        ParsingContext* ctx;
        ClassNode* classNode;
        Node* value;
        AttributeMap attributeMap;
        std::unique_ptr<MemberFunctionNode> memFun;
        std::unique_ptr<IdentifierNode> qid;
        Specifiers fromSpecifiers;
        Node* fromTypeExpr;
        FunctionGroupIdNode* fromFunctionGroupId;
        CompoundStatementNode* fromCompoundStatement;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ClassGrammar::MemberVariableRule : public Cm::Parsing::Rule
{
public:
    MemberVariableRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<MemberVariableRule>(this, &MemberVariableRule::A0Action));
        Cm::Parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal("Specifiers");
        specifiersNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<MemberVariableRule>(this, &MemberVariableRule::PostSpecifiers));
        Cm::Parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<MemberVariableRule>(this, &MemberVariableRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<MemberVariableRule>(this, &MemberVariableRule::PostTypeExpr));
        Cm::Parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<MemberVariableRule>(this, &MemberVariableRule::PostIdentifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new MemberVariableNode(span, context.fromSpecifiers, context.fromTypeExpr, context.fromIdentifier);
    }
    void PostSpecifiers(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context.fromSpecifiers = *static_cast<Cm::Parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
    }
    void PreTypeExpr(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostTypeExpr(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context.fromTypeExpr = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
            stack.pop();
        }
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
private:
    struct Context
    {
        Context(): ctx(), value(), fromSpecifiers(), fromTypeExpr(), fromIdentifier() {}
        ParsingContext* ctx;
        Node* value;
        Specifiers fromSpecifiers;
        Node* fromTypeExpr;
        IdentifierNode* fromIdentifier;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ClassGrammar::PropertyRule : public Cm::Parsing::Rule
{
public:
    PropertyRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "property"));
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<PropertyRule>(this, &PropertyRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<PropertyRule>(this, &PropertyRule::A1Action));
        Cm::Parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal("Specifiers");
        specifiersNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<PropertyRule>(this, &PropertyRule::PostSpecifiers));
        Cm::Parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<PropertyRule>(this, &PropertyRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<PropertyRule>(this, &PropertyRule::PostTypeExpr));
        Cm::Parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<PropertyRule>(this, &PropertyRule::PostIdentifier));
        Cm::Parsing::NonterminalParser* getterOrSetterNonterminalParser = GetNonterminal("GetterOrSetter");
        getterOrSetterNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<PropertyRule>(this, &PropertyRule::PreGetterOrSetter));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.property.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.property.reset(new PropertyNode(span, context.fromSpecifiers, context.fromTypeExpr, context.fromIdentifier));
    }
    void PostSpecifiers(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context.fromSpecifiers = *static_cast<Cm::Parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
    }
    void PreTypeExpr(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostTypeExpr(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context.fromTypeExpr = *static_cast<Cm::Parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
            stack.pop();
        }
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
    void PreGetterOrSetter(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Node*>(context.property.get())));
    }
private:
    struct Context
    {
        Context(): ctx(), value(), property(), fromSpecifiers(), fromTypeExpr(), fromIdentifier() {}
        ParsingContext* ctx;
        Node* value;
        std::unique_ptr<Node> property;
        Specifiers fromSpecifiers;
        Node* fromTypeExpr;
        IdentifierNode* fromIdentifier;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ClassGrammar::GetterOrSetterRule : public Cm::Parsing::Rule
{
public:
    GetterOrSetterRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("Node*", "subject"));
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<Cm::Parsing::Object> subject_value = std::move(stack.top());
        context.subject = *static_cast<Cm::Parsing::ValueObject<Node*>*>(subject_value.get());
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
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<GetterOrSetterRule>(this, &GetterOrSetterRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<GetterOrSetterRule>(this, &GetterOrSetterRule::A1Action));
        Cm::Parsing::NonterminalParser* getterNonterminalParser = GetNonterminal("getter");
        getterNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<GetterOrSetterRule>(this, &GetterOrSetterRule::Pregetter));
        getterNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<GetterOrSetterRule>(this, &GetterOrSetterRule::Postgetter));
        Cm::Parsing::NonterminalParser* setterNonterminalParser = GetNonterminal("setter");
        setterNonterminalParser->SetPreCall(new Cm::Parsing::MemberPreCall<GetterOrSetterRule>(this, &GetterOrSetterRule::Presetter));
        setterNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<GetterOrSetterRule>(this, &GetterOrSetterRule::Postsetter));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.subject->SetGetter(context.fromgetter);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.subject->SetSetter(context.fromsetter);
    }
    void Pregetter(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postgetter(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromgetter_value = std::move(stack.top());
            context.fromgetter = *static_cast<Cm::Parsing::ValueObject<CompoundStatementNode*>*>(fromgetter_value.get());
            stack.pop();
        }
    }
    void Presetter(Cm::Parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postsetter(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromsetter_value = std::move(stack.top());
            context.fromsetter = *static_cast<Cm::Parsing::ValueObject<CompoundStatementNode*>*>(fromsetter_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), subject(), fromgetter(), fromsetter() {}
        ParsingContext* ctx;
        Node* subject;
        CompoundStatementNode* fromgetter;
        CompoundStatementNode* fromsetter;
    };
    std::stack<Context> contextStack;
    Context context;
};

void ClassGrammar::GetReferencedGrammars()
{
    Cm::Parsing::ParsingDomain* pd = GetParsingDomain();
    Cm::Parsing::Grammar* grammar0 = pd->GetGrammar("cminor.parser.SpecifierGrammar");
    if (!grammar0)
    {
        grammar0 = cminor::parser::SpecifierGrammar::Create(pd);
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
    Cm::Parsing::Grammar* grammar3 = pd->GetGrammar("cminor.parser.ExpressionGrammar");
    if (!grammar3)
    {
        grammar3 = cminor::parser::ExpressionGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
    Cm::Parsing::Grammar* grammar4 = pd->GetGrammar("cminor.parser.StatementGrammar");
    if (!grammar4)
    {
        grammar4 = cminor::parser::StatementGrammar::Create(pd);
    }
    AddGrammarReference(grammar4);
    Cm::Parsing::Grammar* grammar5 = pd->GetGrammar("cminor.parser.TypeExprGrammar");
    if (!grammar5)
    {
        grammar5 = cminor::parser::TypeExprGrammar::Create(pd);
    }
    AddGrammarReference(grammar5);
    Cm::Parsing::Grammar* grammar6 = pd->GetGrammar("cminor.parser.ParameterGrammar");
    if (!grammar6)
    {
        grammar6 = cminor::parser::ParameterGrammar::Create(pd);
    }
    AddGrammarReference(grammar6);
    Cm::Parsing::Grammar* grammar7 = pd->GetGrammar("Cm.Parsing.stdlib");
    if (!grammar7)
    {
        grammar7 = Cm::Parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar7);
}

void ClassGrammar::CreateRules()
{
    AddRuleLink(new Cm::Parsing::RuleLink("Specifiers", this, "SpecifierGrammar.Specifiers"));
    AddRuleLink(new Cm::Parsing::RuleLink("Identifier", this, "IdentifierGrammar.Identifier"));
    AddRuleLink(new Cm::Parsing::RuleLink("Attributes", this, "FunctionGrammar.Attributes"));
    AddRuleLink(new Cm::Parsing::RuleLink("QualifiedId", this, "IdentifierGrammar.QualifiedId"));
    AddRuleLink(new Cm::Parsing::RuleLink("ArgumentList", this, "ExpressionGrammar.ArgumentList"));
    AddRuleLink(new Cm::Parsing::RuleLink("TypeExpr", this, "TypeExprGrammar.TypeExpr"));
    AddRuleLink(new Cm::Parsing::RuleLink("CompoundStatement", this, "StatementGrammar.CompoundStatement"));
    AddRuleLink(new Cm::Parsing::RuleLink("ParameterList", this, "ParameterGrammar.ParameterList"));
    AddRuleLink(new Cm::Parsing::RuleLink("FunctionGroupId", this, "FunctionGrammar.FunctionGroupId"));
    AddRuleLink(new Cm::Parsing::RuleLink("identifier", this, "Cm.Parsing.stdlib.identifier"));
    AddRule(new ClassRule("Class", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::SequenceParser(
                                new Cm::Parsing::NonterminalParser("Specifiers", "Specifiers", 0),
                                new Cm::Parsing::KeywordParser("class")),
                            new Cm::Parsing::ActionParser("A0",
                                new Cm::Parsing::NonterminalParser("Identifier", "Identifier", 0))),
                        new Cm::Parsing::OptionalParser(
                            new Cm::Parsing::NonterminalParser("InheritanceAndInterfaces", "InheritanceAndInterfaces", 2))),
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::CharParser('{'))),
                new Cm::Parsing::NonterminalParser("ClassContent", "ClassContent", 2)),
            new Cm::Parsing::ExpectationParser(
                new Cm::Parsing::CharParser('}')))));
    AddRule(new InheritanceAndInterfacesRule("InheritanceAndInterfaces", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::CharParser(':'),
            new Cm::Parsing::ListParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::ActionParser("A0",
                        new Cm::Parsing::EmptyParser()),
                    new Cm::Parsing::ActionParser("A1",
                        new Cm::Parsing::NonterminalParser("BaseClassOrInterface", "BaseClassOrInterface", 1))),
                new Cm::Parsing::CharParser(',')))));
    AddRule(new BaseClassOrInterfaceRule("BaseClassOrInterface", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::NonterminalParser("QualifiedId", "QualifiedId", 0))));
    AddRule(new ClassContentRule("ClassContent", GetScope(),
        new Cm::Parsing::KleeneStarParser(
            new Cm::Parsing::ActionParser("A0",
                new Cm::Parsing::NonterminalParser("ClassMember", "ClassMember", 2)))));
    AddRule(new ClassMemberRule("ClassMember", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::AlternativeParser(
                new Cm::Parsing::AlternativeParser(
                    new Cm::Parsing::AlternativeParser(
                        new Cm::Parsing::ActionParser("A0",
                            new Cm::Parsing::NonterminalParser("StaticConstructor", "StaticConstructor", 2)),
                        new Cm::Parsing::ActionParser("A1",
                            new Cm::Parsing::NonterminalParser("Constructor", "Constructor", 2))),
                    new Cm::Parsing::ActionParser("A2",
                        new Cm::Parsing::NonterminalParser("MemberFunction", "MemberFunction", 2))),
                new Cm::Parsing::ActionParser("A3",
                    new Cm::Parsing::NonterminalParser("MemberVariable", "MemberVariable", 1))),
            new Cm::Parsing::ActionParser("A4",
                new Cm::Parsing::NonterminalParser("Property", "Property", 1)))));
    AddRule(new StaticConstructorRule("StaticConstructor", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::ActionParser("A0",
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::SequenceParser(
                                new Cm::Parsing::OptionalParser(
                                    new Cm::Parsing::NonterminalParser("Attributes", "Attributes", 1)),
                                new Cm::Parsing::NonterminalParser("Specifiers", "Specifiers", 0)),
                            new Cm::Parsing::ActionParser("A1",
                                new Cm::Parsing::NonterminalParser("Identifier", "Identifier", 0))),
                        new Cm::Parsing::CharParser('(')),
                    new Cm::Parsing::CharParser(')'))),
            new Cm::Parsing::ActionParser("A2",
                new Cm::Parsing::NonterminalParser("CompoundStatement", "CompoundStatement", 1)))));
    AddRule(new ConstructorRule("Constructor", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::ActionParser("A0",
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::SequenceParser(
                                new Cm::Parsing::OptionalParser(
                                    new Cm::Parsing::NonterminalParser("Attributes", "Attributes", 1)),
                                new Cm::Parsing::NonterminalParser("Specifiers", "Specifiers", 0)),
                            new Cm::Parsing::ActionParser("A1",
                                new Cm::Parsing::NonterminalParser("Identifier", "Identifier", 0)))),
                    new Cm::Parsing::NonterminalParser("ParameterList", "ParameterList", 2)),
                new Cm::Parsing::OptionalParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::CharParser(':'),
                        new Cm::Parsing::ActionParser("A2",
                            new Cm::Parsing::NonterminalParser("Initializer", "Initializer", 1))))),
            new Cm::Parsing::ActionParser("A3",
                new Cm::Parsing::NonterminalParser("CompoundStatement", "CompoundStatement", 1)))));
    AddRule(new InitializerRule("Initializer", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::ActionParser("A0",
                            new Cm::Parsing::KeywordParser("this", "identifier")),
                        new Cm::Parsing::ExpectationParser(
                            new Cm::Parsing::CharParser('('))),
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::NonterminalParser("thisArgs", "ArgumentList", 2))),
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::CharParser(')'))),
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::ActionParser("A1",
                            new Cm::Parsing::KeywordParser("base", "identifier")),
                        new Cm::Parsing::ExpectationParser(
                            new Cm::Parsing::CharParser('('))),
                    new Cm::Parsing::ExpectationParser(
                        new Cm::Parsing::NonterminalParser("baseArgs", "ArgumentList", 2))),
                new Cm::Parsing::ExpectationParser(
                    new Cm::Parsing::CharParser(')'))))));
    AddRule(new MemberFunctionRule("MemberFunction", GetScope(),
        new Cm::Parsing::SequenceParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::ActionParser("A0",
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::SequenceParser(
                            new Cm::Parsing::SequenceParser(
                                new Cm::Parsing::OptionalParser(
                                    new Cm::Parsing::NonterminalParser("Attributes", "Attributes", 1)),
                                new Cm::Parsing::NonterminalParser("Specifiers", "Specifiers", 0)),
                            new Cm::Parsing::NonterminalParser("TypeExpr", "TypeExpr", 1)),
                        new Cm::Parsing::NonterminalParser("FunctionGroupId", "FunctionGroupId", 1))),
                new Cm::Parsing::NonterminalParser("ParameterList", "ParameterList", 2)),
            new Cm::Parsing::ActionParser("A1",
                new Cm::Parsing::AlternativeParser(
                    new Cm::Parsing::ActionParser("A2",
                        new Cm::Parsing::NonterminalParser("CompoundStatement", "CompoundStatement", 1)),
                    new Cm::Parsing::CharParser(';'))))));
    AddRule(new MemberVariableRule("MemberVariable", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::NonterminalParser("Specifiers", "Specifiers", 0),
                        new Cm::Parsing::NonterminalParser("TypeExpr", "TypeExpr", 1)),
                    new Cm::Parsing::NonterminalParser("Identifier", "Identifier", 0)),
                new Cm::Parsing::CharParser(';')))));
    AddRule(new PropertyRule("Property", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::SequenceParser(
                        new Cm::Parsing::ActionParser("A1",
                            new Cm::Parsing::SequenceParser(
                                new Cm::Parsing::SequenceParser(
                                    new Cm::Parsing::NonterminalParser("Specifiers", "Specifiers", 0),
                                    new Cm::Parsing::NonterminalParser("TypeExpr", "TypeExpr", 1)),
                                new Cm::Parsing::NonterminalParser("Identifier", "Identifier", 0))),
                        new Cm::Parsing::CharParser('{')),
                    new Cm::Parsing::PositiveParser(
                        new Cm::Parsing::NonterminalParser("GetterOrSetter", "GetterOrSetter", 2))),
                new Cm::Parsing::CharParser('}')))));
    AddRule(new GetterOrSetterRule("GetterOrSetter", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::KeywordParser("get"),
                new Cm::Parsing::ActionParser("A0",
                    new Cm::Parsing::NonterminalParser("getter", "CompoundStatement", 1))),
            new Cm::Parsing::SequenceParser(
                new Cm::Parsing::KeywordParser("set"),
                new Cm::Parsing::ActionParser("A1",
                    new Cm::Parsing::NonterminalParser("setter", "CompoundStatement", 1))))));
}

} } // namespace cminor.parser
