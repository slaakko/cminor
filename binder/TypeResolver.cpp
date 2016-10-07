// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/TypeResolver.hpp>
#include <cminor/binder/BoundCompileUnit.hpp>
#include <cminor/ast/Visitor.hpp>

namespace cminor { namespace binder {

class TypeResolverVisitor : public Visitor
{
public:
    TypeResolverVisitor(BoundCompileUnit& boundCompileUnit_, ContainerScope* containerScope_);
    TypeSymbol* GetType() const { return type; }
    void Visit(BoolNode& boolNode) override;
    void Visit(SByteNode& sbyteNode) override;
    void Visit(ByteNode& byteNode) override;
    void Visit(ShortNode& shortNode) override;
    void Visit(UShortNode& ushortNode) override;
    void Visit(IntNode& intNode) override;
    void Visit(UIntNode& uintNode) override;
    void Visit(LongNode& longNode) override;
    void Visit(ULongNode& ulongNode) override;
    void Visit(FloatNode& floatNode) override;
    void Visit(DoubleNode& doubleNode) override;
    void Visit(CharNode& charNode) override;
    void Visit(StringNode& stringNode) override;
    void Visit(VoidNode& voidNode) override;
private:
    BoundCompileUnit& boundCompileUnit;
    ContainerScope* containerScope;
    TypeSymbol* type;
};

TypeResolverVisitor::TypeResolverVisitor(BoundCompileUnit& boundCompileUnit_, ContainerScope* containerScope_) : boundCompileUnit(boundCompileUnit_), containerScope(containerScope_), type(nullptr)
{
}

void TypeResolverVisitor::Visit(BoolNode& boolNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"bool");
}

void TypeResolverVisitor::Visit(SByteNode& sbyteNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"sbyte");
}

void TypeResolverVisitor::Visit(ByteNode& byteNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"byte");
}

void TypeResolverVisitor::Visit(ShortNode& shortNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"short");
}

void TypeResolverVisitor::Visit(UShortNode& ushortNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"ushort");
}

void TypeResolverVisitor::Visit(IntNode& intNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"int");
}

void TypeResolverVisitor::Visit(UIntNode& uintNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"uint");
}

void TypeResolverVisitor::Visit(LongNode& longNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"long");
}

void TypeResolverVisitor::Visit(ULongNode& ulongNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"ulong");
}

void TypeResolverVisitor::Visit(FloatNode& floatNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"float");
}

void TypeResolverVisitor::Visit(DoubleNode& doubleNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"double");
}

void TypeResolverVisitor::Visit(CharNode& charNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"char");
}

void TypeResolverVisitor::Visit(StringNode& stringNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"string");
}

void TypeResolverVisitor::Visit(VoidNode& voidNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"void");
}

TypeSymbol* ResolveType(BoundCompileUnit& boundCompileUnit, ContainerScope* containerScope, Node* typeExprNode)
{
    TypeResolverVisitor typeResolver(boundCompileUnit, containerScope);
    typeExprNode->Accept(typeResolver);
    return typeResolver.GetType();
}

} } // namespace cminor::binder
