// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_PARSING_NAMESPACE_INCLUDED
#define CMINOR_PARSING_NAMESPACE_INCLUDED
#include <cminor/pl/ParsingObject.hpp>
#include <cminor/pom/Declaration.hpp>

namespace cminor { namespace parsing {

class Namespace: public ParsingObject
{
public:
    Namespace(const std::string& name_, Scope* enclosingScope_);
    virtual void Accept(Visitor& visitor);
    virtual bool IsNamespace() const { return true; }
};

class Scope;

class UsingObject: public ParsingObject
{
public:
    UsingObject(cminor::pom::UsingObject* subject_, Scope* enclosingScope_);
    virtual void Accept(Visitor& visitor);
    cminor::pom::UsingObject* Subject() const { return subject; }
private:
    cminor::pom::UsingObject* subject;
    std::unique_ptr<cminor::pom::UsingObject> ownedSubject;
};

} } // namespace cminor::parsing

#endif // CMINOR_PARSING_NAMESPACE_INCLUDED
