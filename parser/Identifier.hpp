#ifndef Identifier_hpp_11565
#define Identifier_hpp_11565

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/ast/Identifier.hpp>

namespace cminor { namespace parser {

using cminor::ast::IdentifierNode;
class IdentifierGrammar : public cminor::parsing::Grammar
{
public:
    static IdentifierGrammar* Create();
    static IdentifierGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    IdentifierNode* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName);
private:
    IdentifierGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class IdentifierRule;
    class QualifiedIdRule;
};

} } // namespace cminor.parser

#endif // Identifier_hpp_11565
