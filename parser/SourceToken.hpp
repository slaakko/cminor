#ifndef SourceToken_hpp_27813
#define SourceToken_hpp_27813

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/ast/SourceToken.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class SourceTokenGrammar : public cminor::parsing::Grammar
{
public:
    static SourceTokenGrammar* Create();
    static SourceTokenGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    void Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, SourceTokenFormatter* formatter);
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

#endif // SourceToken_hpp_27813
