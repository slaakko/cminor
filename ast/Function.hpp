// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_FUNCTION_INCLUDED
#define CMINOR_AST_FUNCTION_INCLUDED
#include <cminor/ast/Specifier.hpp>
#include <cminor/ast/Parameter.hpp>
#include <cminor/ast/Identifier.hpp>
#include <cminor/ast/NodeList.hpp>
#include <unordered_map>

namespace cminor { namespace ast {

class CompoundStatementNode;
class CompileUnitNode;

class FunctionGroupIdNode : public Node
{
public:
    FunctionGroupIdNode(const Span& span_);
    FunctionGroupIdNode(const Span& span_, const std::string& functionGroupId_);
    NodeType GetNodeType() const override { return NodeType::functionGroupIdNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const std::string& Str() const { return functionGroupId; }
private:
    std::string functionGroupId;
};

class AttributeMap
{
public:
    void AddAttribute(const std::string& name_, const std::string& value_);
    std::string GetAttribute(const std::string& name) const;
    void Write(AstWriter& writer);
    void Read(AstReader& reader);
private:
    std::unordered_map<std::string, std::string> nameValuePairs;
};

class FunctionNode : public Node
{
public:
    FunctionNode(const Span& span_);
    FunctionNode(const Span& span_, Specifiers specifiers_, Node* returnTypeExpr_, FunctionGroupIdNode* groupId_);
    NodeType GetNodeType() const override { return NodeType::functionNode; }
    void AddParameter(ParameterNode* parameter) override;
    void SetBody(CompoundStatementNode* body_);
    void SetBodySource(CompoundStatementNode* bodySource_);
    void SwitchToBody();
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    Specifiers GetSpecifiers() const { return specifiers; }
    Node* ReturnTypeExpr() const { return returnTypeExpr.get(); }
    std::string Name() const;
    std::string ToString() const override { return Name(); }
    FunctionGroupIdNode* GroupId() const { return groupId.get(); }
    const NodeList<ParameterNode>& Parameters() const { return parameters; }
    CompoundStatementNode* Body() const { return body.get(); }
    bool HasBody() const { return body != nullptr; }
    CompoundStatementNode* BodySource() const { return bodySource.get(); }
    CompileUnitNode* GetCompileUnit() const { return compileUnit; }
    void SetCompileUnit(CompileUnitNode* compileUnit_) { compileUnit = compileUnit_; }
    void SetAttributes(const AttributeMap& attributes_);
    const AttributeMap& Attributes() const { return attributes; }
private:
    Specifiers specifiers;
    std::unique_ptr<Node> returnTypeExpr;
    std::unique_ptr<FunctionGroupIdNode> groupId;
    NodeList<ParameterNode> parameters;
    std::unique_ptr<CompoundStatementNode> body;
    std::unique_ptr<CompoundStatementNode> bodySource;
    CompileUnitNode* compileUnit;
    AttributeMap attributes;
};

} } // namespace cminor::ast

#endif // CMINOR_AST_FUNCTION_INCLUDED
