// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/xml/XmlParser.hpp>
#include <cminor/xml/XmlGrammar.hpp>
#include <cminor/util/Unicode.hpp>
#include <cminor/util/MappedInputFile.hpp>

namespace cminor { namespace xml {

using namespace cminor::util;
using namespace cminor::unicode;

void ParseXmlFile(const std::string& xmlFileName, XmlContentHandler* contentHandler)
{
    std::string xmlContent = ReadFile(xmlFileName);
    ParseXmlContent(xmlContent, xmlFileName, contentHandler);
}

void ParseXmlContent(const std::string& xmlContent, const std::string& systemId, XmlContentHandler* contentHandler)
{
    ParseXmlContent(ToUtf32(xmlContent), systemId, contentHandler);
}

XmlGrammar* xmlGrammar = nullptr;

void ParseXmlContent(const std::u32string& xmlContent, const std::string& systemId, XmlContentHandler* contentHandler)
{
    const char32_t* contentStart = &xmlContent[0];
    const char32_t* contentEnd = &xmlContent[0] + xmlContent.length();
    XmlProcessor xmlProcessor(contentStart, contentEnd, contentHandler);
    if (xmlGrammar == nullptr)
    {
        xmlGrammar = XmlGrammar::Create();
    }
    xmlGrammar->Parse(contentStart, contentEnd, 0, systemId, &xmlProcessor);
}

} } // namespace cminor::xml
