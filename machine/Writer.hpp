// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_WRITER_INCLUDED
#define CMINOR_MACHINE_WRITER_INCLUDED
#include <cminor/machine/Error.hpp>
#include <stdint.h>

namespace cminor { namespace machine {

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

typedef std::basic_string<char32_t> utf32_string;

class Writer
{
public:
    Writer(const std::string& fileName_);
    virtual ~Writer();
    void Put(bool x);
    void Put(uint8_t x);
    void Put(int8_t x);
    void Put(uint16_t x);
    void Put(int16_t x);
    void Put(uint32_t x);
    void Put(int32_t x);
    void Put(uint64_t x);
    void Put(int64_t x);
    void Put(float x);
    void Put(double x);
    void Put(char32_t x);
    void Put(const std::string& s);
    void Put(const utf32_string& s);
    void Put(const Span& span);
private:
    static const int N = 8192;
    std::string fileName;
    FilePtr file;
    uint8_t buffer[N];
    uint8_t* bufp;
    uint8_t* bufend;
    void BufferReset() { bufp = buffer; bufend = buffer + N;  }
    bool BufferFull() const { return bufp == bufend; }
    void FlushBuffer();
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_WRITER_INCLUDED
