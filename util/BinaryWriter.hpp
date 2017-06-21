// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_UTIL_BINARY_WRITER_INCLUDED
#define CMINOR_UTIL_BINARY_WRITER_INCLUDED
#include <string>
#include <stdio.h>

namespace cminor { namespace util {

class FilePtr
{
public:
    FilePtr(FILE* fp_) : fp(fp_)
    {
    }
    ~FilePtr()
    {
        if (fp != nullptr)
        {
            fclose(fp);
        }
    }
    operator FILE*() const { return fp; }
private:
    FILE* fp;
};

class BinaryWriter
{
public:
    BinaryWriter(const std::string& fileName_);
    virtual ~BinaryWriter();
    void Write(bool x);
    void Write(uint8_t x);
    void Write(int8_t x);
    void Write(uint16_t x);
    void Write(int16_t x);
    void Write(uint32_t x);
    void Write(int32_t x);
    void Write(uint64_t x);
    void Write(int64_t x);
    void Write(float x);
    void Write(double x);
    void Write(char32_t x);
    void Write(const std::string& s);
    void Write(const std::u32string& s);
    void WriteEncodedUInt(uint32_t x);
    uint32_t Pos() const { return pos; }
    void Seek(uint32_t pos_);
private:
    static const int N = 8192;
    std::string fileName;
    FilePtr file;
    uint8_t buffer[N];
    uint8_t* bufp;
    uint8_t* bufend;
    uint32_t pos;
    void BufferReset() { bufp = buffer; bufend = buffer + N; }
    bool BufferFull() const { return bufp == bufend; }
    void FlushBuffer();
};

} } // namespace cminor::util

#endif // CMINOR_UTIL_BINARY_WRITER_INCLUDED
