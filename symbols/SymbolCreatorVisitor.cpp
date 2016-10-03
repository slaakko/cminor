// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/SymbolCreatorVisitor.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/ast/CompileUnit.hpp>

namespace cminor { namespace symbols {

SymbolCreatorVisitor::SymbolCreatorVisitor(Assembly& assembly_) : assembly(assembly_), symbolTable(assembly.GetSymbolTable())
{
}

void SymbolCreatorVisitor::Visit(CompileUnitNode& compileUnitNode)
{
    compileUnitNode.GlobalNs()->Accept(*this);
}

void SymbolCreatorVisitor::Visit(NamespaceNode& namespaceNode)
{
    utf32_string nsName = ToUtf32(namespaceNode.Id()->Str());
    StringPtr namespaceName(nsName.c_str());
    symbolTable.BeginNamespaceScope(namespaceName, namespaceNode.GetSpan());
    NodeList<Node>& members = namespaceNode.Members();
    int n = members.Count();
    for (int i = 0; i < n; ++i)
    {
        Node* member = members[i];
        member->Accept(*this);
    }
    symbolTable.EndNamespaceScope();
}

void SymbolCreatorVisitor::Visit(FunctionNode& functionNode)
{

}

} } // namespace cminor::symbols
