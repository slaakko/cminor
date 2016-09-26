// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_IDENTIFIER_INCLUDED
#define CMINOR_AST_IDENTIFIER_INCLUDED
#include <cminor/ast/Node.hpp>

namespace cminor { namespace ast {

class IdentifierNode : public Node
{
public:
    IdentifierNode(const Span& span_);
    IdentifierNode(const Span& span_, const std::string& identifier_);
    NodeType GetNodeType() const override { return NodeType::identifierNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    const std::string& Str() const { return identifier; }
private:
    std::string identifier;
};

} } // namespace cminor::ast

#endif // CMINOR_AST_IDENTIFIER_INCLUDED
