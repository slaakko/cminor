#ifndef LibraryFile_hpp_20364
#define LibraryFile_hpp_20364

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/parsing/ParsingDomain.hpp>

namespace cminor { namespace syntax {

class LibraryFileGrammar : public cminor::parsing::Grammar
{
public:
    static LibraryFileGrammar* Create();
    static LibraryFileGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    void Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, cminor::parsing::ParsingDomain* parsingDomain);
private:
    LibraryFileGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class LibraryFileRule;
    class NamespaceContentRule;
    class NamespaceRule;
    class GrammarRule;
    class GrammarContentRule;
    class RuleRule;
};

} } // namespace cminor.syntax

#endif // LibraryFile_hpp_20364
