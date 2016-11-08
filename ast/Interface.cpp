// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Interface.hpp>
#include <cminor/ast/Visitor.hpp>

namespace cminor { namespace ast {

InterfaceNode::InterfaceNode(const Span& span_) : Node(span_)
{
}

InterfaceNode::InterfaceNode(const Span& span_, Specifiers specifiers_, IdentifierNode* id_) : Node(span_), specifiers(specifiers_), id(id_)
{
    id->SetParent(this);
}

void InterfaceNode::AddMember(Node* member)
{
    member->SetParent(this);
    members.Add(member);
}

Node* InterfaceNode::Clone(CloneContext& cloneContext) const
{
    InterfaceNode* clone = new InterfaceNode(GetSpan(), specifiers, static_cast<IdentifierNode*>(id->Clone(cloneContext)));
    int n = members.Count();
    for (int i = 0; i < n; ++i)
    {
        clone->AddMember(members[i]->Clone(cloneContext));
    }
    return clone;
}

void InterfaceNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Put(specifiers);
    writer.Put(id.get());
    members.Write(writer);
}

void InterfaceNode::Read(AstReader& reader)
{
    Node::Read(reader);
    specifiers = reader.GetSpecifiers();
    id.reset(reader.GetIdentifierNode());
    id->SetParent(this);
    members.Read(reader);
    members.SetParent(this);
}

void InterfaceNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::ast
