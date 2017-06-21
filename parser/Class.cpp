#include "Class.hpp"
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
using namespace cminor::util;
using namespace cminor::unicode;

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

ClassGrammar::ClassGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("ClassGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.parser")), parsingDomain_)
{
    SetOwner(0);
}

ClassNode* ClassGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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
    ClassNode* result = *static_cast<cminor::parsing::ValueObject<ClassNode*>*>(value.get());
    stack.pop();
    return result;
}

class ClassGrammar::ClassRule : public cminor::parsing::Rule
{
public:
    ClassRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassRule>(this, &ClassRule::A0Action));
        cminor::parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal(ToUtf32("Specifiers"));
        specifiersNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassRule>(this, &ClassRule::PostSpecifiers));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("Identifier"));
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassRule>(this, &ClassRule::PostIdentifier));
        cminor::parsing::NonterminalParser* templateParameterListNonterminalParser = GetNonterminal(ToUtf32("TemplateParameterList"));
        templateParameterListNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassRule>(this, &ClassRule::PreTemplateParameterList));
        cminor::parsing::NonterminalParser* inheritanceAndInterfacesNonterminalParser = GetNonterminal(ToUtf32("InheritanceAndInterfaces"));
        inheritanceAndInterfacesNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassRule>(this, &ClassRule::PreInheritanceAndInterfaces));
        cminor::parsing::NonterminalParser* classContentNonterminalParser = GetNonterminal(ToUtf32("ClassContent"));
        classContentNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassRule>(this, &ClassRule::PreClassContent));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ClassNode(span, context->fromSpecifiers, context->fromIdentifier);
    }
    void PostSpecifiers(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context->fromSpecifiers = *static_cast<cminor::parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
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
    void PreTemplateParameterList(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->value)));
    }
    void PreInheritanceAndInterfaces(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ClassNode*>(context->value)));
    }
    void PreClassContent(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ClassNode*>(context->value)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), fromSpecifiers(), fromIdentifier() {}
        ParsingContext* ctx;
        ClassNode* value;
        Specifiers fromSpecifiers;
        IdentifierNode* fromIdentifier;
    };
};

class ClassGrammar::InheritanceAndInterfacesRule : public cminor::parsing::Rule
{
public:
    InheritanceAndInterfacesRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ClassNode*"), ToUtf32("classNode")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> classNode_value = std::move(stack.top());
        context->classNode = *static_cast<cminor::parsing::ValueObject<ClassNode*>*>(classNode_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InheritanceAndInterfacesRule>(this, &InheritanceAndInterfacesRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InheritanceAndInterfacesRule>(this, &InheritanceAndInterfacesRule::A1Action));
        a1ActionParser->SetFailureAction(new cminor::parsing::MemberFailureAction<InheritanceAndInterfacesRule>(this, &InheritanceAndInterfacesRule::A1ActionFail));
        cminor::parsing::NonterminalParser* baseClassOrInterfaceNonterminalParser = GetNonterminal(ToUtf32("BaseClassOrInterface"));
        baseClassOrInterfaceNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<InheritanceAndInterfacesRule>(this, &InheritanceAndInterfacesRule::PreBaseClassOrInterface));
        baseClassOrInterfaceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<InheritanceAndInterfacesRule>(this, &InheritanceAndInterfacesRule::PostBaseClassOrInterface));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ctx->BeginParsingTypeExpr();
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->classNode->AddBaseClassOrInterface(context->fromBaseClassOrInterface);
        context->ctx->EndParsingTypeExpr();
    }
    void A1ActionFail(ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ctx->EndParsingTypeExpr();
    }
    void PreBaseClassOrInterface(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostBaseClassOrInterface(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromBaseClassOrInterface_value = std::move(stack.top());
            context->fromBaseClassOrInterface = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromBaseClassOrInterface_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), classNode(), fromBaseClassOrInterface() {}
        ParsingContext* ctx;
        ClassNode* classNode;
        Node* fromBaseClassOrInterface;
    };
};

class ClassGrammar::BaseClassOrInterfaceRule : public cminor::parsing::Rule
{
public:
    BaseClassOrInterfaceRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BaseClassOrInterfaceRule>(this, &BaseClassOrInterfaceRule::A0Action));
        cminor::parsing::NonterminalParser* qualifiedIdNonterminalParser = GetNonterminal(ToUtf32("QualifiedId"));
        qualifiedIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<BaseClassOrInterfaceRule>(this, &BaseClassOrInterfaceRule::PostQualifiedId));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromQualifiedId;
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
        Context(): ctx(), value(), fromQualifiedId() {}
        ParsingContext* ctx;
        Node* value;
        IdentifierNode* fromQualifiedId;
    };
};

class ClassGrammar::ClassContentRule : public cminor::parsing::Rule
{
public:
    ClassContentRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ClassNode*"), ToUtf32("classNode")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> classNode_value = std::move(stack.top());
        context->classNode = *static_cast<cminor::parsing::ValueObject<ClassNode*>*>(classNode_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassContentRule>(this, &ClassContentRule::A0Action));
        cminor::parsing::NonterminalParser* classMemberNonterminalParser = GetNonterminal(ToUtf32("ClassMember"));
        classMemberNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassContentRule>(this, &ClassContentRule::PreClassMember));
        classMemberNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassContentRule>(this, &ClassContentRule::PostClassMember));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->classNode->AddMember(context->fromClassMember);
    }
    void PreClassMember(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ClassNode*>(context->classNode)));
    }
    void PostClassMember(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromClassMember_value = std::move(stack.top());
            context->fromClassMember = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromClassMember_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), classNode(), fromClassMember() {}
        ParsingContext* ctx;
        ClassNode* classNode;
        Node* fromClassMember;
    };
};

