// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/BoundCompileUnit.hpp>

namespace cminor { namespace binder {

BoundCompileUnit::BoundCompileUnit(Assembly& assembly_, CompileUnitNode* compileUnitNode_) : assembly(assembly_), compileUnitNode(compileUnitNode_)
{
}

void BoundCompileUnit::AddFileScope(std::unique_ptr<FileScope>&& fileScope)
{
    fileScopes.push_back(std::move(fileScope));
}

void BoundCompileUnit::AddBoundNode(std::unique_ptr<BoundNode>&& boundNode)
{
    boundNodes.push_back(std::move(boundNode));
}

} } // namespace cminor::binder
