// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_JIT_JIT_COMPILER_INCLUDED
#define CMINOR_JIT_JIT_COMPILER_INCLUDED

namespace cminor { namespace machine {

class Function;

} } // namespace cminor::machine;

namespace cminor { namespace jit {

using namespace cminor::machine;

class JitCompilerImpl;

class JitCompiler
{
public:
    JitCompiler(Function& main);
    ~JitCompiler();
    int ProgramReturnValue() const;
private:
    JitCompilerImpl* impl;
};

} } // namespace cminor::jit

#endif // CMINOR_JIT_JIT_COMPILER_INCLUDED
