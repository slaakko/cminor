/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#include <cminor/pom/Operator.hpp>

namespace cminor { namespace pom {

void Init()
{
    OperatorInit();
}

void Done()
{
    OperatorDone();
}

} } // namespace cminor::pom
