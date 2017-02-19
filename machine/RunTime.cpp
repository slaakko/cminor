// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/RunTime.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Instruction.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/util/Util.hpp>

namespace cminor { namespace machine {

#ifdef _WIN32
    __declspec(thread) FunctionStackEntry* functionStack = nullptr;
#else
    __thread FunctionStackEntry* functionStack = nullptr;
#endif

extern "C" MACHINE_API void RtEnterFunction(void* functionStackEntry)
{
    FunctionStackEntry* entry = static_cast<FunctionStackEntry*>(functionStackEntry);
    entry->next = functionStack;
    functionStack = entry;
}

extern "C" MACHINE_API void RtLeaveFunction(void* functionStackEntry)
{
    FunctionStackEntry* entry = static_cast<FunctionStackEntry*>(functionStackEntry);
    functionStack = entry->next;
}

extern "C" MACHINE_API int8_t RtLoadFieldSb(uint64_t objectReference, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
    return fieldValue.AsSByte();
}

extern "C" MACHINE_API uint8_t RtLoadFieldBy(uint64_t objectReference, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
    return fieldValue.AsByte();
}

extern "C" MACHINE_API int16_t RtLoadFieldSh(uint64_t objectReference, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
    return fieldValue.AsShort();
}

extern "C" MACHINE_API uint16_t RtLoadFieldUs(uint64_t objectReference, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
    return fieldValue.AsUShort();
}

extern "C" MACHINE_API int32_t RtLoadFieldIn(uint64_t objectReference, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
    return fieldValue.AsInt();
}

extern "C" MACHINE_API uint32_t RtLoadFieldUi(uint64_t objectReference, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
    return fieldValue.AsUInt();
}

extern "C" MACHINE_API int64_t RtLoadFieldLo(uint64_t objectReference, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
    return fieldValue.AsLong();
}

extern "C" MACHINE_API uint64_t RtLoadFieldUl(uint64_t objectReference, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
    return fieldValue.AsULong();
}

extern "C" MACHINE_API float RtLoadFieldFl(uint64_t objectReference, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
    return fieldValue.AsFloat();
}

extern "C" MACHINE_API double RtLoadFieldDo(uint64_t objectReference, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
    return fieldValue.AsDouble();
}

extern "C" MACHINE_API uint32_t RtLoadFieldCh(uint64_t objectReference, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
    return fieldValue.AsChar();
}

extern "C" MACHINE_API bool RtLoadFieldBo(uint64_t objectReference, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
    return fieldValue.AsBool();
}

extern "C" MACHINE_API uint64_t RtLoadFieldOb(uint64_t objectReference, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
    return fieldValue.AsULong();
}

extern "C" MACHINE_API void RtStoreFieldSb(uint64_t objectReference, int8_t fieldValue, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::sbyteType));
}

extern "C" MACHINE_API void RtStoreFieldBy(uint64_t objectReference, uint8_t fieldValue, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::byteType));
}

extern "C" MACHINE_API void RtStoreFieldSh(uint64_t objectReference, int16_t fieldValue, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::shortType));
}

extern "C" MACHINE_API void RtStoreFieldUs(uint64_t objectReference, uint16_t fieldValue, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::ushortType));
}

extern "C" MACHINE_API void RtStoreFieldIn(uint64_t objectReference, int32_t fieldValue, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::intType));
}

extern "C" MACHINE_API void RtStoreFieldUi(uint64_t objectReference, uint32_t fieldValue, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::uintType));
}

extern "C" MACHINE_API void RtStoreFieldLo(uint64_t objectReference, int64_t fieldValue, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::longType));
}

extern "C" MACHINE_API void RtStoreFieldUl(uint64_t objectReference, uint64_t fieldValue, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::ulongType));
}

extern "C" MACHINE_API void RtStoreFieldFl(uint64_t objectReference, float fieldValue, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::floatType));
}

extern "C" MACHINE_API void RtStoreFieldDo(uint64_t objectReference, double fieldValue, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::doubleType));
}

