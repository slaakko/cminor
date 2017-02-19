// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/util/Unicode.hpp>
#include <stdexcept>

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

} } // namespace cminor::util
