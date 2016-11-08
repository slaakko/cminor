// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_TEMPLATE_INCLUDED
#define CMINOR_AST_TEMPLATE_INCLUDED
#include <cminor/ast/Identifier.hpp>
#include <cminor/ast/NodeList.hpp>

namespace cminor { namespace ast {

class TemplateIdNode : public Node
{
public:
    TemplateIdNode(const Span& span_);
    TemplateIdNode(const Span& span_, Node* primary_);
    NodeType GetNodeType() const override { return NodeType::templateIdNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void AddTemplateArgument(Node* templateArgument);
    void Accept(Visitor& visitor) override;
    Node* Primary() const { return primary.get(); }
    const NodeList<Node>& TemplateArguments() const { return templateArguments; }
private:
    std::unique_ptr<Node> primary;
    NodeList<Node> templateArguments;
};

class TemplateParameterNode : public Node
{
public:
    TemplateParameterNode(const Span& span_);
    TemplateParameterNode(const Span& span_, IdentifierNode* id_, Node* defaultTemplateArgument_);
    NodeType GetNodeType() const override { return NodeType::templateParameterNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    IdentifierNode* Id() const { return id.get(); }
    Node* DefaultTemplateArgument() const { return defaultTemplateArgument.get(); }
private:
    std::unique_ptr<IdentifierNode> id;
    std::unique_ptr<Node> defaultTemplateArgument;
};

} } // namespace cminor::ast

#endif // CMINOR_AST_TEMPLATE_INCLUDED
