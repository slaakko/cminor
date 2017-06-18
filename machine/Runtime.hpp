// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_RUNTIME_INCLUDED
#define CMINOR_MACHINE_RUNTIME_INCLUDED
#include <cminor/machine/MachineApi.hpp>
#include <cminor/machine/CminorException.hpp>
#include <stdint.h>

namespace cminor { namespace machine {

MACHINE_API void SetTrace();

class Function;

struct MACHINE_API FunctionStackEntry
{
    FunctionStackEntry* next;   // 0: pointer to caller's function stack entry
    Function* function;         // 1: pointer to IL function
    int32_t lineNumber;         // 2: current line number (updated before each call and throw)
    int32_t numGcRoots;         // 3: number of GC roots in this function
    uint64_t** gcEntry;         // 4: pointer to array of GC root pointers (array contains numGcRoots GC root pointers)
};

extern "C" MACHINE_API FunctionStackEntry* RtGetFunctionStack();
extern "C" MACHINE_API void RtThrowException(uint64_t exceptionObjectReference);
extern "C" MACHINE_API bool RtHandleException(void* classDataPtr);
extern "C" MACHINE_API uint64_t RtGetException();
extern "C" MACHINE_API void RtDisposeException();

extern "C" MACHINE_API void RtEnterFunction(void* functionStackEntry);
extern "C" MACHINE_API void RtLeaveFunction(void* functionStackEntry);
extern "C" MACHINE_API void RtUnwindFunctionStack(void* functionStackEntry);

extern "C" MACHINE_API int8_t RtLoadFieldSb(uint64_t objectReference, int32_t fieldIndex);
extern "C" MACHINE_API uint8_t RtLoadFieldBy(uint64_t objectReference, int32_t fieldIndex);
extern "C" MACHINE_API int16_t RtLoadFieldSh(uint64_t objectReference, int32_t fieldIndex);
extern "C" MACHINE_API uint16_t RtLoadFieldUs(uint64_t objectReference, int32_t fieldIndex);
extern "C" MACHINE_API int32_t RtLoadFieldIn(uint64_t objectReference, int32_t fieldIndex);
extern "C" MACHINE_API uint32_t RtLoadFieldUi(uint64_t objectReference, int32_t fieldIndex);
extern "C" MACHINE_API int64_t RtLoadFieldLo(uint64_t objectReference, int32_t fieldIndex);
extern "C" MACHINE_API uint64_t RtLoadFieldUl(uint64_t objectReference, int32_t fieldIndex);
extern "C" MACHINE_API float RtLoadFieldFl(uint64_t objectReference, int32_t fieldIndex);
extern "C" MACHINE_API double RtLoadFieldDo(uint64_t objectReference, int32_t fieldIndex);
extern "C" MACHINE_API uint32_t RtLoadFieldCh(uint64_t objectReference, int32_t fieldIndex);
extern "C" MACHINE_API bool RtLoadFieldBo(uint64_t objectReference, int32_t fieldIndex);
extern "C" MACHINE_API uint64_t RtLoadFieldOb(uint64_t objectReference, int32_t fieldIndex);

extern "C" MACHINE_API void RtStoreFieldSb(uint64_t objectReference, int8_t fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreFieldBy(uint64_t objectReference, uint8_t fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreFieldSh(uint64_t objectReference, int16_t fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreFieldUs(uint64_t objectReference, uint16_t fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreFieldIn(uint64_t objectReference, int32_t fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreFieldUi(uint64_t objectReference, uint32_t fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreFieldLo(uint64_t objectReference, int64_t fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreFieldUl(uint64_t objectReference, uint64_t fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreFieldFl(uint64_t objectReference, float fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreFieldDo(uint64_t objectReference, double fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreFieldCh(uint64_t objectReference, uint32_t fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreFieldBo(uint64_t objectReference, bool fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreFieldOb(uint64_t objectReference, uint64_t fieldValue, int32_t fieldIndex);

extern "C" MACHINE_API int8_t RtLoadElemSb(uint64_t arrayReference, int32_t elementIndex);
extern "C" MACHINE_API uint8_t RtLoadElemBy(uint64_t arrayReference, int32_t elementIndex);
extern "C" MACHINE_API int16_t RtLoadElemSh(uint64_t arrayReference, int32_t elementIndex);
extern "C" MACHINE_API uint16_t RtLoadElemUs(uint64_t arrayReference, int32_t elementIndex);
extern "C" MACHINE_API int32_t RtLoadElemIn(uint64_t arrayReference, int32_t elementIndex);
extern "C" MACHINE_API uint32_t RtLoadElemUi(uint64_t arrayReference, int32_t elementIndex);
extern "C" MACHINE_API int64_t RtLoadElemLo(uint64_t arrayReference, int32_t elementIndex);
extern "C" MACHINE_API uint64_t RtLoadElemUl(uint64_t arrayReference, int32_t elementIndex);
extern "C" MACHINE_API float RtLoadElemFl(uint64_t arrayReference, int32_t elementIndex);
extern "C" MACHINE_API double RtLoadElemDo(uint64_t arrayReference, int32_t elementIndex);
extern "C" MACHINE_API uint32_t RtLoadElemCh(uint64_t arrayReference, int32_t elementIndex);
extern "C" MACHINE_API bool RtLoadElemBo(uint64_t arrayReference, int32_t elementIndex);
extern "C" MACHINE_API uint64_t RtLoadElemOb(uint64_t arrayReference, int32_t elementIndex);

extern "C" MACHINE_API void RtStoreElemSb(uint64_t arrayReference, int8_t elementValue, int32_t elementIndex);
extern "C" MACHINE_API void RtStoreElemBy(uint64_t arrayReference, uint8_t elementValue, int32_t elementIndex);
extern "C" MACHINE_API void RtStoreElemSh(uint64_t arrayReference, int16_t elementValue, int32_t elementIndex);
extern "C" MACHINE_API void RtStoreElemUs(uint64_t arrayReference, uint16_t elementValue, int32_t elementIndex);
extern "C" MACHINE_API void RtStoreElemIn(uint64_t arrayReference, int32_t elementValue, int32_t elementIndex);
extern "C" MACHINE_API void RtStoreElemUi(uint64_t arrayReference, uint32_t elementValue, int32_t elementIndex);
extern "C" MACHINE_API void RtStoreElemLo(uint64_t arrayReference, int64_t elementValue, int32_t elementIndex);
extern "C" MACHINE_API void RtStoreElemUl(uint64_t arrayReference, uint64_t elementValue, int32_t elementIndex);
extern "C" MACHINE_API void RtStoreElemFl(uint64_t arrayReference, float elementValue, int32_t elementIndex);
extern "C" MACHINE_API void RtStoreElemDo(uint64_t arrayReference, double elementValue, int32_t elementIndex);
extern "C" MACHINE_API void RtStoreElemCh(uint64_t arrayReference, uint32_t elementValue, int32_t elementIndex);
extern "C" MACHINE_API void RtStoreElemBo(uint64_t arrayReference, bool elementValue, int32_t elementIndex);
extern "C" MACHINE_API void RtStoreElemOb(uint64_t arrayReference, uint64_t elementValue, int32_t elementIndex);

extern "C" MACHINE_API void* RtResolveVirtualFunctionCallAddress(uint64_t objectReference, uint32_t vmtIndex);

extern "C" MACHINE_API void* RtResolveInterfaceCallAddress(uint64_t objectReference, uint32_t imtIndex, uint64_t* receiver);

struct MACHINE_API VmCallContext
{
    int32_t numLocals;
    uint8_t* localTypes;
    uint64_t* locals;
    uint8_t retValType;
    uint64_t retVal;
};

extern "C" MACHINE_API void RtVmCall(void* function, void* constantPool, uint32_t vmFunctionNameId, VmCallContext* vmCallContext);

extern "C" MACHINE_API void* RtResolveDelegateCallAddress(void* function);

extern "C" MACHINE_API void* RtResolveClassDelegateCallAddress(uint64_t classDlg, uint64_t* classObject);

extern "C" MACHINE_API void RtSetClassDataPtr(uint64_t objectReference, void* classDataPtr);

extern "C" MACHINE_API uint64_t RtCreateObject(void* classDataPtr);

extern "C" MACHINE_API uint64_t RtCopyObject(uint64_t objectReference);

extern "C" MACHINE_API uint64_t RtStrLitToString(const char32_t* strLitValue);

extern "C" MACHINE_API uint32_t RtLoadStringChar(int32_t index, uint64_t str);

extern "C" MACHINE_API uint64_t RtDownCast(uint64_t objectReference, void* classDataPtr);

extern "C" MACHINE_API void* RtStaticInit(void* classDataPtr);

extern "C" MACHINE_API void RtDoneStaticInit(void* classDataPtr);

extern "C" MACHINE_API int8_t RtLoadStaticFieldSb(void* classDataPtr, int32_t fieldIndex);
extern "C" MACHINE_API uint8_t RtLoadStaticFieldBy(void* classDataPtr, int32_t fieldIndex);
extern "C" MACHINE_API int16_t RtLoadStaticFieldSh(void* classDataPtr, int32_t fieldIndex);
extern "C" MACHINE_API uint16_t RtLoadStaticFieldUs(void* classDataPtr, int32_t fieldIndex);
extern "C" MACHINE_API int32_t RtLoadStaticFieldIn(void* classDataPtr, int32_t fieldIndex);
extern "C" MACHINE_API uint32_t RtLoadStaticFieldUi(void* classDataPtr, int32_t fieldIndex);
extern "C" MACHINE_API int64_t RtLoadStaticFieldLo(void* classDataPtr, int32_t fieldIndex);
extern "C" MACHINE_API uint64_t RtLoadStaticFieldUl(void* classDataPtr, int32_t fieldIndex);
extern "C" MACHINE_API float RtLoadStaticFieldFl(void* classDataPtr, int32_t fieldIndex);
extern "C" MACHINE_API double RtLoadStaticFieldDo(void* classDataPtr, int32_t fieldIndex);
extern "C" MACHINE_API uint32_t RtLoadStaticFieldCh(void* classDataPtr, int32_t fieldIndex);
extern "C" MACHINE_API bool RtLoadStaticFieldBo(void* classDataPtr, int32_t fieldIndex);
extern "C" MACHINE_API uint64_t RtLoadStaticFieldOb(void* classDataPtr, int32_t fieldIndex);

extern "C" MACHINE_API void RtStoreStaticFieldSb(void* classDataPtr, int8_t fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreStaticFieldBy(void* classDataPtr, uint8_t fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreStaticFieldSh(void* classDataPtr, int16_t fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreStaticFieldUs(void* classDataPtr, uint16_t fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreStaticFieldIn(void* classDataPtr, int32_t fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreStaticFieldUI(void* classDataPtr, uint32_t fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreStaticFieldLo(void* classDataPtr, int64_t fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreStaticFieldUl(void* classDataPtr, uint64_t fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreStaticFieldFl(void* classDataPtr, float fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreStaticFieldDo(void* classDataPtr, double fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreStaticFieldCh(void* classDataPtr, uint32_t fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreStaticFieldBo(void* classDataPtr, bool fieldValue, int32_t fieldIndex);
extern "C" MACHINE_API void RtStoreStaticFieldOb(void* classDataPtr, uint64_t fieldValue, int32_t fieldIndex);

extern "C" MACHINE_API uint64_t RtBoxSb(int8_t value);
extern "C" MACHINE_API uint64_t RtBoxBy(uint8_t value);
extern "C" MACHINE_API uint64_t RtBoxSh(int16_t value);
extern "C" MACHINE_API uint64_t RtBoxUs(uint16_t value);
extern "C" MACHINE_API uint64_t RtBoxIn(int32_t value);
extern "C" MACHINE_API uint64_t RtBoxUi(uint32_t value);
extern "C" MACHINE_API uint64_t RtBoxLo(int64_t value);
extern "C" MACHINE_API uint64_t RtBoxUl(uint64_t value);
extern "C" MACHINE_API uint64_t RtBoxFl(float value);
extern "C" MACHINE_API uint64_t RtBoxDo(double value);
extern "C" MACHINE_API uint64_t RtBoxCh(uint32_t value);
extern "C" MACHINE_API uint64_t RtBoxBo(bool value);

extern "C" MACHINE_API int8_t RtUnboxSb(uint64_t objectReference);
extern "C" MACHINE_API uint8_t RtUnboxBy(uint64_t objectReference);
extern "C" MACHINE_API int16_t RtUnboxSh(uint64_t objectReference);
extern "C" MACHINE_API uint16_t RtUnboxUs(uint64_t objectReference);
extern "C" MACHINE_API int32_t RtUnboxIn(uint64_t objectReference);
extern "C" MACHINE_API uint32_t RtUnboxUi(uint64_t objectReference);
extern "C" MACHINE_API int64_t RtUnboxLo(uint64_t objectReference);
extern "C" MACHINE_API uint64_t RtUnboxUl(uint64_t objectReference);
extern "C" MACHINE_API float RtUnboxFl(uint64_t objectReference);
extern "C" MACHINE_API double RtUnboxDo(uint64_t objectReference);
extern "C" MACHINE_API uint32_t RtUnboxCh(uint64_t objectReference);
extern "C" MACHINE_API bool RtUnboxBo(uint64_t objectReference);

extern "C" MACHINE_API void RtAllocateArrayElements(uint64_t arr, int32_t length, void* elementTypePtr);

extern "C" MACHINE_API bool RtIs(uint64_t objectReference, void* classDataPtr);

extern "C" MACHINE_API uint64_t RtAs(uint64_t objectReference, void* classDataPtr);

extern "C" MACHINE_API const char32_t* RtLoadStringLiteral(void* constantPool, uint32_t constantId);

extern "C" MACHINE_API void RtMemFun2ClassDelegate(uint64_t classObjectRererence, uint64_t classDelegateObjectReference, void* memberFunction);

extern "C" MACHINE_API void RtRequestGc();

extern "C" MACHINE_API void RtPollGc();

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_RUNTIME_INCLUDED
