#include "Primitive.hpp"
#include <cminor/parsing/Action.hpp>
#include <cminor/parsing/Rule.hpp>
#include <cminor/parsing/ParsingDomain.hpp>
#include <cminor/parsing/Primitive.hpp>
#include <cminor/parsing/Composite.hpp>
#include <cminor/parsing/Nonterminal.hpp>
#include <cminor/parsing/Exception.hpp>
#include <cminor/parsing/StdLib.hpp>
#include <cminor/parsing/XmlLog.hpp>
#include <cminor/util/Unicode.hpp>
#include <cminor/syntax/Element.hpp>

namespace cminor { namespace syntax {

using namespace cminor::parsing;
using namespace cminor::util;
using namespace cminor::unicode;

PrimitiveGrammar* PrimitiveGrammar::Create()
{
    return Create(new cminor::parsing::ParsingDomain());
}

PrimitiveGrammar* PrimitiveGrammar::Create(cminor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    PrimitiveGrammar* grammar(new PrimitiveGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

PrimitiveGrammar::PrimitiveGrammar(cminor::parsing::ParsingDomain* parsingDomain_): cminor::parsing::Grammar(ToUtf32("PrimitiveGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cminor.syntax")), parsingDomain_)
{
    SetOwner(0);
}

cminor::parsing::Parser* PrimitiveGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName)
{
    cminor::parsing::Scanner scanner(start, end, fileName, fileIndex, SkipRule());
    std::unique_ptr<cminor::parsing::XmlLog> xmlLog;
    if (Log())
    {
        xmlLog.reset(new cminor::parsing::XmlLog(*Log(), MaxLogLineLength()));
        scanner.SetLog(xmlLog.get());
        xmlLog->WriteBeginRule("parse");
    }
    cminor::parsing::ObjectStack stack;
    std::unique_ptr<cminor::parsing::ParsingData> parsingData(new cminor::parsing::ParsingData(GetParsingDomain()->GetNumRules()));
    scanner.SetParsingData(parsingData.get());
    cminor::parsing::Match match = cminor::parsing::Grammar::Parse(scanner, stack, parsingData.get());
    cminor::parsing::Span stop = scanner.GetSpan();
    if (Log())
    {
        xmlLog->WriteEndRule("parse");
    }
    if (!match.Hit() || stop.Start() != int(end - start))
    {
        if (StartRule())
        {
            throw cminor::parsing::ExpectationFailure(StartRule()->Info(), fileName, stop, start, end);
        }
        else
        {
            throw cminor::parsing::ParsingException("grammar '" + ToUtf8(Name()) + "' has no start rule", fileName, scanner.GetSpan(), start, end);
        }
    }
    std::unique_ptr<cminor::parsing::Object> value = std::move(stack.top());
    cminor::parsing::Parser* result = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(value.get());
    stack.pop();
    return result;
}

class PrimitiveGrammar::PrimitiveRule : public cminor::parsing::Rule
{
public:
    PrimitiveRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A2Action));
        cminor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A3Action));
        cminor::parsing::ActionParser* a4ActionParser = GetAction(ToUtf32("A4"));
        a4ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A4Action));
        cminor::parsing::ActionParser* a5ActionParser = GetAction(ToUtf32("A5"));
        a5ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A5Action));
        cminor::parsing::ActionParser* a6ActionParser = GetAction(ToUtf32("A6"));
        a6ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A6Action));
        cminor::parsing::ActionParser* a7ActionParser = GetAction(ToUtf32("A7"));
        a7ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A7Action));
        cminor::parsing::ActionParser* a8ActionParser = GetAction(ToUtf32("A8"));
        a8ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A8Action));
        cminor::parsing::ActionParser* a9ActionParser = GetAction(ToUtf32("A9"));
        a9ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A9Action));
        cminor::parsing::ActionParser* a10ActionParser = GetAction(ToUtf32("A10"));
        a10ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A10Action));
        cminor::parsing::ActionParser* a11ActionParser = GetAction(ToUtf32("A11"));
        a11ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A11Action));
        cminor::parsing::ActionParser* a12ActionParser = GetAction(ToUtf32("A12"));
        a12ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A12Action));
        cminor::parsing::ActionParser* a13ActionParser = GetAction(ToUtf32("A13"));
        a13ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A13Action));
        cminor::parsing::ActionParser* a14ActionParser = GetAction(ToUtf32("A14"));
        a14ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A14Action));
        cminor::parsing::ActionParser* a15ActionParser = GetAction(ToUtf32("A15"));
        a15ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A15Action));
        cminor::parsing::ActionParser* a16ActionParser = GetAction(ToUtf32("A16"));
        a16ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A16Action));
        cminor::parsing::ActionParser* a17ActionParser = GetAction(ToUtf32("A17"));
        a17ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A17Action));
        cminor::parsing::ActionParser* a18ActionParser = GetAction(ToUtf32("A18"));
        a18ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A18Action));
        cminor::parsing::ActionParser* a19ActionParser = GetAction(ToUtf32("A19"));
        a19ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A19Action));
        cminor::parsing::ActionParser* a20ActionParser = GetAction(ToUtf32("A20"));
        a20ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A20Action));
        cminor::parsing::ActionParser* a21ActionParser = GetAction(ToUtf32("A21"));
        a21ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A21Action));
        cminor::parsing::ActionParser* a22ActionParser = GetAction(ToUtf32("A22"));
        a22ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A22Action));
        cminor::parsing::ActionParser* a23ActionParser = GetAction(ToUtf32("A23"));
        a23ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A23Action));
        cminor::parsing::ActionParser* a24ActionParser = GetAction(ToUtf32("A24"));
        a24ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A24Action));
        cminor::parsing::ActionParser* a25ActionParser = GetAction(ToUtf32("A25"));
        a25ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A25Action));
        cminor::parsing::ActionParser* a26ActionParser = GetAction(ToUtf32("A26"));
        a26ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A26Action));
        cminor::parsing::ActionParser* a27ActionParser = GetAction(ToUtf32("A27"));
        a27ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A27Action));
        cminor::parsing::ActionParser* a28ActionParser = GetAction(ToUtf32("A28"));
        a28ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A28Action));
        cminor::parsing::ActionParser* a29ActionParser = GetAction(ToUtf32("A29"));
        a29ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A29Action));
        cminor::parsing::ActionParser* a30ActionParser = GetAction(ToUtf32("A30"));
        a30ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A30Action));
        cminor::parsing::ActionParser* a31ActionParser = GetAction(ToUtf32("A31"));
        a31ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A31Action));
        cminor::parsing::ActionParser* a32ActionParser = GetAction(ToUtf32("A32"));
        a32ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A32Action));
        cminor::parsing::ActionParser* a33ActionParser = GetAction(ToUtf32("A33"));
        a33ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A33Action));
        cminor::parsing::ActionParser* a34ActionParser = GetAction(ToUtf32("A34"));
        a34ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A34Action));
        cminor::parsing::ActionParser* a35ActionParser = GetAction(ToUtf32("A35"));
        a35ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A35Action));
        cminor::parsing::ActionParser* a36ActionParser = GetAction(ToUtf32("A36"));
        a36ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A36Action));
        cminor::parsing::ActionParser* a37ActionParser = GetAction(ToUtf32("A37"));
        a37ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A37Action));
        cminor::parsing::ActionParser* a38ActionParser = GetAction(ToUtf32("A38"));
        a38ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A38Action));
        cminor::parsing::ActionParser* a39ActionParser = GetAction(ToUtf32("A39"));
        a39ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A39Action));
        cminor::parsing::ActionParser* a40ActionParser = GetAction(ToUtf32("A40"));
        a40ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A40Action));
        cminor::parsing::ActionParser* a41ActionParser = GetAction(ToUtf32("A41"));
        a41ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A41Action));
        cminor::parsing::ActionParser* a42ActionParser = GetAction(ToUtf32("A42"));
        a42ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A42Action));
        cminor::parsing::ActionParser* a43ActionParser = GetAction(ToUtf32("A43"));
        a43ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A43Action));
        cminor::parsing::ActionParser* a44ActionParser = GetAction(ToUtf32("A44"));
        a44ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A44Action));
        cminor::parsing::ActionParser* a45ActionParser = GetAction(ToUtf32("A45"));
        a45ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A45Action));
        cminor::parsing::ActionParser* a46ActionParser = GetAction(ToUtf32("A46"));
        a46ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A46Action));
        cminor::parsing::ActionParser* a47ActionParser = GetAction(ToUtf32("A47"));
        a47ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A47Action));
        cminor::parsing::ActionParser* a48ActionParser = GetAction(ToUtf32("A48"));
        a48ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A48Action));
        cminor::parsing::ActionParser* a49ActionParser = GetAction(ToUtf32("A49"));
        a49ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A49Action));
        cminor::parsing::ActionParser* a50ActionParser = GetAction(ToUtf32("A50"));
        a50ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A50Action));
        cminor::parsing::ActionParser* a51ActionParser = GetAction(ToUtf32("A51"));
        a51ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A51Action));
        cminor::parsing::ActionParser* a52ActionParser = GetAction(ToUtf32("A52"));
        a52ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A52Action));
        cminor::parsing::ActionParser* a53ActionParser = GetAction(ToUtf32("A53"));
        a53ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrimitiveRule>(this, &PrimitiveRule::A53Action));
        cminor::parsing::NonterminalParser* charNonterminalParser = GetNonterminal(ToUtf32("Char"));
        charNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostChar));
        cminor::parsing::NonterminalParser* stringNonterminalParser = GetNonterminal(ToUtf32("String"));
        stringNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostString));
        cminor::parsing::NonterminalParser* charSetNonterminalParser = GetNonterminal(ToUtf32("CharSet"));
        charSetNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostCharSet));
        cminor::parsing::NonterminalParser* keywordNonterminalParser = GetNonterminal(ToUtf32("Keyword"));
        keywordNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostKeyword));
        cminor::parsing::NonterminalParser* keywordListNonterminalParser = GetNonterminal(ToUtf32("KeywordList"));
        keywordListNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostKeywordList));
        cminor::parsing::NonterminalParser* rangeNonterminalParser = GetNonterminal(ToUtf32("Range"));
        rangeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostRange));
        cminor::parsing::NonterminalParser* emptyNonterminalParser = GetNonterminal(ToUtf32("Empty"));
        emptyNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostEmpty));
        cminor::parsing::NonterminalParser* spaceNonterminalParser = GetNonterminal(ToUtf32("Space"));
        spaceNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostSpace));
        cminor::parsing::NonterminalParser* anyCharNonterminalParser = GetNonterminal(ToUtf32("AnyChar"));
        anyCharNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostAnyChar));
        cminor::parsing::NonterminalParser* letterNonterminalParser = GetNonterminal(ToUtf32("Letter"));
        letterNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostLetter));
        cminor::parsing::NonterminalParser* digitNonterminalParser = GetNonterminal(ToUtf32("Digit"));
        digitNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostDigit));
        cminor::parsing::NonterminalParser* hexDigitNonterminalParser = GetNonterminal(ToUtf32("HexDigit"));
        hexDigitNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostHexDigit));
        cminor::parsing::NonterminalParser* upperLetterNonterminalParser = GetNonterminal(ToUtf32("UpperLetter"));
        upperLetterNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostUpperLetter));
        cminor::parsing::NonterminalParser* lowerLetterNonterminalParser = GetNonterminal(ToUtf32("LowerLetter"));
        lowerLetterNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostLowerLetter));
        cminor::parsing::NonterminalParser* titleLetterNonterminalParser = GetNonterminal(ToUtf32("TitleLetter"));
        titleLetterNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostTitleLetter));
        cminor::parsing::NonterminalParser* modifierLetterNonterminalParser = GetNonterminal(ToUtf32("ModifierLetter"));
        modifierLetterNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostModifierLetter));
        cminor::parsing::NonterminalParser* otherLetterNonterminalParser = GetNonterminal(ToUtf32("OtherLetter"));
        otherLetterNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostOtherLetter));
        cminor::parsing::NonterminalParser* casedLetterNonterminalParser = GetNonterminal(ToUtf32("CasedLetter"));
        casedLetterNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostCasedLetter));
        cminor::parsing::NonterminalParser* markNonterminalParser = GetNonterminal(ToUtf32("Mark"));
        markNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostMark));
        cminor::parsing::NonterminalParser* nonspacingMarkNonterminalParser = GetNonterminal(ToUtf32("NonspacingMark"));
        nonspacingMarkNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostNonspacingMark));
        cminor::parsing::NonterminalParser* enclosingMarkNonterminalParser = GetNonterminal(ToUtf32("EnclosingMark"));
        enclosingMarkNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostEnclosingMark));
        cminor::parsing::NonterminalParser* spacingMarkNonterminalParser = GetNonterminal(ToUtf32("SpacingMark"));
        spacingMarkNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostSpacingMark));
        cminor::parsing::NonterminalParser* numberNonterminalParser = GetNonterminal(ToUtf32("Number"));
        numberNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostNumber));
        cminor::parsing::NonterminalParser* decimalNumberNonterminalParser = GetNonterminal(ToUtf32("DecimalNumber"));
        decimalNumberNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostDecimalNumber));
        cminor::parsing::NonterminalParser* letterNumberNonterminalParser = GetNonterminal(ToUtf32("LetterNumber"));
        letterNumberNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostLetterNumber));
        cminor::parsing::NonterminalParser* otherNumberNonterminalParser = GetNonterminal(ToUtf32("OtherNumber"));
        otherNumberNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostOtherNumber));
        cminor::parsing::NonterminalParser* punctuationNonterminalParser = GetNonterminal(ToUtf32("Punctuation"));
        punctuationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostPunctuation));
        cminor::parsing::NonterminalParser* connectorPunctuationNonterminalParser = GetNonterminal(ToUtf32("ConnectorPunctuation"));
        connectorPunctuationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostConnectorPunctuation));
        cminor::parsing::NonterminalParser* dashPunctuationNonterminalParser = GetNonterminal(ToUtf32("DashPunctuation"));
        dashPunctuationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostDashPunctuation));
        cminor::parsing::NonterminalParser* openPunctuationNonterminalParser = GetNonterminal(ToUtf32("OpenPunctuation"));
        openPunctuationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostOpenPunctuation));
        cminor::parsing::NonterminalParser* closePunctuationNonterminalParser = GetNonterminal(ToUtf32("ClosePunctuation"));
        closePunctuationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostClosePunctuation));
        cminor::parsing::NonterminalParser* initialPunctuationNonterminalParser = GetNonterminal(ToUtf32("InitialPunctuation"));
        initialPunctuationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostInitialPunctuation));
        cminor::parsing::NonterminalParser* finalPunctuationNonterminalParser = GetNonterminal(ToUtf32("FinalPunctuation"));
        finalPunctuationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostFinalPunctuation));
        cminor::parsing::NonterminalParser* otherPunctuationNonterminalParser = GetNonterminal(ToUtf32("OtherPunctuation"));
        otherPunctuationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostOtherPunctuation));
        cminor::parsing::NonterminalParser* symbolNonterminalParser = GetNonterminal(ToUtf32("Symbol"));
        symbolNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostSymbol));
        cminor::parsing::NonterminalParser* mathSymbolNonterminalParser = GetNonterminal(ToUtf32("MathSymbol"));
        mathSymbolNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostMathSymbol));
        cminor::parsing::NonterminalParser* currencySymbolNonterminalParser = GetNonterminal(ToUtf32("CurrencySymbol"));
        currencySymbolNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostCurrencySymbol));
        cminor::parsing::NonterminalParser* modifierSymbolNonterminalParser = GetNonterminal(ToUtf32("ModifierSymbol"));
        modifierSymbolNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostModifierSymbol));
        cminor::parsing::NonterminalParser* otherSymbolNonterminalParser = GetNonterminal(ToUtf32("OtherSymbol"));
        otherSymbolNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostOtherSymbol));
        cminor::parsing::NonterminalParser* separatorNonterminalParser = GetNonterminal(ToUtf32("Separator"));
        separatorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostSeparator));
        cminor::parsing::NonterminalParser* spaceSeparatorNonterminalParser = GetNonterminal(ToUtf32("SpaceSeparator"));
        spaceSeparatorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostSpaceSeparator));
        cminor::parsing::NonterminalParser* lineSeparatorNonterminalParser = GetNonterminal(ToUtf32("LineSeparator"));
        lineSeparatorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostLineSeparator));
        cminor::parsing::NonterminalParser* paragraphSeparatorNonterminalParser = GetNonterminal(ToUtf32("ParagraphSeparator"));
        paragraphSeparatorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostParagraphSeparator));
        cminor::parsing::NonterminalParser* otherNonterminalParser = GetNonterminal(ToUtf32("Other"));
        otherNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostOther));
        cminor::parsing::NonterminalParser* controlNonterminalParser = GetNonterminal(ToUtf32("Control"));
        controlNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostControl));
        cminor::parsing::NonterminalParser* formatNonterminalParser = GetNonterminal(ToUtf32("Format"));
        formatNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostFormat));
        cminor::parsing::NonterminalParser* surrogateNonterminalParser = GetNonterminal(ToUtf32("Surrogate"));
        surrogateNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostSurrogate));
        cminor::parsing::NonterminalParser* privateUseNonterminalParser = GetNonterminal(ToUtf32("PrivateUse"));
        privateUseNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostPrivateUse));
        cminor::parsing::NonterminalParser* unassignedNonterminalParser = GetNonterminal(ToUtf32("Unassigned"));
        unassignedNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostUnassigned));
        cminor::parsing::NonterminalParser* graphicNonterminalParser = GetNonterminal(ToUtf32("Graphic"));
        graphicNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostGraphic));
        cminor::parsing::NonterminalParser* baseCharNonterminalParser = GetNonterminal(ToUtf32("BaseChar"));
        baseCharNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostBaseChar));
        cminor::parsing::NonterminalParser* alphabeticNonterminalParser = GetNonterminal(ToUtf32("Alphabetic"));
        alphabeticNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostAlphabetic));
        cminor::parsing::NonterminalParser* idStartNonterminalParser = GetNonterminal(ToUtf32("IdStart"));
        idStartNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostIdStart));
        cminor::parsing::NonterminalParser* idContNonterminalParser = GetNonterminal(ToUtf32("IdCont"));
        idContNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<PrimitiveRule>(this, &PrimitiveRule::PostIdCont));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromChar;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromString;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromCharSet;
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromKeyword;
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromKeywordList;
    }
    void A5Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromRange;
    }
    void A6Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromEmpty;
    }
    void A7Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromSpace;
    }
    void A8Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromAnyChar;
    }
    void A9Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromLetter;
    }
    void A10Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromDigit;
    }
    void A11Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromHexDigit;
    }
    void A12Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromUpperLetter;
    }
    void A13Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromLowerLetter;
    }
    void A14Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromTitleLetter;
    }
    void A15Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromModifierLetter;
    }
    void A16Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromOtherLetter;
    }
    void A17Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromCasedLetter;
    }
    void A18Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromMark;
    }
    void A19Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromNonspacingMark;
    }
    void A20Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromEnclosingMark;
    }
    void A21Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromSpacingMark;
    }
    void A22Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromNumber;
    }
    void A23Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromDecimalNumber;
    }
    void A24Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromLetterNumber;
    }
    void A25Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromOtherNumber;
    }
    void A26Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromPunctuation;
    }
    void A27Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromConnectorPunctuation;
    }
    void A28Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromDashPunctuation;
    }
    void A29Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromOpenPunctuation;
    }
    void A30Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromClosePunctuation;
    }
    void A31Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromInitialPunctuation;
    }
    void A32Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromFinalPunctuation;
    }
    void A33Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromOtherPunctuation;
    }
    void A34Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromSymbol;
    }
    void A35Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromMathSymbol;
    }
    void A36Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromCurrencySymbol;
    }
    void A37Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromModifierSymbol;
    }
    void A38Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromOtherSymbol;
    }
    void A39Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromSeparator;
    }
    void A40Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromSpaceSeparator;
    }
    void A41Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromLineSeparator;
    }
    void A42Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromParagraphSeparator;
    }
    void A43Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromOther;
    }
    void A44Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromControl;
    }
    void A45Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromFormat;
    }
    void A46Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromSurrogate;
    }
    void A47Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromPrivateUse;
    }
    void A48Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromUnassigned;
    }
    void A49Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromGraphic;
    }
    void A50Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromBaseChar;
    }
    void A51Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromAlphabetic;
    }
    void A52Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromIdStart;
    }
    void A53Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromIdCont;
    }
    void PostChar(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromChar_value = std::move(stack.top());
            context->fromChar = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromChar_value.get());
            stack.pop();
        }
    }
    void PostString(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromString_value = std::move(stack.top());
            context->fromString = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromString_value.get());
            stack.pop();
        }
    }
    void PostCharSet(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCharSet_value = std::move(stack.top());
            context->fromCharSet = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromCharSet_value.get());
            stack.pop();
        }
    }
    void PostKeyword(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromKeyword_value = std::move(stack.top());
            context->fromKeyword = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromKeyword_value.get());
            stack.pop();
        }
    }
    void PostKeywordList(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromKeywordList_value = std::move(stack.top());
            context->fromKeywordList = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromKeywordList_value.get());
            stack.pop();
        }
    }
    void PostRange(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromRange_value = std::move(stack.top());
            context->fromRange = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromRange_value.get());
            stack.pop();
        }
    }
    void PostEmpty(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromEmpty_value = std::move(stack.top());
            context->fromEmpty = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromEmpty_value.get());
            stack.pop();
        }
    }
    void PostSpace(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpace_value = std::move(stack.top());
            context->fromSpace = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromSpace_value.get());
            stack.pop();
        }
    }
    void PostAnyChar(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAnyChar_value = std::move(stack.top());
            context->fromAnyChar = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromAnyChar_value.get());
            stack.pop();
        }
    }
    void PostLetter(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromLetter_value = std::move(stack.top());
            context->fromLetter = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromLetter_value.get());
            stack.pop();
        }
    }
    void PostDigit(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDigit_value = std::move(stack.top());
            context->fromDigit = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromDigit_value.get());
            stack.pop();
        }
    }
    void PostHexDigit(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromHexDigit_value = std::move(stack.top());
            context->fromHexDigit = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromHexDigit_value.get());
            stack.pop();
        }
    }
    void PostUpperLetter(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromUpperLetter_value = std::move(stack.top());
            context->fromUpperLetter = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromUpperLetter_value.get());
            stack.pop();
        }
    }
    void PostLowerLetter(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromLowerLetter_value = std::move(stack.top());
            context->fromLowerLetter = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromLowerLetter_value.get());
            stack.pop();
        }
    }
    void PostTitleLetter(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromTitleLetter_value = std::move(stack.top());
            context->fromTitleLetter = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromTitleLetter_value.get());
            stack.pop();
        }
    }
    void PostModifierLetter(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromModifierLetter_value = std::move(stack.top());
            context->fromModifierLetter = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromModifierLetter_value.get());
            stack.pop();
        }
    }
    void PostOtherLetter(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromOtherLetter_value = std::move(stack.top());
            context->fromOtherLetter = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromOtherLetter_value.get());
            stack.pop();
        }
    }
    void PostCasedLetter(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCasedLetter_value = std::move(stack.top());
            context->fromCasedLetter = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromCasedLetter_value.get());
            stack.pop();
        }
    }
    void PostMark(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromMark_value = std::move(stack.top());
            context->fromMark = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromMark_value.get());
            stack.pop();
        }
    }
    void PostNonspacingMark(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromNonspacingMark_value = std::move(stack.top());
            context->fromNonspacingMark = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromNonspacingMark_value.get());
            stack.pop();
        }
    }
    void PostEnclosingMark(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromEnclosingMark_value = std::move(stack.top());
            context->fromEnclosingMark = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromEnclosingMark_value.get());
            stack.pop();
        }
    }
    void PostSpacingMark(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpacingMark_value = std::move(stack.top());
            context->fromSpacingMark = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromSpacingMark_value.get());
            stack.pop();
        }
    }
    void PostNumber(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromNumber_value = std::move(stack.top());
            context->fromNumber = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromNumber_value.get());
            stack.pop();
        }
    }
    void PostDecimalNumber(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDecimalNumber_value = std::move(stack.top());
            context->fromDecimalNumber = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromDecimalNumber_value.get());
            stack.pop();
        }
    }
    void PostLetterNumber(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromLetterNumber_value = std::move(stack.top());
            context->fromLetterNumber = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromLetterNumber_value.get());
            stack.pop();
        }
    }
    void PostOtherNumber(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromOtherNumber_value = std::move(stack.top());
            context->fromOtherNumber = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromOtherNumber_value.get());
            stack.pop();
        }
    }
    void PostPunctuation(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromPunctuation_value = std::move(stack.top());
            context->fromPunctuation = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromPunctuation_value.get());
            stack.pop();
        }
    }
    void PostConnectorPunctuation(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromConnectorPunctuation_value = std::move(stack.top());
            context->fromConnectorPunctuation = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromConnectorPunctuation_value.get());
            stack.pop();
        }
    }
    void PostDashPunctuation(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromDashPunctuation_value = std::move(stack.top());
            context->fromDashPunctuation = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromDashPunctuation_value.get());
            stack.pop();
        }
    }
    void PostOpenPunctuation(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromOpenPunctuation_value = std::move(stack.top());
            context->fromOpenPunctuation = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromOpenPunctuation_value.get());
            stack.pop();
        }
    }
    void PostClosePunctuation(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromClosePunctuation_value = std::move(stack.top());
            context->fromClosePunctuation = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromClosePunctuation_value.get());
            stack.pop();
        }
    }
    void PostInitialPunctuation(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromInitialPunctuation_value = std::move(stack.top());
            context->fromInitialPunctuation = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromInitialPunctuation_value.get());
            stack.pop();
        }
    }
    void PostFinalPunctuation(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFinalPunctuation_value = std::move(stack.top());
            context->fromFinalPunctuation = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromFinalPunctuation_value.get());
            stack.pop();
        }
    }
    void PostOtherPunctuation(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromOtherPunctuation_value = std::move(stack.top());
            context->fromOtherPunctuation = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromOtherPunctuation_value.get());
            stack.pop();
        }
    }
    void PostSymbol(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSymbol_value = std::move(stack.top());
            context->fromSymbol = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromSymbol_value.get());
            stack.pop();
        }
    }
    void PostMathSymbol(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromMathSymbol_value = std::move(stack.top());
            context->fromMathSymbol = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromMathSymbol_value.get());
            stack.pop();
        }
    }
    void PostCurrencySymbol(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCurrencySymbol_value = std::move(stack.top());
            context->fromCurrencySymbol = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromCurrencySymbol_value.get());
            stack.pop();
        }
    }
    void PostModifierSymbol(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromModifierSymbol_value = std::move(stack.top());
            context->fromModifierSymbol = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromModifierSymbol_value.get());
            stack.pop();
        }
    }
    void PostOtherSymbol(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromOtherSymbol_value = std::move(stack.top());
            context->fromOtherSymbol = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromOtherSymbol_value.get());
            stack.pop();
        }
    }
    void PostSeparator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSeparator_value = std::move(stack.top());
            context->fromSeparator = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromSeparator_value.get());
            stack.pop();
        }
    }
    void PostSpaceSeparator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSpaceSeparator_value = std::move(stack.top());
            context->fromSpaceSeparator = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromSpaceSeparator_value.get());
            stack.pop();
        }
    }
    void PostLineSeparator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromLineSeparator_value = std::move(stack.top());
            context->fromLineSeparator = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromLineSeparator_value.get());
            stack.pop();
        }
    }
    void PostParagraphSeparator(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromParagraphSeparator_value = std::move(stack.top());
            context->fromParagraphSeparator = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromParagraphSeparator_value.get());
            stack.pop();
        }
    }
    void PostOther(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromOther_value = std::move(stack.top());
            context->fromOther = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromOther_value.get());
            stack.pop();
        }
    }
    void PostControl(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromControl_value = std::move(stack.top());
            context->fromControl = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromControl_value.get());
            stack.pop();
        }
    }
    void PostFormat(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromFormat_value = std::move(stack.top());
            context->fromFormat = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromFormat_value.get());
            stack.pop();
        }
    }
    void PostSurrogate(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromSurrogate_value = std::move(stack.top());
            context->fromSurrogate = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromSurrogate_value.get());
            stack.pop();
        }
    }
    void PostPrivateUse(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromPrivateUse_value = std::move(stack.top());
            context->fromPrivateUse = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromPrivateUse_value.get());
            stack.pop();
        }
    }
    void PostUnassigned(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromUnassigned_value = std::move(stack.top());
            context->fromUnassigned = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromUnassigned_value.get());
            stack.pop();
        }
    }
    void PostGraphic(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromGraphic_value = std::move(stack.top());
            context->fromGraphic = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromGraphic_value.get());
            stack.pop();
        }
    }
    void PostBaseChar(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromBaseChar_value = std::move(stack.top());
            context->fromBaseChar = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromBaseChar_value.get());
            stack.pop();
        }
    }
    void PostAlphabetic(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromAlphabetic_value = std::move(stack.top());
            context->fromAlphabetic = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromAlphabetic_value.get());
            stack.pop();
        }
    }
    void PostIdStart(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIdStart_value = std::move(stack.top());
            context->fromIdStart = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromIdStart_value.get());
            stack.pop();
        }
    }
    void PostIdCont(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromIdCont_value = std::move(stack.top());
            context->fromIdCont = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromIdCont_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromChar(), fromString(), fromCharSet(), fromKeyword(), fromKeywordList(), fromRange(), fromEmpty(), fromSpace(), fromAnyChar(), fromLetter(), fromDigit(), fromHexDigit(), fromUpperLetter(), fromLowerLetter(), fromTitleLetter(), fromModifierLetter(), fromOtherLetter(), fromCasedLetter(), fromMark(), fromNonspacingMark(), fromEnclosingMark(), fromSpacingMark(), fromNumber(), fromDecimalNumber(), fromLetterNumber(), fromOtherNumber(), fromPunctuation(), fromConnectorPunctuation(), fromDashPunctuation(), fromOpenPunctuation(), fromClosePunctuation(), fromInitialPunctuation(), fromFinalPunctuation(), fromOtherPunctuation(), fromSymbol(), fromMathSymbol(), fromCurrencySymbol(), fromModifierSymbol(), fromOtherSymbol(), fromSeparator(), fromSpaceSeparator(), fromLineSeparator(), fromParagraphSeparator(), fromOther(), fromControl(), fromFormat(), fromSurrogate(), fromPrivateUse(), fromUnassigned(), fromGraphic(), fromBaseChar(), fromAlphabetic(), fromIdStart(), fromIdCont() {}
        cminor::parsing::Parser* value;
        cminor::parsing::Parser* fromChar;
        cminor::parsing::Parser* fromString;
        cminor::parsing::Parser* fromCharSet;
        cminor::parsing::Parser* fromKeyword;
        cminor::parsing::Parser* fromKeywordList;
        cminor::parsing::Parser* fromRange;
        cminor::parsing::Parser* fromEmpty;
        cminor::parsing::Parser* fromSpace;
        cminor::parsing::Parser* fromAnyChar;
        cminor::parsing::Parser* fromLetter;
        cminor::parsing::Parser* fromDigit;
        cminor::parsing::Parser* fromHexDigit;
        cminor::parsing::Parser* fromUpperLetter;
        cminor::parsing::Parser* fromLowerLetter;
        cminor::parsing::Parser* fromTitleLetter;
        cminor::parsing::Parser* fromModifierLetter;
        cminor::parsing::Parser* fromOtherLetter;
        cminor::parsing::Parser* fromCasedLetter;
        cminor::parsing::Parser* fromMark;
        cminor::parsing::Parser* fromNonspacingMark;
        cminor::parsing::Parser* fromEnclosingMark;
        cminor::parsing::Parser* fromSpacingMark;
        cminor::parsing::Parser* fromNumber;
        cminor::parsing::Parser* fromDecimalNumber;
        cminor::parsing::Parser* fromLetterNumber;
        cminor::parsing::Parser* fromOtherNumber;
        cminor::parsing::Parser* fromPunctuation;
        cminor::parsing::Parser* fromConnectorPunctuation;
        cminor::parsing::Parser* fromDashPunctuation;
        cminor::parsing::Parser* fromOpenPunctuation;
        cminor::parsing::Parser* fromClosePunctuation;
        cminor::parsing::Parser* fromInitialPunctuation;
        cminor::parsing::Parser* fromFinalPunctuation;
        cminor::parsing::Parser* fromOtherPunctuation;
        cminor::parsing::Parser* fromSymbol;
        cminor::parsing::Parser* fromMathSymbol;
        cminor::parsing::Parser* fromCurrencySymbol;
        cminor::parsing::Parser* fromModifierSymbol;
        cminor::parsing::Parser* fromOtherSymbol;
        cminor::parsing::Parser* fromSeparator;
        cminor::parsing::Parser* fromSpaceSeparator;
        cminor::parsing::Parser* fromLineSeparator;
        cminor::parsing::Parser* fromParagraphSeparator;
        cminor::parsing::Parser* fromOther;
        cminor::parsing::Parser* fromControl;
        cminor::parsing::Parser* fromFormat;
        cminor::parsing::Parser* fromSurrogate;
        cminor::parsing::Parser* fromPrivateUse;
        cminor::parsing::Parser* fromUnassigned;
        cminor::parsing::Parser* fromGraphic;
        cminor::parsing::Parser* fromBaseChar;
        cminor::parsing::Parser* fromAlphabetic;
        cminor::parsing::Parser* fromIdStart;
        cminor::parsing::Parser* fromIdCont;
    };
};