class ClassGrammar::ClassMemberRule : public cminor::parsing::Rule
{
public:
    ClassMemberRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ClassNode*"), ToUtf32("classNode")));
        SetValueTypeName(ToUtf32("Node*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> classNode_value = std::move(stack.top());
        context->classNode = *static_cast<cminor::parsing::ValueObject<ClassNode*>*>(classNode_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction(ToUtf32("A4"));
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction(ToUtf32("A5"));
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction(ToUtf32("A6"));
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction(ToUtf32("A7"));
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction(ToUtf32("A8"));
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A8Action));
        cminor::parsing::ActionParser* a9ActionParser = GetAction(ToUtf32("A9"));
        a9ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A9Action));
        cminor::parsing::ActionParser* a10ActionParser = GetAction(ToUtf32("A10"));
        a10ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClassMemberRule>(this, &ClassMemberRule::A10Action));
        cminor::parsing::NonterminalParser* staticConstructorNonterminalParser = GetNonterminal(ToUtf32("StaticConstructor"));
        staticConstructorNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreStaticConstructor));
        staticConstructorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostStaticConstructor));
        cminor::parsing::NonterminalParser* constructorNonterminalParser = GetNonterminal(ToUtf32("Constructor"));
        constructorNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreConstructor));
        constructorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostConstructor));
        cminor::parsing::NonterminalParser* memberFunctionNonterminalParser = GetNonterminal(ToUtf32("MemberFunction"));
        memberFunctionNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreMemberFunction));
        memberFunctionNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostMemberFunction));
        cminor::parsing::NonterminalParser* memberVariableNonterminalParser = GetNonterminal(ToUtf32("MemberVariable"));
        memberVariableNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreMemberVariable));
        memberVariableNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostMemberVariable));
        cminor::parsing::NonterminalParser* propertyNonterminalParser = GetNonterminal(ToUtf32("Property"));
        propertyNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreProperty));
        propertyNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostProperty));
        cminor::parsing::NonterminalParser* indexerNonterminalParser = GetNonterminal(ToUtf32("Indexer"));
        indexerNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreIndexer));
        indexerNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostIndexer));
        cminor::parsing::NonterminalParser* classNonterminalParser = GetNonterminal(ToUtf32("Class"));
        classNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreClass));
        classNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostClass));
        cminor::parsing::NonterminalParser* enumTypeNonterminalParser = GetNonterminal(ToUtf32("EnumType"));
        enumTypeNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreEnumType));
        enumTypeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostEnumType));
        cminor::parsing::NonterminalParser* constantNonterminalParser = GetNonterminal(ToUtf32("Constant"));
        constantNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreConstant));
        constantNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostConstant));
        cminor::parsing::NonterminalParser* delegateNonterminalParser = GetNonterminal(ToUtf32("Delegate"));
        delegateNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreDelegate));
        delegateNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostDelegate));
        cminor::parsing::NonterminalParser* classDelegateNonterminalParser = GetNonterminal(ToUtf32("ClassDelegate"));
        classDelegateNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ClassMemberRule>(this, &ClassMemberRule::PreClassDelegate));
        classDelegateNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ClassMemberRule>(this, &ClassMemberRule::PostClassDelegate));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromStaticConstructor;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromConstructor;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromMemberFunction;
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromMemberVariable;
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromProperty;
    }
    void A5Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromIndexer;
    }
    void A6Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromClass;
    }
    void A7Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromEnumType;
    }
    void A8Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromConstant;
    }
    void A9Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromDelegate;
    }
    void A10Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromClassDelegate;
    }
    void PreStaticConstructor(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ClassNode*>(context->classNode)));
    }
    void PostStaticConstructor(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromStaticConstructor_value = std::move(stack.top());
            context->fromStaticConstructor = *static_cast<cminor::parsing::ValueObject<StaticConstructorNode*>*>(fromStaticConstructor_value.get());
            stack.pop();
        }
    }
    void PreConstructor(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ClassNode*>(context->classNode)));
    }
    void PostConstructor(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromConstructor_value = std::move(stack.top());
            context->fromConstructor = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromConstructor_value.get());
            stack.pop();
        }
    }
    void PreMemberFunction(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ClassNode*>(context->classNode)));
    }
    void PostMemberFunction(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromMemberFunction_value = std::move(stack.top());
            context->fromMemberFunction = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromMemberFunction_value.get());
            stack.pop();
        }
    }
    void PreMemberVariable(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostMemberVariable(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromMemberVariable_value = std::move(stack.top());
            context->fromMemberVariable = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromMemberVariable_value.get());
            stack.pop();
        }
    }
    void PreProperty(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostProperty(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromProperty_value = std::move(stack.top());
            context->fromProperty = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromProperty_value.get());
            stack.pop();
        }
    }
    void PreIndexer(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostIndexer(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIndexer_value = std::move(stack.top());
            context->fromIndexer = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromIndexer_value.get());
            stack.pop();
        }
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
};

