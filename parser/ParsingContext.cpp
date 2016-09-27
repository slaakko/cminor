// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

ParsingContext::ParsingContext() : parsingSimpleStatement(false), parsingLvalue(false), parsingArguments(false)
{
}

void ParsingContext::PushParsingSimpleStatement(bool enable)
{
    parsingSimpleStatementStack.push(parsingSimpleStatement);
    parsingSimpleStatement = enable;
}

void ParsingContext::PopParsingSimpleStatement()
{
    parsingSimpleStatement = parsingSimpleStatementStack.top();
    parsingSimpleStatementStack.pop();
}

void ParsingContext::PushParsingLvalue(bool enable)
{
    parsingLvalueStack.push(parsingLvalue);
    parsingLvalue = enable;
}

void ParsingContext::PopParsingLvalue()
{
    parsingLvalue = parsingLvalueStack.top();
    parsingLvalueStack.pop();
}

void ParsingContext::BeginParsingArguments()
{
    parsingArgumentsStack.push(parsingArguments);
    parsingArguments = true;
}

void ParsingContext::EndParsingArguments()
{
    parsingArguments = parsingArgumentsStack.top(); 
    parsingArgumentsStack.pop();
}

} } // namespace cminor::parser
