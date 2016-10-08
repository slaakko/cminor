// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/BoundExpression.hpp>

namespace cminor { namespace binder {

BoundExpression::BoundExpression(TypeSymbol* type_) : BoundNode(), type(type_)
{
}

BoundLiteral::BoundLiteral(TypeSymbol* type_, Constant value_) : BoundExpression(type_), value(value_)
{
}

BoundConstant::BoundConstant(TypeSymbol* type_, ConstantSymbol* constantSymbol_) : BoundExpression(type_), constantSymbol(constantSymbol_)
{
}

BoundLocalVariable::BoundLocalVariable(TypeSymbol* type_, LocalVariableSymbol* localVariableSymbol_) : BoundExpression(type_), localVariableSymbol(localVariableSymbol_)
{
}

BoundMemberVariable::BoundMemberVariable(TypeSymbol* type_, MemberVariableSymbol* memberVariableSymbol_) : BoundExpression(type_), memberVariableSymbol(memberVariableSymbol_)
{
}

BoundParameter::BoundParameter(TypeSymbol* type_, ParameterSymbol* parameterSymbol_) : BoundExpression(type_), parameterSymbol(parameterSymbol_)
{
}

BoundNamespace::BoundNamespace(NamespaceSymbol* ns_) : BoundExpression(nullptr), ns(ns_)
{
}

BoundFunctionCall::BoundFunctionCall(TypeSymbol* type_) : BoundExpression(type_)
{
}

void BoundFunctionCall::AddArgument(std::unique_ptr<BoundExpression>&& argument)
{
    arguments.push_back(std::move(argument));
}

} } // namespace cminor::binder
