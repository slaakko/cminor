// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/RefTypeFun.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/VariableSymbol.hpp>

namespace cminor { namespace symbols {

RefTypeInit::RefTypeInit(const Span& span_, Constant name_) : BasicTypeInit(span_, name_)
{
}

void RefTypeInit::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    Assert(objects.size() == 2, "copy init needs two objects");
    GenObject* source = objects[1];
    source->GenLoad(machine, function);
    GenObject* target = objects[0];
    InstIndexRequest startStore;
    function.GetEmitter()->AddIndexRequest(&startStore);
    target->GenStore(machine, function);
    function.GetEmitter()->BackpatchConDisSet(startStore.Index());
    function.MapPCToSourceLine(startStore.Index(), function.GetEmitter()->CurrentSourceLine());
}

RefTypeAssignment::RefTypeAssignment(const Span& span_, Constant name_) : BasicTypeFun(span_, name_)
{
}

void RefTypeAssignment::ComputeName()
{
    ConstantPool& constantPool = GetAssembly()->GetConstantPool();
    Constant groupName = constantPool.GetConstant(constantPool.Install(StringPtr(U"@assignment")));
    SetGroupNameConstant(groupName);
    BasicTypeFun::ComputeName();
}

void RefTypeAssignment::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    Assert(objects.size() == 2, "assignment needs two objects");
    GenObject* source = objects[1];
    source->GenLoad(machine, function);
    GenObject* target = objects[0];
    InstIndexRequest startStore;
    function.GetEmitter()->AddIndexRequest(&startStore);
    target->GenStore(machine, function);
    function.GetEmitter()->BackpatchConDisSet(startStore.Index());
    function.MapPCToSourceLine(startStore.Index(), function.GetEmitter()->CurrentSourceLine());
}

void CreateRefTypeBasicFun(Assembly& assembly, RefTypeSymbol* refTypeSymbol)
{
    assembly.GetSymbolTable().BeginNamespace(StringPtr(U"System"), Span());
    ConstantPool& constantPool = assembly.GetConstantPool();
    Constant thisParamName = constantPool.GetConstant(constantPool.Install(U"this"));
    Constant thatParamName = constantPool.GetConstant(constantPool.Install(U"that"));
    RefTypeAssignment* assignmentFromBaseType = new RefTypeAssignment(Span(), constantPool.GetEmptyStringConstant());
    assignmentFromBaseType->SetAssembly(&assembly);
    assignmentFromBaseType->SetType(refTypeSymbol);
    ParameterSymbol* thisParam3 = new ParameterSymbol(Span(), thisParamName);
    thisParam3->SetAssembly(&assembly);
    thisParam3->SetType(refTypeSymbol);
    ParameterSymbol* thatParam3 = new ParameterSymbol(Span(), thatParamName);
    thatParam3->SetAssembly(&assembly);
    thatParam3->SetType(refTypeSymbol->GetBaseType());
    assignmentFromBaseType->AddSymbol(std::unique_ptr<Symbol>(thisParam3));
    assignmentFromBaseType->AddSymbol(std::unique_ptr<Symbol>(thatParam3));
    assignmentFromBaseType->ComputeName();
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(assignmentFromBaseType));

    RefTypeAssignment* assignmentToBaseType = new RefTypeAssignment(Span(), constantPool.GetEmptyStringConstant());
    assignmentToBaseType->SetAssembly(&assembly);
    assignmentToBaseType->SetType(refTypeSymbol->GetBaseType());
    ParameterSymbol* thisParam4 = new ParameterSymbol(Span(), thisParamName);
    thisParam4->SetAssembly(&assembly);
    thisParam4->SetType(refTypeSymbol->GetBaseType());
    ParameterSymbol* thatParam4 = new ParameterSymbol(Span(), thatParamName);
    thatParam4->SetAssembly(&assembly);
    thatParam4->SetType(refTypeSymbol);
    assignmentToBaseType->AddSymbol(std::unique_ptr<Symbol>(thisParam4));
    assignmentToBaseType->AddSymbol(std::unique_ptr<Symbol>(thatParam4));
    assignmentToBaseType->ComputeName();
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(assignmentToBaseType));

    RefTypeInit* initToBaseType = new RefTypeInit(Span(), constantPool.GetEmptyStringConstant());
    initToBaseType->SetAssembly(&assembly);
    initToBaseType->SetType(refTypeSymbol->GetBaseType());
    ParameterSymbol* thisParam5 = new ParameterSymbol(Span(), thisParamName);
    thisParam5->SetAssembly(&assembly);
    thisParam5->SetType(refTypeSymbol->GetBaseType());
    ParameterSymbol* thatParam5 = new ParameterSymbol(Span(), thatParamName);
    thatParam5->SetAssembly(&assembly);
    thatParam5->SetType(refTypeSymbol);
    initToBaseType->AddSymbol(std::unique_ptr<Symbol>(thisParam5));
    initToBaseType->AddSymbol(std::unique_ptr<Symbol>(thatParam5));
    initToBaseType->ComputeName();
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(initToBaseType));

    assembly.GetSymbolTable().EndNamespace();
}

} } // namespace cminor::symbols
