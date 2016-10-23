// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_EXPRESSION_BINDER_INCLUDED
#define CMINOR_BINDER_EXPRESSION_BINDER_INCLUDED
#include <cminor/binder/BoundExpression.hpp>
#include <cminor/ast/Node.hpp>

namespace cminor { namespace binder {

using namespace cminor::ast;

class BoundCompileUnit;
class BoundFunction;

std::unique_ptr<BoundExpression> BindExpression(BoundCompileUnit& boundCompileUnit, BoundFunction* boundFunction, ContainerScope* containerScope, Node* node);

} } // namespace cminor::binder

#endif // CMINOR_BINDER_EXPRESSION_BINDER_INCLUDED
