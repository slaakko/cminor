// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/jit/JitCompiler.hpp>
#include <cminor/machine/MachineFunctionVisitor.hpp>
#include <cminor/machine/Instruction.hpp>
#include <cminor/machine/Constant.hpp>
#include <fstream>
#include <iostream>
#ifdef _WIN32
#pragma warning(disable:4267)
#pragma warning(disable:4244)
#pragma warning(disable:4141)
#pragma warning(disable:4624)
#pragma warning(disable:4291)
#endif
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Argument.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/AssemblyAnnotationWriter.h>
#include <llvm/IR/Mangler.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/RuntimeDyld.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/ExecutionEngine/Orc/CompileOnDemandLayer.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/JITSymbol.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/LambdaResolver.h>
#include <llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Target/TargetMachine.h>
#ifdef _WIN32
#pragma warning(default:4267)
#pragma warning(default:4244)
#pragma warning(default:4141)
#pragma warning(default:4624)
#pragma warning(default:4291)
#endif

namespace cminor { namespace jit {

using namespace llvm;
using namespace llvm::orc;

class ValueStack
{
public:
    void Push(Value* value)
    {
        s.push_back(value);
    }
    Value* Pop()
    {
        Assert(!s.empty(), "value stack is empty");
        Value* top = s.back();
        s.pop_back();
        return top;
    }
private:
    std::vector<Value*> s;
};

enum class LlvmBinOp
{
    add, fadd, sub, fsub, mul, fmul, udiv, sdiv, fdiv, urem, srem, shl, lshr, ashr, and_, or_, xor_
};

enum class LlvmBinPred
{
    ieq, oeq, ult, slt, olt
};

enum class LlvmConv
{
    zext, sext, trunc, bitcast, fptoui, fptosi, uitofp, sitofp, fpext, fptrunc
};

class JitCompilerImpl : public MachineFunctionVisitor
{
public:
    JitCompilerImpl();
    int ProgramReturnValue() const { return programReturnValue;  }
    void BeginVisitFunction(cminor::machine::Function& function) override;
    void EndVisitFunction(cminor::machine::Function& function) override;
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
private:
    int programReturnValue;
    std::ofstream stream;
    std::vector<AllocaInst*> locals;
    Function* function;
    ConstantPool* constantPool;
    ValueStack valueStack;
    LLVMContext context;
    IRBuilder<> builder;
    std::unique_ptr<Module> module;
    std::unique_ptr<TargetMachine> targetMachine;
    std::unique_ptr<DataLayout> dataLayout;
    std::unique_ptr<ObjectLinkingLayer<>> objectLinkingLayer;
    std::unique_ptr<IRCompileLayer<ObjectLinkingLayer<>>> compileLayer;
    std::unique_ptr<JITCompileCallbackManager> compileCallbackManager;
    std::unique_ptr<CompileOnDemandLayer<IRCompileLayer<ObjectLinkingLayer<>>>> compileOnDemandLayer;
    llvm::Function* fun;
    std::string groupName;
    std::unordered_map<std::string, LlvmBinOp> binOpMap;
    std::unordered_map<std::string, LlvmBinPred> binPredMap;
    std::unordered_map<std::string, LlvmConv> convMap;
    llvm::BasicBlock* currentBasicBlock;
    std::unordered_map<int, llvm::BasicBlock*> basicBlocks;
    std::unordered_set<int32_t> jumpTargets;
    typedef CompileOnDemandLayer<IRCompileLayer<ObjectLinkingLayer<>>>::ModuleSetHandleT ModuleSetHandle;
    ModuleSetHandle AddModule();
    JITSymbol FindSymbol(const std::string& name);
    std::string Mangle(const std::string& name);
    void PushDefaultValue(ValueType type);
    void PushIntegerMinusOne(ValueType type);
    llvm::Type* GetType(ValueType type);
    void InitMaps();
};

JitCompilerImpl::JitCompilerImpl() :
    programReturnValue(0), stream("C:\\Temp\\jit.log"), module(), targetMachine(), dataLayout(), objectLinkingLayer(), compileLayer(), fun(nullptr), currentBasicBlock(nullptr), builder(context), 
    function(nullptr), constantPool(nullptr)
{
    EngineBuilder engineBuilder;
    targetMachine.reset(engineBuilder.selectTarget());
    dataLayout.reset(new DataLayout(targetMachine->createDataLayout()));
    objectLinkingLayer.reset(new ObjectLinkingLayer<>());
    compileLayer.reset(new IRCompileLayer<ObjectLinkingLayer<>>(*objectLinkingLayer, SimpleCompiler(*targetMachine)));
    TargetAddress errorHandlerAddress = 0;
    compileCallbackManager = std::move(createLocalCompileCallbackManager(targetMachine->getTargetTriple(), errorHandlerAddress));
    compileOnDemandLayer.reset(new CompileOnDemandLayer<IRCompileLayer<ObjectLinkingLayer<>>>(*compileLayer, [this](llvm::Function& f) { return std::set<llvm::Function*>({ &f }); },
        *compileCallbackManager, createLocalIndirectStubsManagerBuilder(targetMachine->getTargetTriple())));
    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
    InitMaps();
}

JitCompilerImpl::ModuleSetHandle JitCompilerImpl::AddModule()
{
    auto resolver = createLambdaResolver(
        [&](const std::string &name) {
        if (auto sym = compileOnDemandLayer->findSymbol(name, false))
            return sym.toRuntimeDyldSymbol();
        return RuntimeDyld::SymbolInfo(nullptr);
    },
        [](const std::string &name) {
        if (auto symAddr =
            RTDyldMemoryManager::getSymbolAddressInProcess(name))
            return RuntimeDyld::SymbolInfo(symAddr, JITSymbolFlags::Exported);
        return RuntimeDyld::SymbolInfo(nullptr);
    });
    std::vector<std::unique_ptr<Module>> moduleSet;
    moduleSet.push_back(std::move(module));
    return compileOnDemandLayer->addModuleSet(std::move(moduleSet), std::unique_ptr<SectionMemoryManager>(new SectionMemoryManager()), std::move(resolver));
}

JITSymbol JitCompilerImpl::FindSymbol(const std::string& name)
{
    return compileOnDemandLayer->findSymbol(Mangle(name), false);
}

std::string JitCompilerImpl::Mangle(const std::string& name)
{
    std::string mangledName;
    raw_string_ostream mangledNameStream(mangledName);
    Mangler::getNameWithPrefix(mangledNameStream, name, *dataLayout);
    return mangledNameStream.str();
}

void JitCompilerImpl::BeginVisitFunction(cminor::machine::Function& function)
{
    basicBlocks.clear();
    jumpTargets.clear();
    groupName = ToUtf8(function.GroupName().Value().AsStringLiteral());
    module.reset(new Module(groupName.c_str(), context));
    module->setDataLayout(*dataLayout);
    CodeFormatter formatter(stream);
    formatter.WriteLine("before cleaning:");
    function.Dump(formatter);
    function.RemoveUnreachableInstructions(jumpTargets);
    formatter.WriteLine("after cleaning:");
    function.Dump(formatter);
    llvm::Type* returnType = GetType(function.ReturnType());
    std::vector<llvm::Type*> paramTypes;
    for (ValueType pvt : function.ParameterTypes())
    {
        llvm::Type* paramType = GetType(pvt);
        paramTypes.push_back(paramType);
    }
    llvm::FunctionType* funType = llvm::FunctionType::get(returnType, paramTypes, false);
    fun = cast<llvm::Function>(module->getOrInsertFunction(groupName, funType));
    BasicBlock* entryBlock = BasicBlock::Create(context, "entry", fun);
    builder.SetInsertPoint(entryBlock);
    for (ValueType lt : function.LocalTypes())
    {
        llvm::Type* type = GetType(lt);
        llvm::AllocaInst* allocaInst = builder.CreateAlloca(type);
        locals.push_back(allocaInst);
    }
    this->function = &function;
    constantPool = &function.GetConstantPool();
    currentBasicBlock = entryBlock;
}

void JitCompilerImpl::EndVisitFunction(cminor::machine::Function& function)
{
    if (!function.ReturnsValue())
    {
        builder.CreateRetVoid();
    }
    raw_os_ostream ros(stream);
    Module* m = module.get();
    if (verifyModule(*m, &ros))
    {
        stream << "module verification failed" << std::endl;
        throw std::runtime_error("module verification failed");
    }
    m->print(ros, nullptr);
    AddModule();
    JITSymbol symbol = FindSymbol(groupName);
    if (symbol)
    {
        llvm::orc::TargetAddress targetAddr = symbol.getAddress();
        void* addr = reinterpret_cast<void*>(targetAddr);
        if (function.ReturnsValue())
        {
            int (*main)(void) = static_cast<int(*)(void)>(addr);
            programReturnValue = main();
        }
        else
        {
            void (*main)(void) = static_cast<void(*)(void)>(addr);
            main();
        }
    }
    else
    {
        throw std::runtime_error("function symbol not found");
    }
    TargetAddress addr = RTDyldMemoryManager::getSymbolAddressInProcess("RtLoadFieldSb");
    std::cout << addr << std::endl;
}

void JitCompilerImpl::BeginVisitInstruction(int instructionNumber, bool prevEndsBasicBlock, Instruction* inst)
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
    else if (jumpTargets.find(instructionNumber) != jumpTargets.cend())
    {
        currentBasicBlock = BasicBlock::Create(context, "target", fun);
        basicBlocks[instructionNumber] = currentBasicBlock;
        if (!prevEndsBasicBlock)
        {
            builder.CreateBr(currentBasicBlock);
        }
        builder.SetInsertPoint(currentBasicBlock);
    }
}

void JitCompilerImpl::VisitInvalidInst(InvalidInst& instruction)
{
    throw std::runtime_error("invalid instruction encountered");
}

void JitCompilerImpl::VisitLoadDefaultValueBaseInst(LoadDefaultValueBaseInst& instruction)
{
    ValueType valueType = instruction.GetValueType();
    PushDefaultValue(valueType);
}

void JitCompilerImpl::VisitUnaryOpBaseInst(UnaryOpBaseInst& instruction)
{
    if (instruction.GroupName() == "neg")
    {
        Value* operand = valueStack.Pop();
        Value* result = builder.CreateNeg(operand);
        valueStack.Push(result);
    }
    else if (instruction.GroupName() == "cpl")
    {
        PushIntegerMinusOne(instruction.GetValueType());
        Value* left = valueStack.Pop();
        Value* right = valueStack.Pop();
        Value* result = builder.CreateXor(left, right);
        valueStack.Push(result);
    }
}

void JitCompilerImpl::VisitBinaryOpBaseInst(BinaryOpBaseInst& instruction)
{
    auto it = binOpMap.find(instruction.Name());
    if (it != binOpMap.cend())
    {
        Value* right = valueStack.Pop();
        Value* left = valueStack.Pop();
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
                throw std::runtime_error("jit: invalid llvm bin op");
            }
        }
    }
    else
    {
        throw std::runtime_error("jit: binary operation not found");
    }
}

