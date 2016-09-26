// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Util.hpp>

namespace cminor { namespace machine {

char HexNibble(uint8_t n)
{
    static const char* h = "0123456789ABCDEF";
    return h[n];
}

std::string ToHexString(uint8_t x)
{
    std::string s;
    s.append(1, HexNibble(x >> 4)).append(1, HexNibble(x & 0x0F));
    return s;
}

} } // namespace cminor::machine

