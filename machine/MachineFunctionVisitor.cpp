// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/MachineFunctionVisitor.hpp>

namespace cminor { namespace machine {

MachineFunctionVisitor::MachineFunctionVisitor() : currentInstructionIndex(-1)
{
}

MachineFunctionVisitor::~MachineFunctionVisitor()
{
}

} } // namespace cminor::machine
