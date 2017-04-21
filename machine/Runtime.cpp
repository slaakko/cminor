// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Runtime.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Instruction.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/util/Util.hpp>
#include <iostream>

namespace cminor { namespace machine {

bool trace = false;

MACHINE_API void SetTrace()
{
    trace = true;
}

#ifdef _WIN32
    __declspec(thread) FunctionStackEntry* functionStack = nullptr;
#else
    __thread FunctionStackEntry* functionStack = nullptr;
#endif

#ifdef _WIN32
    __declspec(thread) uint64_t currentException = 0;
#else
    __thread uint64_t currentException = 0;
#endif

MACHINE_API FunctionStackEntry* RtGetFunctionStack()
{
    return functionStack;
}

utf32_string GetStackTrace() 
{
    utf32_string stackTrace;
    bool first = true;
    FunctionStackEntry* entry = functionStack;
    while (entry)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            stackTrace.append(1, U'\n');
        }
        utf32_string fun = U"at ";
        Function* function = entry->function;
        fun.append(function->FullName().Value().AsStringLiteral());
        if (function->HasSourceFilePath())
        {
            if (entry->lineNumber != -1)
            {
                fun.append(U" [").append(function->SourceFilePath().Value().AsStringLiteral());
                fun.append(U":").append(ToUtf32(std::to_string(entry->lineNumber))).append(U"]");
            }
        }
        stackTrace.append(fun);
        entry = entry->next;
    }
    return stackTrace;
}

void RtThrowCminorException(const std::string& message, const utf32_string& exceptionTypeName)
{
    Type* type = TypeTable::GetType(StringPtr(exceptionTypeName.c_str()));
    ObjectType* objectType = dynamic_cast<ObjectType*>(type);
    Assert(objectType, "object type expected");
    Thread& thread = GetCurrentThread();
    ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
    std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
    ObjectReference objectReference = memoryPool.CreateObject(thread, objectType, lock);
    currentException = objectReference.Value();
    ClassData* classData = ClassDataTable::GetClassData(StringPtr(exceptionTypeName.c_str()));
    void* object = memoryPool.GetObject(objectReference, lock);
    ManagedAllocationHeader* header = GetAllocationHeader(object);
    header->Reference();
    IntegralValue classDataValue(classData);
    SetObjectField(object, classDataValue, 0);
    ObjectReference messageStr = memoryPool.CreateString(thread, ToUtf32(message), lock);
    SetObjectField(object, messageStr, 1);
    utf32_string stackTrace = GetStackTrace();
    ObjectReference stackTraceStr = memoryPool.CreateString(thread, stackTrace, lock);
    SetObjectField(object, stackTraceStr, 2);
    throw CminorException(objectReference.Value());
}

void RtThrowSystemException(const SystemException& ex)
{
    RtThrowCminorException(ex.Message(), U"System.SystemException");
}

void RtThrowNullReferenceException(const NullReferenceException& ex)
{
    RtThrowCminorException(ex.Message(), U"System.NullReferenceException");
}

void RtThrowIndexOutOfRangeException(const IndexOutOfRangeException& ex)
{
    RtThrowCminorException(ex.Message(), U"System.IndexOutOfRangeException");
}

void RtThrowArgumentOutOfRangeException(const ArgumentOutOfRangeException& ex)
{
    RtThrowCminorException(ex.Message(), U"System.ArgumentOutOfRangeException");
}

void RtThrowInvalidCastException(const InvalidCastException& ex)
{
    RtThrowCminorException(ex.Message(), U"System.InvalidCastException");
}

void RtThrowFileSystemException(const FileSystemError& ex)
{
    RtThrowCminorException(ex.Message(), U"System.FileSystemException");
}

void RtThrowStackOverflowException(const StackOverflowException& ex)
{
    RtThrowCminorException(ex.Message(), U"System.StackOverflowException");
}

void RtThrowThreadingException(const ThreadingException& ex)
{
    RtThrowCminorException(ex.Message(), U"System.Threading.ThreadingException");
}

extern "C" MACHINE_API void RtThrowException(uint64_t exceptionObjectReference)
{
    if (exceptionObjectReference == 0)
    {
        RtThrowSystemException(SystemException("tried to throw null reference"));
        return;
    }
    currentException = exceptionObjectReference;
    ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
    std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
    void* object = memoryPool.GetObject(exceptionObjectReference, lock);
    ManagedAllocationHeader* header = GetAllocationHeader(object);
    header->Reference();
    ObjectReference stackTraceStr = memoryPool.CreateString(GetCurrentThread(), GetStackTrace(), lock);
    SetObjectField(object, stackTraceStr, 2);
    throw CminorException(exceptionObjectReference);
}

extern "C" MACHINE_API bool RtHandleException(void* classDataPtr)
{
    ObjectReference exceptionObjectReference = currentException;
    if (exceptionObjectReference.IsNull())
    {
        return false;
    }
    ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
    std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
    void* object = memoryPool.GetObject(exceptionObjectReference, lock);
    ManagedAllocationHeader* header = GetAllocationHeader(object);
    ObjectHeader* objectHeader = &header->objectHeader;
    uint64_t exceptionObjectTypeId = objectHeader->GetType()->Id();
    ClassData* classToCatch = static_cast<ClassData*>(classDataPtr);
    uint64_t catchTypeId = classToCatch->Type()->Id();
    bool result = exceptionObjectTypeId % catchTypeId == 0;
    return result;
}

extern "C" MACHINE_API uint64_t RtGetException()
{
    ObjectReference exceptionObjectReference = currentException;
    return exceptionObjectReference.Value();
}

