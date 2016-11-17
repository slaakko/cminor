// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/Evaluator.hpp>
#include <cminor/binder/TypeResolver.hpp>
#include <cminor/binder/TypeBinderVisitor.hpp>
#include <cminor/ast/BasicType.hpp>
#include <cminor/ast/CompileUnit.hpp>
#include <cminor/ast/Constant.hpp>
#include <cminor/ast/Expression.hpp>
#include <cminor/ast/Literal.hpp>
#include <cminor/ast/Visitor.hpp>
#include <cminor/symbols/ConstantSymbol.hpp>
#include <cminor/symbols/EnumSymbol.hpp>
#include <functional>

namespace cminor { namespace binder {

class ScopedValue : public Value
{
public:
    ScopedValue(ContainerSymbol* containerSymbol_);
    ContainerSymbol* GetContainerSymbol() const { return containerSymbol; }
    ValueKind GetValueKind() const override { throw std::runtime_error("not applicable"); }
    Value* Clone() const override { throw std::runtime_error("not applicable"); }
    void Write(SymbolWriter& writer) override { throw std::runtime_error("not applicable"); }
    void Read(SymbolReader& reader) override { throw std::runtime_error("not applicable"); }
    Value* As(ValueKind targetKind, bool cast, const Span& span) const override { throw std::runtime_error("not applicable"); }
    IntegralValue GetIntegralValue() const override { throw std::runtime_error("not applicable"); }
    bool IsComplete() const override { return false; }
private:
    ContainerSymbol* containerSymbol;
};

typedef Value* (*BinaryOperatorFun)(Value* left, Value* right, const Span& span);
typedef Value* (*UnaryOperatorFun)(Value* subject, const Span& span);

ScopedValue::ScopedValue(ContainerSymbol* containerSymbol_) : containerSymbol(containerSymbol_)
{
}

class Evaluator : public Visitor, SymbolEvaluator
{
public:
    Evaluator(ValueKind targetKind_, bool cast_, ContainerScope* containerScope_, BoundCompileUnit& boundCompileUnit_, const Span& span_);
    Value* GetValue();

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

    void Visit(IdentifierNode& identifierNode) override;
    void Visit(ParameterNode& parameterNode) override;
    void Visit(FunctionNode& functionNode) override;
    void Visit(FunctionGroupIdNode& functionGroupIdNode) override;
    void Visit(AttributeMap& attributeMap) override;
    void Visit(NamespaceNode& namespaceNode) override;
    void Visit(AliasNode& aliasNode) override;
    void Visit(NamespaceImportNode& namespaceImportNode) override;
    void Visit(CompileUnitNode& compileUnitNode) override;
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
    void Visit(ThisNode& thisNode) override;
    void Visit(BaseNode& baseNode) override;

    void Visit(CompoundStatementNode& compoundStatementNode) override;
    void Visit(ReturnStatementNode& returnStatementNode) override;
    void Visit(IfStatementNode& ifStatementNode) override;
    void Visit(WhileStatementNode& whileStatementNode) override;
    void Visit(DoStatementNode& doStatementNode) override;
    void Visit(ForStatementNode& forStatementNode) override;
    void Visit(BreakStatementNode& breakStatementNode) override;
    void Visit(ContinueStatementNode& continueStatementNode) override;
    void Visit(ConstructionStatementNode& constructionStatementNode) override;
    void Visit(AssignmentStatementNode& assignmentStatementNode) override;
    void Visit(ExpressionStatementNode& expressionStatementNode) override;
    void Visit(EmptyStatementNode& emptyStatementNode) override;
    void Visit(IncrementStatementNode& incrementStatementNode) override;
    void Visit(DecrementStatementNode& decrementStatementNode) override;

    void Visit(EnumTypeNode& enumTypeNode) override;
    void Visit(EnumConstantNode& enumConstantNode) override;
    void Visit(ConstantNode& constantNode) override;

