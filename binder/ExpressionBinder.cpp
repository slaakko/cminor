// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/ExpressionBinder.hpp>
#include <cminor/binder/BoundCompileUnit.hpp>
#include <cminor/binder/BoundExpression.hpp>
#include <cminor/binder/OverloadResolution.hpp>
#include <cminor/binder/Access.hpp>
#include <cminor/binder/BoundFunction.hpp>
#include <cminor/binder/TypeResolver.hpp>
#include <cminor/ast/Visitor.hpp>
#include <cminor/ast/Literal.hpp>
#include <cminor/ast/Expression.hpp>
#include <cminor/machine/Type.hpp>

namespace cminor { namespace binder {

class ExpressionBinder : public Visitor
{
public:
    ExpressionBinder(BoundCompileUnit& boundCompileUnit_, BoundFunction* boundFunction_, ContainerScope* containerScope_);
    std::unique_ptr<BoundExpression> GetExpression() { return std::move(expression); }

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

    void Visit(IdentifierNode& identifierNode) override;
    void Visit(DotNode& dotNode) override;
    void Visit(InvokeNode& invokeNode) override;
    void Visit(CastNode& castNode) override;
    void Visit(NewNode& newNode) override;
private:
    BoundCompileUnit& boundCompileUnit;
    BoundFunction* boundFunction;
    ContainerScope* containerScope;
    std::unique_ptr<BoundExpression> expression;
    void BindSymbol(Symbol* symbol);
    void BindUnaryOp(UnaryNode& unaryNode, StringPtr groupName);
    void BindUnaryOp(BoundExpression* operand, Node& node, StringPtr groupName);
    void BindBinaryOp(BinaryNode& binaryNode, StringPtr groupName);
    void BindBinaryOp(BoundExpression* left, BoundExpression* right, Node& node, StringPtr groupName);
};

ExpressionBinder::ExpressionBinder(BoundCompileUnit& boundCompileUnit_, BoundFunction* boundFunction_, ContainerScope* containerScope_) : 
    boundCompileUnit(boundCompileUnit_), boundFunction(boundFunction_), containerScope(containerScope_)
{
}

void ExpressionBinder::Visit(BoolNode& boolNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"bool");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(SByteNode& sbyteNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"sbyte");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(ByteNode& byteNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"byte");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(ShortNode& shortNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"short");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(UShortNode& ushortNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"ushort");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(IntNode& intNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"int");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(UIntNode& uintNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"uint");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(LongNode& longNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"long");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(ULongNode& ulongNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"ulong");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(FloatNode& floatNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"float");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(DoubleNode& doubleNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"double");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(CharNode& charNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"char");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(StringNode& stringNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"string");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(VoidNode& voidNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"void");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(ObjectNode& objectNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"object");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
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

void ExpressionBinder::BindUnaryOp(BoundExpression* operand, Node& node, StringPtr groupName)
{
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    arguments.push_back(std::unique_ptr<BoundExpression>(operand));
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, operand->GetType()->ClassOrNsScope()));
    std::unique_ptr<BoundFunctionCall> operatorFunCall = ResolveOverload(boundCompileUnit, groupName, functionScopeLookups, arguments, node.GetSpan());
    CheckAccess(boundFunction->GetFunctionSymbol(), operatorFunCall->GetFunctionSymbol());
    expression.reset(operatorFunCall.release());
}

void ExpressionBinder::BindUnaryOp(UnaryNode& unaryNode, StringPtr groupName)
{
    unaryNode.Child()->Accept(*this);
    BoundExpression* operand = expression.release();
    BindUnaryOp(operand, unaryNode, groupName);
}

void ExpressionBinder::BindBinaryOp(BoundExpression* left, BoundExpression* right, Node& node, StringPtr groupName)
{
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    arguments.push_back(std::unique_ptr<BoundExpression>(left));
    arguments.push_back(std::unique_ptr<BoundExpression>(right));
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, left->GetType()->ClassOrNsScope()));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, right->GetType()->ClassOrNsScope()));
    std::unique_ptr<BoundFunctionCall> operatorFunCall = ResolveOverload(boundCompileUnit, groupName, functionScopeLookups, arguments, node.GetSpan());
    CheckAccess(boundFunction->GetFunctionSymbol(), operatorFunCall->GetFunctionSymbol());
    expression.reset(operatorFunCall.release());
}

void ExpressionBinder::BindBinaryOp(BinaryNode& binaryNode, StringPtr groupName)
{
    binaryNode.Left()->Accept(*this);
    BoundExpression* left = expression.release();
    binaryNode.Right()->Accept(*this);
    BoundExpression* right = expression.release();
    BindBinaryOp(left, right, binaryNode, groupName);
}

