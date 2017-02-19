// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BUILD_NATIVE_COMPILER_INCLUDED
#define CMINOR_BUILD_NATIVE_COMPILER_INCLUDED
#include <cminor/symbols/Assembly.hpp>

namespace cminor { namespace build {

using namespace cminor::symbols;

class NativeCompilerImpl;

class NativeCompiler
{
public:
    NativeCompiler();
    ~NativeCompiler();
    void Compile(const std::string& assemblyFilePath, std::string& nativeImportLibraryFilePath, std::string& nativeSharedLibraryFilePath);
private:
    NativeCompilerImpl* impl;
};

} } // namespace cminor::build

#endif // CMINOR_BUILD_NATIVE_COMPILER_INCLUDED
