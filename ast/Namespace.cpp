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

} } // namespace cminor::ast
