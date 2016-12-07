// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Function.hpp>
#include <cminor/ast/Statement.hpp>
#include <cminor/ast/Visitor.hpp>
#include <cminor/machine/Constant.hpp>

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
    utf32_string s = ToUtf32(functionGroupId);
    ConstantId id = writer.GetConstantPool()->GetIdFor(s);
    Assert(id != noConstantId, "got no id for constant");
    id.Write(writer);
}

void FunctionGroupIdNode::Read(AstReader& reader)
{
    Node::Read(reader);
    ConstantId id;
    id.Read(reader);
    Constant constant = reader.GetConstantPool()->GetConstant(id);
    Assert(constant.Value().GetType() == ValueType::stringLiteral, "string literal expected");
    functionGroupId = ToUtf8(constant.Value().AsStringLiteral());
}

void FunctionGroupIdNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
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
        uint32_t n = uint32_t(nameValuePairs.size());
        writer.AsMachineWriter().PutEncodedUInt(n);
        for (const auto& p : nameValuePairs)
        {
            const std::string& name = p.first;
            utf32_string nameS = ToUtf32(name);
            ConstantId nameId = writer.GetConstantPool()->GetIdFor(nameS);
            Assert(nameId != noConstantId, "got no id for constant");
            nameId.Write(writer);
            const std::string& value = p.second;
            utf32_string valueS = ToUtf32(value);
            ConstantId valueId = writer.GetConstantPool()->GetIdFor(valueS);
            Assert(valueId != noConstantId, "got no id for constant");
            valueId.Write(writer);
        }
    }
}

void AttributeMap::Read(AstReader& reader)
{
    bool hasAttributes = reader.GetBool();
    if (hasAttributes)
    {
        uint32_t n = reader.GetEncodedUInt();
        for (uint32_t i = 0; i < n; ++i)
        {
            ConstantId nameId;
            nameId.Read(reader);
            Constant nameConstant = reader.GetConstantPool()->GetConstant(nameId);
            Assert(nameConstant.Value().GetType() == ValueType::stringLiteral, "string literal expected");
            std::string name = ToUtf8(nameConstant.Value().AsStringLiteral());
            ConstantId valueId;
            valueId.Read(reader);
            Constant valueConstant = reader.GetConstantPool()->GetConstant(valueId);
            Assert(valueConstant.Value().GetType() == ValueType::stringLiteral, "string literal expected");
            std::string value = ToUtf8(valueConstant.Value().AsStringLiteral());
            nameValuePairs[name] = value;
        }
    }
}

void AttributeMap::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

FunctionNode::FunctionNode(const Span& span_) : Node(span_)
{
}

FunctionNode::FunctionNode(const Span& span_, Specifiers specifiers_, Node* returnTypeExpr_, FunctionGroupIdNode* groupId_) :
    Node(span_), specifiers(specifiers_), returnTypeExpr(returnTypeExpr_), groupId(groupId_)
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

void FunctionNode::SetBodySource(CompoundStatementNode* bodySource_)
{
    bodySource.reset(bodySource_);
    if (bodySource)
    {
        bodySource->SetParent(this);
    }
}

void FunctionNode::SwitchToBody()
{
    if (bodySource && !body)
    {
        SetBody(bodySource.release());
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
    if (hasBody)
    {
        writer.Put(body.get());
    }
    bool hasBodySource = bodySource != nullptr;
    writer.AsMachineWriter().Put(hasBodySource);
    if (hasBodySource)
    {
        writer.Put(bodySource.get());
    }
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
    bool hasBodySource = reader.GetBool();
    if (hasBodySource)
    {
        bodySource.reset(reader.GetCompoundStatementNode());
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
