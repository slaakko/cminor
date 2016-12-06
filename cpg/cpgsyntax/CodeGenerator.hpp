/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#ifndef CPG_SYNTAX_CODEGENERATOR_INCLUDED
#define CPG_SYNTAX_CODEGENERATOR_INCLUDED

#include <cminor/cpg/cpgsyntax/ParserFileContent.hpp>

namespace cpg { namespace syntax {

void SetForceCodeGen(bool force);
void GenerateCode(const std::vector<std::unique_ptr<ParserFileContent>>& parserFiles);

} } // namespace cpg::syntax

#endif // CPG_SYNTAX_CODEGENERATOR_INCLUDED
