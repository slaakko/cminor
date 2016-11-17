// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Enumeration.hpp>
#include <cminor/ast/Visitor.hpp>
#include <cminor/ast/Literal.hpp>
#include <cminor/ast/Expression.hpp>

namespace cminor { namespace ast {

EnumTypeNode::EnumTypeNode(const Span& span_) : Node(span_)
{
}

EnumTypeNode::EnumTypeNode(const Span& span_, Specifiers specifiers_, IdentifierNode* id_) : Node(span_), specifiers(specifiers_), id(id_)
{
    id->SetParent(this);
}

Node* EnumTypeNode::Clone(CloneContext& cloneContext) const
{
    EnumTypeNode* clone = new EnumTypeNode(GetSpan(), specifiers, static_cast<IdentifierNode*>(id->Clone(cloneContext)));
    if (underlyingType)
    {
        clone->SetUnderlyingType(underlyingType->Clone(cloneContext));
    }
    int n = constants.Count();
    for (int i = 0; i < n; ++i)
    {
        EnumConstantNode* constant = constants[i];
        clone->AddConstant(static_cast<EnumConstantNode*>(constant->Clone(cloneContext)));
    }
    return clone;
}

void EnumTypeNode::SetUnderlyingType(Node* underlyingType_)
{
    underlyingType.reset(underlyingType_);
    underlyingType->SetParent(this);
}

void EnumTypeNode::AddConstant(EnumConstantNode* constant)
{
    constant->SetParent(this);
    constants.Add(constant);
}

EnumConstantNode* EnumTypeNode::GetLastConstant() const
{
    if (constants.Count() == 0)
    {
        return nullptr;
    }
    else
    {
        return constants[constants.Count() - 1];
    }
}

void EnumTypeNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Put(specifiers);
    writer.Put(id.get());
    bool hasUnderlyingType = underlyingType != nullptr;
    writer.AsMachineWriter().Put(hasUnderlyingType);
    if (hasUnderlyingType)
    {
        writer.Put(underlyingType.get());
    }
    constants.Write(writer);
}

void EnumTypeNode::Read(AstReader& reader)
{
    Node::Read(reader);
    specifiers = reader.GetSpecifiers();
    id.reset(reader.GetIdentifierNode());
    id->SetParent(this);
    bool hasUnderlyingType = reader.GetBool();
    if (hasUnderlyingType)
    {
        underlyingType.reset(reader.GetNode());
        underlyingType->SetParent(this);
    }
    constants.Read(reader);
    constants.SetParent(this);
}

void EnumTypeNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

EnumConstantNode::EnumConstantNode(const Span& span_) : Node(span_)
{
}

EnumConstantNode::EnumConstantNode(const Span& span_, IdentifierNode* id_, Node* value_) : Node(span_), id(id_), value(value_)
{
    id->SetParent(this);
    value->SetParent(this);
}

Node* EnumConstantNode::Clone(CloneContext& cloneContext) const
{
    EnumConstantNode* clone = new EnumConstantNode(GetSpan(), static_cast<IdentifierNode*>(id->Clone(cloneContext)), value->Clone(cloneContext));
    return clone;
}

void EnumConstantNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Put(id.get());
    writer.Put(value.get());
}

void EnumConstantNode::Read(AstReader& reader)
{
    Node::Read(reader);
    id.reset(reader.GetIdentifierNode());
    id->SetParent(this);
    value.reset(reader.GetNode());
    value->SetParent(this);
}

void EnumConstantNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

Node* MakeNextEnumConstantValue(const Span& span, EnumTypeNode* enumType)
{
    EnumConstantNode* lastConstant = enumType->GetLastConstant();
    if (lastConstant)
    {
        Node* lastValue = lastConstant->GetValue();
        if (lastValue)
        {
            CloneContext cloneContext;
            Node* clonedValue = lastValue->Clone(cloneContext);
            if (enumType->GetUnderlyingType())
            {
                if (enumType->GetUnderlyingType()->IsUnsignedTypeNode())
                {
                    return new AddNode(span, clonedValue, new ByteLiteralNode(span, 1));
                }
            }
            return new AddNode(span, clonedValue, new SByteLiteralNode(span, 1));
        }
        else
        {
            throw std::runtime_error("last constant returned null value");
        }
    }
    else
    {
        if (enumType->GetUnderlyingType())
        {
            if (enumType->GetUnderlyingType()->IsUnsignedTypeNode())
            {
                return new ByteLiteralNode(span, 0);
            }
        }
        return new SByteLiteralNode(span, 0);
    }
}

} } // namespace cminor::ast
