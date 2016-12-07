/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#include <cminor/pl/InitDone.hpp>
#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/pom/InitDone.hpp>

namespace cminor { namespace parsing {

void Init()
{
    cminor::pom::Init();
    ParsingDomainInit();
    KeywordInit();
}

void Done()
{
    KeywordDone();
    ParsingDomainDone();
    cminor::pom::Done();
}

} } // namespace cminor::parsing

