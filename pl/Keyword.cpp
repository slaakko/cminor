// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/pl/Keyword.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Primitive.hpp>
#include <cminor/pl/Visitor.hpp>

namespace cminor { namespace parsing {

class IdentifierCharsRule
{
public:
    static void Init();
    static void Done();
    static IdentifierCharsRule& Instance();
    Rule* GetRule() const{ return rule.get(); }
private:
    IdentifierCharsRule();
    std::unique_ptr<Rule> rule;
    static std::unique_ptr<IdentifierCharsRule> instance;
    static int initCount;
};

IdentifierCharsRule::IdentifierCharsRule(): 
    rule(new Rule("identifier_chars", nullptr, -1,
            new PositiveParser(
                new AlternativeParser(
                    new AlternativeParser(
                        new AlternativeParser(
                            new LetterParser(),
                            new DigitParser()),
                        new CharParser('_')),
                    new CharParser('.')))))
{
    rule->SetOwned();
}

void IdentifierCharsRule::Init()
{
    if (initCount++ == 0)
    {
        instance.reset(new IdentifierCharsRule());
    }
}

void IdentifierCharsRule::Done()
{
    if (--initCount == 0)
    {
        instance.reset();
    }
}

IdentifierCharsRule& IdentifierCharsRule::Instance()
{
    return *instance;
}

std::unique_ptr<IdentifierCharsRule> IdentifierCharsRule::instance;

int IdentifierCharsRule::initCount = 0;

KeywordParser::KeywordParser(const std::string& keyword_): Parser("keyword", "\"" + keyword_ + "\""), keyword(keyword_), continuationRuleName() 
{
    continuationRule = IdentifierCharsRule::Instance().GetRule();
    keywordStringParser = new StringParser(keyword);
    Own(keywordStringParser);
    CreateKeywordRule();
}

KeywordParser::KeywordParser(const std::string& keyword_, const std::string& continuationRuleName_): Parser("keyword", "\"" + keyword_ + "\""), keyword(keyword_), continuationRuleName(continuationRuleName_) 
{
    continuationRule = IdentifierCharsRule::Instance().GetRule();
    keywordStringParser = new StringParser(keyword);
    Own(keywordStringParser);
    CreateKeywordRule();
}

void KeywordParser::SetContinuationRule(Rule* continuationRule_)
{
    if (continuationRule_)
    {
        continuationRule = continuationRule_;
        Own(continuationRule);
        CreateKeywordRule();
    }
}

void KeywordParser::CreateKeywordRule()
{
    keywordRule = new Rule(keyword, nullptr, -1,
        new DifferenceParser(
            keywordStringParser,
            new TokenParser(
                new SequenceParser(
                    keywordStringParser,
                    continuationRule))));
    Own(keywordRule);
}

Match KeywordParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    return keywordRule->Parse(scanner, stack, parsingData);
}

void KeywordParser::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

KeywordListParser::KeywordListParser(const std::string& selectorRuleName_, const std::vector<std::string>& keywords_): 
    Parser("keywordList", "keyword_list"), selectorRuleName(selectorRuleName_), keywords(keywords_.begin(), keywords_.end())
{
}

Match KeywordListParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    if (selectorRule)
    {
        Span save = scanner.GetSpan();
        bool synchronizing = scanner.Synchronizing();
        Match match = selectorRule->Parse(scanner, stack, parsingData);
        if (match.Hit())
        {
            const char* matchBegin = scanner.Start() + save.Start();
            const char* matchEnd = scanner.Start() + scanner.GetSpan().Start();
            std::string keyword(matchBegin, matchEnd);
            if (keywords.find(keyword) != keywords.end())
            {
                return match;
            }
            scanner.SetSpan(save);
            scanner.SetSynchronizing(synchronizing);
        }
    }
    return Match::Nothing();
}

void KeywordListParser::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void KeywordInit()
{
    IdentifierCharsRule::Init();
}

void KeywordDone()
{
    IdentifierCharsRule::Done();
}

} } // namespace cminor::parsing