    void EvaluateContainerSymbol(ContainerSymbol* containerSymbol) override;
    void EvaluateConstantSymbol(ConstantSymbol* constantSymbol) override;
    void EvaluateEnumConstantSymbol(EnumConstantSymbol* enumConstantSymbol) override;
private:
    ValueKind targetKind;
    bool cast;
    ContainerScope* containerScope;
    BoundCompileUnit& boundCompileUnit;
    Span span;
    std::unique_ptr<Value> value;
    void EvaluateSymbol(Symbol* symbol, const Span& span);
    void EvaluateBinOp(const BinaryNode& node, BinaryOperatorFun* fun);
    void EvaluateUnaryOp(const UnaryNode& node, UnaryOperatorFun* fun);
};

Evaluator::Evaluator(ValueKind targetKind_, bool cast_, ContainerScope* containerScope_, BoundCompileUnit& boundCompileUnit_, const Span& span_) :
    targetKind(targetKind_), cast(cast_), containerScope(containerScope_), boundCompileUnit(boundCompileUnit_), span(span_)
{
}

Value* Evaluator::GetValue()
{ 
    if (!value->IsComplete())
    {
        throw Exception("value not complete", span);
    }
    value.reset(value->As(targetKind, cast, span));
    return value.release();
}

void Evaluator::Visit(BoolNode& boolNode)
{
    throw Exception("cannot evaluate statically", boolNode.GetSpan());
}

void Evaluator::Visit(SByteNode& sbyteNode)
{
    throw Exception("cannot evaluate statically", sbyteNode.GetSpan());
}

void Evaluator::Visit(ByteNode& byteNode)
{
    throw Exception("cannot evaluate statically", byteNode.GetSpan());
}

void Evaluator::Visit(ShortNode& shortNode)
{
    throw Exception("cannot evaluate statically", shortNode.GetSpan());
}

void Evaluator::Visit(UShortNode& ushortNode)
{
    throw Exception("cannot evaluate statically", ushortNode.GetSpan());
}

void Evaluator::Visit(IntNode& intNode)
{
    throw Exception("cannot evaluate statically", intNode.GetSpan());
}

void Evaluator::Visit(UIntNode& uintNode)
{
    throw Exception("cannot evaluate statically", uintNode.GetSpan());
}

void Evaluator::Visit(LongNode& longNode)
{
    throw Exception("cannot evaluate statically", longNode.GetSpan());
}

void Evaluator::Visit(ULongNode& ulongNode)
{
    throw Exception("cannot evaluate statically", ulongNode.GetSpan());
}

void Evaluator::Visit(FloatNode& floatNode)
{
    throw Exception("cannot evaluate statically", floatNode.GetSpan());
}

void Evaluator::Visit(DoubleNode& doubleNode)
{
    throw Exception("cannot evaluate statically", doubleNode.GetSpan());
}

void Evaluator::Visit(CharNode& charNode)
{
    throw Exception("cannot evaluate statically", charNode.GetSpan());
}

void Evaluator::Visit(StringNode& stringNode)
{
    throw Exception("cannot evaluate statically", stringNode.GetSpan());
}

void Evaluator::Visit(VoidNode& voidNode)
{
    throw Exception("cannot evaluate statically", voidNode.GetSpan());
}

void Evaluator::Visit(ObjectNode& objectNode)
{
    throw Exception("cannot evaluate statically", objectNode.GetSpan());
}

void Evaluator::Visit(BooleanLiteralNode& booleanLiteralNode)
{
    value.reset(new BoolValue(booleanLiteralNode.Value()));
}

void Evaluator::Visit(SByteLiteralNode& sbyteLiteralNode)
{
    value.reset(new SByteValue(sbyteLiteralNode.Value()));
}

void Evaluator::Visit(ByteLiteralNode& byteLiteralNode)
{
    value.reset(new ByteValue(byteLiteralNode.Value()));
}

void Evaluator::Visit(ShortLiteralNode& shortLiteralNode)
{
    value.reset(new ShortValue(shortLiteralNode.Value()));
}

void Evaluator::Visit(UShortLiteralNode& ushortLiteralNode)
{
    value.reset(new UShortValue(ushortLiteralNode.Value()));
}

void Evaluator::Visit(IntLiteralNode& intLiteralNode)
{
    value.reset(new IntValue(intLiteralNode.Value()));
}

void Evaluator::Visit(UIntLiteralNode& uintLiteralNode) 
{
    value.reset(new UIntValue(uintLiteralNode.Value()));
}

void Evaluator::Visit(LongLiteralNode& longLiteralNode)
{
    value.reset(new LongValue(longLiteralNode.Value()));
}

void Evaluator::Visit(ULongLiteralNode& ulongLiteralNode)
{
    value.reset(new ULongValue(ulongLiteralNode.Value()));
}

void Evaluator::Visit(FloatLiteralNode& floatLiteralNode)
{
    value.reset(new FloatValue(floatLiteralNode.Value()));
}

void Evaluator::Visit(DoubleLiteralNode& doubleLiteralNode)
{
    value.reset(new DoubleValue(doubleLiteralNode.Value()));
}

void Evaluator::Visit(CharLiteralNode& charLiteralNode)
{
    value.reset(new CharValue(charLiteralNode.Value()));
}

void Evaluator::Visit(StringLiteralNode& stringLiteralNode)
{
    throw Exception("cannot evaluate statically", stringLiteralNode.GetSpan());
}

void Evaluator::Visit(NullLiteralNode& nullLiteralNode)
{
    throw Exception("cannot evaluate statically", nullLiteralNode.GetSpan());
}

void Evaluator::Visit(IdentifierNode& identifierNode)
{
    utf32_string s = ToUtf32(identifierNode.Str());
    Symbol* symbol = containerScope->Lookup(StringPtr(s.c_str()), ScopeLookup::this_and_base_and_parent);;
    if (!symbol)
    {
        for (const std::unique_ptr<FileScope>& fileScope : boundCompileUnit.FileScopes())
        {
            symbol = fileScope->Lookup(StringPtr(s.c_str()));
            if (symbol) break;
        }
    }
    if (symbol)
    {
        EvaluateSymbol(symbol, identifierNode.GetSpan());
    }
    else
    {
        throw Exception("symbol '" + identifierNode.Str() + "' not found", identifierNode.GetSpan());
    }
}

void Evaluator::Visit(ParameterNode& parameterNode)
{
    throw Exception("cannot evaluate statically", parameterNode.GetSpan());
}

void Evaluator::Visit(FunctionNode& functionNode)
{
    throw Exception("cannot evaluate statically", functionNode.GetSpan());
}

void Evaluator::Visit(FunctionGroupIdNode& functionGroupIdNode)
{
    throw Exception("cannot evaluate statically", functionGroupIdNode.GetSpan());
}

void Evaluator::Visit(AttributeMap& attributeMap)
{
    throw Exception("cannot evaluate statically", span);
}

void Evaluator::Visit(NamespaceNode& namespaceNode)
{
    throw Exception("cannot evaluate statically", namespaceNode.GetSpan());
}

void Evaluator::Visit(AliasNode& aliasNode)
{
    throw Exception("cannot evaluate statically", aliasNode.GetSpan());
}

void Evaluator::Visit(NamespaceImportNode& namespaceImportNode)
{
    throw Exception("cannot evaluate statically", namespaceImportNode.GetSpan());
}

void Evaluator::Visit(CompileUnitNode& compileUnitNode)
{
    throw Exception("cannot evaluate statically", compileUnitNode.GetSpan());
}

void Evaluator::Visit(ClassNode& classNode)
{
    throw Exception("cannot evaluate statically", classNode.GetSpan());
}

void Evaluator::Visit(InterfaceNode& interfaceNode)
{
    throw Exception("cannot evaluate statically", interfaceNode.GetSpan());
}

void Evaluator::Visit(TemplateIdNode& templateIdNode)
{
    throw Exception("cannot evaluate statically", templateIdNode.GetSpan());
}

void Evaluator::Visit(TemplateParameterNode& templateParameterNode)
{
    throw Exception("cannot evaluate statically", templateParameterNode.GetSpan());
}

void Evaluator::Visit(StaticConstructorNode& staticConstructorNode)
{
    throw Exception("cannot evaluate statically", staticConstructorNode.GetSpan());
}

void Evaluator::Visit(BaseInitializerNode& baseInitializerNode)
{
    throw Exception("cannot evaluate statically", baseInitializerNode.GetSpan());
}

void Evaluator::Visit(ThisInitializerNode& thisInitializerNode)
{
    throw Exception("cannot evaluate statically", thisInitializerNode.GetSpan());
}

void Evaluator::Visit(ConstructorNode& constructorNode)
{
    throw Exception("cannot evaluate statically", constructorNode.GetSpan());
}

void Evaluator::Visit(MemberFunctionNode& memberFunctionNode)
{
    throw Exception("cannot evaluate statically", memberFunctionNode.GetSpan());
}

void Evaluator::Visit(MemberVariableNode& memberVariableNode)
{
    throw Exception("cannot evaluate statically", memberVariableNode.GetSpan());
}

void Evaluator::Visit(PropertyNode& propertyNode)
{
    throw Exception("cannot evaluate statically", propertyNode.GetSpan());
}

void Evaluator::Visit(IndexerNode& indexerNode)
{
    throw Exception("cannot evaluate statically", indexerNode.GetSpan());
}

void Evaluator::EvaluateSymbol(Symbol* symbol, const Span& span)
{
    this->span = span;
    symbol->Evaluate(this, span);
}

void Evaluator::EvaluateContainerSymbol(ContainerSymbol* containerSymbol)
{
    value.reset(new ScopedValue(containerSymbol));
}

void Evaluator::EvaluateConstantSymbol(ConstantSymbol* constantSymbol)
{
    if (constantSymbol->Evaluating())
    {
        throw Exception("cyclic depenency detected", span);
    }
    Value* constantValue = constantSymbol->GetValue();
    if (constantValue)
    {
        value.reset(constantValue);
    }
    else
    {
        Node* node = boundCompileUnit.GetAssembly().GetSymbolTable().GetNode(constantSymbol);
        ConstantNode* constantNode = dynamic_cast<ConstantNode*>(node);
        Assert(constantNode, "constant node expected");
        constantSymbol->SetEvaluating();
        TypeBinderVisitor typeBinderVisitor(boundCompileUnit);
        typeBinderVisitor.SetContainerScope(containerScope);
        constantNode->Accept(typeBinderVisitor);
        constantSymbol->ResetEvaluating();
        Value* constantValue = constantSymbol->GetValue();
        Assert(constantValue, "constant value expected");
        value.reset(constantValue);
    }
}

void Evaluator::EvaluateEnumConstantSymbol(EnumConstantSymbol* enumConstantSymbol)
{
    if (enumConstantSymbol->Evaluating())
    {
        throw Exception("cyclic depenency detected", span);
    }
    Value* enumConstantValue = enumConstantSymbol->GetValue();
    if (enumConstantValue)
    {
        value.reset(enumConstantValue);
    }
    else
    {
        Symbol* symbol = enumConstantSymbol->Parent();
        EnumTypeSymbol* enumTypeSymbol = dynamic_cast<EnumTypeSymbol*>(symbol);
        Assert(enumTypeSymbol, "enumerated type symbol expected");
        Node* node = boundCompileUnit.GetAssembly().GetSymbolTable().GetNode(enumTypeSymbol);
        EnumTypeNode* enumTypeNode = dynamic_cast<EnumTypeNode*>(node);
        TypeBinderVisitor typeBinderVisitor(boundCompileUnit);
        typeBinderVisitor.SetContainerScope(containerScope);
        enumTypeNode->Accept(typeBinderVisitor);
        enumConstantSymbol->ResetEvaluating();
        Value* enumConstantValue = enumConstantSymbol->GetValue();
        Assert(enumConstantValue, "enum constant value expected");
        value.reset(enumConstantValue);
    }
}

Value* NotSupported(Value* subject, const Span& span)
{
    throw Exception("operation not supported for type " + ValueKindStr(subject->GetValueKind()), span);
}

Value* NotSupported(Value* left, Value* right, const Span& span)
{
    throw Exception("operation not supported for types " + ValueKindStr(left->GetValueKind()) + " and " + ValueKindStr(right->GetValueKind()), span);
}

template <typename ValueT, typename Op>
Value* BinaryEvaluate(Value* left, Value* right, Op op)
{
    ValueT* left_ = static_cast<ValueT*>(left);
    ValueT* right_ = static_cast<ValueT*>(right);
    return new ValueT(op(left_->Value(), right_->Value()));
}

template<typename ValueT>
Value* Disjunction(Value* left, Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, std::logical_or<typename ValueT::OperandType>());
}

BinaryOperatorFun disjunction[uint8_t(ValueKind::max)] =
{
    NotSupported, Disjunction<BoolValue>, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported
};

void Evaluator::EvaluateBinOp(const BinaryNode& node, BinaryOperatorFun* fun)
{
    node.Left()->Accept(*this);
    std::unique_ptr<Value> left(value.release());
    node.Right()->Accept(*this);
    std::unique_ptr<Value> right(value.release());
    ValueKind leftKind = left->GetValueKind();
    ValueKind rightKind = right->GetValueKind();
    ValueKind commonKind = GetCommonType(leftKind, rightKind);
    ValueKind operationKind = commonKind;
    if (targetKind > commonKind)
    {
        operationKind = targetKind;
    }
    std::unique_ptr<Value> left_(left->As(operationKind, cast, span));
    std::unique_ptr<Value> right_(right->As(operationKind, cast, span));
    BinaryOperatorFun operation = fun[uint8_t(operationKind)];
    value.reset(operation(left_.get(), right_.get(), span));
}

void Evaluator::Visit(DisjunctionNode& disjunctionNode)
{
    EvaluateBinOp(disjunctionNode, disjunction);
}

template<typename ValueT>
Value* Conjunction(Value* left, Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, std::logical_or<typename ValueT::OperandType>());
}

