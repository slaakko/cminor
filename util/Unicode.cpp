// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/util/Unicode.hpp>
#include <cminor/util/Path.hpp>
#include <stdexcept>
#include <unordered_map>
#include <memory>
#include <cstdio>
#include <cctype>

namespace cminor { namespace util {

utf32_string ToUtf32(const std::string& utf8Str)
{
    utf32_string result;
    const char* p = utf8Str.c_str();
    int bytesRemaining = int(utf8Str.length());
    while (bytesRemaining > 0)
    {
        char c = *p;
        uint8_t x = static_cast<uint8_t>(c);
        if ((x & 0x80u) == 0u)
        {
            result.append(1, static_cast<char32_t>(static_cast<uint32_t>(x)));
            --bytesRemaining;
            ++p;
        }
        else if ((x & 0xE0u) == 0xC0u)
        {
            if (bytesRemaining < 2)
            {
                throw std::runtime_error("invalid UTF-8 sequence");
            }
            char32_t u = static_cast<char32_t>(static_cast<uint32_t>(0u));
            uint8_t b1 = static_cast<uint8_t>(p[1]);
            if ((b1 & 0xC0u) != 0x80u)
            {
                throw std::runtime_error("invalid UTF-8 sequence");
            }
            uint8_t shift = 0u;
            for (uint8_t i = 0u; i < 6u; ++i)
            {
                uint8_t bit = b1 & 1u;
                b1 = b1 >> 1u;
                u = static_cast<char32_t>(static_cast<uint32_t>(u) | (static_cast<uint32_t>(bit) << shift));
                ++shift;
            }
            uint8_t b0 = x;
            for (uint8_t i = 0u; i < 5u; ++i)
            {
                uint8_t bit = b0 & 1u;
                b0 = b0 >> 1u;
                u = static_cast<char32_t>(static_cast<uint32_t>(u) | (static_cast<uint32_t>(bit) << shift));
                ++shift;
            }
            result.append(1, u);
            bytesRemaining = bytesRemaining - 2;
            p = p + 2;
        }
        else if ((x & 0xF0u) == 0xE0u)
        {
            if (bytesRemaining < 3)
            {
                throw std::runtime_error("invalid UTF-8 sequence");
            }
            char32_t u = static_cast<char32_t>(static_cast<uint32_t>(0u));
            uint8_t b2 = static_cast<uint8_t>(p[2]);
            if ((b2 & 0xC0u) != 0x80u)
            {
                throw std::runtime_error("invalid UTF-8 sequence");
            }
            uint8_t shift = 0u;
            for (uint8_t i = 0u; i < 6u; ++i)
            {
                uint8_t bit = b2 & 1u;
                b2 = b2 >> 1u;
                u = static_cast<char32_t>(static_cast<uint32_t>(u) | (static_cast<uint32_t>(bit) << shift));
                ++shift;
            }
            uint8_t b1 = static_cast<uint8_t>(p[1]);
            if ((b1 & 0xC0u) != 0x80u)
            {
                throw std::runtime_error("invalid UTF-8 sequence");
            }
            for (uint8_t i = 0u; i < 6u; ++i)
            {
                uint8_t bit = b1 & 1u;
                b1 = b1 >> 1u;
                u = static_cast<char32_t>(static_cast<uint32_t>(u) | (static_cast<uint32_t>(bit) << shift));
                ++shift;
            }
            uint8_t b0 = x;
            for (uint8_t i = 0u; i < 4u; ++i)
            {
                uint8_t bit = b0 & 1u;
                b0 = b0 >> 1u;
                u = static_cast<char32_t>(static_cast<uint32_t>(u) | (static_cast<uint32_t>(bit) << shift));
                ++shift;
            }
            result.append(1, u);
            bytesRemaining = bytesRemaining - 3;
            p = p + 3;
        }
        else if ((x & 0xF8u) == 0xF0u)
        {
            if (bytesRemaining < 4)
            {
                throw std::runtime_error("invalid UTF-8 sequence");
            }
            char32_t u = static_cast<char32_t>(static_cast<uint32_t>(0u));
            uint8_t b3 = static_cast<uint8_t>(p[3]);
            if ((b3 & 0xC0u) != 0x80u)
            {
                throw std::runtime_error("invalid UTF-8 sequence");
            }
            uint8_t shift = 0u;
            for (uint8_t i = 0u; i < 6u; ++i)
            {
                uint8_t bit = b3 & 1u;
                b3 = b3 >> 1u;
                u = static_cast<char32_t>(static_cast<uint32_t>(u) | (static_cast<uint32_t>(bit) << shift));
                ++shift;
            }
            uint8_t b2 = static_cast<uint8_t>(p[2]);
            if ((b2 & 0xC0u) != 0x80u)
            {
                throw std::runtime_error("invalid UTF-8 sequence");
            }
            for (uint8_t i = 0u; i < 6u; ++i)
            {
                uint8_t bit = b2 & 1u;
                b2 = b2 >> 1u;
                u = static_cast<char32_t>(static_cast<uint32_t>(u) | (static_cast<uint32_t>(bit) << shift));
                ++shift;
            }
            uint8_t b1 = static_cast<uint8_t>(p[1]);
            if ((b1 & 0xC0u) != 0x80u)
            {
                throw std::runtime_error("invalid UTF-8 sequence");
            }
            for (uint8_t i = 0u; i < 6u; ++i)
            {
                uint8_t bit = b1 & 1u;
                b1 = b1 >> 1u;
                u = static_cast<char32_t>(static_cast<uint32_t>(u) | (static_cast<uint32_t>(bit) << shift));
                ++shift;
            }
            uint8_t b0 = x;
            for (uint8_t i = 0u; i < 3u; ++i)
            {
                uint8_t bit = b0 & 1u;
                b0 = b0 >> 1u;
                u = static_cast<char32_t>(static_cast<uint32_t>(u) | (static_cast<uint32_t>(bit) << shift));
                ++shift;
            }
            result.append(1, u);
            bytesRemaining = bytesRemaining - 4;
            p = p + 4;
        }
        else
        {
            throw std::runtime_error("invalid UTF-8 sequence");
        }
    }
    return result;
}

utf32_string ToUtf32(const utf16_string& utf16Str)
{
    utf32_string result;
    const char16_t* w = utf16Str.c_str();
    int remaining = int(utf16Str.length());
    while (remaining > 0)
    {
        char16_t w1 = *w++;
        --remaining;
        if (static_cast<uint16_t>(w1) < 0xD800u || static_cast<uint16_t>(w1) > 0xDFFFu)
        {
            result.append(1, w1);
        }
        else
        {
            if (static_cast<uint16_t>(w1) < 0xD800u || static_cast<uint16_t>(w1) > 0xDBFFu)
            {
                throw std::runtime_error("invalid UTF-16 sequence");
            }
            if (remaining > 0)
            {
                char16_t w2 = *w++;
                --remaining;
                if (static_cast<uint16_t>(w2) < 0xDC00u || static_cast<uint16_t>(w2) > 0xDFFFu)
                {
                    throw std::runtime_error("invalid UTF-16 sequence");
                }
                else
                {
                    char32_t uprime = static_cast<char32_t>(((0x03FFu & static_cast<uint32_t>(w1)) << 10u) | (0x03FFu & static_cast<uint32_t>(w2)));
                    char32_t u = static_cast<char32_t>(static_cast<uint32_t>(uprime) + 0x10000u);
                    result.append(1, u);
                }
            }
            else
            {
                throw std::runtime_error("invalid UTF-16 sequence");
            }
        }
    }
    return result;
}

utf16_string ToUtf16(const utf32_string& utf32Str)
{
    utf16_string result;
    for (char32_t u : utf32Str)
    {
        if (static_cast<uint32_t>(u) > 0x10FFFFu)
        {
            throw std::runtime_error("invalid UTF-32 code point");
        }
        if (static_cast<uint32_t>(u) < 0x10000u)
        {
            if (static_cast<uint32_t>(u) >= 0xD800 && static_cast<uint32_t>(u) <= 0xDFFF)
            {
                throw std::runtime_error("invalid UTF-32 code point (reserved for UTF-16)");
            }
            char16_t x = static_cast<char16_t>(u);
            result.append(1, x);
        }
        else
        {
            char32_t uprime = static_cast<char32_t>(static_cast<uint32_t>(u) - 0x10000u);
            char16_t w1 = static_cast<char16_t>(0xD800u);
            char16_t w2 = static_cast<char16_t>(0xDC00u);
            for (uint16_t i = 0u; i < 10u; ++i)
            {
                uint16_t bit = static_cast<uint16_t>(static_cast<uint32_t>(uprime) & (static_cast<uint32_t>(0x1u) << i));
                w2 = static_cast<char16_t>(static_cast<uint16_t>(w2) | bit);
            }
            for (uint16_t i = 10u; i < 20u; ++i)
            {
                uint16_t bit = static_cast<uint16_t>((static_cast<uint32_t>(uprime) & (static_cast<uint32_t>(0x1u) << i)) >> 10u);
                w1 = static_cast<char16_t>(static_cast<uint16_t>(w1) | bit);
            }
            result.append(1, w1);
            result.append(1, w2);
        }
    }
    return result;
}

utf16_string ToUtf16(const std::string& utf8Str)
{
    return ToUtf16(ToUtf32(utf8Str));
}

std::string ToUtf8(const utf32_string& utf32Str)
{
    std::string result;
    for (char32_t c : utf32Str)
    {
        uint32_t x = static_cast<uint32_t>(c);
        if (x < 0x80u)
        {
            result.append(1, static_cast<char>(x & 0x7Fu));
        }
        else if (x < 0x800u)
        {
            uint8_t b1 = 0x80u;
            for (uint8_t i = 0u; i < 6u; ++i)
            {
                b1 = b1 | (static_cast<uint8_t>(x & 1u) << i);
                x = x >> 1u;
            }
            uint8_t b0 = 0xC0u;
            for (uint8_t i = 0u; i < 5u; ++i)
            {
                b0 = b0 | (static_cast<uint8_t>(x & 1u) << i);
                x = x >> 1u;
            }
            result.append(1, static_cast<char>(b0));
            result.append(1, static_cast<char>(b1));
        }
        else if (x < 0x10000u)
        {
            uint8_t b2 = 0x80u;
            for (uint8_t i = 0u; i < 6u; ++i)
            {
                b2 = b2 | (static_cast<uint8_t>(x & 1u) << i);
                x = x >> 1u;
            }
            uint8_t b1 = 0x80u;
            for (uint8_t i = 0u; i < 6u; ++i)
            {
                b1 = b1 | (static_cast<uint8_t>(x & 1u) << i);
                x = x >> 1u;
            }
            uint8_t b0 = 0xE0u;
            for (uint8_t i = 0u; i < 4u; ++i)
            {
                b0 = b0 | (static_cast<uint8_t>(x & 1u) << i);
                x = x >> 1u;
            }
            result.append(1, static_cast<char>(b0));
            result.append(1, static_cast<char>(b1));
            result.append(1, static_cast<char>(b2));
        }
        else if (x < 0x110000u)
        {
            uint8_t b3 = 0x80u;
            for (uint8_t i = 0u; i < 6u; ++i)
            {
                b3 = b3 | (static_cast<uint8_t>(x & 1u) << i);
                x = x >> 1u;
            }
            uint8_t b2 = 0x80u;
            for (uint8_t i = 0u; i < 6u; ++i)
            {
                b2 = b2 | (static_cast<uint8_t>(x & 1u) << i);
                x = x >> 1u;
            }
            uint8_t b1 = 0x80u;
            for (uint8_t i = 0u; i < 6u; ++i)
            {
                b1 = b1 | (static_cast<uint8_t>(x & 1u) << i);
                x = x >> 1u;
            }
            uint8_t b0 = 0xF0u;
            for (uint8_t i = 0u; i < 3u; ++i)
            {
                b0 = b0 | (static_cast<uint8_t>(x & 1u) << i);
                x = x >> 1u;
            }
            result.append(1, static_cast<char>(b0));
            result.append(1, static_cast<char>(b1));
            result.append(1, static_cast<char>(b2));
            result.append(1, static_cast<char>(b3));
        }
        else
        {
            throw std::runtime_error("invalid UTF-32 code point");
        }
    }
    return result;
}

std::string ToUtf8(const utf16_string& utf16Str)
{
    return ToUtf8(ToUtf32(utf16Str));
}

std::vector<uint8_t> EncodeUInt(uint32_t x)
{
    std::vector<uint8_t> bytes;
    if (x < 0x80u)
    {
        bytes.push_back(static_cast<uint8_t>(x & 0x7Fu));
    }
    else if (x < 0x800u)
    {
        uint8_t b1 = 0x80u;
        for (uint8_t i = 0u; i < 6u; ++i)
        {
            b1 = b1 | (static_cast<uint8_t>(x & 1u) << i);
            x = x >> 1u;
        }
        uint8_t b0 = 0xC0u;
        for (uint8_t i = 0u; i < 5u; ++i)
        {
            b0 = b0 | (static_cast<uint8_t>(x & 1u) << i);
            x = x >> 1u;
        }
        bytes.push_back(b0);
        bytes.push_back(b1);
    }
    else if (x < 0x10000u)
    {
        uint8_t b2 = 0x80u;
        for (uint8_t i = 0u; i < 6u; ++i)
        {
            b2 = b2 | (static_cast<uint8_t>(x & 1u) << i);
            x = x >> 1u;
        }
        uint8_t b1 = 0x80u;
        for (uint8_t i = 0u; i < 6u; ++i)
        {
            b1 = b1 | (static_cast<uint8_t>(x & 1u) << i);
            x = x >> 1u;
        }
        uint8_t b0 = 0xE0u;
        for (uint8_t i = 0u; i < 4u; ++i)
        {
            b0 = b0 | (static_cast<uint8_t>(x & 1u) << i);
            x = x >> 1u;
        }
        bytes.push_back(b0);
        bytes.push_back(b1);
        bytes.push_back(b2);
    }
    else if (x < 0x110000u)
    {
        uint8_t b3 = 0x80u;
        for (uint8_t i = 0u; i < 6u; ++i)
        {
            b3 = b3 | (static_cast<uint8_t>(x & 1u) << i);
            x = x >> 1u;
        }
        uint8_t b2 = 0x80u;
        for (uint8_t i = 0u; i < 6u; ++i)
        {
            b2 = b2 | (static_cast<uint8_t>(x & 1u) << i);
            x = x >> 1u;
        }
        uint8_t b1 = 0x80u;
        for (uint8_t i = 0u; i < 6u; ++i)
        {
            b1 = b1 | (static_cast<uint8_t>(x & 1u) << i);
            x = x >> 1u;
        }
        uint8_t b0 = 0xF0u;
        for (uint8_t i = 0u; i < 3u; ++i)
        {
            b0 = b0 | (static_cast<uint8_t>(x & 1u) << i);
            x = x >> 1u;
        }
        bytes.push_back(b0);
        bytes.push_back(b1);
        bytes.push_back(b2);
        bytes.push_back(b3);
    }
    else
    {
        throw std::runtime_error("could not encode " + std::to_string(x));
    }
    return bytes;
}

uint32_t DecodeUInt(const std::vector<uint8_t>& bytes)
{
    uint32_t result = 0;
    if (!bytes.empty())
    {
        uint8_t x = bytes[0];
        if ((x & 0x80u) == 0u)
        {
            result = x;
        }
        else if ((x & 0xE0u) == 0xC0u)
        {
            if (bytes.size() < 2)
            {
                throw std::runtime_error("could not decode: invalid byte sequence");
            }
            uint8_t b1 = bytes[1];
            if ((b1 & 0xC0u) != 0x80u)
            {
                throw std::runtime_error("could not decode: invalid byte sequence");
            }
            uint8_t shift = 0u;
            for (uint8_t i = 0u; i < 6u; ++i)
            {
                uint8_t bit = b1 & 1u;
                b1 = b1 >> 1u;
                result = result | static_cast<uint32_t>(bit) << shift;
                ++shift;
            }
            uint8_t b0 = x;
            for (uint8_t i = 0u; i < 5u; ++i)
            {
                uint8_t bit = b0 & 1u;
                b0 = b0 >> 1u;
                result = result | static_cast<uint32_t>(bit) << shift;
                ++shift;
            }
        }
        else if ((x & 0xF0u) == 0xE0u)
        {
            if (bytes.size() < 3)
            {
                throw std::runtime_error("could not decode: invalid byte sequence");
            }
            uint8_t b2 = bytes[2];
            if ((b2 & 0xC0u) != 0x80u)
            {
                throw std::runtime_error("could not decode: invalid byte sequence");
            }
            uint8_t shift = 0u;
            for (uint8_t i = 0u; i < 6u; ++i)
            {
                uint8_t bit = b2 & 1u;
                b2 = b2 >> 1u;
                result = result | static_cast<uint32_t>(bit) << shift;
                ++shift;
            }
            uint8_t b1 = bytes[1];
            if ((b1 & 0xC0u) != 0x80u)
            {
                throw std::runtime_error("could not decode: invalid byte sequence");
            }
            for (uint8_t i = 0u; i < 6u; ++i)
            {
                uint8_t bit = b1 & 1u;
                b1 = b1 >> 1u;
                result = result | static_cast<uint32_t>(bit) << shift;
                ++shift;
            }
            uint8_t b0 = x;
            for (uint8_t i = 0u; i < 4u; ++i)
            {
                uint8_t bit = b0 & 1u;
                b0 = b0 >> 1u;
                result = result | static_cast<uint32_t>(bit) << shift;
                ++shift;
            }
        }
        else if ((x & 0xF8u) == 0xF0u)
        {
            if (bytes.size() < 4)
            {
                throw std::runtime_error("could not decode: invalid byte sequence");
            }
            uint8_t b3 = bytes[3];
            if ((b3 & 0xC0u) != 0x80u)
            {
                throw std::runtime_error("could not decode: invalid byte sequence");
            }
            uint8_t shift = 0u;
            for (uint8_t i = 0u; i < 6u; ++i)
            {
                uint8_t bit = b3 & 1u;
                b3 = b3 >> 1u;
                result = result | static_cast<uint32_t>(bit) << shift;
                ++shift;
            }
            uint8_t b2 = bytes[2];
            if ((b2 & 0xC0u) != 0x80u)
            {
                throw std::runtime_error("could not decode: invalid byte sequence");
            }
            for (uint8_t i = 0u; i < 6u; ++i)
            {
                uint8_t bit = b2 & 1u;
                b2 = b2 >> 1u;
                result = result | static_cast<uint32_t>(bit) << shift;
                ++shift;
            }
            uint8_t b1 = bytes[1];
            if ((b1 & 0xC0u) != 0x80u)
            {
                throw std::runtime_error("could not decode: invalid byte sequence");
            }
            for (uint8_t i = 0u; i < 6u; ++i)
            {
                uint8_t bit = b1 & 1u;
                b1 = b1 >> 1u;
                result = result | static_cast<uint32_t>(bit) << shift;
                ++shift;
            }
            uint8_t b0 = x;
            for (uint8_t i = 0u; i < 3u; ++i)
            {
                uint8_t bit = b0 & 1u;
                b0 = b0 >> 1u;
                result = result | static_cast<uint32_t>(bit) << shift;
                ++shift;
            }
        }
    }
    else
    {
        throw std::runtime_error("could not decode: invalid byte sequence");
    }
    return result; 
}

namespace unicode {

std::string PathToUnicodeDirectory()
{
    std::string cminorRoot;
    char* root = std::getenv("CMINOR_ROOT");
    if (root)
    {
        cminorRoot = root;
    }
    if (cminorRoot.empty())
    {
        throw std::runtime_error("CMINOR_ROOT environment variable not set (set it to point to /path/to/cminor directory)");
    }
    return GetFullPath(Path::Combine(cminorRoot, "unicode"));
}

std::string PathToUnicodeBinFile()
{
    return GetFullPath(Path::Combine(PathToUnicodeDirectory(), "unicode.bin"));
}

class CategoryMap
{
public:
	static void Init();
	static void Done();
	static CategoryMap& Instance() { return *instance; }
	Category GetCategory(utf32_string categoryName) const;
	utf32_string GetCategoryName(Category category) const;
private:
	static std::unique_ptr<CategoryMap> instance;
	CategoryMap();
	std::unordered_map<utf32_string, Category> strCategoryMap;
	std::unordered_map<Category, utf32_string, CategoryHash> categoryStrMap;
};

std::unique_ptr<CategoryMap> CategoryMap::instance;

void CategoryMap::Init()
{
	instance.reset(new CategoryMap());
}

void CategoryMap::Done()
{
	instance.reset();
}

CategoryMap::CategoryMap()
{
	strCategoryMap[U"Lu"] = Category::letterUpper;
	categoryStrMap[Category::letterUpper] = U"Lu";
	strCategoryMap[U"Ll"] = Category::letterLower;
	categoryStrMap[Category::letterLower] = U"Ll";
	strCategoryMap[U"LC"] = Category::letterCased;
	categoryStrMap[Category::letterCased] = U"LC";
	strCategoryMap[U"Lm"] = Category::letterModifier;
	categoryStrMap[Category::letterModifier] = U"Lm";
	strCategoryMap[U"Lo"] = Category::letterOther;
	categoryStrMap[Category::letterOther] = U"Lo";
	strCategoryMap[U"Lt"] = Category::letterTitle;
	categoryStrMap[Category::letterTitle] = U"Lt";
	strCategoryMap[U"Mc"] = Category::markSpacing;
	categoryStrMap[Category::markSpacing] = U"Mc";
	strCategoryMap[U"Me"] = Category::markEnclosing;
	categoryStrMap[Category::markEnclosing] = U"Me";
	strCategoryMap[U"Mn"] = Category::markNonspacing;
	categoryStrMap[Category::markNonspacing] = U"Mn";
	strCategoryMap[U"Nd"] = Category::numberDecimal;
	categoryStrMap[Category::numberDecimal] = U"Nd";
	strCategoryMap[U"Nl"] = Category::numberLetter;
	categoryStrMap[Category::numberLetter] = U"Nl";
	strCategoryMap[U"No"] = Category::numberOther;
	categoryStrMap[Category::numberOther] = U"No";
	strCategoryMap[U"Pc"] = Category::punctuationConnector;
	categoryStrMap[Category::punctuationConnector] = U"Pc";
	strCategoryMap[U"Pd"] = Category::punctuationDash;
	categoryStrMap[Category::punctuationDash] = U"Pd";
	strCategoryMap[U"Pe"] = Category::punctuationClose;
	categoryStrMap[Category::punctuationClose] = U"Pe";
	strCategoryMap[U"Pf"] = Category::punctuationFinalQuote;
	categoryStrMap[Category::punctuationFinalQuote] = U"Pf";
	strCategoryMap[U"Pi"] = Category::punctuationInitialQuote;
	categoryStrMap[Category::punctuationInitialQuote] = U"Pi";
	strCategoryMap[U"Po"] = Category::punctuationOther;
	categoryStrMap[Category::punctuationOther] = U"Po";
	strCategoryMap[U"Ps"] = Category::punctuationOpen;
	categoryStrMap[Category::punctuationOpen] = U"Ps";
	strCategoryMap[U"Sc"] = Category::symbolCurrency;
	categoryStrMap[Category::symbolCurrency] = U"Sc";
	strCategoryMap[U"Sk"] = Category::symbolModifier;
	categoryStrMap[Category::symbolModifier] = U"Sk";
	strCategoryMap[U"Sm"] = Category::symbolMath;
	categoryStrMap[Category::symbolMath] = U"Sm";
	strCategoryMap[U"So"] = Category::symbolOther;
	categoryStrMap[Category::symbolOther] = U"So";
	strCategoryMap[U"Zl"] = Category::separatorLine;
	categoryStrMap[Category::separatorLine] = U"Zl";
	strCategoryMap[U"Zp"] = Category::separatorParagraph;
	categoryStrMap[Category::separatorParagraph] = U"Zp";
	strCategoryMap[U"Zs"] = Category::separatorSpace;
	categoryStrMap[Category::separatorSpace] = U"Zs";
}

Category CategoryMap::GetCategory(utf32_string categoryName) const
{
	Category category = Category::none;
	auto it = strCategoryMap.find(categoryName);
	if (it != strCategoryMap.cend())
	{
		return it->second;
	}
	return Category::none;
}

utf32_string CategoryMap::GetCategoryName(Category category) const
{
	auto it = categoryStrMap.find(category);
	if (it != categoryStrMap.cend())
	{
		return it->second;
	}
	return utf32_string();
}

void ThrowReadError()
{
	throw std::runtime_error("error reading unicode.bin");
}

void ThrowWriteError()
{
	throw std::runtime_error("error writing unicode.bin");
}

uint32_t ReadUInt(FILE* s)
{
	uint8_t x0 = 0;
	if (std::fread(&x0, 1, 1, s) != 1)
	{
		ThrowReadError();
	}
	uint8_t x1 = 0;
	if (std::fread(&x1, 1, 1, s) != 1)
	{
		ThrowReadError();
	}
	uint8_t x2 = 0;
	if (std::fread(&x2, 1, 1, s) != 1)
	{
		ThrowReadError();
	}
	uint8_t x3 = 0;
	if (std::fread(&x3, 1, 1, s) != 1)
	{
		ThrowReadError();
	}
	return static_cast<uint32_t>(x0) << 24 | static_cast<uint32_t>(x1) << 16 | static_cast<uint32_t>(x2) << 8 | static_cast<uint32_t>(x3);
}

void WriteUInt(FILE* s, uint32_t x)
{
	uint8_t x0 = static_cast<uint8_t>(x >> 24u);
	uint8_t x1 = static_cast<uint8_t>(x >> 16u);
	uint8_t x2 = static_cast<uint8_t>(x >> 8u);
	uint8_t x3 = static_cast<uint8_t>(x);
	if (std::fwrite(&x0, 1, 1, s) != 1)
	{
		ThrowWriteError();
	}
	if (std::fwrite(&x1, 1, 1, s) != 1)
	{
		ThrowWriteError();
	}
	if (std::fwrite(&x2, 1, 1, s) != 1)
	{
		ThrowWriteError();
	}
	if (std::fwrite(&x3, 1, 1, s) != 1)
	{
		ThrowWriteError();
	}
}

std::string ReadString(FILE* s)
{
	std::string str;
	uint8_t x = 0;
	if (std::fread(&x, 1, 1, s) != 1)
	{
		ThrowReadError();
	}
	while (x != 0)
	{
		str.append(1, char(x));
		if (std::fread(&x, 1, 1, s) != 1)
		{
			ThrowReadError();
		}
	}
	return str;
}

void WriteString(FILE* s, const std::string& str)
{
	for (char c : str)
	{
		uint8_t x = uint8_t(c);
		if (std::fwrite(&x, 1, 1, s) != 1)
		{
			ThrowWriteError();
		}
	}
	uint8_t x = 0;
	if (std::fwrite(&x, 1, 1, s) != 1)
	{
		ThrowWriteError();
	}
}

struct FilePtr
{
    FilePtr(FILE* file_) : file(file_) {}
    ~FilePtr() { if (file) fclose(file); }
    operator FILE*() const { return file; }
    FILE* file;
};

class CharacterInfo
{
public:
	CharacterInfo();
	CharacterInfo(char32_t code_, const utf32_string& name_, Category category_, char32_t toLower_, char32_t toUpper_);
	char32_t Code() const { return code; }
	Category Cat() const { return category; }
	char32_t Lower() const { return toLower; }
	char32_t Upper() const { return toUpper; }
	bool IsLetter() const { return (category & Category::letter) != Category::none; }
	bool IsMark() const { return (category & Category::mark) != Category::none; }
	bool IsNumber() const { return (category & Category::number) != Category::none; }
	bool IsPunctuation() const { return (category & Category::punctuation) != Category::none; }
	bool IsSymbol() const { return (category & Category::symbol) != Category::none; }
	bool IsSeparator() const { return (category & Category::separator) != Category::none; }
	void Read(FILE* unicodeBin);
	void Write(FILE* unicodeBin);
private:
	char32_t code;
	Category category;
	char32_t toLower;
	char32_t toUpper;
};

CharacterInfo::CharacterInfo() : code(char32_t(0)), category(Category::none), toLower(char32_t(0)), toUpper(char32_t(0))
{
}

CharacterInfo::CharacterInfo(char32_t code_, const utf32_string& name_, Category category_, char32_t toLower_, char32_t toUpper_) :
	code(code_), category(category_), toLower(toLower_), toUpper(toUpper_)
{
}

void CharacterInfo::Read(FILE* unicodeBin)
{
	code = static_cast<char32_t>(ReadUInt(unicodeBin));
	category = static_cast<Category>(ReadUInt(unicodeBin));
	toLower = static_cast<char32_t>(ReadUInt(unicodeBin));
	toUpper = static_cast<char32_t>(ReadUInt(unicodeBin));
}

void CharacterInfo::Write(FILE* unicodeBin)
{
	WriteUInt(unicodeBin, static_cast<uint32_t>(code));
	WriteUInt(unicodeBin, static_cast<uint32_t>(category));
	WriteUInt(unicodeBin, static_cast<uint32_t>(toLower));
	WriteUInt(unicodeBin, static_cast<uint32_t>(toUpper));
}

const uint32_t numCharactersInPage = 4096;
const uint32_t characterInfoPageSize = 4 * sizeof(uint32_t) * numCharactersInPage;

class CharacterInfoPage
{
public:
    CharacterInfoPage();
    void Read(FILE* unicodeBin);
    void Write(FILE* unicodeBin);
    const CharacterInfo& operator[](int index) const { return charInfos[index]; }
    void SetCharacterInfo(int index, const CharacterInfo& info) { charInfos[index] = info; }
private:
    std::vector<CharacterInfo> charInfos;
};

CharacterInfoPage::CharacterInfoPage()
{
    charInfos.resize(numCharactersInPage);
}

void CharacterInfoPage::Read(FILE* unicodeBin)
{
    for (CharacterInfo& charInfo : charInfos)
    {
        charInfo.Read(unicodeBin);
    }
}

void CharacterInfoPage::Write(FILE* unicodeBin)
{
    for (CharacterInfo& charInfo : charInfos)
    {
        charInfo.Write(unicodeBin);
    }
}

const char* headerMagic = "UNICODE1";

class UnicodeBinHeader
{
public:
    UnicodeBinHeader();
    void Write(FILE* unicodeBin);
    void Read(FILE* unicodeBin);
    uint32_t NumCharacterInfoPages() const { return numCharacterInfoPages; }
    void SetNumCharacterInfoPages(uint32_t numCharacterInfoPages_) { numCharacterInfoPages = numCharacterInfoPages_;  }
    uint32_t StartCharNameTable() const { return startCharNameTable; }
    void SetStartCharNameTable(uint32_t startCharNameTable_) { startCharNameTable = startCharNameTable_; }
    uint32_t StartCharInfoPagesPos() const { return startCharInfoPagesPos; }
private:
    uint32_t numCharacterInfoPages;
    uint32_t startCharNameTable;
    uint32_t startCharInfoPagesPos;
};

UnicodeBinHeader::UnicodeBinHeader(): numCharacterInfoPages(0), startCharInfoPagesPos(0), startCharNameTable(0)
{
}

void UnicodeBinHeader::Write(FILE* unicodeBin)
{
    int headerMagicLen = int(std::strlen(headerMagic));
    size_t n = std::fwrite(headerMagic, 1, headerMagicLen, unicodeBin);
    if (n != headerMagicLen)
    {
        throw std::runtime_error("could not write to unicode.bin: " + std::string(strerror(errno)));
    }
    WriteUInt(unicodeBin, numCharacterInfoPages);
    WriteUInt(unicodeBin, startCharNameTable);
}

void UnicodeBinHeader::Read(FILE* unicodeBin)
{
    int headerMagicLen = int(std::strlen(headerMagic));
    std::string headerBuf;
    headerBuf.resize(headerMagicLen);
    size_t n = std::fread(&headerBuf[0], 1, headerMagicLen, unicodeBin);
    if (n != headerMagicLen)
    {
        if (std::ferror(unicodeBin))
        {
            throw std::runtime_error("could not read from unicode.bin: " + std::string(strerror(errno)));
        }
        else
        {
            throw std::runtime_error("could not read from unicode.bin: unexpected end of file encountered while reading header");
        }
    }
    if (headerBuf != headerMagic)
    {
        std::string headerMagicStr = headerMagic;
        if (headerBuf.substr(0, headerMagicLen - 1) != headerMagicStr.substr(0, headerMagicLen - 1))
        {
            throw std::runtime_error("invalid unicode.bin header magic (not " + headerMagicStr + ")");
        }
        if (headerBuf.substr(headerMagicLen - 1) != headerMagicStr.substr(headerMagicLen - 1))
        {
            throw std::runtime_error("invalid unicode.bin version ('" + headerBuf.substr(headerMagicLen - 1) + "' read, '" + headerMagicStr.substr(headerMagicLen - 1) + "' expected)");
        }
    }
    numCharacterInfoPages = ReadUInt(unicodeBin);
    startCharNameTable = ReadUInt(unicodeBin);
    startCharInfoPagesPos = std::ftell(unicodeBin);
}

class CharacterInfoMap
{
public:
	static CharacterInfoMap& Instance() { return *instance; }
    static void Init();
    static void Done();
	void Write();
    void Add(const CharacterInfo& characterInfo);
    const CharacterInfo* GetCharacterInfo(char32_t c);
private:
	static std::unique_ptr<CharacterInfoMap> instance;
    std::unique_ptr<UnicodeBinHeader> header;
    std::vector<std::unique_ptr<CharacterInfoPage>> charInfoPages;
    CharacterInfoMap();
    CharacterInfoPage* GetCharInfoPage(char32_t c)
    {
        uint32_t charIndex = static_cast<uint32_t>(c);
        uint32_t pageNumber = charIndex / numCharactersInPage;
        while (pageNumber >= charInfoPages.size())
        {
            charInfoPages.push_back(std::unique_ptr<CharacterInfoPage>());
        }
        return charInfoPages[pageNumber].get();
    }
    void SetCharInfoPage(char32_t c, CharacterInfoPage* page)
    {
        uint32_t charIndex = static_cast<uint32_t>(c);
        uint32_t pageNumber = charIndex / numCharactersInPage;
        charInfoPages[pageNumber] = std::unique_ptr<CharacterInfoPage>(page);
    }
};

std::unique_ptr<CharacterInfoMap> CharacterInfoMap::instance;

void CharacterInfoMap::Init()
{
    instance.reset(new CharacterInfoMap());
}

void CharacterInfoMap::Done()
{
    instance.reset();
}

CharacterInfoMap::CharacterInfoMap()
{
}

void CharacterInfoMap::Write()
{
    std::string unicodeBinFilePath = PathToUnicodeBinFile();
    FilePtr unicodeBin = std::fopen(unicodeBinFilePath.c_str(), "wb");
    header.reset(new UnicodeBinHeader());
    header->SetNumCharacterInfoPages(uint32_t(charInfoPages.size()));
    header->Write(unicodeBin);
    CharacterInfoPage emptyPage;
    for (const std::unique_ptr<CharacterInfoPage>& page : charInfoPages)
    {
        if (page)
        {
            page->Write(unicodeBin);
        }
        else
        {
            emptyPage.Write(unicodeBin);
        }
    }
    header->SetStartCharNameTable(static_cast<uint32_t>(std::ftell(unicodeBin)));
    int seekResult = std::fseek(unicodeBin, 0, SEEK_SET);
    if (seekResult != 0)
    {
        throw std::runtime_error("could not seek unicode.bin: " + std::string(std::strerror(errno)));
    }
    header->Write(unicodeBin);
}

void CharacterInfoMap::Add(const CharacterInfo& characterInfo)
{
    char32_t c = characterInfo.Code();
    CharacterInfoPage* page = GetCharInfoPage(c);
    if (!page)
    {
        page = new CharacterInfoPage();
        SetCharInfoPage(c, page);
    }
    uint32_t charIndex = static_cast<uint32_t>(c);
    uint32_t infoIndex = charIndex % numCharactersInPage;
    page->SetCharacterInfo(infoIndex, characterInfo);
}

const CharacterInfo* CharacterInfoMap::GetCharacterInfo(char32_t c) 
{
    CharacterInfoPage* page = GetCharInfoPage(c);
    if (!page)
    {
        page = new CharacterInfoPage();
        std::string unicodeBinFilePath = PathToUnicodeBinFile();
        FilePtr unicodeBin = std::fopen(unicodeBinFilePath.c_str(), "rb");
        if (!unicodeBin)
        {
            throw std::runtime_error("could not open unicode.bin: " + std::string(std::strerror(errno)));
        }
        if (!header)
        {
            header.reset(new UnicodeBinHeader());
            header->Read(unicodeBin);
        }
        uint32_t charIndex = static_cast<uint32_t>(c);
        uint32_t pageNumber = charIndex / numCharactersInPage;
        uint32_t numPages = header->NumCharacterInfoPages();
        if (pageNumber < numPages)
        {
            int result = std::fseek(unicodeBin, static_cast<int>(header->StartCharInfoPagesPos()) + static_cast<int>(pageNumber) * static_cast<int>(characterInfoPageSize), SEEK_SET);
            if (result != 0)
            {
                throw std::runtime_error("could not seek unicode.bin: " + std::string(std::strerror(errno)));
            }
            page->Read(unicodeBin);
            SetCharInfoPage(c, page);
        }
        else
        {
            return nullptr;
        }
    }
    uint32_t charIndex = static_cast<uint32_t>(c);
    uint32_t infoIndex = charIndex % numCharactersInPage;
    const CharacterInfo& info = (*page)[infoIndex];
    if (info.Code() == c)
    {
        return &info;
    }
    return nullptr;
}

Category GetCategory(char32_t c)
{
	const CharacterInfo* info = CharacterInfoMap::Instance().GetCharacterInfo(c);
	if (info != nullptr)
	{
		return info->Cat();
	}
	return Category::none;
}

utf32_string GetCategoryName(Category category)
{
	return CategoryMap::Instance().GetCategoryName(category);
}

utf32_string GetCharacterName(char32_t c)
{
	return U""; // todo
}

char32_t ToLower(char32_t c)
{
	const CharacterInfo* info = CharacterInfoMap::Instance().GetCharacterInfo(c);
	if (info != nullptr)
	{
		char32_t toLower = info->Lower();
		if (toLower != static_cast<char32_t>(0))
		{
			return toLower;
		}
	}
	return c;
}

char32_t ToUpper(char32_t c)
{
	const CharacterInfo* info = CharacterInfoMap::Instance().GetCharacterInfo(c);
	if (info != nullptr)
	{
		char32_t toUpper = info->Upper();
		if (toUpper != static_cast<char32_t>(0))
		{
			return toUpper;
		}
	}
	return c;
}

bool IsLetter(char32_t c)
{
	const CharacterInfo* info = CharacterInfoMap::Instance().GetCharacterInfo(c);
	if (info != nullptr)
	{
		return info->IsLetter();
	}
	return false;
}

bool IsLower(char32_t c)
{
	return GetCategory(c) == Category::letterLower;
}

bool IsUpper(char32_t c)
{
	return GetCategory(c) == Category::letterUpper;
}

bool IsMark(char32_t c)
{
	const CharacterInfo* info = CharacterInfoMap::Instance().GetCharacterInfo(c);
	if (info != nullptr)
	{
		return info->IsMark();
	}
	return false;
}

bool IsNumber(char32_t c)
{
	const CharacterInfo* info = CharacterInfoMap::Instance().GetCharacterInfo(c);
	if (info != nullptr)
	{
		return info->IsNumber();
	}
	return false;
}

bool IsPunctuation(char32_t c)
{
	const CharacterInfo* info = CharacterInfoMap::Instance().GetCharacterInfo(c);
	if (info != nullptr)
	{
		return info->IsPunctuation();
	}
	return false;
}

bool IsSymbol(char32_t c)
{
	const CharacterInfo* info = CharacterInfoMap::Instance().GetCharacterInfo(c);
	if (info != nullptr)
	{
		return info->IsSymbol();
	}
	return false;
}

bool IsSeparator(char32_t c)
{
	const CharacterInfo* info = CharacterInfoMap::Instance().GetCharacterInfo(c);
	if (info != nullptr)
	{
		return info->IsSeparator();
	}
	return false;
}

utf32_string ToLower(const utf32_string& s)
{
	utf32_string lower;
	for (char32_t c : s)
	{
		lower.append(1, ToLower(c));
	}
	return lower;
}

utf32_string ToUpper(const utf32_string& s)
{
	utf32_string upper;
	for (char32_t c : s)
	{
		upper.append(1, ToUpper(c));
	}
	return upper;
}

bool IsIdentifier(const utf32_string& s)
{
    if (s.empty()) return false;
    char32_t first = s[0];
    if (!std::isalpha(char(first)) && first != U'_') return false;
    int n = int(s.length());
    for (int i = 1; i < n; ++i)
    {
        char32_t c = s[i];
        if (!std::isalnum(char(c)) && c != U'_') return false;
    }
/*
	char32_t first = s[0];
	if (!IsLetter(first) && first != U'_') return false;
	int n = int(s.length());
	for (int i = 1; i < n; ++i)
	{
		char32_t c = s[i];
		if (!IsLetter(c) && !IsNumber(c) && c != U'_') return false;
	}
    */
    return true;
}

void Init()
{
	CategoryMap::Init();
    CharacterInfoMap::Init();
}

void Done()
{
    CharacterInfoMap::Done();
	CategoryMap::Done();
}

} } } // namespace cminor::util::unicode
