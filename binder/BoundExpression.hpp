// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_BOUND_EXPRESSION_INCLUDED
#define CMINOR_BINDER_BOUND_EXPRESSION_INCLUDED
#include <cminor/binder/BoundNode.hpp>
#include <cminor/symbols/ConstantSymbol.hpp>
#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/machine/GenObject.hpp>

namespace cminor { namespace binder {

using namespace cminor::symbols;

class BoundExpression : public BoundNode, public GenObject
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
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
private:
    Constant value;
};

class BoundConstant : public BoundExpression
{
public:
    BoundConstant(TypeSymbol* type_, ConstantSymbol* constantSymbol_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
private:
    ConstantSymbol* constantSymbol;
};

class BoundLocalVariable : public BoundExpression
{
public:
    BoundLocalVariable(TypeSymbol* type_, LocalVariableSymbol* localVariableSymbol_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
private:
    LocalVariableSymbol* localVariableSymbol;
};

class BoundMemberVariable : public BoundExpression
{
public:
    BoundMemberVariable(TypeSymbol* type_, MemberVariableSymbol* memberVariableSymbol_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
private:
    MemberVariableSymbol* memberVariableSymbol;
};

class BoundParameter : public BoundExpression
{
public:
    BoundParameter(TypeSymbol* type_, ParameterSymbol* parameterSymbol_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
private:
    ParameterSymbol* parameterSymbol;
};

class BoundConversion : public BoundExpression
{
public:
    BoundConversion(std::unique_ptr<BoundExpression>&& sourceExpr_, FunctionSymbol* conversionFun_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
private:
    std::unique_ptr<BoundExpression> sourceExpr;
    FunctionSymbol* conversionFun;
};

class BoundNamespace : public BoundExpression
{
public:
    BoundNamespace(NamespaceSymbol* ns_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
private:
    NamespaceSymbol* ns;
};

class BoundFunctionCall : public BoundExpression
{
public:
    BoundFunctionCall(FunctionSymbol* functionSymbol_);
    void AddArgument(std::unique_ptr<BoundExpression>&& argument);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
private:
    FunctionSymbol* functionSymbol;
    std::vector<std::unique_ptr<BoundExpression>> arguments;
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_BOUND_EXPRESSION_INCLUDED
