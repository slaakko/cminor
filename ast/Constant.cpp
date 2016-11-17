// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Constant.hpp>
#include <cminor/ast/AstWriter.hpp>
#include <cminor/ast/AstReader.hpp>
#include <cminor/ast/Visitor.hpp>

namespace cminor { namespace ast {

ConstantNode::ConstantNode(const Span& span_) : Node(span_)
{
}

ConstantNode::ConstantNode(const Span& span_, Specifiers specifiers_, Node* typeExpr_, IdentifierNode* id_, Node* value_) :
    Node(span_), specifiers(specifiers_), typeExpr(typeExpr_), id(id_), value(value_)
{
    typeExpr->SetParent(this);
    id->SetParent(this);
    value->SetParent(this);
}

Node* ConstantNode::Clone(CloneContext& cloneContext) const
{
    ConstantNode* clone = new ConstantNode(GetSpan(), specifiers, typeExpr->Clone(cloneContext), static_cast<IdentifierNode*>(id->Clone(cloneContext)), value->Clone(cloneContext));
    return clone;
}

void ConstantNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Put(specifiers);
    writer.Put(typeExpr.get());
    writer.Put(id.get());
    writer.Put(value.get());
}

void ConstantNode::Read(AstReader& reader)
{
    Node::Read(reader);
    specifiers = reader.GetSpecifiers();
    typeExpr.reset(reader.GetNode());
    typeExpr->SetParent(this);
    id.reset(reader.GetIdentifierNode());
    id->SetParent(this);
    value.reset(reader.GetNode());
    value->SetParent(this);
}

void ConstantNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::ast
