// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/RefTypeFun.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/VariableSymbol.hpp>

namespace cminor { namespace symbols {

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
    RefTypeAssignment* assignment = new RefTypeAssignment(Span(), constantPool.GetEmptyStringConstant());
    assignment->SetAssembly(&assembly);
    assignment->SetType(refTypeSymbol);
    ParameterSymbol* thisParam3 = new ParameterSymbol(Span(), thisParamName);
    thisParam3->SetAssembly(&assembly);
    thisParam3->SetType(refTypeSymbol);
    ParameterSymbol* thatParam3 = new ParameterSymbol(Span(), thatParamName);
    thatParam3->SetAssembly(&assembly);
    thatParam3->SetType(refTypeSymbol->GetBaseType());
    assignment->AddSymbol(std::unique_ptr<Symbol>(thisParam3));
    assignment->AddSymbol(std::unique_ptr<Symbol>(thatParam3));
    assignment->ComputeName();
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(assignment));

    assembly.GetSymbolTable().EndNamespace();
}

} } // namespace cminor::symbols
