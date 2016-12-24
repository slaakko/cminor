// =================================
// Copyright (c) 2017 Seppo Laakko
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

enum class TypeResolverFlags : uint8_t
{
    none = 0, parameterType = 1
};

inline TypeResolverFlags operator&(TypeResolverFlags left, TypeResolverFlags right)
{
    return TypeResolverFlags(uint8_t(left) & uint8_t(right));
}

TypeSymbol* ResolveType(BoundCompileUnit& boundCompileUnit, ContainerScope* containerScope, Node* typeExprNode);

TypeSymbol* ResolveType(BoundCompileUnit& boundCompileUnit, ContainerScope* containerScope, Node* typeExprNode, TypeResolverFlags flags);

} } // namespace cminor::binder

#endif // CMINOR_BINDER_TYPE_RESOLVER_INCLUDED
