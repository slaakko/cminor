// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/ExpressionBinder.hpp>
#include <cminor/binder/BoundCompileUnit.hpp>
#include <cminor/binder/BoundExpression.hpp>
#include <cminor/binder/OverloadResolution.hpp>
#include <cminor/binder/Access.hpp>
#include <cminor/binder/BoundFunction.hpp>
#include <cminor/binder/TypeBinderVisitor.hpp>
#include <cminor/binder/TypeResolver.hpp>
#include <cminor/binder/StatementBinderVisitor.hpp>
#include <cminor/symbols/PropertySymbol.hpp>
#include <cminor/symbols/IndexerSymbol.hpp>
#include <cminor/symbols/EnumSymbol.hpp>
#include <cminor/symbols/SymbolCreatorVisitor.hpp>
#include <cminor/symbols/DelegateSymbol.hpp>
#include <cminor/ast/Visitor.hpp>
#include <cminor/ast/Literal.hpp>
#include <cminor/ast/Expression.hpp>
#include <cminor/ast/BasicType.hpp>

namespace cminor { namespace binder {

class ExpressionBinder : public Visitor
{
public:
    ExpressionBinder(BoundCompileUnit& boundCompileUnit_, BoundFunction* boundFunction_, ContainerScope* containerScope_, const Span& span_, bool lvalue);
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
    void Visit(TemplateIdNode& templateIdNode) override;
    void Visit(DotNode& dotNode) override;
    void Visit(IndexingNode& indexingNode) override;
    void Visit(InvokeNode& invokeNode) override;
    void Visit(CastNode& castNode) override;
    void Visit(NewNode& newNode) override;
    void Visit(ThisNode& thisNode) override;
    void Visit(BaseNode& baseNode) override;
    void Visit(DefaultNode& defaultNode) override;

