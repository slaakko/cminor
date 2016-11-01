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

BoundExpression::BoundExpression(Assembly& assembly_, TypeSymbol* type_) : BoundNode(assembly_), type(type_), flags(BoundExpressionFlags::none)
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

BoundMemberVariable::BoundMemberVariable(Assembly& assembly_, TypeSymbol* type_, MemberVariableSymbol* memberVariableSymbol_) : 
    BoundExpression(assembly_, type_), memberVariableSymbol(memberVariableSymbol_)
{
}

void BoundMemberVariable::SetClassObject(std::unique_ptr<BoundExpression>&& classObject_)
{
    classObject = std::move(classObject_);
}

void BoundMemberVariable::GenLoad(Machine& machine, Function& function)
{
    int32_t index = memberVariableSymbol->Index();
    if (index != -1)
    {
        classObject->GenLoad(machine, function);
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
        classObject->GenLoad(machine, function);
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

BoundProperty::BoundProperty(Assembly& assembly_, TypeSymbol* type_, PropertySymbol* propertySymbol_) : BoundExpression(assembly_, type_), propertySymbol(propertySymbol_)
{
}

void BoundProperty::SetClassObject(std::unique_ptr<BoundExpression>&& classObject_)
{
    classObject = std::move(classObject_);
}

void BoundProperty::GenLoad(Machine& machine, Function& function)
{
    classObject->GenLoad(machine, function);
    std::vector<GenObject*> emptyObjects;
    Assert(propertySymbol->Getter(), "property has no getter");
    propertySymbol->Getter()->GenerateCall(machine, GetAssembly(), function, emptyObjects, 0);
}

void BoundProperty::GenStore(Machine& machine, Function& function)
{
    classObject->GenLoad(machine, function);
    std::unique_ptr<Instruction> swapInst = machine.CreateInst("swap");
    function.AddInst(std::move(swapInst));
    std::vector<GenObject*> emptyObjects;
    Assert(propertySymbol->Setter(), "property has no setter");
    propertySymbol->Setter()->GenerateCall(machine, GetAssembly(), function, emptyObjects, 0);
}

void BoundProperty::Accept(BoundNodeVisitor& visitor)
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

BoundArrayElement::BoundArrayElement(Assembly& assembly_, ArrayTypeSymbol* arrayTypeSymbol_, std::unique_ptr<BoundExpression>&& arr_, std::unique_ptr<BoundExpression>&& index_) :
    BoundExpression(assembly_, arrayTypeSymbol_->ElementType()), arr(std::move(arr_)), index(std::move(index_))
{
}

void BoundArrayElement::GenLoad(Machine& machine, Function& function)
{
    arr->GenLoad(machine, function);
    index->GenLoad(machine, function);
    std::unique_ptr<Instruction> loadElemInst = machine.CreateInst("loadarrayelem");
    function.AddInst(std::move(loadElemInst));
}

void BoundArrayElement::GenStore(Machine& machine, Function& function)
{
    arr->GenLoad(machine, function);
    index->GenLoad(machine, function);
    std::unique_ptr<Instruction> storeElemInst = machine.CreateInst("storearrayelem");
    function.AddInst(std::move(storeElemInst));
}

void BoundArrayElement::Accept(BoundNodeVisitor& visitor)
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
    conversionFun->GenerateCall(machine, GetAssembly(), function, emptyObjects, 0);
}

void BoundConversion::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to conversion");
}

void BoundConversion::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundNamespaceExpression::BoundNamespaceExpression(Assembly& assembly_, NamespaceSymbol* ns_) : BoundExpression(assembly_, nullptr), ns(ns_)
{
}

void BoundNamespaceExpression::GenLoad(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot load from namespace");
}

void BoundNamespaceExpression::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to namespace");
}

void BoundNamespaceExpression::Accept(BoundNodeVisitor& visitor)
{
    throw std::runtime_error("cannot visit bound namespace");
}

BoundTypeExpression::BoundTypeExpression(Assembly& assembly_, TypeSymbol* type_) : BoundExpression(assembly_, type_)
{
}

void BoundTypeExpression::GenLoad(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot load from type");
}

void BoundTypeExpression::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to type");
}

void BoundTypeExpression::Accept(BoundNodeVisitor& visitor)
{
    throw std::runtime_error("cannot visit bound type expression");
}

BoundFunctionGroupExpression::BoundFunctionGroupExpression(Assembly& assembly_, FunctionGroupSymbol* functionGroupSymbol_) : BoundExpression(assembly_, nullptr), functionGroupSymbol(functionGroupSymbol_)
{
}

void BoundFunctionGroupExpression::GenLoad(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot load from function group");
}

void BoundFunctionGroupExpression::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to function group");
}

