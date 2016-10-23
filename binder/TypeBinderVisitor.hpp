// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_TYPE_BINDER_VISITOR_INCLUDED
#define CMINOR_BINDER_TYPE_BINDER_VISITOR_INCLUDED
#include <cminor/ast/Visitor.hpp>
#include <cminor/symbols/Symbol.hpp>

namespace cminor { namespace binder {

using namespace cminor::ast;
using namespace cminor::symbols;

class BoundCompileUnit;

class TypeBinderVisitor : public Visitor
{
public:
    TypeBinderVisitor(BoundCompileUnit& boundCompileUnit_);
    void Visit(CompileUnitNode& compileUnitNode) override;
    void Visit(NamespaceNode& namespaceNode) override;
    void Visit(NamespaceImportNode& namespaceImportNode) override;
    void Visit(AliasNode& aliasNode) override;
    void Visit(ClassNode& classNode) override;
    void Visit(StaticConstructorNode& staticConstructorNode) override;
    void Visit(ConstructorNode& constructorNode) override;
    void Visit(MemberFunctionNode& memberFunctionNode) override;
    void Visit(MemberVariableNode& memberVariableNode) override;
    void Visit(FunctionNode& functionNode) override;
    void Visit(CompoundStatementNode& compoundStatementNode) override;
    void Visit(ForStatementNode& forStatementNode) override;
    void Visit(ConstructionStatementNode& constructionStatementNode) override;
private:
    BoundCompileUnit& boundCompileUnit;
    ContainerScope* containerScope;
    void BindClass(ClassTypeSymbol* classTypeSymbol, ClassNode& classNode);
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_TYPE_BINDER_VISITOR_INCLUDED
