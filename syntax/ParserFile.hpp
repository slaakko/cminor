#ifndef ParserFile_hpp_20364
#define ParserFile_hpp_20364

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/syntax/ParserFileContent.hpp>

namespace cminor { namespace syntax {

class ParserFileGrammar : public cminor::parsing::Grammar
{
public:
    static ParserFileGrammar* Create();
    static ParserFileGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    ParserFileContent* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, int id_, cminor::parsing::ParsingDomain* parsingDomain_);
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

} } // namespace cminor.syntax

#endif // ParserFile_hpp_20364
