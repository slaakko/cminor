// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_ERROR_INCLUDED
#define CMINOR_MACHINE_ERROR_INCLUDED
#include <cminor/machine/Unicode.hpp>
#include <cminor/pl/Scanner.hpp>
#include <stdexcept>

namespace cminor { namespace machine {

using cminor::parsing::Span;

#ifdef NDEBUG

#define Assert(expression, message) ((void)0)

#else

#define Assert(expression, message) if (!(expression)) throw std::runtime_error(std::string("assertion failed: ") + message)

#endif

std::string Expand(const std::string& errorMessage, const Span& span);
std::string Expand(const std::string& errorMessage, const Span& primarySpan, const Span& referenceSpan);
std::string Expand(const std::string& errorMessage, const Span& span, const std::vector<Span>& references);

class Exception : public std::runtime_error
{
public:
    Exception(const std::string& message_, const Span& defined_);
    Exception(const std::string& message_, const Span& defined_, const Span& referenced_);
    Exception(const std::string& message_, const Span& defined_, const std::vector<Span>& references_);
    const std::string& Message() const { return message; }
    const Span& Defined() const { return defined; }
    const std::vector<Span>& References() const { return references; }
private:
    std::string message;
    Span defined;
    std::vector<Span> references;
};

class MachineException : public std::runtime_error
{
public:
    MachineException(const std::string& message_);
};

class SystemException : public MachineException
{
public:
    SystemException(const std::string& message_);
};

class NullReferenceException : public SystemException
{
public:
    NullReferenceException(const std::string& message_);
};

class IndexOutOfRangeException : public SystemException
{
public:
    IndexOutOfRangeException(const std::string& message_);
};

class ArgumentException : public SystemException
{
public:
    ArgumentException(const std::string& message_);
};

class ArgumentOutOfRangeException : public ArgumentException
{
public:
    ArgumentOutOfRangeException(const std::string& message_);
};

class InvalidCastException : public SystemException
{
public:
    InvalidCastException(const std::string& message_);
};

class FileSystemError : public SystemException
{
public:
    FileSystemError(const std::string& errorMessage);
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_ERROR_INCLUDED
