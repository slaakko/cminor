// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_FUNCTION_INCLUDED
#define CMINOR_AST_FUNCTION_INCLUDED
#include <cminor/ast/Specifier.hpp>
#include <cminor/ast/Parameter.hpp>
#include <cminor/ast/Identifier.hpp>

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
    const std::string& Str() const { return functionGroupId; }
private:
    std::string functionGroupId;
};

class FunctionNode : public Node
{
public:
    FunctionNode(const Span& span_);
    FunctionNode(const Span& span_, Specifiers specifiers_, Node* returnTypeExpr_, FunctionGroupIdNode* groupId_);
    NodeType GetNodeType() const override { return NodeType::functionNode; }
    void AddParameter(ParameterNode* parameter) override;
    void SetBody(CompoundStatementNode* body_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    Specifiers GetSpecifiers() const { return specifiers; }
    Node* ReturnTypeExpr() const { return returnTypeExpr.get(); }
    FunctionGroupIdNode* GroupId() const { return groupId.get(); }
    const NodeList<ParameterNode>& Parameters() const { return parameters; }
    CompoundStatementNode* Body() const { return body.get(); }
    bool HasBody() const { return body != nullptr; }
    CompileUnitNode* GetCompileUnit() const { return compileUnit; }
    void SetCompileUnit(CompileUnitNode* compileUnit_) { compileUnit = compileUnit_; }
private:
    Specifiers specifiers;
    std::unique_ptr<Node> returnTypeExpr;
    std::unique_ptr<FunctionGroupIdNode> groupId;
    NodeList<ParameterNode> parameters;
    std::unique_ptr<CompoundStatementNode> body;
    CompileUnitNode* compileUnit;
};

} } // namespace cminor::ast

#endif // CMINOR_AST_FUNCTION_INCLUDED
