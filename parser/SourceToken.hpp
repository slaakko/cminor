#ifndef SourceToken_hpp_21399
#define SourceToken_hpp_21399

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/ast/SourceToken.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class SourceTokenGrammar : public cminor::parsing::Grammar
{
public:
    static SourceTokenGrammar* Create();
    static SourceTokenGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    void Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, SourceTokenFormatter* formatter);
private:
    SourceTokenGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class SourceTokensRule;
    class SourceTokenRule;
    class SpacesRule;
    class OtherRule;
};

} } // namespace cminor.parser

#endif // SourceToken_hpp_21399
