// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_TYPE_RESOLVER_INCLUDED
#define CMINOR_BINDER_TYPE_RESOLVER_INCLUDED
#include <cminor/symbols/Symbol.hpp>
#include <cminor/ast/Node.hpp>

namespace cminor { namespace binder {

using namespace cminor::symbols;
using namespace cminor::ast;

class BoundCompileUnit;

TypeSymbol* ResolveType(BoundCompileUnit& boundCompileUnit, ContainerScope* containerScope, Node* typeExprNode);

} } // namespace cminor::binder

#endif // CMINOR_BINDER_TYPE_RESOLVER_INCLUDED
