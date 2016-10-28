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
class ParameterNode;

enum class NodeType : uint8_t
{
    boolNode, sbyteNode, byteNode, shortNode, ushortNode, intNode, uintNode, longNode, ulongNode, floatNode, doubleNode, charNode, stringNode, voidNode, objectNode,
    booleanLiteralNode, sbyteLiteralNode, byteLiteralNode, shortLiteralNode, ushortLiteralNode, intLiteralNode, uintLiteralNode, longLiteralNode, ulongLiteralNode,
    floatLiteralNode, doubleLiteralNode, charLiteralNode, stringLiteralNode, nullLiteralNode,
    identifierNode, parameterNode, functionGroupIdNode, functionNode, namespaceNode, compileUnitNode, aliasNode, namespaceImportNode,
    disjunctionNode, conjunctionNode, bitOrNode, bitXorNode, bitAndNode, equalNode, notEqualNode, lessNode, greaterNode, lessOrEqualNode, greaterOrEqualNode,
    shiftLeftNode, shiftRightNode, addNode, subNode, mulNode, divNode, remNode, notNode, unaryPlusNode, unaryMinusNode, complementNode, isNode, asNode, dotNode,
    invokeNode, castNode, classNode, newNode, memberVariableNode, staticConstructorNode, constructorNode, memberFunctionNode, baseInitializerNode, thisInitializerNode,
    labelNode, thisNode, baseNode,
    compoundStatementNode, returnStatementNode, ifStatementNode, whileStatementNode, doStatementNode, forStatementNode, breakStatementNode, continueStatementNode, 
    constructionStatementNode, assignmentStatementNode, expressionStatementNode, emptyStatementNode, incrementStatementNode, decrementStatementNode,
    maxNode
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
    virtual std::string ToString() const { return ""; }
    Span& GetSpan() { return span; }
    const Span& GetSpan() const { return span; }
    Node* Parent() const { return parent; }
    void SetParent(Node* parent_) { parent = parent_; }
    virtual void AddArgument(Node* argument);
    virtual void AddParameter(ParameterNode* parameter);
    virtual bool IsBreakEnclosingStatementNode() const { return false; }
    virtual bool IsContinueEnclosingStatementNode() const { return false; }
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

template<typename T>
class NodeList
{
public:
    int Count() const
    {
        return static_cast<int>(nodes.size());
    }
    T* operator[](int index) const
    {
        return nodes[index].get();
    }
    void Add(T* node)
    {
        nodes.push_back(std::unique_ptr<T>(node));
    }
    void SetParent(Node* parent)
    {
        for (const std::unique_ptr<T>& node : nodes)
        {
            node->SetParent(parent);
        }
    }
private:
    std::vector<std::unique_ptr<T>> nodes;
}; 

} } // namespace cminor::ast

#endif // CMINOR_AST_NODE_INCLUDED
