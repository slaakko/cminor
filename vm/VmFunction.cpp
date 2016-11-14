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
    StrLitToStringInst strLit2StringInst;
    const char32_t* nameLiteral = name.Value();
    frame.OpStack().Push(IntegralValue(nameLiteral));
    strLit2StringInst.Execute(frame);
}

class VmSystemObjectEqual : public VmFunction
{
public:
    VmSystemObjectEqual(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemObjectEqual::VmSystemObjectEqual(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.Object.Equal"));
    SetName(name);
    VmFunctionTable::Instance().RegisterVmFunction(this);
}

void VmSystemObjectEqual::Execute(Frame& frame)
{
    IntegralValue leftValue = frame.Local(0).GetValue();
    Assert(leftValue.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference leftRef(leftValue.Value());
    IntegralValue rightValue = frame.Local(1).GetValue();
    Assert(rightValue.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference rightRef(rightValue.Value());
    bool result = false;
    if (leftRef.IsNull() && rightRef.IsNull())
    {
        result = true;
    }
    else if (leftRef.IsNull() || rightRef.IsNull())
    {
        result = false;
    }
    else
    {
        Object& left = frame.GetManagedMemoryPool().GetObject(leftRef);
        MemPtr leftMem = left.GetMemPtr();
        Object& right = frame.GetManagedMemoryPool().GetObject(rightRef);
        MemPtr rightMem = right.GetMemPtr();
        result = leftMem == rightMem;
    }
    frame.OpStack().Push(IntegralValue(result, ValueType::boolType));
}

class VmSystemObjectLess : public VmFunction
{
public:
    VmSystemObjectLess(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemObjectLess::VmSystemObjectLess(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.Object.Less"));
    SetName(name);
    VmFunctionTable::Instance().RegisterVmFunction(this);
}

void VmSystemObjectLess::Execute(Frame& frame)
{
    IntegralValue leftValue = frame.Local(0).GetValue();
    Assert(leftValue.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference leftRef(leftValue.Value());
    IntegralValue rightValue = frame.Local(1).GetValue();
    Assert(rightValue.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference rightRef(rightValue.Value());
    bool result = false;
    if (leftRef.IsNull() && rightRef.IsNull())
    {
        result = false;
    }
    else if (leftRef.IsNull())
    {
        result = true;
    }
    else if (rightRef.IsNull())
    {
        result = false;
    }
    else
    {
        Object& left = frame.GetManagedMemoryPool().GetObject(leftRef);
        MemPtr leftMem = left.GetMemPtr();
        Object& right = frame.GetManagedMemoryPool().GetObject(rightRef);
        MemPtr rightMem = right.GetMemPtr();
        result = leftMem < rightMem;
    }
    frame.OpStack().Push(IntegralValue(result, ValueType::boolType));
}

class VmSystemStringConstructorCharArray : public VmFunction
{
public:
    VmSystemStringConstructorCharArray(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemStringConstructorCharArray::VmSystemStringConstructorCharArray(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"System.String.Constructor.CharArray"));
    SetName(name);
    VmFunctionTable::Instance().RegisterVmFunction(this);
}

void VmSystemStringConstructorCharArray::Execute(Frame& frame)
{
    IntegralValue stringValue = frame.Local(0).GetValue();
    Assert(stringValue.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference stringRef(stringValue.Value());
    IntegralValue charArrayValue = frame.Local(1).GetValue();
    Assert(charArrayValue.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference charArrayRef(charArrayValue.Value());
    std::pair<AllocationHandle, int32_t> stringCharactersHandleStringSLengthPair = frame.GetManagedMemoryPool().CreateStringCharsFromCharArray(frame.GetThread(), charArrayRef);
    AllocationHandle stringCharactersHandle = stringCharactersHandleStringSLengthPair.first;
    int32_t stringLength = stringCharactersHandleStringSLengthPair.second;
    Object& str = frame.GetManagedMemoryPool().GetObject(stringRef);
    str.SetField(IntegralValue(stringLength, ValueType::intType), 1);
    str.SetField(IntegralValue(stringCharactersHandle.Value(), ValueType::allocationHandle), 2);
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
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemObjectEqual(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemStringConstructorCharArray(constantPool)));
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
