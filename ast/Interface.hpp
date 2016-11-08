// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_INTERFACE_INCLUDED
#define CMINOR_AST_INTERFACE_INCLUDED
#include <cminor/ast/Function.hpp>

namespace cminor { namespace ast {

class InterfaceNode : public Node
{
public:
    InterfaceNode(const Span& span_);
    InterfaceNode(const Span& span_, Specifiers specifiers_, IdentifierNode* id_);
    NodeType GetNodeType() const override { return NodeType::interfaceNode; }
    void AddMember(Node* member);
    const NodeList<Node>& Members() const { return members; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    Specifiers GetSpecifiers() const { return specifiers; }
    IdentifierNode* Id() const { return id.get(); }
    void Accept(Visitor& visitor) override;
private:
    Specifiers specifiers;
    std::unique_ptr<IdentifierNode> id;
    NodeList<Node> members;
};

} } // namespace cminor::ast

#endif // CMINOR_AST_INTERFACE_INCLUDED