extern "C" MACHINE_API void RtStoreFieldCh(uint64_t objectReference, uint32_t fieldValue, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::charType));
}

extern "C" MACHINE_API void RtStoreFieldBo(uint64_t objectReference, bool fieldValue, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::boolType));
}

extern "C" MACHINE_API void RtStoreFieldOb(uint64_t objectReference, uint64_t fieldValue, int32_t fieldIndex)
{
    ObjectReference reference(objectReference);
    GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::objectReference));
}

extern "C" MACHINE_API int8_t RtLoadElemSb(uint64_t arrayReference, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
    return elementValue.AsSByte();
}

extern "C" MACHINE_API uint8_t RtLoadElemBy(uint64_t arrayReference, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
    return elementValue.AsByte();
}

extern "C" MACHINE_API int16_t RtLoadElemSh(uint64_t arrayReference, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
    return elementValue.AsShort();
}

extern "C" MACHINE_API uint16_t RtLoadElemUs(uint64_t arrayReference, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
    return elementValue.AsUShort();
}

extern "C" MACHINE_API int32_t RtLoadElemIn(uint64_t arrayReference, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
    return elementValue.AsInt();
}

extern "C" MACHINE_API uint32_t RtLoadElemUi(uint64_t arrayReference, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
    return elementValue.AsUInt();
}

extern "C" MACHINE_API int64_t RtLoadElemLo(uint64_t arrayReference, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
    return elementValue.AsLong();
}

extern "C" MACHINE_API uint64_t RtLoadElemUl(uint64_t arrayReference, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
    return elementValue.AsULong();
}

extern "C" MACHINE_API float RtLoadElemFl(uint64_t arrayReference, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
    return elementValue.AsFloat();
}

extern "C" MACHINE_API double RtLoadElemDo(uint64_t arrayReference, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
    return elementValue.AsDouble();
}

extern "C" MACHINE_API uint32_t RtLoadElemCh(uint64_t arrayReference, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
    return elementValue.AsChar();
}

extern "C" MACHINE_API bool RtLoadElemBo(uint64_t arrayReference, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
    return elementValue.AsBool();
}

extern "C" MACHINE_API uint64_t RtLoadElemOb(uint64_t arrayReference, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
    return elementValue.AsULong();
}

extern "C" MACHINE_API void RtStoreElemSb(uint64_t arrayReference, int8_t elementValue, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue value(elementValue, ValueType::sbyteType);
    GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
}

extern "C" MACHINE_API void RtStoreElemBy(uint64_t arrayReference, uint8_t elementValue, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue value(elementValue, ValueType::byteType);
    GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
}

extern "C" MACHINE_API void RtStoreElemSh(uint64_t arrayReference, int16_t elementValue, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue value(elementValue, ValueType::shortType);
    GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
}

extern "C" MACHINE_API void RtStoreElemUs(uint64_t arrayReference, uint16_t elementValue, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue value(elementValue, ValueType::ushortType);
    GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
}

extern "C" MACHINE_API void RtStoreElemIn(uint64_t arrayReference, int32_t elementValue, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue value(elementValue, ValueType::intType);
    GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
}

extern "C" MACHINE_API void RtStoreElemUi(uint64_t arrayReference, uint32_t elementValue, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue value(elementValue, ValueType::uintType);
    GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
}

extern "C" MACHINE_API void RtStoreElemLo(uint64_t arrayReference, int64_t elementValue, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue value(elementValue, ValueType::longType);
    GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
}

extern "C" MACHINE_API void RtStoreElemUl(uint64_t arrayReference, uint64_t elementValue, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue value(elementValue, ValueType::ulongType);
    GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
}

extern "C" MACHINE_API void RtStoreElemFl(uint64_t arrayReference, float elementValue, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue value(elementValue, ValueType::floatType);
    GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
}

extern "C" MACHINE_API void RtStoreElemDo(uint64_t arrayReference, double elementValue, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue value(elementValue, ValueType::doubleType);
    GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
}

extern "C" MACHINE_API void RtStoreElemCh(uint64_t arrayReference, uint32_t elementValue, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue value(elementValue, ValueType::charType);
    GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
}

