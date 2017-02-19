// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_UTIL_PRIME_INCLUDED
#define CMINOR_UTIL_PRIME_INCLUDED
#include <stdint.h>

namespace cminor { namespace util {

// Returns smallest prime greater than or equal to x

uint64_t NextPrime(uint64_t x);

} } // namespace cminor::util

#endif // CMINOR_UTIL_PRIME_INCLUDED


