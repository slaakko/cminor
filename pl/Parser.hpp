// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_PARSING_PARSER_INCLUDED
#define CMINOR_PARSING_PARSER_INCLUDED
#include <cminor/pl/Match.hpp>
#include <cminor/pl/ParsingData.hpp>
#include <cminor/pl/ParsingObject.hpp>
#include <stack>

namespace cminor { namespace parsing {

class Object
{
public:
    virtual ~Object();
};

typedef std::stack<std::unique_ptr<Object>> ObjectStack;

template<typename ValueType>
class ValueObject: public Object
{
public:
    ValueObject(): value() {}
    ValueObject(const ValueType& value_): value(value_) {}
    ValueObject<ValueType>& operator=(const ValueType& v) { value = v; return *this; }
    operator ValueType() const { return value; }
private:
    ValueType value;
};

class Scanner;

class Parser: public ParsingObject
{
public:
    Parser(const std::string& name_, const std::string& info_);
    const std::string& Info() const { return info; }
    virtual Match Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData) = 0;
private:
    std::string info;
};

} } // namespace cminor::parsing

#endif // CMINOR_PARSING_PARSER_INCLUDED