extern "C" MACHINE_API void RtDisposeException()
{
    ObjectReference exceptionObjectReference = currentException;
    currentException = 0;
    if (exceptionObjectReference.IsNull())
    {
        return;
    }
    ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
    std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
    void* object = memoryPool.GetObject(exceptionObjectReference, lock);
    ManagedAllocationHeader* header = GetAllocationHeader(object);
    header->Unreference();
}

extern "C" MACHINE_API void RtEnterFunction(void* functionStackEntry)
{
    FunctionStackEntry* entry = static_cast<FunctionStackEntry*>(functionStackEntry);
    if (trace)
    {
        std::cerr << "> " << ToUtf8(entry->function->CallName().Value().AsStringLiteral()) << std::endl;
    }
    entry->next = functionStack;
    functionStack = entry;
}

extern "C" MACHINE_API void RtLeaveFunction(void* functionStackEntry)
{
    FunctionStackEntry* entry = static_cast<FunctionStackEntry*>(functionStackEntry);
    if (trace)
    {
        std::cerr << "< " << ToUtf8(entry->function->CallName().Value().AsStringLiteral()) << std::endl;
    }
    functionStack = entry->next;
}

extern "C" MACHINE_API void RtUnwindFunctionStack(void* functionStackEntry)
{
    FunctionStackEntry* entry = static_cast<FunctionStackEntry*>(functionStackEntry);
    functionStack = entry;
}

