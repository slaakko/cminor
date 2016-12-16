// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/DelegateSymbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/machine/Machine.hpp>

namespace cminor { namespace symbols {

DelegateTypeSymbol::DelegateTypeSymbol(const Span& span_, Constant name_) : TypeSymbol(span_, name_), returnType(nullptr)
{
    machineType.reset(new DelegateType());
}

void DelegateTypeSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        throw Exception("delegate type cannot be static", GetSpan());
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception("delegate type cannot be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception("delegate type cannot be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        throw Exception("delegate type cannot be abstract", GetSpan());
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        throw Exception("delegate type cannot be inline", GetSpan());
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception("delegate type cannot be external", GetSpan());
    }
}

void DelegateTypeSymbol::AddSymbol(std::unique_ptr<Symbol>&& symbol)
{
    Symbol* s = symbol.get();
    TypeSymbol::AddSymbol(std::move(symbol));
    if (ParameterSymbol* parameterSymbol = dynamic_cast<ParameterSymbol*>(s))
    {
        parameterSymbol->SetIndex(int32_t(parameters.size()));
        parameters.push_back(parameterSymbol);
    }
}

void DelegateTypeSymbol::Write(SymbolWriter& writer)
{
    TypeSymbol::Write(writer);
    utf32_string returnTypeFullName = returnType->FullName();
    ConstantId returnTypeNameId = GetAssembly()->GetConstantPool().GetIdFor(returnTypeFullName);
    Assert(returnTypeNameId != noConstantId, "got no id for return type");
    returnTypeNameId.Write(writer);
}

void DelegateTypeSymbol::Read(SymbolReader& reader)
{
    TypeSymbol::Read(reader);
    ConstantId returnTypeNameId;
    returnTypeNameId.Read(reader);
    reader.EmplaceTypeRequest(this, returnTypeNameId, 0);
}

void DelegateTypeSymbol::EmplaceType(TypeSymbol* type, int index)
{
    if (index == 0)
    {
        returnType = type;
    }
    else
    {
        throw std::runtime_error("delegate type symbol emplace type got invalid type index " + std::to_string(index));
    }
}

void DelegateTypeSymbol::DumpHeader(CodeFormatter& formatter)
{
    std::string returnTypeStr;
    if (returnType)
    {
        returnTypeStr = ": " + ToUtf8(returnType->FullName());
    }
    formatter.WriteLine(TypeString() + " " + ToUtf8(Name().Value()) + returnTypeStr);
}

DelegateDefaultInit::DelegateDefaultInit(const Span& span_, Constant name_) : BasicTypeInit(span_, name_)
{
}

void DelegateDefaultInit::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    std::unique_ptr<Instruction> inst = machine.CreateInst("def", "@delegate");
    function.AddInst(std::move(inst));
    GenObject* target = objects[0];
    target->GenStore(machine, function);
}

void CreateDelegateFun(Assembly& assembly, TypeSymbol* type)
{
    assembly.GetSymbolTable().BeginNamespace(StringPtr(U"System"), Span());

    ConstantPool& constantPool = assembly.GetConstantPool();
    Constant thisParamName = constantPool.GetConstant(constantPool.Install(U"this"));
    Constant thatParamName = constantPool.GetConstant(constantPool.Install(U"that"));
    ParameterSymbol* thisParam1 = new ParameterSymbol(Span(), thisParamName);
    thisParam1->SetAssembly(&assembly);
    thisParam1->SetType(type);
    DelegateDefaultInit* defaultInit = new DelegateDefaultInit(Span(), constantPool.GetEmptyStringConstant());
    defaultInit->SetAssembly(&assembly);
    defaultInit->SetType(type);
    defaultInit->AddSymbol(std::unique_ptr<Symbol>(thisParam1));
    defaultInit->ComputeName();
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(defaultInit));

    CreateBasicTypeBasicFun(assembly, type, true);

    assembly.GetSymbolTable().EndNamespace();
}

ClassDelegateTypeSymbol::ClassDelegateTypeSymbol(const Span& span_, Constant name_) : ClassTypeSymbol(span_, name_), returnType(nullptr)
{
}

void ClassDelegateTypeSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        throw Exception("class delegate type cannot be static", GetSpan());
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception("class delegate type cannot be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception("class delegate type cannot be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        throw Exception("class delegate type cannot be abstract", GetSpan());
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        throw Exception("class delegate type cannot be inline", GetSpan());
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception("class delegate type cannot be external", GetSpan());
    }
}

void ClassDelegateTypeSymbol::AddSymbol(std::unique_ptr<Symbol>&& symbol)
{
    Symbol* s = symbol.get();
    ClassTypeSymbol::AddSymbol(std::move(symbol));
    if (ParameterSymbol* parameterSymbol = dynamic_cast<ParameterSymbol*>(s))
    {
        parameterSymbol->SetIndex(int32_t(parameters.size()));
        parameters.push_back(parameterSymbol);
    }
}

void ClassDelegateTypeSymbol::Write(SymbolWriter& writer)
{
    ClassTypeSymbol::Write(writer);
    utf32_string returnTypeFullName = returnType->FullName();
    ConstantId returnTypeNameId = GetAssembly()->GetConstantPool().GetIdFor(returnTypeFullName);
    Assert(returnTypeNameId != noConstantId, "got no id for return type");
    returnTypeNameId.Write(writer);
}

void ClassDelegateTypeSymbol::Read(SymbolReader& reader)
{
    ClassTypeSymbol::Read(reader);
    ConstantId returnTypeNameId;
    returnTypeNameId.Read(reader);
    reader.EmplaceTypeRequest(this, returnTypeNameId, 0);
}

void ClassDelegateTypeSymbol::EmplaceType(TypeSymbol* type, int index)
{
    if (index == 0)
    {
        returnType = type;
    }
    else
    {
        throw std::runtime_error("class delegate type symbol emplace type got invalid type index " + std::to_string(index));
    }
}

void ClassDelegateTypeSymbol::DumpHeader(CodeFormatter& formatter)
{
    std::string returnTypeStr;
    if (returnType)
    {
        returnTypeStr = ": " + ToUtf8(returnType->FullName());
    }
    formatter.WriteLine(TypeString() + " " + ToUtf8(Name().Value()) + returnTypeStr);
}

} } // namespace cminor::symbols
