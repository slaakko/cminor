// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_PRIME_INCLUDED
#define CMINOR_MACHINE_PRIME_INCLUDED
#include <stdint.h>

namespace cminor { namespace machine {

// Returns smallest prime greater than or equal to x

uint64_t NextPrime(uint64_t x);

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_PRIME_INCLUDED


