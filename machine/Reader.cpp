// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Reader.hpp>

namespace cminor { namespace machine {

Reader::Reader(Machine& machine_, const std::string& fileName_) :
    machine(machine_), fileName(fileName_), file(fileName), begin(reinterpret_cast<const uint8_t*>(file.Begin())), end(reinterpret_cast<const uint8_t*>(file.End()))
{
}

Reader::~Reader()
{
}

bool Reader::GetBool()
{
    uint8_t x = GetByte();
    return static_cast<bool>(x);
}

uint8_t Reader::GetByte()
{
    CheckEof();
    return *begin++;
}

int8_t Reader::GetSByte()
{
    uint8_t x = GetByte();
    return static_cast<int8_t>(x);
}

uint16_t Reader::GetUShort()
{
    uint8_t h = GetByte();
    uint8_t l = GetByte();
    return (static_cast<uint16_t>(h) << 8) | static_cast<uint16_t>(l);
}

int16_t Reader::GetShort()
{
    uint16_t x = GetUShort();
    return static_cast<int16_t>(x);
}

uint32_t Reader::GetUInt()
{
    uint8_t b0 = GetByte();
    uint8_t b1 = GetByte();
    uint8_t b2 = GetByte();
    uint8_t b3 = GetByte();
    return (static_cast<uint32_t>(b0) << 24) | (static_cast<uint32_t>(b1) << 16) | (static_cast<uint32_t>(b2) << 8) | static_cast<uint32_t>(b3);
}

int32_t Reader::GetInt()
{
    uint32_t x = GetUInt();
    return static_cast<int32_t>(x);
}

uint64_t Reader::GetULong()
{
    uint8_t b0 = GetByte();
    uint8_t b1 = GetByte();
    uint8_t b2 = GetByte();
    uint8_t b3 = GetByte();
    uint8_t b4 = GetByte();
    uint8_t b5 = GetByte();
    uint8_t b6 = GetByte();
    uint8_t b7 = GetByte();
    return (static_cast<uint64_t>(b0) << 56) | (static_cast<uint64_t>(b1) << 48) | (static_cast<uint64_t>(b2) << 40) | (static_cast<uint64_t>(b3) << 32) | (static_cast<uint64_t>(b4) << 24) |
        (static_cast<uint64_t>(b5) << 16) | (static_cast<uint64_t>(b6) << 8) | static_cast<uint64_t>(b7);
}

int64_t Reader::GetLong()
{
    uint64_t x = GetULong();
    return static_cast<int64_t>(x);
}

union floatLayout
{
    float f;
    uint32_t i;
};

float Reader::GetFloat()
{
    floatLayout layout;
    layout.i = GetUInt();
    return layout.f;
}

union doubleLayout
{
    double d;
    uint64_t l;
};

double Reader::GetDouble()
{
    doubleLayout layout;
    layout.l = GetULong();
    return layout.d;
}

char32_t Reader::GetChar()
{
    uint32_t x = GetUInt();
    return static_cast<char32_t>(x);
}

std::string Reader::GetUtf8String()
{
    std::string s;
    uint8_t x = GetByte();
    while (x != 0)
    {
        s.append(1, static_cast<char>(x));
        x = GetByte();
    }
    return s;
}

utf32_string Reader::GetUtf32String()
{
    std::string s = GetUtf8String();
    return ToUtf32(s);
}

void Reader::CheckEof()
{
    if (begin == end) throw std::runtime_error("unexpected end of file '" + fileName + "'");
}

Span Reader::GetSpan()
{
    bool valid = GetBool();
    if (!valid) return Span();
    int32_t fileIndex = GetInt();
    int32_t lineNumber = GetInt();
    int32_t start = GetInt();
    int32_t end = GetInt();
    return Span(fileIndex, lineNumber, start, end);
}

} } // namespace cminor::machine