class PrimitiveGrammar::CharRule : public cminor::parsing::Rule
{
public:
    CharRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharRule>(this, &CharRule::A0Action));
        cminor::parsing::NonterminalParser* chrNonterminalParser = GetNonterminal(ToUtf32("chr"));
        chrNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CharRule>(this, &CharRule::Postchr));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new CharParser(context->fromchr);
    }
    void Postchr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromchr_value = std::move(stack.top());
            context->fromchr = *static_cast<cminor::parsing::ValueObject<char32_t>*>(fromchr_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromchr() {}
        cminor::parsing::Parser* value;
        char32_t fromchr;
    };
};

class PrimitiveGrammar::StringRule : public cminor::parsing::Rule
{
public:
    StringRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<StringRule>(this, &StringRule::A0Action));
        cminor::parsing::NonterminalParser* strNonterminalParser = GetNonterminal(ToUtf32("str"));
        strNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<StringRule>(this, &StringRule::Poststr));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new StringParser(context->fromstr);
    }
    void Poststr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromstr_value = std::move(stack.top());
            context->fromstr = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromstr_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromstr() {}
        cminor::parsing::Parser* value;
        std::u32string fromstr;
    };
};

class PrimitiveGrammar::CharSetRule : public cminor::parsing::Rule
{
public:
    CharSetRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("bool"), ToUtf32("inverse")));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::u32string"), ToUtf32("s")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharSetRule>(this, &CharSetRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharSetRule>(this, &CharSetRule::A1Action));
        cminor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharSetRule>(this, &CharSetRule::A2Action));
        cminor::parsing::NonterminalParser* charSetRangeNonterminalParser = GetNonterminal(ToUtf32("CharSetRange"));
        charSetRangeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CharSetRule>(this, &CharSetRule::PostCharSetRange));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new CharSetParser(context->s, context->inverse);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->inverse = true;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->s.append(context->fromCharSetRange);
    }
    void PostCharSetRange(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromCharSetRange_value = std::move(stack.top());
            context->fromCharSetRange = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromCharSetRange_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), inverse(), s(), fromCharSetRange() {}
        cminor::parsing::Parser* value;
        bool inverse;
        std::u32string s;
        std::u32string fromCharSetRange;
    };
};

