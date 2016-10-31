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
#include <cminor/machine/Function.hpp>

namespace cminor { namespace symbols {

BasicTypeFun::BasicTypeFun(const Span& span_, Constant name_) : FunctionSymbol(span_, name_), type(nullptr)
{
}

void BasicTypeFun::Write(SymbolWriter& writer)
{
    FunctionSymbol::Write(writer);
    utf32_string typeFullName = type->FullName();
    ConstantId typeNameId = GetAssembly()->GetConstantPool().GetIdFor(typeFullName);
    Assert(typeNameId != noConstantId, "got no id");
    typeNameId.Write(writer);
}

void BasicTypeFun::Read(SymbolReader& reader)
{
    FunctionSymbol::Read(reader);
    ConstantId typeNameId;
    typeNameId.Read(reader);
    reader.EmplaceTypeRequest(this, typeNameId, 1);
}

void BasicTypeFun::EmplaceType(TypeSymbol* type, int index)
{
    if (index == 1)
    {
        this->type = type;
    }
    else
    {
        FunctionSymbol::EmplaceType(type, index);
    }
}

BasicTypeInit::BasicTypeInit(const Span& span_, Constant name_) : BasicTypeFun(span_, name_)
{
}

void BasicTypeInit::ComputeName()
{
    ConstantPool& constantPool = GetAssembly()->GetConstantPool();
    Constant groupName = constantPool.GetConstant(constantPool.Install(StringPtr(U"@init")));
    SetGroupNameConstant(groupName);
    BasicTypeFun::ComputeName();
}

BasicTypeDefaultInit::BasicTypeDefaultInit(const Span& span_, Constant name_) : BasicTypeInit(span_, name_)
{
}

void BasicTypeDefaultInit::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    std::string typeName = ToUtf8(GetType()->FullName());
    std::unique_ptr<Instruction> inst = machine.CreateInst("def", typeName);
    function.AddInst(std::move(inst));
    GenObject* target = objects[0];
    target->GenStore(machine, function);
}

BasicTypeCopyInit::BasicTypeCopyInit(const Span& span_, Constant name_) : BasicTypeInit(span_, name_)
{
}

void BasicTypeCopyInit::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    Assert(objects.size() == 2, "copy init needs two objects");
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
    Constant groupName = constantPool.GetConstant(constantPool.Install(StringPtr(U"@assignment")));
    SetGroupNameConstant(groupName);
    BasicTypeFun::ComputeName();
}

void BasicTypeAssignment::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    Assert(objects.size() == 2, "assignment needs two objects");
    GenObject* source = objects[1];
    source->GenLoad(machine, function);
    GenObject* target = objects[0];
    target->GenStore(machine, function);
}

BasicTypeReturn::BasicTypeReturn(const Span& span_, Constant name_) : BasicTypeFun(span_, name_)
{
}

void BasicTypeReturn::ComputeName()
{
    ConstantPool& constantPool = GetAssembly()->GetConstantPool();
    Constant groupName = constantPool.GetConstant(constantPool.Install(StringPtr(U"@return")));
    SetGroupNameConstant(groupName);
    BasicTypeFun::ComputeName();
}

void BasicTypeReturn::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    Assert(objects.size() == 1, "return needs one object");
    GenObject* value = objects[0];
    value->GenLoad(machine, function);
}

BasicTypeConversion::BasicTypeConversion(const Span& span_, Constant name_) : 
    BasicTypeFun(span_, name_), conversionType(ConversionType::implicit_), conversionDistance(0), sourceType(nullptr), targetType(nullptr), conversionInstructionName()
{
    SetConversionFun();
}

void BasicTypeConversion::Write(SymbolWriter& writer)
{
    BasicTypeFun::Write(writer);
    writer.AsMachineWriter().Put(uint8_t(conversionType));
    writer.AsMachineWriter().Put(conversionDistance);
    utf32_string sourceTypeFullName = sourceType->FullName();
    ConstantId sourceTypeId = GetAssembly()->GetConstantPool().GetIdFor(sourceTypeFullName);
    Assert(sourceTypeId != noConstantId, "got no id");
    sourceTypeId.Write(writer);
    utf32_string targetTypeFullName = targetType->FullName();
    ConstantId targetTypeId = GetAssembly()->GetConstantPool().GetIdFor(targetTypeFullName);
    Assert(targetTypeId != noConstantId, "got no id");
    targetTypeId.Write(writer);
    writer.AsMachineWriter().Put(conversionInstructionName);
}

void BasicTypeConversion::Read(SymbolReader& reader)
{
    BasicTypeFun::Read(reader);
    conversionType = ConversionType(reader.GetByte());
    conversionDistance = reader.GetInt();
    ConstantId sourceTypeId;
    sourceTypeId.Read(reader);
    reader.EmplaceTypeRequest(this, sourceTypeId, 2);
    ConstantId targetTypeId;
    targetTypeId.Read(reader);
    reader.EmplaceTypeRequest(this, targetTypeId, 3);
    conversionInstructionName = reader.GetUtf8String();
}

void BasicTypeConversion::EmplaceType(TypeSymbol* type, int index)
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

void BasicTypeConversion::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    std::unique_ptr<Instruction> inst =  machine.CreateInst(conversionInstructionName);
    function.AddInst(std::move(inst));
}

BasicTypeUnaryOpFun::BasicTypeUnaryOpFun(const Span& span_, Constant name_) : BasicTypeFun(span_, name_), instGroupName(), typeName()
{
}

void BasicTypeUnaryOpFun::Write(SymbolWriter& writer)
{
    BasicTypeFun::Write(writer);
    writer.AsMachineWriter().Put(instGroupName);
    writer.AsMachineWriter().Put(typeName);
}

void BasicTypeUnaryOpFun::Read(SymbolReader& reader)
{
    BasicTypeFun::Read(reader);
    instGroupName = reader.GetUtf8String();
    typeName = reader.GetUtf8String();
}

void BasicTypeUnaryOpFun::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    Assert(objects.size() == 1, "unary operator function needs one object");
    GenObject* operand = objects[0];
    operand->GenLoad(machine, function);
    std::unique_ptr<Instruction> inst = machine.CreateInst(instGroupName, typeName);
    function.AddInst(std::move(inst));
}

BasicTypeBinaryOpFun::BasicTypeBinaryOpFun(const Span& span_, Constant name_) : BasicTypeFun(span_, name_), instGroupName(), typeName()
{
}

void BasicTypeBinaryOpFun::Write(SymbolWriter& writer)
{
    BasicTypeFun::Write(writer);
    writer.AsMachineWriter().Put(instGroupName);
    writer.AsMachineWriter().Put(typeName);
}

void BasicTypeBinaryOpFun::Read(SymbolReader& reader)
{
    BasicTypeFun::Read(reader);
    instGroupName = reader.GetUtf8String();
    typeName = reader.GetUtf8String();
}

void BasicTypeBinaryOpFun::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    Assert(objects.size() == 2, "binary operator function needs two objects");
    GenObject* left = objects[0];
    left->GenLoad(machine, function);
    GenObject* right = objects[1];
    right->GenLoad(machine, function);
    std::unique_ptr<Instruction> inst = machine.CreateInst(instGroupName, typeName);
    function.AddInst(std::move(inst));
}

void CreateBasicTypeUnaryOpFun(Assembly& assembly, TypeSymbol* type, const utf32_string& groupName, const std::string& instGroupName)
{
    std::string typeName = ToUtf8(type->FullName());
    ConstantPool& constantPool = assembly.GetConstantPool();
    Constant operandName = constantPool.GetConstant(constantPool.Install(U"operand"));
    Constant groupNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(groupName.c_str())));
    ParameterSymbol* operandParam = new ParameterSymbol(Span(), operandName);
    operandParam->SetAssembly(&assembly);
    operandParam->SetType(type);
    BasicTypeUnaryOpFun* fun = new BasicTypeUnaryOpFun(Span(), constantPool.GetEmptyStringConstant());
    fun->SetInstGroupName(instGroupName);
    fun->SetTypeName(typeName);
    fun->SetGroupNameConstant(groupNameConstant);
    fun->SetAssembly(&assembly);
    fun->SetType(type);
    fun->AddSymbol(std::unique_ptr<Symbol>(operandParam));
    fun->SetReturnType(type);
    fun->ComputeName();
    assembly.GetSymbolTable().BeginNamespace(StringPtr(U"System"), Span());
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(fun));
    assembly.GetSymbolTable().EndNamespace();
}

void CreateBasicTypeBinaryOpFun(Assembly& assembly, TypeSymbol* type, const utf32_string& groupName, const std::string& instGroupName)
{
    std::string typeName = ToUtf8(type->FullName());
    ConstantPool& constantPool = assembly.GetConstantPool();
    Constant leftParamName = constantPool.GetConstant(constantPool.Install(U"left"));
    ParameterSymbol* leftParam = new ParameterSymbol(Span(), leftParamName);
    leftParam->SetAssembly(&assembly);
    leftParam->SetType(type);
    Constant rightParamName = constantPool.GetConstant(constantPool.Install(U"right"));
    ParameterSymbol* rightParam = new ParameterSymbol(Span(), rightParamName);
    rightParam->SetAssembly(&assembly);
    rightParam->SetType(type);
    Constant groupNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(groupName.c_str())));
    BasicTypeBinaryOpFun* fun = new BasicTypeBinaryOpFun(Span(), constantPool.GetEmptyStringConstant());
    fun->SetGroupNameConstant(groupNameConstant);
    fun->SetInstGroupName(instGroupName);
    fun->SetTypeName(typeName);
    fun->SetAssembly(&assembly);
    fun->SetType(type);
    fun->AddSymbol(std::unique_ptr<Symbol>(leftParam));
    fun->AddSymbol(std::unique_ptr<Symbol>(rightParam));
    fun->SetReturnType(type);
    fun->ComputeName();
    assembly.GetSymbolTable().BeginNamespace(StringPtr(U"System"), Span());
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(fun));
    assembly.GetSymbolTable().EndNamespace();
}

