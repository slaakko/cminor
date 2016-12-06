/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#include <cminor/pom/Object.hpp>

namespace cminor { namespace pom {

CppObject::CppObject(const std::string& name_): name(name_), isOwned(false)
{
}

CppObject::~CppObject()
{
}

void CppObject::Own(CppObject* object)
{
    if (object)
    {
        if (!object->IsOwned())
        {
            object->SetOwned();
            ownedObjects.push_back(std::unique_ptr<CppObject>(object));
        }
    }
}

std::string CppObject::ToString() const
{
    return name;
}

void CppObject::Print(CodeFormatter& formatter)
{
    formatter.Write(name);
}

} } // namespace cminor::pom
