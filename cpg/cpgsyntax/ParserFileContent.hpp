/*========================================================================
    Copyright (c) 2011-2013 Seppo Laakko
    http://sourceforge.net/projects/soulparsing/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#ifndef CPG_SYNTAX_PARSERFILECONTENT_INCLUDED
#define CPG_SYNTAX_PARSERFILECONTENT_INCLUDED

#include <cminor/pl/ParsingDomain.hpp>
#include <cminor/pl/Namespace.hpp>

namespace cpg { namespace syntax {

class ParserFileContent
{
public:
    ParserFileContent(int id_, cminor::parsing::ParsingDomain* parsingDomain_);
    void SetFilePath(const std::string& filePath_) { filePath = filePath_; }
    void AddIncludeDirective(const std::string& includeDirective_, const std::string& fileAttribute_);
    void AddUsingObject(cminor::pom::UsingObject* usingObjectSubject);
    void AddGrammar(cminor::parsing::Grammar* grammar);
    void BeginNamespace(const std::string& ns);
    void EndNamespace();
    int Id() const { return id; }
    const std::string& FilePath() const { return filePath; }
    cminor::parsing::ParsingDomain* ParsingDomain() const { return parsingDomain; }
    const std::vector<std::pair<std::string, std::string>>& IncludeDirectives() const { return includeDirectives; }
    const std::vector<std::unique_ptr<cminor::parsing::UsingObject>>& UsingObjects() const { return usingObjects; }
    cminor::parsing::Scope* CurrentScope() const { return parsingDomain->CurrentScope(); }
private:
    int id;
    std::string filePath;
    cminor::parsing::ParsingDomain* parsingDomain;
    std::vector<std::pair<std::string, std::string>> includeDirectives;
    std::vector<std::unique_ptr<cminor::parsing::UsingObject>> usingObjects;
};

} } // namespace cpg::syntax

#endif // CPG_SYNTAX_PARSERFILECONTENT_INCLUDED
