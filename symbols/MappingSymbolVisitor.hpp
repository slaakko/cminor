// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_MAPPING_SYMBOL_VISITOR_INCLUDED
#define CMINOR_SYMBOLS_MAPPING_SYMBOL_VISITOR_INCLUDED
#include <cminor/ast/Visitor.hpp>
#include <cminor/symbols/Symbol.hpp>

namespace cminor { namespace symbols {

class MappingSymbolVisitor : public Visitor
{
public:
    MappingSymbolVisitor(Assembly& targetAssembly_, Assembly& sourceAssembly_);
    void SetImplementedInterfaces(std::vector<InterfaceTypeSymbol*>* implementedInterfaces_);
    void Visit(NamespaceNode& namespaceNode) override;
    void Visit(ClassNode& classNode) override;
    void Visit(FunctionNode& functionNode) override;
    void Visit(ParameterNode& parameterNode) override;
    void Visit(StaticConstructorNode& staticConstructorNode) override;
    void Visit(ConstructorNode& constructorNode) override;
    void Visit(MemberFunctionNode& memberFunctionNode) override;
    void Visit(MemberVariableNode& memberVariableNode) override;
    void Visit(PropertyNode& propertyNode) override;
    void Visit(IndexerNode& indexerNode) override;
    void Visit(CompoundStatementNode& compoundStatementNode) override;
    void Visit(IfStatementNode& ifStatementNode) override;
    void Visit(WhileStatementNode& whileStatementNode) override;
    void Visit(DoStatementNode& doStatementNode) override;
    void Visit(ForStatementNode& forStatementNode) override;
    void Visit(ConstructionStatementNode& constructionStatementNode) override;
private:
    Assembly& targetAssembly;
    Assembly& sourceAssembly;
    std::vector<InterfaceTypeSymbol*>* implementedInterfaces;
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_MAPPING_SYMBOL_VISITOR_INCLUDED
