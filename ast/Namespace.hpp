// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_NAMESPACE_INCLUDED
#define CMINOR_AST_NAMESPACE_INCLUDED
#include <cminor/ast/Identifier.hpp>

namespace cminor { namespace ast {

class IdentifierNode;

class NamespaceNode : public Node
{
public:
    NamespaceNode(const Span& span_);
    NamespaceNode(const Span& span_, IdentifierNode* id_);
    NodeType GetNodeType() const override { return NodeType::namespaceNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void AddMember(Node* member);
    IdentifierNode* Id() const { return id.get(); }
    NodeList<Node>& Members() { return members; }
    const NodeList<Node>& Members() const { return members; }
private:
    std::unique_ptr<IdentifierNode> id;
    NodeList<Node> members;

};

} } // namespace cminor::ast

#endif // CMINOR_AST_NAMESPACE_INCLUDED
