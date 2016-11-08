// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Template.hpp>
#include <cminor/ast/Visitor.hpp>

namespace cminor { namespace ast {

TemplateIdNode::TemplateIdNode(const Span& span_) : Node(span_)
{
}

TemplateIdNode::TemplateIdNode(const Span& span_, Node* primary_) : Node(span_), primary(primary_)
{
    primary->SetParent(this);
}

void TemplateIdNode::AddTemplateArgument(Node* templateArgument)
{
    templateArgument->SetParent(this);
    templateArguments.Add(templateArgument);
}

Node* TemplateIdNode::Clone(CloneContext& cloneContext) const
{
    TemplateIdNode* clone = new TemplateIdNode(GetSpan(), primary->Clone(cloneContext));
    int n = templateArguments.Count();
    for (int i = 0; i < n; ++i)
    {
        Node* templateArgument = templateArguments[i];
        clone->AddTemplateArgument(templateArgument->Clone(cloneContext));
    }
    return clone;
}

void TemplateIdNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Put(primary.get());
    templateArguments.Write(writer);
}

void TemplateIdNode::Read(AstReader& reader)
{
    Node::Read(reader);
    primary.reset(reader.GetNode());
    primary->SetParent(this);
    templateArguments.Read(reader);
    templateArguments.SetParent(this);
}

void TemplateIdNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

TemplateParameterNode::TemplateParameterNode(const Span& span_) : Node(span_)
{
}

TemplateParameterNode::TemplateParameterNode(const Span& span_, IdentifierNode* id_, Node* defaultTemplateArgument_) : Node(span_), id(id_), defaultTemplateArgument(defaultTemplateArgument_)
{
    id->SetParent(this);
    if (defaultTemplateArgument)
    {
        defaultTemplateArgument->SetParent(this);
    }
}

Node* TemplateParameterNode::Clone(CloneContext& cloneContext) const
{
    Node* clonedDefaultTemplateArgument = nullptr;
    if (defaultTemplateArgument)
    {
        clonedDefaultTemplateArgument = defaultTemplateArgument->Clone(cloneContext);
    }
    return new TemplateParameterNode(GetSpan(), static_cast<IdentifierNode*>(id->Clone(cloneContext)), clonedDefaultTemplateArgument);
}

void TemplateParameterNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Put(id.get());
    bool hasDefaultTemplateArgument = defaultTemplateArgument != nullptr;
    writer.AsMachineWriter().Put(hasDefaultTemplateArgument);
    if (hasDefaultTemplateArgument)
    {
        writer.Put(defaultTemplateArgument.get());
    }
}

void TemplateParameterNode::Read(AstReader& reader)
{
    Node::Read(reader);
    id.reset(reader.GetIdentifierNode());
    id->SetParent(this);
    bool hasDefaultTemplateArgument = reader.GetBool();
    if (hasDefaultTemplateArgument)
    {
        defaultTemplateArgument.reset(reader.GetNode());
        defaultTemplateArgument->SetParent(this);
    }
}

void TemplateParameterNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::ast
