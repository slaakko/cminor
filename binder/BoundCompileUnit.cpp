// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/BoundCompileUnit.hpp>

namespace cminor { namespace binder {

BoundCompileUnit::BoundCompileUnit(Assembly& assembly_) : assembly(assembly_)
{
}

void BoundCompileUnit::AddFileScope(std::unique_ptr<FileScope>&& fileScope)
{
    fileScopes.push_back(std::move(fileScope));
}

} } // namespace cminor::binder
