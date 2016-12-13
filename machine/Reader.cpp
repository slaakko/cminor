// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Reader.hpp>

namespace cminor { namespace machine {

Reader::Reader(const std::string& filePath_) :
    machine(nullptr), filePath(filePath_), file(filePath), begin(reinterpret_cast<const uint8_t*>(file.Begin())), end(reinterpret_cast<const uint8_t*>(file.End())), constantPool(nullptr), pos(0)
{
}

Reader::~Reader()
{
}

void Reader::AddCallInst(CallInst* callInst)
{
    callInstructions.push_back(callInst);
}

void Reader::AddFun2DlgInst(Fun2DlgInst* fun2DlgInst)
{
    fun2DlgInstructions.push_back(fun2DlgInst);
}

void Reader::AddTypeInstruction(TypeInstruction* typeInst)
{
    typeInstructions.push_back(typeInst);
}

void Reader::AddSetClassDataInst(SetClassDataInst* setClassDataInst)
{
    setClassDataInstructions.push_back(setClassDataInst);
}

bool Reader::GetBool()
{
    uint8_t x = GetByte();
    return static_cast<bool>(x);
}

uint8_t Reader::GetByte()
{
    CheckEof();
    uint8_t x = *begin++;
    ++pos;
    return x;
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
    if (begin == end) throw std::runtime_error("unexpected end of file '" + filePath + "'");
}

Span Reader::GetSpan()
{
    bool valid = GetBool();
    if (!valid) return Span();
    uint32_t fileIndex = GetEncodedUInt();
    uint32_t lineNumber = GetEncodedUInt();
    uint32_t start = GetEncodedUInt();
    uint32_t end = GetEncodedUInt();
    return Span(fileIndex, lineNumber, start, end);
}

uint32_t Reader::GetEncodedUInt()
{
    uint8_t x = GetByte();
    if ((x & 0x80u) == 0u)
    {
        return x;
    }
    else if ((x & 0xE0u) == 0xC0u)
    {
        uint32_t result = 0;
        uint8_t b1 = GetByte();
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
        return result;
    }
    else if ((x & 0xF0u) == 0xE0u)
    {
        uint32_t result = 0;
        uint8_t b1 = GetByte();
        uint8_t b2 = GetByte();
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
        return result;
    }
    else if ((x & 0xF8u) == 0xF0u)
    {
        uint32_t result = 0;
        uint8_t b1 = GetByte();
        uint8_t b2 = GetByte();
        uint8_t b3 = GetByte();
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
        return result;
    }
    else
    {
        throw std::runtime_error("could not decode: invalid byte sequence");
    }
}

void Reader::Skip(uint32_t size)
{
    begin += size;
    pos += size;
}

} } // namespace cminor::machine
