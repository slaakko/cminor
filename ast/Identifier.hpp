// =================================
// Copyright (c) 2017 Seppo Laakko
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
    IdentifierNode(const Span& span_, const std::u32string& identifier_);
    NodeType GetNodeType() const override { return NodeType::identifierNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void Accept(Visitor& visitor) override;
    std::u32string ToString() const override { return identifier; }
    const std::u32string& Str() const { return identifier; }
private:
    std::u32string identifier;
};

} } // namespace cminor::ast

#endif // CMINOR_AST_IDENTIFIER_INCLUDED
