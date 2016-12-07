// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_CONSTANT_INCLUDED
#define CMINOR_AST_CONSTANT_INCLUDED
#include <cminor/ast/Identifier.hpp>
#include <cminor/ast/Specifier.hpp>

namespace cminor { namespace ast {

class ConstantNode : public Node
{
public:
    ConstantNode(const Span& span_);
    ConstantNode(const Span& span_, Specifiers specifiers_, Node* typeExpr_, IdentifierNode* id_, Node* value_);
    NodeType GetNodeType() const override { return NodeType::constantNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    Specifiers GetSpecifiers() const { return specifiers; }
    Node* TypeExpr() const { return typeExpr.get(); }
    IdentifierNode* Id() const { return id.get(); }
    Node* Value() const { return value.get(); }
private:
    Specifiers specifiers;
    std::unique_ptr<Node> typeExpr;
    std::unique_ptr<IdentifierNode> id;
    std::unique_ptr<Node> value;
};

} } // namespace cminor::ast

#endif // CMINOR_AST_CONSTANT_INCLUDED
