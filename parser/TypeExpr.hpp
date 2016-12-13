#ifndef TypeExpr_hpp_1192
#define TypeExpr_hpp_1192

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/ast/Expression.hpp>
#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class TypeExprGrammar : public cminor::parsing::Grammar
{
public:
    static TypeExprGrammar* Create();
    static TypeExprGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    Node* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    TypeExprGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class TypeExprRule;
    class PostfixTypeExprRule;
    class PrimaryTypeExprRule;
};

} } // namespace cminor.parser

#endif // TypeExpr_hpp_1192
