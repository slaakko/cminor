// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Class.hpp>
#include <cminor/ast/Statement.hpp>
#include <cminor/ast/Visitor.hpp>
#include <cminor/machine/Error.hpp>

namespace cminor { namespace ast {

using namespace cminor::machine;

ClassNode::ClassNode(const Span& span_) : Node(span_)
{
}

ClassNode::ClassNode(const Span& span_, Specifiers specifiers_, IdentifierNode* id_) : Node(span_), specifiers(specifiers_), id(id_)
{
    id->SetParent(this);
}

void ClassNode::AddBaseClassOrInterface(Node* baseClassOrInterface)
{
    baseClassOrInterface->SetParent(this);
    baseClassOrInterfaces.Add(baseClassOrInterface);
}

void ClassNode::AddMember(Node* member) 
{
    member->SetParent(this);
    members.Add(member);
}

void ClassNode::AddTemplateParameter(TemplateParameterNode* templateParameter)
{
    templateParameter->SetParent(this);
    templateParameters.Add(templateParameter);
}

Node* ClassNode::Clone(CloneContext& cloneContext) const
{
    ClassNode* clone = new ClassNode(GetSpan(), specifiers, static_cast<IdentifierNode*>(id->Clone(cloneContext)));
    if (!cloneContext.InstantiateClassNode())
    {
        int n = templateParameters.Count();
        for (int i = 0; i < n; ++i)
        {
            clone->AddTemplateParameter(static_cast<TemplateParameterNode*>(templateParameters[i]->Clone(cloneContext)));
        }
    }
    int nb = baseClassOrInterfaces.Count();
    for (int i = 0; i < nb; ++i)
    {
        Node* baseClassOrInterface = baseClassOrInterfaces[i];
        clone->AddBaseClassOrInterface(baseClassOrInterface->Clone(cloneContext));
    }
    int nm = members.Count();
    for (int i = 0; i < nm; ++i)
    {
        Node* member = members[i];
        clone->AddMember(member->Clone(cloneContext));
    }
    return clone;
}

void ClassNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Put(specifiers);
    writer.Put(id.get());
    templateParameters.Write(writer);
    baseClassOrInterfaces.Write(writer);
    members.Write(writer);
}

void ClassNode::Read(AstReader& reader)
{
    Node::Read(reader);
    specifiers = reader.GetSpecifiers();
    id.reset(reader.GetIdentifierNode());
    id->SetParent(this);
    templateParameters.Read(reader);
    templateParameters.SetParent(this);
    baseClassOrInterfaces.Read(reader);
    baseClassOrInterfaces.SetParent(this);
    members.Read(reader);
    members.SetParent(this);
}

void ClassNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void ClassNode::SetId(IdentifierNode* id_)
{
    id.reset(id_);
}

StaticConstructorNode::StaticConstructorNode(const Span& span_) : FunctionNode(span_)
{
}

StaticConstructorNode::StaticConstructorNode(const Span& span_, Specifiers specifiers_) : FunctionNode(span_, specifiers_, nullptr, new FunctionGroupIdNode(span_, "@static_constructor"))
{
}

Node* StaticConstructorNode::Clone(CloneContext& cloneContext) const
{
    StaticConstructorNode* clone = new StaticConstructorNode(GetSpan(), GetSpecifiers());
    if (Body())
    {
        if (!cloneContext.InstantiateClassNode())
        {
            clone->SetBodySource(static_cast<CompoundStatementNode*>(Body()->Clone(cloneContext)));
        }
        else
        {
            clone->SetBody(static_cast<CompoundStatementNode*>(Body()->Clone(cloneContext)));
        }
    }
    return clone;
}

void StaticConstructorNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

InitializerNode::InitializerNode(const Span& span_) : Node(span_)
{
}

void InitializerNode::AddArgument(Node* argument)
{
    argument->SetParent(this);
    arguments.Add(argument);
}

void InitializerNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    arguments.Write(writer);
}

void InitializerNode::Read(AstReader& reader)
{
    Node::Read(reader);
    arguments.Read(reader);
    arguments.SetParent(this);
}


BaseInitializerNode::BaseInitializerNode(const Span& span_) : InitializerNode(span_)
{
}

Node* BaseInitializerNode::Clone(CloneContext& cloneContext) const
{
    BaseInitializerNode* clone = new BaseInitializerNode(GetSpan());
    int n = Arguments().Count();
    for (int i = 0; i < n; ++i)
    {
        clone->AddArgument(Arguments()[i]);
    }
    return clone;
}

void BaseInitializerNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ThisInitializerNode::ThisInitializerNode(const Span& span_) : InitializerNode(span_)
{
}

Node* ThisInitializerNode::Clone(CloneContext& cloneContext) const
{
    ThisInitializerNode* clone = new ThisInitializerNode(GetSpan());
    int n = Arguments().Count();
    for (int i = 0; i < n; ++i)
    {
        clone->AddArgument(Arguments()[i]);
    }
    return clone;
}

void ThisInitializerNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ConstructorNode::ConstructorNode(const Span& span_) : FunctionNode(span_)
{
}

ConstructorNode::ConstructorNode(const Span& span_, Specifiers specifiers_) : FunctionNode(span_, specifiers_, nullptr, new FunctionGroupIdNode(span_, "@constructor"))
{
}

Node* ConstructorNode::Clone(CloneContext& cloneContext) const
{
    ConstructorNode* clone = new ConstructorNode(GetSpan(), GetSpecifiers());
    int n = Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameter = Parameters()[i];
        clone->AddParameter(static_cast<ParameterNode*>(parameter->Clone(cloneContext)));
    }
    if (initializer)
    {
        clone->SetInitializer(static_cast<InitializerNode*>(initializer->Clone(cloneContext)));
    }
    if (Body())
    {
        if (cloneContext.InstantiateClassNode())
        {
            clone->SetBodySource(static_cast<CompoundStatementNode*>(Body()->Clone(cloneContext)));
        }
        else
        {
            clone->SetBody(static_cast<CompoundStatementNode*>(Body()->Clone(cloneContext)));
        }
    }
    return clone;
}

void ConstructorNode::Write(AstWriter& writer)
{
    FunctionNode::Write(writer);
    bool hasInitializer = initializer != nullptr;
    writer.AsMachineWriter().Put(hasInitializer);
    if (hasInitializer)
    {
        writer.Put(initializer.get());
    }
}

void ConstructorNode::Read(AstReader& reader)
{
    FunctionNode::Read(reader);
    bool hasInitializer = reader.GetBool();
    if (hasInitializer)
    {
        Node* node = reader.GetNode();
        InitializerNode* initializerNode = dynamic_cast<InitializerNode*>(node);
        Assert(initializerNode, "initializer node expected");
        initializer.reset(initializerNode);
        initializer->SetParent(this);
    }
}

void ConstructorNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void ConstructorNode::SetInitializer(InitializerNode* initializer_)
{
    initializer_->SetParent(this);
    initializer.reset(initializer_);
}

MemberFunctionNode::MemberFunctionNode(const Span& span_) : FunctionNode(span_)
{
}

MemberFunctionNode::MemberFunctionNode(const Span& span_, Specifiers specifiers_, Node* returnTypeExpr_, FunctionGroupIdNode* groupId_) : FunctionNode(span_, specifiers_, returnTypeExpr_, groupId_)
{
}

Node* MemberFunctionNode::Clone(CloneContext& cloneContext) const
{
    MemberFunctionNode* clone = new MemberFunctionNode(GetSpan(), GetSpecifiers(), ReturnTypeExpr()->Clone(cloneContext), static_cast<FunctionGroupIdNode*>(GroupId()->Clone(cloneContext)));
    int n = Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameter = Parameters()[i];
        clone->AddParameter(static_cast<ParameterNode*>(parameter->Clone(cloneContext)));
    }
    if (Body())
    {
        if (cloneContext.InstantiateClassNode())
        {
            clone->SetBodySource(static_cast<CompoundStatementNode*>(Body()->Clone(cloneContext)));
        }
        else
        {
            clone->SetBody(static_cast<CompoundStatementNode*>(Body()->Clone(cloneContext)));
        }
    }
    return clone;
}

void MemberFunctionNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

MemberVariableNode::MemberVariableNode(const Span& span_) : Node(span_)
{
}

MemberVariableNode::MemberVariableNode(const Span& span_, Specifiers specifiers_, Node* typeExpr_, IdentifierNode* id_) : Node(span_), specifiers(specifiers_), typeExpr(typeExpr_), id(id_)
{
    typeExpr->SetParent(this);
    id->SetParent(this);
}

Node* MemberVariableNode::Clone(CloneContext& cloneContext) const
{
    return new MemberVariableNode(GetSpan(), specifiers, typeExpr->Clone(cloneContext), static_cast<IdentifierNode*>(id->Clone(cloneContext)));
}

void MemberVariableNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Put(specifiers);
    writer.Put(typeExpr.get());
    writer.Put(id.get());
}

void MemberVariableNode::Read(AstReader& reader)
{
    Node::Read(reader);
    specifiers = reader.GetSpecifiers();
    typeExpr.reset(reader.GetNode());
    typeExpr->SetParent(this);
    id.reset(reader.GetIdentifierNode());
    id->SetParent(this);
}

void MemberVariableNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

PropertyNode::PropertyNode(const Span& span_) : Node(span_)
{
}

PropertyNode::PropertyNode(const Span& span_, Specifiers specifiers_, Node* typeExpr_, IdentifierNode* id_) : Node(span_), specifiers(specifiers_), typeExpr(typeExpr_), id(id_)
{
    typeExpr->SetParent(this);
    id->SetParent(this);
}


