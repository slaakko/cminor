// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/BoundExpression.hpp>
#include <cminor/binder/BoundNodeVisitor.hpp>
#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/machine/Machine.hpp>

namespace cminor { namespace binder {

BoundExpression::BoundExpression(Assembly& assembly_, TypeSymbol* type_) : BoundNode(assembly_), type(type_)
{
}

BoundLiteral::BoundLiteral(Assembly& assembly_, TypeSymbol* type_, Constant value_) : BoundExpression(assembly_, type_), value(value_)
{
}

void BoundLiteral::GenLoad(Machine& machine, Function& function)
{
    std::unique_ptr<Instruction> loadConstantInst = machine.CreateInst("loadconstant");
    ConstantId constantId = GetAssembly().GetConstantPool().GetIdFor(value);
    if (constantId != noConstantId)
    {
        loadConstantInst->SetIndex(constantId.Value());
        function.AddInst(std::move(loadConstantInst));
    }
    else
    {
        throw std::runtime_error("id for constant not found");
    }
}

void BoundLiteral::GenStore(Machine& machine, Function& function) 
{
    throw std::runtime_error("cannot store to literal");
}

void BoundLiteral::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundConstant::BoundConstant(Assembly& assembly_, TypeSymbol* type_, ConstantSymbol* constantSymbol_) : BoundExpression(assembly_, type_), constantSymbol(constantSymbol_)
{
}

void BoundConstant::GenLoad(Machine& machine, Function& function)
{
    std::unique_ptr<Instruction> loadConstantInst = machine.CreateInst("loadconstant");
    ConstantId constantId = GetType()->GetAssembly()->GetConstantPool().GetIdFor(constantSymbol->Value());
    if (constantId != noConstantId)
    {
        loadConstantInst->SetIndex(constantId.Value());
        function.AddInst(std::move(loadConstantInst));
    }
    else
    {
        throw std::runtime_error("id for constant not found");
    }
}

void BoundConstant::GenStore(Machine& machine, Function& function) 
{
    throw std::runtime_error("cannot store to constant");
}

void BoundConstant::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundLocalVariable::BoundLocalVariable(Assembly& assembly_, TypeSymbol* type_, LocalVariableSymbol* localVariableSymbol_) : BoundExpression(assembly_, type_), localVariableSymbol(localVariableSymbol_)
{
}

void BoundLocalVariable::GenLoad(Machine& machine, Function& function)
{
    int32_t index = localVariableSymbol->Index();
    if (index != -1)
    {
        std::unique_ptr<Instruction> loadLocalInst = machine.CreateInst("loadlocal");
        loadLocalInst->SetIndex(index);
        function.AddInst(std::move(loadLocalInst));
    }
    else
    {
        throw std::runtime_error("invalid local variable index");
    }
}

void BoundLocalVariable::GenStore(Machine& machine, Function& function)
{
    int32_t index = localVariableSymbol->Index();
    if (index != -1)
    {
        std::unique_ptr<Instruction> storeLocalInst = machine.CreateInst("storelocal");
        storeLocalInst->SetIndex(index);
        function.AddInst(std::move(storeLocalInst));
    }
    else
    {
        throw std::runtime_error("invalid local variable index");
    }
}

void BoundLocalVariable::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundMemberVariable::BoundMemberVariable(Assembly& assembly_, TypeSymbol* type_, MemberVariableSymbol* memberVariableSymbol_) : BoundExpression(assembly_, type_), memberVariableSymbol(memberVariableSymbol_)
{
}

void BoundMemberVariable::GenLoad(Machine& machine, Function& function)
{
    int32_t index = memberVariableSymbol->Index();
    if (index != -1)
    {
        std::unique_ptr<Instruction> loadFieldInst = machine.CreateInst("loadfield");
        loadFieldInst->SetIndex(index);
        function.AddInst(std::move(loadFieldInst));
    }
    else
    {
        throw std::runtime_error("invalid member variable index");
    }
}

void BoundMemberVariable::GenStore(Machine& machine, Function& function)
{
    int32_t index = memberVariableSymbol->Index();
    if (index != -1)
    {
        std::unique_ptr<Instruction> storeFieldlInst = machine.CreateInst("storefield");
        storeFieldlInst->SetIndex(index);
        function.AddInst(std::move(storeFieldlInst));
    }
    else
    {
        throw std::runtime_error("invalid member variable index");
    }
}

void BoundMemberVariable::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundParameter::BoundParameter(Assembly& assembly_, TypeSymbol* type_, ParameterSymbol* parameterSymbol_) : BoundExpression(assembly_, type_), parameterSymbol(parameterSymbol_)
{
}

void BoundParameter::GenLoad(Machine& machine, Function& function)
{
    int32_t index = parameterSymbol->Index();
    if (index != -1)
    {
        std::unique_ptr<Instruction> loadLocalInst = machine.CreateInst("loadlocal");
        loadLocalInst->SetIndex(index);
        function.AddInst(std::move(loadLocalInst));
    }
    else
    {
        throw std::runtime_error("invalid parameter index");
    }
}

void BoundParameter::GenStore(Machine& machine, Function& function)
{
    int32_t index = parameterSymbol->Index();
    if (index != -1)
    {
        std::unique_ptr<Instruction> storeLocalInst = machine.CreateInst("storelocal");
        storeLocalInst->SetIndex(index);
        function.AddInst(std::move(storeLocalInst));
    }
    else
    {
        throw std::runtime_error("invalid parameter index");
    }
}

void BoundParameter::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundConversion::BoundConversion(Assembly& assembly_, std::unique_ptr<BoundExpression>&& sourceExpr_, FunctionSymbol* conversionFun_) :
    BoundExpression(assembly_, conversionFun_->ConversionTargetType()), sourceExpr(std::move(sourceExpr_)), conversionFun(conversionFun_)
{
}

void BoundConversion::GenLoad(Machine& machine, Function& function)
{
    sourceExpr->GenLoad(machine, function);
    std::vector<GenObject*> emptyObjects;
    conversionFun->GenerateCall(machine, GetAssembly(), function, emptyObjects);
}

void BoundConversion::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to conversion");
}

void BoundConversion::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundNamespace::BoundNamespace(Assembly& assembly_, NamespaceSymbol* ns_) : BoundExpression(assembly_, nullptr), ns(ns_)
{
}

void BoundNamespace::GenLoad(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot load from namespace");
}

void BoundNamespace::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to namespace");
}

void BoundNamespace::Accept(BoundNodeVisitor& visitor)
{
    throw std::runtime_error("cannot visit bound namespace");
}

BoundFunctionCall::BoundFunctionCall(Assembly& assembly_, FunctionSymbol* functionSymbol_) : BoundExpression(assembly_, functionSymbol_->ReturnType()), functionSymbol(functionSymbol_)
{
}

void BoundFunctionCall::AddArgument(std::unique_ptr<BoundExpression>&& argument)
{
    arguments.push_back(std::move(argument));
}

void BoundFunctionCall::GenLoad(Machine& machine, Function& function)
{
    std::vector<GenObject*> objects;
    for (const std::unique_ptr<BoundExpression>& argument : arguments)
    {
        objects.push_back(argument.get());
    }
    functionSymbol->GenerateCall(machine, GetAssembly(), function, objects);
}

void BoundFunctionCall::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to function call");
}

void BoundFunctionCall::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::binder