void ExpressionBinder::Visit(DisjunctionNode& disjunctionNode)
{
    disjunctionNode.Left()->Accept(*this);
    BoundExpression* left = expression.release();
    disjunctionNode.Right()->Accept(*this);
    BoundExpression* right = expression.release();
    if (!dynamic_cast<BoolTypeSymbol*>(left->GetType()))
    {
        throw Exception("left subexpression of disjunction expression is not of Boolean type", disjunctionNode.Left()->GetSpan());
    }
    if (!dynamic_cast<BoolTypeSymbol*>(right->GetType()))
    {
        throw Exception("right subexpression of disjunction expression is not of Boolean type", disjunctionNode.Right()->GetSpan());
    }
    expression.reset(new BoundDisjunction(boundCompileUnit.GetAssembly(), left, right));
}

void ExpressionBinder::Visit(ConjunctionNode& conjunctionNode)
{
    conjunctionNode.Left()->Accept(*this);
    BoundExpression* left = expression.release();
    conjunctionNode.Right()->Accept(*this);
    BoundExpression* right = expression.release();
    if (!dynamic_cast<BoolTypeSymbol*>(left->GetType()))
    {
        throw Exception("left subexpression of conjunction expression is not of Boolean type", conjunctionNode.Left()->GetSpan());
    }
    if (!dynamic_cast<BoolTypeSymbol*>(right->GetType()))
    {
        throw Exception("right subexpression of conjunction expression is not of Boolean type", conjunctionNode.Right()->GetSpan());
    }
    expression.reset(new BoundConjunction(boundCompileUnit.GetAssembly(), left, right));
}

void ExpressionBinder::Visit(BitOrNode& bitOrNode)
{
    BindBinaryOp(bitOrNode, U"operator|");
}

void ExpressionBinder::Visit(BitXorNode& bitXorNode)
{
    BindBinaryOp(bitXorNode, U"operator^");
}

void ExpressionBinder::Visit(BitAndNode& bitAndNode)
{
    BindBinaryOp(bitAndNode, U"operator&");
}

void ExpressionBinder::Visit(EqualNode& equalNode)
{
    BindBinaryOp(equalNode, U"operator==");
}

void ExpressionBinder::Visit(NotEqualNode& notEqualNode)
{
    BindBinaryOp(notEqualNode, U"operator==");
    BindUnaryOp(expression.release(), notEqualNode, U"operator!");
}

void ExpressionBinder::Visit(LessNode& lessNode)
{
    BindBinaryOp(lessNode, U"operator<");
}

void ExpressionBinder::Visit(GreaterNode& greaterNode)
{
    greaterNode.Left()->Accept(*this);
    BoundExpression* left = expression.release();
    greaterNode.Right()->Accept(*this);
    BoundExpression* right = expression.release();
    BindBinaryOp(right, left, greaterNode, U"operator<");
}

void ExpressionBinder::Visit(LessOrEqualNode& lessOrEqualNode)
{
    lessOrEqualNode.Left()->Accept(*this);
    BoundExpression* left = expression.release();
    lessOrEqualNode.Right()->Accept(*this);
    BoundExpression* right = expression.release();
    BindBinaryOp(right, left, lessOrEqualNode, U"operator<");
    BindUnaryOp(expression.release(), lessOrEqualNode, U"operator!");
}

void ExpressionBinder::Visit(GreaterOrEqualNode& greaterOrEqualNode)
{
    BindBinaryOp(greaterOrEqualNode, U"operator<");
    BindUnaryOp(expression.release(), greaterOrEqualNode, U"operator!");
}

void ExpressionBinder::Visit(ShiftLeftNode& shiftLeftNode)
{
    BindBinaryOp(shiftLeftNode, U"operator<<");
}

void ExpressionBinder::Visit(ShiftRightNode& shiftRightNode)
{
    BindBinaryOp(shiftRightNode, U"operator>>");
}

void ExpressionBinder::Visit(AddNode& addNode)
{
    BindBinaryOp(addNode, U"operator+");
}

void ExpressionBinder::Visit(SubNode& subNode)
{
    BindBinaryOp(subNode, U"operator-");
}

void ExpressionBinder::Visit(MulNode& mulNode)
{
    BindBinaryOp(mulNode, U"operator*");
}

void ExpressionBinder::Visit(DivNode& divNode)
{
    BindBinaryOp(divNode, U"operator/");
}

void ExpressionBinder::Visit(RemNode& remNode)
{
    BindBinaryOp(remNode, U"operator%");
}

void ExpressionBinder::Visit(NotNode& notNode)
{
    BindUnaryOp(notNode, U"operator!");
}

