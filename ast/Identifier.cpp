// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Identifier.hpp>
#include <cminor/ast/Visitor.hpp>

namespace cminor { namespace ast {

IdentifierNode::IdentifierNode(const Span& span_) : Node(span_), identifier()
{
}

IdentifierNode::IdentifierNode(const Span& span_, const std::string& identifier_) : Node(span_), identifier(identifier_)
{
}

Node* IdentifierNode::Clone(CloneContext& cloneContext) const
{
    return new IdentifierNode(GetSpan(), identifier);
}

void IdentifierNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::ast
