// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_UTIL_UNICODE_INCLUDED
#define CMINOR_UTIL_UNICODE_INCLUDED
#include <string>
#include <vector>

namespace cminor { namespace util {

typedef char32_t utf32_char;
typedef char16_t utf16_char;
typedef std::basic_string<char32_t> utf32_string;
typedef std::basic_string<char16_t> utf16_string;

utf32_string ToUtf32(const std::string& utf8Str);
utf32_string ToUtf32(const utf16_string& utf16Str);
utf16_string ToUtf16(const utf32_string& utf32Str);
utf16_string ToUtf16(const std::string& utf8Str);
std::string ToUtf8(const utf32_string& utf32Str);
std::string ToUtf8(const utf16_string& utf16Str);
std::vector<uint8_t> EncodeUInt(uint32_t x);
uint32_t DecodeUInt(const std::vector<uint8_t>& bytes);

} } // namespace cminor::util

#endif // CMINOR_UTIL_UNICODE_INCLUDED
