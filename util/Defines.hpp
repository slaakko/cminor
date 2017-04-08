// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_UTIL_DEFINES_INCLUDED
#define CMINOR_UTIL_DEFINES_INCLUDED

#ifdef _WIN32
    #if defined(_WIN64)
        #define BITS_64 1
    #else 
        #define BITS_32 1
    #endif
#else
    #if defined(__x86_64__)
        #define BITS_64 1
    #else 
        #define BITS_32 1
    #endif
#endif 

#endif // CMINOR_UTIL_DEFINES_INCLUDED