extern "C" MACHINE_API void RtStoreElemBo(uint64_t arrayReference, bool elementValue, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue value(elementValue, ValueType::boolType);
    GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
}

extern "C" MACHINE_API void RtStoreElemOb(uint64_t arrayReference, uint64_t elementValue, int32_t elementIndex)
{
    ObjectReference reference(arrayReference);
    IntegralValue value(elementValue, ValueType::objectReference);
    GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
}

extern "C" MACHINE_API void* RtResolveVirtualFunctionCallAddress(uint64_t objectReference, uint32_t vmtIndex)
{
    ObjectReference receiver(objectReference);
    IntegralValue classDataField = GetManagedMemoryPool().GetField(receiver, 0);
    ClassData* classData = classDataField.AsClassDataPtr();
    if (classData)
    {
        Function* method = classData->Vmt().GetMethod(vmtIndex);
        if (method)
        {
            return method->Address();
        }
    }
    return nullptr;
}

extern "C" MACHINE_API void* RtResolveInterfaceCallAddress(uint64_t objectReference, uint32_t imtIndex, uint64_t* receiver)
{
    ObjectReference interfaceObject(objectReference);
    IntegralValue receiverField = GetManagedMemoryPool().GetField(interfaceObject, 0);
    ObjectReference receiverObject(receiverField.Value());
    *receiver = receiverObject.Value();
    IntegralValue classDataField = GetManagedMemoryPool().GetField(receiverObject, 0);
    ClassData* classData = classDataField.AsClassDataPtr();
    if (classData)
    {
        IntegralValue itabIndex = GetManagedMemoryPool().GetField(interfaceObject, 1);
        MethodTable& imt = classData->Imt(itabIndex.AsInt());
        Function* method = imt.GetMethod(imtIndex);
        if (method)
        {
            return method->Address();
        }
    }
    return nullptr;
}

extern "C" MACHINE_API void RtVmCall(void* function, void* constantPool, uint32_t vmFunctionNameId, VmCallContext* vmCallContext)
{
    Function* fun = static_cast<Function*>(function);
    Thread& thread = GetCurrentThread();
    int numLocals = vmCallContext->numLocals;
    uint64_t frameSize = Align(sizeof(Frame), 8) + numLocals * Align(sizeof(LocalVariable), 8);
    std::unique_ptr<uint8_t> frameMem(new uint8_t[frameSize]);
    Frame* frame = new (frameMem.get()) Frame(frameSize, thread, *fun);
    for (int i = 0; i < numLocals; ++i)
    {
        IntegralValue value(vmCallContext->locals[i], ValueType(vmCallContext->localTypes[i]));
        frame->Local(i).SetValue(value);
    }
    ConstantPool* cpool = static_cast<ConstantPool*>(constantPool);
    ConstantId vmFunctionNameConstantId(vmFunctionNameId);
    Constant vmFunctionNameConstant = cpool->GetConstant(vmFunctionNameId);
    VmFunction* vmFunction = VmFunctionTable::GetVmFunction(StringPtr(vmFunctionNameConstant.Value().AsStringLiteral()));
    vmFunction->Execute(*frame);
    if (vmCallContext->retValType != uint8_t(ValueType::none))
    {
        IntegralValue returnValue = frame->OpStack().Pop();
        vmCallContext->retVal = returnValue.Value();
    }
}

extern "C" MACHINE_API void* RtResolveDelegateCallAddress(void* function)
{
    Function* fun = static_cast<Function*>(function);
    if (fun)
    {
        return fun->Address();
    }
    return nullptr;
}

extern "C" MACHINE_API void* RtResolveClassDelegateCallAddress(uint64_t classDlg, uint64_t* classObject)
{
    ObjectReference classDelegateRef(classDlg);
    Object& classDelegateObject = GetManagedMemoryPool().GetObject(classDelegateRef);
    IntegralValue classObjectValue = classDelegateObject.GetField(1);
    *classObject = classObjectValue.Value();
    IntegralValue funValue = classDelegateObject.GetField(2);
    Function* function = funValue.AsFunctionPtr();
    if (function)
    {
        return function->Address();
    }
    return nullptr;
}

