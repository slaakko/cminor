#ifndef ProjectFile_hpp_27058
#define ProjectFile_hpp_27058

#include <Cm.Parsing/Grammar.hpp>
#include <Cm.Parsing/Keyword.hpp>
#include <cminor/ast/Project.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class ProjectGrammar : public Cm::Parsing::Grammar
{
public:
    static ProjectGrammar* Create();
    static ProjectGrammar* Create(Cm::Parsing::ParsingDomain* parsingDomain);
    Project* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, std::string config);
private:
    ProjectGrammar(Cm::Parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class ProjectRule;
    class DeclarationRule;
    class AssemblyReferenceDeclarationRule;
    class SourceFileDeclarationRule;
    class TargetDeclarationRule;
    class TargetRule;
    class FilePathRule;
};

} } // namespace cminor.parser

#endif // ProjectFile_hpp_27058