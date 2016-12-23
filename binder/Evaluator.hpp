// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_EVALUATOR_INCLUDED
#define CMINOR_BINDER_EVALUATOR_INCLUDED
#include <cminor/binder/BoundCompileUnit.hpp>
#include <cminor/symbols/Value.hpp>

namespace cminor { namespace binder {

enum class EvaluationFlags : uint8_t
{
    none = 0, dontThrow = 1
};

inline EvaluationFlags operator&(EvaluationFlags left, EvaluationFlags right)
{
    return EvaluationFlags(uint8_t(left) & uint8_t(right));
}

Value* Evaluate(ValueKind targetKind, bool cast, ContainerScope* containerScope, BoundCompileUnit& boundCompileUnit, Node* value);

Value* Evaluate(ValueKind targetKind, bool cast, ContainerScope* containerScope, BoundCompileUnit& boundCompileUnit, Node* value, EvaluationFlags flags);

bool IsAlwaysTrue(ContainerScope* containerScope, BoundCompileUnit& boundCompileUnit, Node* value);

} } // namespace cminor::binder

#endif // CMINOR_BINDER_EVALUATOR_INCLUDED
