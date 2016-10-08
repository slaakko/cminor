// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_BOUND_EXPRESSION_INCLUDED
#define CMINOR_BINDER_BOUND_EXPRESSION_INCLUDED
#include <cminor/binder/BoundNode.hpp>
#include <cminor/symbols/ConstantSymbol.hpp>
#include <cminor/symbols/VariableSymbol.hpp>

namespace cminor { namespace binder {

using namespace cminor::symbols;

class BoundExpression : public BoundNode
{
public:
    BoundExpression(TypeSymbol* type_);
    TypeSymbol* GetType() const { return type; }
private:
    TypeSymbol* type;
};

class BoundLiteral : public BoundExpression
{
public:
    BoundLiteral(TypeSymbol* type_, Constant value_);
    Constant Value() const { return value; }
private:
    Constant value;
};

class BoundConstant : public BoundExpression
{
public:
    BoundConstant(TypeSymbol* type_, ConstantSymbol* constantSymbol_);
private:
    ConstantSymbol* constantSymbol;
};

class BoundLocalVariable : public BoundExpression
{
public:
    BoundLocalVariable(TypeSymbol* type_, LocalVariableSymbol* localVariableSymbol_);
private:
    LocalVariableSymbol* localVariableSymbol;
};

class BoundMemberVariable : public BoundExpression
{
public:
    BoundMemberVariable(TypeSymbol* type_, MemberVariableSymbol* memberVariableSymbol_);
private:
    MemberVariableSymbol* memberVariableSymbol;
};

class BoundParameter : public BoundExpression
{
public:
    BoundParameter(TypeSymbol* type_, ParameterSymbol* parameterSymbol_);
private:
    ParameterSymbol* parameterSymbol;
};

class BoundNamespace : public BoundExpression
{
public:
    BoundNamespace(NamespaceSymbol* ns_);
private:
    NamespaceSymbol* ns;
};

class BoundFunctionCall : public BoundExpression
{
public:
    BoundFunctionCall(TypeSymbol* type_);
    void AddArgument(std::unique_ptr<BoundExpression>&& argument);
private:
    std::vector<std::unique_ptr<BoundExpression>> arguments;
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_BOUND_EXPRESSION_INCLUDED
