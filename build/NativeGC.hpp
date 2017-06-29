// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BUILD_NATIVE_GC_INCLUDED
#define CMINOR_BUILD_NATIVE_GC_INCLUDED
#include <cminor/machine/Function.hpp>
#include <fstream>
#ifdef _WIN32
#pragma warning(disable:4267)
#pragma warning(disable:4244)
#pragma warning(disable:4141)
#pragma warning(disable:4624)
#pragma warning(disable:4291)
#endif
#include <llvm/CodeGen/GCMetadata.h>
#include <llvm/CodeGen/MachineFunctionPass.h>
#include <llvm/CodeGen/MachineFrameInfo.h>
#include <llvm/Support/raw_os_ostream.h>
#ifdef _WIN32
#pragma warning(default:4267)
#pragma warning(default:4244)
#pragma warning(default:4141)
#pragma warning(default:4624)
#pragma warning(default:4291)
#endif

namespace cminor { namespace build {
    
struct CminorGCStrategy : public llvm::GCStrategy
{
    CminorGCStrategy();
};

class CminorGCMachineFunctionPass : public llvm::MachineFunctionPass
{
public:
    CminorGCMachineFunctionPass();
    CminorGCMachineFunctionPass(std::unordered_map<const llvm::Function*, cminor::machine::Function*>* llvmFunctionMap_, std::ofstream* listStream_, llvm::raw_os_ostream* os_);
    bool runOnMachineFunction(llvm::MachineFunction& mf) override;
    void getAnalysisUsage(llvm::AnalysisUsage& info) const override;
    static char ID;
private:
    std::unordered_map<const llvm::Function*, cminor::machine::Function*>* llvmFunctionMap;
    std::ofstream* listStream;
    llvm::raw_os_ostream* os;
    void FindStackOffsets(llvm::MachineFunction& mf, cminor::machine::Function* fun);
    llvm::GCFunctionInfo* functionInfo;
};

void link_cminor_gc();

} } // namespace cminor::build

#endif // CMINOR_BUILD_NATIVE_GC_INCLUDED