extern "C" MACHINE_API void RtSetClassDataPtr(uint64_t objectReference, void* classDataPtr)
{
    ObjectReference reference(objectReference);
    IntegralValue classDataFieldValue = GetManagedMemoryPool().GetField(reference, 0);
    if (!classDataFieldValue.AsClassDataPtr())
    {
        ClassData* cd = static_cast<ClassData*>(classDataPtr);
        IntegralValue classDataFieldValue = IntegralValue(cd);
        GetManagedMemoryPool().SetField(reference, 0, classDataFieldValue);
    }
}

extern "C" MACHINE_API uint64_t RtCreateObject(void* classDataPtr)
{
    ClassData* cd = static_cast<ClassData*>(classDataPtr);
    ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), cd->Type());
    return objectReference.Value();
}

extern "C" MACHINE_API uint64_t RtCopyObject(uint64_t objectReference)
{
    ObjectReference reference(objectReference);
    ObjectReference copy = GetManagedMemoryPool().CopyObject(GetCurrentThread(), reference);
    return copy.Value();
}

extern "C" MACHINE_API uint64_t RtStrLitToString(const char32_t* strLitValue)
{
    uint32_t len = static_cast<uint32_t>(StringLen(strLitValue));
    ClassData* classData = ClassDataTable::GetSystemStringClassData();
    ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), classData->Type());
    Object& o = GetManagedMemoryPool().GetObject(objectReference);
    o.Pin();
    AllocationHandle charsHandle = GetManagedMemoryPool().CreateStringCharsFromLiteral(GetCurrentThread(), strLitValue, len);
    o.SetField(IntegralValue(classData), 0);
    o.SetField(IntegralValue(static_cast<int32_t>(len), ValueType::intType), 1);
    o.SetField(charsHandle, 2);
    o.Unpin();
    return objectReference.Value();
}

extern "C" MACHINE_API uint32_t RtLoadStringChar(int32_t index, uint64_t str)
{
    ObjectReference strReference(str);
    IntegralValue value = GetManagedMemoryPool().GetStringChar(strReference, index);
    return value.AsUInt();
}

extern "C" MACHINE_API uint64_t RtDownCast(uint64_t objectReference, void* classDataPtr)
{
    ObjectReference reference(objectReference);
    if (reference.IsNull())
    {
        return reference.Value();
    }
    else
    {
        IntegralValue classDataField = GetManagedMemoryPool().GetField(objectReference, 0);
        ClassData* classData = classDataField.AsClassDataPtr();
        uint64_t sourceTypeId = classData->Type()->Id();
        ClassData* targetClassData = static_cast<ClassData*>(classDataPtr);
        uint64_t targetTypeId = targetClassData->Type()->Id();
        if (sourceTypeId % targetTypeId != 0)
        {
            return 0;
        }
        ObjectReference copy = reference;
        return copy.Value();
    }
}

extern "C" MACHINE_API void* RtStaticInit(void* classDataPtr)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    if (!staticClassData) return nullptr;
    if (staticClassData->Initialized()) return nullptr;
    staticClassData->Lock();
    if (!staticClassData->Initialized() && !staticClassData->Initializing())
    {
        staticClassData->SetInitializing();
        staticClassData->AllocateStaticData();
        StringPtr staticConstructorName = staticClassData->StaticConstructorName().Value().AsStringLiteral();
        if (staticConstructorName.Value())
        {
            Function* staticConstructor = FunctionTable::GetFunction(staticConstructorName);
            return staticConstructor->Address();
        }
        else
        {
            staticClassData->ResetInitializing();
            staticClassData->SetInitialized();
            staticClassData->Unlock();
        }
    }
    else
    {
        staticClassData->Unlock();
    }
    return nullptr;
}

extern "C" MACHINE_API void RtDoneStaticInit(void* classDataPtr)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    staticClassData->ResetInitializing();
    staticClassData->SetInitialized();
    staticClassData->Unlock();
}

