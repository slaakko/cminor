#ifndef Delegate_hpp_21399
#define Delegate_hpp_21399

#include <cminor/parsing/Grammar.hpp>
#include <cminor/parsing/Keyword.hpp>
#include <cminor/ast/Delegate.hpp>
#include <cminor/parser/ParsingContext.hpp>

namespace cminor { namespace parser {

using namespace cminor::ast;
class DelegateGrammar : public cminor::parsing::Grammar
{
public:
    static DelegateGrammar* Create();
    static DelegateGrammar* Create(cminor::parsing::ParsingDomain* parsingDomain);
    Node* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    DelegateGrammar(cminor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class DelegateRule;
    class ClassDelegateRule;
};

} } // namespace cminor.parser

#endif // Delegate_hpp_21399
