// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/ObjectFun.hpp>
#include <cminor/symbols/StringFun.hpp>
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

void CreateArraySizeConstructor(Assembly& assembly, ArrayTypeSymbol* arrayTypeSymbol)
{
    ConstantPool& constantPool = assembly.GetConstantPool();
    ArraySizeConstructorSymbol* arraySizeConstructorSymbol = new ArraySizeConstructorSymbol(Span(), constantPool.GetEmptyStringConstant());
    arraySizeConstructorSymbol->SetAssembly(&assembly);
    arraySizeConstructorSymbol->SetPublic();
    arraySizeConstructorSymbol->SetGroupNameConstant(constantPool.GetConstant(constantPool.Install(U"@constructor")));
    ParameterSymbol* thisParameterSymbol = new ParameterSymbol(Span(), constantPool.GetConstant(constantPool.Install(U"this")));
    thisParameterSymbol->SetAssembly(&assembly);
    thisParameterSymbol->SetType(arrayTypeSymbol);
    thisParameterSymbol->SetBound();
    arraySizeConstructorSymbol->AddSymbol(std::unique_ptr<Symbol>(thisParameterSymbol));
    ParameterSymbol* sizeParameterSymbol = new ParameterSymbol(Span(), constantPool.GetConstant(constantPool.Install(U"size")));
    sizeParameterSymbol->SetAssembly(&assembly);
    sizeParameterSymbol->SetType(assembly.GetSymbolTable().GetType(U"System.Int32"));
    sizeParameterSymbol->SetBound();
    arraySizeConstructorSymbol->AddSymbol(std::unique_ptr<Symbol>(sizeParameterSymbol));
    arraySizeConstructorSymbol->ComputeName();
    arrayTypeSymbol->AddSymbol(std::unique_ptr<Symbol>(arraySizeConstructorSymbol));
}

ObjectDefaultInit::ObjectDefaultInit(const Span& span_, Constant name_) : BasicTypeInit(span_, name_)
{
}

void ObjectDefaultInit::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    std::unique_ptr<Instruction> inst = machine.CreateInst("def", "object");
    function.AddInst(std::move(inst));
    GenObject* target = objects[0];
    target->GenStore(machine, function);
}

ObjectCopyInit::ObjectCopyInit(const Span& span_, Constant name_) : BasicTypeInit(span_, name_)
{
}

void ObjectCopyInit::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
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

void ObjectNullInit::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    std::unique_ptr<Instruction> inst = machine.CreateInst("def", "object");
    function.AddInst(std::move(inst));
    GenObject* target = objects[0];
    target->GenStore(machine, function);
}

ObjectAssignment::ObjectAssignment(const Span& span_, Constant name_) : BasicTypeFun(span_, name_)
{
}

void ObjectAssignment::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    Assert(objects.size() == 2, "assign needs two objects");
    GenObject* source = objects[1];
    source->GenLoad(machine, function);
    std::unique_ptr<Instruction> inst = machine.CreateInst("copy", "object");
    function.AddInst(std::move(inst));
    GenObject* target = objects[0];
    target->GenStore(machine, function);
}

ObjectNullAssignment::ObjectNullAssignment(const Span& span_, Constant name_) : BasicTypeFun(span_, name_)
{
}

void ObjectNullAssignment::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    std::unique_ptr<Instruction> inst = machine.CreateInst("def", "object");
    function.AddInst(std::move(inst));
    GenObject* target = objects[0];
    target->GenStore(machine, function);
}

ObjectNullEqual::ObjectNullEqual(const Span& span_, Constant name_) : BasicTypeFun(span_, name_)
{
}

void ObjectNullEqual::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    std::unique_ptr<Instruction> inst = machine.CreateInst("equalonull");
    Assert(objects.size() == 2, "null equality comparison needs two objects");
    GenObject* left = objects[0];
    left->GenLoad(machine, function);
    GenObject* right = objects[1];
    right->GenLoad(machine, function);
    function.AddInst(std::move(inst));
}

NullObjectEqual::NullObjectEqual(const Span& span_, Constant name_) : BasicTypeFun(span_, name_)
{
}

void NullObjectEqual::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
    std::unique_ptr<Instruction> inst = machine.CreateInst("equalnullo");
    Assert(objects.size() == 2, "null equality comparison needs two objects");
    GenObject* left = objects[0];
    left->GenLoad(machine, function);
    GenObject* right = objects[1];
    right->GenLoad(machine, function);
    function.AddInst(std::move(inst));
}

