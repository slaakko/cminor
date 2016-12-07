// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Writer.hpp>

namespace cminor { namespace machine {

Writer::Writer(const std::string& fileName_) : fileName(fileName_), file(std::fopen(fileName.c_str(), "wb")), bufp(buffer), bufend(buffer + N), constantPool(nullptr), pos(0)
{
    if (!file)
    {
        throw std::runtime_error("could not open '" + fileName + "' for writing: " + strerror(errno));
    }
}

Writer::~Writer()
{
    FlushBuffer();
}

void Writer::Put(bool x)
{
    Put(uint8_t(x));
}

void Writer::Put(uint8_t x)
{
    if (BufferFull()) FlushBuffer();
    *bufp++ = x;
    ++pos;
}

void Writer::Put(int8_t x)
{
    Put(static_cast<uint8_t>(x));
}

void Writer::Put(uint16_t x)
{
    uint8_t b0 = static_cast<uint8_t>(x >> 8);
    uint8_t b1 = static_cast<uint8_t>(x);
    Put(b0);
    Put(b1);
}

void Writer::Put(int16_t x)
{
    Put(static_cast<uint16_t>(x));
}

void Writer::Put(uint32_t x)
{
    uint8_t b0 = static_cast<uint8_t>(x >> 24);
    uint8_t b1 = static_cast<uint8_t>(x >> 16);
    uint8_t b2 = static_cast<uint8_t>(x >> 8);
    uint8_t b3 = static_cast<uint8_t>(x);
    Put(b0);
    Put(b1);
    Put(b2);
    Put(b3);
}

void Writer::Put(int32_t x)
{
    Put(static_cast<uint32_t>(x));
}

void Writer::Put(uint64_t x)
{
    uint8_t b0 = static_cast<uint8_t>(x >> 56);
    uint8_t b1 = static_cast<uint8_t>(x >> 48);
    uint8_t b2 = static_cast<uint8_t>(x >> 40);
    uint8_t b3 = static_cast<uint8_t>(x >> 32);
    uint8_t b4 = static_cast<uint8_t>(x >> 24);
    uint8_t b5 = static_cast<uint8_t>(x >> 16);
    uint8_t b6 = static_cast<uint8_t>(x >> 8);
    uint8_t b7 = static_cast<uint8_t>(x);
    Put(b0);
    Put(b1);
    Put(b2);
    Put(b3);
    Put(b4);
    Put(b5);
    Put(b6);
    Put(b7);
}

void Writer::Put(int64_t x)
{
    Put(static_cast<uint64_t>(x));
}

union floatLayout
{
    float f;
    uint32_t i;
};

void Writer::Put(float x)
{
    floatLayout layout;
    layout.f = x;
    Put(layout.i);
}

union doubleLayout
{
    double d;
    uint64_t l;
};

void Writer::Put(double x)
{
    doubleLayout layout;
    layout.d = x;
    Put(layout.l);
}

void Writer::Put(char32_t x)
{
    Put(static_cast<uint32_t>(x));
}

void Writer::Put(const std::string& s)
{
    for (char c : s)
    {
        uint8_t x = static_cast<uint8_t>(c);
        Put(x);
    }
    Put(static_cast<uint8_t>(0));
}

void Writer::Put(const utf32_string& s)
{
    std::string utf8_str = ToUtf8(s);
    Put(utf8_str);
}

void Writer::Put(const Span& span)
{
    if (!span.Valid())
    {
        Put(false);
    }
    else
    {
        Put(true);
        PutEncodedUInt(span.FileIndex());
        PutEncodedUInt(span.LineNumber());
        PutEncodedUInt(span.Start());
        PutEncodedUInt(span.End());
    }
}

void Writer::PutEncodedUInt(uint32_t x)
{
    if (x < 0x80u)
    {
        Put(static_cast<uint8_t>(x & 0x7Fu));
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
        Put(b0);
        Put(b1);
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
        Put(b0);
        Put(b1);
        Put(b2);
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
        Put(b0);
        Put(b1);
        Put(b2);
        Put(b3);
    }
    else
    {
        throw std::runtime_error("could not encode " + std::to_string(x));
    }
}

void Writer::Seek(uint32_t pos_)
{
    FlushBuffer();
    pos = pos_;
    int result = fseek(file, pos, SEEK_SET);
    if (result != 0)
    {
        throw std::runtime_error("seek failed");
    }
}

void Writer::FlushBuffer()
{
    if (bufp != buffer)
    {
        size_t n = bufp - buffer;
        size_t numWritten = fwrite(buffer, 1, n, file);
        if (numWritten != n)
        {
            throw std::runtime_error("could not write to '" + fileName + "': " + strerror(errno));
        }
        BufferReset();
    }
}

} } // namespace cminor::machine
