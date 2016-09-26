// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Node.hpp>

namespace cminor { namespace ast {

Node::Node() : span(), parent(nullptr)
{
}

Node::Node(const Span& span_) : span(span_), parent(nullptr)
{
}

Node::~Node()
{
}

void Node::Accept(Visitor& visitor)
{
    throw std::runtime_error("accept missing");
}

UnaryNode::UnaryNode(const Span& span_) : Node(span_)
{
}

UnaryNode::UnaryNode(const Span& span_, Node* child_) : Node(span_), child(child_)
{
    child->SetParent(this);
}

void UnaryNode::Accept(Visitor& visitor)
{
    child->Accept(visitor);
}

BinaryNode::BinaryNode(const Span& span_) : Node(span_)
{
}

BinaryNode::BinaryNode(const Span& span_, Node* left_, Node* right_) : Node(span_), left(left_), right(right_)
{
    left->SetParent(this);
    right->SetParent(this);
}

void BinaryNode::Accept(Visitor& visitor)
{
    left->Accept(visitor);
    right->Accept(visitor);
}

} } // namespace cminor::ast
