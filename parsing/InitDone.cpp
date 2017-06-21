// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/parsing/InitDone.hpp>
#include <cminor/parsing/ParsingDomain.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/codedom/InitDone.hpp>

namespace cminor { namespace parsing {

void Init()
{
    cminor::codedom::Init();
    ParsingDomainInit();
    KeywordInit();
}

void Done()
{
    KeywordDone();
    ParsingDomainDone();
    cminor::codedom::Done();
}

} } // namespace cminor::parsing

