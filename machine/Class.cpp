// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Class.hpp>
#include <cminor/machine/Type.hpp>

namespace cminor { namespace machine {

void MethodTable::Resize(int32_t n)
{
    methods.resize(n);
}

StringPtr MethodTable::GetMethodName(int32_t index) const
{
    Assert(index >= 0 && index < int32_t(methods.size()), "invalid method index");
    Constant methodConstant = methods[index];
    Assert(methodConstant.Value().GetType() == ValueType::stringLiteral, "string literal expected");
    return StringPtr(methodConstant.Value().AsStringLiteral());
}

void MethodTable::SetMethodName(int32_t index, Constant methodNameConstant)
{
    Assert(index >= 0 && index < int32_t(methods.size()), "invalid method index");
    Assert(methodNameConstant.Value().GetType() == ValueType::stringLiteral, "string literal expected");
    methods[index] = methodNameConstant;
}

Function* MethodTable::GetMethod(int32_t index) const
{
    Assert(index >= 0 && index < int32_t(methods.size()), "invalid method index");
    Constant methodConstant = methods[index];
    Assert(methodConstant.Value().GetType() == ValueType::functionPtr, "function pointer expected");
    return methodConstant.Value().AsFunctionPtr();
}

void MethodTable::SetMethod(int32_t index, Function* function)
{
    Assert(index >= 0 && index < int32_t(methods.size()), "invalid method index");
    methods[index].SetValue(IntegralValue(function));
}

void MethodTable::Write(Writer& writer)
{
    uint32_t n = uint32_t(methods.size());
    writer.PutEncodedUInt(n);
    for (uint32_t i = 0; i < n; ++i)
    {
        Constant method = methods[i];
        if (method.Value().GetType() == ValueType::functionPtr)
        {
            if (method.Value().AsFunctionPtr())
            {
                method = method.Value().AsFunctionPtr()->CallName();
            }
            else
            {
                method = writer.GetConstantPool()->GetEmptyStringConstant();
            }
        }
        ConstantId constantId = writer.GetConstantPool()->GetIdFor(method);
        if (constantId != noConstantId)
        {
            constantId.Write(writer);
        }
        else
        {
            throw std::runtime_error("got no id for constant");
        }
    }
}

void MethodTable::Read(Reader& reader) 
{
    uint32_t n = reader.GetEncodedUInt();
    for (uint32_t i = 0; i < n; ++i)
    {
        ConstantId constantId;
        constantId.Read(reader);
        Constant method = reader.GetConstantPool()->GetConstant(constantId);
        methods.push_back(method);
    }
}

StaticClassData::StaticClassData() : initialized(false), initializing(false), initMtx(), staticConstructorName(), staticData()
{
}

void StaticClassData::Write(Writer& writer)
{
    bool hasStaticConstructorName = staticConstructorName.Value().AsStringLiteral() != nullptr;
    writer.Put(hasStaticConstructorName);
    if (hasStaticConstructorName)
    {
        ConstantId staticConstructorNameId = writer.GetConstantPool()->GetIdFor(staticConstructorName);
        Assert(staticConstructorNameId != noConstantId, "got no id for constant");
        staticConstructorNameId.Write(writer);
    }
    staticLayout.Write(writer);
}

void StaticClassData::Read(Reader& reader)
{
    bool hasStaticConstructorName = reader.GetBool();
    if (hasStaticConstructorName)
    {
        ConstantId staticConstructorNameId;
        staticConstructorNameId.Read(reader);
        staticConstructorName = reader.GetConstantPool()->GetConstant(staticConstructorNameId);
    }
    staticLayout.Read(reader);
}

void StaticClassData::SetStaticConstructorName(Constant staticConstructorName_)
{
    staticConstructorName = staticConstructorName_;
}

void StaticClassData::AllocateStaticData()
{
    size_t size = staticLayout.Size();
    staticData = new uint8_t[size];
    std::memset(staticData, 0, size);
}

void StaticClassData::SetStaticField(IntegralValue fieldValue, int32_t index)
{
    Field& field = staticLayout.GetField(index);
    void* fieldPtr = static_cast<uint8_t*>(staticData) + field.Offset().Value();
    switch (field.GetType())
    {
        case ValueType::boolType: *static_cast<bool*>(fieldPtr) = fieldValue.AsBool(); break;
        case ValueType::sbyteType: *static_cast<int8_t*>(fieldPtr) = fieldValue.AsSByte(); break;
        case ValueType::byteType: *static_cast<uint8_t*>(fieldPtr) = fieldValue.AsByte(); break;
        case ValueType::shortType: *static_cast<int16_t*>(fieldPtr) = fieldValue.AsShort(); break;
        case ValueType::ushortType: *static_cast<uint16_t*>(fieldPtr) = fieldValue.AsUShort(); break;
        case ValueType::intType: *static_cast<int32_t*>(fieldPtr) = fieldValue.AsInt(); break;
        case ValueType::uintType: *static_cast<uint32_t*>(fieldPtr) = fieldValue.AsUInt(); break;
        case ValueType::longType: *static_cast<int64_t*>(fieldPtr) = fieldValue.AsLong(); break;
        case ValueType::ulongType: *static_cast<uint64_t*>(fieldPtr) = fieldValue.AsULong(); break;
        case ValueType::floatType: *static_cast<float*>(fieldPtr) = fieldValue.AsFloat(); break;
        case ValueType::doubleType: *static_cast<double*>(fieldPtr) = fieldValue.AsDouble(); break;
        case ValueType::charType: *static_cast<char32_t*>(fieldPtr) = fieldValue.AsChar(); break;
        case ValueType::memPtr: *static_cast<uint8_t**>(fieldPtr) = fieldValue.AsMemPtr(); break;
        case ValueType::classDataPtr: *static_cast<ClassData**>(fieldPtr) = fieldValue.AsClassDataPtr(); break;
        case ValueType::typePtr: *static_cast<Type**>(fieldPtr) = fieldValue.AsTypePtr(); break;
        case ValueType::stringLiteral: *static_cast<const char32_t**>(fieldPtr) = fieldValue.AsStringLiteral(); break;
        case ValueType::allocationHandle: *static_cast<uint64_t*>(fieldPtr) = fieldValue.Value(); break;
        case ValueType::objectReference: *static_cast<uint64_t*>(fieldPtr) = fieldValue.Value(); break;
        default: throw std::runtime_error("invalid field type " + std::to_string(int(field.GetType())));
    }
}

IntegralValue StaticClassData::GetStaticField(int32_t index) const
{
    Field& field = staticLayout.GetField(index);
    void* fieldPtr = static_cast<uint8_t*>(staticData) + field.Offset().Value();
    switch (field.GetType())
    {
        case ValueType::boolType: return IntegralValue(*static_cast<bool*>(fieldPtr), field.GetType());
        case ValueType::sbyteType: return IntegralValue(*static_cast<int8_t*>(fieldPtr), field.GetType());
        case ValueType::byteType: return IntegralValue(*static_cast<uint8_t*>(fieldPtr), field.GetType());
        case ValueType::shortType: return IntegralValue(*static_cast<int16_t*>(fieldPtr), field.GetType());
        case ValueType::ushortType: return IntegralValue(*static_cast<uint16_t*>(fieldPtr), field.GetType());
        case ValueType::intType: return IntegralValue(*static_cast<int32_t*>(fieldPtr), field.GetType());
        case ValueType::uintType: return IntegralValue(*static_cast<uint32_t*>(fieldPtr), field.GetType());
        case ValueType::longType: return IntegralValue(*static_cast<int64_t*>(fieldPtr), field.GetType());
        case ValueType::ulongType: return IntegralValue(*static_cast<uint64_t*>(fieldPtr), field.GetType());
        case ValueType::floatType: return IntegralValue(static_cast<uint64_t>(*static_cast<float*>(fieldPtr)), field.GetType());
        case ValueType::doubleType: return IntegralValue(static_cast<uint64_t>(*static_cast<double*>(fieldPtr)), field.GetType());
        case ValueType::charType: return IntegralValue(*static_cast<char32_t*>(fieldPtr), field.GetType());
        case ValueType::memPtr: return IntegralValue(static_cast<uint8_t*>(fieldPtr));
        case ValueType::classDataPtr: return IntegralValue(*static_cast<ClassData**>(fieldPtr));
        case ValueType::typePtr: return IntegralValue(*static_cast<ObjectType**>(fieldPtr));
        case ValueType::stringLiteral: return IntegralValue(static_cast<const char32_t*>(fieldPtr));
        case ValueType::allocationHandle: return IntegralValue(*static_cast<uint64_t*>(fieldPtr), field.GetType());
        case ValueType::objectReference: return IntegralValue(*static_cast<uint64_t*>(fieldPtr), field.GetType());
        default: return IntegralValue(*static_cast<uint64_t*>(fieldPtr), field.GetType());
    }
}

ClassData::ClassData(ObjectType* type_) : type(type_)
{
}

void ClassData::Write(Writer& writer)
{
    vmt.Write(writer);
    uint32_t n = uint32_t(imts.size());
    writer.PutEncodedUInt(n);
    for (uint32_t i = 0; i < n; ++i)
    {
        imts[i].Write(writer);
    }
    bool hasStaticClassData = staticClassData != nullptr;
    writer.Put(hasStaticClassData);
    if (hasStaticClassData)
    {
        staticClassData->Write(writer);
    }
}

void ClassData::Read(Reader& reader)
{
    vmt.Read(reader);
    uint32_t n = reader.GetEncodedUInt();
    AllocImts(n);
    for (uint32_t i = 0; i < n; ++i)
    {
        imts[i].Read(reader);
    }
    bool hasStaticClassData = reader.GetBool();
    if (hasStaticClassData)
    {
        CreateStaticClassData();
        staticClassData->Read(reader);
    }
}

MethodTable& ClassData::Imt(int32_t index)
{
    Assert(index >= 0 && index < int32_t(imts.size()), "invalid itable index");
    return imts[index];
}

void ClassData::AllocImts(int32_t numInterfaces)
{
    imts.resize(numInterfaces);
}

void ClassData::CreateStaticClassData()
{
    staticClassData.reset(new StaticClassData());
}

class ClassDataTableImpl
{
public:
    static void Init();
    static void Done();
    static ClassDataTableImpl& Instance();
    ClassData* GetClassData(StringPtr fullClassName);
    ClassData* GetSystemStringClassData();
    void SetClassData(ClassData* classData);
    std::unordered_map<StringPtr, ClassData*, StringPtrHash>& ClassDataMap() { return classDataMap; }
private:
    static std::unique_ptr<ClassDataTableImpl> instance;
    ClassDataTableImpl();
    std::unordered_map<StringPtr, ClassData*, StringPtrHash> classDataMap;
    ClassData* systemStringClassData;
};

std::unique_ptr<ClassDataTableImpl> ClassDataTableImpl::instance;

ClassDataTableImpl::ClassDataTableImpl() : systemStringClassData(nullptr)
{
}

void ClassDataTableImpl::Init()
{
    instance.reset(new ClassDataTableImpl());
}

void ClassDataTableImpl::Done()
{
    instance.reset();
}

ClassDataTableImpl& ClassDataTableImpl::Instance()
{
    Assert(instance, "class data table instance not set");
    return *instance;
}

ClassData* ClassDataTableImpl::GetClassData(StringPtr fullClassName)
{
    auto it = classDataMap.find(fullClassName);
    if (it != classDataMap.cend())
    {
        return it->second;
    }
    else
    {
        throw std::runtime_error("class data for class name '" + ToUtf8(fullClassName.Value()) + "' not found");
    }
}

ClassData* ClassDataTableImpl::GetSystemStringClassData()
{
    if (!systemStringClassData)
    {
        systemStringClassData = GetClassData(StringPtr(U"System.String"));
    }
    return systemStringClassData;
}

void ClassDataTableImpl::SetClassData(ClassData* classData)
{
    auto it = classDataMap.find(classData->Type()->Name());
    if (it != classDataMap.cend())
    {
        throw std::runtime_error("class data with class name '" + ToUtf8(classData->Type()->Name().Value()) + "' already in use");
    }
    classDataMap[classData->Type()->Name()] = classData;
}

MACHINE_API void ClassDataTable::Init()
{
    ClassDataTableImpl::Init();
}

MACHINE_API void ClassDataTable::Done()
{
    ClassDataTableImpl::Done();
}

MACHINE_API ClassData* ClassDataTable::GetClassData(StringPtr fullClassName)
{
    return ClassDataTableImpl::Instance().GetClassData(fullClassName);
}

MACHINE_API ClassData* ClassDataTable::GetSystemStringClassData()
{
    return ClassDataTableImpl::Instance().GetSystemStringClassData();
}

MACHINE_API void ClassDataTable::SetClassData(ClassData* classData)
{
    ClassDataTableImpl::Instance().SetClassData(classData);
}

MACHINE_API std::unordered_map<StringPtr, ClassData*, StringPtrHash>& ClassDataTable::ClassDataMap()
{
    return ClassDataTableImpl::Instance().ClassDataMap();
}

MACHINE_API ClassData* GetClassDataForBoxedType(ValueType valueType)
{
    switch (valueType)
    {
        case cminor::machine::ValueType::boolType: return ClassDataTable::GetClassData(U"System.BoxedBoolean");
        case cminor::machine::ValueType::sbyteType: return ClassDataTable::GetClassData(U"System.BoxedInt8");
        case cminor::machine::ValueType::byteType: return ClassDataTable::GetClassData(U"System.BoxedUInt8");
        case cminor::machine::ValueType::shortType: return ClassDataTable::GetClassData(U"System.BoxedInt16");
        case cminor::machine::ValueType::ushortType: return ClassDataTable::GetClassData(U"System.BoxedUInt16");
        case cminor::machine::ValueType::intType: return ClassDataTable::GetClassData(U"System.BoxedInt32");
        case cminor::machine::ValueType::uintType: return ClassDataTable::GetClassData(U"System.BoxedUInt32");
        case cminor::machine::ValueType::longType: return ClassDataTable::GetClassData(U"System.BoxedInt64");
        case cminor::machine::ValueType::ulongType: return ClassDataTable::GetClassData(U"System.BoxedUInt64");
        case cminor::machine::ValueType::floatType: return ClassDataTable::GetClassData(U"System.BoxedFloat");
        case cminor::machine::ValueType::doubleType: return ClassDataTable::GetClassData(U"System.BoxedDouble");
        case cminor::machine::ValueType::charType: return ClassDataTable::GetClassData(U"System.BoxedChar");
    }
    Assert(false, "unknown value type");
    return nullptr;
}

} } // namespace cminor::machine
