// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_ENUMERATION_INCLUDED
#define CMINOR_AST_ENUMERATION_INCLUDED
#include <cminor/ast/Identifier.hpp>
#include <cminor/ast/NodeList.hpp>
#include <cminor/ast/Specifier.hpp>

namespace cminor { namespace ast {

class EnumConstantNode;

class EnumTypeNode : public Node
{
public:
    EnumTypeNode(const Span& span_);
    EnumTypeNode(const Span& span_, Specifiers specifiers_, IdentifierNode* id_);
    NodeType GetNodeType() const override { return NodeType::enumTypeNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void AddConstant(EnumConstantNode* constant);
    EnumConstantNode* GetLastConstant() const;
    Specifiers GetSpecifiers() const { return specifiers; }
    IdentifierNode* Id() const { return id.get(); }
    void SetUnderlyingType(Node* underlyingType_);
    Node* GetUnderlyingType() const { return underlyingType.get(); }
    const NodeList<EnumConstantNode>& Constants() const { return constants; }
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
private:
    Specifiers specifiers;
    std::unique_ptr<IdentifierNode> id;
    std::unique_ptr<Node> underlyingType;
    NodeList<EnumConstantNode> constants;
};

class EnumConstantNode : public Node
{
public:
    EnumConstantNode(const Span& span_);
    EnumConstantNode(const Span& span_, IdentifierNode* id_, Node* value_);
    NodeType GetNodeType() const override { return NodeType::enumConstantNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    Node* GetValue() const { return value.get(); }
    IdentifierNode* Id() const { return id.get(); }
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
private:
    std::unique_ptr<IdentifierNode> id;
    std::unique_ptr<Node> value;
};

Node* MakeNextEnumConstantValue(const Span& span, EnumTypeNode* enumType);

} } // namespace cminor::ast

#endif // CMINOR_AST_ENUMERATION_INCLUDED
