/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#ifndef CPG_SYNTAX_PROJECT_INCLUDED
#define CPG_SYNTAX_PROJECT_INCLUDED
#include <boost/filesystem.hpp>
#include <string>
#include <vector>
#include <memory>

namespace cpg { namespace syntax {

class Project
{
public:
    Project(const std::string& name_, const std::string& filePath_);
    const std::string& Name() const { return name; }
    const std::string& FilePath() const { return filePath; }
    std::string BasePath() const;
    void AddSourceFile(const std::string& sourceFilePath);
    void AddReferenceFile(const std::string& referenceFilePath);
    const std::vector<std::string>& ReferenceFiles() const { return referenceFiles; }
    const std::vector<std::string>& SourceFiles() const { return sourceFiles; }
private:
    std::string name;
    std::string filePath;
    std::vector<std::string> sourceFiles;
    std::vector<std::string> referenceFiles;
    boost::filesystem::path base;
};

} } // namespace cpg::syntax

#endif // CPG_SYNTAX_PROJECT_INCLUDED
