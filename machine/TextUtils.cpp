/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#include <cminor/machine/TextUtils.hpp>
#include <cctype>
#include <sstream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <stdexcept>

namespace cminor { namespace machine {

std::string Trim(const std::string& s)
{
    int b = 0;
    while (b < int(s.length()) && std::isspace(s[b])) ++b;
    int e = int(s.length()) - 1;
    while (e >= b && std::isspace(s[e])) --e;
    return s.substr(b, e - b + 1);
}

std::string TrimAll(const std::string& s)
{
    std::string result;
    result.reserve(s.length());
    int state = 0;
    std::string::const_iterator e = s.cend();
    for (std::string::const_iterator i = s.cbegin(); i != e; ++i)
    {
        char c = *i;
        switch (state)
        {
            case 0:
            {
                if (!std::isspace(c))
                {
                    result.append(1, c);
                    state = 1;
                }
                break;
            }
            case 1:
            {
                if (std::isspace(c))
                {
                    state = 2;
                }
                else
                {
                    result.append(1, c);
                }
                break;
            }
            case 2:
            {
                if (!std::isspace(c))
                {
                    result.append(1, ' ');
                    result.append(1, c);
                    state = 1;
                }
                break;
            }
        }
    }
    return result;
}

std::vector<std::string> Split(const std::string& s, char c)
{
    std::vector<std::string> v;
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

std::string Replace(const std::string& s, char oldChar, char newChar)
{
    std::string t(s);
    std::string::iterator e = t.end();
    for (std::string::iterator i = t.begin(); i != e; ++i)
    {
        if (*i == oldChar)
        {
            *i = newChar;
        }
    }
    return t;
}

std::string Replace(const std::string& s, const std::string& oldString, const std::string& newString)
{
    std::string r;
    std::string::size_type start = 0;
    std::string::size_type pos = s.find(oldString.c_str(), start);
    while (pos != std::string::npos)
    {
        r.append(s.substr(start, pos - start));
        r.append(newString);
        start = pos + oldString.length();
        pos = s.find(oldString.c_str(), start);
    }
    r.append(s.substr(start, s.length() - start));
    return r;
}

std::string HexEscape(char c)
{
    std::stringstream s;
    s << "\\x" << std::hex << int(static_cast<unsigned char>(c));
    return s.str();
}

std::string HexEscape(uint16_t c)
{
    std::stringstream s;
    s << "\\x" << std::hex << c;
    return s.str();
}

std::string HexEscape(uint32_t c)
{
    std::stringstream s;
    s << "\\x" << std::hex << c;
    return s.str();
}

std::string CharStr(char c)
{
    switch (c)
    {
        case '\'': return "\\'";
        case '\"': return "\\\"";
        case '\\': return "\\\\";
        case '\a': return "\\a";
        case '\b': return "\\b";
        case '\f': return "\\f";
        case '\n': return "\\n";
        case '\r': return "\\r";
        case '\t': return "\\t";
        case '\v': return "\\v";
        case '\0': return "\\0";
        default:
        {
            if (c >= 32 && c <= 126)
            {
                return std::string(1, c);
            }
            else
            {
                return HexEscape(c);
            }
        }
    }
}

std::string WCharStr(uint16_t c)
{
    if (c < 0x100)
    {
        return CharStr(static_cast<char>(c));
    }
    else
    {
        return HexEscape(c);
    }
}

std::string UCharStr(uint32_t c)
{
    if (c < 0x100)
    {
        return CharStr(static_cast<char>(c));
    }
    else
    {
        return HexEscape(c);
    }
}

std::string StringStr(const std::string& s)
{
    std::string r;
    int n = int(s.length());
    for (int i = 0; i < n; ++i)
    {
        r.append(CharStr(s[i]));
    }
    return r;
}

std::string QuotedPath(const std::string& path)
{
    if (path.find(' ') != std::string::npos)
    {
        return std::string("\"") + path + "\"";
    }
    return path;
}

std::string ReadFile(const std::string& fileName)
{
    std::string content;
    content.reserve(4096);
    std::ifstream file(fileName.c_str());
    if (!file)
    {
        throw std::runtime_error("Error: Could not open input file: " + fileName);
    }
    file.unsetf(std::ios::skipws);
    std::copy(
        std::istream_iterator<char>(file),
        std::istream_iterator<char>(),
        std::back_inserter(content));
    int start = 0;
    if (content.size() >= 3)
    {
        if (((unsigned char)content[0] == (unsigned char)0xEF) &&
            ((unsigned char)content[1] == (unsigned char)0xBB) &&
            ((unsigned char)content[2] == (unsigned char)0xBF))
        {
            start += 3;
        }
    }
    return start == 0 ? content : content.substr(start);
}

bool LastComponentsEqual(const std::string& s0, const std::string& s1, char componentSeparator)
{
    std::vector<std::string> c0 = Split(s0, componentSeparator);
    std::vector<std::string> c1 = Split(s1, componentSeparator);
    int n0 = int(c0.size());
    int n1 = int(c1.size());
    int n = std::min(n0, n1);
    for (int i = 0; i < n; ++i)
    {
        if (c0[n0 - i - 1] != c1[n1 - i - 1]) return false;
    }
    return true;
}

bool StartsWith(const std::string& s, const std::string& prefix)
{
    int n = int(prefix.length());    
    return int(s.length()) >= n && s.substr(0, n) == prefix;
}

bool EndsWith(const std::string& s, const std::string& suffix)
{
    int n = int(suffix.length());
    int m = int(s.length());
    return m >= n && s.substr(m - n, n) == suffix;
}

std::string NarrowString(const char* str, int length)
{
#if defined(__linux) || defined(__posix) || defined(__unix)
    return std::string(str, length);
#elif defined(_WIN32)
    std::string narrow;
    narrow.reserve(length);
    int state = 0;
    for (int i = 0; i < length; ++i)
    {
        char c = str[i];
        switch (state)
        {
            case 0:
            {
                if (c == '\r') state = 1; else narrow.append(1, c);
                break;
            }
            case 1:
            {
                if (c == '\n') narrow.append(1, '\n'); else narrow.append(1, '\r').append(1, c);
                state = 0;
                break;
            }
        }
    }
    return narrow;
#else
    #error unknown platform
#endif
}

std::string ToUpper(const std::string& s)
{
    std::string result;
    int n = int(s.size());
    result.reserve(n);
    for (int i = 0; i < n; ++i)
    {
        result.append(1, std::toupper(s[i]));
    }
    return result;
}

std::string ToLower(const std::string& s)
{
    std::string result;
    int n = int(s.size());
    result.reserve(n);
    for (int i = 0; i < n; ++i)
    {
        result.append(1, std::tolower(s[i]));
    }
    return result;
}

std::string ToString(double x)
{
    return ToString(x, 15);
}

std::string ToString(double x, int maxNumDecimals)
{
    return ToString(x, 0, maxNumDecimals);
}

std::string ToString(double x, int minNumDecimals, int maxNumDecimals)
{
    std::string result;
    if (x < 0)
    {
        x = -x;
        result.append(1, '-');
    }
    result.append(std::to_string(static_cast<int>(x)));
    double d = x - static_cast<int>(x);
    if (d > 0 || minNumDecimals > 0)
    {
        result.append(1, '.');
        for (int i = 0; (d > 0 || i < minNumDecimals) && i < maxNumDecimals; ++i)
        {
            d = 10 * d;
            int digit = static_cast<int>(d) % 10;
            result.append(1, static_cast<char>(static_cast<int>('0') + digit));
            d = d - static_cast<int>(d);
        }
    }
    return result;
}

std::vector<uint32_t> ToUtf32(const std::string& utf8Str)
{
    std::vector<uint32_t> result;
    const char* p = &utf8Str[0];
    int bytesRemaining = int(utf8Str.length());
    while (bytesRemaining > 0)
    {
        char c = *p;
        uint8_t x = static_cast<uint8_t>(c);
        if ((x & 0x80) == 0)
        {
            result.push_back(static_cast<uint32_t>(x));
            --bytesRemaining;
            ++p;
        }
        else if ((x & 0xE0) == 0xC0)
        {
            if (bytesRemaining < 2)
            {
                throw std::runtime_error("invalid UTF-8 sequence");
            }
            uint32_t u = 0;
            uint8_t b1 = static_cast<uint8_t>(p[1]);
            if ((b1 & 0xC0) != 0x80)
            {
                throw std::runtime_error("invalid UTF-8 sequence");
            }
            uint8_t shift = 0;
            for (int i = 0; i < 6; ++i)
            {
                uint8_t bit = b1 & 1;
                b1 = b1 >> 1;
                u = u | (bit << shift);
                ++shift;
            }
            uint8_t b0 = x;
            for (int i = 0; i < 5; ++i)
            {
                uint8_t bit = b0 & 1;
                b0 = b0 >> 1;
                u = u | (bit << shift);
                ++shift;
            }
            result.push_back(u);
            bytesRemaining = bytesRemaining - 2;
            p = p + 2;
        }
        else if ((x & 0xF0) == 0xE0)
        {
            if (bytesRemaining < 3)
            {
                throw std::runtime_error("invalid UTF-8 sequence");
            }
            uint32_t u = 0;
            uint8_t b2 = static_cast<uint8_t>(p[2]);
            if ((b2 & 0xC0) != 0x80)
            {
                throw std::runtime_error("invalid UTF-8 sequence");
            }
            uint8_t shift = 0;
            for (int i = 0; i < 6; ++i)
            {
                uint8_t bit = b2 & 1;
                b2 = b2 >> 1;
                u = u | (bit << shift);
                ++shift;
            }
            uint8_t b1 = static_cast<uint8_t>(p[1]);
            if ((b1 & 0xC0) != 0x80)
            {
                throw std::runtime_error("invalid UTF-8 sequence");
            }
            for (int i = 0; i < 6; ++i)
            {
                uint8_t bit = b1 & 1;
                b1 = b1 >> 1;
                u = u | (bit << shift);
                ++shift;
            }
            uint8_t b0 = x;
            for (int i = 0; i < 4; ++i)
            {
                uint8_t bit = b0 & 1;
                b0 = b0 >> 1;
                u = u | (bit << shift);
                ++shift;
            }
            result.push_back(u);
            bytesRemaining = bytesRemaining - 3;
            p = p + 3;
        }
        else if ((x & 0xF8) == 0xF0)
        {
            if (bytesRemaining < 4)
            {
                throw std::runtime_error("invalid UTF-8 sequence");
            }
            uint32_t u = 0;
            uint8_t b3 = static_cast<uint8_t>(p[3]);
            if ((b3 & 0xC0) != 0x80)
            {
                throw std::runtime_error("invalid UTF-8 sequence");
            }
            uint8_t shift = 0;
            for (int i = 0; i < 6; ++i)
            {
                uint8_t bit = b3 & 1;
                b3 = b3 >> 1;
                u = u | (bit << shift);
                ++shift;
            }
            uint8_t b2 = static_cast<uint8_t>(p[2]);
            if ((b2 & 0xC0) != 0x80)
            {
                throw std::runtime_error("invalid UTF-8 sequence");
            }
            for (int i = 0; i < 6; ++i)
            {
                uint8_t bit = b2 & 1;
                b2 = b2 >> 1;
                u = u | (bit << shift);
                ++shift;
            }
            uint8_t b1 = static_cast<uint8_t>(p[1]);
            if ((b1 & 0xC0) != 0x80)
            {
                throw std::runtime_error("invalid UTF-8 sequence");
            }
            for (int i = 0; i < 6; ++i)
            {
                uint8_t bit = b1 & 1;
                b1 = b1 >> 1;
                u = u | (bit << shift);
                ++shift;
            }
            uint8_t b0 = x;
            for (int i = 0; i < 3; ++i)
            {
                uint8_t bit = b0 & 1;
                b0 = b0 >> 1;
                u = u | (bit << shift);
                ++shift;
            }
            result.push_back(u);
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

std::vector<uint16_t> ToUtf16(const std::vector<uint32_t>& utf32)
{
    std::vector<uint16_t> result;
    for (uint32_t u : utf32)
    {
        if (u > 0x10FFFF)
        {
            throw std::runtime_error("invalid UTF-32 code point");
        }
        if (u < 0x10000)
        {
            if (u >= 0xD800 && u <= 0xDFFF)
            {
                throw std::runtime_error("invalid UTF-32 code point (reserved for UTF-16");
            }
            uint16_t x = static_cast<uint16_t>(u);
            result.push_back(x);
        }
        else
        {
            uint32_t uprime = u - 0x10000;
            uint16_t w1 = 0xD800;
            uint16_t w2 = 0xDC00;
            for (uint16_t i = 0; i < 10; ++i)
            {
                uint16_t bit = static_cast<uint16_t>(uprime & (static_cast<uint16_t>(0x1) << i));
                w2 = w2 | bit;
            }
            for (uint16_t i = 10; i < 20; ++i)
            {
                uint16_t bit = static_cast<uint16_t>((uprime & (static_cast<uint32_t>(0x1) << i)) >> 10);
                w1 = w1 | bit;
            }
            result.push_back(w1);
            result.push_back(w2);
        }
    }
    return result;
}

std::vector<uint32_t> ToUtf32(const std::vector<uint16_t>& utf16Str)
{
    std::vector<uint32_t> result;
    const uint16_t* w = &utf16Str[0];
    int remaining = int(utf16Str.size());
    while (remaining > 0)
    {
        uint16_t w1 = *w++;
        --remaining;
        if (w1 < 0xD800 || w1 > 0xDFFF)
        {
            result.push_back(w1);
        }
        else
        {
            if (w1 < 0xD800 || w1 > 0xDBFF)
            {
                throw std::runtime_error("invalid UTF-16 sequence");
            }
            if (remaining > 0)
            {
                uint16_t w2 = *w++;
                --remaining;
                if (w2 < 0xDC00 || w2 > 0xDFFF)
                {
                    throw std::runtime_error("invalid UTF-16 sequence");
                }
                else
                {
                    uint32_t uprime = ((0x03FF & static_cast<uint32_t>(w1)) << 10) | (0x03FF & static_cast<uint32_t>(w2));
                    uint32_t u = uprime + 0x10000;
                    result.push_back(u);
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

std::vector<uint16_t> ToUtf16(const std::string& utf8Str)
{
    return ToUtf16(ToUtf32(utf8Str));
}

std::string ToUtf8(const std::vector<uint32_t>& utf32Str)
{
    std::string s;
    for (uint32_t c : utf32Str)
    {
        if (c < 0x80)
        {
            s.append(1, static_cast<char>(c & 0x7F));
        }
        else if (c < 0x800)
        {
            uint8_t b1 = 0x80;
            for (int i = 0; i < 6; ++i)
            {
                b1 = b1 | static_cast<uint8_t>((c & 1) << i);
                c = static_cast<uint32_t>(c >> 1);
            }
            uint8_t b0 = 0xC0;
            for (int i = 0; i < 5; ++i)
            {
                b0 = b0 | static_cast<uint8_t>((c & 1) << i);
                c = static_cast<uint32_t>(c >> 1);
            }
            s.append(1, static_cast<char>(b0));
            s.append(1, static_cast<char>(b1));
        }
        else if (c < 0x10000)
        {
            uint8_t b2 = 0x80;
            for (int i = 0; i < 6; ++i)
            {
                b2 = b2 | static_cast<uint8_t>((c & 1) << i);
                c = static_cast<uint32_t>(c >> 1);
            }
            uint8_t b1 = 0x80;
            for (int i = 0; i < 6; ++i)
            {
                b1 = b1 | static_cast<uint8_t>((c & 1) << i);
                c = static_cast<uint32_t>(c >> 1);
            }
            uint8_t b0 = 0xE0;
            for (int i = 0; i < 4; ++i)
            {
                b0 = b0 | static_cast<uint8_t>((c & 1) << i);
                c = static_cast<uint32_t>(c >> 1);
            }
            s.append(1, static_cast<char>(b0));
            s.append(1, static_cast<char>(b1));
            s.append(1, static_cast<char>(b2));
        }
        else if (c < 0x110000)
        {
            uint8_t b3 = 0x80;
            for (int i = 0; i < 6; ++i)
            {
                b3 = b3 | static_cast<uint8_t>((c & 1) << i);
                c = static_cast<uint32_t>(c >> 1);
            }
            uint8_t b2 = 0x80;
            for (int i = 0; i < 6; ++i)
            {
                b2 = b2 | static_cast<uint8_t>((c & 1) << i);
                c = static_cast<uint32_t>(c >> 1);
            }
            uint8_t b1 = 0x80;
            for (int i = 0; i < 6; ++i)
            {
                b1 = b1 | static_cast<uint8_t>((c & 1) << i);
                c = static_cast<uint32_t>(c >> 1);
            }
            uint8_t b0 = 0xF0;
            for (int i = 0; i < 3; ++i)
            {
                b0 = b0 | static_cast<uint8_t>((c & 1) << i);
                c = static_cast<uint32_t>(c >> 1);
            }
            s.append(1, static_cast<char>(b0));
            s.append(1, static_cast<char>(b1));
            s.append(1, static_cast<char>(b2));
            s.append(1, static_cast<char>(b3));
        }
        else
        {
            throw std::runtime_error("invalid UTF-32 code point");
        }
    }
    return s;
}

std::string ToUtf8(const std::vector<uint16_t>& utf16Str)
{
    return ToUtf8(ToUtf32(utf16Str));
}

} } // namespace cminor::machine