extern "C" MACHINE_API int8_t RtLoadFieldSb(uint64_t objectReference, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
        return fieldValue.AsSByte();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint8_t RtLoadFieldBy(uint64_t objectReference, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
        return fieldValue.AsByte();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    return 0;
}

extern "C" MACHINE_API int16_t RtLoadFieldSh(uint64_t objectReference, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
        return fieldValue.AsShort();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint16_t RtLoadFieldUs(uint64_t objectReference, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
        return fieldValue.AsUShort();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    return 0;
}

extern "C" MACHINE_API int32_t RtLoadFieldIn(uint64_t objectReference, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
        return fieldValue.AsInt();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint32_t RtLoadFieldUi(uint64_t objectReference, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
        return fieldValue.AsUInt();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    return 0;
}

extern "C" MACHINE_API int64_t RtLoadFieldLo(uint64_t objectReference, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
        return fieldValue.AsLong();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint64_t RtLoadFieldUl(uint64_t objectReference, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
        return fieldValue.AsULong();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    return 0;
}

extern "C" MACHINE_API float RtLoadFieldFl(uint64_t objectReference, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
        return fieldValue.AsFloat();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    return 0;
}

extern "C" MACHINE_API double RtLoadFieldDo(uint64_t objectReference, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
        return fieldValue.AsDouble();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint32_t RtLoadFieldCh(uint64_t objectReference, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
        return fieldValue.AsChar();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    return 0;
}

extern "C" MACHINE_API bool RtLoadFieldBo(uint64_t objectReference, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
        return fieldValue.AsBool();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    return false;
}

extern "C" MACHINE_API uint64_t RtLoadFieldOb(uint64_t objectReference, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        IntegralValue fieldValue = GetManagedMemoryPool().GetField(reference, fieldIndex);
        return fieldValue.AsULong();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    return 0;
}

extern "C" MACHINE_API void RtStoreFieldSb(uint64_t objectReference, int8_t fieldValue, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        GetManagedMemoryPool().SetField(reference, fieldIndex, MakeIntegralValue<int8_t>(fieldValue, ValueType::sbyteType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
}

extern "C" MACHINE_API void RtStoreFieldBy(uint64_t objectReference, uint8_t fieldValue, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        GetManagedMemoryPool().SetField(reference, fieldIndex, MakeIntegralValue<uint8_t>(fieldValue, ValueType::byteType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
}

extern "C" MACHINE_API void RtStoreFieldSh(uint64_t objectReference, int16_t fieldValue, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        GetManagedMemoryPool().SetField(reference, fieldIndex, MakeIntegralValue<int16_t>(fieldValue, ValueType::shortType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
}

extern "C" MACHINE_API void RtStoreFieldUs(uint64_t objectReference, uint16_t fieldValue, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        GetManagedMemoryPool().SetField(reference, fieldIndex, MakeIntegralValue<uint16_t>(fieldValue, ValueType::ushortType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
}

extern "C" MACHINE_API void RtStoreFieldIn(uint64_t objectReference, int32_t fieldValue, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        GetManagedMemoryPool().SetField(reference, fieldIndex, MakeIntegralValue<int32_t>(fieldValue, ValueType::intType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
}

extern "C" MACHINE_API void RtStoreFieldUi(uint64_t objectReference, uint32_t fieldValue, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        GetManagedMemoryPool().SetField(reference, fieldIndex, MakeIntegralValue<uint32_t>(fieldValue, ValueType::uintType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
}

extern "C" MACHINE_API void RtStoreFieldLo(uint64_t objectReference, int64_t fieldValue, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        GetManagedMemoryPool().SetField(reference, fieldIndex, MakeIntegralValue<int64_t>(fieldValue, ValueType::longType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
}

extern "C" MACHINE_API void RtStoreFieldUl(uint64_t objectReference, uint64_t fieldValue, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        GetManagedMemoryPool().SetField(reference, fieldIndex, MakeIntegralValue<uint64_t>(fieldValue, ValueType::ulongType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
}

extern "C" MACHINE_API void RtStoreFieldFl(uint64_t objectReference, float fieldValue, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        GetManagedMemoryPool().SetField(reference, fieldIndex, MakeIntegralValue<float>(fieldValue, ValueType::floatType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
}

extern "C" MACHINE_API void RtStoreFieldDo(uint64_t objectReference, double fieldValue, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        GetManagedMemoryPool().SetField(reference, fieldIndex, MakeIntegralValue<double>(fieldValue, ValueType::doubleType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
}

extern "C" MACHINE_API void RtStoreFieldCh(uint64_t objectReference, uint32_t fieldValue, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        GetManagedMemoryPool().SetField(reference, fieldIndex, MakeIntegralValue<char32_t>(fieldValue, ValueType::charType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
}

extern "C" MACHINE_API void RtStoreFieldBo(uint64_t objectReference, bool fieldValue, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        GetManagedMemoryPool().SetField(reference, fieldIndex, MakeIntegralValue<bool>(fieldValue, ValueType::boolType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
}

extern "C" MACHINE_API void RtStoreFieldOb(uint64_t objectReference, uint64_t fieldValue, int32_t fieldIndex)
{
    try
    {
        ObjectReference reference(objectReference);
        GetManagedMemoryPool().SetField(reference, fieldIndex, MakeIntegralValue<uint64_t>(fieldValue, ValueType::objectReference));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
}

extern "C" MACHINE_API int8_t RtLoadElemSb(uint64_t arrayReference, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
        return elementValue.AsSByte();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint8_t RtLoadElemBy(uint64_t arrayReference, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
        return elementValue.AsByte();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API int16_t RtLoadElemSh(uint64_t arrayReference, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
        return elementValue.AsShort();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint16_t RtLoadElemUs(uint64_t arrayReference, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
        return elementValue.AsUShort();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API int32_t RtLoadElemIn(uint64_t arrayReference, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
        return elementValue.AsInt();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint32_t RtLoadElemUi(uint64_t arrayReference, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
        return elementValue.AsUInt();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API int64_t RtLoadElemLo(uint64_t arrayReference, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
        return elementValue.AsLong();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint64_t RtLoadElemUl(uint64_t arrayReference, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
        return elementValue.AsULong();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API float RtLoadElemFl(uint64_t arrayReference, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
        return elementValue.AsFloat();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API double RtLoadElemDo(uint64_t arrayReference, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
        return elementValue.AsDouble();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint32_t RtLoadElemCh(uint64_t arrayReference, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
        return elementValue.AsChar();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API bool RtLoadElemBo(uint64_t arrayReference, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
        return elementValue.AsBool();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return false;
}

extern "C" MACHINE_API uint64_t RtLoadElemOb(uint64_t arrayReference, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        IntegralValue elementValue = GetManagedMemoryPool().GetArrayElement(reference, elementIndex);
        return elementValue.AsULong();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API void RtStoreElemSb(uint64_t arrayReference, int8_t elementValue, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, MakeIntegralValue<int8_t>(elementValue, ValueType::sbyteType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreElemBy(uint64_t arrayReference, uint8_t elementValue, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, MakeIntegralValue<uint8_t>(elementValue, ValueType::byteType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreElemSh(uint64_t arrayReference, int16_t elementValue, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, MakeIntegralValue<int16_t>(elementValue, ValueType::shortType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreElemUs(uint64_t arrayReference, uint16_t elementValue, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, MakeIntegralValue<uint16_t>(elementValue, ValueType::ushortType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreElemIn(uint64_t arrayReference, int32_t elementValue, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, MakeIntegralValue<int32_t>(elementValue, ValueType::intType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreElemUi(uint64_t arrayReference, uint32_t elementValue, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, MakeIntegralValue<uint32_t>(elementValue, ValueType::uintType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreElemLo(uint64_t arrayReference, int64_t elementValue, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, MakeIntegralValue<int64_t>(elementValue, ValueType::longType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreElemUl(uint64_t arrayReference, uint64_t elementValue, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, MakeIntegralValue<uint64_t>(elementValue, ValueType::ulongType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreElemFl(uint64_t arrayReference, float elementValue, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, MakeIntegralValue<float>(elementValue, ValueType::floatType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreElemDo(uint64_t arrayReference, double elementValue, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, MakeIntegralValue<double>(elementValue, ValueType::doubleType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreElemCh(uint64_t arrayReference, uint32_t elementValue, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, MakeIntegralValue<char32_t>(elementValue, ValueType::charType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreElemBo(uint64_t arrayReference, bool elementValue, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, MakeIntegralValue<bool>(elementValue, ValueType::boolType));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreElemOb(uint64_t arrayReference, uint64_t elementValue, int32_t elementIndex)
{
    try
    {
        ObjectReference reference(arrayReference);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, MakeIntegralValue<uint64_t>(elementValue, ValueType::objectReference));
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void* RtResolveVirtualFunctionCallAddress(uint64_t objectReference, uint32_t vmtIndex)
{
    try
    {
        ObjectReference receiver(objectReference);
        IntegralValue classDataField = GetManagedMemoryPool().GetField(receiver, 0);
        ClassData* classData = classDataField.AsClassDataPtr();
        if (classData)
        {
            Function* method = classData->Vmt().GetMethod(vmtIndex);
            if (method)
            {
                if (method->Address())
                {
                    return method->Address();
                }
                else
                {
                    throw SystemException("resolving virtual function call address failed: native method address not set");
                }
            }
            else
            {
                throw SystemException("resolving virtual function call address failed: tried to call an abstract method");
            }
        }
        else
        {
            throw SystemException("resolving virtual function call address failed: class data field not set");
        }
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return nullptr;
}

extern "C" MACHINE_API void* RtResolveInterfaceCallAddress(uint64_t objectReference, uint32_t imtIndex, uint64_t* receiver)
{
    try
    {
        ObjectReference interfaceObject(objectReference);
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        IntegralValue receiverField = memoryPool.GetField(interfaceObject, 0, lock);
        ObjectReference receiverObject(receiverField.Value());
        *receiver = receiverObject.Value();
        IntegralValue classDataField = memoryPool.GetField(receiverObject, 0, lock);
        ClassData* classData = classDataField.AsClassDataPtr();
        if (classData)
        {
            IntegralValue itabIndex = memoryPool.GetField(interfaceObject, 1, lock);
            MethodTable& imt = classData->Imt(itabIndex.AsInt());
            Function* method = imt.GetMethod(imtIndex);
            if (method)
            {
                if (method->Address())
                {
                    return method->Address();
                }
                else
                {
                    throw SystemException("resolving interface call address failed: native method address not set");
                }
            }
            else
            {
                throw SystemException("resolving interface call address failed: interface method has no implementation");
            }
        }
        else
        {
            throw SystemException("resolving interface call address failed: class data field not set");
        }
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return nullptr;
}

IntegralValue ToIntegralValue(ValueType type, void* ptr)
{
    switch (type)
    {
        case ValueType::boolType: return MakeIntegralValue<bool>(*static_cast<bool*>(ptr), type);
        case ValueType::charType: return MakeIntegralValue<char32_t>(*static_cast<char32_t*>(ptr), type);
        case ValueType::sbyteType: return MakeIntegralValue<int8_t>(*static_cast<int8_t*>(ptr), type);
        case ValueType::byteType: return MakeIntegralValue<uint8_t>(*static_cast<uint8_t*>(ptr), type);
        case ValueType::shortType: return MakeIntegralValue<int16_t>(*static_cast<int16_t*>(ptr), type);
        case ValueType::ushortType: return MakeIntegralValue<uint16_t>(*static_cast<uint16_t*>(ptr), type);
        case ValueType::intType: return MakeIntegralValue<int32_t>(*static_cast<int32_t*>(ptr), type);
        case ValueType::uintType: return MakeIntegralValue<uint32_t>(*static_cast<uint32_t*>(ptr), type);
        case ValueType::longType: return MakeIntegralValue<int64_t>(*static_cast<int64_t*>(ptr), type);
        case ValueType::ulongType: return MakeIntegralValue<uint64_t>(*static_cast<uint64_t*>(ptr), type);
        case ValueType::floatType: return MakeIntegralValue<float>(*static_cast<float*>(ptr), type);
        case ValueType::doubleType: return MakeIntegralValue<double>(*static_cast<double*>(ptr), type);
        case ValueType::objectReference: return MakeIntegralValue<uint64_t>(*static_cast<uint64_t*>(ptr), type);
        case ValueType::allocationHandle: return MakeIntegralValue<uint64_t>(*static_cast<uint64_t*>(ptr), type);
        case ValueType::variableReference: return MakeIntegralValue<uint64_t>(*static_cast<uint64_t*>(ptr), type);
    }
    return MakeIntegralValue<uint64_t>(*static_cast<uint64_t*>(ptr), type);
}

void FromIntegralValue(ValueType type, void* ptr, IntegralValue value)
{
    switch (type)
    {
        case ValueType::boolType: *static_cast<bool*>(ptr) = value.AsBool(); break;
        case ValueType::charType: *static_cast<char32_t*>(ptr) = value.AsChar(); break;
        case ValueType::sbyteType: *static_cast<int8_t*>(ptr) = value.AsSByte(); break;
        case ValueType::byteType: *static_cast<uint8_t*>(ptr) = value.AsByte(); break;
        case ValueType::shortType: *static_cast<int16_t*>(ptr) = value.AsShort(); break;
        case ValueType::ushortType: *static_cast<uint16_t*>(ptr) = value.AsUShort(); break;
        case ValueType::intType: *static_cast<int32_t*>(ptr) = value.AsInt(); break;
        case ValueType::uintType: *static_cast<uint32_t*>(ptr) = value.AsUInt(); break;
        case ValueType::longType: *static_cast<int64_t*>(ptr) = value.AsLong(); break;
        case ValueType::ulongType: *static_cast<uint64_t*>(ptr) = value.AsULong(); break;
        case ValueType::floatType: *static_cast<float*>(ptr) = value.AsFloat(); break;
        case ValueType::doubleType: *static_cast<double*>(ptr) = value.AsDouble(); break;
        case ValueType::objectReference: *static_cast<uint64_t*>(ptr) = value.AsULong(); break;
        case ValueType::allocationHandle: *static_cast<uint64_t*>(ptr) = value.AsULong(); break;
        case ValueType::variableReference: *static_cast<uint64_t*>(ptr) = value.AsULong(); break;
    }
}

extern "C" MACHINE_API void RtVmCall(void* function, void* constantPool, uint32_t vmFunctionNameId, VmCallContext* vmCallContext)
{
    try
    {
        Function* fun = static_cast<Function*>(function);
        Thread& thread = GetCurrentThread();
        int numLocals = vmCallContext->numLocals;
        uint64_t frameSize = Align(sizeof(Frame), 8) + numLocals * Align(sizeof(LocalVariable), 8);
        std::unique_ptr<uint8_t> frameMem(new uint8_t[frameSize]);
        Frame* frame = new (frameMem.get()) Frame(frameSize, thread, *fun);
        for (int i = 0; i < numLocals; ++i)
        {
            frame->Local(i).SetValue(ToIntegralValue(ValueType(vmCallContext->localTypes[i]), &vmCallContext->locals[i]));
        }
        ConstantPool* cpool = static_cast<ConstantPool*>(constantPool);
        ConstantId vmFunctionNameConstantId(vmFunctionNameId);
        Constant vmFunctionNameConstant = cpool->GetConstant(vmFunctionNameId);
        VmFunction* vmFunction = VmFunctionTable::GetVmFunction(StringPtr(vmFunctionNameConstant.Value().AsStringLiteral()));
        vmFunction->Execute(*frame);
        if (vmCallContext->retValType != uint8_t(ValueType::none))
        {
            IntegralValue returnValue = frame->OpStack().Pop();
            FromIntegralValue(ValueType(vmCallContext->retValType), &vmCallContext->retVal, returnValue);
        }
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const IndexOutOfRangeException& ex)
    {
        RtThrowIndexOutOfRangeException(ex);
    }
    catch (const ArgumentOutOfRangeException& ex)
    {
        RtThrowArgumentOutOfRangeException(ex);
    }
    catch (const FileSystemError& ex)
    {
        RtThrowFileSystemException(ex);
    }
    catch (const ThreadingException& ex)
    {
        RtThrowThreadingException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    catch (const CapturedException& ex)
    {
        currentException = ex.GetException();
        throw CminorException(ex.GetException());
    }
}

extern "C" MACHINE_API void* RtResolveDelegateCallAddress(void* function)
{
    try
    {
        Function* fun = static_cast<Function*>(function);
        if (fun)
        {
            if (fun->Address())
            {
                return fun->Address();
            }
            else
            {
                throw SystemException("resolving delegate call address failed: native method address not set");
            }
        }
        else
        {
            throw NullReferenceException("resolving delegate call address failed: value of delegate is null");
        }
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return nullptr;
}

extern "C" MACHINE_API void* RtResolveClassDelegateCallAddress(uint64_t classDlg, uint64_t* classObject)
{
    try
    {
        ObjectReference classDelegateRef(classDlg);
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        IntegralValue classObjectValue = memoryPool.GetField(classDelegateRef, 1, lock);
        if (classObjectValue.Value() == 0)
        {
            throw NullReferenceException("resolving class delegate call address failed: value of class delegate receiver is null");
        }
        *classObject = classObjectValue.Value();
        IntegralValue funValue = memoryPool.GetField(classDelegateRef, 2, lock);
        Function* function = funValue.AsFunctionPtr();
        if (function)
        {
            if (function->Address())
            {
                return function->Address();
            }
            else
            {
                throw SystemException("resolving class delegate call address failed: native method address not set");
            }
        }
        else
        {
            throw NullReferenceException("resolving class delegate call address failed: value of class delegate method is null");
        }
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return nullptr;
}

extern "C" MACHINE_API void RtSetClassDataPtr(uint64_t objectReference, void* classDataPtr)
{
    try
    {
        ObjectReference reference(objectReference);
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        IntegralValue classDataFieldValue = memoryPool.GetField(reference, 0, lock);
        if (!classDataFieldValue.AsClassDataPtr())
        {
            ClassData* cd = static_cast<ClassData*>(classDataPtr);
            IntegralValue classDataFieldValue = IntegralValue(cd);
            memoryPool.SetField(reference, 0, classDataFieldValue, lock);
        }
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
}

extern "C" MACHINE_API uint64_t RtCreateObject(void* classDataPtr)
{
    try
    {
        ClassData* cd = static_cast<ClassData*>(classDataPtr);
        ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), cd->Type());
        return objectReference.Value();
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint64_t RtCopyObject(uint64_t objectReference)
{
    try
    {
        ObjectReference reference(objectReference);
        ObjectReference copy = GetManagedMemoryPool().CopyObject(GetCurrentThread(), reference);
        return copy.Value();
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint64_t RtStrLitToString(const char32_t* strLitValue)
{
    try
    {
        uint32_t len = static_cast<uint32_t>(StringLen(strLitValue));
        ClassData* classData = ClassDataTable::GetSystemStringClassData();
        Thread& thread = GetCurrentThread();
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        ObjectReference objectReference = memoryPool.CreateObject(thread, classData->Type(), lock);
        AllocationHandle charsHandle = memoryPool.CreateStringCharsFromLiteral(thread, strLitValue, len, lock);
		memoryPool.SetField(objectReference, 0, IntegralValue(classData), lock);
		memoryPool.SetField(objectReference, 1, MakeIntegralValue<int32_t>(len, ValueType::intType), lock);
		memoryPool.SetField(objectReference, 2, charsHandle, lock);
        return objectReference.Value();
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint32_t RtLoadStringChar(int32_t index, uint64_t str)
{
    try
    {
        ObjectReference strReference(str);
        IntegralValue value = GetManagedMemoryPool().GetStringChar(strReference, index);
        return value.AsUInt();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint64_t RtDownCast(uint64_t objectReference, void* classDataPtr)
{
    try
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
                throw InvalidCastException("invalid cast from '" + ToUtf8(classData->Type()->Name().Value()) + "' to '" + ToUtf8(targetClassData->Type()->Name().Value()));
            }
            return reference.Value();
        }
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const InvalidCastException& ex)
    {
        RtThrowInvalidCastException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API void* RtStaticInit(void* classDataPtr)
{
    try
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
                if (staticConstructor->Address())
                {
                    return staticConstructor->Address();
                }
                else
                {
                    throw SystemException("static initialization failed: native address of static constructor not set");
                }
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
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return nullptr;
}

extern "C" MACHINE_API void RtDoneStaticInit(void* classDataPtr)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        staticClassData->ResetInitializing();
        staticClassData->SetInitialized();
        staticClassData->Unlock();
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API int8_t RtLoadStaticFieldSb(void* classDataPtr, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        IntegralValue value = staticClassData->GetStaticField(fieldIndex);
        return value.AsSByte();
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint8_t RtLoadStaticFieldBy(void* classDataPtr, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        IntegralValue value = staticClassData->GetStaticField(fieldIndex);
        return value.AsByte();
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API int16_t RtLoadStaticFieldSh(void* classDataPtr, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        IntegralValue value = staticClassData->GetStaticField(fieldIndex);
        return value.AsShort();
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint16_t RtLoadStaticFieldUs(void* classDataPtr, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        IntegralValue value = staticClassData->GetStaticField(fieldIndex);
        return value.AsUShort();
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API int32_t RtLoadStaticFieldIn(void* classDataPtr, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        IntegralValue value = staticClassData->GetStaticField(fieldIndex);
        return value.AsInt();
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint32_t RtLoadStaticFieldUi(void* classDataPtr, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        IntegralValue value = staticClassData->GetStaticField(fieldIndex);
        return value.AsUInt();
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API int64_t RtLoadStaticFieldLo(void* classDataPtr, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        IntegralValue value = staticClassData->GetStaticField(fieldIndex);
        return value.AsLong();
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint64_t RtLoadStaticFieldUl(void* classDataPtr, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        IntegralValue value = staticClassData->GetStaticField(fieldIndex);
        return value.AsULong();
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API float RtLoadStaticFieldFl(void* classDataPtr, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        IntegralValue value = staticClassData->GetStaticField(fieldIndex);
        return value.AsFloat();
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API double RtLoadStaticFieldDo(void* classDataPtr, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        IntegralValue value = staticClassData->GetStaticField(fieldIndex);
        return value.AsDouble();
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint32_t RtLoadStaticFieldCh(void* classDataPtr, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        IntegralValue value = staticClassData->GetStaticField(fieldIndex);
        return value.AsChar();
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API bool RtLoadStaticFieldBo(void* classDataPtr, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        IntegralValue value = staticClassData->GetStaticField(fieldIndex);
        return value.AsBool();
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return false;
}

extern "C" MACHINE_API uint64_t RtLoadStaticFieldOb(void* classDataPtr, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        IntegralValue value = staticClassData->GetStaticField(fieldIndex);
        return value.AsULong();
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API void RtStoreStaticFieldSb(void* classDataPtr, int8_t fieldValue, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        staticClassData->SetStaticField(MakeIntegralValue<int8_t>(fieldValue, ValueType::sbyteType), fieldIndex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreStaticFieldBy(void* classDataPtr, uint8_t fieldValue, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        staticClassData->SetStaticField(MakeIntegralValue<uint8_t>(fieldValue, ValueType::byteType), fieldIndex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreStaticFieldSh(void* classDataPtr, int16_t fieldValue, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        staticClassData->SetStaticField(MakeIntegralValue<int16_t>(fieldValue, ValueType::shortType), fieldIndex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreStaticFieldUs(void* classDataPtr, uint16_t fieldValue, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        staticClassData->SetStaticField(MakeIntegralValue<uint16_t>(fieldValue, ValueType::ushortType), fieldIndex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreStaticFieldIn(void* classDataPtr, int32_t fieldValue, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        staticClassData->SetStaticField(MakeIntegralValue<int32_t>(fieldValue, ValueType::intType), fieldIndex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreStaticFieldUI(void* classDataPtr, uint32_t fieldValue, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        staticClassData->SetStaticField(MakeIntegralValue<uint32_t>(fieldValue, ValueType::uintType), fieldIndex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreStaticFieldLo(void* classDataPtr, int64_t fieldValue, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        staticClassData->SetStaticField(MakeIntegralValue<int64_t>(fieldValue, ValueType::longType), fieldIndex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreStaticFieldUl(void* classDataPtr, uint64_t fieldValue, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        staticClassData->SetStaticField(MakeIntegralValue<uint64_t>(fieldValue, ValueType::ulongType), fieldIndex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreStaticFieldFl(void* classDataPtr, float fieldValue, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        staticClassData->SetStaticField(MakeIntegralValue<float>(fieldValue, ValueType::floatType), fieldIndex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreStaticFieldDo(void* classDataPtr, double fieldValue, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        staticClassData->SetStaticField(MakeIntegralValue<double>(fieldValue, ValueType::doubleType), fieldIndex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreStaticFieldCh(void* classDataPtr, uint32_t fieldValue, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        staticClassData->SetStaticField(MakeIntegralValue<char32_t>(fieldValue, ValueType::charType), fieldIndex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreStaticFieldBo(void* classDataPtr, bool fieldValue, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        staticClassData->SetStaticField(MakeIntegralValue<bool>(fieldValue, ValueType::boolType), fieldIndex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtStoreStaticFieldOb(void* classDataPtr, uint64_t fieldValue, int32_t fieldIndex)
{
    try
    {
        ClassData* classData = static_cast<ClassData*>(classDataPtr);
        if (!classData)
        {
            throw SystemException("class data pointer not set");
        }
        StaticClassData* staticClassData = classData->GetStaticClassData();
        if (!staticClassData)
        {
            throw SystemException("static data pointer not set");
        }
        staticClassData->SetStaticField(MakeIntegralValue<uint64_t>(fieldValue, ValueType::objectReference), fieldIndex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API uint64_t RtBoxSb(int8_t value)
{
    try
    {
        ObjectType* objectType = GetBoxedType(ValueType::sbyteType);
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        ObjectReference objectReference = memoryPool.CreateObject(GetCurrentThread(), objectType, lock);
        ClassData* classData = GetClassDataForBoxedType(ValueType::sbyteType);
        IntegralValue classDataValue(classData);
        memoryPool.SetField(objectReference, 0, classDataValue, lock);
        memoryPool.SetField(objectReference, 1, MakeIntegralValue<int8_t>(value, ValueType::sbyteType), lock);
        return objectReference.Value();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint64_t RtBoxBy(uint8_t value)
{
    try
    {
        ObjectType* objectType = GetBoxedType(ValueType::byteType);
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        ObjectReference objectReference = memoryPool.CreateObject(GetCurrentThread(), objectType, lock);
        ClassData* classData = GetClassDataForBoxedType(ValueType::byteType);
        IntegralValue classDataValue(classData);
        memoryPool.SetField(objectReference, 0, classDataValue, lock);
        memoryPool.SetField(objectReference, 1, MakeIntegralValue<uint8_t>(value, ValueType::byteType), lock);
        return objectReference.Value();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint64_t RtBoxSh(int16_t value)
{
    try
    {
        ObjectType* objectType = GetBoxedType(ValueType::shortType);
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        ObjectReference objectReference = memoryPool.CreateObject(GetCurrentThread(), objectType, lock);
        ClassData* classData = GetClassDataForBoxedType(ValueType::shortType);
        IntegralValue classDataValue(classData);
        memoryPool.SetField(objectReference, 0, classDataValue, lock);
        memoryPool.SetField(objectReference, 1, MakeIntegralValue<int16_t>(value, ValueType::shortType), lock);
        return objectReference.Value();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint64_t RtBoxUs(uint16_t value)
{
    try
    {
        ObjectType* objectType = GetBoxedType(ValueType::ushortType);
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        ObjectReference objectReference = memoryPool.CreateObject(GetCurrentThread(), objectType, lock);
        ClassData* classData = GetClassDataForBoxedType(ValueType::ushortType);
        IntegralValue classDataValue(classData);
        memoryPool.SetField(objectReference, 0, classDataValue, lock);
        memoryPool.SetField(objectReference, 1, MakeIntegralValue<uint16_t>(value, ValueType::ushortType), lock);
        return objectReference.Value();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint64_t RtBoxIn(int32_t value)
{
    try
    {
        ObjectType* objectType = GetBoxedType(ValueType::intType);
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        ObjectReference objectReference = memoryPool.CreateObject(GetCurrentThread(), objectType, lock);
        ClassData* classData = GetClassDataForBoxedType(ValueType::intType);
        IntegralValue classDataValue(classData);
        memoryPool.SetField(objectReference, 0, classDataValue, lock);
        memoryPool.SetField(objectReference, 1, MakeIntegralValue<int32_t>(value, ValueType::intType), lock);
        return objectReference.Value();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint64_t RtBoxUi(uint32_t value)
{
    try
    {
        ObjectType* objectType = GetBoxedType(ValueType::uintType);
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        ObjectReference objectReference = memoryPool.CreateObject(GetCurrentThread(), objectType, lock);
        ClassData* classData = GetClassDataForBoxedType(ValueType::uintType);
        IntegralValue classDataValue(classData);
        memoryPool.SetField(objectReference, 0, classDataValue, lock);
        memoryPool.SetField(objectReference, 1, MakeIntegralValue<uint32_t>(value, ValueType::uintType), lock);
        return objectReference.Value();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint64_t RtBoxLo(int64_t value)
{
    try
    {
        ObjectType* objectType = GetBoxedType(ValueType::longType);
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        ObjectReference objectReference = memoryPool.CreateObject(GetCurrentThread(), objectType, lock);
        ClassData* classData = GetClassDataForBoxedType(ValueType::longType);
        IntegralValue classDataValue(classData);
        memoryPool.SetField(objectReference, 0, classDataValue, lock);
        memoryPool.SetField(objectReference, 1, MakeIntegralValue<int64_t>(value, ValueType::longType), lock);
        return objectReference.Value();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint64_t RtBoxUl(uint64_t value)
{
    try
    {
        ObjectType* objectType = GetBoxedType(ValueType::ulongType);
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        ObjectReference objectReference = memoryPool.CreateObject(GetCurrentThread(), objectType, lock);
        ClassData* classData = GetClassDataForBoxedType(ValueType::ulongType);
        IntegralValue classDataValue(classData);
        memoryPool.SetField(objectReference, 0, classDataValue, lock);
        memoryPool.SetField(objectReference, 1, MakeIntegralValue<uint64_t>(value, ValueType::ulongType), lock);
        return objectReference.Value();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint64_t RtBoxFl(float value)
{
    try
    {
        ObjectType* objectType = GetBoxedType(ValueType::floatType);
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        ObjectReference objectReference = memoryPool.CreateObject(GetCurrentThread(), objectType, lock);
        ClassData* classData = GetClassDataForBoxedType(ValueType::floatType);
        IntegralValue classDataValue(classData);
        memoryPool.SetField(objectReference, 0, classDataValue, lock);
        memoryPool.SetField(objectReference, 1, MakeIntegralValue<float>(value, ValueType::floatType), lock);
        return objectReference.Value();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint64_t RtBoxDo(double value)
{
    try
    {
        ObjectType* objectType = GetBoxedType(ValueType::doubleType);
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        ObjectReference objectReference = memoryPool.CreateObject(GetCurrentThread(), objectType, lock);
        ClassData* classData = GetClassDataForBoxedType(ValueType::doubleType);
        IntegralValue classDataValue(classData);
        memoryPool.SetField(objectReference, 0, classDataValue, lock);
        memoryPool.SetField(objectReference, 1, MakeIntegralValue<double>(value, ValueType::doubleType), lock);
        return objectReference.Value();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint64_t RtBoxCh(uint32_t value)
{
    try
    {
        ObjectType* objectType = GetBoxedType(ValueType::charType);
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        ObjectReference objectReference = memoryPool.CreateObject(GetCurrentThread(), objectType, lock);
        ClassData* classData = GetClassDataForBoxedType(ValueType::charType);
        IntegralValue classDataValue(classData);
        memoryPool.SetField(objectReference, 0, classDataValue, lock);
        memoryPool.SetField(objectReference, 1, MakeIntegralValue<char32_t>(value, ValueType::charType), lock);
        return objectReference.Value();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint64_t RtBoxBo(bool value)
{
    try
    {
        ObjectType* objectType = GetBoxedType(ValueType::boolType);
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        ObjectReference objectReference = memoryPool.CreateObject(GetCurrentThread(), objectType, lock);
        ClassData* classData = GetClassDataForBoxedType(ValueType::boolType);
        IntegralValue classDataValue(classData);
        memoryPool.SetField(objectReference, 0, classDataValue, lock);
        memoryPool.SetField(objectReference, 1, MakeIntegralValue<bool>(value, ValueType::boolType), lock);
        return objectReference.Value();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API int8_t RtUnboxSb(uint64_t objectReference)
{
    try
    {
        ObjectReference reference(objectReference);
        if (reference.IsNull())
        {
            throw NullReferenceException("tried to unbox null object reference");
        }
        IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
        return value.AsSByte();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint8_t RtUnboxBy(uint64_t objectReference)
{
    try
    {
        ObjectReference reference(objectReference);
        if (reference.IsNull())
        {
            throw NullReferenceException("tried to unbox null object reference");
        }
        IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
        return value.AsByte();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API int16_t RtUnboxSh(uint64_t objectReference)
{
    try
    {
        ObjectReference reference(objectReference);
        if (reference.IsNull())
        {
            throw NullReferenceException("tried to unbox null object reference");
        }
        IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
        return value.AsShort();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint16_t RtUnboxUs(uint64_t objectReference)
{
    try
    {
        ObjectReference reference(objectReference);
        if (reference.IsNull())
        {
            throw NullReferenceException("tried to unbox null object reference");
        }
        IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
        return value.AsUShort();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API int32_t RtUnboxIn(uint64_t objectReference)
{
    try
    {
        ObjectReference reference(objectReference);
        if (reference.IsNull())
        {
            throw NullReferenceException("tried to unbox null object reference");
        }
        IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
        return value.AsInt();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint32_t RtUnboxUi(uint64_t objectReference)
{
    try
    {
        ObjectReference reference(objectReference);
        if (reference.IsNull())
        {
            throw NullReferenceException("tried to unbox null object reference");
        }
        IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
        return value.AsUInt();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API int64_t RtUnboxLo(uint64_t objectReference)
{
    try
    {
        ObjectReference reference(objectReference);
        if (reference.IsNull())
        {
            throw NullReferenceException("tried to unbox null object reference");
        }
        IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
        return value.AsLong();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint64_t RtUnboxUl(uint64_t objectReference)
{
    try
    {
        ObjectReference reference(objectReference);
        if (reference.IsNull())
        {
            throw NullReferenceException("tried to unbox null object reference");
        }
        IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
        return value.AsULong();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API float RtUnboxFl(uint64_t objectReference)
{
    try
    {
        ObjectReference reference(objectReference);
        if (reference.IsNull())
        {
            throw NullReferenceException("tried to unbox null object reference");
        }
        IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
        return value.AsFloat();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API double RtUnboxDo(uint64_t objectReference)
{
    try
    {
        ObjectReference reference(objectReference);
        if (reference.IsNull())
        {
            throw NullReferenceException("tried to unbox null object reference");
        }
        IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
        return value.AsDouble();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API uint32_t RtUnboxCh(uint64_t objectReference)
{
    try
    {
        ObjectReference reference(objectReference);
        if (reference.IsNull())
        {
            throw NullReferenceException("tried to unbox null object reference");
        }
        IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
        return value.AsChar();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API bool RtUnboxBo(uint64_t objectReference)
{
    try
    {
        ObjectReference reference(objectReference);
        if (reference.IsNull())
        {
            throw NullReferenceException("tried to unbox null object reference");
        }
        IntegralValue value = GetManagedMemoryPool().GetField(reference, 1);
        return value.AsBool();
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return 0;
}

extern "C" MACHINE_API void RtAllocateArrayElements(uint64_t arr, int32_t length, void* elementTypePtr)
{
    try
    {
        Type* elementType = static_cast<Type*>(elementTypePtr);
        if (!elementType)
        {
            throw SystemException("element type is null");
        }
        ObjectReference arrayReference(arr);
        GetManagedMemoryPool().AllocateArrayElements(GetCurrentThread(), arrayReference, elementType, length);
    }
    catch (const ArgumentOutOfRangeException& ex)
    {
        RtThrowArgumentOutOfRangeException(ex);
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API bool RtIs(uint64_t objectReference, void* classDataPtr)
{
    try
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
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
    return false;
}

extern "C" MACHINE_API uint64_t RtAs(uint64_t objectReference, void* classDataPtr)
{
    try
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
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
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
    try
    {
        ObjectReference classDelegateReference(classDelegateObjectReference);
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        void* classDelegateObject = GetManagedMemoryPool().GetObject(classDelegateReference, lock);
        ObjectReference classObjectValue(classObjectRererence);
        SetObjectField(classDelegateObject, classObjectValue, 1);
        Function* memberFun = static_cast<Function*>(memberFunction);
        IntegralValue memberFunctionValue(memberFun);
        SetObjectField(classDelegateObject, memberFunctionValue, 2);
    }
    catch (const NullReferenceException& ex)
    {
        RtThrowNullReferenceException(ex);
    }
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
    }
}

extern "C" MACHINE_API void RtRequestGc(bool requestFullCollection)
{
    Thread& thread = GetCurrentThread();
    thread.RequestGc(requestFullCollection);
    thread.WaitUntilGarbageCollected();
}

extern "C" MACHINE_API void RtPollGc()
{
    if (wantToCollectGarbage)
    {
        GetCurrentThread().WaitUntilGarbageCollected();
    }
}

} } // namespace cminor::machine
