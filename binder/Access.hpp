// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_ACCESS_INCLUDED
#define CMINOR_BINDER_ACCESS_INCLUDED
#include <cminor/symbols/FunctionSymbol.hpp>

namespace cminor {  namespace binder {

using namespace cminor::symbols;

void CheckAccess(FunctionSymbol* fromFunction, Symbol* toSymbol);

} } // namespace cminor::binder

#endif // CMINOR_BINDER_ACCESS_INCLUDED
