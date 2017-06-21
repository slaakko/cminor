#ifndef ProjectFile_hpp_20364
#define ProjectFile_hpp_20364

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/syntax/Project.hpp>

namespace cminor { namespace syntax {

class ProjectFileGrammar : public cminor::parsing::Grammar
{
public:
    static ProjectFileGrammar* Create();
    static ProjectFileGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    Project* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
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

} } // namespace cminor.syntax

#endif // ProjectFile_hpp_20364
