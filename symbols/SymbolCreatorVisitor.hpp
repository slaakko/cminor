// =================================
// Copyright (c) 2016 Seppo Laakko
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
    void Visit(CompileUnitNode& compileUnitNode) override;
    void Visit(NamespaceNode& namespaceNode) override;
    void Visit(FunctionNode& functionNode) override;
    void Visit(ParameterNode& parameterNode) override;
    void Visit(CompoundStatementNode& compoundStatementNode) override;
    void Visit(ConstructionStatementNode& constructionStatementNode) override;
private:
    Assembly& assembly;
    SymbolTable& symbolTable;
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_SYMBOL_CREATOR_VISITOR_INCLUDED
