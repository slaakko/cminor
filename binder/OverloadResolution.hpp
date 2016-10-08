// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_OVERLOAD_RESOLUTION_INCLUDED
#define CMINOR_BINDER_OVERLOAD_RESOLUTION_INCLUDED
#include <cminor/binder/BoundExpression.hpp>

namespace cminor { namespace binder {

class BoundCompileUnit;

struct FunctionScopeLookup
{
    FunctionScopeLookup(ScopeLookup scopeLookup_) : scopeLookup(scopeLookup_), scope(nullptr) {}
    FunctionScopeLookup(ScopeLookup scopeLookup_, ContainerScope* scope_) : scopeLookup(scopeLookup_), scope(scope_) {}
    ScopeLookup scopeLookup;
    ContainerScope* scope;
};

std::unique_ptr<BoundFunctionCall> ResolveOverload(BoundCompileUnit& boundCompileUnit, StringPtr groupName, const std::vector<FunctionScopeLookup>& functionScopeLookups, 
    std::vector<std::unique_ptr<BoundExpression>>& arguments, const Span& span);

} } // namespace cminor::binder

#endif // CMINOR_BINDER_OVERLOAD_RESOLUTION_INCLUDED