BinaryOperatorFun conjunction[uint8_t(ValueKind::max)] =
{
    NotSupported, Conjunction<BoolValue>, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported
};

void Evaluator::Visit(ConjunctionNode& conjunctionNode)
{
    EvaluateBinOp(conjunctionNode, conjunction);
}

template<typename ValueT>
Value* BitOr(Value* left, Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, std::bit_or<typename ValueT::OperandType>());
}

BinaryOperatorFun bitOr[uint8_t(ValueKind::max)] =
{
    NotSupported, NotSupported, NotSupported, BitOr<SByteValue>, BitOr<ByteValue>, BitOr<ShortValue>, BitOr<UShortValue>, 
    BitOr<IntValue>, BitOr<UIntValue>, BitOr<LongValue>, BitOr<ULongValue>, NotSupported, NotSupported
};

void Evaluator::Visit(BitOrNode& bitOrNode)
{
    EvaluateBinOp(bitOrNode, bitOr);
}

template<typename ValueT>
Value* BitXor(Value* left, Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, std::bit_xor<typename ValueT::OperandType>());
}

BinaryOperatorFun bitXor[uint8_t(ValueKind::max)] =
{
    NotSupported, NotSupported, NotSupported, BitXor<SByteValue>, BitXor<ByteValue>, BitXor<ShortValue>, BitXor<UShortValue>,
    BitXor<IntValue>, BitXor<UIntValue>, BitXor<LongValue>, BitXor<ULongValue>, NotSupported, NotSupported
};

