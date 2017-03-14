// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_ERROR_INCLUDED
#define CMINOR_MACHINE_ERROR_INCLUDED
#include <cminor/machine/MachineApi.hpp>
#include <cminor/util/Error.hpp>
#include <cminor/util/Unicode.hpp>
#include <cminor/pl/Scanner.hpp>

namespace cminor { namespace machine {

using cminor::parsing::Span;

MACHINE_API std::string Expand(const std::string& errorMessage, const Span& span);
MACHINE_API std::string Expand(const std::string& errorMessage, const Span& primarySpan, const Span& referenceSpan);
MACHINE_API std::string Expand(const std::string& errorMessage, const Span& primarySpan, const Span& referenceSpan, const std::string& title);
MACHINE_API std::string Expand(const std::string& errorMessage, const Span& span, const std::vector<Span>& references);
MACHINE_API std::string Expand(const std::string& errorMessage, const Span& span, const std::vector<Span>& references, const std::string& title);

class MACHINE_API Exception
{
public:
    Exception(const std::string& message_, const Span& defined_);
    Exception(const std::string& message_, const Span& defined_, const Span& referenced_);
    Exception(const std::string& message_, const Span& defined_, const std::vector<Span>& references_);
    virtual ~Exception();
    const std::string& What() const { return what; }
    const std::string& Message() const { return message; }
    const Span& Defined() const { return defined; }
    const std::vector<Span>& References() const { return references; }
private:
    std::string what;
    std::string message;
    Span defined;
    std::vector<Span> references;
};

class MACHINE_API CastOverloadException : public Exception
{
public:
    CastOverloadException(const std::string& message_, const Span& defined_);
    CastOverloadException(const std::string& message_, const Span& defined_, const Span& referenced_);
    CastOverloadException(const std::string& message_, const Span& defined_, const std::vector<Span>& references_);
};

class MACHINE_API RefOverloadException : public Exception
{
public:
    RefOverloadException(const std::string& message_, const Span& defined_);
    RefOverloadException(const std::string& message_, const Span& defined_, const Span& referenced_);
    RefOverloadException(const std::string& message_, const Span& defined_, const std::vector<Span>& references_);
};

class MACHINE_API CastException : public Exception
{
public:
    CastException(const std::string& message_, const Span& defined_);
    CastException(const std::string& message_, const Span& defined_, const Span& referenced_);
    CastException(const std::string& message_, const Span& defined_, const std::vector<Span>& references_);
};

class MACHINE_API MachineException 
{
public:
    MachineException(const std::string& message_);
    virtual ~MachineException();
    const std::string& Message() const { return message; }
private:
    std::string message;
};

class MACHINE_API SystemException : public MachineException
{
public:
    SystemException(const std::string& message_);
};

class MACHINE_API NullReferenceException : public SystemException
{
public:
    NullReferenceException(const std::string& message_);
};

class MACHINE_API IndexOutOfRangeException : public SystemException
{
public:
    IndexOutOfRangeException(const std::string& message_);
};

class MACHINE_API ArgumentException : public SystemException
{
public:
    ArgumentException(const std::string& message_);
};

class MACHINE_API ArgumentOutOfRangeException : public ArgumentException
{
public:
    ArgumentOutOfRangeException(const std::string& message_);
};

class MACHINE_API InvalidCastException : public SystemException
{
public:
    InvalidCastException(const std::string& message_);
};

class MACHINE_API FileSystemError : public SystemException
{
public:
    FileSystemError(const std::string& errorMessage);
};

class MACHINE_API StackOverflowException : public SystemException
{
public:
    StackOverflowException();
};

class MACHINE_API ThreadingException : public SystemException
{
public:
    ThreadingException(const std::string& message_);
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_ERROR_INCLUDED
