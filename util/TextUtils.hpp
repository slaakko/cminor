// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_UTIL_TEXTUTILS_INCLUDED
#define CMINOR_UTIL_TEXTUTILS_INCLUDED
#include <string>
#include <vector>
#include <stdint.h>

namespace cminor { namespace util {

std::string Trim(const std::string& s);
std::string TrimAll(const std::string& s);
std::vector<std::string> Split(const std::string& s, char c);
std::string Replace(const std::string& s, char oldChar, char newChar);
std::string Replace(const std::string& s, const std::string& oldString, const std::string& newString);
std::string HexEscape(char c);
std::string HexEscape(uint16_t c);
std::string HexEscape(uint32_t c);
std::string CharStr(char c);
std::string WCharStr(uint16_t c);
std::string UCharStr(uint32_t c);
std::string StringStr(const std::string& s);
std::string QuotedPath(const std::string& path);
std::string ReadFile(const std::string& fileName);
bool LastComponentsEqual(const std::string& s0, const std::string& s1, char componentSeparator);
bool StartsWith(const std::string& s, const std::string& prefix);
bool EndsWith(const std::string& s, const std::string& suffix);
std::string NarrowString(const char* str, int length);
std::string ToUpper(const std::string& s);
std::string ToLower(const std::string& s);
std::string ToString(double x);
std::string ToString(double x, int maxNumDecimals);
std::string ToString(double x, int minNumDecimals, int maxNumDecimals);
std::string ToHexString(uint8_t x);
std::string ToHexString(uint16_t x);
std::string ToHexString(uint32_t x);
std::string ToHexString(uint64_t x);

} } // namespace cminor::util

#endif //
