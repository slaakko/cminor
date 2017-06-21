// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Error.hpp>
#include <cminor/machine/FileRegistry.hpp>
#include <cminor/util/MappedInputFile.hpp>
#include <cminor/util/Unicode.hpp>
#include <cminor/parsing/Exception.hpp>

namespace cminor {namespace machine {

using namespace cminor::util;
using namespace cminor::unicode;

MACHINE_API std::string Expand(const std::string& errorMessage, const Span& span)
{
    std::vector<Span> references;
    return Expand(errorMessage, span, references);
}

MACHINE_API std::string Expand(const std::string& errorMessage, const Span& primarySpan, const Span& referenceSpan)
{
    std::vector<Span> references(1, referenceSpan);
    return Expand(errorMessage, primarySpan, references, "Error");
}

MACHINE_API std::string Expand(const std::string& errorMessage, const Span& primarySpan, const Span& referenceSpan, const std::string& title)
{
    std::vector<Span> references(1, referenceSpan);
    return Expand(errorMessage, primarySpan, references, title);
}

MACHINE_API std::string Expand(const std::string& errorMessage, const Span& span, const std::vector<Span>& references)
{
    return Expand(errorMessage, span, references, "Error");
}

MACHINE_API std::string Expand(const std::string& errorMessage, const Span& span, const std::vector<Span>& references, const std::string& title)
{
    std::string expandedMessage = title + ": " + errorMessage;
    if (span.Valid())
    {
        const std::string& fileName = FileRegistry::GetParsedFileName(span.FileIndex());
        if (!fileName.empty())
        {
            expandedMessage.append(" (file '" + fileName + "', line " + std::to_string(span.LineNumber()) + ")");
            MappedInputFile file(fileName);
            std::string s(file.Begin(), file.End());
            std::u32string t(ToUtf32(s));
            expandedMessage.append(":\n").append(ToUtf8(cminor::parsing::GetErrorLines(&t[0], &t[0] + t.length(), span)));
        }
        for (const Span& referenceSpan : references)
        {
            const std::string& fileName = FileRegistry::GetParsedFileName(referenceSpan.FileIndex());
            if (!fileName.empty())
            {
                expandedMessage.append("\nsee reference to file '" + fileName + "', line " + std::to_string(referenceSpan.LineNumber()));
                MappedInputFile file(fileName);
                std::string s(file.Begin(), file.End());
                std::u32string t(ToUtf32(s));
                expandedMessage.append(":\n").append(ToUtf8(cminor::parsing::GetErrorLines(&t[0], &t[0] + t.length(), referenceSpan)));
            }
        }
    }
    return expandedMessage;
}

Exception::Exception(const std::string& message_, const Span& defined_) : what(Expand(message_, defined_)), message(message_), defined(defined_)
{
}

Exception::Exception(const std::string& message_, const Span& defined_, const Span& referenced_) : what(Expand(message_, defined_, referenced_)), message(message_), defined(defined_)
{
    references.push_back(referenced_);
}

Exception::Exception(const std::string& message_, const Span& defined_, const std::vector<Span>& references_) : 
    what(Expand(message_, defined_, references_)), message(message_), defined(defined_), references(references_)
{
}

Exception::~Exception()
{
}

CastOverloadException::CastOverloadException(const std::string& message_, const Span& defined_) : Exception(message_, defined_)
{
}

CastOverloadException::CastOverloadException(const std::string& message_, const Span& defined_, const Span& referenced_) : Exception(message_, defined_, referenced_)
{
}

CastOverloadException::CastOverloadException(const std::string& message_, const Span& defined_, const std::vector<Span>& references_) : Exception(message_, defined_, references_)
{
}

RefOverloadException::RefOverloadException(const std::string& message_, const Span& defined_) : Exception(message_, defined_)
{
}

RefOverloadException::RefOverloadException(const std::string& message_, const Span& defined_, const Span& referenced_) : Exception(message_, defined_, referenced_)
{
}

RefOverloadException::RefOverloadException(const std::string& message_, const Span& defined_, const std::vector<Span>& references_) : Exception(message_, defined_, references_)
{
}

CastException::CastException(const std::string& message_, const Span& defined_) : Exception(message_, defined_)
{
}

CastException::CastException(const std::string& message_, const Span& defined_, const Span& referenced_) : Exception(message_, defined_, referenced_)
{
}

CastException::CastException(const std::string& message_, const Span& defined_, const std::vector<Span>& references_) : Exception(message_, defined_, references_)
{
}

MachineException::MachineException(const std::string& message_) : message(message_)
{
}

MachineException::~MachineException()
{
}

SystemException::SystemException(const std::string& message_) : MachineException(message_)
{
}

NullReferenceException::NullReferenceException(const std::string& message_) : SystemException(message_)
{
}

IndexOutOfRangeException::IndexOutOfRangeException(const std::string& message_) : SystemException(message_)
{
}

ArgumentException::ArgumentException(const std::string& message_) : SystemException(message_)
{
}

ArgumentOutOfRangeException::ArgumentOutOfRangeException(const std::string& message_) : ArgumentException(message_)
{
}

InvalidCastException::InvalidCastException(const std::string& message_) : SystemException(message_)
{
}

FileSystemError::FileSystemError(const std::string& errorMessage) : SystemException(errorMessage)
{
}

SocketError::SocketError(const std::string& errorMessage) : SystemException(errorMessage), errorCode(-1)
{
}

StackOverflowException::StackOverflowException() : SystemException("stack overflow")
{
}

ThreadingException::ThreadingException(const std::string& message_) : SystemException(message_)
{
}

CapturedException::CapturedException(uint64_t exception_) : Exception("captured", Span()), exception(exception_)
{
}

} } // namespace cminor::machine