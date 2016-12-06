/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#ifndef CPG_SYNTAX_COMPILER_INCLUDED
#define CPG_SYNTAX_COMPILER_INCLUDED

#include <string>
#include <vector>

namespace cpg { namespace syntax {

void Compile(const std::string& projectFilePath, const std::vector<std::string>& libraryDirectories);

} } // namespace cpg::syntax

#endif // CPG_SYNTAX_COMPILER_INCLUDED

