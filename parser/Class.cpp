#include "Class.hpp"
#include <cminor/pl/Action.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Nonterminal.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/StdLib.hpp>
#include <cminor/pl/XmlLog.hpp>
#include <cminor/parser/Specifier.hpp>
#include <cminor/parser/Identifier.hpp>
#include <cminor/parser/TypeExpr.hpp>
#include <cminor/parser/Expression.hpp>
#include <cminor/parser/Statement.hpp>
#include <cminor/parser/Parameter.hpp>
#include <cminor/parser/Function.hpp>
#include <cminor/parser/Template.hpp>
#include <cminor/parser/Enumeration.hpp>
#include <cminor/parser/Constant.hpp>
#include <cminor/parser/Delegate.hpp>

namespace cminor { namespace parser {

using namespace cminor::parsing;

ClassGrammar* ClassGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

ClassGrammar* ClassGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    ClassGrammar* grammar(new ClassGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

ClassGrammar::ClassGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar("ClassGrammar", parsingDomain_->GetNamespaceScope("cminor.parser"), parsingDomain_)
{
    SetOwner(0);
}

ClassNode* ClassGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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
    ClassNode* result = *static_cast<cminor::parsing::ValueObject<ClassNode*>*>(value.get());
    stack.pop();
    return result;
}

class ClassGrammar::ClassRule : public cminor::parsing::Rule
{
public:
    ClassRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassRule>(this, &ClassRule::A0Action));
        cminor::parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal("Specifiers");
        specifiersNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassRule>(this, &ClassRule::PostSpecifiers));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassRule>(this, &ClassRule::PostIdentifier));
        cminor::parsing::NonterminalParser* templateParameterListNonterminalParser = GetNonterminal("TemplateParameterList");
        templateParameterListNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassRule>(this, &ClassRule::PreTemplateParameterList));
        cminor::parsing::NonterminalParser* inheritanceAndInterfacesNonterminalParser = GetNonterminal("InheritanceAndInterfaces");
        inheritanceAndInterfacesNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassRule>(this, &ClassRule::PreInheritanceAndInterfaces));
        cminor::parsing::NonterminalParser* classContentNonterminalParser = GetNonterminal("ClassContent");
        classContentNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassRule>(this, &ClassRule::PreClassContent));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ClassNode(span, context.fromSpecifiers, context.fromIdentifier);
    }
    void PostSpecifiers(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context.fromSpecifiers = *static_cast<cminor::parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
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
    void PreTemplateParameterList(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.value)));
    }
    void PreInheritanceAndInterfaces(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ClassNode*>(context.value)));
    }
    void PreClassContent(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ClassNode*>(context.value)));
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

class ClassGrammar::InheritanceAndInterfacesRule : public cminor::parsing::Rule
{
public:
    InheritanceAndInterfacesRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("ClassNode*", "classNode"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> classNode_value = std::move(stack.top());
        context.classNode = *static_cast<cminor::parsing::ValueObject<ClassNode*>*>(classNode_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InheritanceAndInterfacesRule>(this, &InheritanceAndInterfacesRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InheritanceAndInterfacesRule>(this, &InheritanceAndInterfacesRule::A1Action));
        a1ActionParser->SetFailureAction(new cminor::parsing::MemberFailureAction<InheritanceAndInterfacesRule>(this, &InheritanceAndInterfacesRule::A1ActionFail));
        cminor::parsing::NonterminalParser* baseClassOrInterfaceNonterminalParser = GetNonterminal("BaseClassOrInterface");
        baseClassOrInterfaceNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<InheritanceAndInterfacesRule>(this, &InheritanceAndInterfacesRule::PreBaseClassOrInterface));
        baseClassOrInterfaceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InheritanceAndInterfacesRule>(this, &InheritanceAndInterfacesRule::PostBaseClassOrInterface));
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
    void PreBaseClassOrInterface(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostBaseClassOrInterface(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromBaseClassOrInterface_value = std::move(stack.top());
            context.fromBaseClassOrInterface = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromBaseClassOrInterface_value.get());
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

class ClassGrammar::BaseClassOrInterfaceRule : public cminor::parsing::Rule
{
public:
    BaseClassOrInterfaceRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BaseClassOrInterfaceRule>(this, &BaseClassOrInterfaceRule::A0Action));
        cminor::parsing::NonterminalParser* qualifiedIdNonterminalParser = GetNonterminal("QualifiedId");
        qualifiedIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<BaseClassOrInterfaceRule>(this, &BaseClassOrInterfaceRule::PostQualifiedId));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromQualifiedId;
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
        Context(): ctx(), value(), fromQualifiedId() {}
        ParsingContext* ctx;
        Node* value;
        IdentifierNode* fromQualifiedId;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ClassGrammar::ClassContentRule : public cminor::parsing::Rule
{
public:
    ClassContentRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("ClassNode*", "classNode"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> classNode_value = std::move(stack.top());
        context.classNode = *static_cast<cminor::parsing::ValueObject<ClassNode*>*>(classNode_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassContentRule>(this, &ClassContentRule::A0Action));
        cminor::parsing::NonterminalParser* classMemberNonterminalParser = GetNonterminal("ClassMember");
        classMemberNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassContentRule>(this, &ClassContentRule::PreClassMember));
        classMemberNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassContentRule>(this, &ClassContentRule::PostClassMember));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.classNode->AddMember(context.fromClassMember);
    }
    void PreClassMember(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ClassNode*>(context.classNode)));
    }
    void PostClassMember(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromClassMember_value = std::move(stack.top());
            context.fromClassMember = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromClassMember_value.get());
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

