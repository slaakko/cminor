// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_NODE_INCLUDED
#define CMINOR_AST_NODE_INCLUDED
#include <cminor/ast/Clone.hpp>
#include <cminor/pl/Scanner.hpp>

namespace cminor { namespace ast {

using cminor::parsing::Span;
class ParameterNode;
class TemplateParameterNode;
class CompoundStatementNode;
class AstWriter;
class AstReader;

enum class NodeType : uint8_t
{
    boolNode, sbyteNode, byteNode, shortNode, ushortNode, intNode, uintNode, longNode, ulongNode, floatNode, doubleNode, charNode, stringNode, voidNode, objectNode,
    booleanLiteralNode, sbyteLiteralNode, byteLiteralNode, shortLiteralNode, ushortLiteralNode, intLiteralNode, uintLiteralNode, longLiteralNode, ulongLiteralNode,
    floatLiteralNode, doubleLiteralNode, charLiteralNode, stringLiteralNode, nullLiteralNode,
    identifierNode, parameterNode, functionGroupIdNode, functionNode, namespaceNode, compileUnitNode, aliasNode, namespaceImportNode,
    disjunctionNode, conjunctionNode, bitOrNode, bitXorNode, bitAndNode, equalNode, notEqualNode, lessNode, greaterNode, lessOrEqualNode, greaterOrEqualNode,
    shiftLeftNode, shiftRightNode, addNode, subNode, mulNode, divNode, remNode, notNode, unaryPlusNode, unaryMinusNode, complementNode, isNode, asNode, dotNode, arrayNode,
    indexingNode, invokeNode, castNode, classNode, interfaceNode, newNode, memberVariableNode, propertyNode, indexerNode, staticConstructorNode, constructorNode, memberFunctionNode,
    baseInitializerNode, thisInitializerNode, labelNode, thisNode, baseNode, templateIdNode, templateParameterNode,
    compoundStatementNode, returnStatementNode, ifStatementNode, whileStatementNode, doStatementNode, forStatementNode, breakStatementNode, continueStatementNode, gotoStatementNode,
    constructionStatementNode, assignmentStatementNode, expressionStatementNode, emptyStatementNode, incrementStatementNode, decrementStatementNode, forEachStatementNode, 
    switchStatementNode, caseStatementNode, defaultStatementNode, gotoCaseStatementNode, gotoDefaultStatementNode, throwStatementNode, tryStatementNode, catchNode, usingStatementNode,
    enumTypeNode, enumConstantNode, constantNode, delegateNode,
    maxNode
};

std::string NodeTypeStr(NodeType nodeType);

class Visitor;

extern const uint32_t noSymbolId;

class Node
{
public:
    Node();
    Node(const Span& span_);
    virtual ~Node();
    virtual NodeType GetNodeType() const = 0;
    virtual Node* Clone(CloneContext& cloneContext) const = 0;
    virtual void Write(AstWriter& writer);
    virtual void Read(AstReader& reader);
    virtual void Accept(Visitor& visitor);
    virtual std::string ToString() const { return ""; }
    Span& GetSpan() { return span; }
    const Span& GetSpan() const { return span; }
    Node* Parent() const { return parent; }
    void SetParent(Node* parent_) { parent = parent_; }
    virtual void AddArgument(Node* argument);
    virtual void AddParameter(ParameterNode* parameter);
    virtual void AddTemplateParameter(TemplateParameterNode* templateParameter);
    virtual void SetGetter(CompoundStatementNode* getter);
    virtual void SetSetter(CompoundStatementNode* setter);
    virtual bool IsBreakEnclosingStatementNode() const { return false; }
    virtual bool IsContinueEnclosingStatementNode() const { return false; }
    virtual bool IsUnsignedTypeNode() const { return false; }
    uint32_t SymbolId() const { return symbolId; }
    void SetSymbolId(uint32_t symbolId_) { symbolId = symbolId_; }
private:
    Span span;
    Node* parent;
    uint32_t symbolId;
};

class UnaryNode : public Node
{
public:
    UnaryNode(const Span& span_);
    UnaryNode(const Span& span_, Node* child_);
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
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
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    Node* Left() const { return left.get(); }
    Node* Right() const { return right.get(); }
    void Accept(Visitor& visitor) override;
private:
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
};

class NodeCreator
{
public:
    virtual ~NodeCreator();
    virtual Node* CreateNode(const Span& span) = 0;
};

class NodeFactory
{
public:
    static void Init();
    static void Done();
    static NodeFactory& Instance();
    void Register(NodeType nodeType, NodeCreator* creator);
    Node* CreateNode(NodeType nodeType, const Span& span);
private:
    static std::unique_ptr<NodeFactory> instance;
    std::vector<std::unique_ptr<NodeCreator>> creators;
    NodeFactory();
};

void NodeInit();
void NodeDone();

} } // namespace cminor::ast

#endif // CMINOR_AST_NODE_INCLUDED
