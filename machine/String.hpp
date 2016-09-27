// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_STRING_INCLUDED
#define CMINOR_MACHINE_STRING_INCLUDED
#include <cminor/machine/Error.hpp>

namespace cminor { namespace machine {

class StringPtr
{
public:
    StringPtr(char32_t* value_) : value(value_) {}
    char32_t* Value() const { return value; }
private:
    char32_t* value;
};

inline bool operator==(StringPtr left, StringPtr right)
{
    char32_t* p = left.Value();
    char32_t* q = right.Value();
    if (!p && !q) return true;
    if (!p || !q) return false;
    while (*p && *q && *p == *q)
    {
        ++p;
        ++q;
    }
    return !*p && !*q;
}

inline bool operator<(StringPtr left, StringPtr right)
{
    char32_t* p = left.Value();
    char32_t* q = right.Value();
    if (!p && q) return true;
    if (!q) return false;
    while (*p && *q && *p == *q)
    {
        ++p;
        ++q;
    }
    return *p < *q;
}

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_STRING_INCLUDED