class PrimitiveGrammar::CharSetRangeRule : public cminor::parsing::Rule
{
public:
    CharSetRangeRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("std::u32string"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::u32string>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharSetRangeRule>(this, &CharSetRangeRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharSetRangeRule>(this, &CharSetRangeRule::A1Action));
        cminor::parsing::NonterminalParser* firstNonterminalParser = GetNonterminal(ToUtf32("first"));
        firstNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CharSetRangeRule>(this, &CharSetRangeRule::Postfirst));
        cminor::parsing::NonterminalParser* lastNonterminalParser = GetNonterminal(ToUtf32("last"));
        lastNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CharSetRangeRule>(this, &CharSetRangeRule::Postlast));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value.append(1, context->fromfirst);
        context->value.append(1, '-');
        context->value.append(1, context->fromlast);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value.append(1, context->fromfirst);
    }
    void Postfirst(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromfirst_value = std::move(stack.top());
            context->fromfirst = *static_cast<cminor::parsing::ValueObject<char32_t>*>(fromfirst_value.get());
            stack.pop();
        }
    }
    void Postlast(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromlast_value = std::move(stack.top());
            context->fromlast = *static_cast<cminor::parsing::ValueObject<char32_t>*>(fromlast_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromfirst(), fromlast() {}
        std::u32string value;
        char32_t fromfirst;
        char32_t fromlast;
    };
};

