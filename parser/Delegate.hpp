#ifndef Delegate_hpp_7033
#define Delegate_hpp_7033

#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Keyword.hpp>
#include <cminor/ast/Delegate.hpp>
#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class DelegateGrammar : public cminor::parsing::Grammar
{
public:
    static DelegateGrammar* Create();
    static DelegateGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    Node* Parse(const char* start, const char* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    DelegateGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class DelegateRule;
    class ClassDelegateRule;
};

} } // namespace cminor.parser

#endif // Delegate_hpp_7033
