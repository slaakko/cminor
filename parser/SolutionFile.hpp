#ifndef SolutionFile_hpp_6205
#define SolutionFile_hpp_6205

#include <Cm.Parsing/Grammar.hpp>
#include <Cm.Parsing/Keyword.hpp>
#include <cminor/ast/Solution.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class SolutionGrammar : public Cm::Parsing::Grammar
{
public:
    static SolutionGrammar* Create();
    static SolutionGrammar* Create(Cm::Parsing::ParsingDomain* parsingDomain);
    Solution* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName);
private:
    SolutionGrammar(Cm::Parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class SolutionRule;
    class DeclarationRule;
    class SolutionProjectDeclarationRule;
    class FilePathRule;
};

} } // namespace cminor.parser

#endif // SolutionFile_hpp_6205
