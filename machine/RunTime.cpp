// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/RunTime.hpp>
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
    ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
    currentException = objectReference.Value();
    ClassData* classData = ClassDataTable::GetClassData(StringPtr(exceptionTypeName.c_str()));
    Object& o = GetManagedMemoryPool().GetObject(objectReference);
    o.Pin();
    IntegralValue classDataValue(classData);
    o.SetField(classDataValue, 0);
    ObjectReference messageStr = GetManagedMemoryPool().CreateString(GetCurrentThread(), ToUtf32(message));
    o.SetField(messageStr, 1);
    utf32_string stackTrace = GetStackTrace();
    ObjectReference stacTraceStr = GetManagedMemoryPool().CreateString(GetCurrentThread(), stackTrace);
    o.SetField(stacTraceStr, 2);
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

extern "C" MACHINE_API void RtThrowException(uint64_t exceptionObjectReference)
{
    if (exceptionObjectReference == 0)
    {
        RtThrowSystemException(SystemException("tried to throw null reference"));
        return;
    }
    currentException = exceptionObjectReference;
    Object& exceptionObject = GetManagedMemoryPool().GetObject(exceptionObjectReference);
    exceptionObject.Pin();
    utf32_string stackTrace = GetStackTrace();
    ObjectReference stacTraceStr = GetManagedMemoryPool().CreateString(GetCurrentThread(), stackTrace);
    exceptionObject.SetField(stacTraceStr, 2);
    throw CminorException(exceptionObjectReference);
}

