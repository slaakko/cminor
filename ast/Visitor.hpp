// =================================
// Copyright (c) 2016 Seppo Laakko
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

class CompoundStatementNode;
class ConstructionStatementNode;
class AssignmentStatementNode;

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

    virtual void Visit(CompoundStatementNode& compoundStatementNode) {}
    virtual void Visit(ConstructionStatementNode& constructionStatementNode) {}
    virtual void Visit(AssignmentStatementNode& assignmentStatementNode) {}
};

} } // namespace cminor::ast

#endif // CMINOR_AST_VISITOR_INCLUDED
