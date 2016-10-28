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
