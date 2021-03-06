// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/TypeResolver.hpp>
#include <cminor/binder/BoundCompileUnit.hpp>
#include <cminor/binder/ExpressionBinder.hpp>
#include <cminor/ast/Expression.hpp>
#include <cminor/ast/Visitor.hpp>
#include <cminor/util/Unicode.hpp>

namespace cminor { namespace binder {

using namespace cminor::unicode;

class NamespaceTypeSymbol : public TypeSymbol
{
public:
    NamespaceTypeSymbol(NamespaceSymbol* ns_) : TypeSymbol(ns_->GetSpan(), ns_->NameConstant()), ns(ns_) {}
    SymbolType GetSymbolType() const override { return SymbolType::namespaceSymbol; }
    NamespaceSymbol* Ns() const { return ns; }
    Type* GetMachineType() const override { Assert(false, "cannot get machine type from namespace type");  return nullptr; }
    bool IsInComplete() const override { return true; }
private:
    NamespaceSymbol* ns;
};

class TypeResolverVisitor : public Visitor
{
public:
    TypeResolverVisitor(BoundCompileUnit& boundCompileUnit_, ContainerScope* containerScope_, TypeResolverFlags flags_);
    TypeSymbol* GetType() const { Assert(type, "type not set");  return type; }
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
    void Visit(ObjectNode& objectNode) override;
    void Visit(IdentifierNode& identifierNode) override;
    void Visit(TemplateIdNode& templateIdNode) override;
    void Visit(DotNode& dotNode) override;
    void Visit(ArrayNode& arrayNode) override;
    void Visit(RefTypeExprNode& refTypeExprNode) override;
private:
    BoundCompileUnit& boundCompileUnit;
    ContainerScope* containerScope;
    TypeSymbol* type;
    std::unique_ptr<NamespaceTypeSymbol> nsTypeSymbol;
    TypeResolverFlags flags;
    void ResolveSymbol(Node& node, Symbol* symbol);
};

TypeResolverVisitor::TypeResolverVisitor(BoundCompileUnit& boundCompileUnit_, ContainerScope* containerScope_, TypeResolverFlags flags_) : 
    boundCompileUnit(boundCompileUnit_), containerScope(containerScope_), type(nullptr), flags(flags_)
{
}

void TypeResolverVisitor::Visit(BoolNode& boolNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Boolean");
}

void TypeResolverVisitor::Visit(SByteNode& sbyteNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Int8");
}

void TypeResolverVisitor::Visit(ByteNode& byteNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.UInt8");
}

void TypeResolverVisitor::Visit(ShortNode& shortNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Int16");
}

void TypeResolverVisitor::Visit(UShortNode& ushortNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.UInt16");
}

void TypeResolverVisitor::Visit(IntNode& intNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Int32");
}

void TypeResolverVisitor::Visit(UIntNode& uintNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.UInt32");
}

void TypeResolverVisitor::Visit(LongNode& longNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Int64");
}

void TypeResolverVisitor::Visit(ULongNode& ulongNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.UInt64");
}

void TypeResolverVisitor::Visit(FloatNode& floatNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Float");
}

void TypeResolverVisitor::Visit(DoubleNode& doubleNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Double");
}

void TypeResolverVisitor::Visit(CharNode& charNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Char");
}

void TypeResolverVisitor::Visit(StringNode& stringNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.String");
}

void TypeResolverVisitor::Visit(VoidNode& voidNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Void");
}

void TypeResolverVisitor::Visit(ObjectNode& objectNode)
{
    type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Object");
}

void TypeResolverVisitor::ResolveSymbol(Node& node, Symbol* symbol)
{
    if (TypeSymbol* typeSymbol = dynamic_cast<TypeSymbol*>(symbol))
    {
        type = typeSymbol;
    }
    else if (BoundTypeParameterSymbol* boundTypeParameterSymbol = dynamic_cast<BoundTypeParameterSymbol*>(symbol))
    {
        Assert(boundTypeParameterSymbol->GetType(), "type expected");
        type = boundTypeParameterSymbol->GetType();
    }
    else if (NamespaceSymbol* nsSymbol = dynamic_cast<NamespaceSymbol*>(symbol))
    {
        nsTypeSymbol.reset(new NamespaceTypeSymbol(nsSymbol));
        type = nsTypeSymbol.get();
    }
    else
    {
        throw Exception("symbol '" + ToUtf8(symbol->FullName()) + "' does not denote a type", node.GetSpan(), symbol->GetSpan());
    }
}

void TypeResolverVisitor::Visit(IdentifierNode& identifierNode)
{
    std::u32string s = identifierNode.Str();
    Symbol* symbol = containerScope->Lookup(StringPtr(s.c_str()), ScopeLookup::this_and_base_and_parent);
    if (!symbol)
    {
        for (const std::unique_ptr<FileScope>& fileScope : boundCompileUnit.FileScopes())
        {
            symbol = fileScope->Lookup(StringPtr(s.c_str()));
            if (symbol)
            {
                break;
            }
        }
    }
    if (symbol)
    {
        ResolveSymbol(identifierNode, symbol);
    }
    else
    {
        throw Exception("type symbol '" + ToUtf8(identifierNode.Str()) + "' not found", identifierNode.GetSpan());
    }
}

void TypeResolverVisitor::Visit(TemplateIdNode& templateIdNode)
{
    std::vector<TypeSymbol*> typeArguments;
    TypeSymbol* primaryType = ResolveType(boundCompileUnit, containerScope, templateIdNode.Primary());
    boundCompileUnit.GetAssembly().GetConstantPool().Install(primaryType->Name());
    ClassTypeSymbol* primaryClassTemplate = dynamic_cast<ClassTypeSymbol*>(primaryType);
    if (!primaryClassTemplate || !primaryClassTemplate->IsClassTemplate())
    {
        throw Exception("class template expected", templateIdNode.Primary()->GetSpan());
    }
    int n = templateIdNode.TemplateArguments().Count();
    for (int i = 0; i < n; ++i)
    {
        TypeSymbol* typeArgument = ResolveType(boundCompileUnit, containerScope, templateIdNode.TemplateArguments()[i]);
        boundCompileUnit.GetAssembly().GetConstantPool().Install(typeArgument->Name());
        typeArguments.push_back(typeArgument);
    }
    ClassTemplateSpecializationSymbol* classTemplateSpecialization = boundCompileUnit.GetAssembly().GetSymbolTable().MakeClassTemplateSpecialization(primaryClassTemplate, typeArguments, 
        templateIdNode.GetSpan());
    boundCompileUnit.GetClassTemplateRepository().BindClassTemplateSpecialization(classTemplateSpecialization, containerScope);
    type = classTemplateSpecialization;
}

void TypeResolverVisitor::Visit(DotNode& dotNode)
{
    dotNode.Child()->Accept(*this);
    Scope* scope = nullptr;
    if (NamespaceTypeSymbol* nsType = dynamic_cast<NamespaceTypeSymbol*>(type))
    {
        scope = nsType->Ns()->GetContainerScope();
    }
    else if (ClassTypeSymbol* classType = dynamic_cast<ClassTypeSymbol*>(type))
    {
        scope = type->GetContainerScope();
    }
    else
    {
        throw Exception("symbol '" + ToUtf8(type->FullName()) + "' does not denote a class or namespace", dotNode.GetSpan(), type->GetSpan());
    }
    std::u32string s = dotNode.MemberStr();
    Symbol* symbol = scope->Lookup(StringPtr(s.c_str()), ScopeLookup::this_and_base);
    if (symbol)
    {
        ResolveSymbol(dotNode, symbol);
    }
    else
    {
        throw Exception("type symbol '" + ToUtf8(dotNode.MemberStr()) + "' not found", dotNode.GetSpan());
    }
}

void TypeResolverVisitor::Visit(ArrayNode& arrayNode)
{
    arrayNode.Child()->Accept(*this);
    TypeSymbol* elementType = type;
    type = boundCompileUnit.GetAssembly().GetSymbolTable().CreateArrayType(arrayNode, elementType);
}

void TypeResolverVisitor::Visit(RefTypeExprNode& refTypeExprNode)
{
    if ((flags & TypeResolverFlags::parameterType) == TypeResolverFlags::none)
    {
        throw Exception("reference type can be used only as parameter's type", refTypeExprNode.GetSpan());
    }
    refTypeExprNode.Child()->Accept(*this);
    TypeSymbol* baseType = type;
    type = boundCompileUnit.GetAssembly().GetSymbolTable().MakeRefType(refTypeExprNode, baseType);
}

TypeSymbol* ResolveType(BoundCompileUnit& boundCompileUnit, ContainerScope* containerScope, Node* typeExprNode)
{
    return ResolveType(boundCompileUnit, containerScope, typeExprNode, TypeResolverFlags::none);
}

TypeSymbol* ResolveType(BoundCompileUnit& boundCompileUnit, ContainerScope* containerScope, Node* typeExprNode, TypeResolverFlags flags)
{
    TypeResolverVisitor typeResolver(boundCompileUnit, containerScope, flags);
    typeExprNode->Accept(typeResolver);
    TypeSymbol* type = typeResolver.GetType();
    if (type->IsInComplete())
    {
        throw Exception("incomplete type expression", typeExprNode->GetSpan());
    }
    boundCompileUnit.GetAssembly().GetConstantPool().Install(type->Name());
    return type;
}

} } // namespace cminor::binder
