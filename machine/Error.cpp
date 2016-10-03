// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Error.hpp>
#include <cminor/machine/FileRegistry.hpp>
#include <cminor/machine/MappedInputFile.hpp>
#include <Cm.Parsing/Exception.hpp>

namespace cminor {namespace machine {

std::string Expand(const std::string& errorMessage, const Span& span)
{
    std::vector<Span> references;
    return Expand(errorMessage, span, references);
}

std::string Expand(const std::string& errorMessage, const Span& primarySpan, const Span& referenceSpan)
{
    std::vector<Span> references(1, referenceSpan);
    return Expand(errorMessage, primarySpan, references);
}

std::string Expand(const std::string& errorMessage, const Span& span, const std::vector<Span>& references)
{
    std::string expandedMessage = "Error: " + errorMessage;
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
                expandedMessage.append(":\n").append(Cm::Parsing::GetErrorLines(file.Begin(), file.End(), span));
            }
            for (const Span& referenceSpan : references)
            {
                const std::string& fileName = currentFileRegistry->GetParsedFileName(referenceSpan.FileIndex());
                if (!fileName.empty())
                {
                    expandedMessage.append("\nsee reference to file '" + fileName + "', line " + std::to_string(referenceSpan.LineNumber()));
                    MappedInputFile file(fileName);
                    expandedMessage.append(":\n").append(Cm::Parsing::GetErrorLines(file.Begin(), file.End(), referenceSpan));
                }
            }
        }
    }
    return expandedMessage;
}

Exception::Exception(const std::string& message_, const Span& defined_) : std::runtime_error(Expand(message_, defined_)), message(message_), defined(defined_)
{
}

Exception::Exception(const std::string& message_, const Span& defined_, const Span& referenced_) : std::runtime_error(Expand(message_, defined_, referenced_)), message(message_), defined(defined_), referenced(referenced_)
{
}

} } // namespace cminor::machine