extern "C" MACHINE_API int8_t RtLoadStaticFieldSb(void* classDataPtr, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value = staticClassData->GetStaticField(fieldIndex);
    return value.AsSByte();
}

extern "C" MACHINE_API uint8_t RtLoadStaticFieldBy(void* classDataPtr, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value = staticClassData->GetStaticField(fieldIndex);
    return value.AsByte();
}

extern "C" MACHINE_API int16_t RtLoadStaticFieldSh(void* classDataPtr, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value = staticClassData->GetStaticField(fieldIndex);
    return value.AsShort();
}

extern "C" MACHINE_API uint16_t RtLoadStaticFieldUs(void* classDataPtr, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value = staticClassData->GetStaticField(fieldIndex);
    return value.AsUShort();
}

extern "C" MACHINE_API int32_t RtLoadStaticFieldIn(void* classDataPtr, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value = staticClassData->GetStaticField(fieldIndex);
    return value.AsInt();
}

extern "C" MACHINE_API uint32_t RtLoadStaticFieldUi(void* classDataPtr, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value = staticClassData->GetStaticField(fieldIndex);
    return value.AsUInt();
}

extern "C" MACHINE_API int64_t RtLoadStaticFieldLo(void* classDataPtr, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value = staticClassData->GetStaticField(fieldIndex);
    return value.AsLong();
}

extern "C" MACHINE_API uint64_t RtLoadStaticFieldUl(void* classDataPtr, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value = staticClassData->GetStaticField(fieldIndex);
    return value.AsULong();
}

extern "C" MACHINE_API float RtLoadStaticFieldFl(void* classDataPtr, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value = staticClassData->GetStaticField(fieldIndex);
    return value.AsFloat();
}

extern "C" MACHINE_API double RtLoadStaticFieldDo(void* classDataPtr, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value = staticClassData->GetStaticField(fieldIndex);
    return value.AsDouble();
}

extern "C" MACHINE_API uint32_t RtLoadStaticFieldCh(void* classDataPtr, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value = staticClassData->GetStaticField(fieldIndex);
    return value.AsChar();
}

extern "C" MACHINE_API bool RtLoadStaticFieldBo(void* classDataPtr, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value = staticClassData->GetStaticField(fieldIndex);
    return value.AsBool();
}

extern "C" MACHINE_API uint64_t RtLoadStaticFieldOb(void* classDataPtr, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value = staticClassData->GetStaticField(fieldIndex);
    return value.AsULong();
}

extern "C" MACHINE_API void RtStoreStaticFieldSb(void* classDataPtr, int8_t fieldValue, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value(fieldValue, ValueType::sbyteType);
    staticClassData->SetStaticField(value, fieldIndex);
}

extern "C" MACHINE_API void RtStoreStaticFieldBy(void* classDataPtr, uint8_t fieldValue, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value(fieldValue, ValueType::byteType);
    staticClassData->SetStaticField(value, fieldIndex);
}

extern "C" MACHINE_API void RtStoreStaticFieldSh(void* classDataPtr, int16_t fieldValue, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value(fieldValue, ValueType::shortType);
    staticClassData->SetStaticField(value, fieldIndex);
}

extern "C" MACHINE_API void RtStoreStaticFieldUs(void* classDataPtr, uint16_t fieldValue, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value(fieldValue, ValueType::ushortType);
    staticClassData->SetStaticField(value, fieldIndex);
}

extern "C" MACHINE_API void RtStoreStaticFieldIn(void* classDataPtr, int32_t fieldValue, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value(fieldValue, ValueType::intType);
    staticClassData->SetStaticField(value, fieldIndex);
}

extern "C" MACHINE_API void RtStoreStaticFieldUI(void* classDataPtr, uint32_t fieldValue, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value(fieldValue, ValueType::uintType);
    staticClassData->SetStaticField(value, fieldIndex);
}

extern "C" MACHINE_API void RtStoreStaticFieldLo(void* classDataPtr, int64_t fieldValue, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value(fieldValue, ValueType::longType);
    staticClassData->SetStaticField(value, fieldIndex);
}

