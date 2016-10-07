// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_BOUND_COMPILE_UNIT_INCLUDED
#define CMINOR_BINDER_BOUND_COMPILE_UNIT_INCLUDED
#include <cminor/symbols/Assembly.hpp>

namespace cminor { namespace binder {

using namespace cminor::symbols;

class BoundCompileUnit
{
public:
    BoundCompileUnit(Assembly& assembly_);
    Assembly& GetAssembly() const { return assembly; }
    void AddFileScope(std::unique_ptr<FileScope>&& fileScope);
    FileScope* FirstFileScope() const { Assert(!fileScopes.empty(), "file scopes empty");  return fileScopes.front().get(); }
private:
    Assembly& assembly;
    std::vector<std::unique_ptr<FileScope>> fileScopes;
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_BOUND_COMPILE_UNIT_INCLUDED