void JitCompilerImpl::VisitBinaryPredBaseInst(BinaryPredBaseInst& instruction)
{
    auto it = binPredMap.find(instruction.Name());
    if (it != binPredMap.cend())
    {
        Value* right = valueStack.Pop();
        Value* left = valueStack.Pop();
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
                throw std::runtime_error("jit: invalid llvm bin pred");
            }
        }
    }
    else
    {
        throw std::runtime_error("jit: binary predicate operation not found");
    }
}

void JitCompilerImpl::VisitLogicalNotInst(LogicalNotInst& instruction)
{
    valueStack.Push(builder.CreateNot(valueStack.Pop()));
}

void JitCompilerImpl::VisitLoadLocalInst(int32_t localIndex)
{
    valueStack.Push(builder.CreateLoad(locals[localIndex]));
}

void JitCompilerImpl::VisitStoreLocalInst(int32_t localIndex)
{
    Value* value = valueStack.Pop();
    builder.CreateStore(value, locals[localIndex]);
}

void JitCompilerImpl::VisitLoadFieldInst(int32_t fieldIndex, ValueType fieldType)
{
    Value* objectReference = valueStack.Pop();
    std::vector<Value*> args;
    args.push_back(objectReference);
    Value* index = builder.getInt32(fieldIndex);
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
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtLoadFieldOb", GetType(ValueType::objectReference), GetType(ValueType::ulongType), GetType(ValueType::intType), nullptr));
            break;
        }
        default:
        {
            throw std::runtime_error("jit: invalid field type for load field");
        }
    }
    valueStack.Push(builder.CreateCall(callee, args));
}

