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

class DisjunctionNode;
class ConjunctionNode;

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

    virtual void Visit(DisjunctionNode& disjunctionNode) {}
    virtual void Visit(ConjunctionNode& conjunctionNode) {}
};

} } // namespace cminor::ast

#endif // CMINOR_AST_VISITOR_INCLUDED
