#ifndef CommandGrammar_hpp_17145
#define CommandGrammar_hpp_17145

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/db/Command.hpp>

namespace cminor { namespace db {

class CommandGrammar : public cminor::parsing::Grammar
{
public:
    static CommandGrammar* Create();
    static CommandGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    Command* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    CommandGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class CommandRule;
    class FilePathRule;
    class QualifiedIdRule;
};

} } // namespace cminor.db

#endif // CommandGrammar_hpp_17145
