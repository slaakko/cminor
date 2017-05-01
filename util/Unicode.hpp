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

namespace unicode {

enum class Category : uint32_t
{
    none = 0u,
    letterUpper = 1u << 0u,                 // Lu
    letterLower = 1u << 1u,                 // Ll
    letterCased = 1u << 2u,                 // LC 
    letterModifier = 1u << 3u,              // Lm
    letterOther = 1u << 4u,                 // Lo
    letterTitle = 1u << 5u,                 // Lt
    letter = letterUpper | letterLower | letterCased | letterModifier | letterOther | letterTitle,
    markSpacing = 1u << 6u,                 // Mc
    markEnclosing = 1u << 7u,               // Me
    markNonspacing = 1u << 8u,              // Mn
    mark = markSpacing | markEnclosing | markNonspacing,
    numberDecimal = 1u << 9u,               // Nd
    numberLetter = 1u << 10u,               // Nl
    numberOther = 1u << 11u,                // No
    number = numberDecimal | numberLetter | numberOther,
    punctuationConnector = 1u << 12u,       // Pc
    punctuationDash = 1u << 13u,            // Pd
    punctuationClose = 1u << 14u,           // Pe
    punctuationFinalQuote = 1u << 15u,      // Pf
    punctuationInitialQuote = 1u << 16u,    // Pi
    punctuationOther = 1u << 17u,           // Po
    punctuationOpen = 1u << 18u,            // Ps
    punctuation = punctuationConnector | punctuationDash | punctuationClose | punctuationFinalQuote | punctuationInitialQuote | punctuationOther | punctuationOpen,
    symbolCurrency = 1u << 19u,             // Sc
    symbolModifier = 1u << 20u,             // Sk
    symbolMath = 1u << 21u,                 // Sm
    symbolOther = 1u << 22u,                // So
    symbol = symbolCurrency | symbolModifier | symbolMath | symbolOther,
    separatorLine = 1u << 23u,              // Zl
    separatorParagraph = 1u << 24u,         // Zp
    separatorSpace = 1u << 25u,             // Zs
    separator = separatorLine | separatorParagraph | separatorSpace
};

struct CategoryHash
{
    size_t operator()(Category cat) const { return size_t(cat); }
};

inline Category operator&(Category left, Category right)
{
    return Category(uint32_t(left) & uint32_t(right));
}

inline Category operator|(Category left, Category right)
{
    return Category(uint32_t(left) | uint32_t(right));
}

Category GetCategory(char32_t c);
utf32_string GetCategoryName(Category category);
utf32_string GetCharacterName(char32_t c);
char32_t ToLower(char32_t c);
char32_t ToUpper(char32_t c);
bool IsLetter(char32_t c);
bool IsLower(char32_t c);
bool IsUpper(char32_t c);
bool IsMark(char32_t c);
bool IsNumber(char32_t c);
bool IsPunctuation(char32_t c);
bool IsSymbol(char32_t c);
bool IsSeparator(char32_t c);
utf32_string ToLower(const utf32_string& s);
utf32_string ToUpper(const utf32_string& s);
bool IsIdentifier(const utf32_string& s);

void Init();
void Done();

} } } // namespace cminor::util::unicode

#endif // CMINOR_UTIL_UNICODE_INCLUDED
