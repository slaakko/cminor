// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/ExpressionBinder.hpp>
#include <cminor/binder/BoundCompileUnit.hpp>
#include <cminor/ast/Visitor.hpp>
#include <cminor/ast/Literal.hpp>

namespace cminor { namespace binder {

class ExpressionBinder : public Visitor
{
public:
    ExpressionBinder(BoundCompileUnit& boundCompileUnit_, ContainerScope* containerScope_);
    std::unique_ptr<BoundExpression> GetExpression() { return std::move(expression); }

    void Visit(BooleanLiteralNode& booleanLiteralNode) override;
    void Visit(SByteLiteralNode& sbyteLiteralNode) override;
    void Visit(ByteLiteralNode& byteLiteralNode) override;
    void Visit(ShortLiteralNode& shortLiteralNode) override;
    void Visit(UShortLiteralNode& ushortLiteralNode) override;
    void Visit(IntLiteralNode& intLiteralNode) override;
    void Visit(UIntLiteralNode& uintLiteralNode) override;
    void Visit(LongLiteralNode& longLiteralNode) override;
    void Visit(ULongLiteralNode& ulongLiteralNode) override;
    void Visit(FloatLiteralNode& floatLiteralNode) override;
    void Visit(DoubleLiteralNode& doubleLiteralNode) override;
    void Visit(CharLiteralNode& charLiteralNode) override;
    void Visit(StringLiteralNode& stringLiteralNode) override;
    void Visit(NullLiteralNode& nullLiteralNode) override;

    void Visit(IdentifierNode& identifierNode) override;
private:
    BoundCompileUnit& boundCompileUnit;
    ContainerScope* containerScope;
    std::unique_ptr<BoundExpression> expression;
};

ExpressionBinder::ExpressionBinder(BoundCompileUnit& boundCompileUnit_, ContainerScope* containerScope_) : boundCompileUnit(boundCompileUnit_), containerScope(containerScope_)
{
}

void ExpressionBinder::Visit(BooleanLiteralNode& booleanLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"bool");
    Constant value(IntegralValue(booleanLiteralNode.Value(), ValueType::boolType));
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(SByteLiteralNode& sbyteLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"sbyte");
    Constant value(IntegralValue(sbyteLiteralNode.Value(), ValueType::sbyteType));
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(ByteLiteralNode& byteLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"byte");
    Constant value(IntegralValue(byteLiteralNode.Value(), ValueType::byteType));
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(ShortLiteralNode& shortLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"short");
    Constant value(IntegralValue(shortLiteralNode.Value(), ValueType::shortType));
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(UShortLiteralNode& ushortLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"ushort");
    Constant value(IntegralValue(ushortLiteralNode.Value(), ValueType::ushortType));
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(IntLiteralNode& intLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"int");
    Constant value(IntegralValue(intLiteralNode.Value(), ValueType::intType));
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(UIntLiteralNode& uintLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"uint");
    Constant value(IntegralValue(uintLiteralNode.Value(), ValueType::uintType));
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(LongLiteralNode& longLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"long");
    Constant value(IntegralValue(longLiteralNode.Value(), ValueType::longType));
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(ULongLiteralNode& ulongLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"ulong");
    Constant value(IntegralValue(ulongLiteralNode.Value(), ValueType::ulongType));
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(FloatLiteralNode& floatLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"float");
#pragma warning(disable : 4244)
    Constant value(IntegralValue(floatLiteralNode.Value(), ValueType::floatType));
#pragma warning(default : 4244)
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(DoubleLiteralNode& doubleLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"double");
#pragma warning(disable : 4244)
    Constant value(IntegralValue(doubleLiteralNode.Value(), ValueType::doubleType));
#pragma warning(default : 4244)
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(CharLiteralNode& charLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"char");
    Constant value(IntegralValue(charLiteralNode.Value(), ValueType::charType));
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(StringLiteralNode& stringLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"string");
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(StringPtr(stringLiteralNode.Value().c_str()));
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(NullLiteralNode& nullLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"@nullref");
    ObjectReference nullReference;
    Constant value(IntegralValue(nullReference.Value(), ValueType::objectReference));
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(IdentifierNode& identifierNode)
{
    utf32_string str = ToUtf32(identifierNode.Str());
    Symbol* symbol = containerScope->Lookup(StringPtr(str.c_str()), ScopeLookup::this_and_base_and_parent);
    if (!symbol)
    {
        for (const std::unique_ptr<FileScope>& fileScope : boundCompileUnit.FileScopes())
        {
            symbol = fileScope->Lookup(StringPtr(str.c_str()));
            if (symbol)
            {
                break;
            }
        }
    }
    if (symbol)
    {
        switch (symbol->GetSymbolType())
        {
            case SymbolType::classTypeSymbol: /* todo */
            case SymbolType::stringTypeSymbol: /* todo */
            case SymbolType::functionGroupSymbol: /* todo */
            case SymbolType::parameterSymbol: 
            {
                ParameterSymbol* parameterSymbol = static_cast<ParameterSymbol*>(symbol);
                expression.reset(new BoundParameter(boundCompileUnit.GetAssembly(), parameterSymbol->GetType(), parameterSymbol));
                break;
            }
            case SymbolType::localVariableSymbol:
            {
                LocalVariableSymbol* localVariableSymbol = static_cast<LocalVariableSymbol*>(symbol);
                expression.reset(new BoundLocalVariable(boundCompileUnit.GetAssembly(), localVariableSymbol->GetType(), localVariableSymbol));
                break;
            }
            case SymbolType::memberVariableSymbol:
            {
                MemberVariableSymbol* memberVariableSymbol = static_cast<MemberVariableSymbol*>(symbol);
                expression.reset(new BoundMemberVariable(boundCompileUnit.GetAssembly(), memberVariableSymbol->GetType(), memberVariableSymbol));
                break;
            }
            case SymbolType::constantSymbol:
            {
                ConstantSymbol* constantSymbol = static_cast<ConstantSymbol*>(symbol);
                expression.reset(new BoundConstant(boundCompileUnit.GetAssembly(), constantSymbol->GetType(), constantSymbol));
                break;
            }
            case SymbolType::namespaceSymbol:
            {
                NamespaceSymbol* ns = static_cast<NamespaceSymbol*>(symbol);
                expression.reset(new BoundNamespace(boundCompileUnit.GetAssembly(), ns));
                break;
            }
            default: 
            {
                throw Exception("could not bind '" + ToUtf8(symbol->FullName()) + "'", symbol->GetSpan());
            }
        }
    }
    else
    {
        throw Exception("symbol '" + identifierNode.Str() + "' not found", identifierNode.GetSpan());
    }
}

std::unique_ptr<BoundExpression> BindExpression(BoundCompileUnit& boundCompileUnit, ContainerScope* containerScope, Node* node)
{
    ExpressionBinder expressionBinder(boundCompileUnit, containerScope);
    node->Accept(expressionBinder);
    return expressionBinder.GetExpression();
}

} } // namespace cminor::binder
