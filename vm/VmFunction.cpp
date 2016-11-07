// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/vm/VmFunction.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/machine/Class.hpp>
#include <cminor/machine/Type.hpp>

namespace cminor { namespace vm {

class VmSystemObjectToString : public VmFunction
{
public:
    VmSystemObjectToString(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemObjectToString::VmSystemObjectToString(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.Object.ToString"));
    SetName(name);
    VmFunctionTable::Instance().RegisterVmFunction(this);
}

void VmSystemObjectToString::Execute(Frame& frame)
{
    IntegralValue value = frame.Local(0).GetValue();
    Assert(value.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference reference(value.Value());
    IntegralValue classDataValue = frame.GetManagedMemoryPool().GetField(reference, 0);
    Assert(classDataValue.GetType() == ValueType::classDataPtr, "class data pointer expected");
    ClassData* classData = classDataValue.AsClassDataPtr();
    StringPtr name = classData->Type()->Name();
    const char32_t* nameLiteral = name.Value();
    uint64_t len = StringLen(nameLiteral);
    AllocationHandle stringReference = frame.GetManagedMemoryPool().CreateStringFromLiteral(nameLiteral, len);
    frame.OpStack().Push(stringReference);
}

class VmSystemStringLength : public VmFunction
{
public:
    VmSystemStringLength(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemStringLength::VmSystemStringLength(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.String.Length"));
    SetName(name);
    VmFunctionTable::Instance().RegisterVmFunction(this);
}

void VmSystemStringLength::Execute(Frame& frame)
{
    IntegralValue value = frame.Local(0).GetValue();
    Assert(value.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference stringReference(value.Value()); 
    int32_t len = frame.GetManagedMemoryPool().GetStringLength(stringReference);
    frame.OpStack().Push(IntegralValue(len, ValueType::intType));
}

class VmFunctionPool
{
public:
    static void Init();
    static void Done();
    static VmFunctionPool& Instance() { Assert(instance, "vm function pool not initialized"); return *instance; }
    void CreateVmFunctions(ConstantPool& constantPool);
private:
    static std::unique_ptr<VmFunctionPool> instance;
    std::vector<std::unique_ptr<VmFunction>> vmFunctions;
    VmFunctionPool();
};

std::unique_ptr<VmFunctionPool> VmFunctionPool::instance;

VmFunctionPool::VmFunctionPool()
{
}

void VmFunctionPool::Init()
{
    instance.reset(new VmFunctionPool());
}

void VmFunctionPool::Done()
{
    instance.reset();
}

void VmFunctionPool::CreateVmFunctions(ConstantPool& constantPool)
{
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemObjectToString(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemStringLength(constantPool)));
}

void InitVmFunctions(ConstantPool& vmFunctionNamePool) 
{
    VmFunctionPool::Init();
    VmFunctionPool::Instance().CreateVmFunctions(vmFunctionNamePool);
}

void DoneVmFunctions()
{
    VmFunctionPool::Done();
}


} } // namespace cminor::vm
