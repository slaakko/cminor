/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#ifndef CMINOR_SYNTAX_LIBRARY_INCLUDED
#define CMINOR_SYNTAX_LIBRARY_INCLUDED

#include <cminor/syntax/Project.hpp>
#include <cminor/parsing/ParsingDomain.hpp>

namespace cminor { namespace syntax {

void GenerateLibraryFile(Project* project, cminor::parsing::ParsingDomain* parsingDomain);

} } // namespace cpg::syntax

#endif // CMINOR_SYNTAX_LIBRARY_INCLUDED
