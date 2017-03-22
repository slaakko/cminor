// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_PARSING_SCANNER_INCLUDED
#define CMINOR_PARSING_SCANNER_INCLUDED
#include <cminor/pl/ParsingData.hpp>
#include <memory>
#include <string>
#include <stdint.h>
#include <vector>
#include <stack>

namespace cminor { namespace parsing {

class Rule;

void SetCountSourceLines(bool count);
int GetParsedSourceLines();
void SetCC(bool cc_);
bool CC();

class Span
{
public:
    Span(): fileIndex(-1), lineNumber(-1), start(-1), end(-1) {}
    Span(int32_t fileIndex_): fileIndex(fileIndex_), lineNumber(1), start(0), end(0) {}
    Span(int32_t fileIndex_, int32_t lineNumber_, int32_t start_): fileIndex(fileIndex_), lineNumber(lineNumber_), start(start_), end(start + 1) {}
    Span(int32_t fileIndex_, int32_t lineNumber_, int32_t start_, int32_t end_): fileIndex(fileIndex_), lineNumber(lineNumber_), start(start_), end(end_) {}
    int32_t FileIndex() const { return fileIndex; }
    void SetFileIndex(int32_t fileIndex_) { fileIndex = fileIndex_; }
    int32_t LineNumber() const { return lineNumber; }
    int32_t Start() const { return start; }
    int32_t End() const { return end; }
    bool Valid() const { return start != -1; }
    void IncLineNumber() { ++lineNumber; }
    void operator++()
    {
        ++start;
        ++end;
    }
    void operator--()
    {
        --start;
        --end;
    }
    void SetEnd(int32_t end_) { end = end_; }
    bool IsNull() const { return fileIndex == 0 && lineNumber == 0 && start == 0 && end == 0; }
private:
    int32_t fileIndex;
    int32_t lineNumber;
    int32_t start;
    int32_t end;
};

std::string NarrowString(const char* start, const char* end);

class Parser;
class XmlLog;
class ExpectationFailure;

class Scanner
{
public:
    Scanner(const char* start_, const char* end_, const std::string& fileName_, int fileIndex_, Parser* skipper_);
    const char* Start() const { return start; }
    const char* End() const { return end; }
    char GetChar() const { return start[span.Start()]; }
    void operator++();
    bool AtEnd() const { return start + span.Start() >= end; }
    void BeginToken() { ++tokenCounter; }
    void EndToken() { --tokenCounter; }
    void Skip();
    const std::string& FileName() const { return fileName; }
    const Span& GetSpan() const { return span; }
    void SetSpan(const Span& span_) { span = span_; synchronizing = false; }
    bool Skipping() const { return skipping; }
    XmlLog* Log() const { return log; }
    void SetLog(XmlLog* log_) { log = log_; }
    int LineEndIndex();
    std::string RestOfLine();
    void SetSynchronizing(bool synchronizing_) { synchronizing = synchronizing_; }
    bool Synchronizing() const { return synchronizing; }
    void PushCCRule(Rule* ccRule_);
    void PopCCRule();
    void SetParsingData(ParsingData* parsingData_) { parsingData = parsingData_; }
private:
    const char* start;
    const char* end;
    Parser* skipper;
    bool skipping;
    int tokenCounter;
    std::string fileName;
    Span span;
    XmlLog* log;
    bool atBeginningOfLine;
    bool synchronizing;
    Rule* ccRule;
    std::stack<Rule*> ccRuleStack;
    ParsingData* parsingData;
};

} } // namespace cminor::parsing

#endif // CMINOR_PARSING_SCANNER_INCLUDED