    void Visit(IsNode& isNode) override;
    void Visit(AsNode& asNode) override;
    void Visit(RefNode& refNode) override;
private:
    BoundCompileUnit& boundCompileUnit;
    BoundFunction* boundFunction;
    ContainerScope* containerScope;
    bool lvalue;
    Span span;
    std::unique_ptr<BoundExpression> expression;
    void BindSymbol(Symbol* symbol);
    void BindUnaryOp(UnaryNode& unaryNode, StringPtr groupName);
    void BindUnaryOp(BoundExpression* operand, Node& node, StringPtr groupName);
    void BindBinaryOp(BinaryNode& binaryNode, StringPtr groupName);
    void BindBinaryOp(BoundExpression* left, BoundExpression* right, Node& node, StringPtr groupName);
};

ExpressionBinder::ExpressionBinder(BoundCompileUnit& boundCompileUnit_, BoundFunction* boundFunction_, ContainerScope* containerScope_, const Span& span_, bool lvalue_) :
    boundCompileUnit(boundCompileUnit_), boundFunction(boundFunction_), containerScope(containerScope_), span(span_), lvalue(lvalue_)
{
}

void ExpressionBinder::Visit(BoolNode& boolNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Boolean");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(SByteNode& sbyteNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Int8");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(ByteNode& byteNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.UInt8");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(ShortNode& shortNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Int16");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(UShortNode& ushortNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.UInt16");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(IntNode& intNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Int32");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(UIntNode& uintNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.UInt32");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(LongNode& longNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Int64");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(ULongNode& ulongNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.UInt64");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(FloatNode& floatNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Float");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(DoubleNode& doubleNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Double");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(CharNode& charNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Char");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(StringNode& stringNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.String");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(VoidNode& voidNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Void");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(ObjectNode& objectNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Object");
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(BooleanLiteralNode& booleanLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Boolean");
    Constant value(IntegralValue(booleanLiteralNode.Value(), ValueType::boolType));
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(SByteLiteralNode& sbyteLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Int8");
    Constant value(IntegralValue(sbyteLiteralNode.Value(), ValueType::sbyteType));
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(ByteLiteralNode& byteLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.UInt8");
    Constant value(IntegralValue(byteLiteralNode.Value(), ValueType::byteType));
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(ShortLiteralNode& shortLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Int16");
    Constant value(IntegralValue(shortLiteralNode.Value(), ValueType::shortType));
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(UShortLiteralNode& ushortLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.UInt16");
    Constant value(IntegralValue(ushortLiteralNode.Value(), ValueType::ushortType));
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(IntLiteralNode& intLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Int32");
    Constant value(IntegralValue(intLiteralNode.Value(), ValueType::intType));
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(UIntLiteralNode& uintLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.UInt32");
    Constant value(IntegralValue(uintLiteralNode.Value(), ValueType::uintType));
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(LongLiteralNode& longLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Int64");
    Constant value(IntegralValue(longLiteralNode.Value(), ValueType::longType));
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(ULongLiteralNode& ulongLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.UInt64");
    Constant value(IntegralValue(ulongLiteralNode.Value(), ValueType::ulongType));
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(FloatLiteralNode& floatLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Float");
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
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Double");
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
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Char");
    Constant value(IntegralValue(charLiteralNode.Value(), ValueType::charType));
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(value);
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), type, constantPool.GetConstant(id));
    expression.reset(boundLiteral);
}

void ExpressionBinder::Visit(StringLiteralNode& stringLiteralNode)
{
    TypeSymbol* stringType = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.String");
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    ConstantId id = constantPool.Install(StringPtr(stringLiteralNode.Value().c_str()));
    BoundLiteral* boundLiteral = new BoundLiteral(boundCompileUnit.GetAssembly(), stringType, constantPool.GetConstant(id));
    FunctionSymbol* strlit2s = boundCompileUnit.GetConversion(stringType, stringType);
    Assert(strlit2s, "string literal to string conversion not found");
    BoundConversion* boundConversion = new BoundConversion(boundCompileUnit.GetAssembly(), std::unique_ptr<BoundExpression>(boundLiteral), strlit2s);
    expression.reset(boundConversion);
}

void ExpressionBinder::Visit(NullLiteralNode& nullLiteralNode)
{
    TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.@nullref");
    AllocationHandle nullReference;
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
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
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
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
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
        case SymbolType::functionGroupSymbol: 
        {
            FunctionGroupSymbol* functionGroupSymbol = static_cast<FunctionGroupSymbol*>(symbol);
            BoundFunctionGroupExpression* boundFunctionGroupExpression = new BoundFunctionGroupExpression(boundCompileUnit.GetAssembly(), functionGroupSymbol);
            ParameterSymbol* thisParam = boundFunction->GetFunctionSymbol()->GetThisParam();
            if (thisParam)
            {
                boundFunctionGroupExpression->SetClassObject(std::unique_ptr<BoundExpression>(new BoundParameter(boundCompileUnit.GetAssembly(), thisParam->GetType(), thisParam)));
            }
            expression.reset(boundFunctionGroupExpression);
            break;
        }
        case SymbolType::classTypeSymbol:
        {
            ClassTypeSymbol* classTypeSymbol = static_cast<ClassTypeSymbol*>(symbol);
            CheckAccess(boundFunction->GetFunctionSymbol(), classTypeSymbol);
            expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), classTypeSymbol));
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
            BoundMemberVariable* bmv = new BoundMemberVariable(boundCompileUnit.GetAssembly(), memberVariableSymbol->GetType(), memberVariableSymbol);
            if (memberVariableSymbol->IsStatic())
            {
                bool accessFromOwnScope = false;
                Symbol* s = boundFunction->GetFunctionSymbol();
                ClassTypeSymbol* c = s->ContainingClass();
                if (c)
                {
                    ClassTypeSymbol* cp = memberVariableSymbol->ContainingClass();
                    Assert(cp, "class type symbol expected");
                    if (cp == c)
                    {
                        accessFromOwnScope = true;
                    }
                }
                if (!accessFromOwnScope)
                {
                    bmv->SetStaticInitNeeded();
                }
            }
            else
            {
                ParameterSymbol* thisParam = boundFunction->GetFunctionSymbol()->GetThisParam();
                if (thisParam)
                {
                    bmv->SetClassObject(std::unique_ptr<BoundExpression>(new BoundParameter(boundCompileUnit.GetAssembly(), thisParam->GetType(), thisParam)));
                }
            }
            expression.reset(bmv);
            break;
        }
        case SymbolType::propertySymbol:
        {
            PropertySymbol* propertySymbol = static_cast<PropertySymbol*>(symbol);
            CheckAccess(boundFunction->GetFunctionSymbol(), propertySymbol);
            if (!lvalue)
            {
                if (!propertySymbol->Getter())
                {
                    throw Exception("property '" + ToUtf8(propertySymbol->FullName()) + "' is write-only", span, propertySymbol->GetSpan());
                }
                else
                {
                    if (ClassTemplateSpecializationSymbol* classTemplateSpecialization = dynamic_cast<ClassTemplateSpecializationSymbol*>(propertySymbol->ContainingClass()))
                    {
                        if (!propertySymbol->Getter()->IsInstantiated())
                        {
                            propertySymbol->Getter()->SetInstantiationRequested();
                            boundCompileUnit.GetClassTemplateRepository().Add(classTemplateSpecialization);
                        }
                    }
                }
            }
            else
            {
                if (!propertySymbol->Setter())
                {
                    throw Exception("property '" + ToUtf8(propertySymbol->FullName()) + "' is read-only", span, propertySymbol->GetSpan());
                }
                else
                {
                    if (ClassTemplateSpecializationSymbol* classTemplateSpecialization = dynamic_cast<ClassTemplateSpecializationSymbol*>(propertySymbol->ContainingClass()))
                    {
                        if (!propertySymbol->Setter()->IsInstantiated())
                        {
                            propertySymbol->Setter()->SetInstantiationRequested();
                            boundCompileUnit.GetClassTemplateRepository().Add(classTemplateSpecialization);
                        }
                    }
                }
            }
            BoundProperty* bp = new BoundProperty(boundCompileUnit.GetAssembly(), propertySymbol->GetType(), propertySymbol);
            if (propertySymbol->IsStatic())
            {
                bool accessFromOwnScope = false;
                Symbol* s = boundFunction->GetFunctionSymbol();
                ClassTypeSymbol* c = s->ContainingClass();
                if (c)
                {
                    ClassTypeSymbol* cp = propertySymbol->ContainingClass();
                    Assert(cp, "class type symbol expected");
                    if (cp == c) 
                    {
                        accessFromOwnScope = true;
                    }
                }
                if (!accessFromOwnScope)
                {
                    bp->SetStaticInitNeeded();
                }
            }
            else
            {
                ParameterSymbol* thisParam = boundFunction->GetFunctionSymbol()->GetThisParam();
                if (thisParam)
                {
                    bp->SetClassObject(std::unique_ptr<BoundExpression>(new BoundParameter(boundCompileUnit.GetAssembly(), thisParam->GetType(), thisParam)));
                }
            }
            expression.reset(bp);
            break;
        }
        case SymbolType::constantSymbol:
        {
            ConstantSymbol* constantSymbol = static_cast<ConstantSymbol*>(symbol);
            CheckAccess(boundFunction->GetFunctionSymbol(), constantSymbol);
            expression.reset(new BoundConstant(boundCompileUnit.GetAssembly(), constantSymbol->GetType(), constantSymbol));
            break;
        }
        case SymbolType::enumTypeSymbol:
        {
            EnumTypeSymbol* enumTypeSymbol = static_cast<EnumTypeSymbol*>(symbol);
            CheckAccess(boundFunction->GetFunctionSymbol(), enumTypeSymbol);
            expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), enumTypeSymbol));
            break;
        }
        case SymbolType::enumConstantSymbol:
        {
            EnumConstantSymbol* enumConstantSymbol = static_cast<EnumConstantSymbol*>(symbol);
            Symbol* parent = enumConstantSymbol->Parent();
            EnumTypeSymbol* enumType = dynamic_cast<EnumTypeSymbol*>(parent);
            Assert(enumType, "enum type expected");
            Assert(enumType->GetUnderlyingType(), "underlying type expected");
            expression.reset(new BoundEnumConstant(boundCompileUnit.GetAssembly(), enumType, enumConstantSymbol));
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
            throw Exception("could not bind '" + ToUtf8(symbol->FullName()) + "'", span, symbol->GetSpan());
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

void ExpressionBinder::Visit(TemplateIdNode& templateIdNode)
{
    TypeSymbol* type = ResolveType(boundCompileUnit, containerScope, &templateIdNode);
    CheckAccess(boundFunction->GetFunctionSymbol(), type);
    expression.reset(new BoundTypeExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(DotNode& dotNode)
{
    ContainerScope* prevContainerScope = containerScope;
    expression = std::move(BindExpression(boundCompileUnit, boundFunction, containerScope, dotNode.Child(), false, true, true, true));
    if (BoundNamespaceExpression* bns = dynamic_cast<BoundNamespaceExpression*>(expression.get()))
    {
        containerScope = bns->Ns()->GetContainerScope();
        utf32_string str = ToUtf32(dotNode.MemberStr());
        Symbol* symbol = containerScope->Lookup(StringPtr(str.c_str()), ScopeLookup::this_);
        if (symbol)
        {
            BindSymbol(symbol);
            if (expression->IsBoundFunctionGroupExpression())
            {
                BoundFunctionGroupExpression* bfe = static_cast<BoundFunctionGroupExpression*>(expression.get());
                bfe->SetScopeQualified();
                bfe->SetQualifiedScope(containerScope);
            }
        }
        else
        {
            throw Exception("symbol '" + dotNode.MemberStr() + "' not found from namespace '" + ToUtf8(bns->Ns()->FullName()) + "'", dotNode.MemberId()->GetSpan());
        }
    }
    else
    {
        TypeSymbol* type = expression->GetType();
        if (ClassTypeSymbol* classType = dynamic_cast<ClassTypeSymbol*>(type))
        {
            ContainerScope* scope = classType->GetContainerScope();
            utf32_string str = ToUtf32(dotNode.MemberStr());
            Symbol* symbol = scope->Lookup(StringPtr(str.c_str()), ScopeLookup::this_and_base);
            if (symbol)
            {
                BoundExpression* classObject = expression.release();
                BindSymbol(symbol);
                if (BoundFunctionGroupExpression* bfg = dynamic_cast<BoundFunctionGroupExpression*>(expression.get()))
                {
                    if (!classObject->GetFlag(BoundExpressionFlags::argIsThisOrBase))
                    {
                        Symbol* parent = symbol->Parent();
                        ClassTypeSymbol* owner = dynamic_cast<ClassTypeSymbol*>(parent);
                        Assert(owner, "class type symbol expected");
                        if (classType->HasBaseClass(owner))
                        {
                            classObject = new BoundConversion(boundCompileUnit.GetAssembly(), std::unique_ptr<BoundExpression>(classObject), boundCompileUnit.GetConversion(classType, owner));
                        }
                    }
                    if (classObject->IsBoundTypeExpression())
                    {
                        BoundTypeExpression* bte = static_cast<BoundTypeExpression*>(classObject);
                        bfg->SetScopeQualified();
                        bfg->SetQualifiedScope(bte->GetType()->GetContainerScope());
                    }
                    BoundMemberExpression* bme = new BoundMemberExpression(boundCompileUnit.GetAssembly(), std::unique_ptr<BoundExpression>(classObject), std::move(expression));
                    expression.reset(bme);
                }
                else if (BoundMemberVariable* bmv = dynamic_cast<BoundMemberVariable*>(expression.get()))
                {
                    if (!bmv->GetMemberVariableSymbol()->IsStatic())
                    {
                        Symbol* parent = symbol->Parent();
                        ClassTypeSymbol* owner = dynamic_cast<ClassTypeSymbol*>(parent);
                        Assert(owner, "class type symbol expected");
                        if (classType->HasBaseClass(owner))
                        {
                            classObject = new BoundConversion(boundCompileUnit.GetAssembly(), std::unique_ptr<BoundExpression>(classObject), boundCompileUnit.GetConversion(classType, owner));
                        }
                        bmv->SetClassObject(std::unique_ptr<BoundExpression>(classObject));
                    }
                }
                else if (BoundProperty* bp = dynamic_cast<BoundProperty*>(expression.get()))
                {
                    if (!bp->GetPropertySymbol()->IsStatic())
                    {
                        Symbol* parent = symbol->Parent();
                        ClassTypeSymbol* owner = dynamic_cast<ClassTypeSymbol*>(parent);
                        Assert(owner, "class type symbol expected");
                        if (classType->HasBaseClass(owner))
                        {
                            classObject = new BoundConversion(boundCompileUnit.GetAssembly(), std::unique_ptr<BoundExpression>(classObject), boundCompileUnit.GetConversion(classType, owner));
                        }
                        bp->SetClassObject(std::unique_ptr<BoundExpression>(classObject));
                    }
                }
                else
                {
                    throw Exception("symbol '" + dotNode.MemberStr() + "' does not denote a function group, member variable or property", dotNode.MemberId()->GetSpan());
                }
            }
            else
            {
                throw Exception("symbol '" + dotNode.MemberStr() + "' not found from class '" + ToUtf8(classType->FullName()) + "'", dotNode.MemberId()->GetSpan());
            }
        }
        else if (InterfaceTypeSymbol* interfaceType = dynamic_cast<InterfaceTypeSymbol*>(type))
        {
            ContainerScope* scope = interfaceType->GetContainerScope();
            utf32_string str = ToUtf32(dotNode.MemberStr());
            Symbol* symbol = scope->Lookup(StringPtr(str.c_str()), ScopeLookup::this_and_base);
            if (symbol)
            {
                BoundExpression* interfaceObject = expression.release();
                BindSymbol(symbol);
                if (BoundFunctionGroupExpression* bfg = dynamic_cast<BoundFunctionGroupExpression*>(expression.get()))
                {
                    expression.reset(new BoundMemberExpression(boundCompileUnit.GetAssembly(), std::unique_ptr<BoundExpression>(interfaceObject), std::move(expression)));
                }
                else
                {
                    throw Exception("symbol '" + dotNode.MemberStr() + "' does not denote a function group", dotNode.MemberId()->GetSpan());
                }
            }
            else
            {
                throw Exception("symbol '" + dotNode.MemberStr() + "' not found from interface '" + ToUtf8(interfaceType->FullName()) + "'", dotNode.MemberId()->GetSpan());
            }
        }
        else if (EnumTypeSymbol* enumType = dynamic_cast<EnumTypeSymbol*>(type))
        {
            ContainerScope* scope = enumType->GetContainerScope();
            utf32_string str = ToUtf32(dotNode.MemberStr());
            Symbol* symbol = scope->Lookup(StringPtr(str.c_str()));
            if (symbol)
            {
                BindSymbol(symbol);
            }
            else
            {
                throw Exception("symbol '" + dotNode.MemberStr() + "' not found from enumerated type '" + ToUtf8(enumType->FullName()) + "'", dotNode.MemberId()->GetSpan());
            }
        }
        else
        {
            const Span& span = dotNode.GetSpan();
            if (type->HasBoxedType())
            {
                if (expression->IsBoundTypeExpression())
                {
                    CloneContext cloneContext;
                    DotNode classDot(span, new IdentifierNode(span, type->GetBoxedTypeName()), dotNode.MemberId()->Clone(cloneContext));
                    classDot.Accept(*this);
                }
                else
                {
                    NewNode* newNode = new NewNode(span, new IdentifierNode(span, type->GetBoxedTypeName()));
                    CloneContext cloneContext;
                    newNode->AddArgument(dotNode.Child()->Clone(cloneContext));
                    DotNode objectDot(span, newNode, dotNode.MemberId()->Clone(cloneContext));
                    objectDot.Accept(*this);
                }
            }
            else
            {
                throw Exception("expression '" + dotNode.Child()->ToString() + "' must denote a namespace, class type, interface type, enumerated type or boxable value type object", 
                    dotNode.Child()->GetSpan());
             }
        }
    }
    containerScope = prevContainerScope;
}

void ExpressionBinder::Visit(IndexingNode& indexingNode)
{
    indexingNode.Child()->Accept(*this);
    BoundExpression* subject = expression.release();
    TypeSymbol* subjectType = subject->GetType();
    std::unique_ptr<BoundExpression> index = BindExpression(boundCompileUnit, boundFunction, containerScope, indexingNode.Index());
    if (ArrayTypeSymbol* arrayTypeSymbol = dynamic_cast<ArrayTypeSymbol*>(subjectType))
    {
        TypeSymbol* intType = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Int32");
        if (index->GetType() != intType)
        {
            FunctionSymbol* conversionFun = boundCompileUnit.GetConversion(index->GetType(), intType);
            if (conversionFun)
            {
                index.reset(new BoundConversion(boundCompileUnit.GetAssembly(), std::move(index), conversionFun));
            }
            else
            {
                throw Exception("no implicit conversion from '" + ToUtf8(index->GetType()->FullName()) + "' to '" + ToUtf8(intType->FullName()) + " exists for array index", indexingNode.Index()->GetSpan());
            }
        }
        expression.reset(new BoundArrayElement(boundCompileUnit.GetAssembly(), arrayTypeSymbol, std::unique_ptr<BoundExpression>(subject), std::move(index)));
    }
    else if (ClassTypeSymbol* classTypeSymbol = dynamic_cast<ClassTypeSymbol*>(subjectType))
    {
        if (classTypeSymbol->FullName() == U"System.String")
        {
            if (lvalue)
            {
                throw Exception("string indexer is read-only", span);
            }
            TypeSymbol* intType = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Int32");
            if (index->GetType() != intType)
            {
                FunctionSymbol* conversionFun = boundCompileUnit.GetConversion(index->GetType(), intType);
                if (conversionFun)
                {
                    index.reset(new BoundConversion(boundCompileUnit.GetAssembly(), std::move(index), conversionFun));
                }
                else
                {
                    throw Exception("no implicit conversion from '" + ToUtf8(index->GetType()->FullName()) + "' to '" + ToUtf8(intType->FullName()) + " exists for string index", indexingNode.Index()->GetSpan());
                }
            }
            TypeSymbol* charType = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Char");
            expression.reset(new BoundStringChar(boundCompileUnit.GetAssembly(), charType, std::unique_ptr<BoundExpression>(subject), std::move(index)));
        }
        else
        {
            Symbol* symbol = classTypeSymbol->GetContainerScope()->Lookup(StringPtr(U"@indexers"));
            if (symbol)
            {
                IndexerGroupSymbol* indexerGroupSymbol = dynamic_cast<IndexerGroupSymbol*>(symbol);
                Assert(indexerGroupSymbol, "indexer group symbol expected");
                std::vector<IndexerSymbol*> matchedIndexers;
                std::vector<FunctionSymbol*> conversions;
                for (IndexerSymbol* indexer : indexerGroupSymbol->Indexers())
                {
                    TypeSymbol* indexType = indexer->GetIndexType();
                    if (indexType != index->GetType())
                    {
                        FunctionSymbol* conversionFun = boundCompileUnit.GetConversion(index->GetType(), indexType);
                        if (conversionFun)
                        {
                            matchedIndexers.push_back(indexer);
                            conversions.push_back(conversionFun);
                        }
                    }
                    else
                    {
                        matchedIndexers.push_back(indexer);
                        conversions.push_back(nullptr);
                    }
                }
                if (matchedIndexers.empty())
                {
                    throw Exception("indexer resolution failed: no implicit conversion exists for index type '" + ToUtf8(index->GetType()->FullName()) + "'. " +
                        std::to_string(indexerGroupSymbol->Indexers().size()) + " indexers examined.", indexingNode.GetSpan());
                }
                else if (matchedIndexers.size() > 1)
                {
                    throw Exception("indexer resolution failed: more than one possible indexer exists for index type '" + ToUtf8(index->GetType()->FullName()) + "'. " +
                        std::to_string(indexerGroupSymbol->Indexers().size()) + " indexers examined.", indexingNode.GetSpan());
                }
                else
                {
                    IndexerSymbol* indexerSymbol = matchedIndexers[0];
                    if (!lvalue)
                    {
                        if (!indexerSymbol->Getter())
                        {
                            throw Exception("indexer '" + ToUtf8(indexerSymbol->FullName() + U"[" + indexerSymbol->GetIndexType()->FullName()) + "]' is write-only", span, indexerSymbol->GetSpan());
                        }
                        else
                        {
                            if (ClassTemplateSpecializationSymbol* classTemplateSpecialization = dynamic_cast<ClassTemplateSpecializationSymbol*>(indexerSymbol->ContainingClass()))
                            {
                                if (!indexerSymbol->Getter()->IsInstantiated())
                                {
                                    indexerSymbol->Getter()->SetInstantiationRequested();
                                    boundCompileUnit.GetClassTemplateRepository().Add(classTemplateSpecialization);
                                }
                            }
                        }
                    }
                    else
                    {
                        if (!indexerSymbol->Setter())
                        {
                            throw Exception("indexer '" + ToUtf8(indexerSymbol->FullName() + U"[" + indexerSymbol->GetIndexType()->FullName()) + "]' is read-only", span, indexerSymbol->GetSpan());
                        }
                        else
                        {
                            if (ClassTemplateSpecializationSymbol* classTemplateSpecialization = dynamic_cast<ClassTemplateSpecializationSymbol*>(indexerSymbol->ContainingClass()))
                            {
                                if (!indexerSymbol->Setter()->IsInstantiated())
                                {
                                    indexerSymbol->Setter()->SetInstantiationRequested();
                                    boundCompileUnit.GetClassTemplateRepository().Add(classTemplateSpecialization);
                                }
                            }
                        }
                    }
                    FunctionSymbol* conversionFun = conversions[0];
                    if (conversionFun)
                    {
                        index.reset(new BoundConversion(boundCompileUnit.GetAssembly(), std::move(index), conversionFun));
                    }
                    BoundIndexer* bi = new BoundIndexer(boundCompileUnit.GetAssembly(), indexerSymbol->GetValueType(), indexerSymbol, std::unique_ptr<BoundExpression>(subject), std::move(index));
                    if (indexerSymbol->IsStatic())
                    {
                        bool accessFromOwnScope = false;
                        Symbol* s = boundFunction->GetFunctionSymbol();
                        ClassTypeSymbol* c = s->ContainingClass();
                        if (c)
                        {
                            ClassTypeSymbol* cp = indexerSymbol->ContainingClass();
                            Assert(cp, "class type symbol expected");
                            if (cp == c)
                            {
                                accessFromOwnScope = true;
                            }
                        }
                        if (!accessFromOwnScope)
                        {
                            bi->SetStaticInitNeeded();
                        }
                    }
                    expression.reset(bi);
                }
            }
            else
            {
                throw Exception("class '" + ToUtf8(classTypeSymbol->FullName()) + "' does not contain any indexers", indexingNode.GetSpan());
            }
        }
    }
    else
    {
        throw Exception("array or class type expected", indexingNode.GetSpan());
    }
}

void ExpressionBinder::Visit(InvokeNode& invokeNode)
{
    invokeNode.Child()->Accept(*this);
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
    FunctionGroupSymbol* functionGroupSymbol = nullptr;
    bool scopeQualified = false;
    if (BoundFunctionGroupExpression* bfe = dynamic_cast<BoundFunctionGroupExpression*>(expression.get()))
    {
        functionGroupSymbol = bfe->FunctionGroup();
        if (bfe->ScopeQualified())
        {
            functionScopeLookups.clear();
            functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, bfe->QualiedScope()));
            scopeQualified = true;
        }
    }
    else if (BoundMemberExpression* bme = dynamic_cast<BoundMemberExpression*>(expression.get()))
    {
        if (BoundFunctionGroupExpression* bfe = dynamic_cast<BoundFunctionGroupExpression*>(bme->Member()))
        {
            functionGroupSymbol = bfe->FunctionGroup();
            if (bfe->ScopeQualified())
            {
                functionScopeLookups.clear();
                functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, bfe->QualiedScope()));
                scopeQualified = true;
            }
            BoundExpression* classOrInterfaceObject = bme->ReleaseClassObject();
            if (!scopeQualified)
            {
                functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base, classOrInterfaceObject->GetType()->ClassInterfaceOrNsScope()));
            }
            arguments.push_back(std::unique_ptr<BoundExpression>(classOrInterfaceObject));
        }
        else
        {
            throw Exception("invoke cannot be applied to this type of expression", invokeNode.Child()->GetSpan());
        }
    }
    else if (expression->GetType()->IsDelegateType())
    {
        BoundExpression* delegateExpr = expression.release();
        DelegateTypeSymbol* delegateType = static_cast<DelegateTypeSymbol*>(delegateExpr->GetType());
        int n = invokeNode.Arguments().Count();
        if (n != delegateType->Arity())
        {
            throw Exception("wrong number of arguments to delegate call (got " + std::to_string(n) + ", need " + std::to_string(delegateType->Arity()) + ")", invokeNode.GetSpan());
        }
        for (int i = 0; i < n; ++i)
        {
            Node* argument = invokeNode.Arguments()[i];
            argument->Accept(*this);
            TypeSymbol* sourceType = expression->GetType();
            TypeSymbol* targetType = delegateType->Parameters()[i]->GetType();
            if (sourceType != targetType)
            {
                FunctionSymbol* conversionFun = boundCompileUnit.GetConversion(sourceType, targetType);
                if (conversionFun && conversionFun->GetConversionType() == ConversionType::implicit_)
                {
                    BoundConversion* conversion = new BoundConversion(boundCompileUnit.GetAssembly(), std::move(expression), conversionFun);
                    expression.reset(conversion);
                }
                else
                {
                    throw Exception("no implicit conversion from '" + ToUtf8(sourceType->FullName()) + "' to '" + ToUtf8(targetType->FullName()) + "' exists", invokeNode.GetSpan());
                }
            }
            arguments.push_back(std::unique_ptr<BoundExpression>(expression.release()));
        }
        arguments.push_back(std::unique_ptr<BoundExpression>(delegateExpr));
        expression.reset(new BoundDelegateCall(boundCompileUnit.GetAssembly(), delegateType, std::move(arguments)));
        CheckAccess(boundFunction->GetFunctionSymbol(), expression->GetType());
        return;
    }
    else if (expression->GetType()->IsClassDelegateType())
    {
        BoundExpression* classDelegateExpr = expression.release();
        ClassDelegateTypeSymbol* classDelegateType = static_cast<ClassDelegateTypeSymbol*>(classDelegateExpr->GetType());
        int n = invokeNode.Arguments().Count();
        if (n != classDelegateType->Arity())
        {
            throw Exception("wrong number of arguments to delegate call (got " + std::to_string(n) + ", need " + std::to_string(classDelegateType->Arity()) + ")", invokeNode.GetSpan());
        }
        for (int i = 0; i < n; ++i)
        {
            Node* argument = invokeNode.Arguments()[i];
            argument->Accept(*this);
            TypeSymbol* sourceType = expression->GetType();
            TypeSymbol* targetType = classDelegateType->Parameters()[i]->GetType();
            if (sourceType != targetType)
            {
                FunctionSymbol* conversionFun = boundCompileUnit.GetConversion(sourceType, targetType);
                if (conversionFun && conversionFun->GetConversionType() == ConversionType::implicit_)
                {
                    BoundConversion* conversion = new BoundConversion(boundCompileUnit.GetAssembly(), std::move(expression), conversionFun);
                    expression.reset(conversion);
                }
                else
                {
                    throw Exception("no implicit conversion from '" + ToUtf8(sourceType->FullName()) + "' to '" + ToUtf8(targetType->FullName()) + "' exists", invokeNode.GetSpan());
                }
            }
            arguments.push_back(std::unique_ptr<BoundExpression>(expression.release()));
        }
        arguments.push_back(std::unique_ptr<BoundExpression>(classDelegateExpr));
        expression.reset(new BoundClassDelegateCall(boundCompileUnit.GetAssembly(), classDelegateType, std::move(arguments)));
        CheckAccess(boundFunction->GetFunctionSymbol(), expression->GetType());
        return;
    }
    else if (expression->GetType()->IsClassType())
    {
        BoundExpression* classExpr = expression.release();
        arguments.push_back(std::unique_ptr<BoundExpression>(classExpr));
        functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base, classExpr->GetType()->Class()->GetContainerScope()));
        int n = invokeNode.Arguments().Count();
        for (int i = 0; i < n; ++i)
        {
            Node* argument = invokeNode.Arguments()[i];
            argument->Accept(*this);
            arguments.push_back(std::unique_ptr<BoundExpression>(expression.release()));
        }
        std::unique_ptr<BoundFunctionCall> functionCall = ResolveOverload(boundCompileUnit, U"operator()", functionScopeLookups, arguments, invokeNode.GetSpan());
        CheckAccess(boundFunction->GetFunctionSymbol(), functionCall->GetFunctionSymbol());
        expression.reset(functionCall.release());
        return;
    }
    else
    {
        throw Exception("invoke cannot be applied to this type of expression", invokeNode.Child()->GetSpan());
    }
    int n = invokeNode.Arguments().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* argument = invokeNode.Arguments()[i];
        argument->Accept(*this);
        if (!expression->GetType()->IsFunctionGroupTypeSymbol() && !scopeQualified)
        {
            functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, expression->GetType()->ClassInterfaceOrNsScope()));
        }
        arguments.push_back(std::unique_ptr<BoundExpression>(expression.release()));
    }
    if (!scopeQualified)
    {
        functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
    }
    std::unique_ptr<Exception> exception;
    std::unique_ptr<Exception> thisEx;
    std::unique_ptr<Exception> nsEx;
    std::unique_ptr<BoundFunctionCall> functionCall = ResolveOverload(boundCompileUnit, functionGroupSymbol->Name(), functionScopeLookups, arguments, invokeNode.GetSpan(),
        OverloadResolutionFlags::dontThrow, exception);
    if (!functionCall)
    {
        ParameterSymbol* thisParam = boundFunction->GetFunctionSymbol()->GetThisParam();
        bool thisParamInserted = false;
        if (thisParam)
        {
            BoundParameter* boundThisParam = new BoundParameter(boundCompileUnit.GetAssembly(), thisParam->GetType(), thisParam);
            arguments.insert(arguments.begin(), std::unique_ptr<BoundExpression>(boundThisParam));
            thisParamInserted = true;
            functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base, thisParam->GetType()->ClassInterfaceOrNsScope()));
            functionCall = std::move(ResolveOverload(boundCompileUnit, functionGroupSymbol->Name(), functionScopeLookups, arguments, invokeNode.GetSpan(),
                OverloadResolutionFlags::dontThrow, thisEx));
        }
        if (!functionCall)
        {
            if (thisParamInserted)
            {
                arguments.erase(arguments.begin());
            }
			if (!arguments.empty())
			{
				arguments.erase(arguments.begin());
			}
            functionCall = std::move(ResolveOverload(boundCompileUnit, functionGroupSymbol->Name(), functionScopeLookups, arguments, invokeNode.GetSpan(),
                OverloadResolutionFlags::dontThrow, nsEx));
        }
    }
    if (!functionCall)
    {
        if (CastOverloadException* castException = dynamic_cast<CastOverloadException*>(exception.get()))
        {
            throw *exception;
        }
        else if (RefOverloadException* refException = dynamic_cast<RefOverloadException*>(exception.get()))
        {
            throw *exception;
        }
        if (CastOverloadException* castException = dynamic_cast<CastOverloadException*>(thisEx.get()))
        {
            throw *thisEx;
        }
        else if (RefOverloadException* refException = dynamic_cast<RefOverloadException*>(thisEx.get()))
        {
            throw *thisEx;
        }
        if (CastOverloadException* castException = dynamic_cast<CastOverloadException*>(nsEx.get()))
        {
            throw *nsEx;
        }
        else if (RefOverloadException* refException = dynamic_cast<RefOverloadException*>(nsEx.get()))
        {
            throw *nsEx;
        }
        if (exception.get())
        {
            throw *exception;
        }
        else if (thisEx.get())
        {
            throw *thisEx;
        }
        else if (nsEx.get())
        { 
            throw *nsEx;
        }
        else
        {
            throw Exception("overload resolution failed: overload not found", invokeNode.GetSpan());
        }
    }
    CheckAccess(boundFunction->GetFunctionSymbol(), functionCall->GetFunctionSymbol());
    if (MemberFunctionSymbol* memFun = dynamic_cast<MemberFunctionSymbol*>(functionCall->GetFunctionSymbol()))
    {
        if (!memFun->IsStatic())
        {
            Assert(!functionCall->Arguments().empty(), "nonempty argument list expected");
            if (InterfaceTypeSymbol* interfaceType = dynamic_cast<InterfaceTypeSymbol*>(functionCall->Arguments()[0]->GetType()))
            {
                functionCall->SetFunctionCallType(FunctionCallType::interfaceCall);
            }
            else if (memFun->IsVirtualAbstractOrOverride())
            {
                if (!functionCall->Arguments()[0]->GetFlag(BoundExpressionFlags::argIsThisOrBase))
                {
                    functionCall->SetFunctionCallType(FunctionCallType::virtualCall);
                }
            }
        }
    }
    expression.reset(functionCall.release());
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
        functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, targetType->ClassInterfaceOrNsScope()));
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
    if (ArrayTypeSymbol* arrayTypeSymbol = dynamic_cast<ArrayTypeSymbol*>(type))
    {
        ArrayNode* arrayNode = dynamic_cast<ArrayNode*>(newNode.TypeExpr());
        Assert(arrayNode, "array node expected");
        if (arrayNode->Size())
        {
            arrayNode->Size()->Accept(*this);
            arguments.push_back(std::move(expression));
        }
        else
        {
            throw Exception("size of array must be specified in array new expression", newNode.TypeExpr()->GetSpan());
        }
    }
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, type->ClassOrNsScope()));
    std::unique_ptr<BoundFunctionCall> constructorCall = ResolveOverload(boundCompileUnit, U"@constructor", functionScopeLookups, arguments, newNode.GetSpan());
    CheckAccess(boundFunction->GetFunctionSymbol(), constructorCall->GetFunctionSymbol());
    expression.reset(new BoundNewExpression(constructorCall.get(), type));
}

