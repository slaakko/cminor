// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_PARSING_NAMESPACE_INCLUDED
#define CMINOR_PARSING_NAMESPACE_INCLUDED
#include <cminor/parsing/ParsingObject.hpp>
#include <cminor/codedom/Declaration.hpp>

namespace cminor { namespace parsing {

class Namespace: public ParsingObject
{
public:
    Namespace(const std::u32string& name_, Scope* enclosingScope_);
    virtual void Accept(Visitor& visitor);
    virtual bool IsNamespace() const { return true; }
};

class Scope;

class UsingObject: public ParsingObject
{
public:
    UsingObject(cminor::codedom::UsingObject* subject_, Scope* enclosingScope_);
    virtual void Accept(Visitor& visitor);
    cminor::codedom::UsingObject* Subject() const { return subject; }
private:
    cminor::codedom::UsingObject* subject;
    std::unique_ptr<cminor::codedom::UsingObject> ownedSubject;
};

} } // namespace cminor::parsing

#endif // CMINOR_PARSING_NAMESPACE_INCLUDED
