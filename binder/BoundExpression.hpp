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
    BoundExpression(Assembly& assembly_, TypeSymbol* type_);
    TypeSymbol* GetType() const { return type; }
private:
    TypeSymbol* type;
};

class BoundLiteral : public BoundExpression
{
public:
    BoundLiteral(Assembly& assembly_, TypeSymbol* type_, Constant value_);
    Constant Value() const { return value; }
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
private:
    Constant value;
};

class BoundConstant : public BoundExpression
{
public:
    BoundConstant(Assembly& assembly_, TypeSymbol* type_, ConstantSymbol* constantSymbol_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
private:
    ConstantSymbol* constantSymbol;
};

class BoundLocalVariable : public BoundExpression
{
public:
    BoundLocalVariable(Assembly& assembly_, TypeSymbol* type_, LocalVariableSymbol* localVariableSymbol_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
private:
    LocalVariableSymbol* localVariableSymbol;
};

class BoundMemberVariable : public BoundExpression
{
public:
    BoundMemberVariable(Assembly& assembly_, TypeSymbol* type_, MemberVariableSymbol* memberVariableSymbol_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
private:
    MemberVariableSymbol* memberVariableSymbol;
};

class BoundParameter : public BoundExpression
{
public:
    BoundParameter(Assembly& assembly_, TypeSymbol* type_, ParameterSymbol* parameterSymbol_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
private:
    ParameterSymbol* parameterSymbol;
};

class BoundConversion : public BoundExpression
{
public:
    BoundConversion(Assembly& assembly_, std::unique_ptr<BoundExpression>&& sourceExpr_, FunctionSymbol* conversionFun_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
private:
    std::unique_ptr<BoundExpression> sourceExpr;
    FunctionSymbol* conversionFun;
};

class BoundNamespaceExpression : public BoundExpression
{
public:
    BoundNamespaceExpression(Assembly& assembly_, NamespaceSymbol* ns_);
    NamespaceSymbol* Ns() const { return ns; }
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
private:
    NamespaceSymbol* ns;
};

class BoundTypeExpression : public BoundExpression
{
public:
    BoundTypeExpression(Assembly& assembly_, TypeSymbol* type_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
};

class BoundFunctionGroupExpression : public BoundExpression
{
public:
    BoundFunctionGroupExpression(Assembly& assembly_, FunctionGroupSymbol* functionGroupSymbol_);
    FunctionGroupSymbol* FunctionGroup() const { return functionGroupSymbol; }
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
private:
    FunctionGroupSymbol* functionGroupSymbol;
};

class BoundFunctionCall : public BoundExpression
{
public:
    BoundFunctionCall(Assembly& assembly_, FunctionSymbol* functionSymbol_);
    FunctionSymbol* GetFunctionSymbol() const { return functionSymbol; }
    const std::vector<std::unique_ptr<BoundExpression>>& Arguments() const { return arguments; }
    void AddArgument(std::unique_ptr<BoundExpression>&& argument);
    void SetArguments(std::vector<std::unique_ptr<BoundExpression>>&& arguments_);
    std::vector<std::unique_ptr<BoundExpression>> Arguments() { return std::move(arguments); }
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
private:
    FunctionSymbol* functionSymbol;
    std::vector<std::unique_ptr<BoundExpression>> arguments;
};

class BoundNewExpression : public BoundExpression
{
public:
    BoundNewExpression(BoundFunctionCall* boundFunctionCall_, TypeSymbol* type_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
private:
    FunctionSymbol* functionSymbol;
    std::vector<std::unique_ptr<BoundExpression>> arguments;
};

class BoundBooleanBinaryExpression : public BoundExpression
{
public:
    BoundBooleanBinaryExpression(Assembly& assembly_, BoundExpression* left_, BoundExpression* right_);
    BoundExpression* Left() const { return left.get(); }
    BoundExpression* Right() const { return right.get(); }
private:
    std::unique_ptr<BoundExpression> left;
    std::unique_ptr<BoundExpression> right;
};

class BoundConjunction : public BoundBooleanBinaryExpression
{
public:
    BoundConjunction(Assembly& assembly_, BoundExpression* left_, BoundExpression* right_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
};

class BoundDisjunction : public BoundBooleanBinaryExpression
{
public:
    BoundDisjunction(Assembly& assembly_, BoundExpression* left_, BoundExpression* right_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_BOUND_EXPRESSION_INCLUDED
