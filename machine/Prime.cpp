/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#include <cminor/machine/Prime.hpp>

namespace cminor { namespace machine {
        
bool IsPrime(uint64_t x)
{
    uint64_t i = 3;
    while (true)
    {
        uint64_t q = x / i;
        if (q < i)
        {
            return true;
        }
        if (x == q * i)
        {
            return false;
        }
        i += 2;
    }
}

uint64_t NextPrime(uint64_t x)
{
    if (x <= 2)
    {
        return 2;
    }
    if ((x & 1) == 0)
    {
        ++x;
    }
    while (!IsPrime(x))
    {
        x += 2;
    }
    return x;
}

} } // namespace cminor::machine
