// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Parameter.hpp>
#include <cminor/ast/Identifier.hpp>
#include <cminor/ast/Visitor.hpp>
#include <cminor/ast/AstWriter.hpp>
#include <cminor/ast/AstReader.hpp>

namespace cminor { namespace ast {

ParameterNode::ParameterNode(const Span& span_) : Node(span_)
{
}

ParameterNode::ParameterNode(const Span& span_, Node* typeExpr_, IdentifierNode* id_) : Node(span_), typeExpr(typeExpr_), id(id_)
{
    typeExpr->SetParent(this);
    if (id)
    {
        id->SetParent(this);
    }
}

Node* ParameterNode::Clone(CloneContext& cloneContext) const
{
    IdentifierNode* clonedId = nullptr;
    if (id)
    {
        clonedId = static_cast<IdentifierNode*>(id->Clone(cloneContext));
    }
    return new ParameterNode(GetSpan(), typeExpr->Clone(cloneContext), clonedId);
}

void ParameterNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Put(typeExpr.get());
    writer.Put(id.get());
}

void ParameterNode::Read(AstReader& reader)
{
    Node::Read(reader);
    typeExpr.reset(reader.GetNode());
    typeExpr->SetParent(this);
    id.reset(reader.GetIdentifierNode());
    id->SetParent(this);
}

void ParameterNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

std::string ParameterNode::ToString() const
{
    std::string s = typeExpr->ToString();
    s.append(1, ' ').append(id->ToString());
    return s;
}

} } // namespace cminor::ast
