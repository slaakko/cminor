// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_UTIL_CODE_FORMATTER_INCLUDED
#define CMINOR_UTIL_CODE_FORMATTER_INCLUDED
#include <ostream>

namespace cminor { namespace util {

class CodeFormatter
{
public:
    CodeFormatter(std::ostream& stream_) : stream(stream_), indent(0), indentSize(4), atBeginningOfLine(true), line(1) {}
    int Indent() const { return indent; }
    int IndentSize() const { return indentSize; }
    int& IndentSize() { return indentSize; }
    int CurrentIndent() const { return indentSize * indent; }
    void Write(const std::string& text);
    void WriteLine(const std::string& text);
    void NewLine();
    void WriteLine() { NewLine(); }
    void IncIndent()
    {
        ++indent;
    }
    void DecIndent()
    {
        --indent;
    }
    int Line() const { return line; }
    void SetLine(int line_) { line = line_; }
private:
    std::ostream& stream;
    int indent;
    int indentSize;
    bool atBeginningOfLine;
    int line;
};

} } // namespace cminor::util

#endif // CMINOR_UTIL_CODE_FORMATTER_INCLUDED
