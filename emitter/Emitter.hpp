// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_EMITTER_EMITTER_INCLUDED
#define CMINOR_EMITTER_EMITTER_INCLUDED
#include <cminor/binder/BoundCompileUnit.hpp>

namespace cminor { namespace emitter {

using namespace cminor::binder;

void GenerateCode(BoundCompileUnit& boundCompileUnit, Machine& machine);

} } // namespace cminor::emitter

#endif // CMINOR_EMITTER_EMITTER_INCLUDED
