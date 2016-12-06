/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

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

} } // namespace Cm::Parsing

#endif // CMINOR_PARSING_NAMESPACE_INCLUDED
