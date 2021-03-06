// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_NAMESPACE_INCLUDED
#define CMINOR_AST_NAMESPACE_INCLUDED
#include <cminor/ast/Identifier.hpp>
#include <cminor/ast/NodeList.hpp>

namespace cminor { namespace ast {

class IdentifierNode;

class NamespaceNode : public Node
{
public:
    NamespaceNode(const Span& span_);
    NamespaceNode(const Span& span_, IdentifierNode* id_);
    NodeType GetNodeType() const override { return NodeType::namespaceNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    std::u32string ToString() const override { return id->Str(); }
    void AddMember(Node* member);
    IdentifierNode* Id() const { return id.get(); }
    NodeList<Node>& Members() { return members; }
    const NodeList<Node>& Members() const { return members; }
private:
    std::unique_ptr<IdentifierNode> id;
    NodeList<Node> members;

};

class AliasNode : public Node
{
public:
    AliasNode(const Span& span_);
    AliasNode(const Span& span_, IdentifierNode* id_, IdentifierNode* qid_);
    NodeType GetNodeType() const override { return NodeType::aliasNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    IdentifierNode* Id() const { return id.get(); }
    IdentifierNode* Qid() const { return qid.get(); }
private:
    std::unique_ptr<IdentifierNode> id;
    std::unique_ptr<IdentifierNode> qid;
};

class NamespaceImportNode : public Node
{
public:
    NamespaceImportNode(const Span& span_);
    NamespaceImportNode(const Span& span_, IdentifierNode* ns_);
    NodeType GetNodeType() const override { return NodeType::namespaceImportNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    IdentifierNode* Ns() const { return ns.get(); }
private:
    std::unique_ptr<IdentifierNode> ns;
};

} } // namespace cminor::ast

#endif // CMINOR_AST_NAMESPACE_INCLUDED
