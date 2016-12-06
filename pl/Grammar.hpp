/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#ifndef CMINOR_PARSING_GRAMMAR_INCLUDED
#define CMINOR_PARSING_GRAMMAR_INCLUDED
#include <cminor/pl/Parser.hpp>
#include <unordered_set>

namespace cminor { namespace parsing {

class Grammar;
typedef std::unordered_set<Grammar*> GrammarSet;
typedef GrammarSet::const_iterator GrammarSetIt;

class Rule;
class ParsingDomain;
class Namespace;
class RuleLink;

class Grammar: public ParsingObject
{
public:
    typedef std::vector<Rule*> RuleVec;
    Grammar(const std::string& name_, Scope* enclosingScope_);
    Grammar(const std::string& name_, Scope* enclosingScope_, ParsingDomain* parsingDomain_);
    void SetStartRuleName(const std::string& startRuleName_) { startRuleName = startRuleName_; }
    void SetSkipRuleName(const std::string& skipRuleName_) { skipRuleName = skipRuleName_; }
    void SetCCRuleData(const std::string& ccRuleName_, char ccStart_, char ccEnd_, bool ccSkip_); 
    void AddRule(Rule* rule);
    Rule* GetRule(const std::string& ruleName) const;
    virtual void GetReferencedGrammars() {}
    virtual void CreateRules() {}
    virtual void Link();
    void AddGrammarReference(Grammar* grammarReference);
    const GrammarSet& GrammarReferences() const { return grammarReferences; }
    void Accept(Visitor& visitor);
    void Parse(const char* start, const char* end, int fileIndex, const std::string& fileName);
    Match Parse(Scanner& scanner, ObjectStack& stack);
    const std::string& StartRuleName() const { return startRuleName; }
    Rule* StartRule() const { return startRule; }
    const std::string& SkipRuleName() const { return skipRuleName; }
    Rule* SkipRule() const { return skipRule; }
    std::ostream* Log() const { return log; }
    void SetLog(std::ostream* log_) { log = log_; }
    int MaxLogLineLength() const { return maxLogLineLength; }
    void SetMaxLogLineLength(int maxLogLineLength_) { maxLogLineLength = maxLogLineLength_; }
    ParsingDomain* GetParsingDomain() const { return parsingDomain; }
    const RuleVec& Rules() const { return rules; }
    Namespace* Ns() const { return ns; }
    void SetNs(Namespace* ns_) { ns = ns_; }
    void AddRuleLink(RuleLink* ruleLink);
    void ResolveStartRule();
    void ResolveSkipRule();
    const std::string& CCRuleName() const { return ccRuleName; }
    char CCStart() const { return ccStart; }
    char CCEnd() const { return ccEnd; }
    bool CCSkip() const { return ccSkip; }
private:
    ParsingDomain* parsingDomain;
    Namespace* ns;
    std::string startRuleName;
    std::string skipRuleName;
    std::string ccRuleName;
    char ccStart;
    char ccEnd;
    bool ccSkip;
    RuleVec rules;
    bool linking;
    bool linked;
    Parser* contentParser;
    GrammarSet grammarReferences;
    typedef std::unordered_set<RuleLink*> RuleLinkSet;
    typedef RuleLinkSet::const_iterator RuleLinkSetIt;
    RuleLinkSet ruleLinks;
    Rule* startRule;
    Rule* skipRule;
    std::ostream* log;
    int maxLogLineLength;
};

} } // namespace Cm::Parsing

#endif // CMINOR_PARSING_GRAMMAR_INCLUDED
