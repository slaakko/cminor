// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_EVALUATOR_INCLUDED
#define CMINOR_BINDER_EVALUATOR_INCLUDED
#include <cminor/binder/BoundCompileUnit.hpp>
#include <cminor/symbols/Value.hpp>

namespace cminor { namespace binder {

Value* Evaluate(ValueKind targetKind, bool cast, ContainerScope* containerScope, BoundCompileUnit& boundCompileUnit, Node* value);

} } // namespace cminor::binder

#endif // CMINOR_BINDER_EVALUATOR_INCLUDED
