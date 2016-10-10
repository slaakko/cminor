// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_BOUND_COMPILE_UNIT_INCLUDED
#define CMINOR_BINDER_BOUND_COMPILE_UNIT_INCLUDED
#include <cminor/symbols/Assembly.hpp>
#include <cminor/binder/BoundNode.hpp>

namespace cminor { namespace binder {

using namespace cminor::symbols;

class BoundCompileUnit : public BoundNode
{
public:
    BoundCompileUnit(Assembly& assembly_, CompileUnitNode* compileUnitNode_);
    Assembly& GetAssembly() const { return assembly; }
    CompileUnitNode* GetCompileUnitNode() const { return compileUnitNode; }
    void AddFileScope(std::unique_ptr<FileScope>&& fileScope);
    FileScope* FirstFileScope() const { Assert(!fileScopes.empty(), "file scopes empty");  return fileScopes.front().get(); }
    const std::vector<std::unique_ptr<FileScope>>& FileScopes() const { return fileScopes; }
    const std::vector<std::unique_ptr<BoundNode>>& BoundNodes() const { return boundNodes; }
    void AddBoundNode(std::unique_ptr<BoundNode>&& boundNode);
    void Accept(BoundNodeVisitor& visitor) override;
private:
    Assembly& assembly;
    CompileUnitNode* compileUnitNode;
    std::vector<std::unique_ptr<FileScope>> fileScopes;
    std::vector<std::unique_ptr<BoundNode>> boundNodes;
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_BOUND_COMPILE_UNIT_INCLUDED
