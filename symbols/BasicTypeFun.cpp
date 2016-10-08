// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/BasicTypeFun.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/machine/Machine.hpp>

namespace cminor { namespace symbols {

BasicTypeFun::BasicTypeFun(const Span& span_, Constant name_) : FunctionSymbol(span_, name_), type(nullptr)
{
}

void BasicTypeFun::Write(SymbolWriter& writer)
{
    utf32_string typeFullName = type->FullName();
    ConstantId id = GetAssembly()->GetConstantPool().GetIdFor(typeFullName);
    Assert(id != noConstantId, "got no id");
    id.Write(writer);
}

void BasicTypeFun::Read(SymbolReader& reader)
{
    ConstantId id;
    id.Read(reader);
    Constant constant = GetAssembly()->GetConstantPool().GetConstant(id);
    utf32_string typeFullName = constant.Value().AsStringLiteral();
    type = GetAssembly()->GetSymbolTable().GetType(typeFullName);
}

void BasicTypeFun::ComputeName()
{
    utf32_string name;
    if (ReturnType())
    {
        name.append(ReturnType()->FullName()).append(1, U' ');
    }
    name.append(GroupName().Value());
    name.append(1, U'(');
    int n = int(Parameters().size());
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
        {
            name.append(U", ");
        }
        ParameterSymbol* parameter = Parameters()[i];
        name.append(parameter->GetType()->FullName());
    }
    name.append(1, U')');
    SetName(StringPtr(name.c_str()));
}

BasicTypeConstructor::BasicTypeConstructor(const Span& span_, Constant name_) : BasicTypeFun(span_, name_)
{
}

void BasicTypeConstructor::ComputeName()
{
    ConstantPool& constantPool = GetAssembly()->GetConstantPool();
    Constant groupName = constantPool.GetConstant(constantPool.Install(StringPtr(U"@constructor")));
    SetGroupNameConstant(groupName);
    BasicTypeFun::ComputeName();
}

BasicTypeDefaultConstructor::BasicTypeDefaultConstructor(const Span& span_, Constant name_) : BasicTypeConstructor(span_, name_)
{
}

BasicTypeInitConstructor::BasicTypeInitConstructor(const Span& span_, Constant name_) : BasicTypeConstructor(span_, name_)
{
}

void BasicTypeInitConstructor::GenerateCode(Machine& machine, Function& function, std::vector<GenObject*>& objects)
{
    Assert(objects.size() == 2, "init needs two objects");
    GenObject* source = objects[1];
    source->GenLoad(machine, function);
    GenObject* target = objects[0];
    target->GenStore(machine, function);
}

BasicTypeAssignment::BasicTypeAssignment(const Span& span_, Constant name_) : BasicTypeFun(span_, name_)
{
}

void BasicTypeAssignment::ComputeName()
{
    ConstantPool& constantPool = GetAssembly()->GetConstantPool();
    Constant groupName = constantPool.GetConstant(constantPool.Install(StringPtr(U"operator=")));
    SetGroupNameConstant(groupName);
    BasicTypeFun::ComputeName();
}

void BasicTypeAssignment::GenerateCode(Machine& machine, Function& function, std::vector<GenObject*>& objects)
{
    Assert(objects.size() == 2, "assing needs two objects");
    GenObject* source = objects[1];
    source->GenLoad(machine, function);
    GenObject* target = objects[0];
    target->GenStore(machine, function);
}

void InitBasicTypeFun(Assembly& assembly, TypeSymbol* type)
{
    ConstantPool& constantPool = assembly.GetConstantPool();
    Constant thisParamName = constantPool.GetConstant(constantPool.Install(U"this"));
    Constant thatParamName = constantPool.GetConstant(constantPool.Install(U"that"));
    ParameterSymbol* thisParam1 = new ParameterSymbol(Span(), thisParamName);
    thisParam1->SetAssembly(&assembly);
    thisParam1->SetType(type);
    BasicTypeDefaultConstructor* defaultConstructor = new BasicTypeDefaultConstructor(Span(), constantPool.GetEmptyStringConstant());
    defaultConstructor->SetAssembly(&assembly);
    defaultConstructor->SetType(type);
    defaultConstructor->AddSymbol(std::unique_ptr<Symbol>(thisParam1));
    defaultConstructor->ComputeName();
    assembly.GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<FunctionSymbol>(defaultConstructor));

