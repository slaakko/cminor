// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/build/NativeCompiler.hpp>
#include <cminor/machine/MachineFunctionVisitor.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/util/Path.hpp>
#include <cminor/util/TextUtils.hpp>
#include <cminor/util/Sha1.hpp>
#include <cminor/symbols/GlobalFlags.hpp>
#include <cminor/symbols/PropertySymbol.hpp>
#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/ast/Project.hpp>
#include <cminor/util/System.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#ifdef _WIN32
#pragma warning(disable:4267)
#pragma warning(disable:4244)
#pragma warning(disable:4141)
#pragma warning(disable:4624)
#pragma warning(disable:4291)
#endif
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Mangler.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Analysis/TargetTransformInfo.h>
#include <llvm/Analysis/TargetLibraryInfo.h>
#ifdef _WIN32
#pragma warning(default:4267)
#pragma warning(default:4244)
#pragma warning(default:4141)
#pragma warning(default:4624)
#pragma warning(default:4291)
#endif

namespace cminor { namespace build {

using namespace llvm;
using namespace cminor::machine;
using namespace cminor::util;

typedef llvm::SmallVector<llvm::Value*, 4> ArgVector;

class ValueStack
{
public:
    void Push(llvm::Value* value)
    {
        s.push_back(value);
    }
    llvm::Value* Pop()
    {
        Assert(!s.empty(), "value stack is empty");
        llvm::Value* top = s.back();
        s.pop_back();
        return top;
    }
    void Dup()
    {
        s.push_back(s.back());
    }
    void Swap()
    {
        std::swap(s.back(), s[s.size() - 2]);
    }
    void Rotate()
    {
        std::swap(s[s.size() - 3], s[s.size() - 2]);
        std::swap(s.back(), s[s.size() - 2]);
    }
private:
    std::vector<llvm::Value*> s;
};

enum class LlvmBinOp : uint8_t
{
    add, fadd, sub, fsub, mul, fmul, udiv, sdiv, fdiv, urem, srem, shl, lshr, ashr, and_, or_, xor_
};

enum class LlvmBinPred : uint8_t
{
    ieq, oeq, ult, slt, olt
};

enum class LlvmConv : uint8_t
{
    none, zext, sext, trunc, bitcast, fptoui, fptosi, uitofp, sitofp, fpext, fptrunc
};

enum class LlvmPadKind : uint8_t
{
    none, catchSwitch, catchPad, cleanupPad
};

class NativeCompilerImpl : public MachineFunctionVisitor
{
public:
    NativeCompilerImpl();
    void Init(Assembly& assembly);
    void Done();
    void CreateDllMain();
    void GenerateObjectFile(const std::string& assemblyObjectFilePath);
    void GenerateAsmFile(const std::string& asmFilePath);
    void Link(const std::string& assemblyObjectFilePath);
    void LinkWindows(const std::string& assemblyObjectFilePath);
    void BeginVisitFunction(Function& function) override;
    void EndVisitFunction(Function& function) override;
    void BeginVisitInstruction(int instructionNumber, bool prevEndsBasicBlock, Instruction* inst) override;
    void VisitInvalidInst(InvalidInst& instruction) override;
    void VisitLoadDefaultValueBaseInst(LoadDefaultValueBaseInst& instruction) override;
    void VisitUnaryOpBaseInst(UnaryOpBaseInst& instruction) override;
    void VisitBinaryOpBaseInst(BinaryOpBaseInst& instruction) override;
    void VisitBinaryPredBaseInst(BinaryPredBaseInst& instruction) override;
    void VisitLogicalNotInst(LogicalNotInst& instruction) override;
    void VisitLoadLocalInst(int32_t localIndex) override;
    void VisitStoreLocalInst(int32_t localIndex) override;
    void VisitLoadFieldInst(int32_t fieldIndex, ValueType fieldType) override;
    void VisitStoreFieldInst(int32_t fieldIndex, ValueType fieldType) override;
    void VisitLoadElemInst(LoadElemInst& instruction) override;
    void VisitStoreElemInst(StoreElemInst& instruction) override;
    void VisitLoadConstantInst(int32_t constantIndex) override;
    void VisitReceiveInst(ReceiveInst& instruction) override;
    void VisitConversionBaseInst(ConversionBaseInst& instruction) override;
    void VisitJumpInst(JumpInst& instruction) override;
    void VisitJumpTrueInst(JumpTrueInst& instruction) override;
    void VisitJumpFalseInst(JumpFalseInst& instruction) override;
    void VisitContinuousSwitchInst(ContinuousSwitchInst& instruction) override;
    void VisitBinarySearchSwitchInst(BinarySearchSwitchInst& instruction) override;
    void VisitCallInst(CallInst& instruction) override;
    void VisitVirtualCallInst(VirtualCallInst& instruction) override;
    void VisitInterfaceCallInst(InterfaceCallInst& instruction) override;
    void VisitVmCallInst(VmCallInst& instruction) override;
    void VisitDelegateCallInst(DelegateCallInst& instruction) override;
    void VisitClassDelegateCallInst(ClassDelegateCallInst& instruction) override;
    void VisitSetClassDataInst(SetClassDataInst& instruction) override;
    void VisitCreateObjectInst(CreateObjectInst& instruction) override;
    void VisitCopyObjectInst(CopyObjectInst& instruction) override;
    void VisitStrLitToStringInst(StrLitToStringInst& instruction) override;
    void VisitLoadStringCharInst(LoadStringCharInst& instruction) override;
    void VisitDupInst(DupInst& instruction) override;
    void VisitSwapInst(SwapInst& instruction) override;
    void VisitRotateInst(RotateInst& instruction) override;
    void VisitPopInst(PopInst& instruction) override;
    void VisitDownCastInst(DownCastInst& instruction) override;
    void VisitThrowInst(ThrowInst& instruction) override;
    void VisitRethrowInst(RethrowInst& instruction) override;
    void VisitBeginTryInst(BeginTryInst& instruction) override;
    void VisitEndTryInst(EndTryInst& instruction) override;
    void VisitBeginCatchSectionInst(BeginCatchSectionInst& instruction) override;
    void VisitEndCatchSectionInst(EndCatchSectionInst& instruction) override;
    void VisitBeginCatchInst(BeginCatchInst& instruction) override;
    void VisitEndCatchInst(EndCatchInst& instruction) override;
    void VisitBeginFinallyInst(BeginFinallyInst& instruction) override;
    void VisitEndFinallyInst(EndFinallyInst& instruction) override;
    void VisitStaticInitInst(StaticInitInst& instruction) override;
    void VisitDoneStaticInitInst(DoneStaticInitInst& instruction) override;
    void VisitLoadStaticFieldInst(LoadStaticFieldInst& instruction) override;
    void VisitStoreStaticFieldInst(StoreStaticFieldInst& instruction) override;
    void VisitEqualObjectNullInst(EqualObjectNullInst& instruction) override;
    void VisitEqualNullObjectInst(EqualNullObjectInst& instuction) override;
    void VisitBoxBaseInst(BoxBaseInst& instruction) override;
    void VisitUnboxBaseInst(UnboxBaseInst& instruction) override;
    void VisitAllocateArrayElementsInst(AllocateArrayElementsInst& instruction) override;
    void VisitIsInst(IsInst& instruction) override;
    void VisitAsInst(AsInst& instruction) override;
    void VisitFun2DlgInst(Fun2DlgInst& instruction) override;
    void VisitMemFun2ClassDlgInst(MemFun2ClassDlgInst& instruction) override;
    void VisitCreateLocalVariableReferenceInst(CreateLocalVariableReferenceInst& instruction) override;
    void VisitCreateMemberVariableReferenceInst(CreateMemberVariableReferenceInst& instruction) override;
    void VisitLoadVariableReferenceInst(LoadVariableReferenceInst& instruction) override;
    void VisitStoreVariableReferenceInst(StoreVariableReferenceInst& instruction) override;
private:
    Assembly* assembly;
    LLVMContext context;
    IRBuilder<> builder;
    std::unique_ptr<Module> module;
    std::unique_ptr<TargetMachine> targetMachine;
    std::unique_ptr<DataLayout> dataLayout;
    llvm::AttributeSet unwindFunctionAttributes;
    llvm::AttributeSet nounwindFunctionAttributes;
    llvm::AttributeSet mainFunctionAttributes;
    llvm::AttributeSet noReturnFunctionAttributes;
    int inlineLimit;
    llvm::Function* fun;
    Function* function;
    int32_t nextFunctionVarNumber;
    int32_t nextClassDataVarNumber;
    int32_t nextTypePtrVarNumber;
    std::string functionPtrVarName;
    llvm::Constant* functionPtrVar;
    ConstantPool* constantPool;
    llvm::Constant* constantPoolVariable;
    std::vector<AllocaInst*> locals;
    llvm::AllocaInst* functionStackEntry;
    std::unordered_map<std::string, LlvmBinOp> binOpMap;
    std::unordered_map<std::string, LlvmBinPred> binPredMap;
    std::unordered_map<std::string, LlvmConv> convMap;
    std::unordered_map<StringPtr, std::string, StringPtrHash> opFunMap;
    ValueStack valueStack;
    llvm::BasicBlock* currentBasicBlock;
    llvm::BasicBlock* entryBasicBlock;
    llvm::AllocaInst* lastAlloca;
    std::unordered_map<int, llvm::BasicBlock*> basicBlocks;
    std::unordered_map<int, llvm::BasicBlock*> unwindTargets;
    llvm::Value* currentPad;
    std::vector<llvm::Value*> padStack;
    LlvmPadKind currentPadKind;
    std::vector<LlvmPadKind> padKindStack;
    std::unordered_map<int, llvm::Value*> exceptionObjectVariables;
    llvm::Value* exceptionPtr;
    std::unordered_map<int, llvm::BasicBlock*> nextHandler;
    std::unordered_set<int32_t> jumpTargets;
    std::string listFilePath;
    std::unique_ptr<std::ofstream> listStream;
    std::unordered_map<ClassData*, llvm::Constant*> classDataMap;
    std::unordered_map<Type*, llvm::Constant*> typeMap;
    std::unordered_map<Function*, llvm::Constant*> functionPtrMap;
    std::unordered_map<std::string, llvm::Value*> globalStringPtrMap;
    std::vector<int> exceptionBlockIdStack;
    int currentExceptionBlockId;
    std::vector<int> catchSectionExceptionBlockIdStack;
    int currentCatchSectionExceptionBlockId;
    llvm::TargetOptions GetTargetOptions();
    void SetPersonalityFunction();
    void InsertAllocaIntoEntryBlock(llvm::AllocaInst* allocaInst);
    void CreateCall(llvm::Value* callee, const ArgVector& args, bool callReturnsValue);
    void CreateReturnFromFunctionOrFunclet();
    void CreateCatchPad();
    void CreateCatchPadWindows();
    void GenerateRethrow();
    void GenerateRethrowWindows();
    void ExportGlobalVariable(llvm::Constant* globalVariable);
    void ExportFunction(llvm::Function* function);
    void ImportFunction(llvm::Function* function);
    llvm::Constant* GetClassDataPtrVar(ClassData* classData);
    llvm::Constant* GetTypePtrVar(Type* type);
    llvm::Constant* GetFunctionPtrVar(Function* function);
    void CreateEnterFunctionCall();
    void CreateLeaveFunctionCall();
    void SetCurrentLineNumber(uint32_t lineNumber);
    void InitMaps();
    void InitOpFunMap(ConstantPool& constantPool);
    std::string MangleFunctionName(const Function& function) const;
    llvm::Type* GetType(ValueType type);
    llvm::IntegerType* GetIntegerType(ValueType type);
    bool IsSignedType(ValueType type) const;
    llvm::ConstantInt* GetConstantInt(ValueType type, IntegralValue value);
    void PushDefaultValue(ValueType type);
    void PushIntegerMinusOne(ValueType type);
    LlvmConv GetConversionToULongFrom(ValueType type) const;
    llvm::Value* CreateConversionToUlong(llvm::Value* from, ValueType fromType);
    LlvmConv GetConversionFromULong(ValueType type) const;
    llvm::Value* CreateConversionFromULong(llvm::Value* from, ValueType toType);
};    

NativeCompilerImpl::NativeCompilerImpl() : assembly(nullptr), builder(context), module(), inlineLimit(0), targetMachine(), fun(nullptr), function(nullptr), constantPool(nullptr), 
    constantPoolVariable(nullptr), nextFunctionVarNumber(0), nextClassDataVarNumber(0), nextTypePtrVarNumber(0), functionStackEntry(nullptr), currentBasicBlock(nullptr), 
    entryBasicBlock(nullptr), lastAlloca(nullptr), currentExceptionBlockId(-1), currentCatchSectionExceptionBlockId(-1), currentPad(nullptr), currentPadKind(LlvmPadKind::none), 
    exceptionPtr(nullptr)
{
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();
    PassRegistry* passRegistry = PassRegistry::getPassRegistry();
    initializeCore(*passRegistry);
    initializeScalarOpts(*passRegistry);
    initializeVectorization(*passRegistry);
    initializeIPO(*passRegistry);
    initializeAnalysis(*passRegistry);
    initializeTransformUtils(*passRegistry);
    initializeInstCombine(*passRegistry);
    initializeInstrumentation(*passRegistry);
    initializeTarget(*passRegistry);
    initializeCodeGen(*passRegistry);
    InitMaps();
}

std::string NativeCompilerImpl::MangleFunctionName(const Function& function) const
{
    std::string mangledName = ToUtf8(function.GroupName().Value().AsStringLiteral());
    if (mangledName == "main")
    {
        return "main_entry_point";
    }
    if (mangledName.find("operator") != std::string::npos)
    {
        auto it = opFunMap.find(StringPtr(function.GroupName().Value().AsStringLiteral()));
        if (it != opFunMap.cend())
        {
            mangledName = it->second;
        }
        else
        {
            mangledName = "operator";
        }
    }
    else
    {
        mangledName.erase(std::remove(mangledName.begin(), mangledName.end(), '@'), mangledName.end());
    }
    if (!function.FunctionSymbol())
    {
        throw std::runtime_error("NativeCompiler: function symbol address for function '" + ToUtf8(function.CallName().Value().AsStringLiteral()) + "' not set");
    }
    FunctionSymbol* functionSymbol = static_cast<FunctionSymbol*>(function.FunctionSymbol());
    SymbolType symbolType = functionSymbol->GetSymbolType();
    switch (symbolType)
    {
        case SymbolType::staticConstructorSymbol: case SymbolType::constructorSymbol: case SymbolType::memberFunctionSymbol:
        {
            Symbol* parentClass = functionSymbol->Parent();
            mangledName.append("_").append(ToUtf8(parentClass->SimpleName()));
            break;
        }
        case SymbolType::arraySizeConstructorSymbol:
        {
            Symbol* parentArray = functionSymbol->Parent();
            mangledName.append("_").append(ToUtf8(parentArray->SimpleName()));
            break;
        }
        case SymbolType::propertyGetterSymbol:  case SymbolType::propertySetterSymbol:
        {
            Symbol* parentProperty = functionSymbol->Parent();
            Symbol* parentClass = parentProperty->Parent();
            mangledName.append("_").append(ToUtf8(parentClass->SimpleName())).append("_").append(ToUtf8(parentProperty->Name().Value()));
            break;
        }
        case SymbolType::indexerGetterSymbol: case SymbolType::indexerSetterSymbol:
        {
            Symbol* parentClass = functionSymbol->Parent()->Parent();
            mangledName.append("_indexer").append("_").append(ToUtf8(parentClass->SimpleName()));
            break;
        }
    }
    mangledName.append(1, '_').append(GetSha1MessageDigest(ToUtf8(function.CallName().Value().AsStringLiteral())));
    return mangledName;
}

llvm::TargetOptions NativeCompilerImpl::GetTargetOptions()
{
    llvm::TargetOptions targetOptions = {};
#ifdef _WIN32
    targetOptions.ExceptionModel = llvm::ExceptionHandling::WinEH;
#endif
    return targetOptions;
}

void NativeCompilerImpl::SetPersonalityFunction()
{
#ifdef _WIN32
    llvm::FunctionType* personalityFunctionType = llvm::FunctionType::get(GetType(ValueType::intType), true);
    llvm::Function* personalityFunction = cast<llvm::Function>(module->getOrInsertFunction("__CxxFrameHandler3", personalityFunctionType));
    ImportFunction(personalityFunction);
    fun->setPersonalityFn(llvm::ConstantExpr::getBitCast(personalityFunction, PointerType::get(GetType(ValueType::byteType), 0)));
#endif
}

void NativeCompilerImpl::InsertAllocaIntoEntryBlock(llvm::AllocaInst* allocaInst)
{
    if (lastAlloca)
    {
        allocaInst->insertAfter(lastAlloca);
    }
    else
    {
        if (entryBasicBlock->empty())
        {
            entryBasicBlock->getInstList().push_back(allocaInst);
        }
        else
        {
            entryBasicBlock->getInstList().insert(entryBasicBlock->getInstList().begin(), allocaInst);
        }
    }
}

void NativeCompilerImpl::CreateCall(llvm::Value* callee, const ArgVector& args, bool callReturnsValue)
{
    std::vector<OperandBundleDef> bundles;
    if (currentPad != nullptr)
    {
        std::vector<llvm::Value*> inputs;
        inputs.push_back(currentPad);
        bundles.push_back(OperandBundleDef("funclet", inputs));
    }
    if (currentExceptionBlockId == -1)
    {
        if (callReturnsValue)
        {
            if (currentPad == nullptr)
            {
                valueStack.Push(builder.CreateCall(callee, args));
            }
            else
            {
                if (!currentBasicBlock)
                {
                    throw std::runtime_error("current basic block not set");
                }
                valueStack.Push(llvm::CallInst::Create(callee, args, bundles, "", currentBasicBlock));
            }
        }
        else
        {
            if (currentPad == nullptr)
            {
                builder.CreateCall(callee, args);
            }
            else
            {
                if (!currentBasicBlock)
                {
                    throw std::runtime_error("current basic block not set");
                }
                llvm::CallInst::Create(callee, args, bundles, "", currentBasicBlock);
            }
        }
    }
    else
    {
        ExceptionBlock* exceptionBlock = function->GetExceptionBlock(currentExceptionBlockId);
        if (!exceptionBlock)
        {
            throw std::runtime_error("exception block " + std::to_string(currentExceptionBlockId) + " not found");
        }
        llvm::BasicBlock* normalTarget = BasicBlock::Create(context, "continueTarget" + std::to_string(GetCurrentInstructionIndex()), fun);
        llvm::BasicBlock* unwindTarget = nullptr;
        auto it = unwindTargets.find(currentExceptionBlockId);
        if (it != unwindTargets.cend())
        {
            unwindTarget = it->second;
        }
        else
        {
            if (!exceptionBlock->CatchBlocks().empty())
            {
                unwindTarget = BasicBlock::Create(context, "catchswitch" + std::to_string(currentExceptionBlockId), fun);
            }
            else
            {
                unwindTarget = BasicBlock::Create(context, "cleanuppad" + std::to_string(currentExceptionBlockId), fun);
            }
            unwindTargets[currentExceptionBlockId] = unwindTarget;
        }
        if (callReturnsValue)
        {
            if (currentPad == nullptr)
            {
                valueStack.Push(builder.CreateInvoke(callee, normalTarget, unwindTarget, args));
            }
            else
            {
                if (!currentBasicBlock)
                {
                    throw std::runtime_error("current basic block not set");
                }
                valueStack.Push(llvm::InvokeInst::Create(callee, normalTarget, unwindTarget, args, bundles, "", currentBasicBlock));
            }
        }
        else
        {
            if (currentPad == nullptr)
            {
                builder.CreateInvoke(callee, normalTarget, unwindTarget, args);
            }
            else
            {
                if (!currentBasicBlock)
                {
                    throw std::runtime_error("current basic block not set");
                }
                llvm::InvokeInst::Create(callee, normalTarget, unwindTarget, args, bundles, "", currentBasicBlock);
            }
        }
        currentBasicBlock = normalTarget;
        builder.SetInsertPoint(currentBasicBlock);
    }
}

void NativeCompilerImpl::CreateReturnFromFunctionOrFunclet()
{
    switch (currentPadKind)
    {
        case LlvmPadKind::none:
        {
            Assert(currentPad == nullptr, "currentPadKind and currentPad not in sync");
            if (function->ReturnsValue())
            {
                llvm::Value* retValue = valueStack.Pop();
                builder.CreateRet(retValue);
            }
            else
            {
                builder.CreateRetVoid();
            }
            break;
        }
        case LlvmPadKind::catchSwitch:
        {
            throw std::runtime_error("cannot create ret from catchswitch");
        }
        case LlvmPadKind::catchPad:
        {
            if (currentPad == nullptr)
            {
                throw std::runtime_error("current pad not set");
            }
            if (currentCatchSectionExceptionBlockId == -1)
            {
                throw std::runtime_error("not in catch section");
            }
            ExceptionBlock* exceptionBlock = function->GetExceptionBlock(currentCatchSectionExceptionBlockId);
            if (!exceptionBlock)
            {
                throw std::runtime_error("exception block " + std::to_string(currentCatchSectionExceptionBlockId) + " not found");
            }
            llvm::BasicBlock* returnTarget = llvm::BasicBlock::Create(context, "return" + std::to_string(currentCatchSectionExceptionBlockId), fun);
            llvm::AllocaInst* returnValueAlloca = nullptr;
            if (function->ReturnsValue())
            {
                returnValueAlloca = new llvm::AllocaInst(GetType(function->ReturnType()));
                InsertAllocaIntoEntryBlock(returnValueAlloca);
                builder.CreateStore(valueStack.Pop(), returnValueAlloca);
            }
            builder.CreateCatchRet(cast<llvm::CatchPadInst>(currentPad), returnTarget);
            builder.SetInsertPoint(returnTarget);
            CreateLeaveFunctionCall();
            if (function->ReturnsValue())
            {
                builder.CreateRet(builder.CreateLoad(returnValueAlloca));
            }
            else
            {
                builder.CreateRetVoid();
            }
            currentBasicBlock = nullptr;
            break;
        }
        case LlvmPadKind::cleanupPad:
        {
            if (currentPad == nullptr)
            {
                throw std::runtime_error("current pad not set");
            }
            builder.CreateCleanupRet(cast<llvm::CleanupPadInst>(currentPad));
            break;
        }
    }
}

void NativeCompilerImpl::CreateCatchPad()
{
#ifdef _WIN32
    CreateCatchPadWindows();
#endif
}

void NativeCompilerImpl::CreateCatchPadWindows()
{
    ArgVector catchPadArgs;
    llvm::Constant* cminorExceptionTypeDescriptorVar = module->getOrInsertGlobal("??_R0?AVCminorException@machine@cminor@@@8", PointerType::get(GetType(ValueType::byteType), 0));
    catchPadArgs.push_back(cminorExceptionTypeDescriptorVar);
    catchPadArgs.push_back(builder.getInt32(8));
    auto it = exceptionObjectVariables.find(currentCatchSectionExceptionBlockId);
    if (it != exceptionObjectVariables.cend())
    {
        llvm::Value* exceptionObjectVariable = it->second;
        catchPadArgs.push_back(exceptionObjectVariable);
    }
    else
    {
        throw std::runtime_error("exception object variable for exception block " + std::to_string(currentCatchSectionExceptionBlockId) + " not found");
    }
    llvm::CatchPadInst* catchPad = builder.CreateCatchPad(currentPad, catchPadArgs);
    padStack.push_back(currentPad);
    currentPad = catchPad;
    padKindStack.push_back(currentPadKind);
    currentPadKind = LlvmPadKind::catchPad;
}

void NativeCompilerImpl::GenerateRethrow()
{
#ifdef _WIN32
    GenerateRethrowWindows();
#endif
}

void NativeCompilerImpl::GenerateRethrowWindows()
{
    llvm::Function* cxxThrowFunction = cast<llvm::Function>(module->getOrInsertFunction("_CxxThrowException", GetType(ValueType::none), 
        PointerType::get(GetType(ValueType::byteType), 0), 
        PointerType::get(GetType(ValueType::byteType), 0), 
        nullptr));
    ImportFunction(cxxThrowFunction);
    ArgVector args;
    args.push_back(llvm::Constant::getNullValue(PointerType::get(GetType(ValueType::byteType), 0)));
    args.push_back(llvm::Constant::getNullValue(PointerType::get(GetType(ValueType::byteType), 0)));
    if (currentPad != nullptr)
    {
        std::vector<OperandBundleDef> bundles;
        std::vector<llvm::Value*> inputs;
        inputs.push_back(currentPad);
        bundles.push_back(OperandBundleDef("funclet", inputs));
        if (!currentBasicBlock)
        {
            throw std::runtime_error("current basic block not set");
        }
        llvm::CallInst::Create(cxxThrowFunction, args, bundles, "", currentBasicBlock);
    }
    else
    {
        builder.CreateCall(cxxThrowFunction, args);
    }
}

void NativeCompilerImpl::ExportGlobalVariable(llvm::Constant* globalVariable)
{
#ifdef _WIN32
    llvm::GlobalValue* globalValue = cast<llvm::GlobalValue>(globalVariable);
    globalValue->setDLLStorageClass(llvm::GlobalValue::DLLExportStorageClass);
#endif
}

void NativeCompilerImpl::ExportFunction(llvm::Function* function)
{
    function->setLinkage(llvm::GlobalValue::LinkageTypes::ExternalLinkage);
#ifdef _WIN32
    function->setDLLStorageClass(llvm::GlobalValue::DLLExportStorageClass);
#endif
}

void NativeCompilerImpl::ImportFunction(llvm::Function* function)
{
    function->setLinkage(llvm::GlobalValue::LinkageTypes::ExternalLinkage);
#ifdef _WIN32
    function->setDLLStorageClass(llvm::GlobalValue::DLLImportStorageClass);
#endif
}

llvm::Constant* NativeCompilerImpl::GetClassDataPtrVar(ClassData* classData)
{
    auto it = classDataMap.find(classData);
    if (it != classDataMap.cend())
    {
        return it->second;
    }
    std::string classDataPtrVarName = "__CD" + std::to_string(nextClassDataVarNumber++);
    utf32_string classDataPtrVarNameUtf32 = ToUtf32(classDataPtrVarName);
    Constant classDataPtrVarNameConstant = assembly->GetConstantPool().GetConstant(assembly->GetConstantPool().Install(StringPtr(classDataPtrVarNameUtf32.c_str())));
    Constant classDataNameConstant = assembly->GetConstantPool().GetConstant(assembly->GetConstantPool().Install(classData->Type()->Name()));
    assembly->AddClassDataVarMapping(classDataPtrVarNameConstant, classDataNameConstant);
    llvm::Constant* classDataPtrVar = module->getOrInsertGlobal(classDataPtrVarName, PointerType::get(GetType(ValueType::byteType), 0));
    ExportGlobalVariable(classDataPtrVar);
    llvm::GlobalVariable* clsDataPtrVar = cast<llvm::GlobalVariable>(classDataPtrVar);
    clsDataPtrVar->setInitializer(llvm::Constant::getNullValue(PointerType::get(GetType(ValueType::byteType), 0)));
    classDataMap[classData] = classDataPtrVar;
    return classDataPtrVar;
}

llvm::Constant* NativeCompilerImpl::GetTypePtrVar(Type* type)
{
    auto it = typeMap.find(type);
    if (it != typeMap.cend())
    {
        return it->second;
    }
    std::string typePtrVarName = "__T" + std::to_string(nextTypePtrVarNumber++);
    utf32_string typePtrVarNameUtf32 = ToUtf32(typePtrVarName);
    Constant typePtrVarNameConstant = assembly->GetConstantPool().GetConstant(assembly->GetConstantPool().Install(StringPtr(typePtrVarNameUtf32.c_str())));
    Constant typeNameConstant = assembly->GetConstantPool().GetConstant(assembly->GetConstantPool().Install(type->Name()));
    assembly->AddTypePtrVarMapping(typePtrVarNameConstant, typeNameConstant);
    llvm::Constant* typePtrVar = module->getOrInsertGlobal(typePtrVarName, PointerType::get(GetType(ValueType::byteType), 0));
    ExportGlobalVariable(typePtrVar);
    llvm::GlobalVariable* tpPtrVar = cast<llvm::GlobalVariable>(typePtrVar);
    tpPtrVar->setInitializer(llvm::Constant::getNullValue(PointerType::get(GetType(ValueType::byteType), 0)));
    typeMap[type] = typePtrVar;
    return typePtrVar;
}

llvm::Constant* NativeCompilerImpl::GetFunctionPtrVar(Function* function)
{
    auto it = functionPtrMap.find(function);
    if (it != functionPtrMap.cend())
    {
        return it->second;
    }
    std::string functionPtrVarName = "__F" + std::to_string(nextFunctionVarNumber++);
    utf32_string functionPtrVarNameUtf32 = ToUtf32(functionPtrVarName);
    Constant functionPtrVarNameConstant = assembly->GetConstantPool().GetConstant(assembly->GetConstantPool().Install(StringPtr(functionPtrVarNameUtf32.c_str())));
    Constant functionNameConstant = assembly->GetConstantPool().GetConstant(assembly->GetConstantPool().Install(function->CallName()));
    assembly->AddFunctionVarMapping(functionPtrVarNameConstant, functionNameConstant);
    llvm::Constant* functionPtrVar = module->getOrInsertGlobal(functionPtrVarName, PointerType::get(GetType(ValueType::byteType), 0));
    ExportGlobalVariable(functionPtrVar);
    llvm::GlobalVariable* funPtrVar = cast<llvm::GlobalVariable>(functionPtrVar);
    funPtrVar->setInitializer(llvm::Constant::getNullValue(PointerType::get(GetType(ValueType::byteType), 0)));
    functionPtrMap[function] = functionPtrVar;
    return functionPtrVar;
}

void NativeCompilerImpl::Init(Assembly& assembly)
{
    std::string assemblyName = ToUtf8(assembly.Name().Value());
    for (const std::unique_ptr<Assembly>& referencedAssembly : assembly.ReferencedAssemblies())
    {
        if (referencedAssembly->IsCore()) continue;
        if (!referencedAssembly->IsNative())
        {
            throw std::runtime_error("NativeCompiler: cannot compile assembly '" + assemblyName + "' (" + assembly.FilePathReadFrom() +
                ") to native object code because referenced assembly '" + ToUtf8(referencedAssembly->Name().Value()) + "' (" + referencedAssembly->FilePathReadFrom() +
                ") is not compiled to native object code (build with --native).");
        }
        if (GetGlobalFlag(GlobalFlags::linkWithDebugMachine))
        {
            if (!referencedAssembly->LinkedWithDebugMachine())
            {
                throw std::runtime_error("NativeCompiler: trying to link current assembly '" + assemblyName + "' (" + assembly.FilePathReadFrom() +
                    ") with debug version of the Cminor virtual machine (cminormachined.dll) while referenced assembly '" +
                    ToUtf8(referencedAssembly->Name().Value()) + "' (" + referencedAssembly->FilePathReadFrom() + ") is linked with release version of the Cminor virtual machine (cminormachine.dll). " +
                    "Either build current assembly without the --link-with-debug-machine option, or build referenced assemblies with the --link-with-debug-machine option.");
            }
        }
        else
        {
            if (referencedAssembly->LinkedWithDebugMachine())
            {
                throw std::runtime_error("NativeCompiler: trying to link current assembly '" + assemblyName + "' (" + assembly.FilePathReadFrom() +
                    ") with release version of the Cminor virtual machine (cminormachine.dll) while referenced assembly '" +
                    ToUtf8(referencedAssembly->Name().Value()) + "' (" + referencedAssembly->FilePathReadFrom() + ") is linked with debug version of the Cminor virtual machine (cminormachined.dll). " +
                    "Either build current assembly with the --link-with-debug-machine option, or build referenced assemblies without the --link-with-debug-machine option.");
            }
        }
    }
    this->assembly = &assembly;
    InitOpFunMap(assembly.GetConstantPool());
    module.reset(new Module(assemblyName, context));
    std::string targetTriple = sys::getDefaultTargetTriple();
    module->setTargetTriple(targetTriple);
    assembly.SetNativeTargetTriple(targetTriple);
    std::string error;
    const llvm::Target* target = TargetRegistry::lookupTarget(targetTriple, error);
    if (!target)
    {
        throw std::runtime_error("NativeCompiler: TargetRegistry::lookupTarget failed: " + error);
    }
    llvm::TargetOptions targetOptions = GetTargetOptions();
    llvm::Optional<Reloc::Model> relocModel;
    llvm::CodeModel::Model codeModel = CodeModel::Default;
    llvm::CodeGenOpt::Level codeGenLevel = CodeGenOpt::None;
    int optLevel = GetOptimizationLevel();
    switch (optLevel)
    {
        case 0: codeGenLevel = CodeGenOpt::None; break;
        case 1: codeGenLevel = CodeGenOpt::Less; break;
        case 2: codeGenLevel = CodeGenOpt::Default; break;
        case 3: codeGenLevel = CodeGenOpt::Aggressive; break;
    }
    inlineLimit = GetInlineLimit();
    targetMachine.reset(target->createTargetMachine(targetTriple, "generic", "", targetOptions, relocModel, codeModel, codeGenLevel));
    dataLayout.reset(new llvm::DataLayout(targetMachine->createDataLayout()));
    module->setDataLayout(*dataLayout);
    if (GetGlobalFlag(GlobalFlags::list))
    {
        listFilePath = boost::filesystem::path(assembly.FilePathReadFrom()).replace_extension(".list").generic_string();
        if (GetGlobalFlag(GlobalFlags::verbose))
        {
            std::cout << "Generating listing to " << listFilePath << "..." << std::endl;
        }
        listStream.reset(new std::ofstream(listFilePath));
        *listStream << "MODULE " << assemblyName << " : " << assembly.FilePathReadFrom() << std::endl;
        *listStream << "target triple: " << targetTriple << std::endl;
        *listStream << "optimization level: " << optLevel << std::endl;
        *listStream << "inline limit: " << inlineLimit << " or fewer intermediate instructions (0 = no inlining)" << std::endl;
    }
    constantPoolVariable = module->getOrInsertGlobal("__constant_pool", PointerType::get(GetType(ValueType::byteType), 0));
    ExportGlobalVariable(constantPoolVariable);
    llvm::GlobalVariable* constantPoolVar = cast<llvm::GlobalVariable>(constantPoolVariable);
    constantPoolVar->setInitializer(llvm::Constant::getNullValue(PointerType::get(GetType(ValueType::byteType), 0)));
    nextFunctionVarNumber = 0;
    std::vector<Attribute::AttrKind> nounwindAttributes;
    nounwindAttributes.push_back(llvm::Attribute::UWTable);
    nounwindAttributes.push_back(llvm::Attribute::NoUnwind);
    nounwindFunctionAttributes = AttributeSet::get(context, llvm::AttributeSet::FunctionIndex, nounwindAttributes);
    std::vector<Attribute::AttrKind> unwindAttributes;
    unwindAttributes.push_back(llvm::Attribute::UWTable);
    unwindFunctionAttributes = AttributeSet::get(context, llvm::AttributeSet::FunctionIndex, unwindAttributes);
    std::vector<Attribute::AttrKind> mainAttributes;
    mainAttributes.push_back(llvm::Attribute::UWTable);
    mainAttributes.push_back(llvm::Attribute::NoRecurse);
    mainFunctionAttributes = AttributeSet::get(context, llvm::AttributeSet::FunctionIndex, mainAttributes);
    std::vector<Attribute::AttrKind> noReturnAttributes;
    noReturnAttributes.push_back(llvm::Attribute::UWTable);
    noReturnAttributes.push_back(llvm::Attribute::NoReturn);
    noReturnFunctionAttributes = AttributeSet::get(context, llvm::AttributeSet::FunctionIndex, noReturnAttributes);
}

void NativeCompilerImpl::Done()
{
#ifdef _WIN32
    CreateDllMain();
#endif
    std::string assemblyName = ToUtf8(assembly->Name().Value());
    std::string errorMessage;
    llvm::raw_string_ostream errorOs(errorMessage);
    if (verifyModule(*module, &errorOs))
    {
        errorOs.str(); // flush contents of errorOs to errorMessage
        if (listStream)
        {
            *listStream << "NativeCompiler: verification of module '" << assemblyName + "' failed. " << errorMessage << std::endl;
        }
        throw std::runtime_error("NativeCompiler: verification of module '" + assemblyName + "' failed. " + errorMessage);
    }
    std::string llFilePath = boost::filesystem::path(assembly->FilePathReadFrom()).replace_extension(".ll").generic_string();
    if (GetGlobalFlag(GlobalFlags::emitLlvm))
    {
        if (GetGlobalFlag(GlobalFlags::verbose))
        {
            std::cout << "Emitting LLVM intermediate code to " << llFilePath << "..." << std::endl;
        }
        std::ofstream llFile(llFilePath);
        llvm::raw_os_ostream llOs(llFile);
        module->print(llOs, nullptr);
    }
#ifdef _WIN32
    std::string assemblyObjectFilePath = boost::filesystem::path(assembly->FilePathReadFrom()).replace_extension(".obj").generic_string();
#else
    std::string assemblyObjectFilePath = boost::filesystem::path(assembly->FilePathReadFrom()).replace_extension(".o").generic_string();
#endif
    GenerateObjectFile(assemblyObjectFilePath);
    std::string optLlFilePath = boost::filesystem::path(assembly->FilePathReadFrom()).replace_extension(".opt.ll").generic_string();
    if (GetGlobalFlag(GlobalFlags::emitOptLlvm))
    {
        if (GetGlobalFlag(GlobalFlags::verbose))
        {
            std::cout << "Emitting optimized LLVM intermediate code to " << optLlFilePath << "..." << std::endl;
        }
        std::ofstream optLlFile(optLlFilePath);
        llvm::raw_os_ostream optLlOs(optLlFile);
        module->print(optLlOs, nullptr);
    }
    Link(assemblyObjectFilePath);
}

void NativeCompilerImpl::CreateDllMain()
{
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "Creating DllMain..." << std::endl;
    }
    llvm::Function* dllMain = cast<llvm::Function>(module->getOrInsertFunction("DllMain", GetType(ValueType::intType),
        PointerType::get(GetType(ValueType::byteType), 0), 
        GetType(ValueType::ulongType), 
        PointerType::get(GetType(ValueType::byteType), 0),
        nullptr));
    ExportFunction(dllMain);
    dllMain->setCallingConv(llvm::CallingConv::X86_64_Win64);
    dllMain->setAttributes(nounwindFunctionAttributes);
    BasicBlock* entryBlock = BasicBlock::Create(context, "entry", dllMain);
    builder.SetInsertPoint(entryBlock);
    builder.CreateRet(builder.getInt32(1));
    std::unique_ptr<llvm::raw_os_ostream> os;
    if (listStream)
    {
        os.reset(new llvm::raw_os_ostream(*listStream));
    }
    if (os)
    {
        dllMain->print(*os);
    }
    std::string errorMessage;
    llvm::raw_string_ostream errorOs(errorMessage);
    if (verifyFunction(*dllMain, &errorOs))
    {
        errorOs.str(); // flush contents of errorOs to errorMessage
        throw std::runtime_error("NativeCompiler: verification of function 'DllMain' failed. " + errorMessage);
    }
}

void NativeCompilerImpl::GenerateObjectFile(const std::string& assemblyObjectFilePath)
{
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "Generating object code file " << assemblyObjectFilePath << "..." << std::endl;
    }
    legacy::PassManager passManager;
    std::error_code errorCode;
    raw_fd_ostream objectFile(assemblyObjectFilePath, errorCode, sys::fs::F_None);
    if (targetMachine->addPassesToEmitFile(passManager, objectFile, TargetMachine::CGFT_ObjectFile))
    {
        throw std::runtime_error("NativeCompiler: cannot emit object code file '" + assemblyObjectFilePath + "' (llvm::TargetMachine::addPassesToEmitFile failed).");
    }
    passManager.run(*module);
    objectFile.flush();
    if (objectFile.has_error())
    {
        throw std::runtime_error("NativeCompiler: could not emit object code file '" + assemblyObjectFilePath + "': " + errorCode.message());
    }
}

