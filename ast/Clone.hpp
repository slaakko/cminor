// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_CLONE_INCLUDED
#define CMINOR_AST_CLONE_INCLUDED

namespace cminor { namespace ast {

class CloneContext
{
public:
    CloneContext();
    void SetInstantiateClassNode() { instantiateClassNode = true; }
    bool InstantiateClassNode() const { return instantiateClassNode; }
    void SetInstantiateFunctionNode() { instantiateFunctionNode = true; }
    bool InstantiateFunctionNode() const { return instantiateFunctionNode; }
private:
    bool instantiateClassNode;
    bool instantiateFunctionNode;
};

} } // namespace cminor::ast

#endif // CMINOR_AST_CLONE_INCLUDED
