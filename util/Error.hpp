// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_UTIL_ERROR_INCLUDED
#define CMINOR_UTIL_ERROR_INCLUDED
#include <stdexcept>

namespace cminor { namespace util {

#ifdef NDEBUG

#define Assert(expression, message) ((void)0)

#else

#define Assert(expression, message) if (!(expression)) throw std::runtime_error(std::string("assertion failed: ") + message)

#endif

} } // namespace cminor::util

#endif // CMINOR_UTIL_ERROR_INCLUDED