void NativeCompilerImpl::GenerateAsmFile(const std::string& asmFilePath)
{
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "Generating assembly code listing to " << asmFilePath << "..." << std::endl;
    }
    legacy::PassManager passManager;
    std::error_code errorCode;
    raw_fd_ostream asmFile(asmFilePath, errorCode, sys::fs::F_None);
    if (targetMachine->addPassesToEmitFile(passManager, asmFile, TargetMachine::CGFT_AssemblyFile))
    {
        throw std::runtime_error("NativeCompiler: cannot emit asm file '" + asmFilePath + "' (llvm::TargetMachine::addPassesToEmitFile failed).");
    }
    passManager.run(*module);
    asmFile.flush();
    if (asmFile.has_error())
    {
        throw std::runtime_error("NativeCompiler: could not emit asm file '" + asmFilePath + "': " + errorCode.message());
    }
}

void NativeCompilerImpl::Link(const std::string& assemblyObjectFilePath)
{
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "Linking " << assemblyObjectFilePath << "..." << std::endl;
    }
#ifdef _WIN32
    LinkWindows(assemblyObjectFilePath);
#endif
}

void ImportAssemblyReferences(Assembly* assembly, std::vector<std::string>& args, std::ofstream* listStream)
{
    for (const std::unique_ptr<Assembly>& referencedAssembly : assembly->ReferencedAssemblies())
    {
        if (referencedAssembly->IsCore()) continue;
        if (referencedAssembly->Imported()) continue;
        referencedAssembly->SetImported();
        if (referencedAssembly->NativeTargetTriple() != assembly->NativeTargetTriple())
        {
            std::string warningMessage = "NativeCompiler: warning: referenced assembly '" + ToUtf8(referencedAssembly->Name().Value()) +
                "' has different native target triple (" + referencedAssembly->NativeTargetTriple() + ") than assembly '" + ToUtf8(assembly->Name().Value()) +
                "' being compiled (" + assembly->NativeTargetTriple() + ")";
            if (listStream)
            {
                *listStream << warningMessage << std::endl;
            }
            std::cout << warningMessage << std::endl;
        }
        if (referencedAssembly->NativeImportLibraryFileName().empty())
        {
            std::string errorMessage = "NativeCompiler: error: native import library file name of the referenced assembly '" + ToUtf8(referencedAssembly->Name().Value()) +
                "' (" + referencedAssembly->FilePathReadFrom() + ") is empty";
            if (listStream)
            {
                *listStream << errorMessage << std::endl;
            }
            throw std::runtime_error(errorMessage);
        }
        std::string importLibraryFilePath = Path::Combine(Path::GetDirectoryName(referencedAssembly->FilePathReadFrom()), referencedAssembly->NativeImportLibraryFileName());
        args.push_back(QuotedPath(importLibraryFilePath));
        ImportAssemblyReferences(referencedAssembly.get(), args, listStream);
    }
}

