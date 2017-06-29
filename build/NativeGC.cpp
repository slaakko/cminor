// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/build/NativeGC.hpp>
#include <iostream>
#ifdef _WIN32
#pragma warning(disable:4267)
#pragma warning(disable:4244)
#pragma warning(disable:4141)
#pragma warning(disable:4624)
#pragma warning(disable:4291)
#endif
#include <llvm/CodeGen/GCMetadata.h>
#include <llvm/CodeGen/TargetPassConfig.h>
#include <llvm/Target/TargetSubtargetInfo.h>
#include <llvm/Target/TargetRegisterInfo.h>
#include <llvm/Target/TargetFrameLowering.h>
#include <llvm/CodeGen/MachineFunctionPass.h>
#include <llvm/CodeGen/MachineFrameInfo.h>
#include <llvm/CodeGen/MachineModuleInfo.h>
#include <llvm/Pass.h>
#ifdef _WIN32
#pragma warning(default:4267)
#pragma warning(default:4244)
#pragma warning(default:4141)
#pragma warning(default:4624)
#pragma warning(default:4291)
#endif

namespace cminor { namespace build {

CminorGCStrategy::CminorGCStrategy()
{
    UseStatepoints = false;
    NeededSafePoints = 0;
    CustomReadBarriers = false;
    CustomWriteBarriers = false;
    CustomRoots = false;
    InitRoots = false;
    UsesMetadata = false;
}

static llvm::GCRegistry::Add<CminorGCStrategy> cminorGc("cminor-gc", "Cminor GC");

char CminorGCMachineFunctionPass::ID = 0;

static llvm::RegisterPass<CminorGCMachineFunctionPass> cminorGCMachineFunctionPass("cminor-gc-machine-function-pass", "Cminor GC machine function pass", false, false);

CminorGCMachineFunctionPass::CminorGCMachineFunctionPass() : llvm::MachineFunctionPass(ID), llvmFunctionMap(nullptr), listStream(nullptr), os(nullptr), functionInfo(nullptr)
{
}

CminorGCMachineFunctionPass::CminorGCMachineFunctionPass(std::unordered_map<const llvm::Function*, cminor::machine::Function*>* llvmFunctionMap_, std::ofstream* listStream_, 
    llvm::raw_os_ostream* os_) :
    llvm::MachineFunctionPass(ID), llvmFunctionMap(llvmFunctionMap_), listStream(listStream_), os(os_), functionInfo(nullptr)
{
}

void CminorGCMachineFunctionPass::FindStackOffsets(llvm::MachineFunction& mf, cminor::machine::Function* fun)
{
    const llvm::TargetFrameLowering* tfi = mf.getSubtarget().getFrameLowering();
    Assert(tfi, "TargetRegisterInfo not available!");
    bool first = true;
    for (llvm::GCFunctionInfo::roots_iterator rootIterator = functionInfo->roots_begin(); rootIterator != functionInfo->roots_end(); ++rootIterator)
    {
        unsigned frameReg = 0;
        int32_t offset = tfi->getFrameIndexReference(mf, rootIterator->Num, frameReg);
        if (first)
        {
            fun->SetLineNumberVarOffset(offset);
            first = false;
        }
        else
        {
            fun->AddGCRootStackOffset(offset);
        }
    }
}

bool CminorGCMachineFunctionPass::runOnMachineFunction(llvm::MachineFunction& mf)
{
    const llvm::Function* function = mf.getFunction();
    if (!function->hasGC()) return false;
    cminor::machine::Function* fun = nullptr;
    auto it = llvmFunctionMap->find(function);
    if (it != llvmFunctionMap->cend())
    {
        fun = it->second;
    }
    else
    {
        return false;
    }
    functionInfo = &getAnalysis<llvm::GCModuleInfo>().getFunctionInfo(*function);
    const llvm::MachineFrameInfo& frameInfo = mf.getFrameInfo();
    const llvm::TargetRegisterInfo* regInfo = mf.getSubtarget().getRegisterInfo();
    bool hasVariableSizedObjects = frameInfo.hasVarSizedObjects();
    bool needsRealignment = regInfo->needsStackRealignment(mf);
    bool hasDynamicFrameSize = hasVariableSizedObjects || needsRealignment;
    if (hasDynamicFrameSize)
    {
        if (listStream)
        {
            *listStream << "Machine function: " << fun->MangledName() << std::endl;
            if (hasVariableSizedObjects)
            {
                *listStream << "has variable sized objects" << std::endl;
            }
            if (needsRealignment)
            {
                *listStream << "needs realignment" << std::endl;
            }
            mf.print(*os);
        }
    }
    uint64_t frameSize = frameInfo.getStackSize();
    if (hasDynamicFrameSize)
    {
        std::cerr << "warning: function '" << fun->MangledName() << "' has dynamic frame size" << std::endl;
        frameSize = static_cast<uint64_t>(-1);
    }
    fun->SetFrameSize(frameSize);
    FindStackOffsets(mf, fun);
    return false;
}

void CminorGCMachineFunctionPass::getAnalysisUsage(llvm::AnalysisUsage& info) const
{
    MachineFunctionPass::getAnalysisUsage(info);
    info.setPreservesAll();
    info.addRequired<llvm::MachineModuleInfo>();
    info.addRequired<llvm::GCModuleInfo>();
}

void link_cminor_gc()
{
}

} } // namespace cminor::build
