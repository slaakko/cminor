// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_UTIL_INCLUDED
#define CMINOR_MACHINE_UTIL_INCLUDED
#include <vector>
#include <stdint.h>
#include <string>

namespace cminor { namespace machine {

std::string ToHexString(uint8_t x);

template<typename StringT>
std::vector<StringT> Split(const StringT& s, typename StringT::value_type c)
{
    std::vector<StringT> v;
    int start = 0;
    int n = int(s.length());
    for (int i = 0; i < n; ++i)
    {
        if (s[i] == c)
        {
            v.push_back(s.substr(start, i - start));
            start = i + 1;
        }
    }
    if (start < n)
    {
        v.push_back(s.substr(start, n - start));
    }
    return v;
}

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_UTIL_INCLUDED