class ClassGrammar::ClassMemberRule : public cminor::parsing::Rule
{
public:
    ClassMemberRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("ClassNode*", "classNode"));
        SetValueTypeName("Node*");
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> classNode_value = std::move(stack.top());
        context.classNode = *static_cast<cminor::parsing::ValueObject<ClassNode*>*>(classNode_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction("A6");
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction("A7");
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction("A8");
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A8Action));
        cminor::parsing::ActionParser* a9ActionParser = GetAction("A9");
        a9ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A9Action));
        cminor::parsing::ActionParser* a10ActionParser = GetAction("A10");
        a10ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A10Action));
        cminor::parsing::NonterminalParser* staticConstructorNonterminalParser = GetNonterminal("StaticConstructor");
        staticConstructorNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreStaticConstructor));
        staticConstructorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostStaticConstructor));
        cminor::parsing::NonterminalParser* constructorNonterminalParser = GetNonterminal("Constructor");
        constructorNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreConstructor));
        constructorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostConstructor));
        cminor::parsing::NonterminalParser* memberFunctionNonterminalParser = GetNonterminal("MemberFunction");
        memberFunctionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreMemberFunction));
        memberFunctionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostMemberFunction));
        cminor::parsing::NonterminalParser* memberVariableNonterminalParser = GetNonterminal("MemberVariable");
        memberVariableNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreMemberVariable));
        memberVariableNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostMemberVariable));
        cminor::parsing::NonterminalParser* propertyNonterminalParser = GetNonterminal("Property");
        propertyNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreProperty));
        propertyNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostProperty));
        cminor::parsing::NonterminalParser* indexerNonterminalParser = GetNonterminal("Indexer");
        indexerNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreIndexer));
        indexerNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostIndexer));
        cminor::parsing::NonterminalParser* classNonterminalParser = GetNonterminal("Class");
        classNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreClass));
        classNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostClass));
        cminor::parsing::NonterminalParser* enumTypeNonterminalParser = GetNonterminal("EnumType");
        enumTypeNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreEnumType));
        enumTypeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostEnumType));
        cminor::parsing::NonterminalParser* constantNonterminalParser = GetNonterminal("Constant");
        constantNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreConstant));
        constantNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostConstant));
        cminor::parsing::NonterminalParser* delegateNonterminalParser = GetNonterminal("Delegate");
        delegateNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreDelegate));
        delegateNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostDelegate));
        cminor::parsing::NonterminalParser* classDelegateNonterminalParser = GetNonterminal("ClassDelegate");
        classDelegateNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreClassDelegate));
        classDelegateNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostClassDelegate));
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
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromIndexer;
    }
    void A6Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromClass;
    }
    void A7Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromEnumType;
    }
    void A8Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromConstant;
    }
    void A9Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromDelegate;
    }
    void A10Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.fromClassDelegate;
    }
    void PreStaticConstructor(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ClassNode*>(context.classNode)));
    }
    void PostStaticConstructor(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromStaticConstructor_value = std::move(stack.top());
            context.fromStaticConstructor = *static_cast<cminor::parsing::ValueObject<StaticConstructorNode*>*>(fromStaticConstructor_value.get());
            stack.pop();
        }
    }
    void PreConstructor(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ClassNode*>(context.classNode)));
    }
    void PostConstructor(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromConstructor_value = std::move(stack.top());
            context.fromConstructor = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromConstructor_value.get());
            stack.pop();
        }
    }
    void PreMemberFunction(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ClassNode*>(context.classNode)));
    }
    void PostMemberFunction(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromMemberFunction_value = std::move(stack.top());
            context.fromMemberFunction = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromMemberFunction_value.get());
            stack.pop();
        }
    }
    void PreMemberVariable(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostMemberVariable(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromMemberVariable_value = std::move(stack.top());
            context.fromMemberVariable = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromMemberVariable_value.get());
            stack.pop();
        }
    }
    void PreProperty(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostProperty(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromProperty_value = std::move(stack.top());
            context.fromProperty = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromProperty_value.get());
            stack.pop();
        }
    }
    void PreIndexer(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostIndexer(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIndexer_value = std::move(stack.top());
            context.fromIndexer = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromIndexer_value.get());
            stack.pop();
        }
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
    void PreDelegate(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostDelegate(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDelegate_value = std::move(stack.top());
            context.fromDelegate = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromDelegate_value.get());
            stack.pop();
        }
    }
    void PreClassDelegate(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostClassDelegate(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromClassDelegate_value = std::move(stack.top());
            context.fromClassDelegate = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromClassDelegate_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): ctx(), classNode(), value(), fromStaticConstructor(), fromConstructor(), fromMemberFunction(), fromMemberVariable(), fromProperty(), fromIndexer(), fromClass(), fromEnumType(), fromConstant(), fromDelegate(), fromClassDelegate() {}
        ParsingContext* ctx;
        ClassNode* classNode;
        Node* value;
        StaticConstructorNode* fromStaticConstructor;
        Node* fromConstructor;
        Node* fromMemberFunction;
        Node* fromMemberVariable;
        Node* fromProperty;
        Node* fromIndexer;
        ClassNode* fromClass;
        EnumTypeNode* fromEnumType;
        ConstantNode* fromConstant;
        Node* fromDelegate;
        Node* fromClassDelegate;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ClassGrammar::StaticConstructorRule : public cminor::parsing::Rule
{
public:
    StaticConstructorRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("ClassNode*", "classNode"));
        SetValueTypeName("StaticConstructorNode*");
        AddLocalVariable(AttrOrVariable("AttributeMap", "attributeMap"));
        AddLocalVariable(AttrOrVariable("std::unique_ptr<IdentifierNode>", "id"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> classNode_value = std::move(stack.top());
        context.classNode = *static_cast<cminor::parsing::ValueObject<ClassNode*>*>(classNode_value.get());
        stack.pop();
        std::unique_ptr<cminor::parsing::Object> ctx_value = std::move(stack.top());
        context.ctx = *static_cast<cminor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StaticConstructorNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StaticConstructorRule>(this, &StaticConstructorRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StaticConstructorRule>(this, &StaticConstructorRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StaticConstructorRule>(this, &StaticConstructorRule::A2Action));
        cminor::parsing::NonterminalParser* attributesNonterminalParser = GetNonterminal("Attributes");
        attributesNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<StaticConstructorRule>(this, &StaticConstructorRule::PreAttributes));
        cminor::parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal("Specifiers");
        specifiersNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StaticConstructorRule>(this, &StaticConstructorRule::PostSpecifiers));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StaticConstructorRule>(this, &StaticConstructorRule::PostIdentifier));
        cminor::parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal("CompoundStatement");
        compoundStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<StaticConstructorRule>(this, &StaticConstructorRule::PreCompoundStatement));
        compoundStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StaticConstructorRule>(this, &StaticConstructorRule::PostCompoundStatement));
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
    void PreAttributes(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<AttributeMap*>(&context.attributeMap)));
    }
    void PostSpecifiers(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context.fromSpecifiers = *static_cast<cminor::parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
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
    void PreCompoundStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostCompoundStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context.fromCompoundStatement = *static_cast<cminor::parsing::ValueObject<CompoundStatementNode*>*>(fromCompoundStatement_value.get());
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

class ClassGrammar::ConstructorRule : public cminor::parsing::Rule
{
public:
    ConstructorRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("ClassNode*", "classNode"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("AttributeMap", "attributeMap"));
        AddLocalVariable(AttrOrVariable("std::unique_ptr<IdentifierNode>", "id"));
        AddLocalVariable(AttrOrVariable("std::unique_ptr<ConstructorNode>", "ctor"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> classNode_value = std::move(stack.top());
        context.classNode = *static_cast<cminor::parsing::ValueObject<ClassNode*>*>(classNode_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConstructorRule>(this, &ConstructorRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConstructorRule>(this, &ConstructorRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConstructorRule>(this, &ConstructorRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConstructorRule>(this, &ConstructorRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConstructorRule>(this, &ConstructorRule::A4Action));
        cminor::parsing::NonterminalParser* attributesNonterminalParser = GetNonterminal("Attributes");
        attributesNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ConstructorRule>(this, &ConstructorRule::PreAttributes));
        cminor::parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal("Specifiers");
        specifiersNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConstructorRule>(this, &ConstructorRule::PostSpecifiers));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConstructorRule>(this, &ConstructorRule::PostIdentifier));
        cminor::parsing::NonterminalParser* parameterListNonterminalParser = GetNonterminal("ParameterList");
        parameterListNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ConstructorRule>(this, &ConstructorRule::PreParameterList));
        cminor::parsing::NonterminalParser* initializerNonterminalParser = GetNonterminal("Initializer");
        initializerNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ConstructorRule>(this, &ConstructorRule::PreInitializer));
        initializerNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConstructorRule>(this, &ConstructorRule::PostInitializer));
        cminor::parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal("CompoundStatement");
        compoundStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ConstructorRule>(this, &ConstructorRule::PreCompoundStatement));
        compoundStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConstructorRule>(this, &ConstructorRule::PostCompoundStatement));
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
        context.value = context.ctor.release();
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.ctor->SetBody(context.fromCompoundStatement);
    }
    void PreAttributes(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<AttributeMap*>(&context.attributeMap)));
    }
    void PostSpecifiers(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context.fromSpecifiers = *static_cast<cminor::parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
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
    void PreParameterList(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.ctor.get())));
    }
    void PreInitializer(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostInitializer(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromInitializer_value = std::move(stack.top());
            context.fromInitializer = *static_cast<cminor::parsing::ValueObject<InitializerNode*>*>(fromInitializer_value.get());
            stack.pop();
        }
    }
    void PreCompoundStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostCompoundStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context.fromCompoundStatement = *static_cast<cminor::parsing::ValueObject<CompoundStatementNode*>*>(fromCompoundStatement_value.get());
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