void Evaluator::Visit(BitXorNode& bitXorNode)
{
    EvaluateBinOp(bitXorNode, bitXor);
}

template<typename ValueT>
Value* BitAnd(Value* left, Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, std::bit_and<typename ValueT::OperandType>());
}

BinaryOperatorFun bitAnd[uint8_t(ValueKind::max)] =
{
    NotSupported, NotSupported, NotSupported, BitAnd<SByteValue>, BitAnd<ByteValue>, BitAnd<ShortValue>, BitAnd<UShortValue>,
    BitAnd<IntValue>, BitAnd<UIntValue>, BitAnd<LongValue>, BitAnd<ULongValue>, NotSupported, NotSupported
};

void Evaluator::Visit(BitAndNode& bitAndNode)
{
    EvaluateBinOp(bitAndNode, bitAnd);
}

template <typename ValueT, typename Op>
Value* BinaryPredEvaluate(Value* left, Value* right, Op op)
{
    ValueT* left_ = static_cast<ValueT*>(left);
    ValueT* right_ = static_cast<ValueT*>(right);
    return new BoolValue(op(left_->Value(), right_->Value()));
}

template<typename ValueT>
Value* Equal(Value* left, Value* right, const Span& span)
{
    return BinaryPredEvaluate<ValueT>(left, right, std::equal_to<typename ValueT::OperandType>());
}