class ClassGrammar::StaticConstructorRule : public cminor::parsing::Rule
{
public:
    StaticConstructorRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ClassNode*"), ToUtf32("classNode")));
        SetValueTypeName(ToUtf32("StaticConstructorNode*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("AttributeMap"), ToUtf32("attributeMap")));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<IdentifierNode>"), ToUtf32("id")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> classNode_value = std::move(stack.top());
        context->classNode = *static_cast<cminor::parsing::ValueObject<ClassNode*>*>(classNode_value.get());
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<StaticConstructorNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StaticConstructorRule>(this, &StaticConstructorRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StaticConstructorRule>(this, &StaticConstructorRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StaticConstructorRule>(this, &StaticConstructorRule::A2Action));
        cminor::parsing::NonterminalParser* attributesNonterminalParser = GetNonterminal(ToUtf32("Attributes"));
        attributesNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<StaticConstructorRule>(this, &StaticConstructorRule::PreAttributes));
        cminor::parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal(ToUtf32("Specifiers"));
        specifiersNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StaticConstructorRule>(this, &StaticConstructorRule::PostSpecifiers));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("Identifier"));
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StaticConstructorRule>(this, &StaticConstructorRule::PostIdentifier));
        cminor::parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal(ToUtf32("CompoundStatement"));
        compoundStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<StaticConstructorRule>(this, &StaticConstructorRule::PreCompoundStatement));
        compoundStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StaticConstructorRule>(this, &StaticConstructorRule::PostCompoundStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (context->id->Str() != context->classNode->Id()->Str()) pass = false;
        else if (!HasStaticSpecifier(context->fromSpecifiers)) pass = false;
        else
        {
            context->value = new StaticConstructorNode(span, context->fromSpecifiers);
            context->value->SetAttributes(context->attributeMap);
        }
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->id.reset(context->fromIdentifier);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value->SetBody(context->fromCompoundStatement);
    }
    void PreAttributes(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<AttributeMap*>(&context->attributeMap)));
    }
    void PostSpecifiers(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context->fromSpecifiers = *static_cast<cminor::parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
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
    void PreCompoundStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostCompoundStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context->fromCompoundStatement = *static_cast<cminor::parsing::ValueObject<CompoundStatementNode*>*>(fromCompoundStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
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
};

class ClassGrammar::ConstructorRule : public cminor::parsing::Rule
{
public:
    ConstructorRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ClassNode*"), ToUtf32("classNode")));
        SetValueTypeName(ToUtf32("Node*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("AttributeMap"), ToUtf32("attributeMap")));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<IdentifierNode>"), ToUtf32("id")));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<ConstructorNode>"), ToUtf32("ctor")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> classNode_value = std::move(stack.top());
        context->classNode = *static_cast<cminor::parsing::ValueObject<ClassNode*>*>(classNode_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConstructorRule>(this, &ConstructorRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConstructorRule>(this, &ConstructorRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConstructorRule>(this, &ConstructorRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConstructorRule>(this, &ConstructorRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction(ToUtf32("A4"));
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConstructorRule>(this, &ConstructorRule::A4Action));
        cminor::parsing::NonterminalParser* attributesNonterminalParser = GetNonterminal(ToUtf32("Attributes"));
        attributesNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ConstructorRule>(this, &ConstructorRule::PreAttributes));
        cminor::parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal(ToUtf32("Specifiers"));
        specifiersNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConstructorRule>(this, &ConstructorRule::PostSpecifiers));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("Identifier"));
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConstructorRule>(this, &ConstructorRule::PostIdentifier));
        cminor::parsing::NonterminalParser* parameterListNonterminalParser = GetNonterminal(ToUtf32("ParameterList"));
        parameterListNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ConstructorRule>(this, &ConstructorRule::PreParameterList));
        cminor::parsing::NonterminalParser* initializerNonterminalParser = GetNonterminal(ToUtf32("Initializer"));
        initializerNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ConstructorRule>(this, &ConstructorRule::PreInitializer));
        initializerNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConstructorRule>(this, &ConstructorRule::PostInitializer));
        cminor::parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal(ToUtf32("CompoundStatement"));
        compoundStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<ConstructorRule>(this, &ConstructorRule::PreCompoundStatement));
        compoundStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<ConstructorRule>(this, &ConstructorRule::PostCompoundStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (context->id->Str() != context->classNode->Id()->Str()) pass = false;
        else
        {
            context->ctor.reset(new ConstructorNode(span, context->fromSpecifiers));
            context->ctor->SetAttributes(context->attributeMap);
        }
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->id.reset(context->fromIdentifier);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ctor->SetInitializer(context->fromInitializer);
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->ctor.release();
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ctor->SetBody(context->fromCompoundStatement);
    }
    void PreAttributes(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<AttributeMap*>(&context->attributeMap)));
    }
    void PostSpecifiers(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context->fromSpecifiers = *static_cast<cminor::parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
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
    void PreParameterList(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->ctor.get())));
    }
    void PreInitializer(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostInitializer(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromInitializer_value = std::move(stack.top());
            context->fromInitializer = *static_cast<cminor::parsing::ValueObject<InitializerNode*>*>(fromInitializer_value.get());
            stack.pop();
        }
    }
    void PreCompoundStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostCompoundStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context->fromCompoundStatement = *static_cast<cminor::parsing::ValueObject<CompoundStatementNode*>*>(fromCompoundStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
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
};

