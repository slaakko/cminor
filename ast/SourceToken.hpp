// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_SOURCE_TOKEN_INCLUDED
#define CMINOR_AST_SOURCE_TOKEN_INCLUDED
#include <string>

namespace cminor { namespace ast {

class SourceTokenFormatter
{
public:
    virtual ~SourceTokenFormatter();
    virtual void BeginFormat() {}
    virtual void EndFormat() {}
    virtual void Keyword(const std::string& token) {}
    virtual void Identifier(const std::string& token) {}
    virtual void Number(const std::string& token) {}
    virtual void Char(const std::string& token) {}
    virtual void String(const std::string& token) {}
    virtual void Spaces(const std::string& token) {}
    virtual void Comment(const std::string& token) {}
    virtual void NewLine(const std::string& token) {}
    virtual void Other(const std::string& token) {}
};

} } // namespace cminor::ast

#endif // CMINOR_AST_SOURCE_TOKEN_INCLUDED
