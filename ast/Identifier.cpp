// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Identifier.hpp>
#include <cminor/ast/Visitor.hpp>
#include <cminor/ast/AstWriter.hpp>
#include <cminor/ast/AstReader.hpp>
#include <cminor/machine/Constant.hpp>

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
    utf32_string s = ToUtf32(identifier);
    ConstantId id = writer.GetConstantPool()->GetIdFor(s);
    Assert(id != noConstantId, "got no id for constant");
    id.Write(writer);
}

void IdentifierNode::Read(AstReader& reader)
{
    Node::Read(reader);
    ConstantId id;
    id.Read(reader);
    Constant constant = reader.GetConstantPool()->GetConstant(id);
    Assert(constant.Value().GetType() == ValueType::stringLiteral, "string literal expected");
    identifier = ToUtf8(constant.Value().AsStringLiteral());
}

void IdentifierNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::ast
