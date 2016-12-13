#ifndef ProjectFile_hpp_1192
#define ProjectFile_hpp_1192

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/ast/Project.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class ProjectGrammar : public cminor::parsing::Grammar
{
public:
    static ProjectGrammar* Create();
    static ProjectGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    Project* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, std::string config);
private:
    ProjectGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class ProjectRule;
    class DeclarationRule;
    class ReferenceDeclarationRule;
    class SourceFileDeclarationRule;
    class TargetDeclarationRule;
    class TargetRule;
    class FilePathRule;
};

} } // namespace cminor.parser

#endif // ProjectFile_hpp_1192