void ExpressionBinder::Visit(ThisNode& thisNode)
{
    ParameterSymbol* thisParam = boundFunction->GetFunctionSymbol()->GetThisParam();
    if (!thisParam)
    {
        throw Exception("'this' can be used only in non-static member function context", thisNode.GetSpan());
    }
    BoundParameter* boundThisParam = new BoundParameter(boundCompileUnit.GetAssembly(), thisParam->GetType(), thisParam);
    boundThisParam->SetFlag(BoundExpressionFlags::argIsThisOrBase);
    expression.reset(boundThisParam);
}

void ExpressionBinder::Visit(BaseNode& baseNode)
{
    ParameterSymbol* thisParam = boundFunction->GetFunctionSymbol()->GetThisParam();
    if (!thisParam)
    {
        throw Exception("'base' can be used only in non-static member function context", baseNode.GetSpan());
    }
    ClassTypeSymbol* classType = dynamic_cast<ClassTypeSymbol*>(thisParam->GetType());
    Assert(classType, "class type expected");
    if (classType->BaseClass())
    {
        BoundParameter* boundThisParam = new BoundParameter(boundCompileUnit.GetAssembly(), thisParam->GetType(), thisParam);
        FunctionSymbol* thisToBaseConversion = boundCompileUnit.GetConversion(thisParam->GetType(), classType->BaseClass());
        if (!thisToBaseConversion)
        {
            throw Exception("no implicit conversion from '" + ToUtf8(classType->FullName()) + "' to '" + ToUtf8(classType->BaseClass()->FullName()) + "' exists.", baseNode.GetSpan());
        }
        BoundConversion* thisAsBase = new BoundConversion(boundCompileUnit.GetAssembly(), std::unique_ptr<BoundExpression>(boundThisParam), thisToBaseConversion);
        thisAsBase->SetFlag(BoundExpressionFlags::argIsThisOrBase);
        expression.reset(thisAsBase);
    }
    else
    {
        throw Exception("class '" + ToUtf8(classType->FullName()) + "' does not have a base class", baseNode.GetSpan(), classType->GetSpan());
    }
}

