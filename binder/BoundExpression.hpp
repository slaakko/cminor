// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_BOUND_EXPRESSION_INCLUDED
#define CMINOR_BINDER_BOUND_EXPRESSION_INCLUDED
#include <cminor/binder/BoundNode.hpp>
#include <cminor/symbols/ConstantSymbol.hpp>
#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/symbols/PropertySymbol.hpp>
#include <cminor/symbols/IndexerSymbol.hpp>
#include <cminor/machine/GenObject.hpp>

namespace cminor { namespace binder {

using namespace cminor::symbols;

enum class BoundExpressionFlags : uint8_t
{
    none = 0,
    argIsThisOrBase = 1 << 0
};

inline BoundExpressionFlags operator|(BoundExpressionFlags left, BoundExpressionFlags right)
{
    return BoundExpressionFlags(uint8_t(left) | uint8_t(right));
}

inline BoundExpressionFlags operator&(BoundExpressionFlags left, BoundExpressionFlags right)
{
    return BoundExpressionFlags(uint8_t(left) & uint8_t(right));
}

class BoundExpression : public BoundNode, public GenObject
{
public:
    BoundExpression(Assembly& assembly_, TypeSymbol* type_);
    TypeSymbol* GetType() const { return type; }
    void SetFlag(BoundExpressionFlags flag) { flags = flags | flag;  }
    bool GetFlag(BoundExpressionFlags flag) const { return (flags & flag) != BoundExpressionFlags::none;  }
    virtual bool IsComplete() const { return true; }
    virtual bool IsLvalueExpression() const { return false; }
    virtual bool ReturnsValue() const { return false; }
private:
    TypeSymbol* type;
    BoundExpressionFlags flags;
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

class BoundEnumConstant : public BoundExpression
{
public:
    BoundEnumConstant(Assembly& assembly_, TypeSymbol* type_, EnumConstantSymbol* enumConstantSymbol_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
private:
    EnumConstantSymbol* enumConstantSymbol;
};

class BoundLocalVariable : public BoundExpression
{
public:
    BoundLocalVariable(Assembly& assembly_, TypeSymbol* type_, LocalVariableSymbol* localVariableSymbol_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool IsLvalueExpression() const override { return true; }
private:
    LocalVariableSymbol* localVariableSymbol;
};

class BoundMemberVariable : public BoundExpression
{
public:
    BoundMemberVariable(Assembly& assembly_, TypeSymbol* type_, MemberVariableSymbol* memberVariableSymbol_);
    void SetClassObject(std::unique_ptr<BoundExpression>&& classObject_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool IsLvalueExpression() const override { return true; }
    MemberVariableSymbol* GetMemberVariableSymbol() const { return memberVariableSymbol; }
    void SetStaticInitNeeded() { staticInitNeeded = true; }
private:
    std::unique_ptr<BoundExpression> classObject;
    MemberVariableSymbol* memberVariableSymbol;
    bool staticInitNeeded;
};

class BoundProperty : public BoundExpression
{
public:
    BoundProperty(Assembly& assembly_, TypeSymbol* type_, PropertySymbol* propertySymbol_);
    void SetClassObject(std::unique_ptr<BoundExpression>&& classObject_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool IsLvalueExpression() const override { return true; }
    PropertySymbol* GetPropertySymbol() const { return propertySymbol; }
    void SetStaticInitNeeded() { staticInitNeeded = true; }
private:
    std::unique_ptr<BoundExpression> classObject;
    PropertySymbol* propertySymbol;
    bool staticInitNeeded;
};

class BoundIndexer : public BoundExpression
{
public:
    BoundIndexer(Assembly& assembly_, TypeSymbol* type_, IndexerSymbol* indexerSymbol_, std::unique_ptr<BoundExpression>&& classObject_, std::unique_ptr<BoundExpression>&& index_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool IsLvalueExpression() const override { return true; }
    void SetStaticInitNeeded() { staticInitNeeded = true; }
private:
    std::unique_ptr<BoundExpression> classObject;
    std::unique_ptr<BoundExpression> index;
    IndexerSymbol* indexerSymbol;
    bool staticInitNeeded;
};

class BoundParameter : public BoundExpression
{
public:
    BoundParameter(Assembly& assembly_, TypeSymbol* type_, ParameterSymbol* parameterSymbol_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool IsLvalueExpression() const override { return true; }
private:
    ParameterSymbol* parameterSymbol;
};

class BoundArrayElement : public BoundExpression
{
public:
    BoundArrayElement(Assembly& assembly_, ArrayTypeSymbol* arrayTypeSymbol_, std::unique_ptr<BoundExpression>&& arr_, std::unique_ptr<BoundExpression>&& index_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool IsLvalueExpression() const override { return true; }
private:
    std::unique_ptr<BoundExpression> arr;
    std::unique_ptr<BoundExpression> index;
};

class BoundStringChar : public BoundExpression
{
public:
    BoundStringChar(Assembly& assembly_, TypeSymbol* charType_, std::unique_ptr<BoundExpression>&& str_, std::unique_ptr<BoundExpression>&& index_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
private:
    std::unique_ptr<BoundExpression> str;
    std::unique_ptr<BoundExpression> index;
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
    bool IsComplete() const override { return false; }
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
    bool IsComplete() const override { return false; }
};

class BoundFunctionGroupExpression : public BoundExpression
{
public:
    BoundFunctionGroupExpression(Assembly& assembly_, FunctionGroupSymbol* functionGroupSymbol_);
    FunctionGroupSymbol* FunctionGroup() const { return functionGroupSymbol; }
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool IsComplete() const override { return false; }
private:
    FunctionGroupSymbol* functionGroupSymbol;
};

class BoundMemberExpression : public BoundExpression
{
public:
    BoundMemberExpression(Assembly& assembly_, std::unique_ptr<BoundExpression>&& classObject_, std::unique_ptr<BoundExpression>&& member_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
    BoundExpression* ClassObject() const { return classObject.get(); }
    BoundExpression* ReleaseClassObject() { return classObject.release(); }
    BoundExpression* Member() const { return member.get(); }
    bool IsComplete() const override { return false; }
private:
    std::unique_ptr<BoundExpression> classObject;
    std::unique_ptr<BoundExpression> member;
};

enum class FunctionCallType : uint8_t
{
    regularCall, virtualCall, interfaceCall
};

class BoundFunctionCall : public BoundExpression
{
public:
    BoundFunctionCall(Assembly& assembly_, FunctionSymbol* functionSymbol_);
    FunctionSymbol* GetFunctionSymbol() const { return functionSymbol; }
    const std::vector<std::unique_ptr<BoundExpression>>& Arguments() const { return arguments; }
    void AddArgument(std::unique_ptr<BoundExpression>&& argument);
    void SetArguments(std::vector<std::unique_ptr<BoundExpression>>&& arguments_);
    std::vector<std::unique_ptr<BoundExpression>> ReleaseArguments() { return std::move(arguments); }
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
    void SetFunctionCallType(FunctionCallType callType_) { callType = callType_; }
    bool ReturnsValue() const override { return functionSymbol->ReturnType() && !functionSymbol->ReturnType()->IsVoidType(); }
private:
    FunctionSymbol* functionSymbol;
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    FunctionCallType callType;
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

class BoundIsExpression : public BoundExpression
{
public:
    BoundIsExpression(Assembly& assembly_, std::unique_ptr<BoundExpression>&& expr_, TypeSymbol* classType_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
    BoundExpression* Expr() const { return expr.get(); }
    TypeSymbol* ClassType() const { return classType; }
private:
    std::unique_ptr<BoundExpression> expr;
    TypeSymbol* classType;
};

class BoundAsExpression : public BoundExpression
{
public:
    BoundAsExpression(Assembly& assembly_, std::unique_ptr<BoundExpression>&& expr_, TypeSymbol* classType_);
    void GenLoad(Machine& machine, Function& function) override;
    void GenStore(Machine& machine, Function& function) override;
    void Accept(BoundNodeVisitor& visitor) override;
    BoundExpression* Expr() const { return expr.get(); }
    TypeSymbol* ClassType() const { return classType; }
private:
    std::unique_ptr<BoundExpression> expr;
    TypeSymbol* classType;
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_BOUND_EXPRESSION_INCLUDED
