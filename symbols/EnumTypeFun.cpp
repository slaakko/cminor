// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/EnumTypeFun.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/machine/Machine.hpp>

namespace cminor { namespace symbols {

EnumTypeDefaultInit::EnumTypeDefaultInit(const Span& span_, Constant name_) : BasicTypeInit(span_, name_)
{
}

void EnumTypeDefaultInit::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    TypeSymbol* type = GetType();
    EnumTypeSymbol* enumType = dynamic_cast<EnumTypeSymbol*>(type);
    Assert(enumType, "enum type expected");
    TypeSymbol* underlyingType = enumType->GetUnderlyingType();
    std::string typeName = ToUtf8(underlyingType->FullName());
    std::unique_ptr<Instruction> inst = machine.CreateInst("def", typeName);
    function.AddInst(std::move(inst));
    GenObject* target = objects[0];
    target->GenStore(machine, function);
}

EnumTypeConversionFun::EnumTypeConversionFun(const Span& span_, Constant name_) : 
    BasicTypeFun(span_, name_), conversionType(ConversionType::implicit_), conversionDistance(0), sourceType(nullptr), targetType(nullptr)
{
    SetConversionFun();
}

void EnumTypeConversionFun::Write(SymbolWriter& writer)
{
    BasicTypeFun::Write(writer);
    writer.AsMachineWriter().Put(uint8_t(conversionType));
    writer.AsMachineWriter().PutEncodedUInt(conversionDistance);
    utf32_string sourceTypeFullName = sourceType->FullName();
    ConstantId sourceTypeId = GetAssembly()->GetConstantPool().GetIdFor(sourceTypeFullName);
    Assert(sourceTypeId != noConstantId, "got no id");
    sourceTypeId.Write(writer);
    utf32_string targetTypeFullName = targetType->FullName();
    ConstantId targetTypeId = GetAssembly()->GetConstantPool().GetIdFor(targetTypeFullName);
    Assert(targetTypeId != noConstantId, "got no id");
    targetTypeId.Write(writer);

}
void EnumTypeConversionFun::Read(SymbolReader& reader)
{
    BasicTypeFun::Read(reader);
    conversionType = ConversionType(reader.GetByte());
    conversionDistance = reader.GetEncodedUInt();
    ConstantId sourceTypeId;
    sourceTypeId.Read(reader);
    reader.EmplaceTypeRequest(this, sourceTypeId, 2);
    ConstantId targetTypeId;
    targetTypeId.Read(reader);
    reader.EmplaceTypeRequest(this, targetTypeId, 3);
}

void EnumTypeConversionFun::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
}

void EnumTypeConversionFun::EmplaceType(TypeSymbol* type, int index)
{
    if (index == 2)
    {
        sourceType = type;
    }
    else if (index == 3)
    {
        targetType = type;
    }
    else
    {
        BasicTypeFun::EmplaceType(type, index);
    }
}

void CreateEnumFun(Assembly& assembly, EnumTypeSymbol* enumType)
{
    ConstantPool& constantPool = assembly.GetConstantPool();
    utf32_string enumNs = enumType->Ns()->FullName();
    assembly.GetSymbolTable().BeginNamespace(StringPtr(enumNs.c_str()), Span());
    
    CreateBasicTypeBasicFun(assembly, enumType, true);

    Constant thisParamName = constantPool.GetConstant(constantPool.Install(U"this"));
    ParameterSymbol* thisParam1 = new ParameterSymbol(Span(), thisParamName);
    thisParam1->SetAssembly(&assembly);
    thisParam1->SetType(enumType);
    EnumTypeDefaultInit* defaultInit = new EnumTypeDefaultInit(Span(), constantPool.GetEmptyStringConstant());
    defaultInit->SetAssembly(&assembly);
    defaultInit->SetType(enumType);
    defaultInit->AddSymbol(std::unique_ptr<Symbol>(thisParam1));
    defaultInit->ComputeName();
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(defaultInit));

    Constant groupName = constantPool.GetConstant(constantPool.Install(StringPtr(U"@conversion")));
    TypeSymbol* underlyingType = enumType->GetUnderlyingType();

    EnumTypeConversionFun* enum2underlyingType = new EnumTypeConversionFun(Span(), constantPool.GetConstant(constantPool.Install(U"enum2underlyingType")));
    enum2underlyingType->SetAssembly(&assembly);
    enum2underlyingType->SetGroupNameConstant(groupName);
    enum2underlyingType->SetConversionType(ConversionType::implicit_);
    enum2underlyingType->SetConversionDistance(1);
    enum2underlyingType->SetSourceType(enumType);
    enum2underlyingType->SetTargetType(underlyingType);
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(enum2underlyingType));

    EnumTypeConversionFun* underlyingType2enum = new EnumTypeConversionFun(Span(), constantPool.GetConstant(constantPool.Install(U"underlyingType2enum")));
    underlyingType2enum->SetAssembly(&assembly);
    underlyingType2enum->SetGroupNameConstant(groupName);
    underlyingType2enum->SetConversionType(ConversionType::explicit_);
    underlyingType2enum->SetConversionDistance(999);
    underlyingType2enum->SetSourceType(underlyingType);
    underlyingType2enum->SetTargetType(enumType);
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(underlyingType2enum));

    assembly.GetSymbolTable().EndNamespace();
}

} } // namespace cminor::symbols