BinaryOperatorFun equal[uint8_t(ValueKind::max)] =
{
    NotSupported, Equal<BoolValue>, Equal<CharValue>, Equal<SByteValue>, Equal<ByteValue>, Equal<ShortValue>, Equal<UShortValue>, 
    Equal<IntValue>, Equal<UIntValue>, Equal<LongValue>, Equal<ULongValue>, Equal<FloatValue>, Equal<DoubleValue>
};

void Evaluator::Visit(EqualNode& equalNode)
{
    EvaluateBinOp(equalNode, equal);
}

template<typename ValueT>
Value* NotEqual(Value* left, Value* right, const Span& span)
{
    return BinaryPredEvaluate<ValueT>(left, right, std::not_equal_to<typename ValueT::OperandType>());
}

BinaryOperatorFun notEqual[uint8_t(ValueKind::max)] =
{
    NotSupported, NotEqual<BoolValue>, NotEqual<CharValue>, NotEqual<SByteValue>, NotEqual<ByteValue>, NotEqual<ShortValue>, NotEqual<UShortValue>,
    NotEqual<IntValue>, NotEqual<UIntValue>, NotEqual<LongValue>, NotEqual<ULongValue>, NotEqual<FloatValue>, NotEqual<DoubleValue>
};

void Evaluator::Visit(NotEqualNode& notEqualNode)
{
    EvaluateBinOp(notEqualNode, notEqual);
}

template<typename ValueT>
Value* Less(Value* left, Value* right, const Span& span)
{
    return BinaryPredEvaluate<ValueT>(left, right, std::less<typename ValueT::OperandType>());
}

BinaryOperatorFun less[uint8_t(ValueKind::max)] =
{
    NotSupported, Less<BoolValue>, Less<CharValue>, Less<SByteValue>, Less<ByteValue>, Less<ShortValue>, Less<UShortValue>,
    Less<IntValue>, Less<UIntValue>, Less<LongValue>, Less<ULongValue>, Less<FloatValue>, Less<DoubleValue>
};

void Evaluator::Visit(LessNode& lessNode)
{
    EvaluateBinOp(lessNode, less);
}

template<typename ValueT>
Value* Greater(Value* left, Value* right, const Span& span)
{
    return BinaryPredEvaluate<ValueT>(left, right, std::greater<typename ValueT::OperandType>());
}

BinaryOperatorFun greater[uint8_t(ValueKind::max)] =
{
    NotSupported, Greater<BoolValue>, Greater<CharValue>, Greater<SByteValue>, Greater<ByteValue>, Greater<ShortValue>, Greater<UShortValue>,
    Greater<IntValue>, Greater<UIntValue>, Greater<LongValue>, Greater<ULongValue>, Greater<FloatValue>, Greater<DoubleValue>
};

void Evaluator::Visit(GreaterNode& greaterNode)
{
    EvaluateBinOp(greaterNode, greater);
}

template<typename ValueT>
Value* LessOrEqual(Value* left, Value* right, const Span& span)
{
    return BinaryPredEvaluate<ValueT>(left, right, std::less_equal<typename ValueT::OperandType>());
}

BinaryOperatorFun lessOrEqual[uint8_t(ValueKind::max)] =
{
    NotSupported, LessOrEqual<BoolValue>, LessOrEqual<CharValue>, LessOrEqual<SByteValue>, LessOrEqual<ByteValue>, LessOrEqual<ShortValue>, LessOrEqual<UShortValue>,
    LessOrEqual<IntValue>, LessOrEqual<UIntValue>, LessOrEqual<LongValue>, LessOrEqual<ULongValue>, LessOrEqual<FloatValue>, LessOrEqual<DoubleValue>
};

void Evaluator::Visit(LessOrEqualNode& lessOrEqualNode)
{
    EvaluateBinOp(lessOrEqualNode, lessOrEqual);
}

template<typename ValueT>
Value* GreaterOrEqual(Value* left, Value* right, const Span& span)
{
    return BinaryPredEvaluate<ValueT>(left, right, std::greater_equal<typename ValueT::OperandType>());
}

