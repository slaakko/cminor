// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_MACHINE_FUNCTION_VISITOR_INCLUDED
#define CMINOR_MACHINE_MACHINE_FUNCTION_VISITOR_INCLUDED
#include <cminor/machine/MachineApi.hpp>
#include <cminor/machine/Object.hpp>

namespace cminor { namespace machine {

class Function;
class Instruction;
class InvalidInst;
class NopInst;
class LoadDefaultValueBaseInst;
class UnaryOpBaseInst;
class BinaryOpBaseInst;
class BinaryPredBaseInst;
class LogicalNotInst;
class LoadElemInst;
class StoreElemInst;
class ReceiveInst;
class ConversionBaseInst;
class JumpInst;
class JumpTrueInst;
class JumpFalseInst;
class EnterBlockInst;
class ExitBlockInst;
class ContinuousSwitchInst;
class BinarySearchSwitchInst;
class CallInst;
class VirtualCallInst;
class InterfaceCallInst;
class VmCallInst;
class DelegateCallInst;
class ClassDelegateCallInst;
class SetClassDataInst;
class CreateObjectInst;
class CopyObjectInst;
class StrLitToStringInst;
class LoadStringCharInst;
class DupInst;
class SwapInst;
class RotateInst;
class PopInst;
class DownCastInst;
class ThrowInst;
class RethrowInst;
class BeginTryInst;
class EndTryInst;
class BeginCatchSectionInst;
class EndCatchSectionInst;
class BeginCatchInst;
class EndCatchInst;
class BeginFinallyInst;
class EndFinallyInst;
class NextInst;
class StaticInitInst;
class DoneStaticInitInst;
class LoadStaticFieldInst;
class StoreStaticFieldInst;
class EqualObjectNullInst;
class EqualNullObjectInst;
class EqualDlgNullInst;
class EqualNullDlgInst;
class BoxBaseInst;
class UnboxBaseInst;
class AllocateArrayElementsInst;
class IsInst;
class AsInst;
class Fun2DlgInst;
class MemFun2ClassDlgInst;
class CreateLocalVariableReferenceInst;
class CreateMemberVariableReferenceInst;
class LoadVariableReferenceInst;
class StoreVariableReferenceInst;
class GcPollInst;
class RequestGcInst;

class MACHINE_API MachineFunctionVisitor
{
public:
    MachineFunctionVisitor();
    virtual ~MachineFunctionVisitor();
    virtual void BeginVisitFunction(Function& function) {}
    virtual void EndVisitFunction(Function& function) {}
    virtual void BeginVisitInstruction(int instructionNumber, bool prevEndsBasicBlock, Instruction* inst) {}
    virtual void VisitInvalidInst(InvalidInst& instruction) {}
    virtual void VisitNopInst(NopInst& instruction) {}
    virtual void VisitLoadDefaultValueBaseInst(LoadDefaultValueBaseInst& instruction) {}
    virtual void VisitUnaryOpBaseInst(UnaryOpBaseInst& instruction) {}
    virtual void VisitBinaryOpBaseInst(BinaryOpBaseInst& instruction) {}
    virtual void VisitBinaryPredBaseInst(BinaryPredBaseInst& instruction) {}
    virtual void VisitLogicalNotInst(LogicalNotInst& instruction) {}
    virtual void VisitLoadLocalInst(int32_t localIndex) {}
    virtual void VisitStoreLocalInst(int32_t localIndex) {}
    virtual void VisitLoadFieldInst(int32_t fieldIndex, ValueType fieldType) {}
    virtual void VisitStoreFieldInst(int32_t fieldIndex, ValueType fieldType) {}
    virtual void VisitLoadElemInst(LoadElemInst& instruction) {}
    virtual void VisitStoreElemInst(StoreElemInst& instruction) {}
    virtual void VisitLoadConstantInst(int32_t constantIndex) {}
    virtual void VisitReceiveInst(ReceiveInst& instruction) {}
    virtual void VisitConversionBaseInst(ConversionBaseInst& instruction) {}
    virtual void VisitJumpInst(JumpInst& instruction) {}
    virtual void VisitJumpTrueInst(JumpTrueInst& instruction) {}
    virtual void VisitJumpFalseInst(JumpFalseInst& instruction) {}
    virtual void VisitEnterBlockInst(EnterBlockInst& instruction) {}
    virtual void VisitExitBlockInst(ExitBlockInst& instruction) {}
    virtual void VisitContinuousSwitchInst(ContinuousSwitchInst& instruction) {}
    virtual void VisitBinarySearchSwitchInst(BinarySearchSwitchInst& instruction) {}
    virtual void VisitCallInst(CallInst& instruction) {}
    virtual void VisitVirtualCallInst(VirtualCallInst& instruction) {}
    virtual void VisitInterfaceCallInst(InterfaceCallInst& instruction) {}
    virtual void VisitVmCallInst(VmCallInst& instruction) {}
    virtual void VisitDelegateCallInst(DelegateCallInst& instruction) {}
    virtual void VisitClassDelegateCallInst(ClassDelegateCallInst& instruction) {}
    virtual void VisitSetClassDataInst(SetClassDataInst& instruction) {}
    virtual void VisitCreateObjectInst(CreateObjectInst& instruction) {}
    virtual void VisitCopyObjectInst(CopyObjectInst& instruction) {}
    virtual void VisitStrLitToStringInst(StrLitToStringInst& instruction) {}
    virtual void VisitLoadStringCharInst(LoadStringCharInst& instruction) {}
    virtual void VisitDupInst(DupInst& instruction) {}
    virtual void VisitSwapInst(SwapInst& instruction) {}
    virtual void VisitRotateInst(RotateInst& instruction) {}
    virtual void VisitPopInst(PopInst& instruction) {}
    virtual void VisitDownCastInst(DownCastInst& instruction) {}
    virtual void VisitThrowInst(ThrowInst& instruction) {}
    virtual void VisitRethrowInst(RethrowInst& instruction) {}
    virtual void VisitBeginTryInst(BeginTryInst& instruction) {}
    virtual void VisitEndTryInst(EndTryInst& instruction) {}
    virtual void VisitBeginCatchSectionInst(BeginCatchSectionInst& instruction) {}
    virtual void VisitEndCatchSectionInst(EndCatchSectionInst& instruction) {}
    virtual void VisitBeginCatchInst(BeginCatchInst& instruction) {}
    virtual void VisitEndCatchInst(EndCatchInst& instruction) {}
    virtual void VisitBeginFinallyInst(BeginFinallyInst& instruction) {}
    virtual void VisitEndFinallyInst(EndFinallyInst& instruction) {}
    virtual void VisitNextInst(NextInst& instruction) {}
    virtual void VisitStaticInitInst(StaticInitInst& instruction) {}
    virtual void VisitDoneStaticInitInst(DoneStaticInitInst& instruction) {}
    virtual void VisitLoadStaticFieldInst(LoadStaticFieldInst& instruction) {}
    virtual void VisitStoreStaticFieldInst(StoreStaticFieldInst& instruction) {}
    virtual void VisitEqualObjectNullInst(EqualObjectNullInst& instruction) {}
    virtual void VisitEqualNullObjectInst(EqualNullObjectInst& instuction) {}
    virtual void VisitEqualDlgNullInst(EqualDlgNullInst& instruction) {}
    virtual void VisitEqualNullDlgInst(EqualNullDlgInst& instruction) {}
    virtual void VisitBoxBaseInst(BoxBaseInst& instruction) {}
    virtual void VisitUnboxBaseInst(UnboxBaseInst& instruction) {}
    virtual void VisitAllocateArrayElementsInst(AllocateArrayElementsInst& instruction) {}
    virtual void VisitIsInst(IsInst& instruction) {}
    virtual void VisitAsInst(AsInst& instruction) {}
    virtual void VisitFun2DlgInst(Fun2DlgInst& instruction) {}
    virtual void VisitMemFun2ClassDlgInst(MemFun2ClassDlgInst& instruction) {}
    virtual void VisitCreateLocalVariableReferenceInst(CreateLocalVariableReferenceInst& instruction) {}
    virtual void VisitCreateMemberVariableReferenceInst(CreateMemberVariableReferenceInst& instruction) {}
    virtual void VisitLoadVariableReferenceInst(LoadVariableReferenceInst& instruction) {}
    virtual void VisitStoreVariableReferenceInst(StoreVariableReferenceInst& instruction) {}
    virtual void VisitGcPollInst(GcPollInst& instruction) {}
    virtual void VisitRequestGcInst(RequestGcInst& instruction) {}
    void SetCurrentInstructionIndex(int currentInstructionIndex_) { currentInstructionIndex = currentInstructionIndex_; }
    int GetCurrentInstructionIndex() const { return currentInstructionIndex;  }
private:
    int currentInstructionIndex;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_MACHINE_FUNCTION_VISITOR_INCLUDED
