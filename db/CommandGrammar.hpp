#ifndef CommandGrammar_hpp_7786
#define CommandGrammar_hpp_7786

#include <Cm.Parsing/Grammar.hpp>
#include <Cm.Parsing/Keyword.hpp>
#include <cminor/db/Command.hpp>

namespace cminor { namespace db {

class CommandGrammar : public Cm::Parsing::Grammar
{
public:
    static CommandGrammar* Create();
    static CommandGrammar* Create(Cm::Parsing::ParsingDomain* parsingDomain);
    Command* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName);
private:
    CommandGrammar(Cm::Parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class CommandRule;
};

} } // namespace cminor.db

#endif // CommandGrammar_hpp_7786
