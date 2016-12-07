/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#ifndef CMINOR_PARSING_VISITOR_INCLUDED
#define CMINOR_PARSING_VISITOR_INCLUDED

namespace cminor { namespace parsing {

class Namespace;
class UsingObject;
class RuleLink;
class CharParser;
class StringParser;
class CharSetParser;
class EmptyParser;
class SpaceParser;
class LetterParser;
class DigitParser;
class HexDigitParser;
class PunctuationParser;
class AnyCharParser;
class OptionalParser;
class PositiveParser;
class KleeneStarParser;
class ActionParser;
class ExpectationParser;
class CCOptParser;
class TokenParser;
class SequenceParser;
class AlternativeParser;
class DifferenceParser;
class ExclusiveOrParser;
class IntersectionParser;
class ListParser;
class Rule;
class NonterminalParser;
class Grammar;
class KeywordParser;
class KeywordListParser;

class Visitor
{
public:
    virtual void Visit(CharParser& parser) {}
    virtual void Visit(StringParser& parser) {}
    virtual void Visit(CharSetParser& parser) {}
    virtual void Visit(EmptyParser& parser) {}
    virtual void Visit(SpaceParser& parser) {}
    virtual void Visit(LetterParser& parser) {}
    virtual void Visit(DigitParser& parser) {}
    virtual void Visit(HexDigitParser& parser) {}
    virtual void Visit(PunctuationParser& parser) {}
    virtual void Visit(AnyCharParser& parser) {}
    virtual void BeginVisit(OptionalParser& parser) {}
    virtual void EndVisit(OptionalParser& parser) {}
    virtual void BeginVisit(PositiveParser& parser) {}
    virtual void EndVisit(PositiveParser& parser) {}
    virtual void BeginVisit(KleeneStarParser& parser) {}
    virtual void EndVisit(KleeneStarParser& parser) {}
    virtual void BeginVisit(ActionParser& parser) {}
    virtual void EndVisit(ActionParser& parser) {}
    virtual void BeginVisit(ExpectationParser& parser) {}
    virtual void EndVisit(ExpectationParser& parser) {}
    virtual void BeginVisit(CCOptParser& parser) {}
    virtual void EndVisit(CCOptParser& parser) {}
    virtual void BeginVisit(TokenParser& parser) {}
    virtual void EndVisit(TokenParser& parser) {}
    virtual void BeginVisit(SequenceParser& parser) {}
    virtual void Visit(SequenceParser& parser) {}
    virtual void EndVisit(SequenceParser& parser) {}
    virtual void BeginVisit(AlternativeParser& parser) {}
    virtual void Visit(AlternativeParser& parser) {}
    virtual void EndVisit(AlternativeParser& parser) {}
    virtual void BeginVisit(DifferenceParser& parser) {}
    virtual void Visit(DifferenceParser& parser) {}
    virtual void EndVisit(DifferenceParser& parser) {}
    virtual void BeginVisit(ExclusiveOrParser& parser) {}
    virtual void Visit(ExclusiveOrParser& parser) {}
    virtual void EndVisit(ExclusiveOrParser& parser) {}
    virtual void BeginVisit(IntersectionParser& parser) {}
    virtual void Visit(IntersectionParser& parser) {}
    virtual void EndVisit(IntersectionParser& parser) {}
    virtual void BeginVisit(ListParser& parser) {}
    virtual void Visit(ListParser& parser) {}
    virtual void EndVisit(ListParser& parser) {}
    virtual void BeginVisit(Rule& parser) {}
    virtual void EndVisit(Rule& parser) {}
    virtual void Visit(NonterminalParser& parser) {}
    virtual void BeginVisit(Grammar& grammar) {}
    virtual void EndVisit(Grammar& grammar) {}
    virtual void Visit(KeywordParser& parser) {}
    virtual void Visit(KeywordListParser& parser) {}
    virtual void BeginVisit(Namespace& ns) {}
    virtual void EndVisit(Namespace& ns) {}
    virtual void Visit(UsingObject& object) {}
    virtual void Visit(RuleLink& link) {}
};

} } // namespace cminor::parsing

#endif // CMINOR_PARSING_VISITOR_INCLUDED
