/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#include <cminor/pl/Exception.hpp>
#include <cminor/machine/MappedInputFile.hpp>
#include <algorithm>

namespace cminor { namespace parsing {

std::vector<std::string> files;

void SetFileInfo(const std::vector<std::string>& files_)
{
    files = files_;
}

const char* LineStart(const char* start, const char* pos)
{
    while (pos > start && *pos != '\n')
    {
        --pos;
    }
    if (*pos == '\n')
    {
        ++pos;
    }
    return pos;
}

const char* LineEnd(const char* end, const char* pos)
{
    while (pos < end && *pos != '\n')
    {
        ++pos;
    }
    return pos;
}

std::string GetErrorLines(const char* start, const char* end, const Span& span)
{
    const char* startPos = start + span.Start();
    if (startPos < start || startPos >= end)
    {
        return std::string();
    }
    const char* lineStart = LineStart(start, startPos);
    int cols = static_cast<int>(startPos - lineStart);
    if (cols < 0)
    {
        cols = 0;
    }
    const char* lineEnd = LineEnd(end, startPos);
    if (lineEnd < lineStart)
    {
        lineEnd = lineStart;
    }
    int lineLength = static_cast<int>(lineEnd - lineStart);
    std::string lines(NarrowString(lineStart, lineEnd));
    int spanCols = std::max(1, std::min(span.End() -  span.Start(), lineLength - cols));
    lines.append(1, '\n').append(std::string(cols, ' ')).append(spanCols, '^');
    return lines;
}

void ThrowException(const std::string& message, const Span& span)
{
    if (span.FileIndex() >= 0 && span.FileIndex() < int(files.size()))
    {
        const std::string& filePath = files[span.FileIndex()];
        cminor::machine::MappedInputFile file(filePath);
        throw ParsingException(message, filePath, span, file.Begin(), file.End());
    }
    throw std::runtime_error(message);
}

ParsingException::ParsingException(const std::string& message_, const std::string& fileName_, const Span& span_, const char* start_, const char* end_):
    std::runtime_error(message_ + " in file " + fileName_ + " at line " + std::to_string(span_.LineNumber()) + ":\n" + GetErrorLines(start_, end_, span_)), 
    message(message_), fileName(fileName_), span(span_), start(start_), end(end_)
{
}

ExpectationFailure::ExpectationFailure(const std::string& info_, const std::string& fileName_, const Span& span_, const char* start, const char* end):
    ParsingException("parsing failed (" +  info_ + " expected)", fileName_, span_, start, end), info(info_)
{
}

void ExpectationFailure::CombineInfo(const std::string& parentInfo)
{
    info = parentInfo + info;
}

} } // namespace Cm::Parsing