BinaryOperatorFun greaterOrEqual[uint8_t(ValueKind::max)] =
{
    NotSupported, GreaterOrEqual<BoolValue>, GreaterOrEqual<CharValue>, GreaterOrEqual<SByteValue>, GreaterOrEqual<ByteValue>, GreaterOrEqual<ShortValue>, GreaterOrEqual<UShortValue>,
    GreaterOrEqual<IntValue>, GreaterOrEqual<UIntValue>, GreaterOrEqual<LongValue>, GreaterOrEqual<ULongValue>, GreaterOrEqual<FloatValue>, GreaterOrEqual<DoubleValue>
};

void Evaluator::Visit(GreaterOrEqualNode& greaterOrEqualNode)
{
    EvaluateBinOp(greaterOrEqualNode, greaterOrEqual);
}

template<typename T>
struct shiftLeftFun : std::binary_function<T, T, T>
{
    T operator()(const T& left, const T& right) const
    {
        return left << right;
    }
};

template<typename ValueT>
Value* ShiftLeft(Value* left, Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, shiftLeftFun<typename ValueT::OperandType>());
}

BinaryOperatorFun shiftLeft[uint8_t(ValueKind::max)] =
{
    NotSupported, NotSupported, NotSupported, ShiftLeft<SByteValue>, ShiftLeft<ByteValue>, ShiftLeft<ShortValue>, ShiftLeft<UShortValue>,
    ShiftLeft<IntValue>, ShiftLeft<UIntValue>, ShiftLeft<LongValue>, ShiftLeft<ULongValue>, NotSupported, NotSupported
};

void Evaluator::Visit(ShiftLeftNode& shiftLeftNode)
{
    EvaluateBinOp(shiftLeftNode, shiftLeft);
}

template<typename T>
struct shiftRightFun : std::binary_function<T, T, T>
{
    T operator()(const T& left, const T& right) const
    {
        return left >> right;
    }
};

template<typename ValueT>
Value* ShiftRight(Value* left, Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, shiftRightFun<typename ValueT::OperandType>());
}

BinaryOperatorFun shiftRight[uint8_t(ValueKind::max)] =
{
    NotSupported, NotSupported, NotSupported, ShiftRight<SByteValue>, ShiftRight<ByteValue>, ShiftRight<ShortValue>, ShiftRight<UShortValue>,
    ShiftRight<IntValue>, ShiftRight<UIntValue>, ShiftRight<LongValue>, ShiftRight<ULongValue>, NotSupported, NotSupported
};

void Evaluator::Visit(ShiftRightNode& shiftRightNode)
{
    EvaluateBinOp(shiftRightNode, shiftRight);
}

template<typename ValueT>
Value* Add(Value* left, Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, std::plus<typename ValueT::OperandType>());
}

BinaryOperatorFun add[uint8_t(ValueKind::max)] =
{
    NotSupported, NotSupported, NotSupported, Add<SByteValue>, Add<ByteValue>, Add<ShortValue>, Add<UShortValue>,
    Add<IntValue>, Add<UIntValue>, Add<LongValue>, Add<ULongValue>, Add<FloatValue>, Add<DoubleValue>
};

void Evaluator::Visit(AddNode& addNode)
{
    EvaluateBinOp(addNode, add);
}

template<typename ValueT>
Value* Sub(Value* left, Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, std::minus<typename ValueT::OperandType>());
}

BinaryOperatorFun sub[uint8_t(ValueKind::max)] =
{
    NotSupported, NotSupported, NotSupported, Sub<SByteValue>, Sub<ByteValue>, Sub<ShortValue>, Sub<UShortValue>,
    Sub<IntValue>, Sub<UIntValue>, Sub<LongValue>, Sub<ULongValue>, Sub<FloatValue>, Sub<DoubleValue>
};

void Evaluator::Visit(SubNode& subNode)
{
    EvaluateBinOp(subNode, sub);
}

template<typename ValueT>
Value* Mul(Value* left, Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, std::multiplies<typename ValueT::OperandType>());
}

BinaryOperatorFun mul[uint8_t(ValueKind::max)] =
{
    NotSupported, NotSupported, NotSupported, Mul<SByteValue>, Mul<ByteValue>, Mul<ShortValue>, Mul<UShortValue>,
    Mul<IntValue>, Mul<UIntValue>, Mul<LongValue>, Mul<ULongValue>, Mul<FloatValue>, Mul<DoubleValue>
};

void Evaluator::Visit(MulNode& mulNode)
{
    EvaluateBinOp(mulNode, mul);
}

template<typename ValueT>
Value* Div(Value* left, Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, std::divides<typename ValueT::OperandType>());
}

BinaryOperatorFun div[uint8_t(ValueKind::max)] =
{
    NotSupported, NotSupported, NotSupported, Div<SByteValue>, Div<ByteValue>, Div<ShortValue>, Div<UShortValue>,
    Div<IntValue>, Div<UIntValue>, Div<LongValue>, Div<ULongValue>, Div<FloatValue>, Div<DoubleValue>
};

void Evaluator::Visit(DivNode& divNode)
{
    EvaluateBinOp(divNode, div);
}

template<typename ValueT>
Value* Rem(Value* left, Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, std::modulus<typename ValueT::OperandType>());
}