void ExpressionBinder::Visit(DefaultNode& defaultNode)
{
    TypeSymbol* type = ResolveType(boundCompileUnit, containerScope, defaultNode.TypeExpr());
    expression.reset(new BoundDefaultExpression(boundCompileUnit.GetAssembly(), type));
}

void ExpressionBinder::Visit(IsNode& isNode)
{
    isNode.Expr()->Accept(*this);
    TypeSymbol* type = ResolveType(boundCompileUnit, containerScope, isNode.TargetTypeExpr());
    if (ClassTypeSymbol* classType = dynamic_cast<ClassTypeSymbol*>(type))
    {
        BoundIsExpression* boundIsExpression = new BoundIsExpression(boundCompileUnit.GetAssembly(), std::unique_ptr<BoundExpression>(expression.release()), classType);
        expression.reset(boundIsExpression);
    }
    else
    {
        throw Exception("target type or is expression must be a class type", isNode.TargetTypeExpr()->GetSpan());
    }
}

void ExpressionBinder::Visit(AsNode& asNode)
{
    asNode.Expr()->Accept(*this);
    TypeSymbol* type = ResolveType(boundCompileUnit, containerScope, asNode.TargetTypeExpr());
    if (ClassTypeSymbol* classType = dynamic_cast<ClassTypeSymbol*>(type))
    {
        BoundAsExpression* boundAsExpression = new BoundAsExpression(boundCompileUnit.GetAssembly(), std::unique_ptr<BoundExpression>(expression.release()), classType);
        expression.reset(boundAsExpression);
    }
    else
    {
        throw Exception("target type of as expression must be a class type", asNode.TargetTypeExpr()->GetSpan());
    }
}

