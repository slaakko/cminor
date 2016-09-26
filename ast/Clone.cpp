// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Clone.hpp>

namespace cminor { namespace ast {

CloneContext::CloneContext() : instantiateClassNode(false), instantiateFunctionNode(false)
{
}

} } // namespace cminor::ast
