// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/ObjectFun.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/machine/Machine.hpp>

namespace cminor { namespace symbols {

void CreateDefaultConstructor(Assembly& assembly, ClassTypeSymbol* classTypeSymbol)
{
    ConstantPool& constantPool = assembly.GetConstantPool();
    ConstructorSymbol* defaultConstructorSymbol = new ConstructorSymbol(Span(), constantPool.GetEmptyStringConstant());
    defaultConstructorSymbol->SetAssembly(&assembly);
    defaultConstructorSymbol->SetPublic();
    defaultConstructorSymbol->SetGroupNameConstant(constantPool.GetConstant(constantPool.Install(U"@constructor")));
    ParameterSymbol* thisParameterSymbol = new ParameterSymbol(Span(), constantPool.GetConstant(constantPool.Install(U"this")));
    thisParameterSymbol->SetAssembly(&assembly);
    thisParameterSymbol->SetType(classTypeSymbol);
    thisParameterSymbol->SetBound();
    defaultConstructorSymbol->AddSymbol(std::unique_ptr<Symbol>(thisParameterSymbol));
    defaultConstructorSymbol->ComputeName();
    classTypeSymbol->AddSymbol(std::unique_ptr<Symbol>(defaultConstructorSymbol));
}

ObjectDefaultInit::ObjectDefaultInit(const Span& span_, Constant name_) : BasicTypeInit(span_, name_)
{
}

void ObjectDefaultInit::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects)
{
    std::unique_ptr<Instruction> inst = machine.CreateInst("def", "object");
    function.AddInst(std::move(inst));
    GenObject* target = objects[0];
    target->GenStore(machine, function);
}

ObjectCopyInit::ObjectCopyInit(const Span& span_, Constant name_) : BasicTypeInit(span_, name_)
{
}

void ObjectCopyInit::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects)
{
    Assert(objects.size() == 2, "init needs two objects");
    GenObject* source = objects[1];
    source->GenLoad(machine, function);
    std::unique_ptr<Instruction> inst = machine.CreateInst("copy", "object");
    function.AddInst(std::move(inst));
    GenObject* target = objects[0];
    target->GenStore(machine, function);
}

ObjectNullInit::ObjectNullInit(const Span& span_, Constant name_) : BasicTypeInit(span_, name_)
{
}

void ObjectNullInit::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects)
{
    std::unique_ptr<Instruction> inst = machine.CreateInst("def", "object");
    function.AddInst(std::move(inst));
    GenObject* target = objects[0];
    target->GenStore(machine, function);
}

void CreateBasicTypeObjectFun(Assembly& assembly, ClassTypeSymbol* classType, TypeSymbol* boolType)
{
    ConstantPool& constantPool = assembly.GetConstantPool();
    Constant thisParamName = constantPool.GetConstant(constantPool.Install(U"this"));
    Constant thatParamName = constantPool.GetConstant(constantPool.Install(U"that"));
    ParameterSymbol* thisParam1 = new ParameterSymbol(Span(), thisParamName);
    thisParam1->SetAssembly(&assembly);
    thisParam1->SetType(classType);
    ObjectDefaultInit* defaultInit = new ObjectDefaultInit(Span(), constantPool.GetEmptyStringConstant());
    defaultInit->SetAssembly(&assembly);
    defaultInit->SetType(classType);
    defaultInit->AddSymbol(std::unique_ptr<Symbol>(thisParam1));
    defaultInit->ComputeName();
    classType->AddSymbol(std::unique_ptr<FunctionSymbol>(defaultInit));

    ParameterSymbol* thisParam2 = new ParameterSymbol(Span(), thisParamName);
    thisParam2->SetAssembly(&assembly);
    thisParam2->SetType(classType);
    ParameterSymbol* thatParam2 = new ParameterSymbol(Span(), thatParamName);
    thatParam2->SetAssembly(&assembly);
    thatParam2->SetType(classType);
    ObjectCopyInit* copyInit = new ObjectCopyInit(Span(), constantPool.GetEmptyStringConstant());
    copyInit->SetAssembly(&assembly);
    copyInit->SetType(classType);
    copyInit->AddSymbol(std::unique_ptr<Symbol>(thisParam2));
    copyInit->AddSymbol(std::unique_ptr<Symbol>(thatParam2));
    copyInit->ComputeName();
    classType->AddSymbol(std::unique_ptr<FunctionSymbol>(copyInit));

    TypeSymbol* nullRefType = assembly.GetSymbolTable().GetType(U"@nullref");

    ParameterSymbol* thisParam3 = new ParameterSymbol(Span(), thisParamName);
    thisParam3->SetAssembly(&assembly);
    thisParam3->SetType(classType);
    ParameterSymbol* thatParam3 = new ParameterSymbol(Span(), thatParamName);
    thatParam3->SetAssembly(&assembly);
    thatParam3->SetType(nullRefType);
    ObjectNullInit* nullInit = new ObjectNullInit(Span(), constantPool.GetEmptyStringConstant());
    nullInit->SetAssembly(&assembly);
    nullInit->SetType(classType);
    nullInit->AddSymbol(std::unique_ptr<Symbol>(thisParam3));
    nullInit->AddSymbol(std::unique_ptr<Symbol>(thatParam3));
    nullInit->ComputeName();
    classType->AddSymbol(std::unique_ptr<FunctionSymbol>(nullInit));

    BasicTypeReturn* returnFun = new BasicTypeReturn(Span(), constantPool.GetEmptyStringConstant());
    returnFun->SetAssembly(&assembly);
    returnFun->SetType(classType);
    returnFun->SetReturnType(classType);
    Constant valueParamName = constantPool.GetConstant(constantPool.Install(U"value"));
    ParameterSymbol* valueParam = new ParameterSymbol(Span(), valueParamName);
    valueParam->SetAssembly(&assembly);
    valueParam->SetType(classType);
    returnFun->AddSymbol(std::unique_ptr<Symbol>(valueParam));
    returnFun->ComputeName();
    classType->AddSymbol(std::unique_ptr<FunctionSymbol>(returnFun));
}

void CreateBasicTypeObjectFun(Assembly& assembly, ClassTypeSymbol* classType)
{
    TypeSymbol* boolType = assembly.GetSymbolTable().GetType(U"bool");
    CreateBasicTypeObjectFun(assembly, classType, boolType);
}

void InitObjectFun(Assembly& assembly)
{
    TypeSymbol* type = assembly.GetSymbolTable().GetType(U"object");
    ClassTypeSymbol* objectType = dynamic_cast<ClassTypeSymbol*>(type);
    Assert(objectType, "object type expected");
    TypeSymbol* boolType = assembly.GetSymbolTable().GetType(U"bool");
    CreateBasicTypeObjectFun(assembly, objectType, boolType);
    CreateDefaultConstructor(assembly, objectType);
    TypeSymbol* stype = assembly.GetSymbolTable().GetType(U"string");
    ClassTypeSymbol* stringType = dynamic_cast<ClassTypeSymbol*>(stype);
    CreateBasicTypeObjectFun(assembly, stringType, boolType);
    CreateDefaultConstructor(assembly, stringType);
}

} } // namespace cminor::symbols