extern "C" MACHINE_API bool RtHandleException(void* classDataPtr)
{
    ObjectReference exceptionObjectReference = currentException;
    if (exceptionObjectReference.IsNull())
    {
        return false;
    }
    Object& exceptionObject = GetManagedMemoryPool().GetObject(exceptionObjectReference);
    uint64_t exceptionObjectTypeId = exceptionObject.GetType()->Id();
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
    Object& exceptionObject = GetManagedMemoryPool().GetObject(exceptionObjectReference);
    exceptionObject.Unpin();
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

extern "C" MACHINE_API FunctionStackEntry* RtGetFunctionStack()
{
    return functionStack;
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
        GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::sbyteType));
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
        GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::byteType));
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
        GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::shortType));
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
        GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::ushortType));
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
        GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::intType));
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
        GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::uintType));
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
        GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::longType));
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
        GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::ulongType));
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
        GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::floatType));
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
        GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::doubleType));
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
        GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::charType));
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
        GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::boolType));
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
        GetManagedMemoryPool().SetField(reference, fieldIndex, IntegralValue(fieldValue, ValueType::objectReference));
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
        IntegralValue value(elementValue, ValueType::sbyteType);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
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
        IntegralValue value(elementValue, ValueType::byteType);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
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
        IntegralValue value(elementValue, ValueType::shortType);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
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
        IntegralValue value(elementValue, ValueType::ushortType);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
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
        IntegralValue value(elementValue, ValueType::intType);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
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
        IntegralValue value(elementValue, ValueType::uintType);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
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
        IntegralValue value(elementValue, ValueType::longType);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
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
        IntegralValue value(elementValue, ValueType::ulongType);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
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
        IntegralValue value(elementValue, ValueType::floatType);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
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
        IntegralValue value(elementValue, ValueType::doubleType);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
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
        IntegralValue value(elementValue, ValueType::charType);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
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
        IntegralValue value(elementValue, ValueType::boolType);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
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
        IntegralValue value(elementValue, ValueType::objectReference);
        GetManagedMemoryPool().SetArrayElement(reference, elementIndex, value);
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
    catch (const SystemException& ex)
    {
        RtThrowSystemException(ex);
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
        Object& classDelegateObject = GetManagedMemoryPool().GetObject(classDelegateRef);
        IntegralValue classObjectValue = classDelegateObject.GetField(1);
        if (classObjectValue.Value() == 0)
        {
            throw NullReferenceException("resolving class delegate call address failed: value of class delegate receiver is null");
        }
        *classObject = classObjectValue.Value();
        IntegralValue funValue = classDelegateObject.GetField(2);
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
        IntegralValue classDataFieldValue = GetManagedMemoryPool().GetField(reference, 0);
        if (!classDataFieldValue.AsClassDataPtr())
        {
            ClassData* cd = static_cast<ClassData*>(classDataPtr);
            IntegralValue classDataFieldValue = IntegralValue(cd);
            GetManagedMemoryPool().SetField(reference, 0, classDataFieldValue);
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
            ObjectReference copy = reference;
            return copy.Value();
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
        IntegralValue value(fieldValue, ValueType::sbyteType);
        staticClassData->SetStaticField(value, fieldIndex);
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
        IntegralValue value(fieldValue, ValueType::byteType);
        staticClassData->SetStaticField(value, fieldIndex);
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
        IntegralValue value(fieldValue, ValueType::shortType);
        staticClassData->SetStaticField(value, fieldIndex);
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
        IntegralValue value(fieldValue, ValueType::ushortType);
        staticClassData->SetStaticField(value, fieldIndex);
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
        IntegralValue value(fieldValue, ValueType::intType);
        staticClassData->SetStaticField(value, fieldIndex);
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
        IntegralValue value(fieldValue, ValueType::uintType);
        staticClassData->SetStaticField(value, fieldIndex);
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
        IntegralValue value(fieldValue, ValueType::longType);
        staticClassData->SetStaticField(value, fieldIndex);
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
        IntegralValue value(fieldValue, ValueType::ulongType);
        staticClassData->SetStaticField(value, fieldIndex);
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
        IntegralValue value(fieldValue, ValueType::floatType);
        staticClassData->SetStaticField(value, fieldIndex);
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
        IntegralValue value(fieldValue, ValueType::doubleType);
        staticClassData->SetStaticField(value, fieldIndex);
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
        IntegralValue value(fieldValue, ValueType::charType);
        staticClassData->SetStaticField(value, fieldIndex);
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
        IntegralValue value(fieldValue, ValueType::boolType);
        staticClassData->SetStaticField(value, fieldIndex);
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
        IntegralValue value(fieldValue, ValueType::objectReference);
        staticClassData->SetStaticField(value, fieldIndex);
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
        ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
        ClassData* classData = GetClassDataForBoxedType(ValueType::sbyteType);
        IntegralValue classDataValue(classData);
        GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
        IntegralValue val(value, ValueType::sbyteType);
        GetManagedMemoryPool().SetField(objectReference, 1, val);
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
        ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
        ClassData* classData = GetClassDataForBoxedType(ValueType::byteType);
        IntegralValue classDataValue(classData);
        GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
        IntegralValue val(value, ValueType::byteType);
        GetManagedMemoryPool().SetField(objectReference, 1, val);
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
        ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
        ClassData* classData = GetClassDataForBoxedType(ValueType::shortType);
        IntegralValue classDataValue(classData);
        GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
        IntegralValue val(value, ValueType::shortType);
        GetManagedMemoryPool().SetField(objectReference, 1, val);
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
        ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
        ClassData* classData = GetClassDataForBoxedType(ValueType::ushortType);
        IntegralValue classDataValue(classData);
        GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
        IntegralValue val(value, ValueType::ushortType);
        GetManagedMemoryPool().SetField(objectReference, 1, val);
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
        ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
        ClassData* classData = GetClassDataForBoxedType(ValueType::intType);
        IntegralValue classDataValue(classData);
        GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
        IntegralValue val(value, ValueType::intType);
        GetManagedMemoryPool().SetField(objectReference, 1, val);
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
        ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
        ClassData* classData = GetClassDataForBoxedType(ValueType::uintType);
        IntegralValue classDataValue(classData);
        GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
        IntegralValue val(value, ValueType::uintType);
        GetManagedMemoryPool().SetField(objectReference, 1, val);
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
        ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
        ClassData* classData = GetClassDataForBoxedType(ValueType::longType);
        IntegralValue classDataValue(classData);
        GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
        IntegralValue val(value, ValueType::longType);
        GetManagedMemoryPool().SetField(objectReference, 1, val);
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
        ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
        ClassData* classData = GetClassDataForBoxedType(ValueType::ulongType);
        IntegralValue classDataValue(classData);
        GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
        IntegralValue val(value, ValueType::ulongType);
        GetManagedMemoryPool().SetField(objectReference, 1, val);
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
        ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
        ClassData* classData = GetClassDataForBoxedType(ValueType::floatType);
        IntegralValue classDataValue(classData);
        GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
        IntegralValue val(value, ValueType::floatType);
        GetManagedMemoryPool().SetField(objectReference, 1, val);
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
        ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
        ClassData* classData = GetClassDataForBoxedType(ValueType::doubleType);
        IntegralValue classDataValue(classData);
        GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
        IntegralValue val(value, ValueType::doubleType);
        GetManagedMemoryPool().SetField(objectReference, 1, val);
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
        ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
        ClassData* classData = GetClassDataForBoxedType(ValueType::charType);
        IntegralValue classDataValue(classData);
        GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
        IntegralValue val(value, ValueType::charType);
        GetManagedMemoryPool().SetField(objectReference, 1, val);
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
        ObjectReference objectReference = GetManagedMemoryPool().CreateObject(GetCurrentThread(), objectType);
        ClassData* classData = GetClassDataForBoxedType(ValueType::boolType);
        IntegralValue classDataValue(classData);
        GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
        IntegralValue val(value, ValueType::boolType);
        GetManagedMemoryPool().SetField(objectReference, 1, val);
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
        Object& classDelegateObject = GetManagedMemoryPool().GetObject(classDelegateReference);
        ObjectReference classObjectValue(classObjectRererence);
        classDelegateObject.SetField(classObjectValue, 1);
        Function* memberFun = static_cast<Function*>(memberFunction);
        IntegralValue memberFunctionValue(memberFun);
        classDelegateObject.SetField(memberFunctionValue, 2);
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

extern "C" MACHINE_API void RtRequestGc()
{
    Thread& thread = GetCurrentThread();
    thread.SetFunctionStack(functionStack);
    thread.SetState(ThreadState::paused);
#ifdef GC_LOGGING
    LogMessage(">" + std::to_string(thread.Id()) + " (paused)");
#endif
    thread.GetMachine().GetGarbageCollector().RequestGarbageCollection(thread);
#ifdef GC_LOGGING
    LogMessage(">" + std::to_string(thread.Id()) + " (collection requested)");
#endif
    thread.GetMachine().GetGarbageCollector().WaitUntilGarbageCollected(thread);
#ifdef GC_LOGGING
    LogMessage(">" + std::to_string(thread.Id()) + " (collection ended)");
#endif
    thread.SetState(ThreadState::running);
#ifdef GC_LOGGING
    LogMessage(">" + std::to_string(thread.Id()) + " (running)");
#endif
    thread.GetMachine().GetGarbageCollector().WaitForIdle(thread);
}

extern "C" MACHINE_API void RtWaitGc()
{
    GetCurrentThread().SetFunctionStack(functionStack);
    GetCurrentThread().WaitUntilGarbageCollected();
}

} } // namespace cminor::machine
