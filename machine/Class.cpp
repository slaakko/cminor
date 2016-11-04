// =================================
// Copyright (c) 2016 Seppo Laakko
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
    int32_t n = int32_t(methods.size());
    writer.Put(n);
    for (int32_t i = 0; i < n; ++i)
    {
        Constant method = methods[i];
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
    int32_t n = reader.GetInt();
    for (int32_t i = 0; i < n; ++i)
    {
        ConstantId constantId;
        constantId.Read(reader);
        Constant method = reader.GetConstantPool()->GetConstant(constantId);
        methods.push_back(method);
    }
}

ClassData::ClassData(ObjectType* type_) : type(type_)
{
}

void ClassData::Write(Writer& writer)
{
    vmt.Write(writer);
    int32_t n = int32_t(imts.size());
    writer.Put(n);
    for (int32_t i = 0; i < n; ++i)
    {
        imts[i].Write(writer);
    }
}

void ClassData::Read(Reader& reader)
{
    vmt.Read(reader);
    int32_t n = reader.GetInt();
    AllocImts(n);
    for (int32_t i = 0; i < n; ++i)
    {
        imts[i].Read(reader);
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

std::unique_ptr<ClassDataTable> ClassDataTable::instance;

ClassDataTable::ClassDataTable()
{
}

void ClassDataTable::Init()
{
    instance.reset(new ClassDataTable());
}

void ClassDataTable::Done()
{
    instance.reset();
}

ClassDataTable& ClassDataTable::Instance()
{
    Assert(instance, "class data table instance not set");
    return *instance;
}

ClassData* ClassDataTable::GetClassData(StringPtr fullClassName)
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

void ClassDataTable::SetClassData(ClassData* classData)
{
    auto it = classDataMap.find(classData->Type()->Name());
    if (it != classDataMap.cend())
    {
        throw std::runtime_error("class data with class name '" + ToUtf8(classData->Type()->Name().Value()) + "' already in use");
    }
    classDataMap[classData->Type()->Name()] = classData;
}

} } // namespace cminor::machine