void CreateBasicTypeComparisonFun(Assembly& assembly, TypeSymbol* type, TypeSymbol* boolType, const utf32_string& groupName, const std::string& instGroupName)
{
    std::string typeName = ToUtf8(type->FullName());
    ConstantPool& constantPool = assembly.GetConstantPool();
    Constant leftParamName = constantPool.GetConstant(constantPool.Install(U"left"));
    ParameterSymbol* leftParam = new ParameterSymbol(Span(), leftParamName);
    leftParam->SetAssembly(&assembly);
    leftParam->SetType(type);
    Constant rightParamName = constantPool.GetConstant(constantPool.Install(U"right"));
    ParameterSymbol* rightParam = new ParameterSymbol(Span(), rightParamName);
    rightParam->SetAssembly(&assembly);
    rightParam->SetType(type);
    Constant groupNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(groupName.c_str())));
    BasicTypeBinaryOpFun* fun = new BasicTypeBinaryOpFun(Span(), constantPool.GetEmptyStringConstant());
    fun->SetGroupNameConstant(groupNameConstant);
    fun->SetInstGroupName(instGroupName);
    fun->SetTypeName(typeName);
    fun->SetAssembly(&assembly);
    fun->SetType(type);
    fun->AddSymbol(std::unique_ptr<Symbol>(leftParam));
    fun->AddSymbol(std::unique_ptr<Symbol>(rightParam));
    fun->SetReturnType(boolType);
    fun->ComputeName();
    assembly.GetSymbolTable().BeginNamespace(StringPtr(U"System"), Span());
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(fun));
    assembly.GetSymbolTable().EndNamespace();
}

void CreateBasicTypeBasicFun(Assembly& assembly, TypeSymbol* type)
{
    assembly.GetSymbolTable().BeginNamespace(StringPtr(U"System"), Span());
    ConstantPool& constantPool = assembly.GetConstantPool();
    Constant thisParamName = constantPool.GetConstant(constantPool.Install(U"this"));
    Constant thatParamName = constantPool.GetConstant(constantPool.Install(U"that"));
    ParameterSymbol* thisParam1 = new ParameterSymbol(Span(), thisParamName);
    thisParam1->SetAssembly(&assembly);
    thisParam1->SetType(type);
    BasicTypeDefaultInit* defaultInit = new BasicTypeDefaultInit(Span(), constantPool.GetEmptyStringConstant());
    defaultInit->SetAssembly(&assembly);
    defaultInit->SetType(type);
    defaultInit->AddSymbol(std::unique_ptr<Symbol>(thisParam1));
    defaultInit->ComputeName();
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(defaultInit));

    ParameterSymbol* thisParam2 = new ParameterSymbol(Span(), thisParamName);
    thisParam2->SetAssembly(&assembly);
    thisParam2->SetType(type);
    ParameterSymbol* thatParam2 = new ParameterSymbol(Span(), thatParamName);
    thatParam2->SetAssembly(&assembly);
    thatParam2->SetType(type);
    BasicTypeCopyInit* copyInit = new BasicTypeCopyInit(Span(), constantPool.GetEmptyStringConstant());
    copyInit->SetAssembly(&assembly);
    copyInit->SetType(type);
    copyInit->AddSymbol(std::unique_ptr<Symbol>(thisParam2));
    copyInit->AddSymbol(std::unique_ptr<Symbol>(thatParam2));
    copyInit->ComputeName();
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(copyInit));

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
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(assignment));

    BasicTypeReturn* returnFun = new BasicTypeReturn(Span(), constantPool.GetEmptyStringConstant());
    returnFun->SetAssembly(&assembly);
    returnFun->SetType(type);
    returnFun->SetReturnType(type);
    Constant valueParamName = constantPool.GetConstant(constantPool.Install(U"value"));
    ParameterSymbol* valueParam = new ParameterSymbol(Span(), valueParamName);
    valueParam->SetAssembly(&assembly);
    valueParam->SetType(type);
    returnFun->AddSymbol(std::unique_ptr<Symbol>(valueParam));
    returnFun->ComputeName();
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(returnFun));
    assembly.GetSymbolTable().EndNamespace();
}

void CreateBasicTypeBoolFun(Assembly& assembly, TypeSymbol* boolType)
{
    CreateBasicTypeBasicFun(assembly, boolType);
    CreateBasicTypeUnaryOpFun(assembly, boolType, U"operator!", "not");
    CreateBasicTypeComparisonFun(assembly, boolType, boolType, U"operator==", "equal");
}

void CreateBasicTypeCharFun(Assembly& assembly, TypeSymbol* charType, TypeSymbol* boolType)
{
    CreateBasicTypeBasicFun(assembly, charType);
    CreateBasicTypeComparisonFun(assembly, charType, boolType, U"operator==", "equal");
    CreateBasicTypeComparisonFun(assembly, charType, boolType, U"operator<", "less");
}

void CreateBasicTypeIntFun(Assembly& assembly, TypeSymbol* type, TypeSymbol* boolType)
{
    CreateBasicTypeBasicFun(assembly, type);
    CreateBasicTypeUnaryOpFun(assembly, type, U"operator+", "uplus");
    CreateBasicTypeUnaryOpFun(assembly, type, U"operator-", "neg");
    CreateBasicTypeUnaryOpFun(assembly, type, U"operator~", "cpl");
    CreateBasicTypeBinaryOpFun(assembly, type, U"operator+", "add");
    CreateBasicTypeBinaryOpFun(assembly, type, U"operator-", "sub");
    CreateBasicTypeBinaryOpFun(assembly, type, U"operator*", "mul");
    CreateBasicTypeBinaryOpFun(assembly, type, U"operator/", "div");
    CreateBasicTypeBinaryOpFun(assembly, type, U"operator%", "rem");
    CreateBasicTypeBinaryOpFun(assembly, type, U"operator&", "and");
    CreateBasicTypeBinaryOpFun(assembly, type, U"operator|", "or");
    CreateBasicTypeBinaryOpFun(assembly, type, U"operator^", "xor");
    CreateBasicTypeBinaryOpFun(assembly, type, U"operator<<", "shl");
    CreateBasicTypeBinaryOpFun(assembly, type, U"operator>>", "shr");
    CreateBasicTypeComparisonFun(assembly, type, boolType, U"operator==", "equal");
    CreateBasicTypeComparisonFun(assembly, type, boolType, U"operator<", "less");
}

void CreateBasicTypeFloatFun(Assembly& assembly, TypeSymbol* type, TypeSymbol* boolType)
{
    CreateBasicTypeBasicFun(assembly, type);
    CreateBasicTypeUnaryOpFun(assembly, type, U"operator+", "uplus");
    CreateBasicTypeUnaryOpFun(assembly, type, U"operator-", "neg");
    CreateBasicTypeBinaryOpFun(assembly, type, U"operator+", "add");
    CreateBasicTypeBinaryOpFun(assembly, type, U"operator-", "sub");
    CreateBasicTypeBinaryOpFun(assembly, type, U"operator*", "mul");
    CreateBasicTypeBinaryOpFun(assembly, type, U"operator/", "div");
    CreateBasicTypeComparisonFun(assembly, type, boolType, U"operator==", "equal");
    CreateBasicTypeComparisonFun(assembly, type, boolType, U"operator<", "less");
}

