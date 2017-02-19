// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_CLASS_INCLUDED
#define CMINOR_MACHINE_CLASS_INCLUDED
#include <cminor/machine/MachineApi.hpp>
#include <cminor/machine/Function.hpp>
#include <mutex>

namespace cminor { namespace machine {

class ObjectType;

class MACHINE_API MethodTable
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

class MACHINE_API StaticClassData
{
public:
    StaticClassData();
    void Write(Writer& writer);
    void Read(Reader& reader);
    bool Initialized() const { return initialized; }
    bool Initializing() const { return initializing; }
    void SetInitializing() { initializing = true; }
    void ResetInitializing() { initializing = false; }
    void SetInitialized() { initialized = true; }
    void Lock() { initMtx.lock(); }
    void Unlock() { initMtx.unlock(); }
    void SetStaticConstructorName(Constant staticConstructorName_);
    Constant StaticConstructorName() const { return staticConstructorName; }
    Layout& StaticLayout() { return staticLayout; }
    const Layout& StaticLayout() const { return staticLayout; }
    void AllocateStaticData();
    bool HasStaticData() const { return staticData.Value() != nullptr; }
    void SetStaticField(IntegralValue fieldValue, int32_t index);
    IntegralValue GetStaticField(int32_t index) const;
private:
    std::atomic_bool initialized;
    std::atomic_bool initializing;
    std::recursive_mutex initMtx;
    Constant staticConstructorName;
    Layout staticLayout;
    MemPtr staticData;
};

class MACHINE_API ClassData
{
public:
    ClassData(ObjectType* type_);
    void Write(Writer& writer);
    void Read(Reader& reader);
    ObjectType* Type() const { return type; }
    void SetType(ObjectType* type_) { type = type_; }
    MethodTable& Vmt() { return vmt; }
    MethodTable& Imt(int32_t index);
    void AllocImts(int32_t numInterfaces);
    void CreateStaticClassData();
    StaticClassData* GetStaticClassData() const { return staticClassData.get(); }
private:
    ObjectType* type;
    MethodTable vmt;
    std::vector<MethodTable> imts;
    std::unique_ptr<StaticClassData> staticClassData;
};

class ClassDataTable
{
public:
    MACHINE_API static void Init();
    MACHINE_API static void Done();
    MACHINE_API static ClassData* GetClassData(StringPtr fullClassName);
    MACHINE_API static ClassData* GetSystemStringClassData();
    MACHINE_API static void SetClassData(ClassData* classData);
    MACHINE_API static std::unordered_map<StringPtr, ClassData*, StringPtrHash>& ClassDataMap();
};

MACHINE_API ClassData* GetClassDataForBoxedType(ValueType valueType);

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_CLASS_INCLUDED
