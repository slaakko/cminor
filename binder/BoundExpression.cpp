// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/BoundExpression.hpp>
#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/machine/Machine.hpp>

namespace cminor { namespace binder {

BoundExpression::BoundExpression(TypeSymbol* type_) : BoundNode(), type(type_)
{
}

BoundLiteral::BoundLiteral(TypeSymbol* type_, Constant value_) : BoundExpression(type_), value(value_)
{
}

void BoundLiteral::GenLoad(Machine& machine, Function& function)
{
    std::unique_ptr<Instruction> loadConstantInst = machine.CreateInst("loadconstant");
    ConstantId constantId = GetType()->GetAssembly()->GetConstantPool().GetIdFor(value);
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

BoundConstant::BoundConstant(TypeSymbol* type_, ConstantSymbol* constantSymbol_) : BoundExpression(type_), constantSymbol(constantSymbol_)
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

BoundLocalVariable::BoundLocalVariable(TypeSymbol* type_, LocalVariableSymbol* localVariableSymbol_) : BoundExpression(type_), localVariableSymbol(localVariableSymbol_)
{
}

void BoundLocalVariable::GenLoad(Machine& machine, Function& function)
{
    int32_t index = localVariableSymbol->Index();
    if (index != -1)
    {
        std::unique_ptr<Instruction> loadLocalInst = machine.CreateInst("loadlocal");
        loadLocalInst->SetIndex(index);
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
    }
    else
    {
        throw std::runtime_error("invalid local variable index");
    }
}

BoundMemberVariable::BoundMemberVariable(TypeSymbol* type_, MemberVariableSymbol* memberVariableSymbol_) : BoundExpression(type_), memberVariableSymbol(memberVariableSymbol_)
{
}

void BoundMemberVariable::GenLoad(Machine& machine, Function& function)
{
    int32_t index = memberVariableSymbol->Index();
    if (index != -1)
    {
        std::unique_ptr<Instruction> loadFieldInst = machine.CreateInst("loadfield");
        loadFieldInst->SetIndex(index);
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
    }
    else
    {
        throw std::runtime_error("invalid member variable index");
    }
}

BoundParameter::BoundParameter(TypeSymbol* type_, ParameterSymbol* parameterSymbol_) : BoundExpression(type_), parameterSymbol(parameterSymbol_)
{
}

void BoundParameter::GenLoad(Machine& machine, Function& function)
{
    int32_t index = parameterSymbol->Index();
    if (index != -1)
    {
        std::unique_ptr<Instruction> loadLocalInst = machine.CreateInst("loadlocal");
        loadLocalInst->SetIndex(index);
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
    }
    else
    {
        throw std::runtime_error("invalid parameter index");
    }
}

BoundConversion::BoundConversion(std::unique_ptr<BoundExpression>&& sourceExpr_, FunctionSymbol* conversionFun_) : 
    BoundExpression(conversionFun_->ConversionTargetType()), sourceExpr(std::move(sourceExpr_)), conversionFun(conversionFun_)
{
}

void BoundConversion::GenLoad(Machine& machine, Function& function)
{
    sourceExpr->GenLoad(machine, function);
    std::vector<GenObject*> emptyObjects;
    conversionFun->GenerateCode(machine, function, emptyObjects);
}

void BoundConversion::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to conversion");
}

BoundNamespace::BoundNamespace(NamespaceSymbol* ns_) : BoundExpression(nullptr), ns(ns_)
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

BoundFunctionCall::BoundFunctionCall(FunctionSymbol* functionSymbol_) : BoundExpression(functionSymbol_->ReturnType()), functionSymbol(functionSymbol_)
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
        argument->GenLoad(machine, function);
    }
    functionSymbol->GenerateCode(machine, function, objects);
}

void BoundFunctionCall::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to function call");
}

} } // namespace cminor::binder
