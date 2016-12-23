// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_RANDOM_INCLUDED
#define CMINOR_MACHINE_RANDOM_INCLUDED
#include <stdint.h>

namespace cminor { namespace machine {

void InitMt(uint32_t seed);

uint32_t Random();
uint64_t Random64();

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_RANDOM_INCLUDED
