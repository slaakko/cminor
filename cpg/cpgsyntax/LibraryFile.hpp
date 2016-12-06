#ifndef LibraryFile_hpp_8022
#define LibraryFile_hpp_8022

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/pl/ParsingDomain.hpp>

namespace cpg { namespace syntax {

class LibraryFileGrammar : public cminor::parsing::Grammar
{
public:
    static LibraryFileGrammar* Create();
    static LibraryFileGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    void Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, cminor::parsing::ParsingDomain* parsingDomain);
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

} } // namespace cpg.syntax

#endif // LibraryFile_hpp_8022