Node* PropertyNode::Clone(CloneContext& cloneContext) const
{
    PropertyNode* clone = new PropertyNode(GetSpan(), specifiers, typeExpr->Clone(cloneContext), static_cast<IdentifierNode*>(id->Clone(cloneContext)));
    if (getter)
    {
        clone->SetGetter(static_cast<CompoundStatementNode*>(getter->Clone(cloneContext)));
    }
    if (setter)
    {
        clone->SetSetter(static_cast<CompoundStatementNode*>(setter->Clone(cloneContext)));
    }
    return clone;
}

void PropertyNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Put(specifiers);
    writer.Put(typeExpr.get());
    writer.Put(id.get());
    bool hasGetter = getter != nullptr;
    writer.AsMachineWriter().Put(hasGetter);
    if (hasGetter)
    {
        writer.Put(getter.get());
    }
    bool hasSetter = setter != nullptr;
    writer.AsMachineWriter().Put(hasSetter);
    if (hasSetter)
    {
        writer.Put(setter.get());
    }
}

void PropertyNode::Read(AstReader& reader)
{
    Node::Read(reader);
    specifiers = reader.GetSpecifiers();
    typeExpr.reset(reader.GetNode());
    typeExpr->SetParent(this);
    id.reset(reader.GetIdentifierNode());
    id->SetParent(this);
    bool hasGetter = reader.GetBool();
    if (hasGetter)
    {
        getter.reset(reader.GetCompoundStatementNode());
        getter->SetParent(this);
    }
    bool hasSetter = reader.GetBool();
    if (hasSetter)
    {
        setter.reset(reader.GetCompoundStatementNode());
        setter->SetParent(this);
    }
}

void PropertyNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void PropertyNode::SetGetter(CompoundStatementNode* getter_)
{
    if (getter)
    {
        throw Exception("property '" + id->Str() + "' already has a getter", GetSpan());
    }
    getter.reset(getter_);
    getter->SetParent(this);
}

void PropertyNode::SetSetter(CompoundStatementNode* setter_)
{
    if (setter)
    {
        throw Exception("property '" + id->Str() + "' already has a setter", GetSpan());
    }
    setter.reset(setter_);
    setter->SetParent(this);
}

IndexerNode::IndexerNode(const Span& span_) : Node(span_)
{
}

IndexerNode::IndexerNode(const Span& span_, Specifiers specifiers_, Node* valueTypeExpr_, Node* indexTypeExpr_, IdentifierNode* id_) : 
    Node(span_), specifiers(specifiers_), valueTypeExpr(valueTypeExpr_), indexTypeExpr(indexTypeExpr_), id(id_)
{
    valueTypeExpr->SetParent(this);
    indexTypeExpr->SetParent(this);
    id->SetParent(this);
}

Node* IndexerNode::Clone(CloneContext& cloneContext) const
{
    IndexerNode* clone = new IndexerNode(GetSpan(), specifiers, valueTypeExpr->Clone(cloneContext), indexTypeExpr->Clone(cloneContext), static_cast<IdentifierNode*>(id->Clone(cloneContext)));
    if (getter)
    {
        clone->SetGetter(static_cast<CompoundStatementNode*>(getter->Clone(cloneContext)));
    }
    if (setter)
    {
        clone->SetSetter(static_cast<CompoundStatementNode*>(setter->Clone(cloneContext)));
    }
    return clone;
}

void IndexerNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Put(specifiers);
    writer.Put(valueTypeExpr.get());
    writer.Put(indexTypeExpr.get());
    writer.Put(id.get());
    bool hasGetter = getter != nullptr;
    writer.AsMachineWriter().Put(hasGetter);
    if (hasGetter)
    {
        writer.Put(getter.get());
    }
    bool hasSetter = setter != nullptr;
    writer.AsMachineWriter().Put(hasSetter);
    if (hasSetter)
    {
        writer.Put(setter.get());
    }
}

void IndexerNode::Read(AstReader& reader)
{
    Node::Read(reader);
    specifiers = reader.GetSpecifiers();
    valueTypeExpr.reset(reader.GetNode());
    valueTypeExpr->SetParent(this);
    indexTypeExpr.reset(reader.GetNode());
    indexTypeExpr->SetParent(this);
    id.reset(reader.GetIdentifierNode());
    id->SetParent(this);
    bool hasGetter = reader.GetBool();
    if (hasGetter)
    {
        getter.reset(reader.GetCompoundStatementNode());
        getter->SetParent(this);
    }
    bool hasSetter = reader.GetBool();
    if (hasSetter)
    {
        setter.reset(reader.GetCompoundStatementNode());
        setter->SetParent(this);
    }
}

void IndexerNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void IndexerNode::SetGetter(CompoundStatementNode* getter_)
{
    if (getter)
    {
        throw Exception("property '" + id->Str() + "' already has a getter", GetSpan());
    }
    getter.reset(getter_);
    getter->SetParent(this);
}

void IndexerNode::SetSetter(CompoundStatementNode* setter_)
{
    if (setter)
    {
        throw Exception("property '" + id->Str() + "' already has a setter", GetSpan());
    }
    setter.reset(setter_);
    setter->SetParent(this);
}

} } // namespace cminor::ast
