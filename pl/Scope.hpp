/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#ifndef CMINOR_PARSING_SCOPE_INCLUDED
#define CMINOR_PARSING_SCOPE_INCLUDED
#include <cminor/pl/ParsingObject.hpp>
#include <unordered_map>

namespace cminor { namespace parsing {

class Namespace;

class Scope: public ParsingObject
{
public:
    Scope(const std::string& name_, Scope* enclosingScope_);
    virtual std::string FullName() const;
    virtual void Accept(Visitor& visitor);
    void SetNs(Namespace* ns_) { ns = ns_; }
    Namespace* Ns() const { return ns; }
    void Add(ParsingObject* object);
    void AddNamespace(Namespace* ns);
    ParsingObject* Get(const std::string& objectName) const;
    Namespace* GetNamespace(const std::string& fullNamespaceName) const;
private:
    Namespace* ns;
    mutable bool fullNameComputed;
    mutable std::string fullName;
    typedef std::unordered_map<std::string, ParsingObject*> FullNameMap;
    typedef FullNameMap::const_iterator FullNameMapIt;
    FullNameMap fullNameMap;
    typedef std::unordered_map<std::string, ParsingObject*> ShortNameMap;
    typedef ShortNameMap::const_iterator ShortNameMapIt;
    ShortNameMap shortNameMap;
    ParsingObject* GetQualifiedObject(const std::string& qualifiedObjectName) const;
    Scope* GetGlobalScope() const;
};

} } // namespace cminor::parsing

#endif // CMINOR_PARSING_SCOPE_INCLUDED

