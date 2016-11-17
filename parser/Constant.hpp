#ifndef Constant_hpp_4617
#define Constant_hpp_4617

#include <Cm.Parsing/Grammar.hpp>
#include <Cm.Parsing/Keyword.hpp>
#include <cminor/ast/Constant.hpp>
#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class ConstantGrammar : public Cm::Parsing::Grammar
{
public:
    static ConstantGrammar* Create();
    static ConstantGrammar* Create(Cm::Parsing::ParsingDomain* parsingDomain);
    ConstantNode* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    ConstantGrammar(Cm::Parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class ConstantRule;
};

} } // namespace cminor.parser

#endif // Constant_hpp_4617
