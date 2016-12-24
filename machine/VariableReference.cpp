// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/VariableReference.hpp>

namespace cminor { namespace machine {

VariableReference::VariableReference(int32_t id_, int32_t frameId_, int32_t localIndex_) : id(id_), frameId(frameId_), localIndex(localIndex_)
{
}

} } // namespace cminor::machine