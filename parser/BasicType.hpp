#ifndef BasicType_hpp_21399
#define BasicType_hpp_21399

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/ast/BasicType.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class BasicTypeGrammar : public cminor::parsing::Grammar
{
public:
    static BasicTypeGrammar* Create();
    static BasicTypeGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    Node* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    BasicTypeGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class BasicTypeRule;
};

} } // namespace cminor.parser

#endif // BasicType_hpp_21399
