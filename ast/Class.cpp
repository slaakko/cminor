// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Class.hpp>
#include <cminor/ast/Statement.hpp>
#include <cminor/ast/Visitor.hpp>

namespace cminor { namespace ast {

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

Node* ClassNode::Clone(CloneContext& cloneContext) const
{
    ClassNode* clone = new ClassNode(GetSpan(), specifiers, static_cast<IdentifierNode*>(id->Clone(cloneContext)));
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

void ClassNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
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
    clone->SetBody(static_cast<CompoundStatementNode*>(Body()->Clone(cloneContext)));
    return clone;
}

void StaticConstructorNode::Accept(Visitor& visitor)
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
    clone->SetBody(static_cast<CompoundStatementNode*>(Body()->Clone(cloneContext)));
    return clone;
}

void ConstructorNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
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
    clone->SetBody(static_cast<CompoundStatementNode*>(Body()->Clone(cloneContext)));
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

void MemberVariableNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::ast
