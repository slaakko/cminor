#ifndef CommandGrammar_hpp_354
#define CommandGrammar_hpp_354

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/db/Command.hpp>

namespace cminor { namespace db {

class CommandGrammar : public cminor::parsing::Grammar
{
public:
    static CommandGrammar* Create();
    static CommandGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    Command* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName);
private:
    CommandGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class CommandRule;
    class FilePathRule;
    class QualifiedIdRule;
};

} } // namespace cminor.db

#endif // CommandGrammar_hpp_354