void BoundFunctionGroupExpression::Accept(BoundNodeVisitor& visitor)
{
    throw std::runtime_error("cannot visit bound function group");
}

BoundMemberExpression::BoundMemberExpression(Assembly& assembly_, std::unique_ptr<BoundExpression>&& classObject_, std::unique_ptr<BoundExpression>&& member_) :
    BoundExpression(assembly_, member_->GetType()), classObject(std::move(classObject_)), member(std::move(member_))
{
}

void BoundMemberExpression::GenLoad(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot load from bound member expression");
}

void BoundMemberExpression::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to from bound member expression");
}

void BoundMemberExpression::Accept(BoundNodeVisitor& visitor)
{
    throw std::runtime_error("cannot visit member expression");
}

BoundFunctionCall::BoundFunctionCall(Assembly& assembly_, FunctionSymbol* functionSymbol_) : 
    BoundExpression(assembly_, functionSymbol_->ReturnType()), functionSymbol(functionSymbol_), genVirtualCall(false)
{
}

void BoundFunctionCall::AddArgument(std::unique_ptr<BoundExpression>&& argument)
{
    arguments.push_back(std::move(argument));
}

void BoundFunctionCall::SetArguments(std::vector<std::unique_ptr<BoundExpression>>&& arguments_)
{
    arguments = std::move(arguments_);
}

void BoundFunctionCall::GenLoad(Machine& machine, Function& function)
{
    std::vector<GenObject*> objects;
    for (const std::unique_ptr<BoundExpression>& argument : arguments)
    {
        objects.push_back(argument.get());
    }
    if (genVirtualCall)
    {
        MemberFunctionSymbol* memberFunctionSymbol = dynamic_cast<MemberFunctionSymbol*>(functionSymbol);
        Assert(memberFunctionSymbol, "member function symbol exptected");
        memberFunctionSymbol->GenerateVirtualCall(machine, GetAssembly(), function, objects);
    }
    else
    {
        functionSymbol->GenerateCall(machine, GetAssembly(), function, objects, 0);
    }
}

void BoundFunctionCall::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to function call");
}

void BoundFunctionCall::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundNewExpression::BoundNewExpression(BoundFunctionCall* boundFunctionCall_, TypeSymbol* type_) : BoundExpression(boundFunctionCall_->GetAssembly(), type_),
    functionSymbol(boundFunctionCall_->GetFunctionSymbol()), arguments(std::move(boundFunctionCall_->ReleaseArguments()))
{
}

void BoundNewExpression::GenLoad(Machine& machine, Function& function) 
{
    std::unique_ptr<Instruction> createInst;
    createInst = machine.CreateInst("createo");
    ConstantPool& constantPool = GetAssembly().GetConstantPool();
    ClassTypeSymbol* containingClass = functionSymbol->ContainingClass();
    Assert(containingClass, "containing class not found");
    utf32_string fullClassName = containingClass->FullName();
    Constant fullClassNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(fullClassName.c_str())));
    CreateObjectInst* createObjectInst = dynamic_cast<CreateObjectInst*>(createInst.get());
    Assert(createObjectInst, "CreateObject instruction expected");
    createObjectInst->SetTypeName(fullClassNameConstant);
    function.AddInst(std::move(createInst));
    std::unique_ptr<Instruction> dupInst = machine.CreateInst("dup");
    function.AddInst(std::move(dupInst));
    std::vector<GenObject*> objects;
    for (const std::unique_ptr<BoundExpression>& argument : arguments)
    {
        objects.push_back(argument.get());
    }
    functionSymbol->GenerateCall(machine, GetAssembly(), function, objects, 1);
}

void BoundNewExpression::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store to new expression");
}

void BoundNewExpression::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundBooleanBinaryExpression::BoundBooleanBinaryExpression(Assembly& assembly_, BoundExpression* left_, BoundExpression* right_) : BoundExpression(assembly_, left_->GetType()), left(left_), right(right_)
{
}

BoundConjunction::BoundConjunction(Assembly& assembly_, BoundExpression* left_, BoundExpression* right_) : BoundBooleanBinaryExpression(assembly_, left_, right_)
{
}

void BoundConjunction::GenLoad(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot load bound conjunction");
}

void BoundConjunction::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store bound conjunction");
}

void BoundConjunction::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundDisjunction::BoundDisjunction(Assembly& assembly_, BoundExpression* left_, BoundExpression* right_) : BoundBooleanBinaryExpression(assembly_, left_, right_)
{
}

void BoundDisjunction::GenLoad(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot load bound disjunction");
}

void BoundDisjunction::GenStore(Machine& machine, Function& function)
{
    throw std::runtime_error("cannot store bound disjunction");
}

void BoundDisjunction::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::binder
