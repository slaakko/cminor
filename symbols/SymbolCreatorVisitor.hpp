// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_SYMBOL_CREATOR_VISITOR_INCLUDED
#define CMINOR_SYMBOLS_SYMBOL_CREATOR_VISITOR_INCLUDED
#include <cminor/ast/Visitor.hpp>
#include <cminor/symbols/Symbol.hpp>

namespace cminor { namespace symbols {

using namespace cminor::machine;
using namespace cminor::ast;

class Assembly;

class SymbolCreatorVisitor : public Visitor
{
public:
    SymbolCreatorVisitor(Assembly& assembly_);
    void SetClassInstanceNode(ClassNode* classInstanceNode_) { classInstanceNode = classInstanceNode_; }
    void SetClassTemplateSpecialization(ClassTemplateSpecializationSymbol* classTemplateSpecialization_) { classTemplateSpecialization = classTemplateSpecialization_; }
    void Visit(CompileUnitNode& compileUnitNode) override;
    void Visit(NamespaceNode& namespaceNode) override;
    void Visit(FunctionNode& functionNode) override;
    void Visit(ParameterNode& parameterNode) override;
    void Visit(ClassNode& classNode) override;
    void Visit(InterfaceNode& interfaceNode) override;
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
    void Visit(SwitchStatementNode& switchStatementNode) override;
    void Visit(CaseStatementNode& caseStatementNode) override;
    void Visit(DefaultStatementNode& defaultStatementNode) override;
    void Visit(ConstructionStatementNode& constructionStatementNode) override;
    void Visit(TryStatementNode& tryStatementNode) override;
    void Visit(CatchNode& catchNode) override;
    void Visit(EnumTypeNode& enumTypeNode) override;
    void Visit(ConstantNode& constantNode) override;
private:
    Assembly& assembly;
    SymbolTable& symbolTable;
    ClassNode* classInstanceNode;
    ClassTemplateSpecializationSymbol* classTemplateSpecialization;
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_SYMBOL_CREATOR_VISITOR_INCLUDED