void NativeCompilerImpl::LinkWindows(const std::string& assemblyObjectFilePath)
{
    std::vector<std::string> args;
    args.push_back("/dll");
    args.push_back("/entry:DllMain");
    if (!GetGlobalFlag(GlobalFlags::release))
    {
        args.push_back("/debug");
    }
    std::string importLibraryFilePath = Path::ChangeExtension(assemblyObjectFilePath, ".lib");
    std::string importLibraryFileName = Path::GetFileName(importLibraryFilePath);
    args.push_back("/implib:" + QuotedPath(importLibraryFilePath));
    std::string dllFilePath = Path::ChangeExtension(assemblyObjectFilePath, ".dll");
    std::string dllFileName = Path::GetFileName(dllFilePath);
    args.push_back("/out:" + QuotedPath(dllFilePath));
    args.push_back(QuotedPath(assemblyObjectFilePath));
    std::string cminorLibDir = CminorLibDir();
    args.push_back("/libpath:" + QuotedPath(cminorLibDir));
    args.push_back("cminorrt.lib");
    if (GetGlobalFlag(GlobalFlags::linkWithDebugMachine))
    {
        assembly->SetLinkedWithDebugMachine();
        args.push_back("cminormachined.lib");
    }
    else
    {
        args.push_back("cminormachine.lib");
    }
    ImportAssemblyReferences(assembly, args, listStream.get());
    bool useMsLink = true;
    std::string linkCommandLine = "lld-link";
    if (useMsLink)
    {
        linkCommandLine = "link";
    }
    for (const std::string& arg : args)
    {
        linkCommandLine.append(1, ' ').append(arg);
    }
    std::string linkErrorFilePath = Path::Combine(Path::GetDirectoryName(assemblyObjectFilePath), "lld-link.error");
    if (useMsLink)
    {
        linkErrorFilePath = Path::Combine(Path::GetDirectoryName(assemblyObjectFilePath), "link.error");
    }
    try
    {
        int redirectHandle = 2; // stderr
        if (useMsLink)
        {
            redirectHandle = 1; // stdout
        }
        System(linkCommandLine, redirectHandle, linkErrorFilePath);
        boost::filesystem::remove(boost::filesystem::path(linkErrorFilePath));
        assembly->SetNativeImportLibraryFileName(importLibraryFileName);
        assembly->SetNativeSharedLibraryFileName(dllFileName);
        if (GetGlobalFlag(GlobalFlags::verbose))
        {
            std::cout << "Linking " << assemblyObjectFilePath << " succeeded." << std::endl;
            std::cout << "=> " << importLibraryFilePath << std::endl;
            std::cout << "=> " << dllFilePath << std::endl;
        }
    }
    catch (const std::exception& ex)
    {
        std::string errors = ReadFile(linkErrorFilePath);
        if (listStream)
        {
            *listStream << "linking '" + assemblyObjectFilePath + "' failed:\n" << linkCommandLine << "\n:" << errors << std::endl;
        }
        throw std::runtime_error("NativeCompiler: linking '" + assemblyObjectFilePath + "' failed: " + ex.what() + ":\nerrors:\n" + errors);
    }
}

void NativeCompilerImpl::BeginVisitFunction(Function& function)
{
    basicBlocks.clear();
    unwindTargets.clear();
    jumpTargets.clear();
    currentExceptionBlockId = -1;
    exceptionBlockIdStack.clear();
    currentCatchSectionExceptionBlockId = -1;
    catchSectionExceptionBlockIdStack.clear();
    currentPad = nullptr;
    padStack.clear();
    currentPadKind = LlvmPadKind::none;
    padKindStack.clear();
    exceptionPtr = nullptr;
    exceptionObjectVariables.clear();
    std::string mangledFunctionName = MangleFunctionName(function);
    if (mangledFunctionName == "Visit_LinkerVisitor_256E02797B9954BAD4638DCA8D1EAD3F13C7D43E")
    {
        int x = 0;
    }
    function.SetMangledName(mangledFunctionName);
    if (listStream)
    {
        CodeFormatter formatter(*listStream);
        formatter.WriteLine();
        formatter.WriteLine("before removing unreachable instructions:");
        function.Dump(formatter);
    }
    function.RemoveUnreachableInstructions(jumpTargets);
    if (listStream)
    {
        CodeFormatter formatter(*listStream);
        formatter.WriteLine("after removing unreachable instructions:");
        function.Dump(formatter);
    }
    llvm::Type* returnType = GetType(function.ReturnType());
    std::vector<llvm::Type*> paramTypes;
    for (ValueType pvt : function.ParameterTypes())
    {
        llvm::Type* paramType = GetType(pvt);
        paramTypes.push_back(paramType);
    }
    llvm::FunctionType* funType = llvm::FunctionType::get(returnType, paramTypes, false);
    fun = cast<llvm::Function>(module->getOrInsertFunction(mangledFunctionName, funType));
    if (function.IsExported())
    {
        ExportFunction(fun);
        assembly->AddExportedFunction(function.CallName());
    }
    else
    {
        fun->setLinkage(llvm::GlobalValue::LinkageTypes::InternalLinkage);
    }
    if (function.IsMain())
    {
        fun->setAttributes(mainFunctionAttributes);
    }
    else if (function.HasExceptionBlocks() || function.CanThrow())
    {
        fun->setAttributes(unwindFunctionAttributes);
    }
    else
    {
        fun->setAttributes(nounwindFunctionAttributes);
    }
    if (function.HasExceptionBlocks())
    {
        SetPersonalityFunction();
    }
    BasicBlock* entryBlock = BasicBlock::Create(context, "entry", fun);
    entryBasicBlock = entryBlock;
    builder.SetInsertPoint(entryBlock);
    locals.clear();
    lastAlloca = nullptr;
    for (ValueType lt : function.LocalTypes())
    {
        llvm::Type* type = GetType(lt);
        llvm::AllocaInst* allocaInst = builder.CreateAlloca(type);
        locals.push_back(allocaInst);
        lastAlloca = allocaInst;
    }
    int ne = function.NumExceptionBlocks();
    for (int i = 0; i < ne; ++i)
    {
        llvm::AllocaInst* exceptionObjectVar = builder.CreateAlloca(PointerType::get(GetType(ValueType::byteType), 0));
        exceptionObjectVariables[i] = exceptionObjectVar;
    }
    this->function = &function;
    if (function.CanThrow() || function.HasExceptionBlocks())
    {
        llvm::StructType* functionStackEntryType = llvm::StructType::get(
            llvm::PointerType::get(GetType(ValueType::byteType), 0), 
            GetType(ValueType::intType), 
            llvm::PointerType::get(GetType(ValueType::byteType), 0), 
            nullptr);
        functionStackEntry = builder.CreateAlloca(functionStackEntryType);
    }
    else
    {
        functionStackEntry = nullptr;
    }
    constantPool = &function.GetConstantPool();
    currentBasicBlock = entryBlock;
    functionPtrVar = GetFunctionPtrVar(&function);
}

void NativeCompilerImpl::EndVisitFunction(Function& function)
{
    bool lastIsJumpOrThrow = false;
    if (function.NumInsts() > 0)
    {
        Instruction* lastInst = function.GetInst(function.NumInsts() - 1);
        lastIsJumpOrThrow = lastInst->IsJump() || lastInst->IsThrow();
    }
    if (!lastIsJumpOrThrow && !function.ReturnsValue())
    {
        CreateLeaveFunctionCall();
        builder.CreateRetVoid();
    }
    std::unique_ptr<llvm::raw_os_ostream> os;
    if (listStream)
    {
        os.reset(new llvm::raw_os_ostream(*listStream));
    }
    if (os)
    {
        fun->print(*os);
    }
    std::string errorMessage;
    llvm::raw_string_ostream errorOs(errorMessage);
    if (verifyFunction(*fun, &errorOs))
    {
        errorOs.str(); // flush contents of errorOs to errorMessage
        if (listStream)
        {
            *listStream << "NativeCompiler: verification of function '" << ToUtf8(function.CallName().Value().AsStringLiteral()) << "' failed. " << errorMessage << std::endl;
        }
        throw std::runtime_error("NativeCompiler: verification of function '" + ToUtf8(function.CallName().Value().AsStringLiteral()) + "' failed. " + errorMessage);
    }
}

void NativeCompilerImpl::BeginVisitInstruction(int instructionNumber, bool prevEndsBasicBlock, Instruction* inst)
{
    auto it = basicBlocks.find(instructionNumber);
    if (it != basicBlocks.cend())
    {
        currentBasicBlock = it->second;
        if (!prevEndsBasicBlock)
        {
            builder.CreateBr(currentBasicBlock);
        }
        builder.SetInsertPoint(currentBasicBlock);
    }
    else if (jumpTargets.find(instructionNumber) != jumpTargets.cend() && !inst->IsBeginCatchOrFinallyInst())
    {
        currentBasicBlock = BasicBlock::Create(context, "target" + std::to_string(instructionNumber), fun);
        basicBlocks[instructionNumber] = currentBasicBlock;
        if (!prevEndsBasicBlock)
        {
            builder.CreateBr(currentBasicBlock);
        }
        builder.SetInsertPoint(currentBasicBlock);
    }
    else if (currentBasicBlock == nullptr && !inst->IsEhBlockInst())
    {
        currentBasicBlock = BasicBlock::Create(context, "target" + std::to_string(instructionNumber), fun);
        builder.SetInsertPoint(currentBasicBlock);
    }
}

void NativeCompilerImpl::VisitInvalidInst(InvalidInst& instruction)
{
    throw std::runtime_error("NativeCompiler: invalid instruction encountered");
}

void NativeCompilerImpl::VisitLoadDefaultValueBaseInst(LoadDefaultValueBaseInst& instruction)
{
    ValueType valueType = instruction.GetValueType();
    PushDefaultValue(valueType);
}

void NativeCompilerImpl::VisitUnaryOpBaseInst(UnaryOpBaseInst& instruction)
{
    if (instruction.GroupName() == "neg")
    {
        llvm::Value* operand = valueStack.Pop();
        switch (instruction.GetValueType())
        {
            case ValueType::floatType: case ValueType::doubleType:
            {
                valueStack.Push(builder.CreateFNeg(operand));
                break;
            }
            default:
            {
                valueStack.Push(builder.CreateNeg(operand));
                break;
            }
        }
    }
    else if (instruction.GroupName() == "cpl")
    {
        PushIntegerMinusOne(instruction.GetValueType());
        llvm::Value* left = valueStack.Pop();
        llvm::Value* right = valueStack.Pop();
        llvm::Value* result = builder.CreateXor(left, right);
        valueStack.Push(result);
    }
}

void NativeCompilerImpl::VisitBinaryOpBaseInst(BinaryOpBaseInst& instruction)
{
    auto it = binOpMap.find(instruction.Name());
    if (it != binOpMap.cend())
    {
        llvm::Value* right = valueStack.Pop();
        llvm::Value* left = valueStack.Pop();
        LlvmBinOp op = it->second;
        switch (op)
        {
            case LlvmBinOp::add:
            {
                valueStack.Push(builder.CreateAdd(left, right));
                break;
            }
            case LlvmBinOp::fadd:
            {
                valueStack.Push(builder.CreateFAdd(left, right));
                break;
            }
            case LlvmBinOp::sub:
            {
                valueStack.Push(builder.CreateSub(left, right));
                break;
            }
            case LlvmBinOp::fsub:
            {
                valueStack.Push(builder.CreateFSub(left, right));
                break;
            }
            case LlvmBinOp::mul:
            {
                valueStack.Push(builder.CreateMul(left, right));
                break;
            }
            case LlvmBinOp::fmul:
            {
                valueStack.Push(builder.CreateFMul(left, right));
                break;
            }
            case LlvmBinOp::udiv:
            {
                valueStack.Push(builder.CreateUDiv(left, right));
                break;
            }
            case LlvmBinOp::sdiv:
            {
                valueStack.Push(builder.CreateSDiv(left, right));
                break;
            }
            case LlvmBinOp::fdiv:
            {
                valueStack.Push(builder.CreateFDiv(left, right));
                break;
            }
            case LlvmBinOp::urem:
            {
                valueStack.Push(builder.CreateURem(left, right));
                break;
            }
            case LlvmBinOp::srem:
            {
                valueStack.Push(builder.CreateSRem(left, right));
                break;
            }
            case LlvmBinOp::shl:
            {
                valueStack.Push(builder.CreateShl(left, right));
                break;
            }
            case LlvmBinOp::lshr:
            {
                valueStack.Push(builder.CreateLShr(left, right));
                break;
            }
            case LlvmBinOp::ashr:
            {
                valueStack.Push(builder.CreateAShr(left, right));
                break;
            }
            case LlvmBinOp::and_:
            {
                valueStack.Push(builder.CreateAnd(left, right));
                break;
            }
            case LlvmBinOp::or_:
            {
                valueStack.Push(builder.CreateOr(left, right));
                break;
            }
            case LlvmBinOp::xor_:
            {
                valueStack.Push(builder.CreateXor(left, right));
                break;
            }
            default:
            {
                throw std::runtime_error("NativeCompiler: invalid llvm bin op");
            }
        }
    }
    else
    {
        throw std::runtime_error("NativeCompiler: binary operation not found");
    }
}

void NativeCompilerImpl::VisitBinaryPredBaseInst(BinaryPredBaseInst& instruction)
{
    auto it = binPredMap.find(instruction.Name());
    if (it != binPredMap.cend())
    {
        llvm::Value* right = valueStack.Pop();
        llvm::Value* left = valueStack.Pop();
        LlvmBinPred pred = it->second;
        switch (pred)
        {
            case LlvmBinPred::ieq:
            {
                valueStack.Push(builder.CreateICmpEQ(left, right));
                break;
            }
            case LlvmBinPred::oeq:
            {
                valueStack.Push(builder.CreateFCmpOEQ(left, right));
                break;
            }
            case LlvmBinPred::ult:
            {
                valueStack.Push(builder.CreateICmpULT(left, right));
                break;
            }
            case LlvmBinPred::slt:
            {
                valueStack.Push(builder.CreateICmpSLT(left, right));
                break;
            }
            case LlvmBinPred::olt:
            {
                valueStack.Push(builder.CreateFCmpOLT(left, right));
                break;
            }
            default:
            {
                throw std::runtime_error("NativeCompiler: invalid llvm bin pred");
            }
        }
    }
    else
    {
        throw std::runtime_error("NativeCompiler: binary predicate operation not found");
    }
}

void NativeCompilerImpl::VisitLogicalNotInst(LogicalNotInst& instruction)
{
    valueStack.Push(builder.CreateNot(valueStack.Pop()));
}

void NativeCompilerImpl::VisitLoadLocalInst(int32_t localIndex)
{
    valueStack.Push(builder.CreateLoad(locals[localIndex]));
}

void NativeCompilerImpl::VisitStoreLocalInst(int32_t localIndex)
{
    llvm::Value* value = valueStack.Pop();
    builder.CreateStore(value, locals[localIndex]);
}

