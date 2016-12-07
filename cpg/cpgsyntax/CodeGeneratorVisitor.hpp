// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CPG_SYNTAX_CODEGENERATOR_VISITOR_INCLUDED
#define CPG_SYNTAX_CODEGENERATOR_VISITOR_INCLUDED

#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/pl/Visitor.hpp>
#include <cminor/machine/CodeFormatter.hpp>

namespace cpg { namespace syntax {

using cminor::machine::CodeFormatter;
using namespace cminor::parsing;

class CodeGeneratorVisitor : public cminor::parsing::Visitor
{
public:
    CodeGeneratorVisitor(CodeFormatter& cppFormatter_, CodeFormatter& hppFormatter_);
    virtual void BeginVisit(Grammar& grammar);
    virtual void EndVisit(Grammar& grammar);
    virtual void Visit(CharParser& parser);
    virtual void Visit(StringParser& parser);
    virtual void Visit(CharSetParser& parser);
    virtual void Visit(KeywordParser& parser);
    virtual void Visit(KeywordListParser& parser);
    virtual void Visit(EmptyParser& parser);
    virtual void Visit(SpaceParser& parser);
    virtual void Visit(LetterParser& parser);
    virtual void Visit(DigitParser& parser);
    virtual void Visit(HexDigitParser& parser);
    virtual void Visit(PunctuationParser& parser);
    virtual void Visit(AnyCharParser& parser);
    virtual void Visit(NonterminalParser& parser);
    virtual void Visit(RuleLink& link);
    virtual void BeginVisit(Rule& parser);
    virtual void EndVisit(Rule& parser);
    virtual void BeginVisit(OptionalParser& parser);
    virtual void EndVisit(OptionalParser& parser);
    virtual void BeginVisit(PositiveParser& parser);
    virtual void EndVisit(PositiveParser& parser);
    virtual void BeginVisit(KleeneStarParser& parser);
    virtual void EndVisit(KleeneStarParser& parser);
    virtual void BeginVisit(ActionParser& parser);
    virtual void EndVisit(ActionParser& parser);
    virtual void BeginVisit(ExpectationParser& parser);
    virtual void EndVisit(ExpectationParser& parser);
    virtual void BeginVisit(CCOptParser& parser);
    virtual void EndVisit(CCOptParser& parser);
    virtual void BeginVisit(TokenParser& parser);
    virtual void EndVisit(TokenParser& parser);
    virtual void BeginVisit(SequenceParser& parser);
    virtual void Visit(SequenceParser& parser);
    virtual void EndVisit(SequenceParser& parser);
    virtual void BeginVisit(AlternativeParser& parser);
    virtual void Visit(AlternativeParser& parser);
    virtual void EndVisit(AlternativeParser& parser);
    virtual void BeginVisit(DifferenceParser& parser);
    virtual void Visit(DifferenceParser& parser);
    virtual void EndVisit(DifferenceParser& parser);
    virtual void BeginVisit(IntersectionParser& parser);
    virtual void Visit(IntersectionParser& parser);
    virtual void EndVisit(IntersectionParser& parser);
    virtual void BeginVisit(ExclusiveOrParser& parser);
    virtual void Visit(ExclusiveOrParser& parser);
    virtual void EndVisit(ExclusiveOrParser& parser);
    virtual void BeginVisit(ListParser& parser);
    virtual void Visit(ListParser& parser);
    virtual void EndVisit(ListParser& parser);
private:
    CodeFormatter& cppFormatter;
    CodeFormatter& hppFormatter;
    std::string CharSetStr();
};

} } // namespace cpg::syntax

#endif // CPG_SYNTAX_CODEGENERATOR_VISITOR_INCLUDED