NullToObjectConversion::NullToObjectConversion(const Span& span_, Constant name_) : BasicTypeFun(span_, name_)
{
    SetConversionFun();
}

void NullToObjectConversion::GenerateCall(Machine& machine, Assembly& assembly, Function& function, std::vector<GenObject*>& objects, int start)
{
}

void CreateBasicTypeObjectFun(Assembly& assembly, TypeSymbol* classOrInterfaceType)
{
    ConstantPool& constantPool = assembly.GetConstantPool();
    Constant thisParamName = constantPool.GetConstant(constantPool.Install(U"this"));
    Constant thatParamName = constantPool.GetConstant(constantPool.Install(U"that"));
    ParameterSymbol* thisParam1 = new ParameterSymbol(Span(), thisParamName);
    thisParam1->SetAssembly(&assembly);
    thisParam1->SetType(classOrInterfaceType);
    ObjectDefaultInit* defaultInit = new ObjectDefaultInit(Span(), constantPool.GetEmptyStringConstant());
    defaultInit->SetAssembly(&assembly);
    defaultInit->SetType(classOrInterfaceType);
    defaultInit->AddSymbol(std::unique_ptr<Symbol>(thisParam1));
    defaultInit->ComputeName();
    classOrInterfaceType->AddSymbol(std::unique_ptr<FunctionSymbol>(defaultInit));

    ParameterSymbol* thisParam2 = new ParameterSymbol(Span(), thisParamName);
    thisParam2->SetAssembly(&assembly);
    thisParam2->SetType(classOrInterfaceType);
    ParameterSymbol* thatParam2 = new ParameterSymbol(Span(), thatParamName);
    thatParam2->SetAssembly(&assembly);
    thatParam2->SetType(classOrInterfaceType);
    ObjectCopyInit* copyInit = new ObjectCopyInit(Span(), constantPool.GetEmptyStringConstant());
    copyInit->SetAssembly(&assembly);
    copyInit->SetType(classOrInterfaceType);
    copyInit->AddSymbol(std::unique_ptr<Symbol>(thisParam2));
    copyInit->AddSymbol(std::unique_ptr<Symbol>(thatParam2));
    copyInit->ComputeName();
    classOrInterfaceType->AddSymbol(std::unique_ptr<FunctionSymbol>(copyInit));

    TypeSymbol* nullRefType = assembly.GetSymbolTable().GetType(U"System.@nullref");

    ParameterSymbol* thisParam3 = new ParameterSymbol(Span(), thisParamName);
    thisParam3->SetAssembly(&assembly);
    thisParam3->SetType(classOrInterfaceType);
    ParameterSymbol* thatParam3 = new ParameterSymbol(Span(), thatParamName);
    thatParam3->SetAssembly(&assembly);
    thatParam3->SetType(nullRefType);
    ObjectNullInit* nullInit = new ObjectNullInit(Span(), constantPool.GetEmptyStringConstant());
    nullInit->SetAssembly(&assembly);
    nullInit->SetType(classOrInterfaceType);
    nullInit->AddSymbol(std::unique_ptr<Symbol>(thisParam3));
    nullInit->AddSymbol(std::unique_ptr<Symbol>(thatParam3));
    nullInit->ComputeName();
    classOrInterfaceType->AddSymbol(std::unique_ptr<FunctionSymbol>(nullInit));

    ParameterSymbol* thisParam4 = new ParameterSymbol(Span(), thisParamName);
    thisParam4->SetAssembly(&assembly);
    thisParam4->SetType(classOrInterfaceType);
    ParameterSymbol* thatParam4 = new ParameterSymbol(Span(), thatParamName);
    thatParam4->SetAssembly(&assembly);
    thatParam4->SetType(classOrInterfaceType);
    ObjectAssignment* assignment = new ObjectAssignment(Span(), constantPool.GetEmptyStringConstant());
    Constant assignmentGroupName = assembly.GetConstantPool().GetConstant(assembly.GetConstantPool().Install(U"@assignment"));
    assignment->SetGroupNameConstant(assignmentGroupName);
    assignment->SetAssembly(&assembly);
    assignment->SetType(classOrInterfaceType);
    assignment->AddSymbol(std::unique_ptr<Symbol>(thisParam4));
    assignment->AddSymbol(std::unique_ptr<Symbol>(thatParam4));
    assignment->ComputeName();
    classOrInterfaceType->AddSymbol(std::unique_ptr<FunctionSymbol>(assignment));

    ParameterSymbol* thisParam5 = new ParameterSymbol(Span(), thisParamName);
    thisParam5->SetAssembly(&assembly);
    thisParam5->SetType(classOrInterfaceType);
    ParameterSymbol* thatParam5 = new ParameterSymbol(Span(), thatParamName);
    thatParam5->SetAssembly(&assembly);
    thatParam5->SetType(nullRefType);
    ObjectNullAssignment* nullAssignment = new ObjectNullAssignment(Span(), constantPool.GetEmptyStringConstant());
    nullAssignment->SetAssembly(&assembly);
    nullAssignment->SetGroupNameConstant(assignmentGroupName);
    nullAssignment->SetType(classOrInterfaceType);
    nullAssignment->AddSymbol(std::unique_ptr<Symbol>(thisParam5));
    nullAssignment->AddSymbol(std::unique_ptr<Symbol>(thatParam5));
    nullAssignment->ComputeName();
    classOrInterfaceType->AddSymbol(std::unique_ptr<FunctionSymbol>(nullAssignment));

    BasicTypeReturn* returnFun = new BasicTypeReturn(Span(), constantPool.GetEmptyStringConstant());
    returnFun->SetAssembly(&assembly);
    returnFun->SetType(classOrInterfaceType);
    returnFun->SetReturnType(classOrInterfaceType);
    Constant valueParamName = constantPool.GetConstant(constantPool.Install(U"value"));
    ParameterSymbol* valueParam = new ParameterSymbol(Span(), valueParamName);
    valueParam->SetAssembly(&assembly);
    valueParam->SetType(classOrInterfaceType);
    returnFun->AddSymbol(std::unique_ptr<Symbol>(valueParam));
    returnFun->ComputeName();
    classOrInterfaceType->AddSymbol(std::unique_ptr<FunctionSymbol>(returnFun));

    assembly.GetSymbolTable().BeginNamespace(StringPtr(U"System"), Span());
    Constant opEqualGroupName = constantPool.GetConstant(constantPool.Install(StringPtr(U"operator==")));

    ObjectNullEqual* objectNullEqual = new ObjectNullEqual(Span(), constantPool.GetEmptyStringConstant());
    objectNullEqual->SetAssembly(&assembly);
    objectNullEqual->SetGroupNameConstant(opEqualGroupName);
    objectNullEqual->SetType(classOrInterfaceType);
    TypeSymbol* boolType = assembly.GetSymbolTable().GetType(U"System.Boolean");
    objectNullEqual->SetReturnType(boolType);
    ParameterSymbol* thisParam6 = new ParameterSymbol(Span(), thisParamName);
    thisParam6->SetAssembly(&assembly);
    thisParam6->SetType(classOrInterfaceType);
    ParameterSymbol* thatParam6 = new ParameterSymbol(Span(), thatParamName);
    thatParam6->SetAssembly(&assembly);
    thatParam6->SetType(nullRefType);
    objectNullEqual->AddSymbol(std::unique_ptr<Symbol>(thisParam6));
    objectNullEqual->AddSymbol(std::unique_ptr<Symbol>(thatParam6));
    objectNullEqual->ComputeName();

    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<Symbol>(objectNullEqual));

    NullObjectEqual* nullObjectEqual = new NullObjectEqual(Span(), constantPool.GetEmptyStringConstant());
    nullObjectEqual->SetAssembly(&assembly);
    nullObjectEqual->SetGroupNameConstant(opEqualGroupName);
    nullObjectEqual->SetType(classOrInterfaceType);
    nullObjectEqual->SetReturnType(boolType);
    ParameterSymbol* thisParam7 = new ParameterSymbol(Span(), thisParamName);
    thisParam7->SetAssembly(&assembly);
    thisParam7->SetType(nullRefType);
    ParameterSymbol* thatParam7 = new ParameterSymbol(Span(), thatParamName);
    thatParam7->SetAssembly(&assembly);
    thatParam7->SetType(classOrInterfaceType);
    nullObjectEqual->AddSymbol(std::unique_ptr<Symbol>(thisParam7));
    nullObjectEqual->AddSymbol(std::unique_ptr<Symbol>(thatParam7));
    nullObjectEqual->ComputeName();

    assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<Symbol>(nullObjectEqual));

    assembly.GetSymbolTable().EndNamespace();

    if (classOrInterfaceType->FullName() == U"System.Object")
    {
        assembly.GetSymbolTable().BeginNamespace(StringPtr(U"System"), Span());
        TypeSymbol* objectType = classOrInterfaceType;
        ConstantPool& constantPool = assembly.GetConstantPool();
        Constant groupName = constantPool.GetConstant(constantPool.Install(StringPtr(U"@conversion")));

        TypeSymbol* sbyteType = assembly.GetSymbolTable().GetType(U"System.Int8");

        BasicTypeConversion* sbyte2object = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"sbyte2object")));
        sbyte2object->SetAssembly(&assembly);
        sbyte2object->SetGroupNameConstant(groupName);
        sbyte2object->SetConversionType(ConversionType::implicit_);
        sbyte2object->SetConversionDistance(10);
        sbyte2object->SetSourceType(sbyteType);
        sbyte2object->SetTargetType(objectType);
        sbyte2object->SetConversionInstructionName("sb2o");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(sbyte2object));

        BasicTypeConversion* object2sbyte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"object2sbyte")));
        object2sbyte->SetAssembly(&assembly);
        object2sbyte->SetGroupNameConstant(groupName);
        object2sbyte->SetConversionType(ConversionType::explicit_);
        object2sbyte->SetConversionDistance(999);
        object2sbyte->SetSourceType(objectType);
        object2sbyte->SetTargetType(sbyteType);
        object2sbyte->SetConversionInstructionName("o2sb");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(object2sbyte));

        TypeSymbol* byteType = assembly.GetSymbolTable().GetType(U"System.UInt8");

        BasicTypeConversion* byte2object = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"byte2object")));
        byte2object->SetAssembly(&assembly);
        byte2object->SetGroupNameConstant(groupName);
        byte2object->SetConversionType(ConversionType::implicit_);
        byte2object->SetConversionDistance(10);
        byte2object->SetSourceType(byteType);
        byte2object->SetTargetType(objectType);
        byte2object->SetConversionInstructionName("by2o");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(byte2object));

        BasicTypeConversion* object2byte = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"object2byte")));
        object2byte->SetAssembly(&assembly);
        object2byte->SetGroupNameConstant(groupName);
        object2byte->SetConversionType(ConversionType::explicit_);
        object2byte->SetConversionDistance(999);
        object2byte->SetSourceType(objectType);
        object2byte->SetTargetType(byteType);
        object2byte->SetConversionInstructionName("o2by");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(object2byte));

        TypeSymbol* shortType = assembly.GetSymbolTable().GetType(U"System.Int16");

        BasicTypeConversion* short2object = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"short2object")));
        short2object->SetAssembly(&assembly);
        short2object->SetGroupNameConstant(groupName);
        short2object->SetConversionType(ConversionType::implicit_);
        short2object->SetConversionDistance(10);
        short2object->SetSourceType(shortType);
        short2object->SetTargetType(objectType);
        short2object->SetConversionInstructionName("sh2o");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(short2object));

        BasicTypeConversion* object2short = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"object2short")));
        object2short->SetAssembly(&assembly);
        object2short->SetGroupNameConstant(groupName);
        object2short->SetConversionType(ConversionType::explicit_);
        object2short->SetConversionDistance(999);
        object2short->SetSourceType(objectType);
        object2short->SetTargetType(shortType);
        object2short->SetConversionInstructionName("o2sh");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(object2short));

        TypeSymbol* ushortType = assembly.GetSymbolTable().GetType(U"System.UInt16");

        BasicTypeConversion* ushort2object = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ushort2object")));
        ushort2object->SetAssembly(&assembly);
        ushort2object->SetGroupNameConstant(groupName);
        ushort2object->SetConversionType(ConversionType::implicit_);
        ushort2object->SetConversionDistance(10);
        ushort2object->SetSourceType(ushortType);
        ushort2object->SetTargetType(objectType);
        ushort2object->SetConversionInstructionName("us2o");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ushort2object));

        BasicTypeConversion* object2ushort = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"object2ushort")));
        object2ushort->SetAssembly(&assembly);
        object2ushort->SetGroupNameConstant(groupName);
        object2ushort->SetConversionType(ConversionType::explicit_);
        object2ushort->SetConversionDistance(999);
        object2ushort->SetSourceType(objectType);
        object2ushort->SetTargetType(ushortType);
        object2ushort->SetConversionInstructionName("o2us");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(object2ushort));

        TypeSymbol* intType = assembly.GetSymbolTable().GetType(U"System.Int32");

        BasicTypeConversion* int2object = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"int2object")));
        int2object->SetAssembly(&assembly);
        int2object->SetGroupNameConstant(groupName);
        int2object->SetConversionType(ConversionType::implicit_);
        int2object->SetConversionDistance(10);
        int2object->SetSourceType(intType);
        int2object->SetTargetType(objectType);
        int2object->SetConversionInstructionName("in2o");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(int2object));

        BasicTypeConversion* object2int = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"object2int")));
        object2int->SetAssembly(&assembly);
        object2int->SetGroupNameConstant(groupName);
        object2int->SetConversionType(ConversionType::explicit_);
        object2int->SetConversionDistance(999);
        object2int->SetSourceType(objectType);
        object2int->SetTargetType(intType);
        object2int->SetConversionInstructionName("o2in");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(object2int));

        TypeSymbol* uintType = assembly.GetSymbolTable().GetType(U"System.UInt32");

        BasicTypeConversion* uint2object = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"uint2object")));
        uint2object->SetAssembly(&assembly);
        uint2object->SetGroupNameConstant(groupName);
        uint2object->SetConversionType(ConversionType::implicit_);
        uint2object->SetConversionDistance(10);
        uint2object->SetSourceType(uintType);
        uint2object->SetTargetType(objectType);
        uint2object->SetConversionInstructionName("ui2o");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(uint2object));

        BasicTypeConversion* object2uint = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"object2uint")));
        object2uint->SetAssembly(&assembly);
        object2uint->SetGroupNameConstant(groupName);
        object2uint->SetConversionType(ConversionType::explicit_);
        object2uint->SetConversionDistance(999);
        object2uint->SetSourceType(objectType);
        object2uint->SetTargetType(uintType);
        object2uint->SetConversionInstructionName("o2ui");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(object2uint));

        TypeSymbol* longType = assembly.GetSymbolTable().GetType(U"System.Int64");

        BasicTypeConversion* long2object = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"long2object")));
        long2object->SetAssembly(&assembly);
        long2object->SetGroupNameConstant(groupName);
        long2object->SetConversionType(ConversionType::implicit_);
        long2object->SetConversionDistance(10);
        long2object->SetSourceType(longType);
        long2object->SetTargetType(objectType);
        long2object->SetConversionInstructionName("lo2o");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(long2object));

        BasicTypeConversion* object2long = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"object2long")));
        object2long->SetAssembly(&assembly);
        object2long->SetGroupNameConstant(groupName);
        object2long->SetConversionType(ConversionType::explicit_);
        object2long->SetConversionDistance(999);
        object2long->SetSourceType(objectType);
        object2long->SetTargetType(longType);
        object2long->SetConversionInstructionName("o2lo");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(object2long));

        TypeSymbol* ulongType = assembly.GetSymbolTable().GetType(U"System.UInt64");

        BasicTypeConversion* ulong2object = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"ulong2object")));
        ulong2object->SetAssembly(&assembly);
        ulong2object->SetGroupNameConstant(groupName);
        ulong2object->SetConversionType(ConversionType::implicit_);
        ulong2object->SetConversionDistance(10);
        ulong2object->SetSourceType(ulongType);
        ulong2object->SetTargetType(objectType);
        ulong2object->SetConversionInstructionName("ul2o");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(ulong2object));

        BasicTypeConversion* object2ulong = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"object2ulong")));
        object2ulong->SetAssembly(&assembly);
        object2ulong->SetGroupNameConstant(groupName);
        object2ulong->SetConversionType(ConversionType::explicit_);
        object2ulong->SetConversionDistance(999);
        object2ulong->SetSourceType(objectType);
        object2ulong->SetTargetType(ulongType);
        object2ulong->SetConversionInstructionName("o2ul");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(object2ulong));

        TypeSymbol* floatType = assembly.GetSymbolTable().GetType(U"System.Float");

        BasicTypeConversion* float2object = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"float2object")));
        float2object->SetAssembly(&assembly);
        float2object->SetGroupNameConstant(groupName);
        float2object->SetConversionType(ConversionType::implicit_);
        float2object->SetConversionDistance(10);
        float2object->SetSourceType(floatType);
        float2object->SetTargetType(objectType);
        float2object->SetConversionInstructionName("fl2o");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(float2object));

        BasicTypeConversion* object2float = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"object2float")));
        object2float->SetAssembly(&assembly);
        object2float->SetGroupNameConstant(groupName);
        object2float->SetConversionType(ConversionType::explicit_);
        object2float->SetConversionDistance(999);
        object2float->SetSourceType(objectType);
        object2float->SetTargetType(floatType);
        object2float->SetConversionInstructionName("o2fl");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(object2float));

        TypeSymbol* doubleType = assembly.GetSymbolTable().GetType(U"System.Double");

        BasicTypeConversion* double2object = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"double2object")));
        double2object->SetAssembly(&assembly);
        double2object->SetGroupNameConstant(groupName);
        double2object->SetConversionType(ConversionType::implicit_);
        double2object->SetConversionDistance(10);
        double2object->SetSourceType(doubleType);
        double2object->SetTargetType(objectType);
        double2object->SetConversionInstructionName("do2o");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(double2object));

        BasicTypeConversion* object2double = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"object2double")));
        object2double->SetAssembly(&assembly);
        object2double->SetGroupNameConstant(groupName);
        object2double->SetConversionType(ConversionType::explicit_);
        object2double->SetConversionDistance(999);
        object2double->SetSourceType(objectType);
        object2double->SetTargetType(doubleType);
        object2double->SetConversionInstructionName("o2do");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(object2double));

        TypeSymbol* charType = assembly.GetSymbolTable().GetType(U"System.Char");

        BasicTypeConversion* char2object = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"char2object")));
        char2object->SetAssembly(&assembly);
        char2object->SetGroupNameConstant(groupName);
        char2object->SetConversionType(ConversionType::implicit_);
        char2object->SetConversionDistance(10);
        char2object->SetSourceType(charType);
        char2object->SetTargetType(objectType);
        char2object->SetConversionInstructionName("ch2o");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(char2object));

        BasicTypeConversion* object2char = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"object2char")));
        object2char->SetAssembly(&assembly);
        object2char->SetGroupNameConstant(groupName);
        object2char->SetConversionType(ConversionType::explicit_);
        object2char->SetConversionDistance(999);
        object2char->SetSourceType(objectType);
        object2char->SetTargetType(charType);
        object2char->SetConversionInstructionName("o2ch");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(object2char));

        TypeSymbol* boolType = assembly.GetSymbolTable().GetType(U"System.Boolean");

        BasicTypeConversion* bool2object = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"bool2object")));
        bool2object->SetAssembly(&assembly);
        bool2object->SetGroupNameConstant(groupName);
        bool2object->SetConversionType(ConversionType::implicit_);
        bool2object->SetConversionDistance(10);
        bool2object->SetSourceType(boolType);
        bool2object->SetTargetType(objectType);
        bool2object->SetConversionInstructionName("bo2o");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(bool2object));

        BasicTypeConversion* object2bool = new BasicTypeConversion(Span(), constantPool.GetConstant(constantPool.Install(U"object2bool")));
        object2bool->SetAssembly(&assembly);
        object2bool->SetGroupNameConstant(groupName);
        object2bool->SetConversionType(ConversionType::explicit_);
        object2bool->SetConversionDistance(999);
        object2bool->SetSourceType(objectType);
        object2bool->SetTargetType(boolType);
        object2bool->SetConversionInstructionName("o2bo");
        assembly.GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(object2bool));

        assembly.GetSymbolTable().EndNamespace();
    }
    else if (classOrInterfaceType->FullName() == U"System.String")
    {
        ClassTypeSymbol* stringTypeSymbol = dynamic_cast<ClassTypeSymbol*>(classOrInterfaceType);
        Assert(stringTypeSymbol, "System.String expected");
        CreateStringLiteralToStringConversion(assembly, stringTypeSymbol);
    }
}

} } // namespace cminor::symbols