class PrimitiveGrammar::CharSetCharRule : public cminor::parsing::Rule
{
public:
    CharSetCharRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("char32_t"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<char32_t>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharSetCharRule>(this, &CharSetCharRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CharSetCharRule>(this, &CharSetCharRule::A1Action));
        cminor::parsing::NonterminalParser* escapeNonterminalParser = GetNonterminal(ToUtf32("escape"));
        escapeNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CharSetCharRule>(this, &CharSetCharRule::Postescape));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = *matchBegin;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromescape;
    }
    void Postescape(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromescape_value = std::move(stack.top());
            context->fromescape = *static_cast<cminor::parsing::ValueObject<char32_t>*>(fromescape_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromescape() {}
        char32_t value;
        char32_t fromescape;
    };
};

class PrimitiveGrammar::KeywordRule : public cminor::parsing::Rule
{
public:
    KeywordRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<KeywordRule>(this, &KeywordRule::A0Action));
        cminor::parsing::NonterminalParser* keywordBodyNonterminalParser = GetNonterminal(ToUtf32("KeywordBody"));
        keywordBodyNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<KeywordRule>(this, &KeywordRule::PostKeywordBody));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromKeywordBody;
    }
    void PostKeywordBody(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromKeywordBody_value = std::move(stack.top());
            context->fromKeywordBody = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromKeywordBody_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromKeywordBody() {}
        cminor::parsing::Parser* value;
        cminor::parsing::Parser* fromKeywordBody;
    };
};

class PrimitiveGrammar::KeywordBodyRule : public cminor::parsing::Rule
{
public:
    KeywordBodyRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<KeywordBodyRule>(this, &KeywordBodyRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<KeywordBodyRule>(this, &KeywordBodyRule::A1Action));
        cminor::parsing::NonterminalParser* strNonterminalParser = GetNonterminal(ToUtf32("str"));
        strNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<KeywordBodyRule>(this, &KeywordBodyRule::Poststr));
        cminor::parsing::NonterminalParser* continuationNonterminalParser = GetNonterminal(ToUtf32("continuation"));
        continuationNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<KeywordBodyRule>(this, &KeywordBodyRule::Postcontinuation));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new KeywordParser(context->fromstr, context->fromcontinuation);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new KeywordParser(context->fromstr);
    }
    void Poststr(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromstr_value = std::move(stack.top());
            context->fromstr = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromstr_value.get());
            stack.pop();
        }
    }
    void Postcontinuation(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromcontinuation_value = std::move(stack.top());
            context->fromcontinuation = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromcontinuation_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromstr(), fromcontinuation() {}
        cminor::parsing::Parser* value;
        std::u32string fromstr;
        std::u32string fromcontinuation;
    };
};

