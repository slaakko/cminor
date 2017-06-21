// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_DELEGATE_INCLUDED
#define CMINOR_AST_DELEGATE_INCLUDED
#include <cminor/ast/Parameter.hpp>
#include <cminor/ast/NodeList.hpp>

namespace cminor { namespace ast {

class DelegateNode : public Node
{
public:
    DelegateNode(const Span& span_);
    DelegateNode(const Span& span_, Specifiers specifiers_, Node* returnTypeExpr_, IdentifierNode* id_);
    NodeType GetNodeType() const override { return NodeType::delegateNode; }
    void AddParameter(ParameterNode* parameter) override;
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    std::u32string Name() const;
    std::u32string ToString() const override { return Name(); }
    Specifiers GetSpecifiers() const { return specifiers; }
    Node* ReturnTypeExpr() const { return returnTypeExpr.get(); }
    IdentifierNode* Id() const { return id.get(); }
    const NodeList<ParameterNode>& Parameters() const { return parameters; }
private:
    Specifiers specifiers;
    std::unique_ptr<Node> returnTypeExpr;
    std::unique_ptr<IdentifierNode> id;
    NodeList<ParameterNode> parameters;
};

class ClassDelegateNode : public Node
{
public:
    ClassDelegateNode(const Span& span_);
    ClassDelegateNode(const Span& span_, Specifiers specifiers_, Node* returnTypeExpr_, IdentifierNode* id_);
    NodeType GetNodeType() const override { return NodeType::classDelegateNode; }
    void AddParameter(ParameterNode* parameter) override;
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    std::u32string Name() const;
    std::u32string ToString() const override { return Name(); }
    Specifiers GetSpecifiers() const { return specifiers; }
    Node* ReturnTypeExpr() const { return returnTypeExpr.get(); }
    IdentifierNode* Id() const { return id.get(); }
    const NodeList<ParameterNode>& Parameters() const { return parameters; }
private:
    Specifiers specifiers;
    std::unique_ptr<Node> returnTypeExpr;
    std::unique_ptr<IdentifierNode> id;
    NodeList<ParameterNode> parameters;
};

} } // namespace cminor::ast

#endif // CMINOR_AST_DELEGATE_INCLUDED
