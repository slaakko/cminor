// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_UNICODE_INCLUDED
#define CMINOR_MACHINE_UNICODE_INCLUDED
#include <string>

namespace cminor { namespace machine {

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

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_UNICODE_INCLUDED