void ExpressionBinder::Visit(RefNode& refNode)
{
    refNode.Child()->Accept(*this);
    if (expression->IsBoundLocalVariable())
    {
        BoundLocalVariable* boundLocalVariable = static_cast<BoundLocalVariable*>(expression.get());
        TypeSymbol* refType = boundCompileUnit.GetAssembly().GetSymbolTable().MakeRefType(refNode, expression->GetType());
        expression.reset(new BoundLocalRefExpression(boundCompileUnit.GetAssembly(), boundLocalVariable->GetLocalVariableSymbol()->Index(), refType));
    }
    else if (expression->IsBoundParameter())
    {
        BoundParameter* boundParameter = static_cast<BoundParameter*>(expression.get());
        TypeSymbol* refType = boundCompileUnit.GetAssembly().GetSymbolTable().MakeRefType(refNode, expression->GetType());
        expression.reset(new BoundLocalRefExpression(boundCompileUnit.GetAssembly(), boundParameter->GetParameterSymbol()->Index(), refType));
    }
    else if (expression->IsBoundMemberVariable())
    {
        BoundMemberVariable* boundMemberVariable = static_cast<BoundMemberVariable*>(expression.get());
        TypeSymbol* refType = boundCompileUnit.GetAssembly().GetSymbolTable().MakeRefType(refNode, expression->GetType());
        expression.reset(new BoundMemberVarRefExpression(boundCompileUnit.GetAssembly(), std::unique_ptr<BoundExpression>(boundMemberVariable->ReleaseClassObject()),
            boundMemberVariable->GetMemberVariableSymbol()->Index(), refType));
    }
    else
    {
        throw Exception("only refs to local or member variables and parameters supported", refNode.GetSpan());
    }
}