class ClassGrammar::InitializerRule : public cminor::parsing::Rule
{
public:
    InitializerRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("InitializerNode*"));
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
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<InitializerNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitializerRule>(this, &InitializerRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitializerRule>(this, &InitializerRule::A1Action));
        cminor::parsing::NonterminalParser* thisArgsNonterminalParser = GetNonterminal(ToUtf32("thisArgs"));
        thisArgsNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<InitializerRule>(this, &InitializerRule::PrethisArgs));
        cminor::parsing::NonterminalParser* baseArgsNonterminalParser = GetNonterminal(ToUtf32("baseArgs"));
        baseArgsNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<InitializerRule>(this, &InitializerRule::PrebaseArgs));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ThisInitializerNode(span);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BaseInitializerNode(span);
    }
    void PrethisArgs(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->value)));
    }
    void PrebaseArgs(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->value)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value() {}
        ParsingContext* ctx;
        InitializerNode* value;
    };
};

class ClassGrammar::MemberFunctionRule : public cminor::parsing::Rule
{
public:
    MemberFunctionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ClassNode*"), ToUtf32("classNode")));
        SetValueTypeName(ToUtf32("Node*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("AttributeMap"), ToUtf32("attributeMap")));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<MemberFunctionNode>"), ToUtf32("memFun")));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<IdentifierNode>"), ToUtf32("qid")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> classNode_value = std::move(stack.top());
        context->classNode = *static_cast<cminor::parsing::ValueObject<ClassNode*>*>(classNode_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MemberFunctionRule>(this, &MemberFunctionRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MemberFunctionRule>(this, &MemberFunctionRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MemberFunctionRule>(this, &MemberFunctionRule::A2Action));
        cminor::parsing::NonterminalParser* attributesNonterminalParser = GetNonterminal(ToUtf32("Attributes"));
        attributesNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<MemberFunctionRule>(this, &MemberFunctionRule::PreAttributes));
        cminor::parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal(ToUtf32("Specifiers"));
        specifiersNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<MemberFunctionRule>(this, &MemberFunctionRule::PostSpecifiers));
        cminor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal(ToUtf32("TypeExpr"));
        typeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<MemberFunctionRule>(this, &MemberFunctionRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<MemberFunctionRule>(this, &MemberFunctionRule::PostTypeExpr));
        cminor::parsing::NonterminalParser* functionGroupIdNonterminalParser = GetNonterminal(ToUtf32("FunctionGroupId"));
        functionGroupIdNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<MemberFunctionRule>(this, &MemberFunctionRule::PreFunctionGroupId));
        functionGroupIdNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<MemberFunctionRule>(this, &MemberFunctionRule::PostFunctionGroupId));
        cminor::parsing::NonterminalParser* parameterListNonterminalParser = GetNonterminal(ToUtf32("ParameterList"));
        parameterListNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<MemberFunctionRule>(this, &MemberFunctionRule::PreParameterList));
        cminor::parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal(ToUtf32("CompoundStatement"));
        compoundStatementNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<MemberFunctionRule>(this, &MemberFunctionRule::PreCompoundStatement));
        compoundStatementNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<MemberFunctionRule>(this, &MemberFunctionRule::PostCompoundStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->memFun.reset(new MemberFunctionNode(span, context->fromSpecifiers, context->fromTypeExpr, context->fromFunctionGroupId));
        context->memFun->SetAttributes(context->attributeMap);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->memFun.release();
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->memFun->SetBody(context->fromCompoundStatement);
    }
    void PreAttributes(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<AttributeMap*>(&context->attributeMap)));
    }
    void PostSpecifiers(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context->fromSpecifiers = *static_cast<cminor::parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
    }
    void PreTypeExpr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostTypeExpr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context->fromTypeExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
            stack.pop();
        }
    }
    void PreFunctionGroupId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostFunctionGroupId(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFunctionGroupId_value = std::move(stack.top());
            context->fromFunctionGroupId = *static_cast<cminor::parsing::ValueObject<FunctionGroupIdNode*>*>(fromFunctionGroupId_value.get());
            stack.pop();
        }
    }
    void PreParameterList(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->memFun.get())));
    }
    void PreCompoundStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostCompoundStatement(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context->fromCompoundStatement = *static_cast<cminor::parsing::ValueObject<CompoundStatementNode*>*>(fromCompoundStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
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
};

class ClassGrammar::MemberVariableRule : public cminor::parsing::Rule
{
public:
    MemberVariableRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MemberVariableRule>(this, &MemberVariableRule::A0Action));
        cminor::parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal(ToUtf32("Specifiers"));
        specifiersNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<MemberVariableRule>(this, &MemberVariableRule::PostSpecifiers));
        cminor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal(ToUtf32("TypeExpr"));
        typeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<MemberVariableRule>(this, &MemberVariableRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<MemberVariableRule>(this, &MemberVariableRule::PostTypeExpr));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("Identifier"));
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<MemberVariableRule>(this, &MemberVariableRule::PostIdentifier));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new MemberVariableNode(span, context->fromSpecifiers, context->fromTypeExpr, context->fromIdentifier);
    }
    void PostSpecifiers(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context->fromSpecifiers = *static_cast<cminor::parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
    }
    void PreTypeExpr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostTypeExpr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context->fromTypeExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
            stack.pop();
        }
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
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), fromSpecifiers(), fromTypeExpr(), fromIdentifier() {}
        ParsingContext* ctx;
        Node* value;
        Specifiers fromSpecifiers;
        Node* fromTypeExpr;
        IdentifierNode* fromIdentifier;
    };
};

