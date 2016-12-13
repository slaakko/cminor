#ifndef SolutionFile_hpp_1192
#define SolutionFile_hpp_1192

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/ast/Solution.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class SolutionGrammar : public cminor::parsing::Grammar
{
public:
    static SolutionGrammar* Create();
    static SolutionGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    Solution* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName);
private:
    SolutionGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class SolutionRule;
    class DeclarationRule;
    class SolutionProjectDeclarationRule;
    class FilePathRule;
};

} } // namespace cminor.parser

#endif // SolutionFile_hpp_1192