void JitCompilerImpl::VisitStoreFieldInst(int32_t fieldIndex, ValueType fieldType)
{
    Value* objectReference = valueStack.Pop();
    Value* fieldValue = valueStack.Pop();
    std::vector<Value*> args;
    args.push_back(objectReference);
    args.push_back(fieldValue);
    Value* index = builder.getInt32(fieldIndex);
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
            callee = cast<llvm::Function>(module->getOrInsertFunction("RtStoreFieldDo", GetType(ValueType::none), GetType(ValueType::doubleType), GetType(ValueType::intType), nullptr));
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
            throw std::runtime_error("jit: invalid field type for store field");
        }
    }
    builder.CreateCall(callee, args);
}

void JitCompilerImpl::VisitLoadElemInst(LoadElemInst& instruction)
{
    Value* index = valueStack.Pop();
    Value* arr = valueStack.Pop();
    std::vector<Value*> args;
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
            throw std::runtime_error("jit: invalid field type for load element");
        }
    }
    valueStack.Push(builder.CreateCall(callee, args));
}

void JitCompilerImpl::VisitStoreElemInst(StoreElemInst& instruction)
{
    Value* index = valueStack.Pop();
    Value* arr = valueStack.Pop();
    Value* elementValue = valueStack.Pop();
    std::vector<Value*> args;
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
            throw std::runtime_error("jit: invalid field type for store element");
        }
    }
    builder.CreateCall(callee, args);
}

