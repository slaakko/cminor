/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#include <cminor/cpg/cpgsyntax/Library.hpp>
#include <cminor/pl/Visitor.hpp>
#include <cminor/pl/Rule.hpp>
#include <cminor/pl/Grammar.hpp>
#include <cminor/pl/Namespace.hpp>
#include <cminor/util/CodeFormatter.hpp>
#include <cminor/util/TextUtils.hpp>
#include <unordered_map>
#include <iostream>
#include <fstream>

namespace cpg { namespace syntax {

using namespace cminor::parsing;
using cminor::util::CodeFormatter;

class LibraryFileGeneratorVisitor : public Visitor
{
public:
    LibraryFileGeneratorVisitor(CodeFormatter& formatter_): formatter(formatter_)
    {
    }
    virtual void BeginVisit(Grammar& grammar) 
    {
        if (!grammar.IsExternal())
        {
            std::vector<Grammar*>& grammars = nsGrammarMap[grammar.Ns()];
            grammars.push_back(&grammar);
        }
    }
    void GenerateLibraryFile()
    {
        NamespaceGrammarMapIt e = nsGrammarMap.end();
        for (NamespaceGrammarMapIt i = nsGrammarMap.begin(); i != e; ++i)
        {
            Namespace* ns = i->first;
            const std::vector<Grammar*>& grammars = i->second;
            if (!ns->FullName().empty())
            {
                formatter.WriteLine("namespace " + ns->FullName());
                formatter.WriteLine("{");
                formatter.IncIndent();
            }
            int n = int(grammars.size());
            for (int i = 0; i < n; ++i)
            {
                Grammar* grammar = grammars[i];
                formatter.WriteLine("grammar " + grammar->Name());
                formatter.WriteLine("{");
                formatter.IncIndent();
                const std::vector<Rule*>& rules = grammar->Rules();
                int nr = int(rules.size());
                for (int i = 0; i < nr; ++i)
                {
                    Rule* rule = rules[i];
                    std::string ruleLine = rule->Name();
                    if (!rule->InheritedAttributes().empty() || !rule->LocalVariables().empty())
                    {
                        ruleLine.append("(");
                        int ni = int(rule->InheritedAttributes().size());
                        bool first = true;
                        for (int i = 0; i < ni; ++i)
                        {
                            const AttrOrVariable& attr = rule->InheritedAttributes()[i];
                            if (first)
                            {
                                first = false;
                            }
                            else
                            {
                                ruleLine.append(", ");
                            }
                            ruleLine.append(attr.TypeName() + " " + attr.Name());
                        }
                        int nl = int(rule->LocalVariables().size());
                        for (int i = 0; i < nl; ++i)
                        {
                            const AttrOrVariable& var = rule->LocalVariables()[i];
                            if (first)
                            {
                                first = false;
                            }
                            else
                            {
                                ruleLine.append(", ");
                            }
                            ruleLine.append("var " + var.TypeName() + " " + var.Name());
                        }
                        ruleLine.append(")");
                    }
                    if (!rule->ValueTypeName().empty())
                    {
                        ruleLine.append(": ").append(rule->ValueTypeName());
                    }
                    ruleLine.append(";");
                    formatter.WriteLine(ruleLine);
                }
                formatter.DecIndent();
                formatter.WriteLine("}");
            }
            if (!ns->FullName().empty())
            {
                formatter.DecIndent();
                formatter.WriteLine("}");
            }
        }
    }
private:
    CodeFormatter& formatter;
    typedef std::unordered_map<Namespace*, std::vector<Grammar*>> NamespaceGrammarMap;
    typedef NamespaceGrammarMap::const_iterator NamespaceGrammarMapIt;
    NamespaceGrammarMap nsGrammarMap;
};

void GenerateLibraryFile(Project* project, ParsingDomain* parsingDomain)
{
    std::string libraryFilePath = boost::filesystem::path(project->FilePath()).replace_extension(".pl").generic_string();
    std::ofstream libraryFile(libraryFilePath.c_str());
    CodeFormatter formatter(libraryFile);
    LibraryFileGeneratorVisitor visitor(formatter);
    parsingDomain->GlobalNamespace()->Accept(visitor);
    visitor.GenerateLibraryFile();
    std::cout << "=> " << libraryFilePath << std::endl;
}

} } // namespace cpg::syntax
