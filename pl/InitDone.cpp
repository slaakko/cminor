// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

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

