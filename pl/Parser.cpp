/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#include <cminor/pl/Parser.hpp>

namespace cminor { namespace parsing {

Object::~Object()
{
}

Parser::Parser(const std::string& name_, const std::string& info_): ParsingObject(name_), info(info_)
{
}

} } // namespace cminor::parsing