void ExpressionBinder::Visit(UnaryPlusNode& unaryPlusNode)
{
    BindUnaryOp(unaryPlusNode, U"operator+");
}

void ExpressionBinder::Visit(UnaryMinusNode& unaryMinusNode)
{
    BindUnaryOp(unaryMinusNode, U"operator-");
}

void ExpressionBinder::Visit(ComplementNode& complementNode)
{
    BindUnaryOp(complementNode, U"operator~");
}

void ExpressionBinder::BindSymbol(Symbol* symbol)
{
    switch (symbol->GetSymbolType())
    {
        case SymbolType::classTypeSymbol: /* todo */
        case SymbolType::stringTypeSymbol: /* todo */
        case SymbolType::functionGroupSymbol: 
        {
            FunctionGroupSymbol* functionGroupSymbol = static_cast<FunctionGroupSymbol*>(symbol);
            expression.reset(new BoundFunctionGroupExpression(boundCompileUnit.GetAssembly(), functionGroupSymbol));
            break;
        }
        case SymbolType::parameterSymbol:
        {
            ParameterSymbol* parameterSymbol = static_cast<ParameterSymbol*>(symbol);
            CheckAccess(boundFunction->GetFunctionSymbol(), parameterSymbol);
            expression.reset(new BoundParameter(boundCompileUnit.GetAssembly(), parameterSymbol->GetType(), parameterSymbol));
            break;
        }
        case SymbolType::localVariableSymbol:
        {
            LocalVariableSymbol* localVariableSymbol = static_cast<LocalVariableSymbol*>(symbol);
            CheckAccess(boundFunction->GetFunctionSymbol(), localVariableSymbol);
            expression.reset(new BoundLocalVariable(boundCompileUnit.GetAssembly(), localVariableSymbol->GetType(), localVariableSymbol));
            break;
        }
        case SymbolType::memberVariableSymbol:
        {
            MemberVariableSymbol* memberVariableSymbol = static_cast<MemberVariableSymbol*>(symbol);
            CheckAccess(boundFunction->GetFunctionSymbol(), memberVariableSymbol);
            expression.reset(new BoundMemberVariable(boundCompileUnit.GetAssembly(), memberVariableSymbol->GetType(), memberVariableSymbol));
            break;
        }
        case SymbolType::constantSymbol:
        {
            ConstantSymbol* constantSymbol = static_cast<ConstantSymbol*>(symbol);
            CheckAccess(boundFunction->GetFunctionSymbol(), constantSymbol);
            expression.reset(new BoundConstant(boundCompileUnit.GetAssembly(), constantSymbol->GetType(), constantSymbol));
            break;
        }
        case SymbolType::namespaceSymbol:
        {
            NamespaceSymbol* ns = static_cast<NamespaceSymbol*>(symbol);
            expression.reset(new BoundNamespaceExpression(boundCompileUnit.GetAssembly(), ns));
            break;
        }
        default:
        {
            throw Exception("could not bind '" + ToUtf8(symbol->FullName()) + "'", symbol->GetSpan());
        }
    }
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
        BindSymbol(symbol);
    }
    else
    {
        throw Exception("symbol '" + identifierNode.Str() + "' not found", identifierNode.GetSpan());
    }
}

void ExpressionBinder::Visit(DotNode& dotNode)
{
    ContainerScope* prevContainerScope = containerScope;
    dotNode.Child()->Accept(*this);
    if (BoundNamespaceExpression* bns = dynamic_cast<BoundNamespaceExpression*>(expression.get()))
    {
        containerScope = bns->Ns()->GetContainerScope();
        utf32_string str = ToUtf32(dotNode.MemberStr());
        Symbol* symbol = containerScope->Lookup(StringPtr(str.c_str()), ScopeLookup::this_);
        if (symbol)
        {
            BindSymbol(symbol);
        }
        else
        {
            throw Exception("symbol '" + dotNode.MemberStr() + "' not found from namespace '" + ToUtf8(bns->Ns()->FullName()) + "'", dotNode.MemberId()->GetSpan());
        }
    }
    else
    {
        throw Exception("expression '" + dotNode.Child()->ToString() + "' must denote a namespace etc.", dotNode.Child()->GetSpan());
    }
    containerScope = prevContainerScope;
}

