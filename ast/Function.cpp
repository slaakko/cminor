// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Function.hpp>
#include <cminor/ast/Statement.hpp>
#include <cminor/ast/Visitor.hpp>

namespace cminor { namespace ast {

FunctionGroupIdNode::FunctionGroupIdNode(const Span& span_) : Node(span_)
{
}

FunctionGroupIdNode::FunctionGroupIdNode(const Span& span_, const std::string& functionGroupId_) : Node(span_), functionGroupId(functionGroupId_)
{
}

Node* FunctionGroupIdNode::Clone(CloneContext& cloneContext) const
{
    return new FunctionGroupIdNode(GetSpan(), functionGroupId);
}

void FunctionGroupIdNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.AsMachineWriter().Put(functionGroupId);
}

void FunctionGroupIdNode::Read(AstReader& reader)
{
    Node::Read(reader);
    functionGroupId = reader.GetUtf8String();
}

void AttributeMap::AddAttribute(const std::string& name_, const std::string& value_)
{
    nameValuePairs[name_] = value_;
}

std::string AttributeMap::GetAttribute(const std::string& name) const
{
    auto it = nameValuePairs.find(name);
    if (it != nameValuePairs.cend())
    {
        return it->second;
    }
    return std::string();
}

void AttributeMap::Write(AstWriter& writer)
{
    bool hasAttributes = !nameValuePairs.empty();
    writer.AsMachineWriter().Put(hasAttributes);
    if (hasAttributes)
    {
        int32_t n = int32_t(nameValuePairs.size());
        writer.AsMachineWriter().Put(n);
        for (const auto& p : nameValuePairs)
        {
            writer.AsMachineWriter().Put(p.first);
            writer.AsMachineWriter().Put(p.second);
        }
    }
}

void AttributeMap::Read(AstReader& reader)
{
    bool hasAttributes = reader.GetBool();
    if (hasAttributes)
    {
        int32_t n = reader.GetInt();
        for (int32_t i = 0; i < n; ++i)
        {
            std::string name = reader.GetUtf8String();
            std::string value = reader.GetUtf8String();
            nameValuePairs[name] = value;
        }
    }
}

FunctionNode::FunctionNode(const Span& span_) : Node(span_), compileUnit(nullptr)
{
}

FunctionNode::FunctionNode(const Span& span_, Specifiers specifiers_, Node* returnTypeExpr_, FunctionGroupIdNode* groupId_) :
    Node(span_), specifiers(specifiers_), returnTypeExpr(returnTypeExpr_), groupId(groupId_), compileUnit(nullptr)
{
    if (returnTypeExpr)
    {
        returnTypeExpr->SetParent(this);
    }
    groupId->SetParent(this);
}

void FunctionNode::AddParameter(ParameterNode* parameter)
{
    parameter->SetParent(this);
    parameters.Add(parameter);
}

void FunctionNode::SetBody(CompoundStatementNode* body_)
{
    body.reset(body_);
    if (body)
    {
        body->SetParent(this);
    }
}

Node* FunctionNode::Clone(CloneContext& cloneContext) const
{
    FunctionNode* clone = new FunctionNode(GetSpan(), specifiers, returnTypeExpr->Clone(cloneContext), static_cast<FunctionGroupIdNode*>(groupId->Clone(cloneContext)));
    int n = parameters.Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameter = parameters[i];
        clone->AddParameter(static_cast<ParameterNode*>(parameter->Clone(cloneContext)));
    }
    if (body)
    {
        clone->SetBody(static_cast<CompoundStatementNode*>(body->Clone(cloneContext)));
    }

    return clone;
}

void FunctionNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Put(specifiers);
    bool hasReturnTypeExpr = returnTypeExpr != nullptr;
    writer.AsMachineWriter().Put(hasReturnTypeExpr);
    if (hasReturnTypeExpr)
    {
        writer.Put(returnTypeExpr.get());
    }
    writer.Put(groupId.get());
    parameters.Write(writer);
    bool hasBody = body != nullptr;
    writer.AsMachineWriter().Put(hasBody);
    attributes.Write(writer);
}

void FunctionNode::Read(AstReader& reader)
{
    Node::Read(reader);
    specifiers = reader.GetSpecifiers();
    bool hasReturnTypeExpr = reader.GetBool();
    if (hasReturnTypeExpr)
    {
        returnTypeExpr.reset(reader.GetNode());
        returnTypeExpr->SetParent(this);
    }
    groupId.reset(reader.GetFunctionGroupIdNode());
    groupId->SetParent(this);
    parameters.Read(reader);
    parameters.SetParent(this);
    bool hasBody = reader.GetBool();
    if (hasBody)
    {
        body.reset(reader.GetCompoundStatementNode());
    }
    attributes.Read(reader);
}

void FunctionNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

std::string FunctionNode::Name() const
{
    std::string name = groupId->Str();
    name.append(1, '(');
    int n = parameters.Count();
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
        {
            name.append(", ");
        }
        name.append(parameters[i]->ToString());
    }
    name.append(1, ')');
    return name;
}

void FunctionNode::SetAttributes(const AttributeMap& attributes_)
{
    attributes = attributes_;
}

} } // namespace cminor::ast
