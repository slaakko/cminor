// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_PARSING_EXCEPTION_INCLUDED
#define CMINOR_PARSING_EXCEPTION_INCLUDED
#include <cminor/parsing/Scanner.hpp>
#include <stdexcept>

namespace cminor { namespace parsing {

void SetFileInfo(const std::vector<std::string>& files_);
std::u32string GetErrorLines(const char32_t* start, const char32_t* end, const Span& span);
void ThrowException(const std::string& message, const Span& span);

class ParsingException : public std::runtime_error
{
public:
    ParsingException(const std::string& message_, const std::string& fileName_, const Span& span_, const char32_t* start_, const char32_t* end_);
    const std::string& Message() const { return message; }
    const std::string& FileName() const { return fileName; }
    const Span& GetSpan() const { return span; }
    const char32_t* Start() const { return start; }
    const char32_t* End() const { return end; }
private:
    std::string message;
    std::string fileName;
    Span span;
    const char32_t* start;
    const char32_t* end;
};

class ExpectationFailure : public ParsingException
{
public:
    ExpectationFailure(const std::u32string& info_, const std::string& fileName_, const Span& span_, const char32_t* start, const char32_t* end);
    const std::u32string& Info() const { return info; }
    void CombineInfo(const std::u32string& parentInfo);
private:
    std::u32string info;
};

} } // namespace cminor::parsing

#endif // CMINOR_PARSING_EXCEPTION_INCLUDED
