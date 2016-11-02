// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_CLASS_INCLUDED
#define CMINOR_AST_CLASS_INCLUDED
#include <cminor/ast/Function.hpp>

namespace cminor { namespace ast {

class ClassNode : public Node
{
public:
    ClassNode(const Span& span_);
    ClassNode(const Span& span_, Specifiers specifiers_, IdentifierNode* id_);
    NodeType GetNodeType() const override { return NodeType::classNode; }
    void AddBaseClassOrInterface(Node* baseClassOrInterface);
    void AddMember(Node* member);
    Node* Clone(CloneContext& cloneContext) const override;
    Specifiers GetSpecifiers() const { return specifiers; }
    IdentifierNode* Id() const { return id.get(); }
    const NodeList<Node>& BaseClassOrInterfaces() const { return baseClassOrInterfaces; }
    const NodeList<Node>& Members() const { return members; }
    void Accept(Visitor& visitor) override;
private:
    Specifiers specifiers;
    std::unique_ptr<IdentifierNode> id;
    NodeList<Node> baseClassOrInterfaces;
    NodeList<Node> members;
};

class StaticConstructorNode : public FunctionNode
{
public:
    StaticConstructorNode(const Span& span_);
    StaticConstructorNode(const Span& span_, Specifiers specifiers_);
    NodeType GetNodeType() const override { return NodeType::staticConstructorNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class InitializerNode : public Node
{
public:
    InitializerNode(const Span& span_);
    void AddArgument(Node* argument) override;
    const NodeList<Node>& Arguments() const { return arguments; }
private:
    NodeList<Node> arguments;
};

class BaseInitializerNode : public InitializerNode
{
public:
    BaseInitializerNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::baseInitializerNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class ThisInitializerNode : public InitializerNode
{
public:
    ThisInitializerNode(const Span& span_);
    NodeType GetNodeType() const override { return NodeType::thisInitializerNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class ConstructorNode : public FunctionNode
{
public:
    ConstructorNode(const Span& span_);
    ConstructorNode(const Span& span_, Specifiers specifiers_);
    NodeType GetNodeType() const override { return NodeType::constructorNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void SetInitializer(InitializerNode* initializer_);
    InitializerNode* Initializer() const { return initializer.get(); }
private:
    std::unique_ptr<InitializerNode> initializer;
};

class MemberFunctionNode : public FunctionNode
{
public:
    MemberFunctionNode(const Span& span_);
    MemberFunctionNode(const Span& span_, Specifiers specifiers_, Node* returnTypeExpr_, FunctionGroupIdNode* groupId_);
    NodeType GetNodeType() const override { return NodeType::memberFunctionNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class MemberVariableNode : public Node
{
public:
    MemberVariableNode(const Span& span_);
    MemberVariableNode(const Span& span_, Specifiers specifiers_, Node* typeExpr_, IdentifierNode* id_);
    NodeType GetNodeType() const override { return NodeType::memberVariableNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    Specifiers GetSpecifiers() const { return specifiers; }
    Node* TypeExpr() const { return typeExpr.get(); }
    IdentifierNode* Id() const { return id.get(); }
private:
    Specifiers specifiers;
    std::unique_ptr<Node> typeExpr;
    std::unique_ptr<IdentifierNode> id;
};

class PropertyNode : public Node
{
public:
    PropertyNode(const Span& span_);
    PropertyNode(const Span& span_, Specifiers specifiers_, Node* typeExpr_, IdentifierNode* id_);
    NodeType GetNodeType() const override { return NodeType::propertyNode; };
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    Specifiers GetSpecifiers() const { return specifiers; }
    Node* TypeExpr() const { return typeExpr.get(); }
    IdentifierNode* Id() const { return id.get(); }
    void SetGetter(CompoundStatementNode* getter_) override;
    void SetSetter(CompoundStatementNode* setter_) override;
    CompoundStatementNode* Getter() const { return getter.get(); }
    CompoundStatementNode* Setter() const { return setter.get(); }
private:
    Specifiers specifiers;
    std::unique_ptr<Node> typeExpr;
    std::unique_ptr<IdentifierNode> id;
    std::unique_ptr<CompoundStatementNode> getter;
    std::unique_ptr<CompoundStatementNode> setter;
};

class IndexerNode : public Node
{
public:
    IndexerNode(const Span& span_);
    IndexerNode(const Span& span_, Specifiers specifiers_, Node* valueTypeExpr_, Node* indexTypeExpr_, IdentifierNode* id_);
    NodeType GetNodeType() const override { return NodeType::indexerNode; }
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    Specifiers GetSpecifiers() const { return specifiers; }
    Node* ValueTypeExpr() const { return valueTypeExpr.get(); }
    Node* IndexTypeExpr() const { return indexTypeExpr.get(); }
    IdentifierNode* Id() const { return id.get(); }
    void SetGetter(CompoundStatementNode* getter_) override;
    void SetSetter(CompoundStatementNode* setter_) override;
    CompoundStatementNode* Getter() const { return getter.get(); }
    CompoundStatementNode* Setter() const { return setter.get(); }
private:
    Specifiers specifiers;
    std::unique_ptr<Node> valueTypeExpr;
    std::unique_ptr<Node> indexTypeExpr;
    std::unique_ptr<IdentifierNode> id;
    std::unique_ptr<CompoundStatementNode> getter;
    std::unique_ptr<CompoundStatementNode> setter;
};

} } // namespace cminor::ast

#endif // CMINOR_AST_CLASS_INCLUDED