void CreateBasicTypeConversions(Assembly& assembly, TypeSymbol* boolType, TypeSymbol* charType, TypeSymbol* sbyteType, TypeSymbol* byteType, TypeSymbol* shortType, TypeSymbol* ushortType,
    TypeSymbol* intType, TypeSymbol* uintType, TypeSymbol* longType, TypeSymbol* ulongType, TypeSymbol* floatType, TypeSymbol* doubleType)
{
    assembly.GetSymbolTable().BeginNamespace(StringPtr(U"System"), Span());
    ConstantPool& constantPool = assembly.GetConstantPool();
    Constant groupName = constantPool.GetConstant(constantPool.Install(StringPtr(U"@conversion")));

    BasicTypeConversion* sbyte2byte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"sbyte2byte")));
    sbyte2byte->SetAssembly(&assembly);
    sbyte2byte->SetGroupNameConstant(groupName);
    sbyte2byte->SetConversionType(ConversionType::explicit_);
    sbyte2byte->SetConversionDistance(999);
    sbyte2byte->SetSourceType(sbyteType);
    sbyte2byte->SetTargetType(byteType);
    sbyte2byte->SetConversionInstructionName("sb2by");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(sbyte2byte));

    BasicTypeConversion* sbyte2short = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"sbyte2short")));
    sbyte2short->SetAssembly(&assembly);
    sbyte2short->SetGroupNameConstant(groupName);
    sbyte2short->SetConversionType(ConversionType::implicit_);
    sbyte2short->SetConversionDistance(1);
    sbyte2short->SetSourceType(sbyteType);
    sbyte2short->SetTargetType(shortType);
    sbyte2short->SetConversionInstructionName("sb2sh");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(sbyte2short));

    BasicTypeConversion* sbyte2ushort = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"sbyte2ushort")));
    sbyte2ushort->SetAssembly(&assembly);
    sbyte2ushort->SetGroupNameConstant(groupName);
    sbyte2ushort->SetConversionType(ConversionType::explicit_);
    sbyte2ushort->SetConversionDistance(999);
    sbyte2ushort->SetSourceType(sbyteType);
    sbyte2ushort->SetTargetType(ushortType);
    sbyte2ushort->SetConversionInstructionName("sb2us");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(sbyte2ushort));

    BasicTypeConversion* sbyte2int = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"sbyte2int")));
    sbyte2int->SetAssembly(&assembly);
    sbyte2int->SetGroupNameConstant(groupName);
    sbyte2int->SetConversionType(ConversionType::implicit_);
    sbyte2int->SetConversionDistance(2);
    sbyte2int->SetSourceType(sbyteType);
    sbyte2int->SetTargetType(intType);
    sbyte2int->SetConversionInstructionName("sb2in");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(sbyte2int));

    BasicTypeConversion* sbyte2uint = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"sbyte2uint")));
    sbyte2uint->SetAssembly(&assembly);
    sbyte2uint->SetGroupNameConstant(groupName);
    sbyte2uint->SetConversionType(ConversionType::explicit_);
    sbyte2uint->SetConversionDistance(999);
    sbyte2uint->SetSourceType(sbyteType);
    sbyte2uint->SetTargetType(uintType);
    sbyte2uint->SetConversionInstructionName("sb2ui");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(sbyte2uint));

    BasicTypeConversion* sbyte2long = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"sbyte2long")));
    sbyte2long->SetAssembly(&assembly);
    sbyte2long->SetGroupNameConstant(groupName);
    sbyte2long->SetConversionType(ConversionType::implicit_);
    sbyte2long->SetConversionDistance(3);
    sbyte2long->SetSourceType(sbyteType);
    sbyte2long->SetTargetType(longType);
    sbyte2long->SetConversionInstructionName("sb2lo");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(sbyte2long));

    BasicTypeConversion* sbyte2ulong = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"sbyte2ulong")));
    sbyte2ulong->SetAssembly(&assembly);
    sbyte2ulong->SetGroupNameConstant(groupName);
    sbyte2ulong->SetConversionType(ConversionType::explicit_);
    sbyte2ulong->SetConversionDistance(999);
    sbyte2ulong->SetSourceType(sbyteType);
    sbyte2ulong->SetTargetType(ulongType);
    sbyte2ulong->SetConversionInstructionName("sb2ul");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(sbyte2ulong));

    BasicTypeConversion* sbyte2float = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"sbyte2float")));
    sbyte2float->SetAssembly(&assembly);
    sbyte2float->SetGroupNameConstant(groupName);
    sbyte2float->SetConversionType(ConversionType::implicit_);
    sbyte2float->SetConversionDistance(4);
    sbyte2float->SetSourceType(sbyteType);
    sbyte2float->SetTargetType(floatType);
    sbyte2float->SetConversionInstructionName("sb2fl");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(sbyte2float));

    BasicTypeConversion* sbyte2double = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"sbyte2double")));
    sbyte2double->SetAssembly(&assembly);
    sbyte2double->SetGroupNameConstant(groupName);
    sbyte2double->SetConversionType(ConversionType::implicit_);
    sbyte2double->SetConversionDistance(5);
    sbyte2double->SetSourceType(sbyteType);
    sbyte2double->SetTargetType(doubleType);
    sbyte2double->SetConversionInstructionName("sb2do");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(sbyte2double));

    BasicTypeConversion* sbyte2char = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"sbyte2char")));
    sbyte2char->SetAssembly(&assembly);
    sbyte2char->SetGroupNameConstant(groupName);
    sbyte2char->SetConversionType(ConversionType::explicit_);
    sbyte2char->SetConversionDistance(999);
    sbyte2char->SetSourceType(sbyteType);
    sbyte2char->SetTargetType(charType);
    sbyte2char->SetConversionInstructionName("sb2ch");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(sbyte2char));

    BasicTypeConversion* sbyte2bool = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"sbyte2bool")));
    sbyte2bool->SetAssembly(&assembly);
    sbyte2bool->SetGroupNameConstant(groupName);
    sbyte2bool->SetConversionType(ConversionType::explicit_);
    sbyte2bool->SetConversionDistance(999);
    sbyte2bool->SetSourceType(sbyteType);
    sbyte2bool->SetTargetType(boolType);
    sbyte2bool->SetConversionInstructionName("sb2bo");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(sbyte2bool));

    BasicTypeConversion* byte2sbyte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"byte2sbyte")));
    byte2sbyte->SetAssembly(&assembly);
    byte2sbyte->SetGroupNameConstant(groupName);
    byte2sbyte->SetConversionType(ConversionType::explicit_);
    byte2sbyte->SetConversionDistance(999);
    byte2sbyte->SetSourceType(byteType);
    byte2sbyte->SetTargetType(sbyteType);
    byte2sbyte->SetConversionInstructionName("by2sb");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(byte2sbyte));

    BasicTypeConversion* byte2short = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"byte2short")));
    byte2short->SetAssembly(&assembly);
    byte2short->SetGroupNameConstant(groupName);
    byte2short->SetConversionType(ConversionType::implicit_);
    byte2short->SetConversionDistance(1);
    byte2short->SetSourceType(byteType);
    byte2short->SetTargetType(shortType);
    byte2short->SetConversionInstructionName("by2sh");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(byte2short));

    BasicTypeConversion* byte2ushort = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"byte2ushort")));
    byte2ushort->SetAssembly(&assembly);
    byte2ushort->SetGroupNameConstant(groupName);
    byte2ushort->SetConversionType(ConversionType::implicit_);
    byte2ushort->SetConversionDistance(2);
    byte2ushort->SetSourceType(byteType);
    byte2ushort->SetTargetType(ushortType);
    byte2ushort->SetConversionInstructionName("by2us");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(byte2ushort));

    BasicTypeConversion* byte2int = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"byte2int")));
    byte2int->SetAssembly(&assembly);
    byte2int->SetGroupNameConstant(groupName);
    byte2int->SetConversionType(ConversionType::implicit_);
    byte2int->SetConversionDistance(3);
    byte2int->SetSourceType(byteType);
    byte2int->SetTargetType(intType);
    byte2int->SetConversionInstructionName("by2in");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(byte2int));

    BasicTypeConversion* byte2uint = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"byte2uint")));
    byte2uint->SetAssembly(&assembly);
    byte2uint->SetGroupNameConstant(groupName);
    byte2uint->SetConversionType(ConversionType::implicit_);
    byte2uint->SetConversionDistance(4);
    byte2uint->SetSourceType(byteType);
    byte2uint->SetTargetType(uintType);
    byte2uint->SetConversionInstructionName("by2ui");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(byte2uint));

    BasicTypeConversion* byte2long = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"byte2long")));
    byte2long->SetAssembly(&assembly);
    byte2long->SetGroupNameConstant(groupName);
    byte2long->SetConversionType(ConversionType::implicit_);
    byte2long->SetConversionDistance(5);
    byte2long->SetSourceType(byteType);
    byte2long->SetTargetType(longType);
    byte2long->SetConversionInstructionName("by2lo");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(byte2long));

    BasicTypeConversion* byte2ulong = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"byte2ulong")));
    byte2ulong->SetAssembly(&assembly);
    byte2ulong->SetGroupNameConstant(groupName);
    byte2ulong->SetConversionType(ConversionType::implicit_);
    byte2ulong->SetConversionDistance(6);
    byte2ulong->SetSourceType(byteType);
    byte2ulong->SetTargetType(ulongType);
    byte2ulong->SetConversionInstructionName("by2ul");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(byte2ulong));

    BasicTypeConversion* byte2float = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"byte2float")));
    byte2float->SetAssembly(&assembly);
    byte2float->SetGroupNameConstant(groupName);
    byte2float->SetConversionType(ConversionType::implicit_);
    byte2float->SetConversionDistance(7);
    byte2float->SetSourceType(byteType);
    byte2float->SetTargetType(floatType);
    byte2float->SetConversionInstructionName("by2fl");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(byte2float));

    BasicTypeConversion* byte2double = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"byte2double")));
    byte2double->SetAssembly(&assembly);
    byte2double->SetGroupNameConstant(groupName);
    byte2double->SetConversionType(ConversionType::implicit_);
    byte2double->SetConversionDistance(8);
    byte2double->SetSourceType(byteType);
    byte2double->SetTargetType(doubleType);
    byte2double->SetConversionInstructionName("by2do");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(byte2double));

    BasicTypeConversion* byte2char = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"byte2char")));
    byte2char->SetAssembly(&assembly);
    byte2char->SetGroupNameConstant(groupName);
    byte2char->SetConversionType(ConversionType::explicit_);
    byte2char->SetConversionDistance(999);
    byte2char->SetSourceType(byteType);
    byte2char->SetTargetType(charType);
    byte2char->SetConversionInstructionName("by2ch");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(byte2char));

    BasicTypeConversion* byte2bool = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"byte2bool")));
    byte2bool->SetAssembly(&assembly);
    byte2bool->SetGroupNameConstant(groupName);
    byte2bool->SetConversionType(ConversionType::explicit_);
    byte2bool->SetConversionDistance(999);
    byte2bool->SetSourceType(byteType);
    byte2bool->SetTargetType(boolType);
    byte2bool->SetConversionInstructionName("by2bo");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(byte2bool));

    BasicTypeConversion* short2sbyte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"short2sbyte")));
    short2sbyte->SetAssembly(&assembly);
    short2sbyte->SetGroupNameConstant(groupName);
    short2sbyte->SetConversionType(ConversionType::explicit_);
    short2sbyte->SetConversionDistance(999);
    short2sbyte->SetSourceType(shortType);
    short2sbyte->SetTargetType(sbyteType);
    short2sbyte->SetConversionInstructionName("sh2sb");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(short2sbyte));

    BasicTypeConversion* short2byte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"short2byte")));
    short2byte->SetAssembly(&assembly);
    short2byte->SetGroupNameConstant(groupName);
    short2byte->SetConversionType(ConversionType::explicit_);
    short2byte->SetConversionDistance(999);
    short2byte->SetSourceType(shortType);
    short2byte->SetTargetType(byteType);
    short2byte->SetConversionInstructionName("sh2by");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(short2byte));

    BasicTypeConversion* short2ushort = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"short2ushort")));
    short2ushort->SetAssembly(&assembly);
    short2ushort->SetGroupNameConstant(groupName);
    short2ushort->SetConversionType(ConversionType::explicit_);
    short2ushort->SetConversionDistance(999);
    short2ushort->SetSourceType(shortType);
    short2ushort->SetTargetType(ushortType);
    short2ushort->SetConversionInstructionName("sh2us");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(short2ushort));

    BasicTypeConversion* short2int = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"short2int")));
    short2int->SetAssembly(&assembly);
    short2int->SetGroupNameConstant(groupName);
    short2int->SetConversionType(ConversionType::implicit_);
    short2int->SetConversionDistance(1);
    short2int->SetSourceType(shortType);
    short2int->SetTargetType(intType);
    short2int->SetConversionInstructionName("sh2in");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(short2int));

    BasicTypeConversion* short2uint = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"short2uint")));
    short2uint->SetAssembly(&assembly);
    short2uint->SetGroupNameConstant(groupName);
    short2uint->SetConversionType(ConversionType::explicit_);
    short2uint->SetConversionDistance(999);
    short2uint->SetSourceType(shortType);
    short2uint->SetTargetType(uintType);
    short2uint->SetConversionInstructionName("sh2ui");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(short2uint));

    BasicTypeConversion* short2long = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"short2long")));
    short2long->SetAssembly(&assembly);
    short2long->SetGroupNameConstant(groupName);
    short2long->SetConversionType(ConversionType::implicit_);
    short2long->SetConversionDistance(2);
    short2long->SetSourceType(shortType);
    short2long->SetTargetType(longType);
    short2long->SetConversionInstructionName("sh2lo");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(short2long));

    BasicTypeConversion* short2ulong = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"short2ulong")));
    short2ulong->SetAssembly(&assembly);
    short2ulong->SetGroupNameConstant(groupName);
    short2ulong->SetConversionType(ConversionType::explicit_);
    short2ulong->SetConversionDistance(999);
    short2ulong->SetSourceType(shortType);
    short2ulong->SetTargetType(ulongType);
    short2ulong->SetConversionInstructionName("sh2ul");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(short2ulong));

    BasicTypeConversion* short2float = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"short2float")));
    short2float->SetAssembly(&assembly);
    short2float->SetGroupNameConstant(groupName);
    short2float->SetConversionType(ConversionType::implicit_);
    short2float->SetConversionDistance(3);
    short2float->SetSourceType(shortType);
    short2float->SetTargetType(floatType);
    short2float->SetConversionInstructionName("sh2fl");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(short2float));

    BasicTypeConversion* short2double = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"short2double")));
    short2double->SetAssembly(&assembly);
    short2double->SetGroupNameConstant(groupName);
    short2double->SetConversionType(ConversionType::implicit_);
    short2double->SetConversionDistance(4);
    short2double->SetSourceType(shortType);
    short2double->SetTargetType(doubleType);
    short2double->SetConversionInstructionName("sh2do");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(short2double));

    BasicTypeConversion* short2char = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"short2char")));
    short2char->SetAssembly(&assembly);
    short2char->SetGroupNameConstant(groupName);
    short2char->SetConversionType(ConversionType::explicit_);
    short2char->SetConversionDistance(999);
    short2char->SetSourceType(shortType);
    short2char->SetTargetType(charType);
    short2char->SetConversionInstructionName("sh2ch");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(short2char));

    BasicTypeConversion* short2bool = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"short2bool")));
    short2bool->SetAssembly(&assembly);
    short2bool->SetGroupNameConstant(groupName);
    short2bool->SetConversionType(ConversionType::explicit_);
    short2bool->SetConversionDistance(999);
    short2bool->SetSourceType(shortType);
    short2bool->SetTargetType(boolType);
    short2bool->SetConversionInstructionName("sh2bo");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(short2bool));

    BasicTypeConversion* ushort2sbyte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ushort2sbyte")));
    ushort2sbyte->SetAssembly(&assembly);
    ushort2sbyte->SetGroupNameConstant(groupName);
    ushort2sbyte->SetConversionType(ConversionType::explicit_);
    ushort2sbyte->SetConversionDistance(999);
    ushort2sbyte->SetSourceType(ushortType);
    ushort2sbyte->SetTargetType(sbyteType);
    ushort2sbyte->SetConversionInstructionName("us2sb");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ushort2sbyte));

    BasicTypeConversion* ushort2byte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ushort2byte")));
    ushort2byte->SetAssembly(&assembly);
    ushort2byte->SetGroupNameConstant(groupName);
    ushort2byte->SetConversionType(ConversionType::explicit_);
    ushort2byte->SetConversionDistance(999);
    ushort2byte->SetSourceType(ushortType);
    ushort2byte->SetTargetType(byteType);
    ushort2byte->SetConversionInstructionName("us2by");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ushort2byte));

    BasicTypeConversion* ushort2short = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ushort2short")));
    ushort2short->SetAssembly(&assembly);
    ushort2short->SetGroupNameConstant(groupName);
    ushort2short->SetConversionType(ConversionType::explicit_);
    ushort2short->SetConversionDistance(999);
    ushort2short->SetSourceType(ushortType);
    ushort2short->SetTargetType(shortType);
    ushort2short->SetConversionInstructionName("us2sh");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ushort2short));

    BasicTypeConversion* ushort2int = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ushort2int")));
    ushort2int->SetAssembly(&assembly);
    ushort2int->SetGroupNameConstant(groupName);
    ushort2int->SetConversionType(ConversionType::implicit_);
    ushort2int->SetConversionDistance(1);
    ushort2int->SetSourceType(ushortType);
    ushort2int->SetTargetType(intType);
    ushort2int->SetConversionInstructionName("us2in");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ushort2int));

    BasicTypeConversion* ushort2uint = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ushort2uint")));
    ushort2uint->SetAssembly(&assembly);
    ushort2uint->SetGroupNameConstant(groupName);
    ushort2uint->SetConversionType(ConversionType::implicit_);
    ushort2uint->SetConversionDistance(2);
    ushort2uint->SetSourceType(ushortType);
    ushort2uint->SetTargetType(uintType);
    ushort2uint->SetConversionInstructionName("us2ui");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ushort2uint));

    BasicTypeConversion* ushort2long = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ushort2long")));
    ushort2long->SetAssembly(&assembly);
    ushort2long->SetGroupNameConstant(groupName);
    ushort2long->SetConversionType(ConversionType::implicit_);
    ushort2long->SetConversionDistance(3);
    ushort2long->SetSourceType(ushortType);
    ushort2long->SetTargetType(longType);
    ushort2long->SetConversionInstructionName("us2lo");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ushort2long));

    BasicTypeConversion* ushort2ulong = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ushort2ulong")));
    ushort2ulong->SetAssembly(&assembly);
    ushort2ulong->SetGroupNameConstant(groupName);
    ushort2ulong->SetConversionType(ConversionType::implicit_);
    ushort2ulong->SetConversionDistance(4);
    ushort2ulong->SetSourceType(ushortType);
    ushort2ulong->SetTargetType(ulongType);
    ushort2ulong->SetConversionInstructionName("us2ul");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ushort2ulong));

    BasicTypeConversion* ushort2float = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ushort2float")));
    ushort2float->SetAssembly(&assembly);
    ushort2float->SetGroupNameConstant(groupName);
    ushort2float->SetConversionType(ConversionType::implicit_);
    ushort2float->SetConversionDistance(5);
    ushort2float->SetSourceType(ushortType);
    ushort2float->SetTargetType(floatType);
    ushort2float->SetConversionInstructionName("us2fl");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ushort2float));

    BasicTypeConversion* ushort2double = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ushort2double")));
    ushort2double->SetAssembly(&assembly);
    ushort2double->SetGroupNameConstant(groupName);
    ushort2double->SetConversionType(ConversionType::implicit_);
    ushort2double->SetConversionDistance(6);
    ushort2double->SetSourceType(ushortType);
    ushort2double->SetTargetType(doubleType);
    ushort2double->SetConversionInstructionName("us2do");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ushort2double));

    BasicTypeConversion* ushort2char = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ushort2char")));
    ushort2char->SetAssembly(&assembly);
    ushort2char->SetGroupNameConstant(groupName);
    ushort2char->SetConversionType(ConversionType::explicit_);
    ushort2char->SetConversionDistance(999);
    ushort2char->SetSourceType(ushortType);
    ushort2char->SetTargetType(charType);
    ushort2char->SetConversionInstructionName("us2ch");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ushort2char));

    BasicTypeConversion* ushort2bool = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ushort2bool")));
    ushort2bool->SetAssembly(&assembly);
    ushort2bool->SetGroupNameConstant(groupName);
    ushort2bool->SetConversionType(ConversionType::explicit_);
    ushort2bool->SetConversionDistance(999);
    ushort2bool->SetSourceType(ushortType);
    ushort2bool->SetTargetType(boolType);
    ushort2bool->SetConversionInstructionName("us2bo");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ushort2bool));

    BasicTypeConversion* int2sbyte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"int2sbyte")));
    int2sbyte->SetAssembly(&assembly);
    int2sbyte->SetGroupNameConstant(groupName);
    int2sbyte->SetConversionType(ConversionType::explicit_);
    int2sbyte->SetConversionDistance(999);
    int2sbyte->SetSourceType(intType);
    int2sbyte->SetTargetType(sbyteType);
    int2sbyte->SetConversionInstructionName("in2sb");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(int2sbyte));

    BasicTypeConversion* int2byte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"int2byte")));
    int2byte->SetAssembly(&assembly);
    int2byte->SetGroupNameConstant(groupName);
    int2byte->SetConversionType(ConversionType::explicit_);
    int2byte->SetConversionDistance(999);
    int2byte->SetSourceType(intType);
    int2byte->SetTargetType(byteType);
    int2byte->SetConversionInstructionName("in2by");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(int2byte));

    BasicTypeConversion* int2short = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"int2short")));
    int2short->SetAssembly(&assembly);
    int2short->SetGroupNameConstant(groupName);
    int2short->SetConversionType(ConversionType::explicit_);
    int2short->SetConversionDistance(999);
    int2short->SetSourceType(intType);
    int2short->SetTargetType(shortType);
    int2short->SetConversionInstructionName("in2sh");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(int2short));

    BasicTypeConversion* int2ushort = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"int2ushort")));
    int2ushort->SetAssembly(&assembly);
    int2ushort->SetGroupNameConstant(groupName);
    int2ushort->SetConversionType(ConversionType::explicit_);
    int2ushort->SetConversionDistance(999);
    int2ushort->SetSourceType(intType);
    int2ushort->SetTargetType(ushortType);
    int2ushort->SetConversionInstructionName("in2us");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(int2ushort));

    BasicTypeConversion* int2uint = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"int2uint")));
    int2uint->SetAssembly(&assembly);
    int2uint->SetGroupNameConstant(groupName);
    int2uint->SetConversionType(ConversionType::explicit_);
    int2uint->SetConversionDistance(999);
    int2uint->SetSourceType(intType);
    int2uint->SetTargetType(uintType);
    int2uint->SetConversionInstructionName("in2ui");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(int2uint));

    BasicTypeConversion* int2long = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"int2long")));
    int2long->SetAssembly(&assembly);
    int2long->SetGroupNameConstant(groupName);
    int2long->SetConversionType(ConversionType::implicit_);
    int2long->SetConversionDistance(1);
    int2long->SetSourceType(intType);
    int2long->SetTargetType(longType);
    int2long->SetConversionInstructionName("in2lo");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(int2long));

    BasicTypeConversion* int2ulong = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"int2ulong")));
    int2ulong->SetAssembly(&assembly);
    int2ulong->SetGroupNameConstant(groupName);
    int2ulong->SetConversionType(ConversionType::implicit_);
    int2ulong->SetConversionDistance(2);
    int2ulong->SetSourceType(intType);
    int2ulong->SetTargetType(ulongType);
    int2ulong->SetConversionInstructionName("in2ul");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(int2ulong));

    BasicTypeConversion* int2float = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"int2float")));
    int2float->SetAssembly(&assembly);
    int2float->SetGroupNameConstant(groupName);
    int2float->SetConversionType(ConversionType::implicit_);
    int2float->SetConversionDistance(3);
    int2float->SetSourceType(intType);
    int2float->SetTargetType(floatType);
    int2float->SetConversionInstructionName("in2fl");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(int2float));

    BasicTypeConversion* int2double = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"int2double")));
    int2double->SetAssembly(&assembly);
    int2double->SetGroupNameConstant(groupName);
    int2double->SetConversionType(ConversionType::implicit_);
    int2double->SetConversionDistance(4);
    int2double->SetSourceType(intType);
    int2double->SetTargetType(doubleType);
    int2double->SetConversionInstructionName("in2do");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(int2double));

    BasicTypeConversion* int2char = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"int2char")));
    int2char->SetAssembly(&assembly);
    int2char->SetGroupNameConstant(groupName);
    int2char->SetConversionType(ConversionType::explicit_);
    int2char->SetConversionDistance(999);
    int2char->SetSourceType(intType);
    int2char->SetTargetType(charType);
    int2char->SetConversionInstructionName("in2ch");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(int2char));

    BasicTypeConversion* int2bool = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"int2bool")));
    int2bool->SetAssembly(&assembly);
    int2bool->SetGroupNameConstant(groupName);
    int2bool->SetConversionType(ConversionType::explicit_);
    int2bool->SetConversionDistance(999);
    int2bool->SetSourceType(intType);
    int2bool->SetTargetType(boolType);
    int2bool->SetConversionInstructionName("in2bo");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(int2bool));

    BasicTypeConversion* uint2sbyte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"uint2sbyte")));
    uint2sbyte->SetAssembly(&assembly);
    uint2sbyte->SetGroupNameConstant(groupName);
    uint2sbyte->SetConversionType(ConversionType::explicit_);
    uint2sbyte->SetConversionDistance(999);
    uint2sbyte->SetSourceType(uintType);
    uint2sbyte->SetTargetType(sbyteType);
    uint2sbyte->SetConversionInstructionName("ui2sb");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(uint2sbyte));

    BasicTypeConversion* uint2byte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"uint2byte")));
    uint2byte->SetAssembly(&assembly);
    uint2byte->SetGroupNameConstant(groupName);
    uint2byte->SetConversionType(ConversionType::explicit_);
    uint2byte->SetConversionDistance(999);
    uint2byte->SetSourceType(uintType);
    uint2byte->SetTargetType(byteType);
    uint2byte->SetConversionInstructionName("ui2by");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(uint2byte));

    BasicTypeConversion* uint2short = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"uint2short")));
    uint2short->SetAssembly(&assembly);
    uint2short->SetGroupNameConstant(groupName);
    uint2short->SetConversionType(ConversionType::explicit_);
    uint2short->SetConversionDistance(999);
    uint2short->SetSourceType(uintType);
    uint2short->SetTargetType(shortType);
    uint2short->SetConversionInstructionName("ui2sh");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(uint2short));

    BasicTypeConversion* uint2ushort = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"uint2ushort")));
    uint2ushort->SetAssembly(&assembly);
    uint2ushort->SetGroupNameConstant(groupName);
    uint2ushort->SetConversionType(ConversionType::explicit_);
    uint2ushort->SetConversionDistance(999);
    uint2ushort->SetSourceType(uintType);
    uint2ushort->SetTargetType(ushortType);
    uint2ushort->SetConversionInstructionName("ui2us");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(uint2ushort));

    BasicTypeConversion* uint2int = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"uint2int")));
    uint2int->SetAssembly(&assembly);
    uint2int->SetGroupNameConstant(groupName);
    uint2int->SetConversionType(ConversionType::explicit_);
    uint2int->SetConversionDistance(999);
    uint2int->SetSourceType(uintType);
    uint2int->SetTargetType(intType);
    uint2int->SetConversionInstructionName("ui2in");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(uint2int));

    BasicTypeConversion* uint2long = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"uint2long")));
    uint2long->SetAssembly(&assembly);
    uint2long->SetGroupNameConstant(groupName);
    uint2long->SetConversionType(ConversionType::implicit_);
    uint2long->SetConversionDistance(1);
    uint2long->SetSourceType(uintType);
    uint2long->SetTargetType(longType);
    uint2long->SetConversionInstructionName("ui2lo");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(uint2long));

    BasicTypeConversion* uint2ulong = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"uint2ulong")));
    uint2ulong->SetAssembly(&assembly);
    uint2ulong->SetGroupNameConstant(groupName);
    uint2ulong->SetConversionType(ConversionType::implicit_);
    uint2ulong->SetConversionDistance(2);
    uint2ulong->SetSourceType(uintType);
    uint2ulong->SetTargetType(ulongType);
    uint2ulong->SetConversionInstructionName("ui2ul");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(uint2ulong));

    BasicTypeConversion* uint2float = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"uint2float")));
    uint2float->SetAssembly(&assembly);
    uint2float->SetGroupNameConstant(groupName);
    uint2float->SetConversionType(ConversionType::implicit_);
    uint2float->SetConversionDistance(3);
    uint2float->SetSourceType(uintType);
    uint2float->SetTargetType(floatType);
    uint2float->SetConversionInstructionName("ui2fl");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(uint2float));

    BasicTypeConversion* uint2double = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"uint2double")));
    uint2double->SetAssembly(&assembly);
    uint2double->SetGroupNameConstant(groupName);
    uint2double->SetConversionType(ConversionType::implicit_);
    uint2double->SetConversionDistance(4);
    uint2double->SetSourceType(uintType);
    uint2double->SetTargetType(doubleType);
    uint2double->SetConversionInstructionName("ui2do");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(uint2double));

    BasicTypeConversion* uint2char = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"uint2char")));
    uint2char->SetAssembly(&assembly);
    uint2char->SetGroupNameConstant(groupName);
    uint2char->SetConversionType(ConversionType::explicit_);
    uint2char->SetConversionDistance(999);
    uint2char->SetSourceType(uintType);
    uint2char->SetTargetType(charType);
    uint2char->SetConversionInstructionName("ui2ch");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(uint2char));

    BasicTypeConversion* uint2bool = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"uint2bool")));
    uint2bool->SetAssembly(&assembly);
    uint2bool->SetGroupNameConstant(groupName);
    uint2bool->SetConversionType(ConversionType::explicit_);
    uint2bool->SetConversionDistance(999);
    uint2bool->SetSourceType(uintType);
    uint2bool->SetTargetType(boolType);
    uint2bool->SetConversionInstructionName("ui2bo");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(uint2bool));

    BasicTypeConversion* long2sbyte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"long2sbyte")));
    long2sbyte->SetAssembly(&assembly);
    long2sbyte->SetGroupNameConstant(groupName);
    long2sbyte->SetConversionType(ConversionType::explicit_);
    long2sbyte->SetConversionDistance(999);
    long2sbyte->SetSourceType(longType);
    long2sbyte->SetTargetType(sbyteType);
    long2sbyte->SetConversionInstructionName("lo2sb");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(long2sbyte));

    BasicTypeConversion* long2byte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"long2byte")));
    long2byte->SetAssembly(&assembly);
    long2byte->SetGroupNameConstant(groupName);
    long2byte->SetConversionType(ConversionType::explicit_);
    long2byte->SetConversionDistance(999);
    long2byte->SetSourceType(longType);
    long2byte->SetTargetType(byteType);
    long2byte->SetConversionInstructionName("lo2by");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(long2byte));

    BasicTypeConversion* long2short = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"long2short")));
    long2short->SetAssembly(&assembly);
    long2short->SetGroupNameConstant(groupName);
    long2short->SetConversionType(ConversionType::explicit_);
    long2short->SetConversionDistance(999);
    long2short->SetSourceType(longType);
    long2short->SetTargetType(shortType);
    long2short->SetConversionInstructionName("lo2sh");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(long2short));

    BasicTypeConversion* long2ushort = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"long2ushort")));
    long2ushort->SetAssembly(&assembly);
    long2ushort->SetGroupNameConstant(groupName);
    long2ushort->SetConversionType(ConversionType::explicit_);
    long2ushort->SetConversionDistance(999);
    long2ushort->SetSourceType(longType);
    long2ushort->SetTargetType(ushortType);
    long2ushort->SetConversionInstructionName("lo2us");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(long2ushort));

    BasicTypeConversion* long2int = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"long2int")));
    long2int->SetAssembly(&assembly);
    long2int->SetGroupNameConstant(groupName);
    long2int->SetConversionType(ConversionType::explicit_);
    long2int->SetConversionDistance(999);
    long2int->SetSourceType(longType);
    long2int->SetTargetType(intType);
    long2int->SetConversionInstructionName("lo2in");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(long2int));

    BasicTypeConversion* long2uint = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"long2uint")));
    long2uint->SetAssembly(&assembly);
    long2uint->SetGroupNameConstant(groupName);
    long2uint->SetConversionType(ConversionType::explicit_);
    long2uint->SetConversionDistance(999);
    long2uint->SetSourceType(longType);
    long2uint->SetTargetType(uintType);
    long2uint->SetConversionInstructionName("lo2ui");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(long2uint));

    BasicTypeConversion* long2ulong = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"long2ulong")));
    long2ulong->SetAssembly(&assembly);
    long2ulong->SetGroupNameConstant(groupName);
    long2ulong->SetConversionType(ConversionType::explicit_);
    long2ulong->SetConversionDistance(999);
    long2ulong->SetSourceType(longType);
    long2ulong->SetTargetType(ulongType);
    long2ulong->SetConversionInstructionName("lo2ul");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(long2ulong));

    BasicTypeConversion* long2float = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"long2float")));
    long2float->SetAssembly(&assembly);
    long2float->SetGroupNameConstant(groupName);
    long2float->SetConversionType(ConversionType::implicit_);
    long2float->SetConversionDistance(1);
    long2float->SetSourceType(longType);
    long2float->SetTargetType(floatType);
    long2float->SetConversionInstructionName("lo2fl");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(long2float));

    BasicTypeConversion* long2double = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"long2double")));
    long2double->SetAssembly(&assembly);
    long2double->SetGroupNameConstant(groupName);
    long2double->SetConversionType(ConversionType::implicit_);
    long2double->SetConversionDistance(2);
    long2double->SetSourceType(longType);
    long2double->SetTargetType(doubleType);
    long2double->SetConversionInstructionName("lo2do");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(long2double));

    BasicTypeConversion* long2char = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"long2char")));
    long2char->SetAssembly(&assembly);
    long2char->SetGroupNameConstant(groupName);
    long2char->SetConversionType(ConversionType::explicit_);
    long2char->SetConversionDistance(999);
    long2char->SetSourceType(longType);
    long2char->SetTargetType(charType);
    long2char->SetConversionInstructionName("lo2ch");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(long2char));

    BasicTypeConversion* long2bool = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"long2bool")));
    long2bool->SetAssembly(&assembly);
    long2bool->SetGroupNameConstant(groupName);
    long2bool->SetConversionType(ConversionType::explicit_);
    long2bool->SetConversionDistance(999);
    long2bool->SetSourceType(longType);
    long2bool->SetTargetType(boolType);
    long2bool->SetConversionInstructionName("lo2bo");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(long2bool));

    BasicTypeConversion* ulong2sbyte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ulong2sbyte")));
    ulong2sbyte->SetAssembly(&assembly);
    ulong2sbyte->SetGroupNameConstant(groupName);
    ulong2sbyte->SetConversionType(ConversionType::explicit_);
    ulong2sbyte->SetConversionDistance(999);
    ulong2sbyte->SetSourceType(ulongType);
    ulong2sbyte->SetTargetType(sbyteType);
    ulong2sbyte->SetConversionInstructionName("ul2sb");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ulong2sbyte));

    BasicTypeConversion* ulong2byte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ulong2byte")));
    ulong2byte->SetAssembly(&assembly);
    ulong2byte->SetGroupNameConstant(groupName);
    ulong2byte->SetConversionType(ConversionType::explicit_);
    ulong2byte->SetConversionDistance(999);
    ulong2byte->SetSourceType(ulongType);
    ulong2byte->SetTargetType(byteType);
    ulong2byte->SetConversionInstructionName("ul2by");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ulong2byte));

    BasicTypeConversion* ulong2short = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ulong2short")));
    ulong2short->SetAssembly(&assembly);
    ulong2short->SetGroupNameConstant(groupName);
    ulong2short->SetConversionType(ConversionType::explicit_);
    ulong2short->SetConversionDistance(999);
    ulong2short->SetSourceType(ulongType);
    ulong2short->SetTargetType(shortType);
    ulong2short->SetConversionInstructionName("ul2sh");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ulong2short));

    BasicTypeConversion* ulong2ushort = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ulong2ushort")));
    ulong2ushort->SetAssembly(&assembly);
    ulong2ushort->SetGroupNameConstant(groupName);
    ulong2ushort->SetConversionType(ConversionType::explicit_);
    ulong2ushort->SetConversionDistance(999);
    ulong2ushort->SetSourceType(ulongType);
    ulong2ushort->SetTargetType(ushortType);
    ulong2ushort->SetConversionInstructionName("ul2us");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ulong2ushort));

    BasicTypeConversion* ulong2int = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ulong2int")));
    ulong2int->SetAssembly(&assembly);
    ulong2int->SetGroupNameConstant(groupName);
    ulong2int->SetConversionType(ConversionType::explicit_);
    ulong2int->SetConversionDistance(999);
    ulong2int->SetSourceType(ulongType);
    ulong2int->SetTargetType(intType);
    ulong2int->SetConversionInstructionName("ul2in");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ulong2int));

    BasicTypeConversion* ulong2uint = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ulong2uint")));
    ulong2uint->SetAssembly(&assembly);
    ulong2uint->SetGroupNameConstant(groupName);
    ulong2uint->SetConversionType(ConversionType::explicit_);
    ulong2uint->SetConversionDistance(999);
    ulong2uint->SetSourceType(ulongType);
    ulong2uint->SetTargetType(uintType);
    ulong2uint->SetConversionInstructionName("ul2ui");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ulong2uint));

    BasicTypeConversion* ulong2long = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ulong2long")));
    ulong2long->SetAssembly(&assembly);
    ulong2long->SetGroupNameConstant(groupName);
    ulong2long->SetConversionType(ConversionType::explicit_);
    ulong2long->SetConversionDistance(999);
    ulong2long->SetSourceType(ulongType);
    ulong2long->SetTargetType(longType);
    ulong2long->SetConversionInstructionName("ul2lo");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ulong2long));

    BasicTypeConversion* ulong2float = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ulong2float")));
    ulong2float->SetAssembly(&assembly);
    ulong2float->SetGroupNameConstant(groupName);
    ulong2float->SetConversionType(ConversionType::implicit_);
    ulong2float->SetConversionDistance(1);
    ulong2float->SetSourceType(ulongType);
    ulong2float->SetTargetType(floatType);
    ulong2float->SetConversionInstructionName("ul2fl");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ulong2float));

    BasicTypeConversion* ulong2double = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ulong2double")));
    ulong2double->SetAssembly(&assembly);
    ulong2double->SetGroupNameConstant(groupName);
    ulong2double->SetConversionType(ConversionType::implicit_);
    ulong2double->SetConversionDistance(2);
    ulong2double->SetSourceType(ulongType);
    ulong2double->SetTargetType(doubleType);
    ulong2double->SetConversionInstructionName("ul2do");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ulong2double));

    BasicTypeConversion* ulong2char = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ulong2char")));
    ulong2char->SetAssembly(&assembly);
    ulong2char->SetGroupNameConstant(groupName);
    ulong2char->SetConversionType(ConversionType::explicit_);
    ulong2char->SetConversionDistance(999);
    ulong2char->SetSourceType(ulongType);
    ulong2char->SetTargetType(charType);
    ulong2char->SetConversionInstructionName("ul2ch");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ulong2char));

    BasicTypeConversion* ulong2bool = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ulong2bool")));
    ulong2bool->SetAssembly(&assembly);
    ulong2bool->SetGroupNameConstant(groupName);
    ulong2bool->SetConversionType(ConversionType::explicit_);
    ulong2bool->SetConversionDistance(999);
    ulong2bool->SetSourceType(ulongType);
    ulong2bool->SetTargetType(boolType);
    ulong2bool->SetConversionInstructionName("ul2bo");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ulong2bool));

    BasicTypeConversion* float2sbyte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"float2sbyte")));
    float2sbyte->SetAssembly(&assembly);
    float2sbyte->SetGroupNameConstant(groupName);
    float2sbyte->SetConversionType(ConversionType::explicit_);
    float2sbyte->SetConversionDistance(999);
    float2sbyte->SetSourceType(floatType);
    float2sbyte->SetTargetType(sbyteType);
    float2sbyte->SetConversionInstructionName("fl2sb");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(float2sbyte));

    BasicTypeConversion* float2byte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"float2byte")));
    float2byte->SetAssembly(&assembly);
    float2byte->SetGroupNameConstant(groupName);
    float2byte->SetConversionType(ConversionType::explicit_);
    float2byte->SetConversionDistance(999);
    float2byte->SetSourceType(floatType);
    float2byte->SetTargetType(byteType);
    float2byte->SetConversionInstructionName("fl2by");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(float2byte));

    BasicTypeConversion* float2short = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"float2short")));
    float2short->SetAssembly(&assembly);
    float2short->SetGroupNameConstant(groupName);
    float2short->SetConversionType(ConversionType::explicit_);
    float2short->SetConversionDistance(999);
    float2short->SetSourceType(floatType);
    float2short->SetTargetType(shortType);
    float2short->SetConversionInstructionName("fl2sh");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(float2short));

    BasicTypeConversion* float2ushort = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"float2ushort")));
    float2ushort->SetAssembly(&assembly);
    float2ushort->SetGroupNameConstant(groupName);
    float2ushort->SetConversionType(ConversionType::explicit_);
    float2ushort->SetConversionDistance(999);
    float2ushort->SetSourceType(floatType);
    float2ushort->SetTargetType(ushortType);
    float2ushort->SetConversionInstructionName("fl2us");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(float2ushort));

    BasicTypeConversion* float2int = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"float2int")));
    float2int->SetAssembly(&assembly);
    float2int->SetGroupNameConstant(groupName);
    float2int->SetConversionType(ConversionType::explicit_);
    float2int->SetConversionDistance(999);
    float2int->SetSourceType(floatType);
    float2int->SetTargetType(intType);
    float2int->SetConversionInstructionName("fl2in");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(float2int));

    BasicTypeConversion* float2uint = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"float2uint")));
    float2uint->SetAssembly(&assembly);
    float2uint->SetGroupNameConstant(groupName);
    float2uint->SetConversionType(ConversionType::explicit_);
    float2uint->SetConversionDistance(999);
    float2uint->SetSourceType(floatType);
    float2uint->SetTargetType(uintType);
    float2uint->SetConversionInstructionName("fl2ui");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(float2uint));

    BasicTypeConversion* float2long = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"float2long")));
    float2long->SetAssembly(&assembly);
    float2long->SetGroupNameConstant(groupName);
    float2long->SetConversionType(ConversionType::explicit_);
    float2long->SetConversionDistance(999);
    float2long->SetSourceType(floatType);
    float2long->SetTargetType(longType);
    float2long->SetConversionInstructionName("fl2lo");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(float2long));

    BasicTypeConversion* float2ulong = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"float2ulong")));
    float2ulong->SetAssembly(&assembly);
    float2ulong->SetGroupNameConstant(groupName);
    float2ulong->SetConversionType(ConversionType::explicit_);
    float2ulong->SetConversionDistance(999);
    float2ulong->SetSourceType(floatType);
    float2ulong->SetTargetType(ulongType);
    float2ulong->SetConversionInstructionName("fl2ul");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(float2ulong));

    BasicTypeConversion* float2double = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"float2double")));
    float2double->SetAssembly(&assembly);
    float2double->SetGroupNameConstant(groupName);
    float2double->SetConversionType(ConversionType::implicit_);
    float2double->SetConversionDistance(1);
    float2double->SetSourceType(floatType);
    float2double->SetTargetType(doubleType);
    float2double->SetConversionInstructionName("fl2do");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(float2double));

    BasicTypeConversion* float2char = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"float2char")));
    float2char->SetAssembly(&assembly);
    float2char->SetGroupNameConstant(groupName);
    float2char->SetConversionType(ConversionType::explicit_);
    float2char->SetConversionDistance(999);
    float2char->SetSourceType(floatType);
    float2char->SetTargetType(charType);
    float2char->SetConversionInstructionName("fl2ch");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(float2char));

    BasicTypeConversion* float2bool = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"float2bool")));
    float2bool->SetAssembly(&assembly);
    float2bool->SetGroupNameConstant(groupName);
    float2bool->SetConversionType(ConversionType::explicit_);
    float2bool->SetConversionDistance(999);
    float2bool->SetSourceType(floatType);
    float2bool->SetTargetType(boolType);
    float2bool->SetConversionInstructionName("fl2bo");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(float2bool));

    BasicTypeConversion* double2sbyte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"double2sbyte")));
    double2sbyte->SetAssembly(&assembly);
    double2sbyte->SetGroupNameConstant(groupName);
    double2sbyte->SetConversionType(ConversionType::explicit_);
    double2sbyte->SetConversionDistance(999);
    double2sbyte->SetSourceType(doubleType);
    double2sbyte->SetTargetType(sbyteType);
    double2sbyte->SetConversionInstructionName("do2sb");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(double2sbyte));

    BasicTypeConversion* double2byte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"double2byte")));
    double2byte->SetAssembly(&assembly);
    double2byte->SetGroupNameConstant(groupName);
    double2byte->SetConversionType(ConversionType::explicit_);
    double2byte->SetConversionDistance(999);
    double2byte->SetSourceType(doubleType);
    double2byte->SetTargetType(byteType);
    double2byte->SetConversionInstructionName("do2by");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(double2byte));

    BasicTypeConversion* double2short = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"double2short")));
    double2short->SetAssembly(&assembly);
    double2short->SetGroupNameConstant(groupName);
    double2short->SetConversionType(ConversionType::explicit_);
    double2short->SetConversionDistance(999);
    double2short->SetSourceType(doubleType);
    double2short->SetTargetType(shortType);
    double2short->SetConversionInstructionName("do2sh");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(double2short));

    BasicTypeConversion* double2ushort = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"double2ushort")));
    double2ushort->SetAssembly(&assembly);
    double2ushort->SetGroupNameConstant(groupName);
    double2ushort->SetConversionType(ConversionType::explicit_);
    double2ushort->SetConversionDistance(999);
    double2ushort->SetSourceType(doubleType);
    double2ushort->SetTargetType(ushortType);
    double2ushort->SetConversionInstructionName("do2us");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(double2ushort));

    BasicTypeConversion* double2int = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"double2int")));
    double2int->SetAssembly(&assembly);
    double2int->SetGroupNameConstant(groupName);
    double2int->SetConversionType(ConversionType::explicit_);
    double2int->SetConversionDistance(999);
    double2int->SetSourceType(doubleType);
    double2int->SetTargetType(intType);
    double2int->SetConversionInstructionName("do2in");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(double2int));

    BasicTypeConversion* double2uint = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"double2uint")));
    double2uint->SetAssembly(&assembly);
    double2uint->SetGroupNameConstant(groupName);
    double2uint->SetConversionType(ConversionType::explicit_);
    double2uint->SetConversionDistance(999);
    double2uint->SetSourceType(doubleType);
    double2uint->SetTargetType(uintType);
    double2uint->SetConversionInstructionName("do2ui");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(double2uint));

    BasicTypeConversion* double2long = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"double2long")));
    double2long->SetAssembly(&assembly);
    double2long->SetGroupNameConstant(groupName);
    double2long->SetConversionType(ConversionType::explicit_);
    double2long->SetConversionDistance(999);
    double2long->SetSourceType(doubleType);
    double2long->SetTargetType(longType);
    double2long->SetConversionInstructionName("do2lo");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(double2long));

    BasicTypeConversion* double2ulong = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"double2ulong")));
    double2ulong->SetAssembly(&assembly);
    double2ulong->SetGroupNameConstant(groupName);
    double2ulong->SetConversionType(ConversionType::explicit_);
    double2ulong->SetConversionDistance(999);
    double2ulong->SetSourceType(doubleType);
    double2ulong->SetTargetType(ulongType);
    double2ulong->SetConversionInstructionName("do2ul");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(double2ulong));

    BasicTypeConversion* double2char = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"double2char")));
    double2char->SetAssembly(&assembly);
    double2char->SetGroupNameConstant(groupName);
    double2char->SetConversionType(ConversionType::explicit_);
    double2char->SetConversionDistance(999);
    double2char->SetSourceType(doubleType);
    double2char->SetTargetType(charType);
    double2char->SetConversionInstructionName("do2ch");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(double2char));

    BasicTypeConversion* double2bool = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"double2bool")));
    double2bool->SetAssembly(&assembly);
    double2bool->SetGroupNameConstant(groupName);
    double2bool->SetConversionType(ConversionType::explicit_);
    double2bool->SetConversionDistance(999);
    double2bool->SetSourceType(doubleType);
    double2bool->SetTargetType(boolType);
    double2bool->SetConversionInstructionName("do2bo");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(double2bool));

    BasicTypeConversion* char2sbyte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"char2sbyte")));
    char2sbyte->SetAssembly(&assembly);
    char2sbyte->SetGroupNameConstant(groupName);
    char2sbyte->SetConversionType(ConversionType::explicit_);
    char2sbyte->SetConversionDistance(999);
    char2sbyte->SetSourceType(charType);
    char2sbyte->SetTargetType(sbyteType);
    char2sbyte->SetConversionInstructionName("ch2sb");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(char2sbyte));

    BasicTypeConversion* char2byte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"char2byte")));
    char2byte->SetAssembly(&assembly);
    char2byte->SetGroupNameConstant(groupName);
    char2byte->SetConversionType(ConversionType::explicit_);
    char2byte->SetConversionDistance(999);
    char2byte->SetSourceType(charType);
    char2byte->SetTargetType(byteType);
    char2byte->SetConversionInstructionName("ch2by");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(char2byte));

    BasicTypeConversion* char2short = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"char2short")));
    char2short->SetAssembly(&assembly);
    char2short->SetGroupNameConstant(groupName);
    char2short->SetConversionType(ConversionType::explicit_);
    char2short->SetConversionDistance(999);
    char2short->SetSourceType(charType);
    char2short->SetTargetType(shortType);
    char2short->SetConversionInstructionName("ch2sh");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(char2short));

    BasicTypeConversion* char2ushort = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"char2ushort")));
    char2ushort->SetAssembly(&assembly);
    char2ushort->SetGroupNameConstant(groupName);
    char2ushort->SetConversionType(ConversionType::explicit_);
    char2ushort->SetConversionDistance(999);
    char2ushort->SetSourceType(charType);
    char2ushort->SetTargetType(ushortType);
    char2ushort->SetConversionInstructionName("ch2us");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(char2ushort));

    BasicTypeConversion* char2int = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"char2int")));
    char2int->SetAssembly(&assembly);
    char2int->SetGroupNameConstant(groupName);
    char2int->SetConversionType(ConversionType::explicit_);
    char2int->SetConversionDistance(999);
    char2int->SetSourceType(charType);
    char2int->SetTargetType(intType);
    char2int->SetConversionInstructionName("ch2in");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(char2int));

    BasicTypeConversion* char2uint = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"char2uint")));
    char2uint->SetAssembly(&assembly);
    char2uint->SetGroupNameConstant(groupName);
    char2uint->SetConversionType(ConversionType::explicit_);
    char2uint->SetConversionDistance(999);
    char2uint->SetSourceType(charType);
    char2uint->SetTargetType(uintType);
    char2uint->SetConversionInstructionName("ch2ui");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(char2uint));

    BasicTypeConversion* char2long = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"char2long")));
    char2long->SetAssembly(&assembly);
    char2long->SetGroupNameConstant(groupName);
    char2long->SetConversionType(ConversionType::explicit_);
    char2long->SetConversionDistance(999);
    char2long->SetSourceType(charType);
    char2long->SetTargetType(longType);
    char2long->SetConversionInstructionName("ch2lo");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(char2long));

    BasicTypeConversion* char2ulong = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"char2ulong")));
    char2ulong->SetAssembly(&assembly);
    char2ulong->SetGroupNameConstant(groupName);
    char2ulong->SetConversionType(ConversionType::explicit_);
    char2ulong->SetConversionDistance(999);
    char2ulong->SetSourceType(charType);
    char2ulong->SetTargetType(ulongType);
    char2ulong->SetConversionInstructionName("ch2ul");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(char2ulong));

    BasicTypeConversion* char2float = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"char2float")));
    char2float->SetAssembly(&assembly);
    char2float->SetGroupNameConstant(groupName);
    char2float->SetConversionType(ConversionType::explicit_);
    char2float->SetConversionDistance(999);
    char2float->SetSourceType(charType);
    char2float->SetTargetType(floatType);
    char2float->SetConversionInstructionName("ch2fl");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(char2float));

    BasicTypeConversion* char2double = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"char2double")));
    char2double->SetAssembly(&assembly);
    char2double->SetGroupNameConstant(groupName);
    char2double->SetConversionType(ConversionType::explicit_);
    char2double->SetConversionDistance(999);
    char2double->SetSourceType(charType);
    char2double->SetTargetType(doubleType);
    char2double->SetConversionInstructionName("ch2do");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(char2double));

    BasicTypeConversion* char2bool = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"char2bool")));
    char2bool->SetAssembly(&assembly);
    char2bool->SetGroupNameConstant(groupName);
    char2bool->SetConversionType(ConversionType::explicit_);
    char2bool->SetConversionDistance(999);
    char2bool->SetSourceType(charType);
    char2bool->SetTargetType(boolType);
    char2bool->SetConversionInstructionName("ch2bo");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(char2bool));

    BasicTypeConversion* bool2sbyte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"bool2sbyte")));
    bool2sbyte->SetAssembly(&assembly);
    bool2sbyte->SetGroupNameConstant(groupName);
    bool2sbyte->SetConversionType(ConversionType::explicit_);
    bool2sbyte->SetConversionDistance(999);
    bool2sbyte->SetSourceType(boolType);
    bool2sbyte->SetTargetType(sbyteType);
    bool2sbyte->SetConversionInstructionName("bo2sb");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(bool2sbyte));

    BasicTypeConversion* bool2byte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"bool2byte")));
    bool2byte->SetAssembly(&assembly);
    bool2byte->SetGroupNameConstant(groupName);
    bool2byte->SetConversionType(ConversionType::explicit_);
    bool2byte->SetConversionDistance(999);
    bool2byte->SetSourceType(boolType);
    bool2byte->SetTargetType(byteType);
    bool2byte->SetConversionInstructionName("bo2by");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(bool2byte));

    BasicTypeConversion* bool2short = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"bool2short")));
    bool2short->SetAssembly(&assembly);
    bool2short->SetGroupNameConstant(groupName);
    bool2short->SetConversionType(ConversionType::explicit_);
    bool2short->SetConversionDistance(999);
    bool2short->SetSourceType(boolType);
    bool2short->SetTargetType(shortType);
    bool2short->SetConversionInstructionName("bo2sh");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(bool2short));

    BasicTypeConversion* bool2ushort = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"bool2ushort")));
    bool2ushort->SetAssembly(&assembly);
    bool2ushort->SetGroupNameConstant(groupName);
    bool2ushort->SetConversionType(ConversionType::explicit_);
    bool2ushort->SetConversionDistance(999);
    bool2ushort->SetSourceType(boolType);
    bool2ushort->SetTargetType(ushortType);
    bool2ushort->SetConversionInstructionName("bo2us");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(bool2ushort));

    BasicTypeConversion* bool2int = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"bool2int")));
    bool2int->SetAssembly(&assembly);
    bool2int->SetGroupNameConstant(groupName);
    bool2int->SetConversionType(ConversionType::explicit_);
    bool2int->SetConversionDistance(999);
    bool2int->SetSourceType(boolType);
    bool2int->SetTargetType(intType);
    bool2int->SetConversionInstructionName("bo2in");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(bool2int));

    BasicTypeConversion* bool2uint = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"bool2uint")));
    bool2uint->SetAssembly(&assembly);
    bool2uint->SetGroupNameConstant(groupName);
    bool2uint->SetConversionType(ConversionType::explicit_);
    bool2uint->SetConversionDistance(999);
    bool2uint->SetSourceType(boolType);
    bool2uint->SetTargetType(uintType);
    bool2uint->SetConversionInstructionName("bo2ui");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(bool2uint));

    BasicTypeConversion* bool2long = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"bool2long")));
    bool2long->SetAssembly(&assembly);
    bool2long->SetGroupNameConstant(groupName);
    bool2long->SetConversionType(ConversionType::explicit_);
    bool2long->SetConversionDistance(999);
    bool2long->SetSourceType(boolType);
    bool2long->SetTargetType(longType);
    bool2long->SetConversionInstructionName("bo2lo");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(bool2long));

    BasicTypeConversion* bool2ulong = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"bool2ulong")));
    bool2ulong->SetAssembly(&assembly);
    bool2ulong->SetGroupNameConstant(groupName);
    bool2ulong->SetConversionType(ConversionType::explicit_);
    bool2ulong->SetConversionDistance(999);
    bool2ulong->SetSourceType(boolType);
    bool2ulong->SetTargetType(ulongType);
    bool2ulong->SetConversionInstructionName("bo2ul");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(bool2ulong));

    BasicTypeConversion* bool2float = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"bool2float")));
    bool2float->SetAssembly(&assembly);
    bool2float->SetGroupNameConstant(groupName);
    bool2float->SetConversionType(ConversionType::explicit_);
    bool2float->SetConversionDistance(999);
    bool2float->SetSourceType(boolType);
    bool2float->SetTargetType(floatType);
    bool2float->SetConversionInstructionName("bo2fl");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(bool2float));

    BasicTypeConversion* bool2double = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"bool2double")));
    bool2double->SetAssembly(&assembly);
    bool2double->SetGroupNameConstant(groupName);
    bool2double->SetConversionType(ConversionType::explicit_);
    bool2double->SetConversionDistance(999);
    bool2double->SetSourceType(boolType);
    bool2double->SetTargetType(doubleType);
    bool2double->SetConversionInstructionName("bo2do");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(bool2double));

    BasicTypeConversion* bool2char = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"bool2char")));
    bool2char->SetAssembly(&assembly);
    bool2char->SetGroupNameConstant(groupName);
    bool2char->SetConversionType(ConversionType::explicit_);
    bool2char->SetConversionDistance(999);
    bool2char->SetSourceType(boolType);
    bool2char->SetTargetType(charType);
    bool2char->SetConversionInstructionName("bo2ch");
    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(bool2char));
    assembly.GetSymbolTable().EndNamespace();
}

