// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYNTAX_CODEGENERATOR_VISITOR_INCLUDED
#define CMINOR_SYNTAX_CODEGENERATOR_VISITOR_INCLUDED

#include <cminor/parsing/Primitive.hpp>
#include <cminor/parsing/Composite.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/parsing/Visitor.hpp>
#include <cminor/util/CodeFormatter.hpp>

namespace cminor { namespace syntax {

using cminor::util::CodeFormatter;
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
    virtual void Visit(UpperLetterParser& parser);
    virtual void Visit(LowerLetterParser& parser);
    virtual void Visit(TitleLetterParser& parser);
    virtual void Visit(ModifierLetterParser& parser);
    virtual void Visit(OtherLetterParser& parser);
    virtual void Visit(CasedLetterParser& parser);
    virtual void Visit(DigitParser& parser);
    virtual void Visit(HexDigitParser& parser);
    virtual void Visit(MarkParser& parser);
    virtual void Visit(NonspacingMarkParser& parser);
    virtual void Visit(SpacingMarkParser& parser);
    virtual void Visit(EnclosingMarkParser& parser);
    virtual void Visit(NumberParser& parser);
    virtual void Visit(DecimalNumberParser& parser);
    virtual void Visit(LetterNumberParser& parser);
    virtual void Visit(OtherNumberParser& parser);
    virtual void Visit(PunctuationParser& parser);
    virtual void Visit(ConnectorPunctuationParser& parser);
    virtual void Visit(DashPunctuationParser& parser);
    virtual void Visit(OpenPunctuationParser& parser);
    virtual void Visit(ClosePunctuationParser& parser);
    virtual void Visit(InitialPunctuationParser& parser);
    virtual void Visit(FinalPunctuationParser& parser);
    virtual void Visit(OtherPunctuationParser& parser);
    virtual void Visit(SymbolParser& parser);
    virtual void Visit(MathSymbolParser& parser);
    virtual void Visit(CurrencySymbolParser& parser);
    virtual void Visit(ModifierSymbolParser& parser);
    virtual void Visit(OtherSymbolParser& parser);
    virtual void Visit(SeparatorParser& parser);
    virtual void Visit(SpaceSeparatorParser& parser);
    virtual void Visit(LineSeparatorParser& parser);
    virtual void Visit(ParagraphSeparatorParser& parser);
    virtual void Visit(OtherParser& parser);
    virtual void Visit(ControlParser& parser);
    virtual void Visit(FormatParser& parser);
    virtual void Visit(SurrogateParser& parser);
    virtual void Visit(PrivateUseParser& parser);
    virtual void Visit(UnassignedParser& parser);
    virtual void Visit(GraphicParser& parser);
    virtual void Visit(BaseCharParser& parser);
    virtual void Visit(AlphabeticParser& parser);
    virtual void Visit(IdStartParser& parser);
    virtual void Visit(IdContParser& parser);
    virtual void Visit(AnyCharParser& parser);
    virtual void Visit(RangeParser& parser);
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

} } // namespace cminor::syntax

#endif // CMINOR_SYNTAX_CODEGENERATOR_VISITOR_INCLUDED