extern "C" MACHINE_API void RtStoreStaticFieldUl(void* classDataPtr, uint64_t fieldValue, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value(fieldValue, ValueType::ulongType);
    staticClassData->SetStaticField(value, fieldIndex);
}

extern "C" MACHINE_API void RtStoreStaticFieldFl(void* classDataPtr, float fieldValue, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value(fieldValue, ValueType::floatType);
    staticClassData->SetStaticField(value, fieldIndex);
}

extern "C" MACHINE_API void RtStoreStaticFieldDo(void* classDataPtr, double fieldValue, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value(fieldValue, ValueType::doubleType);
    staticClassData->SetStaticField(value, fieldIndex);
}

extern "C" MACHINE_API void RtStoreStaticFieldCh(void* classDataPtr, uint32_t fieldValue, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value(fieldValue, ValueType::charType);
    staticClassData->SetStaticField(value, fieldIndex);
}

extern "C" MACHINE_API void RtStoreStaticFieldBo(void* classDataPtr, bool fieldValue, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value(fieldValue, ValueType::boolType);
    staticClassData->SetStaticField(value, fieldIndex);
}

extern "C" MACHINE_API void RtStoreStaticFieldOb(void* classDataPtr, uint64_t fieldValue, int32_t fieldIndex)
{
    ClassData* classData = static_cast<ClassData*>(classDataPtr);
    StaticClassData* staticClassData = classData->GetStaticClassData();
    IntegralValue value(fieldValue, ValueType::objectReference);
    staticClassData->SetStaticField(value, fieldIndex);
}

extern "C" MACHINE_API uint64_t RtBoxSb(int8_t value)
{
    ObjectType* objectType = GetBoxedType(ValueType::sbyteType);
    ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
    ClassData* classData = GetClassDataForBoxedType(ValueType::sbyteType);
    IntegralValue classDataValue(classData);
    GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
    IntegralValue val(value, ValueType::sbyteType);
    GetManagedMemoryPool().SetField(objectReference, 1, val);
    return objectReference.Value();
}

extern "C" MACHINE_API uint64_t RtBoxBy(uint8_t value)
{
    ObjectType* objectType = GetBoxedType(ValueType::byteType);
    ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
    ClassData* classData = GetClassDataForBoxedType(ValueType::byteType);
    IntegralValue classDataValue(classData);
    GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
    IntegralValue val(value, ValueType::byteType);
    GetManagedMemoryPool().SetField(objectReference, 1, val);
    return objectReference.Value();
}

extern "C" MACHINE_API uint64_t RtBoxSh(int16_t value)
{
    ObjectType* objectType = GetBoxedType(ValueType::shortType);
    ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
    ClassData* classData = GetClassDataForBoxedType(ValueType::shortType);
    IntegralValue classDataValue(classData);
    GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
    IntegralValue val(value, ValueType::shortType);
    GetManagedMemoryPool().SetField(objectReference, 1, val);
    return objectReference.Value();
}

extern "C" MACHINE_API uint64_t RtBoxUs(uint16_t value)
{
    ObjectType* objectType = GetBoxedType(ValueType::ushortType);
    ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
    ClassData* classData = GetClassDataForBoxedType(ValueType::ushortType);
    IntegralValue classDataValue(classData);
    GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
    IntegralValue val(value, ValueType::ushortType);
    GetManagedMemoryPool().SetField(objectReference, 1, val);
    return objectReference.Value();
}

extern "C" MACHINE_API uint64_t RtBoxIn(int32_t value)
{
    ObjectType* objectType = GetBoxedType(ValueType::intType);
    ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
    ClassData* classData = GetClassDataForBoxedType(ValueType::intType);
    IntegralValue classDataValue(classData);
    GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
    IntegralValue val(value, ValueType::intType);
    GetManagedMemoryPool().SetField(objectReference, 1, val);
    return objectReference.Value();
}

