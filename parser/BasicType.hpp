#ifndef BasicType_hpp_1995
#define BasicType_hpp_1995

#include <Cm.Parsing/Grammar.hpp>
#include <Cm.Parsing/Keyword.hpp>
#include <cminor/ast/BasicType.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class BasicTypeGrammar : public Cm::Parsing::Grammar
{
public:
    static BasicTypeGrammar* Create();
    static BasicTypeGrammar* Create(Cm::Parsing::ParsingDomain* parsingDomain);
    Node* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName);
private:
    BasicTypeGrammar(Cm::Parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class BasicTypeRule;
};

} } // namespace cminor.parser

#endif // BasicType_hpp_1995