class ClassGrammar::PropertyRule : public cminor::parsing::Rule
{
public:
    PropertyRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("Node*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<Node>"), ToUtf32("property")));
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PropertyRule>(this, &PropertyRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PropertyRule>(this, &PropertyRule::A1Action));
        cminor::parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal(ToUtf32("Specifiers"));
        specifiersNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PropertyRule>(this, &PropertyRule::PostSpecifiers));
        cminor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal(ToUtf32("TypeExpr"));
        typeExprNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PropertyRule>(this, &PropertyRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PropertyRule>(this, &PropertyRule::PostTypeExpr));
        cminor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("Identifier"));
        identifierNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PropertyRule>(this, &PropertyRule::PostIdentifier));
        cminor::parsing::NonterminalParser* getterOrSetterNonterminalParser = GetNonterminal(ToUtf32("GetterOrSetter"));
        getterOrSetterNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<PropertyRule>(this, &PropertyRule::PreGetterOrSetter));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->property.release();
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->property.reset(new PropertyNode(span, context->fromSpecifiers, context->fromTypeExpr, context->fromIdentifier));
    }
    void PostSpecifiers(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context->fromSpecifiers = *static_cast<cminor::parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
    }
    void PreTypeExpr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostTypeExpr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context->fromTypeExpr = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
            stack.pop();
        }
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
    void PreGetterOrSetter(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->property.get())));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), value(), property(), fromSpecifiers(), fromTypeExpr(), fromIdentifier() {}
        ParsingContext* ctx;
        Node* value;
        std::unique_ptr<Node> property;
        Specifiers fromSpecifiers;
        Node* fromTypeExpr;
        IdentifierNode* fromIdentifier;
    };
};

class ClassGrammar::IndexerRule : public cminor::parsing::Rule
{
public:
    IndexerRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("Node*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<Node>"), ToUtf32("indexer")));
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IndexerRule>(this, &IndexerRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IndexerRule>(this, &IndexerRule::A1Action));
        cminor::parsing::NonterminalParser* specifiersNonterminalParser = GetNonterminal(ToUtf32("Specifiers"));
        specifiersNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IndexerRule>(this, &IndexerRule::PostSpecifiers));
        cminor::parsing::NonterminalParser* valueTypeNonterminalParser = GetNonterminal(ToUtf32("valueType"));
        valueTypeNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<IndexerRule>(this, &IndexerRule::PrevalueType));
        valueTypeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IndexerRule>(this, &IndexerRule::PostvalueType));
        cminor::parsing::NonterminalParser* indexTypeNonterminalParser = GetNonterminal(ToUtf32("indexType"));
        indexTypeNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<IndexerRule>(this, &IndexerRule::PreindexType));
        indexTypeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IndexerRule>(this, &IndexerRule::PostindexType));
        cminor::parsing::NonterminalParser* indexNonterminalParser = GetNonterminal(ToUtf32("index"));
        indexNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<IndexerRule>(this, &IndexerRule::Postindex));
        cminor::parsing::NonterminalParser* getterOrSetterNonterminalParser = GetNonterminal(ToUtf32("GetterOrSetter"));
        getterOrSetterNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<IndexerRule>(this, &IndexerRule::PreGetterOrSetter));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->indexer.release();
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->indexer.reset(new IndexerNode(span, context->fromSpecifiers, context->fromvalueType, context->fromindexType, context->fromindex));
    }
    void PostSpecifiers(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpecifiers_value = std::move(stack.top());
            context->fromSpecifiers = *static_cast<cminor::parsing::ValueObject<Specifiers>*>(fromSpecifiers_value.get());
            stack.pop();
        }
    }
    void PrevalueType(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostvalueType(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromvalueType_value = std::move(stack.top());
            context->fromvalueType = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromvalueType_value.get());
            stack.pop();
        }
    }
    void PreindexType(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostindexType(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromindexType_value = std::move(stack.top());
            context->fromindexType = *static_cast<cminor::parsing::ValueObject<Node*>*>(fromindexType_value.get());
            stack.pop();
        }
    }
    void Postindex(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromindex_value = std::move(stack.top());
            context->fromindex = *static_cast<cminor::parsing::ValueObject<IdentifierNode*>*>(fromindex_value.get());
            stack.pop();
        }
    }
    void PreGetterOrSetter(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<Node*>(context->indexer.get())));
    }
private:
    struct Context : cminor::parsing::Context
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
};

class ClassGrammar::GetterOrSetterRule : public cminor::parsing::Rule
{
public:
    GetterOrSetterRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        AddInheritedAttribute(AttrOrVariable(ToUtf32("Node*"), ToUtf32("subject")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cminor::parsing::Object> subject_value = std::move(stack.top());
        context->subject = *static_cast<cminor::parsing::ValueObject<Node*>*>(subject_value.get());
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
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<GetterOrSetterRule>(this, &GetterOrSetterRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<GetterOrSetterRule>(this, &GetterOrSetterRule::A1Action));
        cminor::parsing::NonterminalParser* getterNonterminalParser = GetNonterminal(ToUtf32("getter"));
        getterNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<GetterOrSetterRule>(this, &GetterOrSetterRule::Pregetter));
        getterNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<GetterOrSetterRule>(this, &GetterOrSetterRule::Postgetter));
        cminor::parsing::NonterminalParser* setterNonterminalParser = GetNonterminal(ToUtf32("setter"));
        setterNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<GetterOrSetterRule>(this, &GetterOrSetterRule::Presetter));
        setterNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<GetterOrSetterRule>(this, &GetterOrSetterRule::Postsetter));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->subject->SetGetter(context->fromgetter);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->subject->SetSetter(context->fromsetter);
    }
    void Pregetter(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void Postgetter(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromgetter_value = std::move(stack.top());
            context->fromgetter = *static_cast<cminor::parsing::ValueObject<CompoundStatementNode*>*>(fromgetter_value.get());
            stack.pop();
        }
    }
    void Presetter(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void Postsetter(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromsetter_value = std::move(stack.top());
            context->fromsetter = *static_cast<cminor::parsing::ValueObject<CompoundStatementNode*>*>(fromsetter_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): ctx(), subject(), fromgetter(), fromsetter() {}
        ParsingContext* ctx;
        Node* subject;
        CompoundStatementNode* fromgetter;
        CompoundStatementNode* fromsetter;
    };
};

void ClassGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cminor.parser.TypeExprGrammar"));
    if (!grammar0)
    {
        grammar0 = cminor::parser::TypeExprGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar(ToUtf32("cminor.parser.SpecifierGrammar"));
    if (!grammar1)
    {
        grammar1 = cminor::parser::SpecifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cminor::parsing::Grammar* grammar2 = pd->GetGrammar(ToUtf32("cminor.parser.ConstantGrammar"));
    if (!grammar2)
    {
        grammar2 = cminor::parser::ConstantGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cminor::parsing::Grammar* grammar3 = pd->GetGrammar(ToUtf32("cminor.parser.IdentifierGrammar"));
    if (!grammar3)
    {
        grammar3 = cminor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
    cminor::parsing::Grammar* grammar4 = pd->GetGrammar(ToUtf32("cminor.parser.TemplateGrammar"));
    if (!grammar4)
    {
        grammar4 = cminor::parser::TemplateGrammar::Create(pd);
    }
    AddGrammarReference(grammar4);
    cminor::parsing::Grammar* grammar5 = pd->GetGrammar(ToUtf32("cminor.parser.FunctionGrammar"));
    if (!grammar5)
    {
        grammar5 = cminor::parser::FunctionGrammar::Create(pd);
    }
    AddGrammarReference(grammar5);
    cminor::parsing::Grammar* grammar6 = pd->GetGrammar(ToUtf32("cminor.parser.ParameterGrammar"));
    if (!grammar6)
    {
        grammar6 = cminor::parser::ParameterGrammar::Create(pd);
    }
    AddGrammarReference(grammar6);
    cminor::parsing::Grammar* grammar7 = pd->GetGrammar(ToUtf32("cminor.parser.DelegateGrammar"));
    if (!grammar7)
    {
        grammar7 = cminor::parser::DelegateGrammar::Create(pd);
    }
    AddGrammarReference(grammar7);
    cminor::parsing::Grammar* grammar8 = pd->GetGrammar(ToUtf32("cminor.parser.EnumerationGrammar"));
    if (!grammar8)
    {
        grammar8 = cminor::parser::EnumerationGrammar::Create(pd);
    }
    AddGrammarReference(grammar8);
    cminor::parsing::Grammar* grammar9 = pd->GetGrammar(ToUtf32("cminor.parser.ExpressionGrammar"));
    if (!grammar9)
    {
        grammar9 = cminor::parser::ExpressionGrammar::Create(pd);
    }
    AddGrammarReference(grammar9);
    cminor::parsing::Grammar* grammar10 = pd->GetGrammar(ToUtf32("cminor.parser.StatementGrammar"));
    if (!grammar10)
    {
        grammar10 = cminor::parser::StatementGrammar::Create(pd);
    }
    AddGrammarReference(grammar10);
    cminor::parsing::Grammar* grammar11 = pd->GetGrammar(ToUtf32("cminor.parsing.stdlib"));
    if (!grammar11)
    {
        grammar11 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar11);
}

void ClassGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Specifiers"), this, ToUtf32("SpecifierGrammar.Specifiers")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Constant"), this, ToUtf32("ConstantGrammar.Constant")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("TemplateParameterList"), this, ToUtf32("TemplateGrammar.TemplateParameterList")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("ParameterList"), this, ToUtf32("ParameterGrammar.ParameterList")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Attributes"), this, ToUtf32("FunctionGrammar.Attributes")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("EnumType"), this, ToUtf32("EnumerationGrammar.EnumType")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Identifier"), this, ToUtf32("IdentifierGrammar.Identifier")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("FunctionGroupId"), this, ToUtf32("FunctionGrammar.FunctionGroupId")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("QualifiedId"), this, ToUtf32("IdentifierGrammar.QualifiedId")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("TypeExpr"), this, ToUtf32("TypeExprGrammar.TypeExpr")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("ClassDelegate"), this, ToUtf32("DelegateGrammar.ClassDelegate")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("ArgumentList"), this, ToUtf32("ExpressionGrammar.ArgumentList")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("Delegate"), this, ToUtf32("DelegateGrammar.Delegate")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("CompoundStatement"), this, ToUtf32("StatementGrammar.CompoundStatement")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("identifier"), this, ToUtf32("cminor.parsing.stdlib.identifier")));
    AddRule(new ClassRule(ToUtf32("Class"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::NonterminalParser(ToUtf32("Specifiers"), ToUtf32("Specifiers"), 0),
                                    new cminor::parsing::KeywordParser(ToUtf32("class"))),
                                new cminor::parsing::ActionParser(ToUtf32("A0"),
                                    new cminor::parsing::NonterminalParser(ToUtf32("Identifier"), ToUtf32("Identifier"), 0))),
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("TemplateParameterList"), ToUtf32("TemplateParameterList"), 2))),
                        new cminor::parsing::OptionalParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("InheritanceAndInterfaces"), ToUtf32("InheritanceAndInterfaces"), 2))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('{'))),
                new cminor::parsing::NonterminalParser(ToUtf32("ClassContent"), ToUtf32("ClassContent"), 2)),
            new cminor::parsing::ExpectationParser(
                new cminor::parsing::CharParser('}')))));
    AddRule(new InheritanceAndInterfacesRule(ToUtf32("InheritanceAndInterfaces"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::CharParser(':'),
            new cminor::parsing::ListParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::ActionParser(ToUtf32("A0"),
                        new cminor::parsing::EmptyParser()),
                    new cminor::parsing::ActionParser(ToUtf32("A1"),
                        new cminor::parsing::NonterminalParser(ToUtf32("BaseClassOrInterface"), ToUtf32("BaseClassOrInterface"), 1))),
                new cminor::parsing::CharParser(',')))));
    AddRule(new BaseClassOrInterfaceRule(ToUtf32("BaseClassOrInterface"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::NonterminalParser(ToUtf32("QualifiedId"), ToUtf32("QualifiedId"), 0))));
    AddRule(new ClassContentRule(ToUtf32("ClassContent"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::KleeneStarParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::NonterminalParser(ToUtf32("ClassMember"), ToUtf32("ClassMember"), 2)))));
    AddRule(new ClassMemberRule(ToUtf32("ClassMember"), GetScope(), GetParsingDomain()->GetNextRuleId(),
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
                                                new cminor::parsing::ActionParser(ToUtf32("A0"),
                                                    new cminor::parsing::NonterminalParser(ToUtf32("StaticConstructor"), ToUtf32("StaticConstructor"), 2)),
                                                new cminor::parsing::ActionParser(ToUtf32("A1"),
                                                    new cminor::parsing::NonterminalParser(ToUtf32("Constructor"), ToUtf32("Constructor"), 2))),
                                            new cminor::parsing::ActionParser(ToUtf32("A2"),
                                                new cminor::parsing::NonterminalParser(ToUtf32("MemberFunction"), ToUtf32("MemberFunction"), 2))),
                                        new cminor::parsing::ActionParser(ToUtf32("A3"),
                                            new cminor::parsing::NonterminalParser(ToUtf32("MemberVariable"), ToUtf32("MemberVariable"), 1))),
                                    new cminor::parsing::ActionParser(ToUtf32("A4"),
                                        new cminor::parsing::NonterminalParser(ToUtf32("Property"), ToUtf32("Property"), 1))),
                                new cminor::parsing::ActionParser(ToUtf32("A5"),
                                    new cminor::parsing::NonterminalParser(ToUtf32("Indexer"), ToUtf32("Indexer"), 1))),
                            new cminor::parsing::ActionParser(ToUtf32("A6"),
                                new cminor::parsing::NonterminalParser(ToUtf32("Class"), ToUtf32("Class"), 1))),
                        new cminor::parsing::ActionParser(ToUtf32("A7"),
                            new cminor::parsing::NonterminalParser(ToUtf32("EnumType"), ToUtf32("EnumType"), 1))),
                    new cminor::parsing::ActionParser(ToUtf32("A8"),
                        new cminor::parsing::NonterminalParser(ToUtf32("Constant"), ToUtf32("Constant"), 1))),
                new cminor::parsing::ActionParser(ToUtf32("A9"),
                    new cminor::parsing::NonterminalParser(ToUtf32("Delegate"), ToUtf32("Delegate"), 1))),
            new cminor::parsing::ActionParser(ToUtf32("A10"),
                new cminor::parsing::NonterminalParser(ToUtf32("ClassDelegate"), ToUtf32("ClassDelegate"), 1)))));
    AddRule(new StaticConstructorRule(ToUtf32("StaticConstructor"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::OptionalParser(
                                    new cminor::parsing::NonterminalParser(ToUtf32("Attributes"), ToUtf32("Attributes"), 1)),
                                new cminor::parsing::NonterminalParser(ToUtf32("Specifiers"), ToUtf32("Specifiers"), 0)),
                            new cminor::parsing::ActionParser(ToUtf32("A1"),
                                new cminor::parsing::NonterminalParser(ToUtf32("Identifier"), ToUtf32("Identifier"), 0))),
                        new cminor::parsing::CharParser('(')),
                    new cminor::parsing::CharParser(')'))),
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::ActionParser(ToUtf32("A2"),
                    new cminor::parsing::NonterminalParser(ToUtf32("CompoundStatement"), ToUtf32("CompoundStatement"), 1)),
                new cminor::parsing::CharParser(';')))));
    AddRule(new ConstructorRule(ToUtf32("Constructor"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::ActionParser(ToUtf32("A0"),
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::OptionalParser(
                                    new cminor::parsing::NonterminalParser(ToUtf32("Attributes"), ToUtf32("Attributes"), 1)),
                                new cminor::parsing::NonterminalParser(ToUtf32("Specifiers"), ToUtf32("Specifiers"), 0)),
                            new cminor::parsing::ActionParser(ToUtf32("A1"),
                                new cminor::parsing::NonterminalParser(ToUtf32("Identifier"), ToUtf32("Identifier"), 0)))),
                    new cminor::parsing::NonterminalParser(ToUtf32("ParameterList"), ToUtf32("ParameterList"), 2)),
                new cminor::parsing::OptionalParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::CharParser(':'),
                        new cminor::parsing::ActionParser(ToUtf32("A2"),
                            new cminor::parsing::NonterminalParser(ToUtf32("Initializer"), ToUtf32("Initializer"), 1))))),
            new cminor::parsing::ActionParser(ToUtf32("A3"),
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::ActionParser(ToUtf32("A4"),
                        new cminor::parsing::NonterminalParser(ToUtf32("CompoundStatement"), ToUtf32("CompoundStatement"), 1)),
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new InitializerRule(ToUtf32("Initializer"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser(ToUtf32("A0"),
                            new cminor::parsing::KeywordParser(ToUtf32("this"), ToUtf32("identifier"))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser('('))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("thisArgs"), ToUtf32("ArgumentList"), 2))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser(ToUtf32("A1"),
                            new cminor::parsing::KeywordParser(ToUtf32("base"), ToUtf32("identifier"))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser('('))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("baseArgs"), ToUtf32("ArgumentList"), 2))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))))));
    AddRule(new MemberFunctionRule(ToUtf32("MemberFunction"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::OptionalParser(
                                    new cminor::parsing::NonterminalParser(ToUtf32("Attributes"), ToUtf32("Attributes"), 1)),
                                new cminor::parsing::NonterminalParser(ToUtf32("Specifiers"), ToUtf32("Specifiers"), 0)),
                            new cminor::parsing::NonterminalParser(ToUtf32("TypeExpr"), ToUtf32("TypeExpr"), 1)),
                        new cminor::parsing::NonterminalParser(ToUtf32("FunctionGroupId"), ToUtf32("FunctionGroupId"), 1))),
                new cminor::parsing::NonterminalParser(ToUtf32("ParameterList"), ToUtf32("ParameterList"), 2)),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::ActionParser(ToUtf32("A2"),
                        new cminor::parsing::NonterminalParser(ToUtf32("CompoundStatement"), ToUtf32("CompoundStatement"), 1)),
                    new cminor::parsing::CharParser(';'))))));
    AddRule(new MemberVariableRule(ToUtf32("MemberVariable"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("Specifiers"), ToUtf32("Specifiers"), 0),
                        new cminor::parsing::NonterminalParser(ToUtf32("TypeExpr"), ToUtf32("TypeExpr"), 1)),
                    new cminor::parsing::NonterminalParser(ToUtf32("Identifier"), ToUtf32("Identifier"), 0)),
                new cminor::parsing::CharParser(';')))));
    AddRule(new PropertyRule(ToUtf32("Property"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser(ToUtf32("A1"),
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::NonterminalParser(ToUtf32("Specifiers"), ToUtf32("Specifiers"), 0),
                                    new cminor::parsing::NonterminalParser(ToUtf32("TypeExpr"), ToUtf32("TypeExpr"), 1)),
                                new cminor::parsing::NonterminalParser(ToUtf32("Identifier"), ToUtf32("Identifier"), 0))),
                        new cminor::parsing::CharParser('{')),
                    new cminor::parsing::PositiveParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("GetterOrSetter"), ToUtf32("GetterOrSetter"), 2))),
                new cminor::parsing::CharParser('}')))));
    AddRule(new IndexerRule(ToUtf32("Indexer"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::ActionParser(ToUtf32("A1"),
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::SequenceParser(
                                    new cminor::parsing::SequenceParser(
                                        new cminor::parsing::SequenceParser(
                                            new cminor::parsing::SequenceParser(
                                                new cminor::parsing::SequenceParser(
                                                    new cminor::parsing::NonterminalParser(ToUtf32("Specifiers"), ToUtf32("Specifiers"), 0),
                                                    new cminor::parsing::NonterminalParser(ToUtf32("valueType"), ToUtf32("TypeExpr"), 1)),
                                                new cminor::parsing::KeywordParser(ToUtf32("this"), ToUtf32("identifier"))),
                                            new cminor::parsing::ExpectationParser(
                                                new cminor::parsing::CharParser('['))),
                                        new cminor::parsing::ExpectationParser(
                                            new cminor::parsing::NonterminalParser(ToUtf32("indexType"), ToUtf32("TypeExpr"), 1))),
                                    new cminor::parsing::ExpectationParser(
                                        new cminor::parsing::NonterminalParser(ToUtf32("index"), ToUtf32("Identifier"), 0))),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::CharParser(']')))),
                        new cminor::parsing::CharParser('{')),
                    new cminor::parsing::PositiveParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("GetterOrSetter"), ToUtf32("GetterOrSetter"), 2))),
                new cminor::parsing::CharParser('}')))));
    AddRule(new GetterOrSetterRule(ToUtf32("GetterOrSetter"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser(ToUtf32("get")),
                new cminor::parsing::ActionParser(ToUtf32("A0"),
                    new cminor::parsing::NonterminalParser(ToUtf32("getter"), ToUtf32("CompoundStatement"), 1))),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::KeywordParser(ToUtf32("set")),
                new cminor::parsing::ActionParser(ToUtf32("A1"),
                    new cminor::parsing::NonterminalParser(ToUtf32("setter"), ToUtf32("CompoundStatement"), 1))))));
}

} } // namespace cminor.parser
