// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_VISITOR_INCLUDED
#define CMINOR_AST_VISITOR_INCLUDED

namespace cminor { namespace ast {

class BoolNode;
class SByteNode;
class ByteNode;
class ShortNode;
class UShortNode;
class IntNode;
class UIntNode;
class LongNode;
class ULongNode;
class FloatNode;
class DoubleNode;
class CharNode;
class StringNode;
class VoidNode;
class ObjectNode;

class BooleanLiteralNode;
class SByteLiteralNode;
class ByteLiteralNode;
class ShortLiteralNode;
class UShortLiteralNode;
class IntLiteralNode;
class UIntLiteralNode;
class LongLiteralNode;
class ULongLiteralNode;
class FloatLiteralNode;
class DoubleLiteralNode;
class CharLiteralNode;
class StringLiteralNode;
class NullLiteralNode;

class IdentifierNode;
class ParameterNode;
class FunctionNode;
class FunctionGroupIdNode;
class AttributeMap;
class NamespaceNode;
class CompileUnitNode;
class AliasNode;
class NamespaceImportNode;
class ClassNode;
class InterfaceNode;
class TemplateIdNode;
class TemplateParameterNode;
class StaticConstructorNode;
class BaseInitializerNode;
class ThisInitializerNode;
class ConstructorNode;
class MemberFunctionNode;
class MemberVariableNode;
class PropertyNode;
class IndexerNode;

class DisjunctionNode;
class ConjunctionNode;
class BitOrNode;
class BitXorNode;
class BitAndNode;
class EqualNode;
class NotEqualNode;
class LessNode;
class GreaterNode;
class LessOrEqualNode;
class GreaterOrEqualNode;
class ShiftLeftNode;
class ShiftRightNode;
class AddNode;
class SubNode;
class MulNode;
class DivNode;
class RemNode;
class NotNode;
class UnaryPlusNode;
class UnaryMinusNode;
class ComplementNode;
class IsNode;
class AsNode;
class DotNode;
class ArrayNode;
class RefTypeExprNode;
class RefNode;
class IndexingNode;
class InvokeNode;
class CastNode;
class NewNode;
class ThisNode;
class BaseNode;
class DefaultNode;

class CompoundStatementNode;
class ReturnStatementNode;
class IfStatementNode;
class WhileStatementNode;
class DoStatementNode;
class ForStatementNode;
class BreakStatementNode;
class ContinueStatementNode;
class GotoStatementNode;
class ConstructionStatementNode;
class AssignmentStatementNode;
class ExpressionStatementNode;
class EmptyStatementNode;
class IncrementStatementNode;
class DecrementStatementNode;
class ForEachStatementNode;
class SwitchStatementNode;
class CaseStatementNode;
class DefaultStatementNode;
class GotoCaseStatementNode;
class GotoDefaultStatementNode;
class ThrowStatementNode;
class CatchNode;
class TryStatementNode;
class UsingStatementNode;
class LockStatementNode;

class EnumTypeNode;
class EnumConstantNode;
class ConstantNode;
class DelegateNode;
class ClassDelegateNode;

class Visitor
{
public:
    virtual ~Visitor();
    virtual void Visit(BoolNode& boolNode) {}
    virtual void Visit(SByteNode& sbyteNode) {}
    virtual void Visit(ByteNode& byteNode) {}
    virtual void Visit(ShortNode& shortNode) {}
    virtual void Visit(UShortNode& ushortNode) {}
    virtual void Visit(IntNode& intNode) {}
    virtual void Visit(UIntNode& uintNode) {}
    virtual void Visit(LongNode& longNode) {}
    virtual void Visit(ULongNode& ulongNode) {}
    virtual void Visit(FloatNode& floatNode) {}
    virtual void Visit(DoubleNode& doubleNode) {}
    virtual void Visit(CharNode& charNode) {}
    virtual void Visit(StringNode& stringNode) {}
    virtual void Visit(VoidNode& voidNode) {}
    virtual void Visit(ObjectNode& objectNode) {}

    virtual void Visit(BooleanLiteralNode& booleanLiteralNode) {}
    virtual void Visit(SByteLiteralNode& sbyteLiteralNode) {}
    virtual void Visit(ByteLiteralNode& byteLiteralNode) {}
    virtual void Visit(ShortLiteralNode& shortLiteralNode) {}
    virtual void Visit(UShortLiteralNode& ushortLiteralNode) {}
    virtual void Visit(IntLiteralNode& intLiteralNode) {}
    virtual void Visit(UIntLiteralNode& uintLiteralNode) {}
    virtual void Visit(LongLiteralNode& longLiteralNode) {}
    virtual void Visit(ULongLiteralNode& ulongLiteralNode) {}
    virtual void Visit(FloatLiteralNode& floatLiteralNode) {}
    virtual void Visit(DoubleLiteralNode& doubleLiteralNode) {}
    virtual void Visit(CharLiteralNode& charLiteralNode) {}
    virtual void Visit(StringLiteralNode& stringLiteralNode) {}
    virtual void Visit(NullLiteralNode& nullLiteralNode) {}

