// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_ERROR_INCLUDED
#define CMINOR_MACHINE_ERROR_INCLUDED
#include <cminor/machine/Unicode.hpp>
#include <Cm.Parsing/Scanner.hpp>
#include <stdexcept>

namespace cminor { namespace machine {

using Cm::Parsing::Span;

#ifdef NDEBUG

#define Assert(expression, message) ((void)0)

#else

#define Assert(expression, message) if (!(expression)) throw std::runtime_error((message))

#endif

std::string Expand(const std::string& errorMessage, const Span& span);
std::string Expand(const std::string& errorMessage, const Span& primarySpan, const Span& referenceSpan);
std::string Expand(const std::string& errorMessage, const Span& span, const std::vector<Span>& references);

class Exception : public std::runtime_error
{
public:
    Exception(const std::string& message_, const Span& defined_);
    Exception(const std::string& message_, const Span& defined_, const Span& referenced_);
    const std::string& Message() const { return message; }
    const Span& Defined() const { return defined; }
    const Span& Referenced() const { return referenced; }
private:
    std::string message;
    Span defined;
    Span referenced;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_ERROR_INCLUDED
