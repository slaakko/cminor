// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/pl/Parser.hpp>

namespace cminor { namespace parsing {

Object::~Object()
{
}

Parser::Parser(const std::string& name_, const std::string& info_): ParsingObject(name_), info(info_)
{
}

} } // namespace cminor::parsing
