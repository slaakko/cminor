// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/util/InitDone.hpp>
#include <cminor/util/Unicode.hpp>

namespace cminor { namespace util {

void Init()
{
    cminor::unicode::UnicodeInit();
}

void Done()
{
    cminor::unicode::UnicodeDone();
}

} } // namespace cminor::util