void InitBasicTypeFun(Assembly& assembly)
{
    TypeSymbol* boolType = assembly.GetSymbolTable().GetType(U"System.Boolean");
    CreateBasicTypeBoolFun(assembly, boolType);
    TypeSymbol* charType = assembly.GetSymbolTable().GetType(U"System.Char");
    CreateBasicTypeCharFun(assembly, charType, boolType);
    TypeSymbol* sbyteType = assembly.GetSymbolTable().GetType(U"System.Int8");
    CreateBasicTypeIntFun(assembly, sbyteType, boolType);
    TypeSymbol* byteType = assembly.GetSymbolTable().GetType(U"System.UInt8");
    CreateBasicTypeIntFun(assembly, byteType, boolType);
    TypeSymbol* shortType = assembly.GetSymbolTable().GetType(U"System.Int16");
    CreateBasicTypeIntFun(assembly, shortType, boolType);
    TypeSymbol* ushortType = assembly.GetSymbolTable().GetType(U"System.UInt16");
    CreateBasicTypeIntFun(assembly, ushortType, boolType);
    TypeSymbol* intType = assembly.GetSymbolTable().GetType(U"System.Int32");
    CreateBasicTypeIntFun(assembly, intType, boolType);
    TypeSymbol* uintType = assembly.GetSymbolTable().GetType(U"System.UInt32");
    CreateBasicTypeIntFun(assembly, uintType, boolType);
    TypeSymbol* longType = assembly.GetSymbolTable().GetType(U"System.Int64");
    CreateBasicTypeIntFun(assembly, longType, boolType);
    TypeSymbol* ulongType = assembly.GetSymbolTable().GetType(U"System.UInt64");
    CreateBasicTypeIntFun(assembly, ulongType, boolType);
    TypeSymbol* floatType = assembly.GetSymbolTable().GetType(U"System.Float");
    CreateBasicTypeFloatFun(assembly, floatType, boolType);
    TypeSymbol* doubleType = assembly.GetSymbolTable().GetType(U"System.Double");
    CreateBasicTypeFloatFun(assembly, doubleType, boolType);
    CreateBasicTypeConversions(assembly, boolType, charType, sbyteType, byteType, shortType, ushortType, intType, uintType, longType, ulongType, floatType, doubleType);
}

} } // namespace cminor::symbols