void ExpressionBinder::Visit(InvokeNode& invokeNode)
{
    invokeNode.Child()->Accept(*this);
    if (BoundFunctionGroupExpression* bfe = dynamic_cast<BoundFunctionGroupExpression*>(expression.get()))
    {
        FunctionGroupSymbol* functionGroupSymbol = bfe->FunctionGroup();
        std::vector<std::unique_ptr<BoundExpression>> arguments;
        std::vector<FunctionScopeLookup> functionScopeLookups;
        functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
        int n = invokeNode.Arguments().Count();
        for (int i = 0; i < n; ++i)
        {
            Node* argument = invokeNode.Arguments()[i];
            argument->Accept(*this);
            functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, expression->GetType()->ClassOrNsScope()));
            arguments.push_back(std::unique_ptr<BoundExpression>(expression.release()));
        }
        functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
        std::unique_ptr<BoundFunctionCall> functionCall = ResolveOverload(boundCompileUnit, functionGroupSymbol->Name(), functionScopeLookups, arguments, invokeNode.GetSpan());
        CheckAccess(boundFunction->GetFunctionSymbol(), functionCall->GetFunctionSymbol());
        expression.reset(functionCall.release());
    }
    else
    {
        throw Exception("invoke cannot be applied to this type of expression", invokeNode.Child()->GetSpan());
    }
}

void ExpressionBinder::Visit(CastNode& castNode)
{
    TypeSymbol* targetType = ResolveType(boundCompileUnit, containerScope, castNode.TargetTypeExpr());
    castNode.SourceExpr()->Accept(*this);
    if (targetType != expression->GetType())
    {
        std::vector<std::unique_ptr<BoundExpression>> targetExprArgs;
        targetExprArgs.push_back(std::unique_ptr<BoundExpression>(new BoundTypeExpression(boundCompileUnit.GetAssembly(), targetType)));
        std::vector<FunctionScopeLookup> functionScopeLookups;
        functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
        functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, targetType->ClassOrNsScope()));
        functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
        std::unique_ptr<BoundFunctionCall> castFunctionCall = ResolveOverload(boundCompileUnit, U"@return", functionScopeLookups, targetExprArgs, castNode.GetSpan());
        std::vector<std::unique_ptr<BoundExpression>> castArguments;
        castArguments.push_back(std::move(expression));
        FunctionMatch functionMatch(castFunctionCall->GetFunctionSymbol());
        bool conversionFound = FindConversions(boundCompileUnit, castFunctionCall->GetFunctionSymbol(), castArguments, functionMatch, ConversionType::explicit_);
        if (conversionFound)
        {
            Assert(!functionMatch.argumentMatches.empty(), "argument match expected");
            FunctionSymbol* conversionFun = functionMatch.argumentMatches[0].conversionFun;
            if (conversionFun)
            {
                castArguments[0].reset(new BoundConversion(boundCompileUnit.GetAssembly(), std::unique_ptr<BoundExpression>(castArguments[0].release()), conversionFun));
            }
            castFunctionCall->SetArguments(std::move(castArguments));
        }
        else
        {
            throw Exception("no explicit conversion from '" + ToUtf8(castArguments[0]->GetType()->FullName()) + "' to '" + ToUtf8(targetType->FullName()) + "' exists",
                castNode.GetSpan(), boundFunction->GetFunctionSymbol()->GetSpan());
        }
        CheckAccess(boundFunction->GetFunctionSymbol(), castFunctionCall->GetFunctionSymbol());
        expression.reset(castFunctionCall.release());
    }
}

void ExpressionBinder::Visit(NewNode& newNode)
{
    TypeSymbol* type = ResolveType(boundCompileUnit, containerScope, newNode.TypeExpr());
    if (type->IsAbstract())
    {
        throw Exception("cannot instantiate an abstract class", newNode.GetSpan(), type->GetSpan());
    }
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    arguments.push_back(std::unique_ptr<BoundExpression>(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type)));
    int n = newNode.Arguments().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* argument = newNode.Arguments()[i];
        argument->Accept(*this);
        arguments.push_back(std::unique_ptr<BoundExpression>(expression.release()));
    }
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, type->ClassOrNsScope()));
    std::unique_ptr<BoundFunctionCall> constructorCall = ResolveOverload(boundCompileUnit, U"@constructor", functionScopeLookups, arguments, newNode.GetSpan());
    CheckAccess(boundFunction->GetFunctionSymbol(), constructorCall->GetFunctionSymbol());
    expression.reset(new BoundNewExpression(constructorCall.get(), type));
}

std::unique_ptr<BoundExpression> BindExpression(BoundCompileUnit& boundCompileUnit, BoundFunction* boundFunction, ContainerScope* containerScope, Node* node)
{
    ExpressionBinder expressionBinder(boundCompileUnit, boundFunction, containerScope);
    node->Accept(expressionBinder);
    return expressionBinder.GetExpression();
}

} } // namespace cminor::binder
