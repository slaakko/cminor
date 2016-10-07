// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Namespace.hpp>
#include <cminor/ast/Visitor.hpp>

namespace cminor { namespace ast {

NamespaceNode::NamespaceNode(const Span& span_) : Node(span_)
{
}

NamespaceNode::NamespaceNode(const Span& span_, IdentifierNode* id_) : Node(span_), id(id_)
{
}

void NamespaceNode::AddMember(Node* member)
{
    member->SetParent(this);
    members.Add(member);
}

Node* NamespaceNode::Clone(CloneContext& cloneContext) const
{
    NamespaceNode* clone = new NamespaceNode(GetSpan(), static_cast<IdentifierNode*>(id->Clone(cloneContext)));
    int n = members.Count();
    for (int i = 0; i < n; ++i)
    {
        Node* member = members[i];
        clone->AddMember(member->Clone(cloneContext));
    }
    return clone;
}

void NamespaceNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

AliasNode::AliasNode(const Span& span_) : Node(span_)
{
}

AliasNode::AliasNode(const Span& span_, IdentifierNode* id_, IdentifierNode* qid_) : Node(span_), id(id_), qid(qid_)
{
    id->SetParent(this);
    qid->SetParent(this);
}

Node* AliasNode::Clone(CloneContext& cloneContext) const
{
    return new AliasNode(GetSpan(), static_cast<IdentifierNode*>(id->Clone(cloneContext)), static_cast<IdentifierNode*>(qid->Clone(cloneContext)));
}

void AliasNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

NamespaceImportNode::NamespaceImportNode(const Span& span_) : Node(span_)
{
}

NamespaceImportNode::NamespaceImportNode(const Span& span_, IdentifierNode* ns_) : Node(span_), ns(ns_)
{
}

Node* NamespaceImportNode::Clone(CloneContext& cloneContext) const
{
    return new NamespaceImportNode(GetSpan(), static_cast<IdentifierNode*>(ns->Clone(cloneContext)));
}

void NamespaceImportNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::ast