class ClassGrammar::InitializerRule : public cminor::parsing::Rule
{
public:
    InitializerRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("InitializerNode*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<InitializerNode*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitializerRule>(this, &InitializerRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitializerRule>(this, &InitializerRule::A1Action));
        cminor::parsing::NonterminalParser* thisArgsNonterminalParser = GetNonterminal("thisArgs");
        thisArgsNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<InitializerRule>(this, &InitializerRule::PrethisArgs));
        cminor::parsing::NonterminalParser* baseArgsNonterminalParser = GetNonterminal("baseArgs");
        baseArgsNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<InitializerRule>(this, &InitializerRule::PrebaseArgs));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new ThisInitializerNode(span);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new BaseInitializerNode(span);
    }
    void PrethisArgs(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.value)));
    }
    void PrebaseArgs(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.value)));
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

class ClassGrammar::MemberFunctionRule : public cminor::parsing::Rule
{
public:
    MemberFunctionRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("ClassNode*", "classNode"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("AttributeMap", "attributeMap"));
        AddLocalVariable(AttrOrVariable("std::unique_ptr<MemberFunctionNode>", "memFun"));
        AddLocalVariable(AttrOrVariable("std::unique_ptr<IdentifierNode>", "qid"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> classNode_value = std::move(stack.top());
        context.classNode = *static_cast<cminor::parsing::ValueObject<ClassNode*>*>(classNode_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MemberFunctionRule>(this, &MemberFunctionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MemberFunctionRule>(this, &MemberFunctionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MemberFunctionRule>(this, &MemberFunctionRule::A2Action));
        cminor::parsing::NonterminalParser* attributesNonterminalParser = GetNonterminal("Attributes");
        attributesNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<MemberFunctionRule>(this, &MemberFunctionRule::PreAttributes));
        cminor::parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal("Specifiers");
        specifiersNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<MemberFunctionRule>(this, &MemberFunctionRule::PostSpecifiers));
        cminor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<MemberFunctionRule>(this, &MemberFunctionRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<MemberFunctionRule>(this, &MemberFunctionRule::PostTypeExpr));
        cminor::parsing::NonterminalParser* functionGroupIdNonterminalParser = GetNonterminal("FunctionGroupId");
        functionGroupIdNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<MemberFunctionRule>(this, &MemberFunctionRule::PreFunctionGroupId));
        functionGroupIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<MemberFunctionRule>(this, &MemberFunctionRule::PostFunctionGroupId));
        cminor::parsing::NonterminalParser* parameterListNonterminalParser = GetNonterminal("ParameterList");
        parameterListNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<MemberFunctionRule>(this, &MemberFunctionRule::PreParameterList));
        cminor::parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal("CompoundStatement");
        compoundStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<MemberFunctionRule>(this, &MemberFunctionRule::PreCompoundStatement));
        compoundStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<MemberFunctionRule>(this, &MemberFunctionRule::PostCompoundStatement));
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
    void PreAttributes(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<AttributeMap*>(&context.attributeMap)));
    }
    void PostSpecifiers(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context.fromSpecifiers = *static_cast<cminor::parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
    }
    void PreTypeExpr(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostTypeExpr(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context.fromTypeExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
            stack.pop();
        }
    }
    void PreFunctionGroupId(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostFunctionGroupId(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFunctionGroupId_value = std::move(stack.top());
            context.fromFunctionGroupId = *static_cast<cminor::parsing::ValueObject<FunctionGroupIdNode*>*>(fromFunctionGroupId_value.get());
            stack.pop();
        }
    }
    void PreParameterList(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.memFun.get())));
    }
    void PreCompoundStatement(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostCompoundStatement(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context.fromCompoundStatement = *static_cast<cminor::parsing::ValueObject<CompoundStatementNode*>*>(fromCompoundStatement_value.get());
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

class ClassGrammar::MemberVariableRule : public cminor::parsing::Rule
{
public:
    MemberVariableRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MemberVariableRule>(this, &MemberVariableRule::A0Action));
        cminor::parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal("Specifiers");
        specifiersNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<MemberVariableRule>(this, &MemberVariableRule::PostSpecifiers));
        cminor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<MemberVariableRule>(this, &MemberVariableRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<MemberVariableRule>(this, &MemberVariableRule::PostTypeExpr));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<MemberVariableRule>(this, &MemberVariableRule::PostIdentifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = new MemberVariableNode(span, context.fromSpecifiers, context.fromTypeExpr, context.fromIdentifier);
    }
    void PostSpecifiers(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context.fromSpecifiers = *static_cast<cminor::parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
    }
    void PreTypeExpr(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostTypeExpr(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context.fromTypeExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
            stack.pop();
        }
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

class ClassGrammar::PropertyRule : public cminor::parsing::Rule
{
public:
    PropertyRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "property"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PropertyRule>(this, &PropertyRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PropertyRule>(this, &PropertyRule::A1Action));
        cminor::parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal("Specifiers");
        specifiersNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PropertyRule>(this, &PropertyRule::PostSpecifiers));
        cminor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal("TypeExpr");
        typeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PropertyRule>(this, &PropertyRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PropertyRule>(this, &PropertyRule::PostTypeExpr));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PropertyRule>(this, &PropertyRule::PostIdentifier));
        cminor::parsing::NonterminalParser* getterOrSetterNonterminalParser = GetNonterminal("GetterOrSetter");
        getterOrSetterNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PropertyRule>(this, &PropertyRule::PreGetterOrSetter));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.property.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.property.reset(new PropertyNode(span, context.fromSpecifiers, context.fromTypeExpr, context.fromIdentifier));
    }
    void PostSpecifiers(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context.fromSpecifiers = *static_cast<cminor::parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
    }
    void PreTypeExpr(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostTypeExpr(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context.fromTypeExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
            stack.pop();
        }
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
    void PreGetterOrSetter(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.property.get())));
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

