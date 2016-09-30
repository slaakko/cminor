// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_STRING_INCLUDED
#define CMINOR_MACHINE_STRING_INCLUDED
#include <cminor/machine/Error.hpp>

namespace cminor { namespace machine {

inline uint64_t StringLen(const char32_t* s)
{
    uint64_t n = 0;
    assert(s, "null string ptr");
    while (*s++) ++n;
    return n;
}

class StringPtr
{
public:
    StringPtr(const char32_t* value_) : value(value_) {}
    const char32_t* Value() const { return value; }
private:
    const char32_t* value;
};

inline bool operator==(StringPtr left, StringPtr right)
{
    const char32_t* p = left.Value();
    const char32_t* q = right.Value();
    assert(p, "null string ptr");
    assert(q, "null string ptr");
    while (*p && *q && *p == *q)
    {
        ++p;
        ++q;
    }
    return !*p && !*q;
}

inline bool operator<(StringPtr left, StringPtr right)
{
    const char32_t* p = left.Value();
    const char32_t* q = right.Value();
    assert(p, "null string ptr");
    assert(q, "null string ptr");
    while (*p && *q && *p == *q)
    {
        ++p;
        ++q;
    }
    return *p < *q;
}

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_STRING_INCLUDED
