#ifndef ProjectFile_hpp_5596
#define ProjectFile_hpp_5596

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/cpg/cpgsyntax/Project.hpp>

namespace cpg { namespace syntax {

class ProjectFileGrammar : public cminor::parsing::Grammar
{
public:
    static ProjectFileGrammar* Create();
    static ProjectFileGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    Project* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName);
private:
    ProjectFileGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class ProjectFileRule;
    class ProjectFileContentRule;
    class SourceRule;
    class ReferenceRule;
    class FilePathRule;
};

} } // namespace cpg.syntax

#endif // ProjectFile_hpp_5596
