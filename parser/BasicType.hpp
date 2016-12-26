#ifndef BasicType_hpp_27569
#define BasicType_hpp_27569

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/ast/BasicType.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class BasicTypeGrammar : public cminor::parsing::Grammar
{
public:
    static BasicTypeGrammar* Create();
    static BasicTypeGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    Node* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName);
private:
    BasicTypeGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class BasicTypeRule;
};

} } // namespace cminor.parser

#endif // BasicType_hpp_27569
