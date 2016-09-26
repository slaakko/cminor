// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_NODE_INCLUDED
#define CMINOR_AST_NODE_INCLUDED
#include <cminor/ast/Clone.hpp>
#include <Cm.Parsing/Scanner.hpp>

namespace cminor { namespace ast {

using Cm::Parsing::Span;

enum class NodeType : uint8_t
{
    boolNode, sbyteNode, byteNode, shortNode, ushortNode, intNode, uintNode, longNode, ulongNode, floatNode, doubleNode, charNode, voidNode,
    booleanLiteralNode, sbyteLiteralNode, byteLiteralNode, shortLiteralNode, ushortLiteralNode, intLiteralNode, uintLiteralNode, longLiteralNode, ulongLiteralNode,
    floatLiteralNode, doubleLiteralNode, charLiteralNode, stringLiteralNode, nullLiteralNode,
    identifierNode,
    disjunctionNode, conjunctionNode, bitOrNode, bitXorNode, bitAndNode, equalNode, notEqualNode, lessNode, greaterNode, lessOrEqualNode, greaterOrEqualNode,
    shiftLeftNode, shiftRightNode, addNode, subNode, mulNode, divNode, remNode, notNode, unaryPlusNode, unaryMinusNode, complementNode
};

class Visitor;

class Node
{
public:
    Node();
    Node(const Span& span_);
    virtual ~Node();
    virtual NodeType GetNodeType() const = 0;
    virtual Node* Clone(CloneContext& cloneContext) const = 0;
    virtual void Accept(Visitor& visitor);
    const Span& GetSpan() const { return span; }
    Node* Parent() const { return parent; }
    void SetParent(Node* parent_) { parent = parent_; }
private:
    Span span;
    Node* parent;
};

class UnaryNode : public Node
{
public:
    UnaryNode(const Span& span_);
    UnaryNode(const Span& span_, Node* child_);
    Node* Child() const { return child.get(); }
    void Accept(Visitor& visitor) override;
private:
    std::unique_ptr<Node> child;
};

class BinaryNode : public Node
{
public:
    BinaryNode(const Span& span_);
    BinaryNode(const Span& span_, Node* left_, Node* right_);
    Node* Left() const { return left.get(); }
    Node* Right() const { return right.get(); }
    void Accept(Visitor& visitor) override;
private:
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
};

} } // namespace cminor::ast

#endif // CMINOR_AST_NODE_INCLUDED
