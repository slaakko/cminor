// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Error.hpp>
#include <cminor/machine/FileRegistry.hpp>
#include <cminor/machine/MappedInputFile.hpp>
#include <cminor/pl/Exception.hpp>

namespace cminor {namespace machine {

std::string Expand(const std::string& errorMessage, const Span& span)
{
    std::vector<Span> references;
    return Expand(errorMessage, span, references);
}

std::string Expand(const std::string& errorMessage, const Span& primarySpan, const Span& referenceSpan)
{
    std::vector<Span> references(1, referenceSpan);
    return Expand(errorMessage, primarySpan, references, "Error");
}

std::string Expand(const std::string& errorMessage, const Span& primarySpan, const Span& referenceSpan, const std::string& title)
{
    std::vector<Span> references(1, referenceSpan);
    return Expand(errorMessage, primarySpan, references, title);
}

std::string Expand(const std::string& errorMessage, const Span& span, const std::vector<Span>& references)
{
    return Expand(errorMessage, span, references, "Error");
}

std::string Expand(const std::string& errorMessage, const Span& span, const std::vector<Span>& references, const std::string& title)
{
    std::string expandedMessage = title + ": " + errorMessage;
    if (span.Valid())
    {
        FileRegistry* currentFileRegistry = FileRegistry::Instance();
        if (currentFileRegistry)
        {
            const std::string& fileName = currentFileRegistry->GetParsedFileName(span.FileIndex());
            if (!fileName.empty())
            {
                expandedMessage.append(" (file '" + fileName + "', line " + std::to_string(span.LineNumber()) + ")");
                MappedInputFile file(fileName);
                expandedMessage.append(":\n").append(cminor::parsing::GetErrorLines(file.Begin(), file.End(), span));
            }
            for (const Span& referenceSpan : references)
            {
                const std::string& fileName = currentFileRegistry->GetParsedFileName(referenceSpan.FileIndex());
                if (!fileName.empty())
                {
                    expandedMessage.append("\nsee reference to file '" + fileName + "', line " + std::to_string(referenceSpan.LineNumber()));
                    MappedInputFile file(fileName);
                    expandedMessage.append(":\n").append(cminor::parsing::GetErrorLines(file.Begin(), file.End(), referenceSpan));
                }
            }
        }
    }
    return expandedMessage;
}

Exception::Exception(const std::string& message_, const Span& defined_) : std::runtime_error(Expand(message_, defined_)), message(message_), defined(defined_)
{
}

Exception::Exception(const std::string& message_, const Span& defined_, const Span& referenced_) : std::runtime_error(Expand(message_, defined_, referenced_)), message(message_), defined(defined_)
{
    references.push_back(referenced_);
}

Exception::Exception(const std::string& message_, const Span& defined_, const std::vector<Span>& references_) : std::runtime_error(Expand(message_, defined_, references_)), message(message_),
    defined(defined_), references(references_)
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

MachineException::MachineException(const std::string& message_) : std::runtime_error(message_)
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

} } // namespace cminor::machine