BinaryOperatorFun rem[uint8_t(ValueKind::max)] =
{
    NotSupported, NotSupported, NotSupported, Rem<SByteValue>, Rem<ByteValue>, Rem<ShortValue>, Rem<UShortValue>,
    Rem<IntValue>, Rem<UIntValue>, Rem<LongValue>, Rem<ULongValue>, NotSupported, NotSupported
};

void Evaluator::Visit(RemNode& remNode)
{
    EvaluateBinOp(remNode, rem);
}

template<typename ValueT, typename Op>
Value* UnaryEvaluate(Value* subject, Op op)
{
    ValueT* subject_ = static_cast<ValueT*>(subject);
    return new ValueT(op(subject_->Value()));
}

template<typename ValueT>
Value* Not(Value* subject, const Span& span)
{
    return UnaryEvaluate<ValueT>(subject, std::logical_not<typename ValueT::OperandType>());
}

UnaryOperatorFun not_[uint8_t(ValueKind::max)] =
{
    NotSupported, Not<BoolValue>, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported
};

void Evaluator::EvaluateUnaryOp(const UnaryNode& node, UnaryOperatorFun* fun)
{
    node.Child()->Accept(*this);
    std::unique_ptr<Value> child(value.release());
    ValueKind childKind = child->GetValueKind();
    ValueKind operationKind = childKind;
    if (targetKind > childKind)
    {
        operationKind = targetKind;
    }
    std::unique_ptr<Value> child_(child->As(operationKind, cast, span));
    UnaryOperatorFun operation = fun[uint8_t(operationKind)];
    value.reset(operation(child_.get(), span));
}

void Evaluator::Visit(NotNode& notNode)
{
    EvaluateUnaryOp(notNode, not_);
}

template<typename T>
struct identityFun: std::unary_function<T, T>
{
    const T& operator()(const T& child) const
    {
        return child;
    }
};

template<typename ValueT>
Value* UnaryPlus(Value* subject, const Span& span)
{
    return UnaryEvaluate<ValueT>(subject, identityFun<typename ValueT::OperandType>());
}

UnaryOperatorFun unaryPlus[uint8_t(ValueKind::max)] =
{
    NotSupported, NotSupported, NotSupported, UnaryPlus<SByteValue>, UnaryPlus<ByteValue>, UnaryPlus<ShortValue>, UnaryPlus<UShortValue>, 
    UnaryPlus<IntValue>, UnaryPlus<UIntValue>, UnaryPlus<LongValue>, UnaryPlus<ULongValue>, UnaryPlus<FloatValue>, UnaryPlus<DoubleValue>
};

void Evaluator::Visit(UnaryPlusNode& unaryPlusNode)
{
    EvaluateUnaryOp(unaryPlusNode, unaryPlus);
}

template<typename ValueT>
Value* UnaryMinus(Value* subject, const Span& span)
{
    return UnaryEvaluate<ValueT>(subject, std::negate<typename ValueT::OperandType>());
}

UnaryOperatorFun unaryMinus[uint8_t(ValueKind::max)] =
{
    NotSupported, NotSupported, NotSupported, UnaryMinus<SByteValue>, UnaryMinus<ByteValue>, UnaryMinus<ShortValue>, UnaryMinus<UShortValue>,
    UnaryMinus<IntValue>, UnaryMinus<UIntValue>, UnaryMinus<LongValue>, UnaryMinus<ULongValue>, UnaryMinus<FloatValue>, UnaryMinus<DoubleValue>
};

void Evaluator::Visit(UnaryMinusNode& unaryMinusNode)
{
    EvaluateUnaryOp(unaryMinusNode, unaryMinus);
}

template<typename ValueT>
Value* Complement(Value* subject, const Span& span)
{
    return UnaryEvaluate<ValueT>(subject, std::bit_not<typename ValueT::OperandType>());
}

UnaryOperatorFun complement[uint8_t(ValueKind::max)] =
{
    NotSupported, NotSupported, NotSupported, Complement<SByteValue>, Complement<ByteValue>, Complement<ShortValue>, Complement<UShortValue>,
    Complement<IntValue>, Complement<UIntValue>, Complement<LongValue>, Complement<ULongValue>, NotSupported, NotSupported
};

void Evaluator::Visit(ComplementNode& complementNode)
{
    EvaluateUnaryOp(complementNode, complement);
}

void Evaluator::Visit(IsNode& isNode)
{
    throw Exception("cannot evaluate statically", isNode.GetSpan());
}

void Evaluator::Visit(AsNode& asNode)
{
    throw Exception("cannot evaluate statically", asNode.GetSpan());
}