class ClassGrammar::IndexerRule : public cminor::parsing::Rule
{
public:
    IndexerRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        SetValueTypeName("Node*");
        AddLocalVariable(AttrOrVariable("std::unique_ptr<Node>", "indexer"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IndexerRule>(this, &IndexerRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IndexerRule>(this, &IndexerRule::A1Action));
        cminor::parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal("Specifiers");
        specifiersNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IndexerRule>(this, &IndexerRule::PostSpecifiers));
        cminor::parsing::NonterminalParser* valueTypeNonterminalParser = GetNonterminal("valueType");
        valueTypeNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<IndexerRule>(this, &IndexerRule::PrevalueType));
        valueTypeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IndexerRule>(this, &IndexerRule::PostvalueType));
        cminor::parsing::NonterminalParser* indexTypeNonterminalParser = GetNonterminal("indexType");
        indexTypeNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<IndexerRule>(this, &IndexerRule::PreindexType));
        indexTypeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IndexerRule>(this, &IndexerRule::PostindexType));
        cminor::parsing::NonterminalParser* indexNonterminalParser = GetNonterminal("index");
        indexNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IndexerRule>(this, &IndexerRule::Postindex));
        cminor::parsing::NonterminalParser* getterOrSetterNonterminalParser = GetNonterminal("GetterOrSetter");
        getterOrSetterNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<IndexerRule>(this, &IndexerRule::PreGetterOrSetter));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.indexer.release();
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.indexer.reset(new IndexerNode(span, context.fromSpecifiers, context.fromvalueType, context.fromindexType, context.fromindex));
    }
    void PostSpecifiers(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context.fromSpecifiers = *static_cast<cminor::parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
    }
    void PrevalueType(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostvalueType(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromvalueType_value = std::move(stack.top());
            context.fromvalueType = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromvalueType_value.get());
            stack.pop();
        }
    }
    void PreindexType(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void PostindexType(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromindexType_value = std::move(stack.top());
            context.fromindexType = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromindexType_value.get());
            stack.pop();
        }
    }
    void Postindex(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromindex_value = std::move(stack.top());
            context.fromindex = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromindex_value.get());
            stack.pop();
        }
    }
    void PreGetterOrSetter(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context.indexer.get())));
    }
