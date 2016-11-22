// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_CONSTANT_POOL_INSTALLER_VISITOR_INCLUDED
#define CMINOR_BINDER_CONSTANT_POOL_INSTALLER_VISITOR_INCLUDED
#include <cminor/ast/Visitor.hpp>
#include <cminor/machine/Constant.hpp>

namespace cminor { namespace binder {

using namespace cminor::ast;
using namespace cminor::machine;

class ConstantPoolInstallerVisitor : public Visitor
{
public:
    ConstantPoolInstallerVisitor(ConstantPool& constantPool_);
    void Visit(IdentifierNode& identifierNode) override;
    void Visit(StringLiteralNode& stringLiteralNode) override;
    void Visit(ParameterNode& parameterNode) override;
    void Visit(FunctionNode& functionNode) override;
    void Visit(FunctionGroupIdNode& functionGroupIdNode) override;
    void Visit(AttributeMap& attributes) override;
    void Visit(NamespaceNode& namespaceNode) override;
    void Visit(AliasNode& aliasNode) override;
    void Visit(NamespaceImportNode& namespaceImportNode) override;
    void Visit(ClassNode& classNode) override;
    void Visit(InterfaceNode& interfaceNode) override;
    void Visit(TemplateIdNode& templateIdNode) override;
    void Visit(TemplateParameterNode& templateParameterNode) override;
    void Visit(StaticConstructorNode& staticConstructorNode) override;
    void Visit(BaseInitializerNode& baseInitializerNode) override;
    void Visit(ThisInitializerNode& thisInitializerNode) override;
    void Visit(ConstructorNode& constructorNode) override;
    void Visit(MemberFunctionNode& memberFunctionNode) override;
    void Visit(MemberVariableNode& memberVariableNode) override;
    void Visit(PropertyNode& propertyNode) override;
    void Visit(IndexerNode& indexerNode) override;
    void Visit(DisjunctionNode& disjunctionNode) override;
    void Visit(ConjunctionNode& conjunctionNode) override;
    void Visit(BitOrNode& bitOrNode) override;
    void Visit(BitXorNode& bitXorNode) override;
    void Visit(BitAndNode& bitAndNode) override;
    void Visit(EqualNode& equalNode) override;
    void Visit(NotEqualNode& notEqualNode) override;
    void Visit(LessNode& lessNode) override;
    void Visit(GreaterNode& greaterNode) override;
    void Visit(LessOrEqualNode& lessOrEqualNode) override;
    void Visit(GreaterOrEqualNode& greaterOrEqualNode) override;
    void Visit(ShiftLeftNode& shiftLeftNode) override;
    void Visit(ShiftRightNode& shiftRightNode) override;
    void Visit(AddNode& addNode) override;
    void Visit(SubNode& subNode) override;
    void Visit(MulNode& mulNode) override;
    void Visit(DivNode& divNode) override;
    void Visit(RemNode& remNode) override;
    void Visit(NotNode& notNode) override;
    void Visit(UnaryPlusNode& unaryPlusNode) override;
    void Visit(UnaryMinusNode& unaryMinusNode) override;
    void Visit(ComplementNode& complementNode) override;
    void Visit(IsNode& isNode) override;
    void Visit(AsNode& asNode) override;
    void Visit(DotNode& dotNode) override;
    void Visit(ArrayNode& arrayNode) override;
    void Visit(IndexingNode& indexingNode) override;
    void Visit(InvokeNode& invokeNode) override;
    void Visit(CastNode& castNode) override;
    void Visit(NewNode& newNode) override;
    void Visit(CompoundStatementNode& compoundStatementNode) override;
    void Visit(ReturnStatementNode& returnStatementNode) override;
    void Visit(IfStatementNode& ifStatementNode) override;
    void Visit(WhileStatementNode& whileStatementNode) override;
    void Visit(DoStatementNode& doStatementNode) override;
    void Visit(ForStatementNode& forStatementNode) override;
    void Visit(ConstructionStatementNode& constructionStatementNode) override;
    void Visit(AssignmentStatementNode& assignmentStatementNode) override;
    void Visit(ExpressionStatementNode& expressionStatementNode) override;
    void Visit(IncrementStatementNode& incrementStatementNode) override;
    void Visit(DecrementStatementNode& decrementStatementNode) override;
    void Visit(ThrowStatementNode& throwStatementNode) override;
    void Visit(CatchNode& catchNode) override;
    void Visit(TryStatementNode& tryStatementNode) override;
    void Visit(EnumTypeNode& enumTypeNode) override;
    void Visit(EnumConstantNode& enumConstantNode) override;
    void Visit(ConstantNode& constantNode) override;
private:
    ConstantPool& constantPool;
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_CONSTANT_POOL_INSTALLER_VISITOR_INCLUDED