void Evaluator::Visit(DotNode& dotNode)
{
    ScopedValue* scopedValue = dynamic_cast<ScopedValue*>(value.get());
    if (scopedValue)
    {
        ContainerSymbol* containerSymbol = scopedValue->GetContainerSymbol();
        ContainerScope* scope = containerSymbol->GetContainerScope();
        utf32_string memberStr = ToUtf32(dotNode.MemberStr());
        Symbol* symbol = scope->Lookup(StringPtr(memberStr.c_str()));
        if (symbol)
        {
            EvaluateSymbol(symbol, dotNode.GetSpan());
        }
        else
        {
            throw Exception("symbol '" + ToUtf8(containerSymbol->FullName()) + "' does not have member '" + dotNode.MemberStr() + "'", dotNode.GetSpan());
        }
    }
    else
    {
        throw Exception("expression '" + dotNode.Child()->ToString() + "' must denote a namespace, class type or enumerated type", dotNode.Child()->GetSpan());
    }
}

void Evaluator::Visit(ArrayNode& arrayNode)
{
    throw Exception("cannot evaluate statically", arrayNode.GetSpan());
}

void Evaluator::Visit(IndexingNode& indexingNode)
{
    throw Exception("cannot evaluate statically", indexingNode.GetSpan());
}

void Evaluator::Visit(InvokeNode& invokeNode)
{
    throw Exception("cannot evaluate statically", invokeNode.GetSpan());
}

void Evaluator::Visit(CastNode& castNode)
{
    Node* targetTypeExpr = castNode.TargetTypeExpr();
    TypeSymbol* type = ResolveType(boundCompileUnit, containerScope, targetTypeExpr);
    SymbolType symbolType = type->GetSymbolType();
    ValueKind valueKind = GetValueKindFor(symbolType, castNode.GetSpan());
    value.reset(Evaluate(valueKind, true, containerScope, boundCompileUnit, castNode.SourceExpr()));
}

void Evaluator::Visit(NewNode& newNode)
{
    throw Exception("cannot evaluate statically", newNode.GetSpan());
}

void Evaluator::Visit(ThisNode& thisNode)
{
    throw Exception("cannot evaluate statically", thisNode.GetSpan());
}

void Evaluator::Visit(BaseNode& baseNode)
{
    throw Exception("cannot evaluate statically", baseNode.GetSpan());
}

void Evaluator::Visit(CompoundStatementNode& compoundStatementNode)
{
    throw Exception("cannot evaluate statically", compoundStatementNode.GetSpan());
}

void Evaluator::Visit(ReturnStatementNode& returnStatementNode)
{
    throw Exception("cannot evaluate statically", returnStatementNode.GetSpan());
}

void Evaluator::Visit(IfStatementNode& ifStatementNode)
{
    throw Exception("cannot evaluate statically", ifStatementNode.GetSpan());
}

void Evaluator::Visit(WhileStatementNode& whileStatementNode)
{
    throw Exception("cannot evaluate statically", whileStatementNode.GetSpan());
}

void Evaluator::Visit(DoStatementNode& doStatementNode)
{
    throw Exception("cannot evaluate statically", doStatementNode.GetSpan());
}

void Evaluator::Visit(ForStatementNode& forStatementNode)
{
    throw Exception("cannot evaluate statically", forStatementNode.GetSpan());
}

void Evaluator::Visit(BreakStatementNode& breakStatementNode)
{
    throw Exception("cannot evaluate statically", breakStatementNode.GetSpan());
}

void Evaluator::Visit(ContinueStatementNode& continueStatementNode)
{
    throw Exception("cannot evaluate statically", continueStatementNode.GetSpan());
}

void Evaluator::Visit(ConstructionStatementNode& constructionStatementNode)
{
    throw Exception("cannot evaluate statically", constructionStatementNode.GetSpan());
}

void Evaluator::Visit(AssignmentStatementNode& assignmentStatementNode)
{
    throw Exception("cannot evaluate statically", assignmentStatementNode.GetSpan());
}

void Evaluator::Visit(ExpressionStatementNode& expressionStatementNode)
{
    throw Exception("cannot evaluate statically", expressionStatementNode.GetSpan());
}

void Evaluator::Visit(EmptyStatementNode& emptyStatementNode)
{
    throw Exception("cannot evaluate statically", emptyStatementNode.GetSpan());
}

void Evaluator::Visit(IncrementStatementNode& incrementStatementNode)
{
    throw Exception("cannot evaluate statically", incrementStatementNode.GetSpan());
}

void Evaluator::Visit(DecrementStatementNode& decrementStatementNode)
{
    throw Exception("cannot evaluate statically", decrementStatementNode.GetSpan());
}

void Evaluator::Visit(EnumTypeNode& enumTypeNode)
{
    throw Exception("cannot evaluate statically", enumTypeNode.GetSpan());
}

void Evaluator::Visit(EnumConstantNode& enumConstantNode)
{
    throw Exception("cannot evaluate statically", enumConstantNode.GetSpan());
}

void Evaluator::Visit(ConstantNode& constantNode)
{
    throw Exception("cannot evaluate statically", constantNode.GetSpan());
}

Value* Evaluate(ValueKind targetKind, bool cast, ContainerScope* containerScope, BoundCompileUnit& boundCompileUnit, Node* value)
{
    Evaluator evaluator(targetKind, cast, containerScope, boundCompileUnit, value->GetSpan());
    value->Accept(evaluator);
    return evaluator.GetValue();
}

} } // namespace cminor::binder