private:
    struct Context
    {
        Context(): ctx(), value(), indexer(), fromSpecifiers(), fromvalueType(), fromindexType(), fromindex() {}
        ParsingContext* ctx;
        Node* value;
        std::unique_ptr<Node> indexer;
        Specifiers fromSpecifiers;
        Node* fromvalueType;
        Node* fromindexType;
        IdentifierNode* fromindex;
    };
    std::stack<Context> contextStack;
    Context context;
};

class ClassGrammar::GetterOrSetterRule : public cminor::parsing::Rule
{
public:
    GetterOrSetterRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        AddInheritedAttribute(AttrOrVariable("ParsingContext*", "ctx"));
        AddInheritedAttribute(AttrOrVariable("Node*", "subject"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
        std::unique_ptr<cminor::parsing::Object> subject_value = std::move(stack.top());
        context.subject = *static_cast<cminor::parsing::ValueObject<Node*>*>(subject_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<GetterOrSetterRule>(this, &GetterOrSetterRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<GetterOrSetterRule>(this, &GetterOrSetterRule::A1Action));
        cminor::parsing::NonterminalParser* getterNonterminalParser = GetNonterminal("getter");
        getterNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<GetterOrSetterRule>(this, &GetterOrSetterRule::Pregetter));
        getterNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<GetterOrSetterRule>(this, &GetterOrSetterRule::Postgetter));
        cminor::parsing::NonterminalParser* setterNonterminalParser = GetNonterminal("setter");
        setterNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<GetterOrSetterRule>(this, &GetterOrSetterRule::Presetter));
        setterNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<GetterOrSetterRule>(this, &GetterOrSetterRule::Postsetter));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.subject->SetGetter(context.fromgetter);
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.subject->SetSetter(context.fromsetter);
    }
    void Pregetter(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postgetter(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromgetter_value = std::move(stack.top());
            context.fromgetter = *static_cast<cminor::parsing::ValueObject<CompoundStatementNode*>*>(fromgetter_value.get());
            stack.pop();
        }
    }
    void Presetter(cminor::parsing::ObjectStack& stack)
    {
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context.ctx)));
    }
    void Postsetter(cminor::parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromsetter_value = std::move(stack.top());
            context.fromsetter = *static_cast<cminor::parsing::ValueObject<CompoundStatementNode*>*>(fromsetter_value.get());
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
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar("cminor.parser.TemplateGrammar");
    if (!grammar0)
    {
        grammar0 = cminor::parser::TemplateGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar("cminor.parser.SpecifierGrammar");
    if (!grammar1)
    {
        grammar1 = cminor::parser::SpecifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar("cminor.parser.IdentifierGrammar");
    if (!grammar2)
    {
        grammar2 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar("cminor.parser.StatementGrammar");
    if (!grammar3)
    {
        grammar3 = cminor::parser::StatementGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
    cminor::parsing::Grammar* grammar4 = pd->GetGrammar("cminor.parser.TypeExprGrammar");
    if (!grammar4)
    {
        grammar4 = cminor::parser::TypeExprGrammar::Create(pd);
    }
    AddGrammarReference(grammar4);
    cminor::parsing::Grammar* grammar5 = pd->GetGrammar("cminor.parser.ExpressionGrammar");
    if (!grammar5)
    {
        grammar5 = cminor::parser::ExpressionGrammar::Create(pd);
    }
    AddGrammarReference(grammar5);
    cminor::parsing::Grammar* grammar6 = pd->GetGrammar("cminor.parsing.stdlib");
    if (!grammar6)
    {
        grammar6 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar6);
    cminor::parsing::Grammar* grammar7 = pd->GetGrammar("cminor.parser.ParameterGrammar");
    if (!grammar7)
    {
        grammar7 = cminor::parser::ParameterGrammar::Create(pd);
    }
    AddGrammarReference(grammar7);
    cminor::parsing::Grammar* grammar8 = pd->GetGrammar("cminor.parser.FunctionGrammar");
    if (!grammar8)
    {
        grammar8 = cminor::parser::FunctionGrammar::Create(pd);
    }
    AddGrammarReference(grammar8);
    cminor::parsing::Grammar* grammar9 = pd->GetGrammar("cminor.parser.EnumerationGrammar");
    if (!grammar9)
    {
        grammar9 = cminor::parser::EnumerationGrammar::Create(pd);
    }
    AddGrammarReference(grammar9);
    cminor::parsing::Grammar* grammar10 = pd->GetGrammar("cminor.parser.ConstantGrammar");
    if (!grammar10)
    {
        grammar10 = cminor::parser::ConstantGrammar::Create(pd);
    }
    AddGrammarReference(grammar10);
    cminor::parsing::Grammar* grammar11 = pd->GetGrammar("cminor.parser.DelegateGrammar");
    if (!grammar11)
    {
        grammar11 = cminor::parser::DelegateGrammar::Create(pd);
    }
    AddGrammarReference(grammar11);
}

void ClassGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink("Specifiers", this, "SpecifierGrammar.Specifiers"));
    AddRuleLink(new cminor::parsing::RuleLink("Identifier", this, "IdentifierGrammar.Identifier"));
    AddRuleLink(new cminor::parsing::RuleLink("QualifiedId", this, "IdentifierGrammar.QualifiedId"));
    AddRuleLink(new cminor::parsing::RuleLink("ArgumentList", this, "ExpressionGrammar.ArgumentList"));
    AddRuleLink(new cminor::parsing::RuleLink("TypeExpr", this, "TypeExprGrammar.TypeExpr"));
    AddRuleLink(new cminor::parsing::RuleLink("CompoundStatement", this, "StatementGrammar.CompoundStatement"));
    AddRuleLink(new cminor::parsing::RuleLink("ParameterList", this, "ParameterGrammar.ParameterList"));
    AddRuleLink(new cminor::parsing::RuleLink("FunctionGroupId", this, "FunctionGrammar.FunctionGroupId"));
    AddRuleLink(new cminor::parsing::RuleLink("Attributes", this, "FunctionGrammar.Attributes"));
    AddRuleLink(new cminor::parsing::RuleLink("TemplateParameterList", this, "TemplateGrammar.TemplateParameterList"));
    AddRuleLink(new cminor::parsing::RuleLink("identifier", this, "cminor.parsing.stdlib.identifier"));
    AddRuleLink(new cminor::parsing::RuleLink("EnumType", this, "EnumerationGrammar.EnumType"));
    AddRuleLink(new cminor::parsing::RuleLink("Constant", this, "ConstantGrammar.Constant"));
    AddRuleLink(new cminor::parsing::RuleLink("Delegate", this, "DelegateGrammar.Delegate"));
    AddRuleLink(new cminor::parsing::RuleLink("ClassDelegate", this, "DelegateGrammar.ClassDelegate"));
    AddRule(new ClassRule("Class", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::NonterminalParser("Specifiers", "Specifiers", 0),
                                    new cminor::parsing::KeywordParser("class")),
                                new cminor::parsing::ActionParser("A0",
                                    new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0))),
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::NonterminalParser("TemplateParameterList", "TemplateParameterList", 2))),
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::NonterminalParser("InheritanceAndInterfaces", "InheritanceAndInterfaces", 2))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('{'))),
                new cminor::parsing::NonterminalParser("ClassContent", "ClassContent", 2)),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::CharParser('}')))));
    AddRule(new InheritanceAndInterfacesRule("InheritanceAndInterfaces", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::CharParser(':'),
            new cminor::parsing::ListParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::ActionParser("A0",
                        new cminor::parsing::EmptyParser()),
                    new cminor::parsing::ActionParser("A1",
                        new cminor::parsing::NonterminalParser("BaseClassOrInterface", "BaseClassOrInterface", 1))),
                new cminor::parsing::CharParser(',')))));
    AddRule(new BaseClassOrInterfaceRule("BaseClassOrInterface", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::NonterminalParser("QualifiedId", "QualifiedId", 0))));
    AddRule(new ClassContentRule("ClassContent", GetScope(),
        new cminor::parsing::KleeneStarParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::NonterminalParser("ClassMember", "ClassMember", 2)))));
    AddRule(new ClassMemberRule("ClassMember", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::AlternativeParser(
                                    new cminor::parsing::AlternativeParser(
                                        new cminor::parsing::AlternativeParser(
                                            new cminor::parsing::AlternativeParser(
                                                new cminor::parsing::ActionParser("A0",
                                                    new cminor::parsing::NonterminalParser("StaticConstructor", "StaticConstructor", 2)),
                                                new cminor::parsing::ActionParser("A1",
                                                    new cminor::parsing::NonterminalParser("Constructor", "Constructor", 2))),
                                            new cminor::parsing::ActionParser("A2",
                                                new cminor::parsing::NonterminalParser("MemberFunction", "MemberFunction", 2))),
                                        new cminor::parsing::ActionParser("A3",
                                            new cminor::parsing::NonterminalParser("MemberVariable", "MemberVariable", 1))),
                                    new cminor::parsing::ActionParser("A4",
                                        new cminor::parsing::NonterminalParser("Property", "Property", 1))),
                                new cminor::parsing::ActionParser("A5",
                                    new cminor::parsing::NonterminalParser("Indexer", "Indexer", 1))),
                            new cminor::parsing::ActionParser("A6",
                                new cminor::parsing::NonterminalParser("Class", "Class", 1))),
                        new cminor::parsing::ActionParser("A7",
                            new cminor::parsing::NonterminalParser("EnumType", "EnumType", 1))),
                    new cminor::parsing::ActionParser("A8",
                        new cminor::parsing::NonterminalParser("Constant", "Constant", 1))),
                new cminor::parsing::ActionParser("A9",
                    new cminor::parsing::NonterminalParser("Delegate", "Delegate", 1))),
            new cminor::parsing::ActionParser("A10",
                new cminor::parsing::NonterminalParser("ClassDelegate", "ClassDelegate", 1)))));
    AddRule(new StaticConstructorRule("StaticConstructor", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser("A0",
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::OptionalParser(
                                    new cminor::parsing::NonterminalParser("Attributes", "Attributes", 1)),
                                new cminor::parsing::NonterminalParser("Specifiers", "Specifiers", 0)),
                            new cminor::parsing::ActionParser("A1",
                                new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0))),
                        new cminor::parsing::CharParser('(')),
                    new cminor::parsing::CharParser(')'))),
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser("A2",
                    new cminor::parsing::NonterminalParser("CompoundStatement", "CompoundStatement", 1)),
                new cminor::parsing::CharParser(';')))));
    AddRule(new ConstructorRule("Constructor", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::ActionParser("A0",
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::OptionalParser(
                                    new cminor::parsing::NonterminalParser("Attributes", "Attributes", 1)),
                                new cminor::parsing::NonterminalParser("Specifiers", "Specifiers", 0)),
                            new cminor::parsing::ActionParser("A1",
                                new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0)))),
                    new cminor::parsing::NonterminalParser("ParameterList", "ParameterList", 2)),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::CharParser(':'),
                        new cminor::parsing::ActionParser("A2",
                            new cminor::parsing::NonterminalParser("Initializer", "Initializer", 1))))),
            new cminor::parsing::ActionParser("A3",
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::ActionParser("A4",
                        new cminor::parsing::NonterminalParser("CompoundStatement", "CompoundStatement", 1)),
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new InitializerRule("Initializer", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser("A0",
                            new cminor::parsing::KeywordParser("this", "identifier")),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser('('))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("thisArgs", "ArgumentList", 2))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser("A1",
                            new cminor::parsing::KeywordParser("base", "identifier")),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser('('))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser("baseArgs", "ArgumentList", 2))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))))));
    AddRule(new MemberFunctionRule("MemberFunction", GetScope(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::OptionalParser(
                                    new cminor::parsing::NonterminalParser("Attributes", "Attributes", 1)),
                                new cminor::parsing::NonterminalParser("Specifiers", "Specifiers", 0)),
                            new cminor::parsing::NonterminalParser("TypeExpr", "TypeExpr", 1)),
                        new cminor::parsing::NonterminalParser("FunctionGroupId", "FunctionGroupId", 1))),
                new cminor::parsing::NonterminalParser("ParameterList", "ParameterList", 2)),
            new cminor::parsing::ActionParser("A1",
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::ActionParser("A2",
                        new cminor::parsing::NonterminalParser("CompoundStatement", "CompoundStatement", 1)),
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new MemberVariableRule("MemberVariable", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::NonterminalParser("Specifiers", "Specifiers", 0),
                        new cminor::parsing::NonterminalParser("TypeExpr", "TypeExpr", 1)),
                    new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0)),
                new cminor::parsing::CharParser(';')))));
    AddRule(new PropertyRule("Property", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser("A1",
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::NonterminalParser("Specifiers", "Specifiers", 0),
                                    new cminor::parsing::NonterminalParser("TypeExpr", "TypeExpr", 1)),
                                new cminor::parsing::NonterminalParser("Identifier", "Identifier", 0))),
                        new cminor::parsing::CharParser('{')),
                    new cminor::parsing::PositiveParser(
                        new cminor::parsing::NonterminalParser("GetterOrSetter", "GetterOrSetter", 2))),
                new cminor::parsing::CharParser('}')))));
    AddRule(new IndexerRule("Indexer", GetScope(),
        new cminor::parsing::ActionParser("A0",
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser("A1",
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::SequenceParser(
                                            new cminor::parsing::SequenceParser(
                                                new cminor::parsing::SequenceParser(
                                                    new cminor::parsing::NonterminalParser("Specifiers", "Specifiers", 0),
                                                    new cminor::parsing::NonterminalParser("valueType", "TypeExpr", 1)),
                                                new cminor::parsing::KeywordParser("this", "identifier")),
                                            new cminor::parsing::ExpectationParser(
                                                new cminor::parsing::CharParser('['))),
                                        new cminor::parsing::ExpectationParser(
                                            new cminor::parsing::NonterminalParser("indexType", "TypeExpr", 1))),
                                    new cminor::parsing::ExpectationParser(
                                        new cminor::parsing::NonterminalParser("index", "Identifier", 0))),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::CharParser(']')))),
                        new cminor::parsing::CharParser('{')),
                    new cminor::parsing::PositiveParser(
                        new cminor::parsing::NonterminalParser("GetterOrSetter", "GetterOrSetter", 2))),
                new cminor::parsing::CharParser('}')))));
    AddRule(new GetterOrSetterRule("GetterOrSetter", GetScope(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser("get"),
                new cminor::parsing::ActionParser("A0",
                    new cminor::parsing::NonterminalParser("getter", "CompoundStatement", 1))),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser("set"),
                new cminor::parsing::ActionParser("A1",
                    new cminor::parsing::NonterminalParser("setter", "CompoundStatement", 1))))));
}

} } // namespace cminor.parser
