/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#ifndef CMINOR_POM_LITERAL_INCLUDED
#define CMINOR_POM_LITERAL_INCLUDED
#include <cminor/pom/Object.hpp>

namespace cminor { namespace pom {

class Literal: public CppObject
{
public:
    Literal(const std::string& name_);
    virtual int Rank() const { return 24; }
    virtual void Accept(Visitor& visitor);
};

} } // namespace cminor::pom

#endif // CMINOR_POM_LITERAL_INCLUDED