    ParameterSymbol* thisParam2 = new ParameterSymbol(Span(), thisParamName);
    thisParam2->SetAssembly(&assembly);
    thisParam2->SetType(type);
    ParameterSymbol* thatParam2 = new ParameterSymbol(Span(), thatParamName);
    thatParam2->SetAssembly(&assembly);
    thatParam2->SetType(type);
    BasicTypeInitConstructor* initConstructor = new BasicTypeInitConstructor(Span(), constantPool.GetEmptyStringConstant());
    initConstructor->SetAssembly(&assembly);
    initConstructor->SetType(type);
    initConstructor->AddSymbol(std::unique_ptr<Symbol>(thisParam2));
    initConstructor->AddSymbol(std::unique_ptr<Symbol>(thatParam2));
    initConstructor->ComputeName();
    assembly.GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<FunctionSymbol>(initConstructor));

    ParameterSymbol* thisParam3 = new ParameterSymbol(Span(), thisParamName);
    thisParam3->SetAssembly(&assembly);
    thisParam3->SetType(type);
    ParameterSymbol* thatParam3 = new ParameterSymbol(Span(), thatParamName);
    thatParam3->SetAssembly(&assembly);
    thatParam3->SetType(type);
    BasicTypeAssignment* assignment = new BasicTypeAssignment(Span(), constantPool.GetEmptyStringConstant());
    assignment->SetAssembly(&assembly);
    assignment->SetType(type);
    assignment->AddSymbol(std::unique_ptr<Symbol>(thisParam3));
    assignment->AddSymbol(std::unique_ptr<Symbol>(thatParam3));
    assignment->ComputeName();
    assembly.GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<FunctionSymbol>(assignment));
}

void InitBasicTypeFun(Assembly& assembly)
{
    TypeSymbol* boolType = assembly.GetSymbolTable().GetType(U"bool");
    InitBasicTypeFun(assembly, boolType);
    TypeSymbol* charType = assembly.GetSymbolTable().GetType(U"char");
    InitBasicTypeFun(assembly, charType);
    TypeSymbol* sbyteType = assembly.GetSymbolTable().GetType(U"sbyte");
    InitBasicTypeFun(assembly, sbyteType);
    TypeSymbol* byteType = assembly.GetSymbolTable().GetType(U"byte");
    InitBasicTypeFun(assembly, byteType);
    TypeSymbol* shortType = assembly.GetSymbolTable().GetType(U"short");
    InitBasicTypeFun(assembly, shortType);
    TypeSymbol* ushortType = assembly.GetSymbolTable().GetType(U"ushort");
    InitBasicTypeFun(assembly, ushortType);
    TypeSymbol* intType = assembly.GetSymbolTable().GetType(U"int");
    InitBasicTypeFun(assembly, intType);
    TypeSymbol* uintType = assembly.GetSymbolTable().GetType(U"uint");
    InitBasicTypeFun(assembly, uintType);
    TypeSymbol* longType = assembly.GetSymbolTable().GetType(U"long");
    InitBasicTypeFun(assembly, longType);
    TypeSymbol* ulongType = assembly.GetSymbolTable().GetType(U"ulong");
    InitBasicTypeFun(assembly, ulongType);
    TypeSymbol* floatType = assembly.GetSymbolTable().GetType(U"float");
    InitBasicTypeFun(assembly, floatType);
    TypeSymbol* doubleType = assembly.GetSymbolTable().GetType(U"double");
    InitBasicTypeFun(assembly, doubleType);
}

} } // namespace cminor::symbols
