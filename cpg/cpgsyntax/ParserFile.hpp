#ifndef ParserFile_hpp_8022
#define ParserFile_hpp_8022

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/cpg/cpgsyntax/ParserFileContent.hpp>

namespace cpg { namespace syntax {

class ParserFileGrammar : public cminor::parsing::Grammar
{
public:
    static ParserFileGrammar* Create();
    static ParserFileGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    ParserFileContent* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, int id_, cminor::parsing::ParsingDomain* parsingDomain_);
private:
    ParserFileGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class ParserFileRule;
    class IncludeDirectivesRule;
    class IncludeDirectiveRule;
    class FileAttributeRule;
    class IncludeFileNameRule;
    class NamespaceContentRule;
    class NamespaceRule;
};

} } // namespace cpg.syntax

#endif // ParserFile_hpp_8022