void JitCompilerImpl::VisitLoadConstantInst(int32_t constantIndex)
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
            // todo
            throw std::runtime_error("jit: invalid constant type");
            break;
        }
        case ValueType::objectReference:
        {
            valueStack.Push(builder.getInt64(value.Value()));
            break;
        }
        default:
        {
            throw std::runtime_error("jit: invalid constant type");
        }
    }
}

void JitCompilerImpl::VisitReceiveInst(ReceiveInst& instruction)
{
    int i = 0;
    for (auto it = fun->arg_begin(); it != fun->arg_end(); ++it)
    {
        llvm::AllocaInst* local = locals[i];
        builder.CreateStore(&*it, local);
        ++i;
    }
}

void JitCompilerImpl::VisitConversionBaseInst(ConversionBaseInst& instruction)
{
    auto it = convMap.find(instruction.Name());
    if (it != convMap.cend())
    {
        LlvmConv conv = it->second;
        Value* from = valueStack.Pop();
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
                throw std::runtime_error("jit: invalid llvm conv");
            }
        }
    }
}

void JitCompilerImpl::VisitJumpInst(JumpInst& instruction)
{
    int32_t target = instruction.Index();
    if (target == endOfFunction)
    {
        if (function->ReturnsValue())
        {
            Value* returnValue = valueStack.Pop();
            builder.CreateRet(returnValue);
        }
        else
        {
            builder.CreateRetVoid();
        }
    }
    else
    {
        BasicBlock* targetBlock = nullptr;
        auto it = basicBlocks.find(instruction.Index());
        if (it != basicBlocks.cend())
        {
            targetBlock = it->second;
        }
        else
        {
            targetBlock = BasicBlock::Create(context, "target", fun);
            basicBlocks[instruction.Index()] = targetBlock;
        }
        builder.CreateBr(targetBlock);
    }
    currentBasicBlock = nullptr;
}

void JitCompilerImpl::VisitJumpTrueInst(JumpTrueInst& instruction)
{
    BasicBlock* trueTargetBlock = nullptr;
    auto it = basicBlocks.find(instruction.Index());
    if (it != basicBlocks.cend())
    {
        trueTargetBlock = it->second;
    }
    else
    {
        trueTargetBlock = BasicBlock::Create(context, "trueTarget", fun);
        basicBlocks[instruction.Index()] = trueTargetBlock;
    }
    BasicBlock* continueBlock = BasicBlock::Create(context, "continueTarget", fun);
    Value* cond = valueStack.Pop();
    builder.CreateCondBr(cond, trueTargetBlock, continueBlock);
    currentBasicBlock = continueBlock;
    builder.SetInsertPoint(currentBasicBlock);
}

void JitCompilerImpl::VisitJumpFalseInst(JumpFalseInst& instruction)
{
    BasicBlock* falseTargetBlock = nullptr;
    auto it = basicBlocks.find(instruction.Index());
    if (it != basicBlocks.cend())
    {
        falseTargetBlock = it->second;
    }
    else
    {
        falseTargetBlock = BasicBlock::Create(context, "falseTarget", fun);
        basicBlocks[instruction.Index()] = falseTargetBlock;
    }
    BasicBlock* continueBlock = BasicBlock::Create(context, "continueTarget", fun);
    Value* cond = valueStack.Pop();
    builder.CreateCondBr(cond, continueBlock, falseTargetBlock);
    currentBasicBlock = continueBlock;
    builder.SetInsertPoint(currentBasicBlock);
}

llvm::Type* JitCompilerImpl::GetType(ValueType type)
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
        case ValueType::none:
        {
            return llvm::Type::getVoidTy(context);
        }
        default:
        {
            throw std::runtime_error("jit: invalid value type for GetType()");
        }
    }
}

void JitCompilerImpl::PushDefaultValue(ValueType type)
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
            valueStack.Push(builder.getInt64(0));
            break;
        }
        default:
        {
            throw std::runtime_error("jit: invalid value type for 'def' instruction");
        }
    }
}

void JitCompilerImpl::PushIntegerMinusOne(ValueType type)
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
            throw std::runtime_error("jit: invalid value type for integer minus one");
        }
    }
}

void JitCompilerImpl::InitMaps()
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

JitCompiler::JitCompiler(Function& main) : impl(new JitCompilerImpl())
{
    main.Accept(*impl);
}

JitCompiler::~JitCompiler()
{
    delete impl;
}

int JitCompiler::ProgramReturnValue() const
{
    return impl->ProgramReturnValue();
}

} } // namespace cminor::jit
