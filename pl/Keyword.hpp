/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#ifndef CMINOR_PARSING_KEYWORD_INCLUDED
#define CMINOR_PARSING_KEYWORD_INCLUDED
#include <cminor/pl/Parser.hpp>
#include <set>

namespace cminor { namespace parsing {

class Rule;

class KeywordParser : public Parser
{
public:
    KeywordParser(const std::string& keyword_);
    KeywordParser(const std::string& keyword_, const std::string& continuationRuleName_);
    const std::string& Keyword() const { return keyword; }
    const std::string& ContinuationRuleName() const { return continuationRuleName; }
    void SetContinuationRule(Rule* continuationRule_);
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
private:
    std::string keyword;
    std::string continuationRuleName;
    Rule* continuationRule;
    Parser* keywordStringParser;
    Rule* keywordRule;
    void CreateKeywordRule();
};

class KeywordListParser : public Parser
{
public:
    typedef std::set<std::string> KeywordSet;
    typedef KeywordSet::const_iterator KeywordSetIt;
    KeywordListParser(const std::string& selectorRuleName_, const std::vector<std::string>& keywords_);
    const std::string& SelectorRuleName() const { return selectorRuleName; }
    const std::string& KeywordVecName() const { return keywordVecName; }
    std::string& KeywordVecName() { return keywordVecName; }
    const KeywordSet& Keywords() const { return keywords; }
    void SetSelectorRule(Rule* selectorRule_) { selectorRule = selectorRule_; }
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
private:
    std::string selectorRuleName;
    std::string keywordVecName;
    KeywordSet keywords;
    Rule* selectorRule;
};

void KeywordInit();
void KeywordDone();

} } // namespace Cm::Parsing

#endif // CMINOR_PARSING_KEYWORD_INCLUDED
