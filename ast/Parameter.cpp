// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Parameter.hpp>
#include <cminor/ast/Identifier.hpp>
#include <cminor/ast/Visitor.hpp>

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
