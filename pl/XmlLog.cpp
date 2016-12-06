/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#include <cminor/pl/XmlLog.hpp>
#include <cminor/pl/Utility.hpp>
#include <algorithm>

namespace cminor { namespace parsing {

void XmlLog::WriteElement(const std::string& elementName, const std::string& elementContent)
{
    std::string converted = XmlEscape(elementContent);
    int convertedLength = int(converted.length());
    int lineLength = indent + 2 * int(elementName.length()) + 5 + convertedLength;
    std::string s = converted;
    if (lineLength > maxLineLength)
    {
        lineLength += 3;
        s = converted.substr(0, std::max(0, convertedLength - (lineLength - maxLineLength))) + "...";
    }
    Write("<" + elementName + ">" + s + "</" + elementName + ">");
}

void XmlLog::Write(const std::string& s)
{
    if (indent > 0)
    {
        stream << std::string(indent, ' ');
    }
    stream << s << std::endl;
}

} } // namespace Cm::Parsing
