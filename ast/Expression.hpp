// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_EXPRESSION_INCLUDED
#define CMINOR_AST_EXPRESSION_INCLUDED
#include <cminor/ast/Identifier.hpp>

namespace cminor { namespace ast {

class DisjunctionNode : public BinaryNode
{
public:
    DisjunctionNode(const Span& span_);
    DisjunctionNode(const Span& span_, Node* left_, Node* right_);
    NodeType GetNodeType() const override { return NodeType::disjunctionNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class ConjunctionNode : public BinaryNode
{
public:
    ConjunctionNode(const Span& span_);
    ConjunctionNode(const Span& span_, Node* left_, Node* right_);
    NodeType GetNodeType() const override { return NodeType::conjunctionNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

} } // namespace cminor::ast

#endif // CMINOR_AST_EXPRESSION_INCLUDED