std::unique_ptr<BoundExpression> BindExpression(BoundCompileUnit& boundCompileUnit, BoundFunction* boundFunction, ContainerScope* containerScope, Node* node)
{
    return BindExpression(boundCompileUnit, boundFunction, containerScope, node, false);
}

std::unique_ptr<BoundExpression> BindExpression(BoundCompileUnit& boundCompileUnit, BoundFunction* boundFunction, ContainerScope* containerScope, Node* node, bool lvalue)
{
    return BindExpression(boundCompileUnit, boundFunction, containerScope, node, lvalue, false, false);
}

std::unique_ptr<BoundExpression> BindExpression(BoundCompileUnit& boundCompileUnit, BoundFunction* boundFunction, ContainerScope* containerScope, Node* node, bool lvalue,
    bool acceptFunctionGroup, bool acceptMemberExpression)
{
    return BindExpression(boundCompileUnit, boundFunction, containerScope, node, lvalue, acceptFunctionGroup, acceptMemberExpression, false);
}

std::unique_ptr<BoundExpression> BindExpression(BoundCompileUnit& boundCompileUnit, BoundFunction* boundFunction, ContainerScope* containerScope, Node* node, bool lvalue,
    bool acceptFunctionGroup, bool acceptMemberExpression, bool acceptIncomplete)
{
    ExpressionBinder expressionBinder(boundCompileUnit, boundFunction, containerScope, node->GetSpan(), lvalue);
    node->Accept(expressionBinder);
    std::unique_ptr<BoundExpression> expression = expressionBinder.GetExpression();
    if (acceptFunctionGroup && expression->IsBoundFunctionGroupExpression())
    {
        return expression;
    }
    if (acceptMemberExpression && expression->IsBoundMemberExpression())
    {
        return expression;
    }
    if (!acceptIncomplete)
    {
        if (!expression->IsComplete())
        {
            throw Exception("incomplete expression", node->GetSpan());
        }
    }
    if (lvalue && !expression->IsLvalueExpression())
    {
        throw Exception("not an lvalue expression", node->GetSpan());
    }
    return expression;
}

} } // namespace cminor::binder