class PrimitiveGrammar::KeywordListRule : public cminor::parsing::Rule
{
public:
    KeywordListRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<KeywordListRule>(this, &KeywordListRule::A0Action));
        cminor::parsing::NonterminalParser* keywordListBodyNonterminalParser = GetNonterminal(ToUtf32("KeywordListBody"));
        keywordListBodyNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<KeywordListRule>(this, &KeywordListRule::PostKeywordListBody));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromKeywordListBody;
    }
    void PostKeywordListBody(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromKeywordListBody_value = std::move(stack.top());
            context->fromKeywordListBody = *static_cast<cminor::parsing::ValueObject<cminor::parsing::Parser*>*>(fromKeywordListBody_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromKeywordListBody() {}
        cminor::parsing::Parser* value;
        cminor::parsing::Parser* fromKeywordListBody;
    };
};

class PrimitiveGrammar::KeywordListBodyRule : public cminor::parsing::Rule
{
public:
    KeywordListBodyRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::vector<std::u32string>"), ToUtf32("keywords")));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<KeywordListBodyRule>(this, &KeywordListBodyRule::A0Action));
        cminor::parsing::NonterminalParser* selectorNonterminalParser = GetNonterminal(ToUtf32("selector"));
        selectorNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<KeywordListBodyRule>(this, &KeywordListBodyRule::Postselector));
        cminor::parsing::NonterminalParser* stringArrayNonterminalParser = GetNonterminal(ToUtf32("StringArray"));
        stringArrayNonterminalParser->SetPreCall(new cminor::parsing::MemberPreCall<KeywordListBodyRule>(this, &KeywordListBodyRule::PreStringArray));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new KeywordListParser(context->fromselector, context->keywords);
    }
    void Postselector(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromselector_value = std::move(stack.top());
            context->fromselector = *static_cast<cminor::parsing::ValueObject<std::u32string>*>(fromselector_value.get());
            stack.pop();
        }
    }
    void PreStringArray(cminor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<std::vector<std::u32string>*>(&context->keywords)));
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), keywords(), fromselector() {}
        cminor::parsing::Parser* value;
        std::vector<std::u32string> keywords;
        std::u32string fromselector;
    };
};

class PrimitiveGrammar::RangeRule : public cminor::parsing::Rule
{
public:
    RangeRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<RangeRule>(this, &RangeRule::A0Action));
        cminor::parsing::NonterminalParser* sNonterminalParser = GetNonterminal(ToUtf32("s"));
        sNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RangeRule>(this, &RangeRule::Posts));
        cminor::parsing::NonterminalParser* eNonterminalParser = GetNonterminal(ToUtf32("e"));
        eNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<RangeRule>(this, &RangeRule::Poste));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new RangeParser(context->froms, context->frome);
    }
    void Posts(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> froms_value = std::move(stack.top());
            context->froms = *static_cast<cminor::parsing::ValueObject<uint32_t>*>(froms_value.get());
            stack.pop();
        }
    }
    void Poste(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> frome_value = std::move(stack.top());
            context->frome = *static_cast<cminor::parsing::ValueObject<uint32_t>*>(frome_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), froms(), frome() {}
        cminor::parsing::Parser* value;
        uint32_t froms;
        uint32_t frome;
    };
};

class PrimitiveGrammar::CodePointRule : public cminor::parsing::Rule
{
public:
    CodePointRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("uint32_t"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<uint32_t>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CodePointRule>(this, &CodePointRule::A0Action));
        cminor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CodePointRule>(this, &CodePointRule::A1Action));
        cminor::parsing::NonterminalParser* xNonterminalParser = GetNonterminal(ToUtf32("x"));
        xNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CodePointRule>(this, &CodePointRule::Postx));
        cminor::parsing::NonterminalParser* dNonterminalParser = GetNonterminal(ToUtf32("d"));
        dNonterminalParser->SetPostCall(new cminor::parsing::MemberPostCall<CodePointRule>(this, &CodePointRule::Postd));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromx;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromd;
    }
    void Postx(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromx_value = std::move(stack.top());
            context->fromx = *static_cast<cminor::parsing::ValueObject<uint32_t>*>(fromx_value.get());
            stack.pop();
        }
    }
    void Postd(cminor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cminor::parsing::Object> fromd_value = std::move(stack.top());
            context->fromd = *static_cast<cminor::parsing::ValueObject<uint32_t>*>(fromd_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value(), fromx(), fromd() {}
        uint32_t value;
        uint32_t fromx;
        uint32_t fromd;
    };
};

