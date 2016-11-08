// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Identifier.hpp>
#include <cminor/ast/Visitor.hpp>
#include <cminor/ast/AstWriter.hpp>
#include <cminor/ast/AstReader.hpp>

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

void IdentifierNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.AsMachineWriter().Put(identifier);
}

void IdentifierNode::Read(AstReader& reader)
{
    Node::Read(reader);
    identifier = reader.GetUtf8String();
}

void IdentifierNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::ast