void NativeCompilerImpl::VisitLoadFieldInst(int32_t fieldIndex, ValueType fieldType)
{
    llvm::Value* objectReference = valueStack.Pop();
    ArgVector args;
    args.push_back(objectReference);
    llvm::Value* index = builder.getInt32(fieldIndex);
    args.push_back(index);
    llvm::Function* callee = nullptr;
    switch (fieldType)
    {
        case ValueType::sbyteType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldSb", GetType(ValueType::sbyteType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::byteType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldBy", GetType(ValueType::byteType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::shortType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldSh", GetType(ValueType::shortType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::ushortType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldUs", GetType(ValueType::ushortType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::intType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldIn", GetType(ValueType::intType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::uintType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldUi", GetType(ValueType::uintType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::longType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldLo", GetType(ValueType::longType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::ulongType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldUl", GetType(ValueType::ulongType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::floatType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldFl", GetType(ValueType::floatType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::doubleType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldDo", GetType(ValueType::doubleType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::charType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldCh", GetType(ValueType::charType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::boolType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldBo", GetType(ValueType::boolType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::objectReference:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldOb", GetType(ValueType::ulongType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        default:
        {
            throw std::runtime_error("NativeCompiler: invalid field type for load field");
        }
    }
    ImportFunction(callee);
    CreateCall(callee, args, true);
}

void NativeCompilerImpl::VisitStoreFieldInst(int32_t fieldIndex, ValueType fieldType)
{
    llvm::Value* objectReference = valueStack.Pop();
    llvm::Value* fieldValue = valueStack.Pop();
    ArgVector args;
    args.push_back(objectReference);
    args.push_back(fieldValue);
    llvm::Value* index = builder.getInt32(fieldIndex);
    args.push_back(index);
    llvm::Function* callee = nullptr;
    switch (fieldType)
    {
        case ValueType::sbyteType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldSb", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::sbyteType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::byteType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldBy", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::byteType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::shortType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldSh", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::shortType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::ushortType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldUs", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::ushortType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::intType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldIn", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::intType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::uintType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldUi", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::uintType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::longType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldLo", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::longType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::ulongType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldUl", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::floatType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldFl", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::floatType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::doubleType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldDo", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::doubleType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::charType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldCh", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::charType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::boolType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldBo", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::boolType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::objectReference:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldOb", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::objectReference), GetType(ValueType::intType), nullptr));
            break;
        }
        default:
        {
            throw std::runtime_error("NativeCompiler: invalid field type for store field");
        }
    }
    ImportFunction(callee);
    CreateCall(callee, args, false);
}

void NativeCompilerImpl::VisitLoadElemInst(LoadElemInst& instruction)
{
    llvm::Value* index = valueStack.Pop();
    llvm::Value* arr = valueStack.Pop();
    ArgVector args;
    args.push_back(arr);
    args.push_back(index);
    llvm::Function* callee = nullptr;
    switch (instruction.GetElemType())
    {
        case ValueType::sbyteType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadElemSb", GetType(ValueType::sbyteType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::byteType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadElemBy", GetType(ValueType::byteType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::shortType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadElemSh", GetType(ValueType::shortType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::ushortType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadElemUs", GetType(ValueType::ushortType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::intType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadElemIn", GetType(ValueType::intType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::uintType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadElemUi", GetType(ValueType::uintType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::longType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadElemLo", GetType(ValueType::longType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::ulongType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadElemUl", GetType(ValueType::ulongType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::floatType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadElemFl", GetType(ValueType::floatType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::doubleType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadElemDo", GetType(ValueType::doubleType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::charType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadElemCh", GetType(ValueType::charType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::boolType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadElemBo", GetType(ValueType::boolType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::objectReference:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadElemOb", GetType(ValueType::objectReference), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        default:
        {
            throw std::runtime_error("NativeCompiler: invalid field type for load element");
        }
    }
    ImportFunction(callee);
    CreateCall(callee, args, true);
}

void NativeCompilerImpl::VisitStoreElemInst(StoreElemInst& instruction)
{
    llvm::Value* index = valueStack.Pop();
    llvm::Value* arr = valueStack.Pop();
    llvm::Value* elementValue = valueStack.Pop();
    ArgVector args;
    args.push_back(arr);
    args.push_back(elementValue);
    args.push_back(index);
    llvm::Function* callee = nullptr;
    switch (instruction.GetElemType())
    {
        case ValueType::sbyteType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreElemSb", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::sbyteType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::byteType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreElemBy", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::byteType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::shortType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreElemSh", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::shortType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::ushortType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreElemUs", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::ushortType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::intType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreElemIn", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::intType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::uintType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreElemUi", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::uintType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::longType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreElemLo", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::longType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::ulongType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreElemUl", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::floatType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreElemFl", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::floatType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::doubleType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreElemDo", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::doubleType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::charType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreElemCh", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::charType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::boolType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreElemBo", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::boolType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::objectReference:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreElemOb", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::objectReference), GetType(ValueType::intType), nullptr));
            break;
        }
        default:
        {
            throw std::runtime_error("NativeCompiler: invalid field type for store element");
        }
    }
    ImportFunction(callee);
    CreateCall(callee, args, false);
}

void NativeCompilerImpl::VisitLoadConstantInst(int32_t constantIndex)
{
    ConstantId constantId(constantIndex);
    cminor::machine::Constant constant = constantPool->GetConstant(constantId);
    IntegralValue value = constant.Value();
    switch (value.GetType())
    {
        case ValueType::sbyteType:
        {
            valueStack.Push(builder.getInt8(value.AsSByte()));
            break;
        }
        case ValueType::byteType:
        {
            valueStack.Push(builder.getInt8(value.AsByte()));
            break;
        }
        case ValueType::shortType:
        {
            valueStack.Push(builder.getInt16(value.AsShort()));
            break;
        }
        case ValueType::ushortType:
        {
            valueStack.Push(builder.getInt16(value.AsUShort()));
            break;
        }
        case ValueType::intType:
        {
            valueStack.Push(builder.getInt32(value.AsInt()));
            break;
        }
        case ValueType::uintType:
        {
            valueStack.Push(builder.getInt32(value.AsUInt()));
            break;
        }
        case ValueType::longType:
        {
            valueStack.Push(builder.getInt64(value.AsLong()));
            break;
        }
        case ValueType::ulongType:
        {
            valueStack.Push(builder.getInt64(value.AsULong()));
            break;
        }
        case ValueType::floatType:
        {
            valueStack.Push(ConstantFP::get(llvm::Type::getFloatTy(context), value.AsFloat()));
            break;
        }
        case ValueType::doubleType:
        {
            valueStack.Push(ConstantFP::get(llvm::Type::getDoubleTy(context), value.AsDouble()));
            break;
        }
        case ValueType::boolType:
        {
            valueStack.Push(builder.getInt1(value.AsBool()));
            break;
        }
        case ValueType::charType:
        {
            valueStack.Push(builder.getInt32(static_cast<uint32_t>(value.AsChar())));
            break;
        }
        case ValueType::stringLiteral:
        {
            llvm::Function* rtLoadStringLiteral = cast<llvm::Function>(module->getOrInsertFunction("RtLoadStringLiteral", PointerType::get(GetType(ValueType::uintType), 0), 
                PointerType::get(GetType(ValueType::byteType), 0), GetType(ValueType::uintType), nullptr));
            ImportFunction(rtLoadStringLiteral);
            ArgVector args;
            args.push_back(builder.CreateLoad(constantPoolVariable));
            args.push_back(builder.getInt32(constantId.Value()));
            CreateCall(rtLoadStringLiteral, args, true);
            break;
        }
        case ValueType::objectReference:
        {
            valueStack.Push(builder.getInt64(value.Value()));
            break;
        }
        default:
        {
            throw std::runtime_error("NativeCompiler: invalid constant type");
        }
    }
}

void NativeCompilerImpl::VisitReceiveInst(ReceiveInst& instruction)
{
    int i = 0;
    for (auto it = fun->arg_begin(); it != fun->arg_end(); ++it)
    {
        llvm::AllocaInst* local = locals[i];
        builder.CreateStore(&*it, local);
        ++i;
    }
    CreateEnterFunctionCall();
}

void NativeCompilerImpl::CreateEnterFunctionCall()
{
    if (!function->CanThrow() && !function->HasExceptionBlocks()) return;
    ArgVector fse00;
    fse00.push_back(builder.getInt32(0));
    fse00.push_back(builder.getInt32(0));
    Assert(functionStackEntry, "function stack entry variable not created");
    llvm::Value* fseFunctionPtrPtr = builder.CreateGEP(nullptr, functionStackEntry, fse00);
    Assert(functionPtrVar, "function ptr var not created");
    llvm::Value* functionPtr = builder.CreateLoad(functionPtrVar);
    builder.CreateStore(functionPtr, fseFunctionPtrPtr);
    SetCurrentLineNumber(function->GetFirstSourceLine());
    llvm::Function* rtEnterFunction = cast<llvm::Function>(module->getOrInsertFunction("RtEnterFunction", GetType(ValueType::none), llvm::PointerType::get(GetType(ValueType::byteType), 0), nullptr));
    ImportFunction(rtEnterFunction);
    ArgVector args;
    args.push_back(builder.CreateBitCast(functionStackEntry, llvm::PointerType::get(GetType(ValueType::byteType), 0)));
    builder.CreateCall(rtEnterFunction, args);
}

void NativeCompilerImpl::CreateLeaveFunctionCall()
{
    if (!function->CanThrow() && !function->HasExceptionBlocks()) return;
    Assert(functionStackEntry, "function stack entry variable not created");
    llvm::Function* rtLeaveFunction = cast<llvm::Function>(module->getOrInsertFunction("RtLeaveFunction", GetType(ValueType::none), llvm::PointerType::get(GetType(ValueType::byteType), 0), nullptr));
    ImportFunction(rtLeaveFunction);
    ArgVector args;
    args.push_back(builder.CreateBitCast(functionStackEntry, llvm::PointerType::get(GetType(ValueType::byteType), 0)));
    builder.CreateCall(rtLeaveFunction, args);
}

void NativeCompilerImpl::SetCurrentLineNumber(uint32_t lineNumber)
{
    if (!function->CanThrow() && !function->HasExceptionBlocks()) return;
    if (lineNumber == uint32_t(-1)) return;
    ArgVector fse01;
    fse01.push_back(builder.getInt32(0));
    fse01.push_back(builder.getInt32(1));
    llvm::Value* fseLineNumberPtr = builder.CreateGEP(nullptr, functionStackEntry, fse01);
    builder.CreateStore(builder.getInt32(lineNumber), fseLineNumberPtr);
}

void NativeCompilerImpl::VisitConversionBaseInst(ConversionBaseInst& instruction)
{
    auto it = convMap.find(instruction.Name());
    if (it != convMap.cend())
    {
        LlvmConv conv = it->second;
        llvm::Value* from = valueStack.Pop();
        llvm::Type* targetType = GetType(instruction.GetTargetType());
        switch (conv)
        {
            case LlvmConv::zext:
            {
                valueStack.Push(builder.CreateZExt(from, targetType));
                break;
            }
            case LlvmConv::sext:
            {
                valueStack.Push(builder.CreateSExt(from, targetType));
                break;
            }
            case LlvmConv::trunc:
            {
                valueStack.Push(builder.CreateTrunc(from, targetType));
                break;
            }
            case LlvmConv::bitcast:
            {
                valueStack.Push(builder.CreateBitCast(from, targetType));
                break;
            }
            case LlvmConv::fptoui:
            {
                valueStack.Push(builder.CreateFPToUI(from, targetType));
                break;
            }
            case LlvmConv::fptosi:
            {
                valueStack.Push(builder.CreateFPToSI(from, targetType));
                break;
            }
            case LlvmConv::uitofp:
            {
                valueStack.Push(builder.CreateUIToFP(from, targetType));
                break;
            }
            case LlvmConv::sitofp:
            {
                valueStack.Push(builder.CreateSIToFP(from, targetType));
                break;
            }
            case LlvmConv::fpext:
            {
                valueStack.Push(builder.CreateFPExt(from, targetType));
                break;
            }
            case LlvmConv::fptrunc:
            {
                valueStack.Push(builder.CreateFPTrunc(from, targetType));
                break;
            }
            default:
            {
                throw std::runtime_error("NativeCompiler: invalid llvm conv");
            }
        }
    }
}

void NativeCompilerImpl::VisitJumpInst(JumpInst& instruction)
{
    int32_t target = instruction.Index();
    if (target == endOfFunction)
    {
        CreateReturnFromFunctionOrFunclet();
    }
    else
    {
        BasicBlock* targetBlock = nullptr;
        auto it = basicBlocks.find(target);
        if (it != basicBlocks.cend())
        {
            targetBlock = it->second;
        }
        else
        {
            targetBlock = BasicBlock::Create(context, "target" + std::to_string(target), fun);
            basicBlocks[target] = targetBlock;
        }
        builder.CreateBr(targetBlock);
    }
    currentBasicBlock = nullptr;
}

void NativeCompilerImpl::VisitJumpTrueInst(JumpTrueInst& instruction)
{
    BasicBlock* trueTargetBlock = nullptr;
    int32_t target = instruction.Index();
    auto it = basicBlocks.find(target);
    if (it != basicBlocks.cend())
    {
        trueTargetBlock = it->second;
    }
    else
    {
        trueTargetBlock = BasicBlock::Create(context, "trueTarget" + std::to_string(target), fun);
        basicBlocks[target] = trueTargetBlock;
    }
    BasicBlock* continueBlock = BasicBlock::Create(context, "continueTarget" + std::to_string(GetCurrentInstructionIndex()), fun);
    llvm::Value* cond = valueStack.Pop();
    builder.CreateCondBr(cond, trueTargetBlock, continueBlock);
    currentBasicBlock = continueBlock;
    builder.SetInsertPoint(currentBasicBlock);
}

void NativeCompilerImpl::VisitJumpFalseInst(JumpFalseInst& instruction)
{
    BasicBlock* falseTargetBlock = nullptr;
    int32_t target = instruction.Index();
    auto it = basicBlocks.find(target);
    if (it != basicBlocks.cend())
    {
        falseTargetBlock = it->second;
    }
    else
    {
        falseTargetBlock = BasicBlock::Create(context, "falseTarget" + std::to_string(target), fun);
        basicBlocks[instruction.Index()] = falseTargetBlock;
    }
    BasicBlock* continueBlock = BasicBlock::Create(context, "continueTarget" + std::to_string(GetCurrentInstructionIndex()), fun);
    llvm::Value* cond = valueStack.Pop();
    builder.CreateCondBr(cond, continueBlock, falseTargetBlock);
    currentBasicBlock = continueBlock;
    builder.SetInsertPoint(currentBasicBlock);
}

void NativeCompilerImpl::VisitContinuousSwitchInst(ContinuousSwitchInst& instruction)
{
    ValueType condType = instruction.CondType();
    llvm::Value* cond = valueStack.Pop();
    IntegralValue begin = instruction.Begin();
    IntegralValue end = instruction.End();
    int32_t defaultTarget = instruction.DefaultTarget();
    BasicBlock* defaultBlock = BasicBlock::Create(context, "defaultTarget" + std::to_string(defaultTarget), fun);
    basicBlocks[defaultTarget] = defaultBlock;
    int n = int(instruction.Targets().size());
    Assert(n == end.Value() - begin.Value() + 1, "invalid switch range");
    SwitchInst* switchInst = builder.CreateSwitch(cond, defaultBlock, n);
    for (int i = 0; i < n; ++i)
    {
        int32_t target = instruction.Targets()[i];
        if (target != defaultTarget)
        {
            BasicBlock* targetBlock = BasicBlock::Create(context, "caseTarget" + std::to_string(target), fun);
            basicBlocks[target] = targetBlock;
            IntegralValue caseValue(begin.Value() + i, begin.GetType());
            switchInst->addCase(GetConstantInt(condType, caseValue), targetBlock);
        }
    }
}

void NativeCompilerImpl::VisitBinarySearchSwitchInst(BinarySearchSwitchInst& instruction)
{
    ValueType condType = instruction.CondType();
    llvm::Value* cond = valueStack.Pop();
    int32_t defaultTarget = instruction.DefaultTarget();
    BasicBlock* defaultBlock = BasicBlock::Create(context, "defaultTarget" + std::to_string(defaultTarget), fun);
    basicBlocks[defaultTarget] = defaultBlock;
    int n = int(instruction.Targets().size());
    SwitchInst* switchInst = builder.CreateSwitch(cond, defaultBlock, n);
    for (int i = 0; i < n; ++i)
    {
        const std::pair<IntegralValue, int32_t>& t = instruction.Targets()[i];
        IntegralValue caseValue = t.first;
        int32_t target = t.second;
        BasicBlock* targetBlock = BasicBlock::Create(context, "caseTarget" + std::to_string(target), fun);
        basicBlocks[target] = targetBlock;
        switchInst->addCase(GetConstantInt(condType, caseValue), targetBlock);
    }
}

void NativeCompilerImpl::VisitCallInst(CallInst& instruction)
{
    Function* calledFunction = instruction.GetFunction();
    llvm::Type* returnType = GetType(calledFunction->ReturnType());
    std::vector<llvm::Type*> paramTypes;
    for (ValueType pvt : calledFunction->ParameterTypes())
    {
        llvm::Type* paramType = GetType(pvt);
        paramTypes.push_back(paramType);
    }
    llvm::FunctionType* funType = llvm::FunctionType::get(returnType, paramTypes, false);
    if (calledFunction->MangledName().empty())
    {
        calledFunction->SetMangledName(MangleFunctionName(*calledFunction));
    }
    llvm::Function* callee = cast<llvm::Function>(module->getOrInsertFunction(calledFunction->MangledName(), funType));
    if (calledFunction->GetAssembly() != assembly)
    {
        ImportFunction(callee);
    }
    ArgVector args;
    int n = int(paramTypes.size());
    args.resize(n);
    for (int i = 0; i < n; ++i)
    {
        llvm::Value* arg = valueStack.Pop();
        args[n - i - 1] = arg;
    }
    int32_t pc = GetCurrentInstructionIndex();
    uint32_t lineNumber = function->GetSourceLine(pc);
    SetCurrentLineNumber(lineNumber);
    CreateCall(callee, args, calledFunction->ReturnsValue());
}

void NativeCompilerImpl::VisitVirtualCallInst(VirtualCallInst& instruction)
{
    llvm::Type* returnType = GetType(instruction.GetFunctionType().ReturnType());
    std::vector<llvm::Type*> paramTypes;
    for (ValueType pvt : instruction.GetFunctionType().ParameterTypes())
    {
        llvm::Type* paramType = GetType(pvt);
        paramTypes.push_back(paramType);
    }
    llvm::FunctionType* funType = llvm::FunctionType::get(returnType, paramTypes, false);
    llvm::PointerType* funPtrType = llvm::PointerType::get(funType, 0);
    ArgVector args;
    int n = int(paramTypes.size());
    args.resize(n);
    for (int i = 0; i < n; ++i)
    {
        llvm::Value* arg = valueStack.Pop();
        args[n - i - 1] = arg;
    }
    ArgVector resolveArgs;
    uint32_t vmtIndex = instruction.VmtIndex();
    resolveArgs.push_back(args[0]);
    resolveArgs.push_back(llvm::ConstantInt::get(GetIntegerType(ValueType::uintType), vmtIndex));
    llvm::Function* resolveVirtualFunctionCallAddress = cast<llvm::Function>(module->getOrInsertFunction("RtResolveVirtualFunctionCallAddress", PointerType::get(GetType(ValueType::byteType), 0),
        GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
    ImportFunction(resolveVirtualFunctionCallAddress);
    CreateCall(resolveVirtualFunctionCallAddress, resolveArgs, true);
    llvm::Value* functionAddress = valueStack.Pop();
    llvm::Value* funPtrValue = builder.CreateBitCast(functionAddress, funPtrType);
    int32_t pc = GetCurrentInstructionIndex();
    uint32_t lineNumber = function->GetSourceLine(pc);
    SetCurrentLineNumber(lineNumber);
    CreateCall(funPtrValue, args, instruction.GetFunctionType().ReturnType() != ValueType::none);
}

void NativeCompilerImpl::VisitInterfaceCallInst(InterfaceCallInst& instruction)
{
    llvm::Type* returnType = GetType(instruction.GetFunctionType().ReturnType());
    std::vector<llvm::Type*> paramTypes;
    for (ValueType pvt : instruction.GetFunctionType().ParameterTypes())
    {
        llvm::Type* paramType = GetType(pvt);
        paramTypes.push_back(paramType);
    }
    llvm::FunctionType* funType = llvm::FunctionType::get(returnType, paramTypes, false);
    llvm::PointerType* funPtrType = llvm::PointerType::get(funType, 0);
    ArgVector args;
    int n = int(paramTypes.size());
    args.resize(n);
    for (int i = 0; i < n; ++i)
    {
        llvm::Value* arg = valueStack.Pop();
        args[n - i - 1] = arg;
    }
    ArgVector resolveArgs;
    resolveArgs.push_back(args[0]);
    uint32_t imtIndex = instruction.ImtIndex();
    AllocaInst* receiver = builder.CreateAlloca(GetType(ValueType::ulongType));
    resolveArgs.push_back(llvm::ConstantInt::get(GetIntegerType(ValueType::uintType), imtIndex));
    resolveArgs.push_back(receiver);
    llvm::Function* rtResolveInterfaceCallAddress = cast<llvm::Function>(module->getOrInsertFunction("RtResolveInterfaceCallAddress", PointerType::get(GetType(ValueType::byteType), 0),
        GetType(ValueType::ulongType), GetType(ValueType::intType), PointerType::get(GetType(ValueType::ulongType), 0), nullptr));
    ImportFunction(rtResolveInterfaceCallAddress);
    CreateCall(rtResolveInterfaceCallAddress, resolveArgs, true);
    llvm::Value* functionAddress = valueStack.Pop();
    args[0] = builder.CreateLoad(receiver);
    llvm::Value* funPtrValue = builder.CreateBitCast(functionAddress, funPtrType);
    int32_t pc = GetCurrentInstructionIndex();
    uint32_t lineNumber = function->GetSourceLine(pc);
    SetCurrentLineNumber(lineNumber);
    CreateCall(funPtrValue, args, instruction.GetFunctionType().ReturnType() != ValueType::none);
}

void NativeCompilerImpl::VisitVmCallInst(VmCallInst& instruction)
{
    ConstantId vmFunctionNameId(instruction.Index());
    std::vector<llvm::Type*> vmCallContextElementTypes;
    vmCallContextElementTypes.push_back(GetType(ValueType::intType));
    vmCallContextElementTypes.push_back(PointerType::get(GetType(ValueType::byteType), 0));
    vmCallContextElementTypes.push_back(PointerType::get(GetType(ValueType::ulongType), 0));
    vmCallContextElementTypes.push_back(GetType(ValueType::byteType));
    vmCallContextElementTypes.push_back(GetType(ValueType::ulongType));
    llvm::StructType* vmCallContextType = llvm::StructType::get(context, vmCallContextElementTypes);
    AllocaInst* vmCallContext = builder.CreateAlloca(vmCallContextType);
    ArgVector cc0;
    cc0.push_back(builder.getInt32(0));
    cc0.push_back(builder.getInt32(0));
    llvm::Value* numLocalsPtr = builder.CreateGEP(nullptr, vmCallContext, cc0);
    uint32_t numLocals = function->NumLocals();
    builder.CreateStore(builder.getInt32(numLocals), numLocalsPtr);
    ArgVector cc1;
    cc1.push_back(builder.getInt32(0));
    cc1.push_back(builder.getInt32(1));
    llvm::Value* localTypesPtr = builder.CreateGEP(nullptr, vmCallContext, cc1);
    AllocaInst* localTypes = builder.CreateAlloca(GetType(ValueType::byteType), builder.getInt32(numLocals));
    for (uint32_t i = 0; i < numLocals; ++i)
    {
        ArgVector lti;
        lti.push_back(builder.getInt32(i));
        llvm::Value* localTypePtr = builder.CreateGEP(nullptr, localTypes, lti);
        ValueType localType = function->LocalTypes()[i];
        builder.CreateStore(builder.getInt8(uint8_t(localType)), localTypePtr);
    }
    builder.CreateStore(localTypes, localTypesPtr);
    AllocaInst* localsAlloca = builder.CreateAlloca(GetType(ValueType::ulongType), builder.getInt32(numLocals));
    for (uint32_t i = 0; i < numLocals; ++i)
    {
        ArgVector la;
        la.push_back(builder.getInt32(i));
        llvm::Value* localPtr = builder.CreateGEP(nullptr, localsAlloca, la);
        LoadInst* loaded = builder.CreateLoad(locals[i]);
        llvm::Value* convertedToULong = CreateConversionToUlong(loaded, function->LocalTypes()[i]);
        builder.CreateStore(convertedToULong, localPtr);
    }
    ArgVector cc2;
    cc2.push_back(builder.getInt32(0));
    cc2.push_back(builder.getInt32(2));
    llvm::Value* localsPtr = builder.CreateGEP(nullptr, vmCallContext, cc2);
    builder.CreateStore(localsAlloca, localsPtr);
    ArgVector cc3;
    cc3.push_back(builder.getInt32(0));
    cc3.push_back(builder.getInt32(3));
    llvm::Value* retValTypePtr = builder.CreateGEP(nullptr, vmCallContext, cc3);
    ValueType retValType = function->ReturnType();
    builder.CreateStore(builder.getInt8(uint8_t(retValType)), retValTypePtr);
    llvm::Function* rtVmCall = cast<llvm::Function>(module->getOrInsertFunction("RtVmCall", GetType(ValueType::none), 
        PointerType::get(GetType(ValueType::byteType), 0), 
        PointerType::get(GetType(ValueType::byteType), 0), 
        GetType(ValueType::intType),
        PointerType::get(vmCallContextType, 0), 
        nullptr));
    ImportFunction(rtVmCall);
    int32_t pc = GetCurrentInstructionIndex();
    uint32_t lineNumber = function->GetSourceLine(pc);
    SetCurrentLineNumber(lineNumber);
    ArgVector args;
    args.push_back(builder.CreateLoad(functionPtrVar));
    args.push_back(builder.CreateLoad(constantPoolVariable));
    args.push_back(builder.getInt32(vmFunctionNameId.Value()));
    args.push_back(vmCallContext);
    CreateCall(rtVmCall, args, false);
    if (retValType != ValueType::none)
    {
        ArgVector cc4;
        cc4.push_back(builder.getInt32(0));
        cc4.push_back(builder.getInt32(4));
        llvm::Value* retValPtr = builder.CreateGEP(nullptr, vmCallContext, cc4);
        llvm::Value* retValAsULong = builder.CreateLoad(retValPtr);
        llvm::Value* retVal = CreateConversionFromULong(retValAsULong, retValType);
        valueStack.Push(retVal);
    }
}

void NativeCompilerImpl::VisitDelegateCallInst(DelegateCallInst& instruction)
{
    llvm::Value* functionPtr = valueStack.Pop();
    ArgVector resolveArgs;
    resolveArgs.push_back(functionPtr);
    llvm::Function* rtResolveDelegateCallAddress = cast<llvm::Function>(module->getOrInsertFunction("RtResolveDelegateCallAddress", PointerType::get(GetType(ValueType::byteType), 0), 
        PointerType::get(GetType(ValueType::byteType), 0), nullptr));
    ImportFunction(rtResolveDelegateCallAddress);
    CreateCall(rtResolveDelegateCallAddress, resolveArgs, true);
    llvm::Value* delegateCallAddress = valueStack.Pop();
    llvm::Type* returnType = GetType(instruction.GetFunctionType().ReturnType());
    std::vector<llvm::Type*> paramTypes;
    for (ValueType pvt : instruction.GetFunctionType().ParameterTypes())
    {
        llvm::Type* paramType = GetType(pvt);
        paramTypes.push_back(paramType);
    }
    llvm::FunctionType* funType = llvm::FunctionType::get(returnType, paramTypes, false);
    llvm::PointerType* funPtrType = llvm::PointerType::get(funType, 0);
    ArgVector args;
    int n = int(paramTypes.size());
    args.resize(n);
    for (int i = 0; i < n; ++i)
    {
        llvm::Value* arg = valueStack.Pop();
        args[n - i - 1] = arg;
    }
    llvm::Value* funPtrValue = builder.CreateBitCast(delegateCallAddress, funPtrType);
    int32_t pc = GetCurrentInstructionIndex();
    uint32_t lineNumber = function->GetSourceLine(pc);
    SetCurrentLineNumber(lineNumber);
    CreateCall(funPtrValue, args, instruction.GetFunctionType().ReturnType() != ValueType::none);
}

void NativeCompilerImpl::VisitClassDelegateCallInst(ClassDelegateCallInst& instruction)
{
    llvm::Value* classDlg = valueStack.Pop();
    AllocaInst* classObjectAlloca = builder.CreateAlloca(GetType(ValueType::ulongType));
    ArgVector resolveArgs;
    resolveArgs.push_back(classDlg);
    resolveArgs.push_back(classObjectAlloca);
    llvm::Function* rtResolveClassDelegateCallAddress = cast<llvm::Function>(module->getOrInsertFunction("RtResolveClassDelegateCallAddress", PointerType::get(GetType(ValueType::byteType), 0),
        GetType(ValueType::ulongType), PointerType::get(GetType(ValueType::ulongType), 0), nullptr));
    ImportFunction(rtResolveClassDelegateCallAddress);
    CreateCall(rtResolveClassDelegateCallAddress, resolveArgs, true);
    llvm::Value* classDelegateCallAddress = valueStack.Pop();
    llvm::Type* returnType = GetType(instruction.GetFunctionType().ReturnType());
    std::vector<llvm::Type*> paramTypes;
    paramTypes.push_back(GetType(ValueType::ulongType));
    for (ValueType pvt : instruction.GetFunctionType().ParameterTypes())
    {
        llvm::Type* paramType = GetType(pvt);
        paramTypes.push_back(paramType);
    }
    llvm::FunctionType* funType = llvm::FunctionType::get(returnType, paramTypes, false);
    llvm::PointerType* funPtrType = llvm::PointerType::get(funType, 0);
    ArgVector args;
    int n = int(paramTypes.size());
    args.resize(n);
    args[0] = builder.CreateLoad(classObjectAlloca);
    for (int i = 0; i < n - 1; ++i)
    {
        llvm::Value* arg = valueStack.Pop();
        args[n - i - 1] = arg;
    }
    llvm::Value* funPtrValue = builder.CreateBitCast(classDelegateCallAddress, funPtrType);
    int32_t pc = GetCurrentInstructionIndex();
    uint32_t lineNumber = function->GetSourceLine(pc);
    SetCurrentLineNumber(lineNumber);
    CreateCall(funPtrValue, args, instruction.GetFunctionType().ReturnType() != ValueType::none);
}

void NativeCompilerImpl::VisitSetClassDataInst(SetClassDataInst& instruction)
{
    llvm::Value* objectReference = valueStack.Pop();
    llvm::Function* rtSetClassDataPtr = cast<llvm::Function>(module->getOrInsertFunction("RtSetClassDataPtr", 
        GetType(ValueType::none), GetType(ValueType::ulongType), PointerType::get(GetType(ValueType::byteType), 0), nullptr));
    ImportFunction(rtSetClassDataPtr);
    ArgVector args;
    args.push_back(objectReference);
    ClassData* classData = instruction.GetClassData();
    llvm::Value* classDataPtrVar = GetClassDataPtrVar(classData);
    args.push_back(builder.CreateLoad(classDataPtrVar));
    CreateCall(rtSetClassDataPtr, args, false);
}

void NativeCompilerImpl::VisitCreateObjectInst(CreateObjectInst& instruction)
{
    llvm::Function* rtCreateObject = cast<llvm::Function>(module->getOrInsertFunction("RtCreateObject", GetType(ValueType::ulongType), PointerType::get(GetType(ValueType::byteType), 0), nullptr));
    ImportFunction(rtCreateObject);
    ArgVector args;
    Type* type = instruction.GetType();
    ClassData* classData = ClassDataTable::GetClassData(StringPtr(type->Name()));
    llvm::Value* classDataPtrVar = GetClassDataPtrVar(classData);
    args.push_back(builder.CreateLoad(classDataPtrVar));
    CreateCall(rtCreateObject, args, true);
}

void NativeCompilerImpl::VisitCopyObjectInst(CopyObjectInst& instruction)
{
    llvm::Value* objectReference = valueStack.Pop();
    llvm::Function* rtCopyObject = cast<llvm::Function>(module->getOrInsertFunction("RtCopyObject", GetType(ValueType::ulongType), GetType(ValueType::ulongType), nullptr));
    ImportFunction(rtCopyObject);
    ArgVector args;
    args.push_back(objectReference);
    CreateCall(rtCopyObject, args, true);
}

void NativeCompilerImpl::VisitStrLitToStringInst(StrLitToStringInst& instruction)
{
    llvm::Value* value = valueStack.Pop();
    llvm::Function* rtStrLitToString = cast<llvm::Function>(module->getOrInsertFunction("RtStrLitToString", GetType(ValueType::ulongType), PointerType::get(GetType(ValueType::uintType), 0), nullptr));
    ImportFunction(rtStrLitToString);
    ArgVector args;
    args.push_back(value);
    CreateCall(rtStrLitToString, args, true);
}

void NativeCompilerImpl::VisitLoadStringCharInst(LoadStringCharInst& instruction)
{
    llvm::Value* index = valueStack.Pop();
    llvm::Value* str = valueStack.Pop();
    llvm::Function* rtLoadStringChar = cast<llvm::Function>(module->getOrInsertFunction("RtLoadStringChar", GetType(ValueType::charType), GetType(ValueType::intType), GetType(ValueType::ulongType), nullptr));
    ImportFunction(rtLoadStringChar);
    ArgVector args;
    args.push_back(index);
    args.push_back(str); 
    CreateCall(rtLoadStringChar, args, true);
}

void NativeCompilerImpl::VisitDupInst(DupInst& instruction)
{
    valueStack.Dup();
}

void NativeCompilerImpl::VisitSwapInst(SwapInst& instruction)
{
    valueStack.Swap();
}

void NativeCompilerImpl::VisitRotateInst(RotateInst& instruction)
{
    valueStack.Rotate();
}

void NativeCompilerImpl::VisitPopInst(PopInst& instruction)
{
    valueStack.Pop();
}

void NativeCompilerImpl::VisitDownCastInst(DownCastInst& instruction)
{
    llvm::Value* value = valueStack.Pop();
    llvm::Function* rtDownCast = cast<llvm::Function>(module->getOrInsertFunction("RtDownCast", GetType(ValueType::ulongType), GetType(ValueType::ulongType),
        PointerType::get(GetType(ValueType::byteType), 0), nullptr));
    ImportFunction(rtDownCast);
    ArgVector args;
    args.push_back(value);
    ClassData* classData = ClassDataTable::GetClassData(instruction.GetType()->Name());
    llvm::Value* classDataPtrVar = GetClassDataPtrVar(classData);
    args.push_back(builder.CreateLoad(classDataPtrVar));
    CreateCall(rtDownCast, args, true);
}

void NativeCompilerImpl::VisitThrowInst(ThrowInst& instruction)
{
    int32_t pc = GetCurrentInstructionIndex();
    uint32_t lineNumber = function->GetSourceLine(pc);
    SetCurrentLineNumber(lineNumber);
    llvm::Value* exceptionObject = valueStack.Pop(); 
    llvm::Function* rtThrowException = cast<llvm::Function>(module->getOrInsertFunction("RtThrowException", GetType(ValueType::none), GetType(ValueType::objectReference), nullptr));
    ImportFunction(rtThrowException);
    rtThrowException->setAttributes(noReturnFunctionAttributes);
    ArgVector args;
    args.push_back(exceptionObject);
    CreateCall(rtThrowException, args, false);
    if (function->ReturnsValue())
    {
        PushDefaultValue(function->ReturnType());
    }
    CreateReturnFromFunctionOrFunclet();
    currentBasicBlock = nullptr;
}

void NativeCompilerImpl::VisitRethrowInst(RethrowInst& instruction)
{
    GenerateRethrow();
    if (function->ReturnsValue())
    {
        PushDefaultValue(function->ReturnType());
    }
    CreateReturnFromFunctionOrFunclet();
}

void NativeCompilerImpl::VisitBeginTryInst(BeginTryInst& instruction)
{
    exceptionBlockIdStack.push_back(currentExceptionBlockId);
    currentExceptionBlockId = instruction.Index();
}

void NativeCompilerImpl::VisitEndTryInst(EndTryInst& instruction)
{
    currentExceptionBlockId = exceptionBlockIdStack.back();
    exceptionBlockIdStack.pop_back();
}

void NativeCompilerImpl::VisitBeginCatchSectionInst(BeginCatchSectionInst& instruction)
{
    catchSectionExceptionBlockIdStack.push_back(currentCatchSectionExceptionBlockId);
    currentCatchSectionExceptionBlockId = instruction.Index();
    ExceptionBlock* exceptionBlock = function->GetExceptionBlock(currentCatchSectionExceptionBlockId);
    if (!exceptionBlock)
    {
        throw std::runtime_error("exception block " + std::to_string(currentCatchSectionExceptionBlockId) + " not found");
    }
    auto it = unwindTargets.find(currentCatchSectionExceptionBlockId);
    if (it != unwindTargets.cend())
    {
        llvm::BasicBlock* unwindTarget = it->second;
        currentBasicBlock = unwindTarget;
        builder.SetInsertPoint(currentBasicBlock);
        llvm::BasicBlock* terminateTarget = nullptr;
        llvm::Value* parentPad = currentPad;
        if (parentPad == nullptr)
        {
            parentPad = llvm::ConstantTokenNone::get(context);
        }
        llvm::CatchSwitchInst* catchSwitch = builder.CreateCatchSwitch(parentPad, terminateTarget, 1);
        padStack.push_back(currentPad);
        currentPad = catchSwitch;
        padKindStack.push_back(currentPadKind);
        currentPadKind = LlvmPadKind::catchSwitch;
        llvm::BasicBlock* catchPadTarget = llvm::BasicBlock::Create(context, "catchpad" + std::to_string(currentCatchSectionExceptionBlockId), fun);
        catchSwitch->addHandler(catchPadTarget);
        currentBasicBlock = catchPadTarget;
        builder.SetInsertPoint(currentBasicBlock);
        CreateCatchPad();
        auto it = exceptionObjectVariables.find(currentCatchSectionExceptionBlockId);
        if (it != exceptionObjectVariables.cend())
        {
            exceptionPtr = nullptr; // todo: load exception pointer
        }
        else
        {
            throw std::runtime_error("exception object variable for exception block " + std::to_string(currentCatchSectionExceptionBlockId) + " not found");
        }
        int firstCatchId = exceptionBlock->CatchBlocks().front()->Id();
        llvm::BasicBlock* catchTarget = llvm::BasicBlock::Create(context, "catch" + std::to_string(currentCatchSectionExceptionBlockId) + std::to_string(firstCatchId), fun);
        nextHandler[currentCatchSectionExceptionBlockId] = catchTarget;
        builder.CreateBr(catchTarget);
    }
    else
    {
        throw std::runtime_error("unwind target for exception block " + std::to_string(currentCatchSectionExceptionBlockId) + " not found");
    }
}

void NativeCompilerImpl::VisitEndCatchSectionInst(EndCatchSectionInst& instruction)
{
    ExceptionBlock* exceptionBlock = function->GetExceptionBlock(currentCatchSectionExceptionBlockId);
    if (!exceptionBlock)
    {
        throw std::runtime_error("exception block " + std::to_string(currentCatchSectionExceptionBlockId) + " not found");
    }
    auto it = nextHandler.find(currentCatchSectionExceptionBlockId);
    if (it != nextHandler.cend())
    {
        currentBasicBlock = it->second;
        builder.SetInsertPoint(currentBasicBlock);
        nextHandler.erase(currentCatchSectionExceptionBlockId);
        GenerateRethrow();
        if (function->ReturnsValue())
        {
            PushDefaultValue(function->ReturnType());
        }
        CreateReturnFromFunctionOrFunclet();
        currentPad = padStack.back();
        padStack.pop_back();
        currentPadKind = padKindStack.back();
        padKindStack.pop_back();
        currentPad = padStack.back();
        padStack.pop_back();
        currentPadKind = padKindStack.back();
        padKindStack.pop_back();
        currentCatchSectionExceptionBlockId = catchSectionExceptionBlockIdStack.back();
        catchSectionExceptionBlockIdStack.pop_back();
        currentBasicBlock = nullptr;
    }
    else
    {
        throw std::runtime_error("next handler target not set for catch section " + std::to_string(currentCatchSectionExceptionBlockId));
    }
}

void NativeCompilerImpl::VisitBeginCatchInst(BeginCatchInst& instruction)
{
    auto it = nextHandler.find(currentCatchSectionExceptionBlockId);
    if (it != nextHandler.cend())
    {
        currentBasicBlock = it->second;
        builder.SetInsertPoint(currentBasicBlock);
        nextHandler.erase(currentCatchSectionExceptionBlockId);
    }
    else
    {
        throw std::runtime_error("next handler target not set");
    }
    ExceptionBlock* exceptionBlock = function->GetExceptionBlock(currentCatchSectionExceptionBlockId);
    if (!exceptionBlock)
    {
        throw std::runtime_error("exception block " + std::to_string(currentCatchSectionExceptionBlockId) + " not found");
    }
    int catchBlockId = instruction.Index();
    CatchBlock* catchBlock = exceptionBlock->GetCatchBlock(catchBlockId);
    ClassData* classData = ClassDataTable::GetClassData(catchBlock->GetExceptionVarClassTypeFullName());
    llvm::Value* classDataPtrVar = GetClassDataPtrVar(classData);
    llvm::Function* rtHandleException = cast<llvm::Function>(module->getOrInsertFunction("RtHandleException", GetType(ValueType::boolType),
        llvm::PointerType::get(GetType(ValueType::byteType), 0),
        nullptr));
    ImportFunction(rtHandleException);
    ArgVector handleExceptionArgs;
    handleExceptionArgs.push_back(builder.CreateLoad(classDataPtrVar));
    CreateCall(rtHandleException, handleExceptionArgs, true);
    llvm::Value* handleResult = valueStack.Pop();
    llvm::BasicBlock* handlerTarget = llvm::BasicBlock::Create(context, "handler" + std::to_string(currentCatchSectionExceptionBlockId) + std::to_string(catchBlockId), fun);
    llvm::BasicBlock* nextTarget = nullptr;
    if (catchBlockId == int(exceptionBlock->CatchBlocks().size()) - 1) // this is last catch block
    {
        nextTarget = llvm::BasicBlock::Create(context, "nohandler" + std::to_string(currentCatchSectionExceptionBlockId), fun);
    }
    else
    {
        nextTarget = llvm::BasicBlock::Create(context, "catch" + std::to_string(currentCatchSectionExceptionBlockId) + std::to_string(catchBlockId + 1), fun);
    }
    nextHandler[currentCatchSectionExceptionBlockId] = nextTarget;
    builder.CreateCondBr(handleResult, handlerTarget, nextTarget);
    currentBasicBlock = handlerTarget;
    builder.SetInsertPoint(currentBasicBlock);
    llvm::Function* rtGetException = cast<llvm::Function>(module->getOrInsertFunction("RtGetException", GetType(ValueType::objectReference), nullptr));
    ImportFunction(rtGetException);
    ArgVector getExceptionArgs;
    int exceptionVarIndex = catchBlock->GetExceptionVarIndex();
    CreateCall(rtGetException, getExceptionArgs, true);
    builder.CreateStore(valueStack.Pop(), locals[exceptionVarIndex]);
    llvm::Function* rtUnwindFunctionStack = cast<llvm::Function>(module->getOrInsertFunction("RtUnwindFunctionStack", GetType(ValueType::none), 
        llvm::PointerType::get(GetType(ValueType::byteType), 0),
        nullptr));
    ImportFunction(rtUnwindFunctionStack);
    ArgVector unwindFunctionStackArgs;
    unwindFunctionStackArgs.push_back(builder.CreateBitCast(functionStackEntry, llvm::PointerType::get(GetType(ValueType::byteType), 0)));
    CreateCall(rtUnwindFunctionStack, unwindFunctionStackArgs, false);
}

void NativeCompilerImpl::VisitEndCatchInst(EndCatchInst& instruction)
{
    ExceptionBlock* exceptionBlock = function->GetExceptionBlock(currentCatchSectionExceptionBlockId);
    if (!exceptionBlock)
    {
        throw std::runtime_error("exception block " + std::to_string(currentCatchSectionExceptionBlockId) + " not found");
    }
    int32_t next = exceptionBlock->NextTarget();
    llvm::BasicBlock* nextTarget = nullptr;
    if (next != -1)
    {
        auto it = basicBlocks.find(next);
        if (it != basicBlocks.cend())
        {
            nextTarget = basicBlocks[next];
        }
        else
        {
            nextTarget = llvm::BasicBlock::Create(context, "target" + std::to_string(next), fun);
            basicBlocks[next] = nextTarget;
        }
    }
    else
    {
        throw std::runtime_error("next target of exception block " + std::to_string(currentCatchSectionExceptionBlockId) + " not set");
    }
    llvm::Function* rtDisposeException = cast<llvm::Function>(module->getOrInsertFunction("RtDisposeException", GetType(ValueType::none), nullptr));
    ImportFunction(rtDisposeException);
    ArgVector rtDisposeExceptionArgs;
    CreateCall(rtDisposeException, rtDisposeExceptionArgs, false);
    if (currentPadKind == LlvmPadKind::catchPad)
    {
        builder.CreateCatchRet(llvm::cast<llvm::CatchPadInst>(currentPad), nextTarget);
    }
    else
    {
        throw std::runtime_error("catchpad expected");
    }
    currentBasicBlock = nullptr;
}

void NativeCompilerImpl::VisitBeginFinallyInst(BeginFinallyInst& instruction)
{
    int finallyExceptionBlockId = instruction.Index();
    auto it = unwindTargets.find(finallyExceptionBlockId);
    if (it != unwindTargets.cend())
    {
        llvm::BasicBlock* unwindTarget = it->second;
        currentBasicBlock = unwindTarget;
        builder.SetInsertPoint(currentBasicBlock);
        llvm::Value* parentPad = currentPad;
        if (parentPad == nullptr)
        {
            parentPad = llvm::ConstantTokenNone::get(context);
        }
        ArgVector args;
        llvm::CleanupPadInst* cleanUpPad = builder.CreateCleanupPad(parentPad, args);
        padStack.push_back(currentPad);
        currentPad = cleanUpPad;
        padKindStack.push_back(currentPadKind);
        currentPadKind = LlvmPadKind::cleanupPad;
    }
    else
    {
        throw std::runtime_error("unwind target for exception block " + std::to_string(finallyExceptionBlockId) + " not found");
    }
}

void NativeCompilerImpl::VisitEndFinallyInst(EndFinallyInst& instruction)
{
    if (currentPadKind == LlvmPadKind::cleanupPad)
    {
        builder.CreateCleanupRet(cast<CleanupPadInst>(currentPad));
        currentPad = padStack.back();
        padStack.pop_back();
        currentPadKind = padKindStack.back();
        padKindStack.pop_back();
    }
    else
    {
        throw std::runtime_error("cleanuppad expected");
    }
    currentBasicBlock = nullptr;
}

void NativeCompilerImpl::VisitStaticInitInst(StaticInitInst& instruction)
{
    llvm::Function* rtStaticInit = cast<llvm::Function>(module->getOrInsertFunction("RtStaticInit", PointerType::get(GetType(ValueType::byteType), 0), 
        PointerType::get(GetType(ValueType::byteType), 0), 
        nullptr));
    ImportFunction(rtStaticInit);
    ArgVector staticInitArgs;
    ClassData* classData = ClassDataTable::GetClassData(instruction.GetType()->Name());
    llvm::Value* classDataPtrVar = GetClassDataPtrVar(classData);
    staticInitArgs.push_back(builder.CreateLoad(classDataPtrVar));
    CreateCall(rtStaticInit, staticInitArgs, true);
    llvm::Value* staticConstructorAddress = valueStack.Pop();
    llvm::Value* staticConstructorAddressIsNull = builder.CreateICmpEQ(staticConstructorAddress, llvm::Constant::getNullValue(PointerType::get(GetType(ValueType::byteType), 0)));
    BasicBlock* trueTargetBlock = BasicBlock::Create(context, "static_init_ready", fun);
    BasicBlock* continueBlock = BasicBlock::Create(context, "call_static_constructor", fun);
    builder.CreateCondBr(staticConstructorAddressIsNull, trueTargetBlock, continueBlock);
    currentBasicBlock = continueBlock;
    builder.SetInsertPoint(currentBasicBlock);
    std::vector<llvm::Type*> paramTypes;
    llvm::FunctionType* funType = llvm::FunctionType::get(GetType(ValueType::none), paramTypes, false);
    llvm::PointerType* funPtrType = llvm::PointerType::get(funType, 0);
    llvm::Value* funPtrValue = builder.CreateBitCast(staticConstructorAddress, funPtrType);
    ArgVector args;
    CreateCall(funPtrValue, args, false);
    builder.CreateBr(trueTargetBlock);
    currentBasicBlock = trueTargetBlock;
    builder.SetInsertPoint(currentBasicBlock);
}

void NativeCompilerImpl::VisitDoneStaticInitInst(DoneStaticInitInst& instruction)
{
    llvm::Function* rtDoneStaticInit = cast<llvm::Function>(module->getOrInsertFunction("RtDoneStaticInit", GetType(ValueType::none), PointerType::get(GetType(ValueType::byteType), 0), nullptr));
    ImportFunction(rtDoneStaticInit);
    ArgVector args;
    ClassData* classData = ClassDataTable::GetClassData(instruction.GetType()->Name());
    llvm::Value* classDataPtrVar = GetClassDataPtrVar(classData);
    args.push_back(builder.CreateLoad(classDataPtrVar));
    CreateCall(rtDoneStaticInit, args, false);
}

void NativeCompilerImpl::VisitLoadStaticFieldInst(LoadStaticFieldInst& instruction)
{
    llvm::Function* rtLoadStaticField = nullptr;
    switch (instruction.GetFieldType())
    {
        case ValueType::sbyteType:
        {
            rtLoadStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtLoadStaticFieldSb", GetType(ValueType::sbyteType), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::byteType:
        {
            rtLoadStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtLoadStaticFieldBy", GetType(ValueType::byteType), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::shortType:
        {
            rtLoadStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtLoadStaticFieldSh", GetType(ValueType::shortType), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::ushortType:
        {
            rtLoadStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtLoadStaticFieldUs", GetType(ValueType::ushortType), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::intType:
        {
            rtLoadStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtLoadStaticFieldIn", GetType(ValueType::intType), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::uintType:
        {
            rtLoadStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtLoadStaticFieldUi", GetType(ValueType::uintType), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::longType:
        {
            rtLoadStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtLoadStaticFieldLo", GetType(ValueType::longType), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::ulongType:
        {
            rtLoadStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtLoadStaticFieldUl", GetType(ValueType::ulongType), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::floatType:
        {
            rtLoadStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtLoadStaticFieldFl", GetType(ValueType::floatType), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::doubleType:
        {
            rtLoadStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtLoadStaticFieldDo", GetType(ValueType::doubleType), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::charType:
        {
            rtLoadStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtLoadStaticFieldCh", GetType(ValueType::charType), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::boolType:
        {
            rtLoadStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtLoadStaticFieldBo", GetType(ValueType::boolType), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::objectReference:
        {
            rtLoadStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtLoadStaticFieldOb", GetType(ValueType::objectReference), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::intType), nullptr));
            break;
        }
        default:
        {
            throw std::runtime_error("NativeCompiler: invalid field type for load static field");
        }
    }
    ImportFunction(rtLoadStaticField);
    ArgVector args;
    ClassData* classData = ClassDataTable::GetClassData(instruction.GetType()->Name());
    llvm::Value* classDataPtrVar = GetClassDataPtrVar(classData);
    args.push_back(builder.CreateLoad(classDataPtrVar));
    llvm::Value* index = builder.getInt32(instruction.Index());
    args.push_back(index);
    CreateCall(rtLoadStaticField, args, true);
}

void NativeCompilerImpl::VisitStoreStaticFieldInst(StoreStaticFieldInst& instruction)
{
    llvm::Value* value = valueStack.Pop();
    llvm::Function* rtStoreStaticField = nullptr;
    switch (instruction.GetFieldType())
    {
        case ValueType::sbyteType:
        {
            rtStoreStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtStoreStaticFieldSb", GetType(ValueType::none), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::sbyteType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::byteType:
        {
            rtStoreStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtStoreStaticFieldBy", GetType(ValueType::none), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::byteType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::shortType:
        {
            rtStoreStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtStoreStaticFieldSh", GetType(ValueType::none), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::shortType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::ushortType:
        {
            rtStoreStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtStoreStaticFieldUs", GetType(ValueType::none), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::ushortType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::intType:
        {
            rtStoreStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtStoreStaticFieldIn", GetType(ValueType::none), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::intType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::uintType:
        {
            rtStoreStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtStoreStaticFieldUi", GetType(ValueType::none), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::uintType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::longType:
        {
            rtStoreStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtStoreStaticFieldLo", GetType(ValueType::none), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::longType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::ulongType:
        {
            rtStoreStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtStoreStaticFieldUl", GetType(ValueType::none), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::floatType:
        {
            rtStoreStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtStoreStaticFieldFl", GetType(ValueType::none), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::floatType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::doubleType:
        {
            rtStoreStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtStoreStaticFieldDo", GetType(ValueType::none), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::doubleType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::charType:
        {
            rtStoreStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtStoreStaticFieldCh", GetType(ValueType::none), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::charType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::boolType:
        {
            rtStoreStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtStoreStaticFieldBo", GetType(ValueType::none), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::boolType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::objectReference:
        {
            rtStoreStaticField = cast<llvm::Function>(module->getOrInsertFunction("RtStoreStaticFieldOb", GetType(ValueType::none), PointerType::get(GetType(ValueType::byteType), 0),
                GetType(ValueType::objectReference), GetType(ValueType::intType), nullptr));
            break;
        }
        default:
        {
            throw std::runtime_error("NativeCompiler: invalid field type for store static field");
        }
    }
    ImportFunction(rtStoreStaticField);
    ArgVector args;
    ClassData* classData = ClassDataTable::GetClassData(instruction.GetType()->Name());
    llvm::Value* classDataPtrVar = GetClassDataPtrVar(classData);
    args.push_back(builder.CreateLoad(classDataPtrVar));
    args.push_back(value);
    llvm::Value* index = builder.getInt32(instruction.Index());
    args.push_back(index);
    CreateCall(rtStoreStaticField, args, false);
}

void NativeCompilerImpl::VisitEqualObjectNullInst(EqualObjectNullInst& instruction)
{
    llvm::Value* right = valueStack.Pop();
    llvm::Value* left = valueStack.Pop();
    llvm::Value* result = builder.CreateICmpEQ(left, llvm::Constant::getNullValue(GetType(ValueType::ulongType)));
    valueStack.Push(result);
}

void NativeCompilerImpl::VisitEqualNullObjectInst(EqualNullObjectInst& instuction)
{
    llvm::Value* right = valueStack.Pop();
    llvm::Value* left = valueStack.Pop();
    llvm::Value* result = builder.CreateICmpEQ(right, llvm::Constant::getNullValue(GetType(ValueType::ulongType)));
    valueStack.Push(result);
}

void NativeCompilerImpl::VisitBoxBaseInst(BoxBaseInst& instruction)
{
    llvm::Value* value = valueStack.Pop();
    ValueType type = instruction.GetValueType();
    llvm::Function* rtBox = nullptr;
    switch (type)
    {
        case ValueType::sbyteType:
        {
            rtBox = cast<llvm::Function>(module->getOrInsertFunction("RtBoxSb", GetType(ValueType::objectReference), GetType(ValueType::sbyteType), nullptr));
            break;
        }
        case ValueType::byteType:
        {
            rtBox = cast<llvm::Function>(module->getOrInsertFunction("RtBoxBy", GetType(ValueType::objectReference), GetType(ValueType::byteType), nullptr));
            break;
        }
        case ValueType::shortType:
        {
            rtBox = cast<llvm::Function>(module->getOrInsertFunction("RtBoxSh", GetType(ValueType::objectReference), GetType(ValueType::shortType), nullptr));
            break;
        }
        case ValueType::ushortType:
        {
            rtBox = cast<llvm::Function>(module->getOrInsertFunction("RtBoxUs", GetType(ValueType::objectReference), GetType(ValueType::ushortType), nullptr));
            break;
        }
        case ValueType::intType:
        {
            rtBox = cast<llvm::Function>(module->getOrInsertFunction("RtBoxIn", GetType(ValueType::objectReference), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::uintType:
        {
            rtBox = cast<llvm::Function>(module->getOrInsertFunction("RtBoxUi", GetType(ValueType::objectReference), GetType(ValueType::uintType), nullptr));
            break;
        }
        case ValueType::longType:
        {
            rtBox = cast<llvm::Function>(module->getOrInsertFunction("RtBoxLo", GetType(ValueType::objectReference), GetType(ValueType::longType), nullptr));
            break;
        }
        case ValueType::ulongType:
        {
            rtBox = cast<llvm::Function>(module->getOrInsertFunction("RtBoxUl", GetType(ValueType::objectReference), GetType(ValueType::ulongType), nullptr));
            break;
        }
        case ValueType::floatType:
        {
            rtBox = cast<llvm::Function>(module->getOrInsertFunction("RtBoxFl", GetType(ValueType::objectReference), GetType(ValueType::floatType), nullptr));
            break;
        }
        case ValueType::doubleType:
        {
            rtBox = cast<llvm::Function>(module->getOrInsertFunction("RtBoxDo", GetType(ValueType::objectReference), GetType(ValueType::doubleType), nullptr));
            break;
        }
        case ValueType::charType:
        {
            rtBox = cast<llvm::Function>(module->getOrInsertFunction("RtBoxCh", GetType(ValueType::objectReference), GetType(ValueType::charType), nullptr));
            break;
        }
        case ValueType::boolType:
        {
            rtBox = cast<llvm::Function>(module->getOrInsertFunction("RtBoxBo", GetType(ValueType::objectReference), GetType(ValueType::boolType), nullptr));
            break;
        }
        default:
        {
            throw std::runtime_error("NativeCompiler: invalid value type for box operation");
        }
    }
    ImportFunction(rtBox);
    ArgVector args;
    args.push_back(value);
    CreateCall(rtBox, args, true);
}

void NativeCompilerImpl::VisitUnboxBaseInst(UnboxBaseInst& instruction)
{
    llvm::Value* objectReference = valueStack.Pop();
    ValueType type = instruction.GetValueType();
    llvm::Function* rtUnbox = nullptr;
    switch (type)
    {
        case ValueType::sbyteType:
        {
            rtUnbox = cast<llvm::Function>(module->getOrInsertFunction("RtUnboxSb", GetType(ValueType::sbyteType), GetType(ValueType::objectReference), nullptr));
            break;
        }
        case ValueType::byteType:
        {
            rtUnbox = cast<llvm::Function>(module->getOrInsertFunction("RtUnboxBy", GetType(ValueType::byteType), GetType(ValueType::objectReference), nullptr));
            break;
        }
        case ValueType::shortType:
        {
            rtUnbox = cast<llvm::Function>(module->getOrInsertFunction("RtUnboxSh", GetType(ValueType::shortType), GetType(ValueType::objectReference), nullptr));
            break;
        }
        case ValueType::ushortType:
        {
            rtUnbox = cast<llvm::Function>(module->getOrInsertFunction("RtUnboxUs", GetType(ValueType::ushortType), GetType(ValueType::objectReference), nullptr));
            break;
        }
        case ValueType::intType:
        {
            rtUnbox = cast<llvm::Function>(module->getOrInsertFunction("RtUnboxIn", GetType(ValueType::intType), GetType(ValueType::objectReference), nullptr));
            break;
        }
        case ValueType::uintType:
        {
            rtUnbox = cast<llvm::Function>(module->getOrInsertFunction("RtUnboxUi", GetType(ValueType::uintType), GetType(ValueType::objectReference), nullptr));
            break;
        }
        case ValueType::longType:
        {
            rtUnbox = cast<llvm::Function>(module->getOrInsertFunction("RtUnboxLo", GetType(ValueType::longType), GetType(ValueType::objectReference), nullptr));
            break;
        }
        case ValueType::ulongType:
        {
            rtUnbox = cast<llvm::Function>(module->getOrInsertFunction("RtUnboxUl", GetType(ValueType::ulongType), GetType(ValueType::objectReference), nullptr));
            break;
        }
        case ValueType::floatType:
        {
            rtUnbox = cast<llvm::Function>(module->getOrInsertFunction("RtUnboxFl", GetType(ValueType::floatType), GetType(ValueType::objectReference), nullptr));
            break;
        }
        case ValueType::doubleType:
        {
            rtUnbox = cast<llvm::Function>(module->getOrInsertFunction("RtUnboxDo", GetType(ValueType::doubleType), GetType(ValueType::objectReference), nullptr));
            break;
        }
        case ValueType::charType:
        {
            rtUnbox = cast<llvm::Function>(module->getOrInsertFunction("RtUnboxCh", GetType(ValueType::charType), GetType(ValueType::objectReference), nullptr));
            break;
        }
        case ValueType::boolType:
        {
            rtUnbox = cast<llvm::Function>(module->getOrInsertFunction("RtUnboxBo", GetType(ValueType::boolType), GetType(ValueType::objectReference), nullptr));
            break;
        }
        default:
        {
            throw std::runtime_error("NativeCompiler: invalid value type for unbox operation");
        }
    }
    ImportFunction(rtUnbox);
    ArgVector args;
    args.push_back(objectReference);
    CreateCall(rtUnbox, args, true);
}

void NativeCompilerImpl::VisitAllocateArrayElementsInst(AllocateArrayElementsInst& instruction)
{
    llvm::Value* length = valueStack.Pop();
    llvm::Value* arr = valueStack.Pop();
    Type* elementType = instruction.GetType();
    llvm::Function* rtAllocateArrayElements = cast<llvm::Function>(module->getOrInsertFunction("RtAllocateArrayElements", GetType(ValueType::none), GetType(ValueType::ulongType),
        GetType(ValueType::intType), PointerType::get(GetType(ValueType::byteType), 0), nullptr));
    ImportFunction(rtAllocateArrayElements);
    ArgVector args;
    args.push_back(arr);
    args.push_back(length);
    llvm::Value* typePtrVar = GetTypePtrVar(elementType);
    args.push_back(builder.CreateLoad(typePtrVar));
    CreateCall(rtAllocateArrayElements, args, false);
}

void NativeCompilerImpl::VisitIsInst(IsInst& instruction)
{
    llvm::Value* objectReference = valueStack.Pop();
    llvm::Function* rtIs = cast<llvm::Function>(module->getOrInsertFunction("RtIs", GetType(ValueType::boolType), GetType(ValueType::objectReference), PointerType::get(GetType(ValueType::byteType), 0), 
        nullptr));
    ImportFunction(rtIs);
    ArgVector args;
    args.push_back(objectReference);
    ClassData* classData = ClassDataTable::GetClassData(instruction.GetType()->Name());
    llvm::Value* classDataPtrVar = GetClassDataPtrVar(classData);
    args.push_back(builder.CreateLoad(classDataPtrVar));
    CreateCall(rtIs, args, true);
}

void NativeCompilerImpl::VisitAsInst(AsInst& instruction)
{
    llvm::Value* objectReference = valueStack.Pop();
    llvm::Function* rtAs = cast<llvm::Function>(module->getOrInsertFunction("RtAs", GetType(ValueType::objectReference), GetType(ValueType::objectReference), PointerType::get(GetType(ValueType::byteType), 0),
        nullptr));
    ImportFunction(rtAs);
    ArgVector args;
    args.push_back(objectReference);
    ClassData* classData = ClassDataTable::GetClassData(instruction.GetType()->Name());
    llvm::Value* classDataPtrVar = GetClassDataPtrVar(classData);
    args.push_back(builder.CreateLoad(classDataPtrVar));
    CreateCall(rtAs, args, true);
}

void NativeCompilerImpl::VisitFun2DlgInst(Fun2DlgInst& instruction)
{
    Function* function = instruction.GetFunction();
    llvm::Value* functionPtrVar = GetFunctionPtrVar(function);
    valueStack.Push(builder.CreateLoad(functionPtrVar));
}

void NativeCompilerImpl::VisitMemFun2ClassDlgInst(MemFun2ClassDlgInst& instruction)
{
    llvm::Value* classObject = valueStack.Pop();
    llvm::Value* classDelegateObject = valueStack.Pop();
    Function* function = instruction.GetFunction();
    llvm::Value* functionPtrVar = GetFunctionPtrVar(function);
    llvm::Function* rtMemFun2ClassDelegate = cast<llvm::Function>(module->getOrInsertFunction("RtMemFun2ClassDelegate", GetType(ValueType::none), 
        GetType(ValueType::objectReference),
        GetType(ValueType::objectReference),
        PointerType::get(GetType(ValueType::byteType), 0), 
        nullptr));
    ImportFunction(rtMemFun2ClassDelegate);
    ArgVector args;
    args.push_back(classObject);
    args.push_back(classDelegateObject);
    args.push_back(builder.CreateLoad(functionPtrVar));
    CreateCall(rtMemFun2ClassDelegate, args, false);
    valueStack.Push(classDelegateObject);
}

void NativeCompilerImpl::VisitCreateLocalVariableReferenceInst(CreateLocalVariableReferenceInst& instruction)
{
    int32_t localIndex = instruction.LocalIndex();
    if (function->LocalTypes()[localIndex] == ValueType::variableReference)
    {
        valueStack.Push(builder.CreateLoad(locals[localIndex]));
    }
    else
    {
        std::vector<llvm::Type*> variableRefContextElementTypes;
        variableRefContextElementTypes.push_back(PointerType::get(GetType(ValueType::byteType), 0));
        variableRefContextElementTypes.push_back(GetType(ValueType::ulongType));
        variableRefContextElementTypes.push_back(GetType(ValueType::intType));
        variableRefContextElementTypes.push_back(GetType(ValueType::byteType));
        llvm::Value* localPtr = builder.CreateBitCast(locals[localIndex], GetType(ValueType::variableReference));
        llvm::StructType* variableRefContextType = llvm::StructType::get(context, variableRefContextElementTypes);
        AllocaInst* variableRefContext = builder.CreateAlloca(variableRefContextType);
        ArgVector vrcLocalIndeces;
        vrcLocalIndeces.push_back(builder.getInt32(0));
        vrcLocalIndeces.push_back(builder.getInt32(0));
        llvm::Value* vrcLocalPtrPtr = builder.CreateGEP(nullptr, variableRefContext, vrcLocalIndeces);
        builder.CreateStore(localPtr, vrcLocalPtrPtr);
        ArgVector vrcRefTypeIndeces;
        vrcRefTypeIndeces.push_back(builder.getInt32(0));
        vrcRefTypeIndeces.push_back(builder.getInt32(3));
        llvm::Value* vrcRefTypePtr = builder.CreateGEP(nullptr, variableRefContext, vrcRefTypeIndeces);
        builder.CreateStore(builder.getInt8(0), vrcRefTypePtr);
        valueStack.Push(builder.CreateBitCast(variableRefContext, PointerType::get(GetType(ValueType::byteType), 0)));
    }
}

void NativeCompilerImpl::VisitCreateMemberVariableReferenceInst(CreateMemberVariableReferenceInst& instruction)
{
    llvm::Value* objectReference = valueStack.Pop();
    int32_t memberVarIndex = instruction.MemberVarIndex();
    std::vector<llvm::Type*> variableRefContextElementTypes;
    variableRefContextElementTypes.push_back(PointerType::get(GetType(ValueType::byteType), 0));
    variableRefContextElementTypes.push_back(GetType(ValueType::ulongType));
    variableRefContextElementTypes.push_back(GetType(ValueType::intType));
    variableRefContextElementTypes.push_back(GetType(ValueType::byteType));
    llvm::StructType* variableRefContextType = llvm::StructType::get(context, variableRefContextElementTypes);
    AllocaInst* variableRefContext = builder.CreateAlloca(variableRefContextType);
    ArgVector vrcObjectIndeces;
    vrcObjectIndeces.push_back(builder.getInt32(0));
    vrcObjectIndeces.push_back(builder.getInt32(1));
    llvm::Value* vrcObjectPtr = builder.CreateGEP(nullptr, variableRefContext, vrcObjectIndeces);
    builder.CreateStore(objectReference, vrcObjectPtr);
    ArgVector vrcMemberVarIndexIndeces;
    vrcMemberVarIndexIndeces.push_back(builder.getInt32(0));
    vrcMemberVarIndexIndeces.push_back(builder.getInt32(2));
    llvm::Value* vrcMemberVarIndexPtr = builder.CreateGEP(nullptr, variableRefContext, vrcMemberVarIndexIndeces);
    builder.CreateStore(builder.getInt32(memberVarIndex), vrcMemberVarIndexPtr);
    ArgVector vrcRefTypeIndeces;
    vrcRefTypeIndeces.push_back(builder.getInt32(0));
    vrcRefTypeIndeces.push_back(builder.getInt32(3));
    llvm::Value* vrcRefTypePtr = builder.CreateGEP(nullptr, variableRefContext, vrcRefTypeIndeces);
    builder.CreateStore(builder.getInt8(1), vrcRefTypePtr);
    valueStack.Push(builder.CreateBitCast(variableRefContext, PointerType::get(GetType(ValueType::byteType), 0)));
}

void NativeCompilerImpl::VisitLoadVariableReferenceInst(LoadVariableReferenceInst& instruction)
{
    llvm::AllocaInst* variable = builder.CreateAlloca(GetType(instruction.GetType()));
    std::vector<llvm::Type*> variableRefContextElementTypes;
    variableRefContextElementTypes.push_back(PointerType::get(GetType(ValueType::byteType), 0));
    variableRefContextElementTypes.push_back(GetType(ValueType::ulongType));
    variableRefContextElementTypes.push_back(GetType(ValueType::intType));
    variableRefContextElementTypes.push_back(GetType(ValueType::byteType));
    llvm::StructType* variableRefContextType = llvm::StructType::get(context, variableRefContextElementTypes);
    ArgVector vrcRefTypeIndeces;
    vrcRefTypeIndeces.push_back(builder.getInt32(0));
    vrcRefTypeIndeces.push_back(builder.getInt32(3));
    llvm::Value* refContextPtr = locals[instruction.Index()];
    llvm::Value* refContext = builder.CreateLoad(refContextPtr);
    llvm::Value* variableRefContext = builder.CreateBitCast(refContext, PointerType::get(variableRefContextType, 0));
    llvm::Value* vrcRefTypePtr = builder.CreateGEP(nullptr, variableRefContext, vrcRefTypeIndeces);
    llvm::Value* refType = builder.CreateLoad(vrcRefTypePtr);
    llvm::Value* isLocalVariableRef = builder.CreateICmpEQ(refType, builder.getInt8(0));
    llvm::BasicBlock* localVariableRefBlock = llvm::BasicBlock::Create(context, "localVariableRefTarget", fun);
    llvm::BasicBlock* memberVariableRefBlock = llvm::BasicBlock::Create(context, "memberVariableRefTarget", fun);
    builder.CreateCondBr(isLocalVariableRef, localVariableRefBlock, memberVariableRefBlock);
    builder.SetInsertPoint(localVariableRefBlock);
    ArgVector vrcLocalIndeces;
    vrcLocalIndeces.push_back(builder.getInt32(0));
    vrcLocalIndeces.push_back(builder.getInt32(0));
    llvm::Value* vrcLocalPtrPtr = builder.CreateGEP(nullptr, variableRefContext, vrcLocalIndeces);
    llvm::Value* localPtr = builder.CreateLoad(vrcLocalPtrPtr);
    llvm::Type* localPtrType = PointerType::get(GetType(instruction.GetType()), 0);
    llvm::Value* ptrToLocal = builder.CreateBitCast(localPtr, localPtrType);
    llvm::Value* loadedLocal = builder.CreateLoad(ptrToLocal);
    builder.CreateStore(loadedLocal, variable);
    BasicBlock* nextBlock = BasicBlock::Create(context, "nextTarget" + std::to_string(GetCurrentInstructionIndex()), fun);
    builder.CreateBr(nextBlock);
    builder.SetInsertPoint(memberVariableRefBlock);
    ArgVector vrcObjectIndeces;
    vrcObjectIndeces.push_back(builder.getInt32(0));
    vrcObjectIndeces.push_back(builder.getInt32(1));
    llvm::Value* vrcObjectPtr = builder.CreateGEP(nullptr, variableRefContext, vrcObjectIndeces);
    llvm::Value* objectReference = builder.CreateLoad(vrcObjectPtr);
    ArgVector vrcMemberVarIndexIndeces;
    vrcMemberVarIndexIndeces.push_back(builder.getInt32(0));
    vrcMemberVarIndexIndeces.push_back(builder.getInt32(2));
    llvm::Value* vrcMemberVarIndexPtr = builder.CreateGEP(nullptr, variableRefContext, vrcMemberVarIndexIndeces);
    llvm::Value* memberVariableIndex = builder.CreateLoad(vrcMemberVarIndexPtr);
    ArgVector args;
    args.push_back(objectReference);
    args.push_back(memberVariableIndex);
    llvm::Function* callee = nullptr;
    switch (instruction.GetType())
    {
        case ValueType::sbyteType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldSb", GetType(ValueType::sbyteType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::byteType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldBy", GetType(ValueType::byteType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::shortType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldSh", GetType(ValueType::shortType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::ushortType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldUs", GetType(ValueType::ushortType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::intType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldIn", GetType(ValueType::intType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::uintType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldUi", GetType(ValueType::uintType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::longType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldLo", GetType(ValueType::longType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::ulongType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldUl", GetType(ValueType::ulongType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::floatType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldFl", GetType(ValueType::floatType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::doubleType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldDo", GetType(ValueType::doubleType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::charType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldCh", GetType(ValueType::charType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::boolType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldBo", GetType(ValueType::boolType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::objectReference:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldOb", GetType(ValueType::ulongType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        default:
        {
            throw std::runtime_error("NativeCompiler: invalid field type for load variable reference");
        }
    }
    ImportFunction(callee);
    CreateCall(callee, args, true);
    builder.CreateStore(valueStack.Pop(), variable);
    builder.CreateBr(nextBlock);
    currentBasicBlock = nextBlock;
    builder.SetInsertPoint(nextBlock);
    valueStack.Push(builder.CreateLoad(variable));
}

void NativeCompilerImpl::VisitStoreVariableReferenceInst(StoreVariableReferenceInst& instruction)
{
    llvm::Value* value = valueStack.Pop();
    std::vector<llvm::Type*> variableRefContextElementTypes;
    variableRefContextElementTypes.push_back(PointerType::get(GetType(ValueType::byteType), 0));
    variableRefContextElementTypes.push_back(GetType(ValueType::ulongType));
    variableRefContextElementTypes.push_back(GetType(ValueType::intType));
    variableRefContextElementTypes.push_back(GetType(ValueType::byteType));
    llvm::StructType* variableRefContextType = llvm::StructType::get(context, variableRefContextElementTypes);
    ArgVector vrcRefTypeIndeces;
    vrcRefTypeIndeces.push_back(builder.getInt32(0));
    vrcRefTypeIndeces.push_back(builder.getInt32(3));
    llvm::Value* refContextPtr = locals[instruction.Index()];
    llvm::Value* refContext = builder.CreateLoad(refContextPtr);
    llvm::Value* variableRefContext = builder.CreateBitCast(refContext, PointerType::get(variableRefContextType, 0));
    llvm::Value* vrcRefTypePtr = builder.CreateGEP(nullptr, variableRefContext, vrcRefTypeIndeces);
    llvm::Value* refType = builder.CreateLoad(vrcRefTypePtr);
    llvm::Value* isLocalVariableRef = builder.CreateICmpEQ(refType, builder.getInt8(0));
    llvm::BasicBlock* localVariableRefBlock = llvm::BasicBlock::Create(context, "localVariableRefTarget", fun);
    llvm::BasicBlock* memberVariableRefBlock = llvm::BasicBlock::Create(context, "memberVariableRefTarget", fun);
    builder.CreateCondBr(isLocalVariableRef, localVariableRefBlock, memberVariableRefBlock);
    builder.SetInsertPoint(localVariableRefBlock);
    ArgVector vrcLocalIndeces;
    vrcLocalIndeces.push_back(builder.getInt32(0));
    vrcLocalIndeces.push_back(builder.getInt32(0));
    llvm::Value* vrcLocalPtrPtr = builder.CreateGEP(nullptr, variableRefContext, vrcLocalIndeces);
    llvm::Value* localPtr = builder.CreateLoad(vrcLocalPtrPtr);
    llvm::Type* localPtrType = PointerType::get(GetType(instruction.GetType()), 0);
    llvm::Value* ptrToLocal = builder.CreateBitCast(localPtr, localPtrType);
    builder.CreateStore(value, ptrToLocal);
    BasicBlock* nextBlock = BasicBlock::Create(context, "nextTarget" + std::to_string(GetCurrentInstructionIndex()), fun);
    builder.CreateBr(nextBlock);
    builder.SetInsertPoint(memberVariableRefBlock);
    ArgVector vrcObjectIndeces;
    vrcObjectIndeces.push_back(builder.getInt32(0));
    vrcObjectIndeces.push_back(builder.getInt32(1));
    llvm::Value* vrcObjectPtr = builder.CreateGEP(nullptr, variableRefContext, vrcObjectIndeces);
    llvm::Value* objectReference = builder.CreateLoad(vrcObjectPtr);
    ArgVector vrcMemberVarIndexIndeces;
    vrcMemberVarIndexIndeces.push_back(builder.getInt32(0));
    vrcMemberVarIndexIndeces.push_back(builder.getInt32(2));
    llvm::Value* vrcMemberVarIndexPtr = builder.CreateGEP(nullptr, variableRefContext, vrcMemberVarIndexIndeces);
    llvm::Value* memberVariableIndex = builder.CreateLoad(vrcMemberVarIndexPtr);
    ArgVector args;
    args.push_back(objectReference);
    args.push_back(value);
    args.push_back(memberVariableIndex);
    llvm::Function* callee = nullptr;
    switch (instruction.GetType())
    {
        case ValueType::sbyteType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldSb", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::sbyteType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::byteType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldBy", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::byteType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::shortType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldSh", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::shortType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::ushortType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldUs", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::ushortType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::intType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldIn", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::intType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::uintType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldUi", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::uintType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::longType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldLo", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::longType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::ulongType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldUl", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::floatType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldFl", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::floatType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::doubleType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldDo", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::doubleType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::charType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldCh", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::charType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::boolType:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldBo", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::boolType), GetType(ValueType::intType), nullptr));
            break;
        }
        case ValueType::objectReference:
        {
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldOb", GetType(ValueType::none), GetType(ValueType::ulongType), GetType(ValueType::objectReference), GetType(ValueType::intType), nullptr));
            break;
        }
        default:
        {
            throw std::runtime_error("NativeCompiler: invalid field type for store variable reference");
        }
    }
    ImportFunction(callee);
    CreateCall(callee, args, false);
    builder.CreateBr(nextBlock);
    currentBasicBlock = nextBlock;
    builder.SetInsertPoint(nextBlock);
}

llvm::Type* NativeCompilerImpl::GetType(ValueType type)
{
    switch (type)
    {
        case ValueType::none:
        {
            return llvm::Type::getVoidTy(context);
        }
        case ValueType::sbyteType: case ValueType::byteType:
        {
            return llvm::Type::getInt8Ty(context);
        }
        case ValueType::shortType: case ValueType::ushortType:
        {
            return llvm::Type::getInt16Ty(context);
        }
        case ValueType::intType: case ValueType::uintType:
        {
            return llvm::Type::getInt32Ty(context);
        }
        case ValueType::longType: case ValueType::ulongType: case ValueType::objectReference:
        {
            return llvm::Type::getInt64Ty(context);
        }
        case ValueType::floatType:
        {
            return llvm::Type::getFloatTy(context);
        }
        case ValueType::doubleType:
        {
            return llvm::Type::getDoubleTy(context);
        }
        case ValueType::boolType:
        {
            return llvm::Type::getInt1Ty(context);
        }
        case ValueType::charType:
        {
            return llvm::Type::getInt32Ty(context);
        }
        case ValueType::variableReference:
        {
            return PointerType::get(llvm::Type::getInt8Ty(context), 0);
        }
        default:
        {
            throw std::runtime_error("NativeCompiler: invalid value type for GetType()");
        }
    }
}

llvm::IntegerType* NativeCompilerImpl::GetIntegerType(ValueType type)
{
    switch (type)
    {
        case ValueType::sbyteType: case ValueType::byteType:
        {
            return llvm::Type::getInt8Ty(context);
        }
        case ValueType::shortType: case ValueType::ushortType:
        {
            return llvm::Type::getInt16Ty(context);
        }
        case ValueType::intType: case ValueType::uintType:
        {
            return llvm::Type::getInt32Ty(context);
        }
        case ValueType::longType: case ValueType::ulongType:
        {
            return llvm::Type::getInt64Ty(context);
        }
        case ValueType::boolType:
        {
            return llvm::Type::getInt1Ty(context);
        }
        case ValueType::charType:
        {
            return llvm::Type::getInt32Ty(context);
        }
        default:
        {
            throw std::runtime_error("NativeCompiler: invalid value type for GetIntegerType()");
        }
    }
}

bool NativeCompilerImpl::IsSignedType(ValueType type) const
{
    switch (type)
    {
        case ValueType::sbyteType: case ValueType::shortType: case ValueType::intType: case ValueType::longType: return true;
        default: return false;
    }
}

llvm::ConstantInt* NativeCompilerImpl::GetConstantInt(ValueType type, IntegralValue value)
{
    return ConstantInt::get(GetIntegerType(type), value.AsULong(), IsSignedType(type));
}

void NativeCompilerImpl::PushDefaultValue(ValueType type)
{
    switch (type)
    {
        case ValueType::sbyteType: case ValueType::byteType:
        {
            valueStack.Push(builder.getInt8(0));
            break;
        }
        case ValueType::shortType: case ValueType::ushortType:
        {
            valueStack.Push(builder.getInt16(0));
            break;
        }
        case ValueType::intType: case ValueType::uintType:
        {
            valueStack.Push(builder.getInt32(0));
            break;
        }
        case ValueType::longType: case ValueType::ulongType:
        {
            valueStack.Push(builder.getInt64(0));
            break;
        }
        case ValueType::floatType:
        {
            valueStack.Push(ConstantFP::get(llvm::Type::getFloatTy(context), 0.0f));
            break;
        }
        case ValueType::doubleType:
        {
            valueStack.Push(ConstantFP::get(llvm::Type::getDoubleTy(context), 0.0));
            break;
        }
        case ValueType::boolType:
        {
            valueStack.Push(builder.getFalse());
            break;
        }
        case ValueType::charType:
        {
            valueStack.Push(builder.getInt32(0));
            break;
        }
        case ValueType::objectReference:
        {
            valueStack.Push(builder.getInt64(0));
            break;
        }
        case ValueType::functionPtr:
        {
            valueStack.Push(llvm::Constant::getNullValue(PointerType::get(GetType(ValueType::byteType), 0)));
            break;
        }
        default:
        {
            throw std::runtime_error("NativeCompiler: invalid value type for 'def' instruction");
        }
    }
}

void NativeCompilerImpl::PushIntegerMinusOne(ValueType type)
{
    switch (type)
    {
        case ValueType::sbyteType: case ValueType::byteType:
        {
            valueStack.Push(builder.getInt8(-1));
            break;
        }
        case ValueType::shortType: case ValueType::ushortType:
        {
            valueStack.Push(builder.getInt16(-1));
            break;
        }
        case ValueType::intType: case ValueType::uintType:
        {
            valueStack.Push(builder.getInt32(-1));
            break;
        }
        case ValueType::longType: case ValueType::ulongType:
        {
            valueStack.Push(builder.getInt64(-1));
            break;
        }
        default:
        {
            throw std::runtime_error("NativeCompiler: invalid value type for integer minus one");
        }
    }
}

LlvmConv NativeCompilerImpl::GetConversionToULongFrom(ValueType type) const
{
    switch (type)
    {
        case ValueType::sbyteType: case ValueType::shortType: case ValueType::intType:
        {
            return LlvmConv::sext;
        }
        case ValueType::byteType: case ValueType::ushortType: case ValueType::uintType: case ValueType::charType: case ValueType::boolType:
        {
            return LlvmConv::zext;
        }
        case ValueType::ulongType: case ValueType::objectReference:
        {
            return LlvmConv::none;
        }
        case ValueType::longType:
        {
            return LlvmConv::bitcast;
        }
        case ValueType::floatType: case ValueType::doubleType:
        {
            return LlvmConv::fptoui;
        }
        default: 
        {
            throw std::runtime_error("NativeCompiler: invalid type for GetConversionToUlongFrom");
        }
    }
}

llvm::Value* NativeCompilerImpl::CreateConversionToUlong(llvm::Value* from, ValueType fromType)
{
    LlvmConv conv = GetConversionToULongFrom(fromType);
    llvm::Type* ulongType = GetType(ValueType::ulongType);
    switch (conv)
    {
        case LlvmConv::none:
        {
            return from;
        }
        case LlvmConv::zext:
        {
            return builder.CreateZExt(from, ulongType);
        }
        case LlvmConv::sext:
        {
            return builder.CreateSExt(from, ulongType);
        }
        case LlvmConv::trunc:
        {
            return builder.CreateTrunc(from, ulongType);
        }
        case LlvmConv::bitcast:
        {
            return builder.CreateBitCast(from, ulongType);
        }
        case LlvmConv::fptoui:
        {
            return builder.CreateFPToUI(from, ulongType);
        }
        case LlvmConv::fptosi:
        {
            return builder.CreateFPToSI(from, ulongType);
        }
        case LlvmConv::uitofp:
        {
            return builder.CreateUIToFP(from, ulongType);
        }
        case LlvmConv::sitofp:
        {
            return builder.CreateSIToFP(from, ulongType);
        }
        case LlvmConv::fpext:
        {
            return builder.CreateFPExt(from, ulongType);
        }
        case LlvmConv::fptrunc:
        {
            return builder.CreateFPTrunc(from, ulongType);
        }
        default:
        {
            throw std::runtime_error("NativeCompiler: invalid llvm conversion op");
        }
    }
}

LlvmConv NativeCompilerImpl::GetConversionFromULong(ValueType type) const
{
    switch (type)
    {
        case ValueType::sbyteType: case ValueType::byteType: case ValueType::shortType: case ValueType::ushortType: case ValueType::intType: case ValueType::uintType: case ValueType::charType: case ValueType::boolType:
        {
            return LlvmConv::trunc;
        }
        case ValueType::ulongType: case ValueType::objectReference:
        {
            return LlvmConv::none;
        }
        case ValueType::longType:
        {
            return LlvmConv::bitcast;
        }
        case ValueType::floatType: case ValueType::doubleType:
        {
            return LlvmConv::uitofp;
        }
        default:
        {
            throw std::runtime_error("NativeCompiler: invalid type for GetConversionFromULong");
        }
    }
}

llvm::Value* NativeCompilerImpl::CreateConversionFromULong(llvm::Value* from, ValueType toType)
{
    LlvmConv conv = GetConversionFromULong(toType);
    switch (conv)
    {
        case LlvmConv::none:
        {
            return from;
        }
        case LlvmConv::zext:
        {
            return builder.CreateZExt(from, GetType(toType));
        }
        case LlvmConv::sext:
        {
            return builder.CreateSExt(from, GetType(toType));
        }
        case LlvmConv::trunc:
        {
            return builder.CreateTrunc(from, GetType(toType));
        }
        case LlvmConv::bitcast:
        {
            return builder.CreateBitCast(from, GetType(toType));
        }
        case LlvmConv::fptoui:
        {
            return builder.CreateFPToUI(from, GetType(toType));
        }
        case LlvmConv::fptosi:
        {
            return builder.CreateFPToSI(from, GetType(toType));
        }
        case LlvmConv::uitofp:
        {
            return builder.CreateUIToFP(from, GetType(toType));
        }
        case LlvmConv::sitofp:
        {
            return builder.CreateSIToFP(from, GetType(toType));
        }
        case LlvmConv::fpext:
        {
            return builder.CreateFPExt(from, GetType(toType));
        }
        case LlvmConv::fptrunc:
        {
            return builder.CreateFPTrunc(from, GetType(toType));
        }
        default:
        {
            throw std::runtime_error("NativeCompiler: invalid llvm conversion op");
        }
    }
}

void NativeCompilerImpl::InitMaps()
{
    binOpMap["addsb"] = LlvmBinOp::add;
    binOpMap["addby"] = LlvmBinOp::add;
    binOpMap["addsh"] = LlvmBinOp::add;
    binOpMap["addus"] = LlvmBinOp::add;
    binOpMap["addin"] = LlvmBinOp::add;
    binOpMap["addui"] = LlvmBinOp::add;
    binOpMap["addlo"] = LlvmBinOp::add;
    binOpMap["addul"] = LlvmBinOp::add;
    binOpMap["addfl"] = LlvmBinOp::fadd;
    binOpMap["adddo"] = LlvmBinOp::fadd;

    binOpMap["subsb"] = LlvmBinOp::sub;
    binOpMap["subby"] = LlvmBinOp::sub;
    binOpMap["subsh"] = LlvmBinOp::sub;
    binOpMap["subus"] = LlvmBinOp::sub;
    binOpMap["subin"] = LlvmBinOp::sub;
    binOpMap["subui"] = LlvmBinOp::sub;
    binOpMap["sublo"] = LlvmBinOp::sub;
    binOpMap["subul"] = LlvmBinOp::sub;
    binOpMap["subfl"] = LlvmBinOp::fsub;
    binOpMap["subdo"] = LlvmBinOp::fsub;

    binOpMap["mulsb"] = LlvmBinOp::mul;
    binOpMap["mulby"] = LlvmBinOp::mul;
    binOpMap["mulsh"] = LlvmBinOp::mul;
    binOpMap["mulus"] = LlvmBinOp::mul;
    binOpMap["mulin"] = LlvmBinOp::mul;
    binOpMap["mului"] = LlvmBinOp::mul;
    binOpMap["mullo"] = LlvmBinOp::mul;
    binOpMap["mulul"] = LlvmBinOp::mul;
    binOpMap["mulfl"] = LlvmBinOp::fmul;
    binOpMap["muldo"] = LlvmBinOp::fmul;

    binOpMap["divsb"] = LlvmBinOp::sdiv;
    binOpMap["divby"] = LlvmBinOp::udiv;
    binOpMap["divsh"] = LlvmBinOp::sdiv;
    binOpMap["divus"] = LlvmBinOp::udiv;
    binOpMap["divin"] = LlvmBinOp::sdiv;
    binOpMap["divui"] = LlvmBinOp::udiv;
    binOpMap["divlo"] = LlvmBinOp::sdiv;
    binOpMap["divul"] = LlvmBinOp::udiv;
    binOpMap["divfl"] = LlvmBinOp::fdiv;
    binOpMap["divdo"] = LlvmBinOp::fdiv;

    binOpMap["remsb"] = LlvmBinOp::srem;
    binOpMap["remby"] = LlvmBinOp::urem;
    binOpMap["remsh"] = LlvmBinOp::srem;
    binOpMap["remus"] = LlvmBinOp::urem;
    binOpMap["remin"] = LlvmBinOp::srem;
    binOpMap["remui"] = LlvmBinOp::urem;
    binOpMap["remlo"] = LlvmBinOp::srem;
    binOpMap["remul"] = LlvmBinOp::urem;

    binOpMap["andsb"] = LlvmBinOp::and_;
    binOpMap["andby"] = LlvmBinOp::and_;
    binOpMap["andsh"] = LlvmBinOp::and_;
    binOpMap["andus"] = LlvmBinOp::and_;
    binOpMap["andin"] = LlvmBinOp::and_;
    binOpMap["andui"] = LlvmBinOp::and_;
    binOpMap["andlo"] = LlvmBinOp::and_;
    binOpMap["andul"] = LlvmBinOp::and_;

    binOpMap["orsb"] = LlvmBinOp::or_;
    binOpMap["orby"] = LlvmBinOp::or_;
    binOpMap["orsh"] = LlvmBinOp::or_;
    binOpMap["orus"] = LlvmBinOp::or_;
    binOpMap["orin"] = LlvmBinOp::or_;
    binOpMap["orui"] = LlvmBinOp::or_;
    binOpMap["orlo"] = LlvmBinOp::or_;
    binOpMap["orul"] = LlvmBinOp::or_;

    binOpMap["xorsb"] = LlvmBinOp::xor_;
    binOpMap["xorby"] = LlvmBinOp::xor_;
    binOpMap["xorsh"] = LlvmBinOp::xor_;
    binOpMap["xorus"] = LlvmBinOp::xor_;
    binOpMap["xorin"] = LlvmBinOp::xor_;
    binOpMap["xorui"] = LlvmBinOp::xor_;
    binOpMap["xorlo"] = LlvmBinOp::xor_;
    binOpMap["xorul"] = LlvmBinOp::xor_;

    binOpMap["shlsb"] = LlvmBinOp::shl;
    binOpMap["shlby"] = LlvmBinOp::shl;
    binOpMap["shlsh"] = LlvmBinOp::shl;
    binOpMap["shlus"] = LlvmBinOp::shl;
    binOpMap["shlin"] = LlvmBinOp::shl;
    binOpMap["shlui"] = LlvmBinOp::shl;
    binOpMap["shllo"] = LlvmBinOp::shl;
    binOpMap["shlul"] = LlvmBinOp::shl;

    binOpMap["shrsb"] = LlvmBinOp::ashr;
    binOpMap["shrby"] = LlvmBinOp::lshr;
    binOpMap["shrsh"] = LlvmBinOp::ashr;
    binOpMap["shrus"] = LlvmBinOp::lshr;
    binOpMap["shrin"] = LlvmBinOp::ashr;
    binOpMap["shrui"] = LlvmBinOp::lshr;
    binOpMap["shrlo"] = LlvmBinOp::ashr;
    binOpMap["shrul"] = LlvmBinOp::lshr;

    binPredMap["equalsb"] = LlvmBinPred::ieq;
    binPredMap["equalby"] = LlvmBinPred::ieq;
    binPredMap["equalsh"] = LlvmBinPred::ieq;
    binPredMap["equalus"] = LlvmBinPred::ieq;
    binPredMap["equalin"] = LlvmBinPred::ieq;
    binPredMap["equalui"] = LlvmBinPred::ieq;
    binPredMap["equallo"] = LlvmBinPred::ieq;
    binPredMap["equalul"] = LlvmBinPred::ieq;
    binPredMap["equalfl"] = LlvmBinPred::oeq;
    binPredMap["equaldo"] = LlvmBinPred::oeq;
    binPredMap["equalch"] = LlvmBinPred::ieq;
    binPredMap["equalbo"] = LlvmBinPred::ieq;

    binPredMap["lesssb"] = LlvmBinPred::slt;
    binPredMap["lessby"] = LlvmBinPred::ult;
    binPredMap["lesssh"] = LlvmBinPred::slt;
    binPredMap["lessus"] = LlvmBinPred::ult;
    binPredMap["lessin"] = LlvmBinPred::slt;
    binPredMap["lessui"] = LlvmBinPred::ult;
    binPredMap["lesslo"] = LlvmBinPred::slt;
    binPredMap["lessul"] = LlvmBinPred::ult;
    binPredMap["lessfl"] = LlvmBinPred::olt;
    binPredMap["lessdo"] = LlvmBinPred::olt;
    binPredMap["lessch"] = LlvmBinPred::ult;

    convMap["sb2by"] = LlvmConv::bitcast;
    convMap["sb2sh"] = LlvmConv::sext;
    convMap["sb2us"] = LlvmConv::sext;
    convMap["sb2in"] = LlvmConv::sext;
    convMap["sb2ui"] = LlvmConv::sext;
    convMap["sb2lo"] = LlvmConv::sext;
    convMap["sb2ul"] = LlvmConv::sext;
    convMap["sb2fl"] = LlvmConv::sitofp;
    convMap["sb2do"] = LlvmConv::sitofp;
    convMap["sb2ch"] = LlvmConv::zext;
    convMap["sb2bo"] = LlvmConv::trunc;

    convMap["by2sb"] = LlvmConv::bitcast;
    convMap["by2sh"] = LlvmConv::zext;
    convMap["by2us"] = LlvmConv::zext;
    convMap["by2in"] = LlvmConv::zext;
    convMap["by2ui"] = LlvmConv::zext;
    convMap["by2lo"] = LlvmConv::zext;
    convMap["by2ul"] = LlvmConv::zext;
    convMap["by2fl"] = LlvmConv::uitofp;
    convMap["by2do"] = LlvmConv::uitofp;
    convMap["by2ch"] = LlvmConv::zext;
    convMap["by2bo"] = LlvmConv::trunc;

    convMap["sh2sb"] = LlvmConv::trunc;
    convMap["sh2by"] = LlvmConv::trunc;
    convMap["sh2us"] = LlvmConv::bitcast;
    convMap["sh2in"] = LlvmConv::sext;
    convMap["sh2ui"] = LlvmConv::sext;
    convMap["sh2lo"] = LlvmConv::sext;
    convMap["sh2ul"] = LlvmConv::sext;
    convMap["sh2fl"] = LlvmConv::sitofp;
    convMap["sh2do"] = LlvmConv::sitofp;
    convMap["sh2ch"] = LlvmConv::zext;
    convMap["sh2bo"] = LlvmConv::trunc;

    convMap["us2sb"] = LlvmConv::trunc;
    convMap["us2by"] = LlvmConv::trunc;
    convMap["us2sh"] = LlvmConv::bitcast;
    convMap["us2in"] = LlvmConv::zext;
    convMap["us2ui"] = LlvmConv::zext;
    convMap["us2lo"] = LlvmConv::zext;
    convMap["us2ul"] = LlvmConv::zext;
    convMap["us2fl"] = LlvmConv::uitofp;
    convMap["us2do"] = LlvmConv::uitofp;
    convMap["us2ch"] = LlvmConv::zext;
    convMap["us2bo"] = LlvmConv::trunc;

    convMap["in2sb"] = LlvmConv::trunc;
    convMap["in2by"] = LlvmConv::trunc;
    convMap["in2sh"] = LlvmConv::trunc;
    convMap["in2us"] = LlvmConv::trunc;
    convMap["in2ui"] = LlvmConv::bitcast;
    convMap["in2lo"] = LlvmConv::sext;
    convMap["in2ul"] = LlvmConv::sext;
    convMap["in2fl"] = LlvmConv::sitofp;
    convMap["in2do"] = LlvmConv::sitofp;
    convMap["in2ch"] = LlvmConv::bitcast;
    convMap["in2bo"] = LlvmConv::trunc;

    convMap["ui2sb"] = LlvmConv::trunc;
    convMap["ui2by"] = LlvmConv::trunc;
    convMap["ui2sh"] = LlvmConv::trunc;
    convMap["ui2us"] = LlvmConv::trunc;
    convMap["ui2in"] = LlvmConv::bitcast;
    convMap["ui2lo"] = LlvmConv::zext;
    convMap["ui2ul"] = LlvmConv::zext;
    convMap["ui2fl"] = LlvmConv::uitofp;
    convMap["ui2do"] = LlvmConv::uitofp;
    convMap["ui2ch"] = LlvmConv::bitcast;
    convMap["ui2bo"] = LlvmConv::trunc;

    convMap["lo2sb"] = LlvmConv::trunc;
    convMap["lo2by"] = LlvmConv::trunc;
    convMap["lo2sh"] = LlvmConv::trunc;
    convMap["lo2us"] = LlvmConv::trunc;
    convMap["lo2in"] = LlvmConv::trunc;
    convMap["lo2ui"] = LlvmConv::trunc;
    convMap["lo2ul"] = LlvmConv::bitcast;
    convMap["lo2fl"] = LlvmConv::sitofp;
    convMap["lo2do"] = LlvmConv::sitofp;
    convMap["lo2ch"] = LlvmConv::trunc;
    convMap["lo2bo"] = LlvmConv::trunc;

    convMap["ul2sb"] = LlvmConv::trunc;
    convMap["ul2by"] = LlvmConv::trunc;
    convMap["ul2sh"] = LlvmConv::trunc;
    convMap["ul2us"] = LlvmConv::trunc;
    convMap["ul2in"] = LlvmConv::trunc;
    convMap["ul2ui"] = LlvmConv::trunc;
    convMap["ul2lo"] = LlvmConv::bitcast;
    convMap["ul2fl"] = LlvmConv::uitofp;
    convMap["ul2do"] = LlvmConv::uitofp;
    convMap["ul2ch"] = LlvmConv::trunc;
    convMap["ul2bo"] = LlvmConv::trunc;

    convMap["fl2sb"] = LlvmConv::fptosi;
    convMap["fl2by"] = LlvmConv::fptoui;
    convMap["fl2sh"] = LlvmConv::fptosi;
    convMap["fl2us"] = LlvmConv::fptoui;
    convMap["fl2in"] = LlvmConv::fptosi;
    convMap["fl2ui"] = LlvmConv::fptoui;
    convMap["fl2lo"] = LlvmConv::fptosi;
    convMap["fl2ul"] = LlvmConv::fptoui;
    convMap["fl2do"] = LlvmConv::fpext;
    convMap["fl2ch"] = LlvmConv::fptoui;
    convMap["fl2bo"] = LlvmConv::fptoui;

    convMap["do2sb"] = LlvmConv::fptosi;
    convMap["do2by"] = LlvmConv::fptoui;
    convMap["do2sh"] = LlvmConv::fptosi;
    convMap["do2us"] = LlvmConv::fptoui;
    convMap["do2in"] = LlvmConv::fptosi;
    convMap["do2ui"] = LlvmConv::fptoui;
    convMap["do2lo"] = LlvmConv::fptosi;
    convMap["do2ul"] = LlvmConv::fptoui;
    convMap["do2fl"] = LlvmConv::fptrunc;
    convMap["do2ch"] = LlvmConv::fptoui;
    convMap["do2bo"] = LlvmConv::fptoui;

    convMap["ch2sb"] = LlvmConv::trunc;
    convMap["ch2by"] = LlvmConv::trunc;
    convMap["ch2sh"] = LlvmConv::trunc;
    convMap["ch2us"] = LlvmConv::trunc;
    convMap["ch2in"] = LlvmConv::bitcast;
    convMap["ch2ui"] = LlvmConv::bitcast;
    convMap["ch2lo"] = LlvmConv::zext;
    convMap["ch2ul"] = LlvmConv::zext;
    convMap["ch2fl"] = LlvmConv::uitofp;
    convMap["ch2do"] = LlvmConv::uitofp;
    convMap["ch2bo"] = LlvmConv::trunc;

    convMap["bo2sb"] = LlvmConv::zext;
    convMap["bo2by"] = LlvmConv::zext;
    convMap["bo2sh"] = LlvmConv::zext;
    convMap["bo2us"] = LlvmConv::zext;
    convMap["bo2in"] = LlvmConv::zext;
    convMap["bo2ui"] = LlvmConv::zext;
    convMap["bo2lo"] = LlvmConv::zext;
    convMap["bo2ul"] = LlvmConv::zext;
    convMap["bo2fl"] = LlvmConv::uitofp;
    convMap["bo2do"] = LlvmConv::uitofp;
    convMap["bo2ch"] = LlvmConv::zext;
}

void NativeCompilerImpl::InitOpFunMap(ConstantPool& constantPool)
{
    opFunMap[StringPtr(constantPool.GetConstant(constantPool.Install(U"operator!")).Value().AsStringLiteral())] = "op_not";
    opFunMap[StringPtr(constantPool.GetConstant(constantPool.Install(U"operator+")).Value().AsStringLiteral())] = "op_plus";
    opFunMap[StringPtr(constantPool.GetConstant(constantPool.Install(U"operator-")).Value().AsStringLiteral())] = "op_minus";
    opFunMap[StringPtr(constantPool.GetConstant(constantPool.Install(U"operator~")).Value().AsStringLiteral())] = "op_cpl";
    opFunMap[StringPtr(constantPool.GetConstant(constantPool.Install(U"operator*")).Value().AsStringLiteral())] = "op_mul";
    opFunMap[StringPtr(constantPool.GetConstant(constantPool.Install(U"operator/")).Value().AsStringLiteral())] = "op_div";
    opFunMap[StringPtr(constantPool.GetConstant(constantPool.Install(U"operator%")).Value().AsStringLiteral())] = "op_rem";
    opFunMap[StringPtr(constantPool.GetConstant(constantPool.Install(U"operator&")).Value().AsStringLiteral())] = "op_and";
    opFunMap[StringPtr(constantPool.GetConstant(constantPool.Install(U"operator|")).Value().AsStringLiteral())] = "op_or";
    opFunMap[StringPtr(constantPool.GetConstant(constantPool.Install(U"operator^")).Value().AsStringLiteral())] = "op_xor";
    opFunMap[StringPtr(constantPool.GetConstant(constantPool.Install(U"operator<<")).Value().AsStringLiteral())] = "op_shl";
    opFunMap[StringPtr(constantPool.GetConstant(constantPool.Install(U"operator>>")).Value().AsStringLiteral())] = "op_shr";
    opFunMap[StringPtr(constantPool.GetConstant(constantPool.Install(U"operator==")).Value().AsStringLiteral())] = "op_equal";
    opFunMap[StringPtr(constantPool.GetConstant(constantPool.Install(U"operator<")).Value().AsStringLiteral())] = "op_less";
}

NativeCompiler::NativeCompiler() : impl(new NativeCompilerImpl())
{
}

NativeCompiler::~NativeCompiler()
{
    delete impl;
}

void NativeCompiler::Compile(const std::string& assemblyFilePath, std::string& nativeImportLibraryFilePath, std::string& nativeSharedLibraryFilePath)
{
    Machine machine;
    FunctionTable::Init();
    ClassDataTable::Init();
    TypeInit();
    AssemblyTable::Init();
    Assembly assembly(machine);
    assembly.SetReadClassNodes();
    assembly.Load(assemblyFilePath);
    std::string assemblyName = ToUtf8(assembly.Name().Value());
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "Compiling assembly '" + assemblyName + "' to native object code..." << std::endl;
    }
    impl->Init(assembly);
    MachineFunctionTable& machineFunctionTable = assembly.GetMachineFunctionTable();
    for (const std::unique_ptr<Function>& function : machineFunctionTable.MachineFunctions())
    {
        function->Accept(*impl);
    }
    impl->Done();
    SymbolWriter writer(assemblyFilePath);
    assembly.SetNative();
    assembly.Write(writer);
    nativeImportLibraryFilePath = Path::Combine(Path::GetDirectoryName(assemblyFilePath), assembly.NativeImportLibraryFileName());
    nativeSharedLibraryFilePath = Path::Combine(Path::GetDirectoryName(assemblyFilePath), assembly.NativeSharedLibraryFileName());
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "Assembly '" + assemblyName + "' successfully compiled to native object code." << std::endl;
        std::cout << "=> " << assemblyFilePath << std::endl;
    }
}

} } // namespace cminor::build
