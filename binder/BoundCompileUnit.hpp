// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_BOUND_COMPILE_UNIT_INCLUDED
#define CMINOR_BINDER_BOUND_COMPILE_UNIT_INCLUDED
#include <cminor/symbols/Assembly.hpp>
#include <cminor/binder/BoundNode.hpp>
#include <cminor/binder/ClassTemplateRepository.hpp>

namespace cminor { namespace binder {

using namespace cminor::symbols;

class BoundCompileUnit : public BoundNode
{
public:
    BoundCompileUnit(Assembly& assembly_, CompileUnitNode* compileUnitNode_);
    Assembly& GetAssembly() const { return assembly; }
    CompileUnitNode* GetCompileUnitNode() const { return compileUnitNode; }
    void AddFileScope(std::unique_ptr<FileScope>&& fileScope);
    void RemoveLastFileScope();
    FileScope* FirstFileScope() const { Assert(!fileScopes.empty(), "file scopes empty");  return fileScopes.front().get(); }
    const std::vector<std::unique_ptr<FileScope>>& FileScopes() const { return fileScopes; }
    const std::vector<std::unique_ptr<BoundNode>>& BoundNodes() const { return boundNodes; }
    void AddBoundNode(std::unique_ptr<BoundNode>&& boundNode);
    void AddNode(std::unique_ptr<Node>&& node_);
    void Accept(BoundNodeVisitor& visitor) override;
    FunctionSymbol* GetConversion(TypeSymbol* sourceType, TypeSymbol* targetType);
    ClassTemplateRepository& GetClassTemplateRepository() { return classTemplateRepository; }
    void SetHasGotos() { hasGotos = true; }
    bool HasGotos() const { return hasGotos; }
private:
    Assembly& assembly;
    CompileUnitNode* compileUnitNode;
    std::vector<std::unique_ptr<FileScope>> fileScopes;
    std::vector<std::unique_ptr<BoundNode>> boundNodes;
    ConversionTable conversionTable;
    ClassTemplateRepository classTemplateRepository;
    std::vector<std::unique_ptr<Node>> nodes;
    bool hasGotos;
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_BOUND_COMPILE_UNIT_INCLUDED
