// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Delegate.hpp>
#include <cminor/ast/Function.hpp>
#include <cminor/ast/Visitor.hpp>

namespace cminor { namespace ast {

DelegateNode::DelegateNode(const Span& span_) : Node(span_)
{
}

DelegateNode::DelegateNode(const Span& span_, Specifiers specifiers_, Node* returnTypeExpr_, IdentifierNode* id_) :
    Node(span_), specifiers(specifiers_), returnTypeExpr(returnTypeExpr_), id(id_)
{
    returnTypeExpr->SetParent(this);
    id->SetParent(this);
}

std::string DelegateNode::Name() const
{
    std::string name = id->Str();
    name.append(1, '(');
    int n = parameters.Count();
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
        {
            name.append(", ");
        }
        name.append(parameters[i]->ToString());
    }
    name.append(1, ')');
    return name;
}

void DelegateNode::AddParameter(ParameterNode* parameter)
{
    parameter->SetParent(this);
    parameters.Add(parameter);
}

Node* DelegateNode::Clone(CloneContext& cloneContext) const
{
    DelegateNode* clone = new DelegateNode(GetSpan(), specifiers, returnTypeExpr->Clone(cloneContext), static_cast<IdentifierNode*>(id->Clone(cloneContext)));
    int n = parameters.Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameter = static_cast<ParameterNode*>(parameters[i]->Clone(cloneContext));
        clone->AddParameter(parameter);
    }
    return clone;
}

void DelegateNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Put(specifiers);
    writer.Put(returnTypeExpr.get());
    writer.Put(id.get());
    parameters.Write(writer);
}

void DelegateNode::Read(AstReader& reader)
{
    Node::Read(reader);
    specifiers = reader.GetSpecifiers();
    returnTypeExpr.reset(reader.GetNode());
    returnTypeExpr->SetParent(this);
    id.reset(reader.GetIdentifierNode());
    id->SetParent(this);
    parameters.Read(reader);
}

void DelegateNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ClassDelegateNode::ClassDelegateNode(const Span& span_) : Node(span_)
{
}

ClassDelegateNode::ClassDelegateNode(const Span& span_, Specifiers specifiers_, Node* returnTypeExpr_, IdentifierNode* id_) :
    Node(span_), specifiers(specifiers_), returnTypeExpr(returnTypeExpr_), id(id_)
{
    returnTypeExpr->SetParent(this);
    id->SetParent(this);
}

std::string ClassDelegateNode::Name() const
{
    std::string name = id->Str();
    name.append(1, '(');
    int n = parameters.Count();
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
        {
            name.append(", ");
        }
        name.append(parameters[i]->ToString());
    }
    name.append(1, ')');
    return name;
}

void ClassDelegateNode::AddParameter(ParameterNode* parameter)
{
    parameter->SetParent(this);
    parameters.Add(parameter);
}

Node* ClassDelegateNode::Clone(CloneContext& cloneContext) const
{
    ClassDelegateNode* clone = new ClassDelegateNode(GetSpan(), specifiers, returnTypeExpr->Clone(cloneContext), static_cast<IdentifierNode*>(id->Clone(cloneContext)));
    int n = parameters.Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameter = static_cast<ParameterNode*>(parameters[i]->Clone(cloneContext));
        clone->AddParameter(parameter);
    }
    return clone;
}

void ClassDelegateNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Put(specifiers);
    writer.Put(returnTypeExpr.get());
    writer.Put(id.get());
    parameters.Write(writer);
}

void ClassDelegateNode::Read(AstReader& reader)
{
    Node::Read(reader);
    specifiers = reader.GetSpecifiers();
    returnTypeExpr.reset(reader.GetNode());
    returnTypeExpr->SetParent(this);
    id.reset(reader.GetIdentifierNode());
    id->SetParent(this);
    parameters.Read(reader);
}

void ClassDelegateNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::ast
