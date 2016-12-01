// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/BoundCompileUnit.hpp>
#include <cminor/binder/BoundNodeVisitor.hpp>

namespace cminor { namespace binder {

BoundCompileUnit::BoundCompileUnit(Assembly& assembly_, CompileUnitNode* compileUnitNode_) : 
    BoundNode(assembly_), assembly(assembly_), compileUnitNode(compileUnitNode_), conversionTable(assembly_), classTemplateRepository(*this), hasGotos(false)
{
    conversionTable.SetConversionMap(assembly.GetSymbolTable().GetConversionTable().ConversionMap());
}

void BoundCompileUnit::AddFileScope(std::unique_ptr<FileScope>&& fileScope)
{
    fileScopes.push_back(std::move(fileScope));
}

void BoundCompileUnit::RemoveLastFileScope()
{
    fileScopes.pop_back();
}

void BoundCompileUnit::AddBoundNode(std::unique_ptr<BoundNode>&& boundNode)
{
    boundNodes.push_back(std::move(boundNode));
}

void BoundCompileUnit::AddNode(std::unique_ptr<Node>&& node_)
{
    nodes.push_back(std::move(node_));
}

void BoundCompileUnit::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

FunctionSymbol* BoundCompileUnit::GetConversion(TypeSymbol* sourceType, TypeSymbol* targetType) 
{
    return conversionTable.GetConversion(sourceType, targetType);
}

} } // namespace cminor::binder
