// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_PARSING_UTILITY_INCLUDED
#define CMINOR_PARSING_UTILITY_INCLUDED
#include <stdint.h>
#include <string>
#include <vector>

namespace cminor { namespace parsing {

std::string HexEscape(char c);
std::string XmlCharStr(char c);
std::string XmlEscape(const std::string& s);

} } // namespace cminor::parsing

#endif // CMINOR_PARSING_UTILITY_INCLUDED
