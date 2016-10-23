// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_CLASS_INCLUDED
#define CMINOR_MACHINE_CLASS_INCLUDED
#include <cminor/machine/Function.hpp>

namespace cminor { namespace machine {

class ObjectType;

class MethodTable
{
public:
    int32_t Count() const { return int32_t(methods.size()); }
    void Resize(int32_t n);
    StringPtr GetMethodName(int32_t index) const;
    void SetMethodName(int32_t index, Constant methodNameConstant);
    Function* GetMethod(int32_t index) const;
    void SetMethod(int32_t index, Function* function);
    void Write(Writer& writer);
    void Read(Reader& reader);
private:
    std::vector<Constant> methods;
};

class ClassData
{
public:
    ClassData(ObjectType* type_);
    void Write(Writer& writer);
    void Read(Reader& reader);
    ObjectType* Type() const { return type; }
    MethodTable& Vmt() { return vmt; }
private:
    ObjectType* type;
    MethodTable vmt;
};

class ClassDataTable
{
public:
    static void Init();
    static void Done();
    static ClassDataTable& Instance();
    ClassData* GetClassData(StringPtr fullClassName);
    void SetClassData(ClassData* classData);
private:
    static std::unique_ptr<ClassDataTable> instance;
    ClassDataTable();
    std::unordered_map<StringPtr, ClassData*, StringPtrHash> classDataMap;
};

class ObjectTypeTable
{
public:
    static void Init();
    static void Done();
    static ObjectTypeTable& Instance();
    ObjectType* GetObjectType(StringPtr fullClassName);
    void SetObjectType(ObjectType* objectType);
private:
    static std::unique_ptr<ObjectTypeTable> instance;
    ObjectTypeTable();
    std::unordered_map<StringPtr, ObjectType*, StringPtrHash> objectTypeMap;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_CLASS_INCLUDED
