// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/util/BinaryReader.hpp>
#include <cminor/util/Unicode.hpp>

namespace cminor { namespace util {

using namespace cminor::unicode;

BinaryReader::BinaryReader(const std::string& fileName_) : 
    fileName(fileName_), file(fileName), begin(reinterpret_cast<const uint8_t*>(file.Begin())), end(reinterpret_cast<const uint8_t*>(file.End()))
{
}

BinaryReader::~BinaryReader()
{
}

bool BinaryReader::ReadBool()
{
    uint8_t x = ReadByte();
    return static_cast<bool>(x);
}

uint8_t BinaryReader::ReadByte()
{
    CheckEof();
    uint8_t x = *begin++;
    ++pos;
    return x;
}

int8_t BinaryReader::ReadSByte()
{
    uint8_t x = ReadByte();
    return static_cast<int8_t>(x);
}

uint16_t BinaryReader::ReadUShort()
{
    uint8_t h = ReadByte();
    uint8_t l = ReadByte();
    return (static_cast<uint16_t>(h) << 8) | static_cast<uint16_t>(l);
}

int16_t BinaryReader::ReadShort()
{
    uint16_t x = ReadUShort();
    return static_cast<int16_t>(x);
}

uint32_t BinaryReader::ReadUInt()
{
    uint8_t b0 = ReadByte();
    uint8_t b1 = ReadByte();
    uint8_t b2 = ReadByte();
    uint8_t b3 = ReadByte();
    return (static_cast<uint32_t>(b0) << 24) | (static_cast<uint32_t>(b1) << 16) | (static_cast<uint32_t>(b2) << 8) | static_cast<uint32_t>(b3);
}

int32_t BinaryReader::ReadInt()
{
    uint32_t x = ReadUInt();
    return static_cast<int32_t>(x);
}

uint64_t BinaryReader::ReadULong()
{
    uint8_t b0 = ReadByte();
    uint8_t b1 = ReadByte();
    uint8_t b2 = ReadByte();
    uint8_t b3 = ReadByte();
    uint8_t b4 = ReadByte();
    uint8_t b5 = ReadByte();
    uint8_t b6 = ReadByte();
    uint8_t b7 = ReadByte();
    return (static_cast<uint64_t>(b0) << 56) | (static_cast<uint64_t>(b1) << 48) | (static_cast<uint64_t>(b2) << 40) | (static_cast<uint64_t>(b3) << 32) | (static_cast<uint64_t>(b4) << 24) |
        (static_cast<uint64_t>(b5) << 16) | (static_cast<uint64_t>(b6) << 8) | static_cast<uint64_t>(b7);
}

int64_t BinaryReader::ReadLong()
{
    uint64_t x = ReadULong();
    return static_cast<int64_t>(x);
}

float BinaryReader::ReadFloat()
{
    uint32_t x = ReadUInt();
    return *reinterpret_cast<float*>(&x);
}

double BinaryReader::ReadDouble()
{
    uint64_t x = ReadULong();
    return *reinterpret_cast<double*>(&x);
}

char32_t BinaryReader::ReadChar()
{
    uint32_t x = ReadInt();
    return static_cast<char32_t>(x);
}

std::string BinaryReader::ReadUtf8String()
{
    std::string s;
    uint8_t x = ReadByte();
    while (x != 0)
    {
        s.append(1, static_cast<char>(x));
        x = ReadByte();
    }
    return s;
}

std::u32string BinaryReader::ReadUtf32String()
{
    std::string s = ReadUtf8String();
    return ToUtf32(s);
}

uint32_t BinaryReader::ReadEncodedUInt()
{
    uint8_t x = ReadByte();
    if ((x & 0x80u) == 0u)
    {
        return x;
    }
    else if ((x & 0xE0u) == 0xC0u)
    {
        uint32_t result = 0;
        uint8_t b1 = ReadByte();
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
        uint8_t b1 = ReadByte();
        uint8_t b2 = ReadByte();
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
        uint8_t b1 = ReadByte();
        uint8_t b2 = ReadByte();
        uint8_t b3 = ReadByte();
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

void BinaryReader::Skip(uint32_t size)
{
    begin += size;
    pos += size;
}

void BinaryReader::CheckEof()
{
    if (begin == end)
    {
        throw std::runtime_error("unexpected end of file '" + fileName + "'");
    }
}

} } // namespace cminor::util