class PrimitiveGrammar::EmptyRule : public cminor::parsing::Rule
{
public:
    EmptyRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EmptyRule>(this, &EmptyRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new EmptyParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::SpaceRule : public cminor::parsing::Rule
{
public:
    SpaceRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SpaceRule>(this, &SpaceRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new SpaceParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::AnyCharRule : public cminor::parsing::Rule
{
public:
    AnyCharRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AnyCharRule>(this, &AnyCharRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new AnyCharParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::LetterRule : public cminor::parsing::Rule
{
public:
    LetterRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LetterRule>(this, &LetterRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new LetterParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::DigitRule : public cminor::parsing::Rule
{
public:
    DigitRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DigitRule>(this, &DigitRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new DigitParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::HexDigitRule : public cminor::parsing::Rule
{
public:
    HexDigitRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<HexDigitRule>(this, &HexDigitRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new HexDigitParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::UpperLetterRule : public cminor::parsing::Rule
{
public:
    UpperLetterRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UpperLetterRule>(this, &UpperLetterRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new UpperLetterParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::LowerLetterRule : public cminor::parsing::Rule
{
public:
    LowerLetterRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LowerLetterRule>(this, &LowerLetterRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new LowerLetterParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::TitleLetterRule : public cminor::parsing::Rule
{
public:
    TitleLetterRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<TitleLetterRule>(this, &TitleLetterRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new TitleLetterParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::ModifierLetterRule : public cminor::parsing::Rule
{
public:
    ModifierLetterRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ModifierLetterRule>(this, &ModifierLetterRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ModifierLetterParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::OtherLetterRule : public cminor::parsing::Rule
{
public:
    OtherLetterRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OtherLetterRule>(this, &OtherLetterRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new OtherLetterParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::CasedLetterRule : public cminor::parsing::Rule
{
public:
    CasedLetterRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CasedLetterRule>(this, &CasedLetterRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new CasedLetterParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::MarkRule : public cminor::parsing::Rule
{
public:
    MarkRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MarkRule>(this, &MarkRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new MarkParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::NonspacingMarkRule : public cminor::parsing::Rule
{
public:
    NonspacingMarkRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NonspacingMarkRule>(this, &NonspacingMarkRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new NonspacingMarkParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::SpacingMarkRule : public cminor::parsing::Rule
{
public:
    SpacingMarkRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SpacingMarkRule>(this, &SpacingMarkRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new SpacingMarkParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::EnclosingMarkRule : public cminor::parsing::Rule
{
public:
    EnclosingMarkRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<EnclosingMarkRule>(this, &EnclosingMarkRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new EnclosingMarkParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::NumberRule : public cminor::parsing::Rule
{
public:
    NumberRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<NumberRule>(this, &NumberRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new NumberParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::DecimalNumberRule : public cminor::parsing::Rule
{
public:
    DecimalNumberRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DecimalNumberRule>(this, &DecimalNumberRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new DecimalNumberParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::LetterNumberRule : public cminor::parsing::Rule
{
public:
    LetterNumberRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LetterNumberRule>(this, &LetterNumberRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new LetterNumberParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::OtherNumberRule : public cminor::parsing::Rule
{
public:
    OtherNumberRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OtherNumberRule>(this, &OtherNumberRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new OtherNumberParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::PunctuationRule : public cminor::parsing::Rule
{
public:
    PunctuationRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PunctuationRule>(this, &PunctuationRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new PunctuationParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::ConnectorPunctuationRule : public cminor::parsing::Rule
{
public:
    ConnectorPunctuationRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ConnectorPunctuationRule>(this, &ConnectorPunctuationRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ConnectorPunctuationParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::DashPunctuationRule : public cminor::parsing::Rule
{
public:
    DashPunctuationRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<DashPunctuationRule>(this, &DashPunctuationRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new DashPunctuationParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::OpenPunctuationRule : public cminor::parsing::Rule
{
public:
    OpenPunctuationRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OpenPunctuationRule>(this, &OpenPunctuationRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new OpenPunctuationParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::ClosePunctuationRule : public cminor::parsing::Rule
{
public:
    ClosePunctuationRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ClosePunctuationRule>(this, &ClosePunctuationRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ClosePunctuationParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::InitialPunctuationRule : public cminor::parsing::Rule
{
public:
    InitialPunctuationRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<InitialPunctuationRule>(this, &InitialPunctuationRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new InitialPunctuationParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::FinalPunctuationRule : public cminor::parsing::Rule
{
public:
    FinalPunctuationRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FinalPunctuationRule>(this, &FinalPunctuationRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new FinalPunctuationParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::OtherPunctuationRule : public cminor::parsing::Rule
{
public:
    OtherPunctuationRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OtherPunctuationRule>(this, &OtherPunctuationRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new OtherPunctuationParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::SymbolRule : public cminor::parsing::Rule
{
public:
    SymbolRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SymbolRule>(this, &SymbolRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new SymbolParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::MathSymbolRule : public cminor::parsing::Rule
{
public:
    MathSymbolRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<MathSymbolRule>(this, &MathSymbolRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new MathSymbolParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::CurrencySymbolRule : public cminor::parsing::Rule
{
public:
    CurrencySymbolRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<CurrencySymbolRule>(this, &CurrencySymbolRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new CurrencySymbolParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::ModifierSymbolRule : public cminor::parsing::Rule
{
public:
    ModifierSymbolRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ModifierSymbolRule>(this, &ModifierSymbolRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ModifierSymbolParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::OtherSymbolRule : public cminor::parsing::Rule
{
public:
    OtherSymbolRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OtherSymbolRule>(this, &OtherSymbolRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new OtherSymbolParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::SeparatorRule : public cminor::parsing::Rule
{
public:
    SeparatorRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SeparatorRule>(this, &SeparatorRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new SeparatorParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::SpaceSeparatorRule : public cminor::parsing::Rule
{
public:
    SpaceSeparatorRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SpaceSeparatorRule>(this, &SpaceSeparatorRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new SpaceSeparatorParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::LineSeparatorRule : public cminor::parsing::Rule
{
public:
    LineSeparatorRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<LineSeparatorRule>(this, &LineSeparatorRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new LineSeparatorParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::ParagraphSeparatorRule : public cminor::parsing::Rule
{
public:
    ParagraphSeparatorRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ParagraphSeparatorRule>(this, &ParagraphSeparatorRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ParagraphSeparatorParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::OtherRule : public cminor::parsing::Rule
{
public:
    OtherRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<OtherRule>(this, &OtherRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new OtherParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::ControlRule : public cminor::parsing::Rule
{
public:
    ControlRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<ControlRule>(this, &ControlRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ControlParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::FormatRule : public cminor::parsing::Rule
{
public:
    FormatRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<FormatRule>(this, &FormatRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new FormatParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::SurrogateRule : public cminor::parsing::Rule
{
public:
    SurrogateRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<SurrogateRule>(this, &SurrogateRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new SurrogateParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::PrivateUseRule : public cminor::parsing::Rule
{
public:
    PrivateUseRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<PrivateUseRule>(this, &PrivateUseRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new PrivateUseParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::UnassignedRule : public cminor::parsing::Rule
{
public:
    UnassignedRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<UnassignedRule>(this, &UnassignedRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new UnassignedParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::GraphicRule : public cminor::parsing::Rule
{
public:
    GraphicRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<GraphicRule>(this, &GraphicRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new GraphicParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::BaseCharRule : public cminor::parsing::Rule
{
public:
    BaseCharRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<BaseCharRule>(this, &BaseCharRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BaseCharParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::AlphabeticRule : public cminor::parsing::Rule
{
public:
    AlphabeticRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<AlphabeticRule>(this, &AlphabeticRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new AlphabeticParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::IdStartRule : public cminor::parsing::Rule
{
public:
    IdStartRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IdStartRule>(this, &IdStartRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new IdStartParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

class PrimitiveGrammar::IdContRule : public cminor::parsing::Rule
{
public:
    IdContRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cminor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("cminor::parsing::Parser*"));
    }
    virtual void Enter(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cminor::parsing::ObjectStack& stack, cminor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cminor::parsing::Object>(new cminor::parsing::ValueObject<cminor::parsing::Parser*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cminor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cminor::parsing::MemberParsingAction<IdContRule>(this, &IdContRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new IdContParser;
    }
private:
    struct Context : cminor::parsing::Context
    {
        Context(): value() {}
        cminor::parsing::Parser* value;
    };
};

void PrimitiveGrammar::GetReferencedGrammars()
{
    cminor::parsing::ParsingDomain* pd = GetParsingDomain();
    cminor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cminor.syntax.ElementGrammar"));
    if (!grammar0)
    {
        grammar0 = cminor::syntax::ElementGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cminor::parsing::Grammar* grammar1 = pd->GetGrammar(ToUtf32("cminor.parsing.stdlib"));
    if (!grammar1)
    {
        grammar1 = cminor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar1);
}

void PrimitiveGrammar::CreateRules()
{
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("StringArray"), this, ToUtf32("ElementGrammar.StringArray")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("QualifiedId"), this, ToUtf32("ElementGrammar.QualifiedId")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("char"), this, ToUtf32("cminor.parsing.stdlib.char")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("string"), this, ToUtf32("cminor.parsing.stdlib.string")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("escape"), this, ToUtf32("cminor.parsing.stdlib.escape")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("int"), this, ToUtf32("cminor.parsing.stdlib.int")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("uint"), this, ToUtf32("cminor.parsing.stdlib.uint")));
    AddRuleLink(new cminor::parsing::RuleLink(ToUtf32("hexuint"), this, ToUtf32("cminor.parsing.stdlib.hexuint")));
    AddRule(new PrimitiveRule(ToUtf32("Primitive"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::AlternativeParser(
                    new cminor::parsing::AlternativeParser(
                        new cminor::parsing::AlternativeParser(
                            new cminor::parsing::AlternativeParser(
                                new cminor::parsing::AlternativeParser(
                                    new cminor::parsing::AlternativeParser(
                                        new cminor::parsing::AlternativeParser(
                                            new cminor::parsing::AlternativeParser(
                                                new cminor::parsing::AlternativeParser(
                                                    new cminor::parsing::AlternativeParser(
                                                        new cminor::parsing::AlternativeParser(
                                                            new cminor::parsing::AlternativeParser(
                                                                new cminor::parsing::AlternativeParser(
                                                                    new cminor::parsing::AlternativeParser(
                                                                        new cminor::parsing::AlternativeParser(
                                                                            new cminor::parsing::AlternativeParser(
                                                                                new cminor::parsing::AlternativeParser(
                                                                                    new cminor::parsing::AlternativeParser(
                                                                                        new cminor::parsing::AlternativeParser(
                                                                                            new cminor::parsing::AlternativeParser(
                                                                                                new cminor::parsing::AlternativeParser(
                                                                                                    new cminor::parsing::AlternativeParser(
                                                                                                        new cminor::parsing::AlternativeParser(
                                                                                                            new cminor::parsing::AlternativeParser(
                                                                                                                new cminor::parsing::AlternativeParser(
                                                                                                                    new cminor::parsing::AlternativeParser(
                                                                                                                        new cminor::parsing::AlternativeParser(
                                                                                                                            new cminor::parsing::AlternativeParser(
                                                                                                                                new cminor::parsing::AlternativeParser(
                                                                                                                                    new cminor::parsing::AlternativeParser(
                                                                                                                                        new cminor::parsing::AlternativeParser(
                                                                                                                                            new cminor::parsing::AlternativeParser(
                                                                                                                                                new cminor::parsing::AlternativeParser(
                                                                                                                                                    new cminor::parsing::AlternativeParser(
                                                                                                                                                        new cminor::parsing::AlternativeParser(
                                                                                                                                                            new cminor::parsing::AlternativeParser(
                                                                                                                                                                new cminor::parsing::AlternativeParser(
                                                                                                                                                                    new cminor::parsing::AlternativeParser(
                                                                                                                                                                        new cminor::parsing::AlternativeParser(
                                                                                                                                                                            new cminor::parsing::AlternativeParser(
                                                                                                                                                                                new cminor::parsing::AlternativeParser(
                                                                                                                                                                                    new cminor::parsing::AlternativeParser(
                                                                                                                                                                                        new cminor::parsing::AlternativeParser(
                                                                                                                                                                                            new cminor::parsing::AlternativeParser(
                                                                                                                                                                                                new cminor::parsing::AlternativeParser(
                                                                                                                                                                                                    new cminor::parsing::AlternativeParser(
                                                                                                                                                                                                        new cminor::parsing::AlternativeParser(
                                                                                                                                                                                                            new cminor::parsing::AlternativeParser(
                                                                                                                                                                                                                new cminor::parsing::AlternativeParser(
                                                                                                                                                                                                                    new cminor::parsing::AlternativeParser(
                                                                                                                                                                                                                        new cminor::parsing::AlternativeParser(
                                                                                                                                                                                                                            new cminor::parsing::ActionParser(ToUtf32("A0"),
                                                                                                                                                                                                                                new cminor::parsing::NonterminalParser(ToUtf32("Char"), ToUtf32("Char"), 0)),
                                                                                                                                                                                                                            new cminor::parsing::ActionParser(ToUtf32("A1"),
                                                                                                                                                                                                                                new cminor::parsing::NonterminalParser(ToUtf32("String"), ToUtf32("String"), 0))),
                                                                                                                                                                                                                        new cminor::parsing::ActionParser(ToUtf32("A2"),
                                                                                                                                                                                                                            new cminor::parsing::NonterminalParser(ToUtf32("CharSet"), ToUtf32("CharSet"), 0))),
                                                                                                                                                                                                                    new cminor::parsing::ActionParser(ToUtf32("A3"),
                                                                                                                                                                                                                        new cminor::parsing::NonterminalParser(ToUtf32("Keyword"), ToUtf32("Keyword"), 0))),
                                                                                                                                                                                                                new cminor::parsing::ActionParser(ToUtf32("A4"),
                                                                                                                                                                                                                    new cminor::parsing::NonterminalParser(ToUtf32("KeywordList"), ToUtf32("KeywordList"), 0))),
                                                                                                                                                                                                            new cminor::parsing::ActionParser(ToUtf32("A5"),
                                                                                                                                                                                                                new cminor::parsing::NonterminalParser(ToUtf32("Range"), ToUtf32("Range"), 0))),
                                                                                                                                                                                                        new cminor::parsing::ActionParser(ToUtf32("A6"),
                                                                                                                                                                                                            new cminor::parsing::NonterminalParser(ToUtf32("Empty"), ToUtf32("Empty"), 0))),
                                                                                                                                                                                                    new cminor::parsing::ActionParser(ToUtf32("A7"),
                                                                                                                                                                                                        new cminor::parsing::NonterminalParser(ToUtf32("Space"), ToUtf32("Space"), 0))),
                                                                                                                                                                                                new cminor::parsing::ActionParser(ToUtf32("A8"),
                                                                                                                                                                                                    new cminor::parsing::NonterminalParser(ToUtf32("AnyChar"), ToUtf32("AnyChar"), 0))),
                                                                                                                                                                                            new cminor::parsing::ActionParser(ToUtf32("A9"),
                                                                                                                                                                                                new cminor::parsing::NonterminalParser(ToUtf32("Letter"), ToUtf32("Letter"), 0))),
                                                                                                                                                                                        new cminor::parsing::ActionParser(ToUtf32("A10"),
                                                                                                                                                                                            new cminor::parsing::NonterminalParser(ToUtf32("Digit"), ToUtf32("Digit"), 0))),
                                                                                                                                                                                    new cminor::parsing::ActionParser(ToUtf32("A11"),
                                                                                                                                                                                        new cminor::parsing::NonterminalParser(ToUtf32("HexDigit"), ToUtf32("HexDigit"), 0))),
                                                                                                                                                                                new cminor::parsing::ActionParser(ToUtf32("A12"),
                                                                                                                                                                                    new cminor::parsing::NonterminalParser(ToUtf32("UpperLetter"), ToUtf32("UpperLetter"), 0))),
                                                                                                                                                                            new cminor::parsing::ActionParser(ToUtf32("A13"),
                                                                                                                                                                                new cminor::parsing::NonterminalParser(ToUtf32("LowerLetter"), ToUtf32("LowerLetter"), 0))),
                                                                                                                                                                        new cminor::parsing::ActionParser(ToUtf32("A14"),
                                                                                                                                                                            new cminor::parsing::NonterminalParser(ToUtf32("TitleLetter"), ToUtf32("TitleLetter"), 0))),
                                                                                                                                                                    new cminor::parsing::ActionParser(ToUtf32("A15"),
                                                                                                                                                                        new cminor::parsing::NonterminalParser(ToUtf32("ModifierLetter"), ToUtf32("ModifierLetter"), 0))),
                                                                                                                                                                new cminor::parsing::ActionParser(ToUtf32("A16"),
                                                                                                                                                                    new cminor::parsing::NonterminalParser(ToUtf32("OtherLetter"), ToUtf32("OtherLetter"), 0))),
                                                                                                                                                            new cminor::parsing::ActionParser(ToUtf32("A17"),
                                                                                                                                                                new cminor::parsing::NonterminalParser(ToUtf32("CasedLetter"), ToUtf32("CasedLetter"), 0))),
                                                                                                                                                        new cminor::parsing::ActionParser(ToUtf32("A18"),
                                                                                                                                                            new cminor::parsing::NonterminalParser(ToUtf32("Mark"), ToUtf32("Mark"), 0))),
                                                                                                                                                    new cminor::parsing::ActionParser(ToUtf32("A19"),
                                                                                                                                                        new cminor::parsing::NonterminalParser(ToUtf32("NonspacingMark"), ToUtf32("NonspacingMark"), 0))),
                                                                                                                                                new cminor::parsing::ActionParser(ToUtf32("A20"),
                                                                                                                                                    new cminor::parsing::NonterminalParser(ToUtf32("EnclosingMark"), ToUtf32("EnclosingMark"), 0))),
                                                                                                                                            new cminor::parsing::ActionParser(ToUtf32("A21"),
                                                                                                                                                new cminor::parsing::NonterminalParser(ToUtf32("SpacingMark"), ToUtf32("SpacingMark"), 0))),
                                                                                                                                        new cminor::parsing::ActionParser(ToUtf32("A22"),
                                                                                                                                            new cminor::parsing::NonterminalParser(ToUtf32("Number"), ToUtf32("Number"), 0))),
                                                                                                                                    new cminor::parsing::ActionParser(ToUtf32("A23"),
                                                                                                                                        new cminor::parsing::NonterminalParser(ToUtf32("DecimalNumber"), ToUtf32("DecimalNumber"), 0))),
                                                                                                                                new cminor::parsing::ActionParser(ToUtf32("A24"),
                                                                                                                                    new cminor::parsing::NonterminalParser(ToUtf32("LetterNumber"), ToUtf32("LetterNumber"), 0))),
                                                                                                                            new cminor::parsing::ActionParser(ToUtf32("A25"),
                                                                                                                                new cminor::parsing::NonterminalParser(ToUtf32("OtherNumber"), ToUtf32("OtherNumber"), 0))),
                                                                                                                        new cminor::parsing::ActionParser(ToUtf32("A26"),
                                                                                                                            new cminor::parsing::NonterminalParser(ToUtf32("Punctuation"), ToUtf32("Punctuation"), 0))),
                                                                                                                    new cminor::parsing::ActionParser(ToUtf32("A27"),
                                                                                                                        new cminor::parsing::NonterminalParser(ToUtf32("ConnectorPunctuation"), ToUtf32("ConnectorPunctuation"), 0))),
                                                                                                                new cminor::parsing::ActionParser(ToUtf32("A28"),
                                                                                                                    new cminor::parsing::NonterminalParser(ToUtf32("DashPunctuation"), ToUtf32("DashPunctuation"), 0))),
                                                                                                            new cminor::parsing::ActionParser(ToUtf32("A29"),
                                                                                                                new cminor::parsing::NonterminalParser(ToUtf32("OpenPunctuation"), ToUtf32("OpenPunctuation"), 0))),
                                                                                                        new cminor::parsing::ActionParser(ToUtf32("A30"),
                                                                                                            new cminor::parsing::NonterminalParser(ToUtf32("ClosePunctuation"), ToUtf32("ClosePunctuation"), 0))),
                                                                                                    new cminor::parsing::ActionParser(ToUtf32("A31"),
                                                                                                        new cminor::parsing::NonterminalParser(ToUtf32("InitialPunctuation"), ToUtf32("InitialPunctuation"), 0))),
                                                                                                new cminor::parsing::ActionParser(ToUtf32("A32"),
                                                                                                    new cminor::parsing::NonterminalParser(ToUtf32("FinalPunctuation"), ToUtf32("FinalPunctuation"), 0))),
                                                                                            new cminor::parsing::ActionParser(ToUtf32("A33"),
                                                                                                new cminor::parsing::NonterminalParser(ToUtf32("OtherPunctuation"), ToUtf32("OtherPunctuation"), 0))),
                                                                                        new cminor::parsing::ActionParser(ToUtf32("A34"),
                                                                                            new cminor::parsing::NonterminalParser(ToUtf32("Symbol"), ToUtf32("Symbol"), 0))),
                                                                                    new cminor::parsing::ActionParser(ToUtf32("A35"),
                                                                                        new cminor::parsing::NonterminalParser(ToUtf32("MathSymbol"), ToUtf32("MathSymbol"), 0))),
                                                                                new cminor::parsing::ActionParser(ToUtf32("A36"),
                                                                                    new cminor::parsing::NonterminalParser(ToUtf32("CurrencySymbol"), ToUtf32("CurrencySymbol"), 0))),
                                                                            new cminor::parsing::ActionParser(ToUtf32("A37"),
                                                                                new cminor::parsing::NonterminalParser(ToUtf32("ModifierSymbol"), ToUtf32("ModifierSymbol"), 0))),
                                                                        new cminor::parsing::ActionParser(ToUtf32("A38"),
                                                                            new cminor::parsing::NonterminalParser(ToUtf32("OtherSymbol"), ToUtf32("OtherSymbol"), 0))),
                                                                    new cminor::parsing::ActionParser(ToUtf32("A39"),
                                                                        new cminor::parsing::NonterminalParser(ToUtf32("Separator"), ToUtf32("Separator"), 0))),
                                                                new cminor::parsing::ActionParser(ToUtf32("A40"),
                                                                    new cminor::parsing::NonterminalParser(ToUtf32("SpaceSeparator"), ToUtf32("SpaceSeparator"), 0))),
                                                            new cminor::parsing::ActionParser(ToUtf32("A41"),
                                                                new cminor::parsing::NonterminalParser(ToUtf32("LineSeparator"), ToUtf32("LineSeparator"), 0))),
                                                        new cminor::parsing::ActionParser(ToUtf32("A42"),
                                                            new cminor::parsing::NonterminalParser(ToUtf32("ParagraphSeparator"), ToUtf32("ParagraphSeparator"), 0))),
                                                    new cminor::parsing::ActionParser(ToUtf32("A43"),
                                                        new cminor::parsing::NonterminalParser(ToUtf32("Other"), ToUtf32("Other"), 0))),
                                                new cminor::parsing::ActionParser(ToUtf32("A44"),
                                                    new cminor::parsing::NonterminalParser(ToUtf32("Control"), ToUtf32("Control"), 0))),
                                            new cminor::parsing::ActionParser(ToUtf32("A45"),
                                                new cminor::parsing::NonterminalParser(ToUtf32("Format"), ToUtf32("Format"), 0))),
                                        new cminor::parsing::ActionParser(ToUtf32("A46"),
                                            new cminor::parsing::NonterminalParser(ToUtf32("Surrogate"), ToUtf32("Surrogate"), 0))),
                                    new cminor::parsing::ActionParser(ToUtf32("A47"),
                                        new cminor::parsing::NonterminalParser(ToUtf32("PrivateUse"), ToUtf32("PrivateUse"), 0))),
                                new cminor::parsing::ActionParser(ToUtf32("A48"),
                                    new cminor::parsing::NonterminalParser(ToUtf32("Unassigned"), ToUtf32("Unassigned"), 0))),
                            new cminor::parsing::ActionParser(ToUtf32("A49"),
                                new cminor::parsing::NonterminalParser(ToUtf32("Graphic"), ToUtf32("Graphic"), 0))),
                        new cminor::parsing::ActionParser(ToUtf32("A50"),
                            new cminor::parsing::NonterminalParser(ToUtf32("BaseChar"), ToUtf32("BaseChar"), 0))),
                    new cminor::parsing::ActionParser(ToUtf32("A51"),
                        new cminor::parsing::NonterminalParser(ToUtf32("Alphabetic"), ToUtf32("Alphabetic"), 0))),
                new cminor::parsing::ActionParser(ToUtf32("A52"),
                    new cminor::parsing::NonterminalParser(ToUtf32("IdStart"), ToUtf32("IdStart"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A53"),
                new cminor::parsing::NonterminalParser(ToUtf32("IdCont"), ToUtf32("IdCont"), 0)))));
    AddRule(new CharRule(ToUtf32("Char"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::NonterminalParser(ToUtf32("chr"), ToUtf32("char"), 0))));
    AddRule(new StringRule(ToUtf32("String"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::NonterminalParser(ToUtf32("str"), ToUtf32("string"), 0))));
    AddRule(new CharSetRule(ToUtf32("CharSet"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::CharParser('['),
                            new cminor::parsing::OptionalParser(
                                new cminor::parsing::ActionParser(ToUtf32("A1"),
                                    new cminor::parsing::CharParser('^')))),
                        new cminor::parsing::KleeneStarParser(
                            new cminor::parsing::ActionParser(ToUtf32("A2"),
                                new cminor::parsing::NonterminalParser(ToUtf32("CharSetRange"), ToUtf32("CharSetRange"), 0)))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser(']')))))));
    AddRule(new CharSetRangeRule(ToUtf32("CharSetRange"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::NonterminalParser(ToUtf32("first"), ToUtf32("CharSetChar"), 0),
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('-'),
                    new cminor::parsing::ActionParser(ToUtf32("A0"),
                        new cminor::parsing::NonterminalParser(ToUtf32("last"), ToUtf32("CharSetChar"), 0))),
                new cminor::parsing::ActionParser(ToUtf32("A1"),
                    new cminor::parsing::EmptyParser())))));
    AddRule(new CharSetCharRule(ToUtf32("CharSetChar"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::CharSetParser(ToUtf32("\\]"), true)),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::NonterminalParser(ToUtf32("escape"), ToUtf32("escape"), 0)))));
    AddRule(new KeywordRule(ToUtf32("Keyword"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser(ToUtf32("keyword")),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('('))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("KeywordBody"), ToUtf32("KeywordBody"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))))));
    AddRule(new KeywordBodyRule(ToUtf32("KeywordBody"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::NonterminalParser(ToUtf32("str"), ToUtf32("string"), 0),
            new cminor::parsing::AlternativeParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser(','),
                    new cminor::parsing::ActionParser(ToUtf32("A0"),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::NonterminalParser(ToUtf32("continuation"), ToUtf32("QualifiedId"), 0)))),
                new cminor::parsing::ActionParser(ToUtf32("A1"),
                    new cminor::parsing::EmptyParser())))));
    AddRule(new KeywordListRule(ToUtf32("KeywordList"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::KeywordParser(ToUtf32("keyword_list")),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::CharParser('('))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("KeywordListBody"), ToUtf32("KeywordListBody"), 0))),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))))));
    AddRule(new KeywordListBodyRule(ToUtf32("KeywordListBody"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::SequenceParser(
            new cminor::parsing::SequenceParser(
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("selector"), ToUtf32("QualifiedId"), 0)),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(','))),
            new cminor::parsing::ActionParser(ToUtf32("A0"),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::NonterminalParser(ToUtf32("StringArray"), ToUtf32("StringArray"), 1))))));
    AddRule(new RangeRule(ToUtf32("Range"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::SequenceParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::SequenceParser(
                        new cminor::parsing::SequenceParser(
                            new cminor::parsing::SequenceParser(
                                new cminor::parsing::KeywordParser(ToUtf32("range")),
                                new cminor::parsing::ExpectationParser(
                                    new cminor::parsing::CharParser('('))),
                            new cminor::parsing::ExpectationParser(
                                new cminor::parsing::NonterminalParser(ToUtf32("s"), ToUtf32("CodePoint"), 0))),
                        new cminor::parsing::ExpectationParser(
                            new cminor::parsing::CharParser(','))),
                    new cminor::parsing::ExpectationParser(
                        new cminor::parsing::NonterminalParser(ToUtf32("e"), ToUtf32("CodePoint"), 0))),
                new cminor::parsing::ExpectationParser(
                    new cminor::parsing::CharParser(')'))))));
    AddRule(new CodePointRule(ToUtf32("CodePoint"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::AlternativeParser(
            new cminor::parsing::TokenParser(
                new cminor::parsing::SequenceParser(
                    new cminor::parsing::CharParser('x'),
                    new cminor::parsing::ActionParser(ToUtf32("A0"),
                        new cminor::parsing::NonterminalParser(ToUtf32("x"), ToUtf32("hexuint"), 0)))),
            new cminor::parsing::ActionParser(ToUtf32("A1"),
                new cminor::parsing::NonterminalParser(ToUtf32("d"), ToUtf32("uint"), 0)))));
    AddRule(new EmptyRule(ToUtf32("Empty"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("empty")))));
    AddRule(new SpaceRule(ToUtf32("Space"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("space")))));
    AddRule(new AnyCharRule(ToUtf32("AnyChar"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("anychar")))));
    AddRule(new LetterRule(ToUtf32("Letter"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("letter")))));
    AddRule(new DigitRule(ToUtf32("Digit"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("digit")))));
    AddRule(new HexDigitRule(ToUtf32("HexDigit"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("hexdigit")))));
    AddRule(new UpperLetterRule(ToUtf32("UpperLetter"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("upper_letter")))));
    AddRule(new LowerLetterRule(ToUtf32("LowerLetter"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("lower_letter")))));
    AddRule(new TitleLetterRule(ToUtf32("TitleLetter"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("title_letter")))));
    AddRule(new ModifierLetterRule(ToUtf32("ModifierLetter"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("modifier_letter")))));
    AddRule(new OtherLetterRule(ToUtf32("OtherLetter"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("other_letter")))));
    AddRule(new CasedLetterRule(ToUtf32("CasedLetter"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("cased_letter")))));
    AddRule(new MarkRule(ToUtf32("Mark"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("mark")))));
    AddRule(new NonspacingMarkRule(ToUtf32("NonspacingMark"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("nonspacing_mark")))));
    AddRule(new SpacingMarkRule(ToUtf32("SpacingMark"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("spacing_mark")))));
    AddRule(new EnclosingMarkRule(ToUtf32("EnclosingMark"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("enclosing_mark")))));
    AddRule(new NumberRule(ToUtf32("Number"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("numbr")))));
    AddRule(new DecimalNumberRule(ToUtf32("DecimalNumber"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("decimal_number")))));
    AddRule(new LetterNumberRule(ToUtf32("LetterNumber"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("letter_number")))));
    AddRule(new OtherNumberRule(ToUtf32("OtherNumber"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("other_number")))));
    AddRule(new PunctuationRule(ToUtf32("Punctuation"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("punctuation")))));
    AddRule(new ConnectorPunctuationRule(ToUtf32("ConnectorPunctuation"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("connector_punctuation")))));
    AddRule(new DashPunctuationRule(ToUtf32("DashPunctuation"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("dash_punctuation")))));
    AddRule(new OpenPunctuationRule(ToUtf32("OpenPunctuation"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("open_punctuation")))));
    AddRule(new ClosePunctuationRule(ToUtf32("ClosePunctuation"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("close_punctuation")))));
    AddRule(new InitialPunctuationRule(ToUtf32("InitialPunctuation"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("initial_punctuation")))));
    AddRule(new FinalPunctuationRule(ToUtf32("FinalPunctuation"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("final_punctuation")))));
    AddRule(new OtherPunctuationRule(ToUtf32("OtherPunctuation"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("other_punctuation")))));
    AddRule(new SymbolRule(ToUtf32("Symbol"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("symbol")))));
    AddRule(new MathSymbolRule(ToUtf32("MathSymbol"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("math_symbol")))));
    AddRule(new CurrencySymbolRule(ToUtf32("CurrencySymbol"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("currency_symbol")))));
    AddRule(new ModifierSymbolRule(ToUtf32("ModifierSymbol"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("modifier_symbol")))));
    AddRule(new OtherSymbolRule(ToUtf32("OtherSymbol"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("other_symbol")))));
    AddRule(new SeparatorRule(ToUtf32("Separator"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("separator")))));
    AddRule(new SpaceSeparatorRule(ToUtf32("SpaceSeparator"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("space_separator")))));
    AddRule(new LineSeparatorRule(ToUtf32("LineSeparator"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("line_separator")))));
    AddRule(new ParagraphSeparatorRule(ToUtf32("ParagraphSeparator"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("paragraph_separator")))));
    AddRule(new OtherRule(ToUtf32("Other"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("other")))));
    AddRule(new ControlRule(ToUtf32("Control"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("control")))));
    AddRule(new FormatRule(ToUtf32("Format"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("format")))));
    AddRule(new SurrogateRule(ToUtf32("Surrogate"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("surrogate")))));
    AddRule(new PrivateUseRule(ToUtf32("PrivateUse"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("private_use")))));
    AddRule(new UnassignedRule(ToUtf32("Unassigned"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("unassigned")))));
    AddRule(new GraphicRule(ToUtf32("Graphic"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("graphic")))));
    AddRule(new BaseCharRule(ToUtf32("BaseChar"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("basechar")))));
    AddRule(new AlphabeticRule(ToUtf32("Alphabetic"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("alphabetic")))));
    AddRule(new IdStartRule(ToUtf32("IdStart"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("idstart")))));
    AddRule(new IdContRule(ToUtf32("IdCont"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cminor::parsing::ActionParser(ToUtf32("A0"),
            new cminor::parsing::KeywordParser(ToUtf32("idcont")))));
}

} } // namespace cminor.syntax
