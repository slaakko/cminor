// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/pl/ParsingData.hpp>

namespace cminor { namespace parsing {

Context::~Context()
{
}

ParsingData::ParsingData(int numRules)
{
    ruleData.resize(numRules);
}

} } // namespace cminor::parsing
