#ifndef Identifier_hpp_5397
#define Identifier_hpp_5397

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>

namespace cpg { namespace cpp {

class IdentifierGrammar : public cminor::parsing::Grammar
{
public:
    static IdentifierGrammar* Create();
    static IdentifierGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    std::string Parse(const char* start, const char* end, int fileIndex, const std::string& fileName);
private:
    IdentifierGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class IdentifierRule;
    class QualifiedIdRule;
};

} } // namespace cpg.cpp

#endif // Identifier_hpp_5397