extern "C" MACHINE_API uint64_t RtBoxUi(uint32_t value)
{
    ObjectType* objectType = GetBoxedType(ValueType::uintType);
    ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
    ClassData* classData = GetClassDataForBoxedType(ValueType::uintType);
    IntegralValue classDataValue(classData);
    GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
    IntegralValue val(value, ValueType::uintType);
    GetManagedMemoryPool().SetField(objectReference, 1, val);
    return objectReference.Value();
}

extern "C" MACHINE_API uint64_t RtBoxLo(int64_t value)
{
    ObjectType* objectType = GetBoxedType(ValueType::longType);
    ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
    ClassData* classData = GetClassDataForBoxedType(ValueType::longType);
    IntegralValue classDataValue(classData);
    GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
    IntegralValue val(value, ValueType::longType);
    GetManagedMemoryPool().SetField(objectReference, 1, val);
    return objectReference.Value();
}

extern "C" MACHINE_API uint64_t RtBoxUl(uint64_t value)
{
    ObjectType* objectType = GetBoxedType(ValueType::ulongType);
    ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
    ClassData* classData = GetClassDataForBoxedType(ValueType::ulongType);
    IntegralValue classDataValue(classData);
    GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
    IntegralValue val(value, ValueType::ulongType);
    GetManagedMemoryPool().SetField(objectReference, 1, val);
    return objectReference.Value();
}

extern "C" MACHINE_API uint64_t RtBoxFl(float value)
{
    ObjectType* objectType = GetBoxedType(ValueType::floatType);
    ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
    ClassData* classData = GetClassDataForBoxedType(ValueType::floatType);
    IntegralValue classDataValue(classData);
    GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
    IntegralValue val(value, ValueType::floatType);
    GetManagedMemoryPool().SetField(objectReference, 1, val);
    return objectReference.Value();
}

extern "C" MACHINE_API uint64_t RtBoxDo(double value)
{
    ObjectType* objectType = GetBoxedType(ValueType::doubleType);
    ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
    ClassData* classData = GetClassDataForBoxedType(ValueType::doubleType);
    IntegralValue classDataValue(classData);
    GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
    IntegralValue val(value, ValueType::doubleType);
    GetManagedMemoryPool().SetField(objectReference, 1, val);
    return objectReference.Value();
}

extern "C" MACHINE_API uint64_t RtBoxCh(uint32_t value)
{
    ObjectType* objectType = GetBoxedType(ValueType::charType);
    ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
    ClassData* classData = GetClassDataForBoxedType(ValueType::charType);
    IntegralValue classDataValue(classData);
    GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
    IntegralValue val(value, ValueType::charType);
    GetManagedMemoryPool().SetField(objectReference, 1, val);
    return objectReference.Value();
}

extern "C" MACHINE_API uint64_t RtBoxBo(bool value)
{
    ObjectType* objectType = GetBoxedType(ValueType::boolType);
    ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
    ClassData* classData = GetClassDataForBoxedType(ValueType::boolType);
    IntegralValue classDataValue(classData);
    GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
    IntegralValue val(value, ValueType::boolType);
    GetManagedMemoryPool().SetField(objectReference, 1, val);
    return objectReference.Value();
}

extern "C" MACHINE_API int8_t RtUnboxSb(uint64_t objectReference)
{
    ObjectReference reference(objectReference);
    // todo: null
    IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
    return value.AsSByte();
}

extern "C" MACHINE_API uint8_t RtUnboxBy(uint64_t objectReference)
{
    ObjectReference reference(objectReference);
    // todo: null
    IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
    return value.AsByte();
}

extern "C" MACHINE_API int16_t RtUnboxSh(uint64_t objectReference)
{
    ObjectReference reference(objectReference);
    // todo: null
    IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
    return value.AsShort();
}

extern "C" MACHINE_API uint16_t RtUnboxUs(uint64_t objectReference)
{
    ObjectReference reference(objectReference);
    // todo: null
    IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
    return value.AsUShort();
}

extern "C" MACHINE_API int32_t RtUnboxIn(uint64_t objectReference)
{
    ObjectReference reference(objectReference);
    // todo: null
    IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
    return value.AsInt();
}

