// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_TYPE_BINDER_VISITOR_INCLUDED
#define CMINOR_BINDER_TYPE_BINDER_VISITOR_INCLUDED
#include <cminor/ast/Visitor.hpp>
#include <cminor/symbols/EnumSymbol.hpp>

namespace cminor { namespace binder {

using namespace cminor::ast;
using namespace cminor::symbols;

class BoundCompileUnit;

class TypeBinderVisitor : public Visitor
{
public:
    TypeBinderVisitor(BoundCompileUnit& boundCompileUnit_);
    void SetInstantiateRequested() { instantiateRequested = true; }
    void SetContainerScope(ContainerScope* containerScope_) { containerScope = containerScope_; }
    void Visit(CompileUnitNode& compileUnitNode) override;
    void Visit(NamespaceNode& namespaceNode) override;
    void Visit(NamespaceImportNode& namespaceImportNode) override;
    void Visit(AliasNode& aliasNode) override;
    void Visit(ClassNode& classNode) override;
    void Visit(InterfaceNode& interfaceNode) override;
    void Visit(StaticConstructorNode& staticConstructorNode) override;
    void Visit(ConstructorNode& constructorNode) override;
    void Visit(MemberFunctionNode& memberFunctionNode) override;
    void Visit(MemberVariableNode& memberVariableNode) override;
    void Visit(PropertyNode& propertyNode) override;
    void Visit(IndexerNode& indexerNode) override;
    void Visit(FunctionNode& functionNode) override;
    void Visit(ConstantNode& constantNode) override;
    void Visit(EnumTypeNode& enumTypeNode) override;
    void Visit(EnumConstantNode& enumConstantNode) override;
    void Visit(CompoundStatementNode& compoundStatementNode) override;
    void Visit(IfStatementNode& ifStatementNode) override;
    void Visit(WhileStatementNode& whileStatementNode) override;
    void Visit(DoStatementNode& doStatementNode) override;
    void Visit(ForStatementNode& forStatementNode) override;
    void Visit(SwitchStatementNode& switchStatementNode) override;
    void Visit(CaseStatementNode& caseStatementNode) override;
    void Visit(DefaultStatementNode& defaultStatementNode) override;
    void Visit(ConstructionStatementNode& constructionStatementNode) override;
private:
    BoundCompileUnit& boundCompileUnit;
    ContainerScope* containerScope;
    std::vector<Node*> usingNodes;
    bool instantiateRequested;
    EnumTypeSymbol* enumType;
    void BindClass(ClassTypeSymbol* classTypeSymbol, ClassNode& classNode);
    void BindInterface(InterfaceTypeSymbol* interfaceTypeSymbol, InterfaceNode& interfaceNode);
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_TYPE_BINDER_VISITOR_INCLUDED
