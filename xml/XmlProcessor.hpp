// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_XML_XML_PROCESSOR
#define CMINOR_XML_XML_PROCESSOR
#include <cminor/xml/XmlContentHandler.hpp>
#include <cminor/parsing/Scanner.hpp>
#include <memory>
#include <unordered_map>
#include <string>
#include <stdexcept>

namespace cminor { namespace xml {

class XmlProcessingException : public std::runtime_error
{
public:
    XmlProcessingException(const std::string& message);
};

class Entity
{
public:
    virtual ~Entity();
    virtual bool IsInternalEntity() const { return false; }
};

class InternalEntity : public Entity
{
public:
    InternalEntity(const std::u32string& value_);
    const std::u32string& Value() const { return value; }
    bool IsInternalEntity() const override { return true; }
private:
    std::u32string value;
};

class XmlProcessor
{
public:
    XmlProcessor(const char32_t* contentStart_, const char32_t* contentEnd_, XmlContentHandler* contentHandler_);
    const char32_t* ContentStart() const { return contentStart; }
    const char32_t* ContentEnd() const { return contentEnd; }
    void StartDocument();
    void EndDocument();
    void Text(const std::u32string& text);
    void Comment(const std::u32string& text);
    void PI(const std::u32string& target, const std::u32string& data);
    void Version(const std::u32string& version);
    void Standalone(bool standalone);
    void Encoding(const std::u32string& encoding);
    void BeginStartTag(const std::u32string& tagName);
    void EndStartTag(const cminor::parsing::Span& span, const std::string& systemId);
    void EndTag(const std::u32string& tagName, const cminor::parsing::Span& span, const std::string& systemId);
    void AddAttribute(const std::u32string& attName, const std::u32string& attValue, const cminor::parsing::Span& span, const std::string& systemId);
    void EntityRef(const std::u32string& entityName, const cminor::parsing::Span& span, const std::string& systemId);
    void BeginAttributeValue(std::u32string* attValue_);
    void EndAttributeValue();
private:
    const char32_t* contentStart;
    const char32_t* contentEnd;
    XmlContentHandler* contentHandler;
    std::stack<std::u32string> tagStack;
    std::u32string currentTagName;
    std::stack<std::u32string*> attValueStack;
    std::u32string* attValue;
    std::stack<std::u32string> namespaceUriStack;
    std::u32string currentNamespaceUri;
    std::stack<std::u32string> namespacePrefixStack;
    std::u32string currentNamespacePrefix;
    std::unordered_map<std::u32string, std::u32string> namespacePrefixMap;
    Attributes attributes;
    std::unordered_map<std::u32string, std::unique_ptr<Entity>> entityMap;
    int GetErrorColumn(int index) const;
    std::string GetErrorLocationStr(const std::string& systemId, const cminor::parsing::Span& span) const;
    void ParseQualifiedName(const std::u32string& qualifiedName, std::u32string& localName, std::u32string& prefix, const cminor::parsing::Span& span, const std::string& systemId);
    std::u32string GetNamespaceUri(const std::u32string& namespacePrefix, const cminor::parsing::Span& span, const std::string& systemId);
};

} } // namespace cminor::xml

#endif // CMINOR_XML_XML_PROCESSOR