    virtual void Visit(IdentifierNode& identifierNode) {}
    virtual void Visit(ParameterNode& parameterNode) {}
    virtual void Visit(FunctionNode& functionNode) {}
    virtual void Visit(FunctionGroupIdNode& functionGroupIdNode) {}
    virtual void Visit(AttributeMap& attributeMap) {}
    virtual void Visit(NamespaceNode& namespaceNode) {}
    virtual void Visit(AliasNode& aliasNode) {}
    virtual void Visit(NamespaceImportNode& namespaceImportNode) {}
    virtual void Visit(CompileUnitNode& compileUnitNode) {}
    virtual void Visit(ClassNode& classNode) {}
    virtual void Visit(InterfaceNode& interfaceNode) {}
    virtual void Visit(TemplateIdNode& templateIdNode) {}
    virtual void Visit(TemplateParameterNode& templateParameterNode) {}
    virtual void Visit(StaticConstructorNode& staticConstructorNode) {}
    virtual void Visit(BaseInitializerNode& baseInitializerNode) {}
    virtual void Visit(ThisInitializerNode& thisInitializerNode) {}
    virtual void Visit(ConstructorNode& constructorNode) {}
    virtual void Visit(MemberFunctionNode& memberFunctionNode) {}
    virtual void Visit(MemberVariableNode& memberVariableNode) {}
    virtual void Visit(PropertyNode& propertyNode) {}
    virtual void Visit(IndexerNode& indexerNode) {}

    virtual void Visit(DisjunctionNode& disjunctionNode) {}
    virtual void Visit(ConjunctionNode& conjunctionNode) {}
    virtual void Visit(BitOrNode& bitOrNode) {}
    virtual void Visit(BitXorNode& bitXorNode) {}
    virtual void Visit(BitAndNode& bitAndNode) {}
    virtual void Visit(EqualNode& equalNode) {}
    virtual void Visit(NotEqualNode& notEqualNode) {}
    virtual void Visit(LessNode& lessNode) {}
    virtual void Visit(GreaterNode& greaterNode) {}
    virtual void Visit(LessOrEqualNode& lessOrEqualNode) {}
    virtual void Visit(GreaterOrEqualNode& greaterOrEqualNode) {}
    virtual void Visit(ShiftLeftNode& shiftLeftNode) {}
    virtual void Visit(ShiftRightNode& shiftRightNode) {}
    virtual void Visit(AddNode& addNode) {}
    virtual void Visit(SubNode& subNode) {}
    virtual void Visit(MulNode& mulNode) {}
    virtual void Visit(DivNode& divNode) {}
    virtual void Visit(RemNode& remNode) {}
    virtual void Visit(NotNode& notNode) {}
    virtual void Visit(UnaryPlusNode& unaryPlusNode) {}
    virtual void Visit(UnaryMinusNode& unaryMinusNode) {}
    virtual void Visit(ComplementNode& complementNode) {}
    virtual void Visit(IsNode& isNode) {}
    virtual void Visit(AsNode& asNode) {}
    virtual void Visit(DotNode& dotNode) {}
    virtual void Visit(ArrayNode& arrayNode) {}
    virtual void Visit(RefTypeExprNode& refTypeExprNode) {}
    virtual void Visit(RefNode& refNode) {}
    virtual void Visit(IndexingNode& indexingNode) {}
    virtual void Visit(InvokeNode& invokeNode) {}
    virtual void Visit(CastNode& castNode) {}
    virtual void Visit(NewNode& newNode) {}
    virtual void Visit(ThisNode& thisNode) {}
    virtual void Visit(BaseNode& baseNode) {}
    virtual void Visit(DefaultNode& defaultNode) {}

    virtual void Visit(CompoundStatementNode& compoundStatementNode) {}
    virtual void Visit(ReturnStatementNode& returnStatementNode) {}
    virtual void Visit(IfStatementNode& ifStatementNode) {}
    virtual void Visit(WhileStatementNode& whileStatementNode) {}
    virtual void Visit(DoStatementNode& doStatementNode) {}
    virtual void Visit(ForStatementNode& forStatementNode) {}
    virtual void Visit(BreakStatementNode& breakStatementNode) {}
    virtual void Visit(ContinueStatementNode& continueStatementNode) {}
    virtual void Visit(GotoStatementNode& gotoStatementNode) {}
    virtual void Visit(ConstructionStatementNode& constructionStatementNode) {}
    virtual void Visit(AssignmentStatementNode& assignmentStatementNode) {}
    virtual void Visit(ExpressionStatementNode& expressionStatementNode) {}
    virtual void Visit(EmptyStatementNode& emptyStatementNode) {}
    virtual void Visit(IncrementStatementNode& incrementStatementNode) {}
    virtual void Visit(DecrementStatementNode& decrementStatementNode) {}
    virtual void Visit(ForEachStatementNode& forEachStatementNode) {}
    virtual void Visit(SwitchStatementNode& switchStatementNode) {}
    virtual void Visit(CaseStatementNode& caseStatementNode) {}
    virtual void Visit(DefaultStatementNode& defaultStatementNode) {}
    virtual void Visit(GotoCaseStatementNode& gotoCaseStatementNode) {}
    virtual void Visit(GotoDefaultStatementNode& gotoDefaultStatementNode) {}
    virtual void Visit(ThrowStatementNode& throwStatementNode) {}
    virtual void Visit(CatchNode& catchNode) {}
    virtual void Visit(TryStatementNode& tryStatementNode) {}
    virtual void Visit(UsingStatementNode& usingStatementNode) {}
    virtual void Visit(LockStatementNode& lockStatementNode) {}

    virtual void Visit(EnumTypeNode& enumTypeNode) {}
    virtual void Visit(EnumConstantNode& enumConstantNode) {}
    virtual void Visit(ConstantNode& constantNode) {}
    virtual void Visit(DelegateNode& delegateNode) {}
    virtual void Visit(ClassDelegateNode& classDelegateNode) {}
};

} } // namespace cminor::ast

#endif // CMINOR_AST_VISITOR_INCLUDED
