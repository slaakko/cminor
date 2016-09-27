// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_ERROR_INCLUDED
#define CMINOR_MACHINE_ERROR_INCLUDED
#include <stdexcept>

namespace cminor { namespace machine {

#ifdef NDEBUG

#define assert(expression, message) ((void)0)

#else

#define assert(expression, message) if (!(expression)) throw std::runtime_error((message))

#endif

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_ERROR_INCLUDED
