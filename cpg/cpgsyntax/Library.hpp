/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#ifndef CPG_SYNTAX_LIBRARY_INCLUDED
#define CPG_SYNTAX_LIBRARY_INCLUDED

#include <cminor/cpg/cpgsyntax/Project.hpp>
#include <cminor/pl/ParsingDomain.hpp>

namespace cpg { namespace syntax {

void GenerateLibraryFile(Project* project, cminor::parsing::ParsingDomain* parsingDomain);

} } // namespace cpg::syntax

#endif // CPG_SYNTAX_LIBRARY_INCLUDED
