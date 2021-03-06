// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_PARSING_XMLLOG_INCLUDED
#define CMINOR_PARSING_XMLLOG_INCLUDED
#include <ostream>
#include <string>

namespace cminor { namespace parsing {

class XmlLog
{
public:
    XmlLog(std::ostream& stream_, int maxLineLength_): stream(stream_), maxLineLength(maxLineLength_), indent(0) {}
    void IncIndent() { ++indent; }
    void DecIndent() { --indent; }
    void WriteBeginRule(const std::string& ruleName) { Write("<" + ruleName + ">"); }
    void WriteEndRule(const std::string& ruleName) { Write("</" + ruleName + ">"); }
    void WriteTry(const std::string& s) { WriteElement("try", s); }
    void WriteSuccess(const std::string& match) { WriteElement("success", match); }
    void WriteFail() { Write("<fail/>"); }
    void WriteElement(const std::string& elementName, const std::string& elementContent);
    void Write(const std::string& s);
private:
    std::ostream& stream;
    int maxLineLength;
    int indent;
};

} } // namespace cminor::parsing

#endif // CMINOR_PARSING_XMLLOG_INCLUDED