extern "C" MACHINE_API uint32_t RtUnboxUi(uint64_t objectReference)
{
    ObjectReference reference(objectReference);
    // todo: null
    IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
    return value.AsUInt();
}

extern "C" MACHINE_API int64_t RtUnboxLo(uint64_t objectReference)
{
    ObjectReference reference(objectReference);
    // todo: null
    IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
    return value.AsLong();
}

extern "C" MACHINE_API uint64_t RtUnboxUl(uint64_t objectReference)
{
    ObjectReference reference(objectReference);
    // todo: null
    IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
    return value.AsULong();
}

extern "C" MACHINE_API float RtUnboxFl(uint64_t objectReference)
{
    ObjectReference reference(objectReference);
    // todo: null
    IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
    return value.AsFloat();
}

extern "C" MACHINE_API double RtUnboxDo(uint64_t objectReference)
{
    ObjectReference reference(objectReference);
    // todo: null
    IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
    return value.AsDouble();
}

extern "C" MACHINE_API uint32_t RtUnboxCh(uint64_t objectReference)
{
    ObjectReference reference(objectReference);
    // todo: null
    IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
    return value.AsChar();
}

extern "C" MACHINE_API bool RtUnboxBo(uint64_t objectReference)
{
    ObjectReference reference(objectReference);
    // todo: null
    IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
    return value.AsBool();
}

extern "C" MACHINE_API void RtAllocateArrayElements(uint64_t arr, int32_t length, void* elementTypePtr)
{
    Type* elementType = static_cast<Type*>(elementTypePtr);
    ObjectReference arrayReference(arr);
    GetManagedMemoryPool().AllocateArrayElements(GetCurrentThread(), arrayReference, elementType, length);
}

extern "C" MACHINE_API bool RtIs(uint64_t objectReference, void* classDataPtr)
{
    ObjectReference reference(objectReference);
    if (reference.IsNull())
    {
        return false;
    }
    IntegralValue classDataField = GetManagedMemoryPool().GetField(reference, 0);
    ClassData* classData = classDataField.AsClassDataPtr();
    uint64_t sourceTypeId = classData->Type()->Id();
    ClassData* targetClassData = static_cast<ClassData*>(classDataPtr);
    uint64_t targetId = targetClassData->Type()->Id();
    bool result = sourceTypeId % targetId == 0;
    return result;
}

extern "C" MACHINE_API uint64_t RtAs(uint64_t objectReference, void* classDataPtr)
{
    ObjectReference reference(objectReference);
    if (reference.IsNull())
    {
        return 0;
    }
    IntegralValue classDataField = GetManagedMemoryPool().GetField(reference, 0);
    ClassData* classData = classDataField.AsClassDataPtr();
    uint64_t sourceTypeId = classData->Type()->Id();
    ClassData* targetClassData = static_cast<ClassData*>(classDataPtr);
    uint64_t targetId = targetClassData->Type()->Id();
    bool result = sourceTypeId % targetId == 0;
    if (result)
    {
        return reference.Value();
    }
    return 0;
}

extern "C" MACHINE_API const char32_t* RtLoadStringLiteral(void* constantPool, uint32_t constantId)
{
    ConstantPool* pool = static_cast<ConstantPool*>(constantPool);
    ConstantId id(constantId);
    Constant constant = pool->GetConstant(id);
    return constant.Value().AsStringLiteral();
}

extern "C" MACHINE_API void RtMemFun2ClassDelegate(uint64_t classObjectRererence, uint64_t classDelegateObjectReference, void* memberFunction)
{
    ObjectReference classDelegateReference(classDelegateObjectReference);
    Object& classDelegateObject = GetManagedMemoryPool().GetObject(classDelegateReference);
    ObjectReference classObjectValue(classObjectRererence);
    classDelegateObject.SetField(classObjectValue, 1);
    Function* memberFun = static_cast<Function*>(memberFunction);
    IntegralValue memberFunctionValue(memberFun);
    classDelegateObject.SetField(memberFunctionValue, 2);
}

} } // namespace cminor::machine
