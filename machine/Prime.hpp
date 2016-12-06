/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#ifndef CMINOR_MACHINE_PRIME_INCLUDED
#define CMINOR_MACHINE_PRIME_INCLUDED
#include <stdint.h>

namespace cminor { namespace machine {

// Returns smallest prime greater than or equal to x

uint64_t NextPrime(uint64_t x);

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_PRIME_INCLUDED


