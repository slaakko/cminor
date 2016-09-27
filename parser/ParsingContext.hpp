// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_PARSING_CONTEXT_INCLUDED
#define CMINOR_PARSING_CONTEXT_INCLUDED
#include <stack>

namespace cminor { namespace parser {

class ParsingContext
{
public:
    ParsingContext();
    bool ParsingSimpleStatement() const { return parsingSimpleStatement; }
    void PushParsingSimpleStatement(bool enable);
    void PopParsingSimpleStatement();
    bool ParsingLvalue() const { return parsingLvalue; }
    void PushParsingLvalue(bool enable);
    void PopParsingLvalue();
    bool ParsingArguments() const { return parsingArguments; }
    void BeginParsingArguments();
    void EndParsingArguments();
private:
    bool parsingSimpleStatement;
    std::stack<bool> parsingSimpleStatementStack;
    bool parsingLvalue;
    std::stack<bool> parsingLvalueStack;
    bool parsingArguments;
    std::stack<bool> parsingArgumentsStack;
};

} } // namespace cminor::parser

#endif // CMINOR_PARSING_CONTEXT_INCLUDED
