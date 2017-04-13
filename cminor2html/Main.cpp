// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/parser/ProjectFile.hpp>
#include <cminor/parser/SolutionFile.hpp>
#include <cminor/parser/SourceToken.hpp>
#include <cminor/ast/SourceToken.hpp>
#include <cminor/pl/InitDone.hpp>
#include <cminor/util/CodeFormatter.hpp>
#include <cminor/util/MappedInputFile.hpp>
#include <cminor/util/TextUtils.hpp>
#include <cminor/machine/Error.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <fstream>

const char* version = "0.0.1";

void PrintHelp()
{
    std::cout << "cminor2html version " << version << std::endl;
    std::cout << "usage: cminor2html [options] { sourcefile.cminor }" << std::endl;
    std::cout << "Convert cminor sourcefiles to html." << std::endl;
    std::cout << "options:" << std::endl;
    std::cout << "-s | --style <filepath> : set stylesheet filepath to <filepath> " << std::endl;
    std::cout << "                          default is \"code.css\"" << std::endl;
    std::cout << "-h | --help             : print this help message" << std::endl;
}

using namespace cminor::ast;
using namespace cminor::parser;
using namespace cminor::util;
using namespace cminor::machine;

std::string HtmlEscape(const std::string& s)
{
    std::string result;
    for (char c : s)
    {
        switch (c)
        {
            case '<': result.append("&lt;"); break;
            case '>': result.append("&gt;"); break;
            case '\'': result.append("&apos;"); break;
            case '"': result.append("&quot;"); break;
            case '&': result.append("&amp;"); break;
            default:
            {
                result.append(1, c);
            }
        }
    }
    return result;
}

class HtmlSourceTokenFormatter : public SourceTokenFormatter
{
public:
    HtmlSourceTokenFormatter(const std::string& sourceFilePath_, const std::string& styleSheetFilePath_);
    void BeginFormat() override;
    void EndFormat() override;
    void Keyword(const std::string& token) override;
    void Identifier(const std::string& token) override;
    void Number(const std::string& token) override;
    void Char(const std::string& token) override;
    void String(const std::string& token) override;
    void Spaces(const std::string& token) override;
    void Comment(const std::string& token) override;
    void NewLine(const std::string& token) override;
    void Other(const std::string& token) override;
private:
    std::string filePath;
    std::string styleSheetFilePath;
    std::ofstream htmlFile;
    std::unique_ptr<CodeFormatter> formatter;
};

class HtmlProjectFormatter : public ProjectFormatter
{
public:
    HtmlProjectFormatter(const std::string& projectFilePath_, const std::string& styleSheetFilePath_);
    void BeginFormat() override;
    void EndFormat() override;
    void FormatName(const std::string& projectName) override;
    void FormatTarget(Target target) override;
    void FormatAssemblyReference(const std::string& assemblyReference) override;
    void FormatSourceFilePath(const std::string& sourceFilePath) override;
private:
    std::string filePath;
    std::string styleSheetFilePath;
    std::ofstream htmlFile;
    std::unique_ptr<CodeFormatter> formatter;
};

class HtmlSolutionFormatter : public SolutionFormatter
{
public:
    HtmlSolutionFormatter(const std::string& solutionFilePath_, const std::string& styleSheetFilePath_);
    void BeginFormat() override;
    void EndFormat() override;
    void FormatName(const std::string& solutionName) override;
    void FormatProjectFilePath(const std::string& projectFilePath) override;
private:
    std::string filePath;
    std::string styleSheetFilePath;
    std::ofstream htmlFile;
    std::unique_ptr<CodeFormatter> formatter;
};

HtmlSourceTokenFormatter::HtmlSourceTokenFormatter(const std::string& sourceFilePath_, const std::string& styleSheetFilePath_) : styleSheetFilePath(styleSheetFilePath_)
{
    filePath = sourceFilePath_;
    std::string sourceFilePath(sourceFilePath_);
    sourceFilePath.append(".html");
    htmlFile.open(sourceFilePath.c_str(), std::ios_base::out);
    formatter.reset(new CodeFormatter(htmlFile));
}

void HtmlSourceTokenFormatter::BeginFormat()
{
    formatter->WriteLine("<!DOCTYPE html>");
    formatter->WriteLine("<html lang=\"en\" xmlns=\"http://www.w3.org/1999/xhtml\">");
    formatter->WriteLine("<head>");
    formatter->IncIndent();
    formatter->WriteLine("<meta charset=\"utf-8\"/>");
    boost::filesystem::path fp = filePath;
    std::string fileName = fp.filename().generic_string();
    formatter->WriteLine("<title>" + fileName + "</title>");
    formatter->WriteLine("<link rel=\"stylesheet\" type=\"text/css\" href=\"" + styleSheetFilePath + "\"/>");
    formatter->DecIndent();
    formatter->WriteLine("</head>");
    formatter->WriteLine("<body>");
    formatter->WriteLine("<pre>");
}

void HtmlSourceTokenFormatter::EndFormat()
{
    formatter->WriteLine("</pre>");
    formatter->WriteLine("</body>");
    formatter->WriteLine("</html>");
}

void HtmlSourceTokenFormatter::Keyword(const std::string& token)
{
    formatter->Write("<span class=\"keyword\">" + token + "</span>");
}

void HtmlSourceTokenFormatter::Identifier(const std::string& token)
{
    formatter->Write("<span class=\"identifier\">" + token + "</span>");
}

void HtmlSourceTokenFormatter::Number(const std::string& token)
{
    formatter->Write("<span class=\"number\">" + token + "</span>");
}

void HtmlSourceTokenFormatter::Char(const std::string& token)
{
    formatter->Write("<span class=\"char\">" + HtmlEscape(token) + "</span>");
}

void HtmlSourceTokenFormatter::String(const std::string& token)
{
    formatter->Write("<span class=\"string\">" + HtmlEscape(token) + "</span>");
}

void HtmlSourceTokenFormatter::Spaces(const std::string& token)
{
    formatter->Write(token);
}

void HtmlSourceTokenFormatter::Comment(const std::string& token)
{
    std::string trimmedToken = Trim(token);
    formatter->Write("<span class=\"comment\">" + HtmlEscape(trimmedToken) + "</span>");
    formatter->WriteLine();
}

void HtmlSourceTokenFormatter::NewLine(const std::string& token)
{
    formatter->WriteLine();
}

void HtmlSourceTokenFormatter::Other(const std::string& token)
{
    formatter->Write("<span class=\"other\">" + HtmlEscape(token) + "</span>");
}

HtmlProjectFormatter::HtmlProjectFormatter(const std::string& projectFilePath_, const std::string& styleSheetFilePath_) : styleSheetFilePath(styleSheetFilePath_)
{
    filePath = projectFilePath_;
    std::string projectFilePath(projectFilePath_);
    projectFilePath.append(".html");
    htmlFile.open(projectFilePath.c_str(), std::ios_base::out);
    formatter.reset(new CodeFormatter(htmlFile));
}

void HtmlProjectFormatter::BeginFormat()
{
    formatter->WriteLine("<!DOCTYPE html>");
    formatter->WriteLine("<html lang=\"en\" xmlns=\"http://www.w3.org/1999/xhtml\">");
    formatter->WriteLine("<head>");
    formatter->IncIndent();
    formatter->WriteLine("<meta charset=\"utf-8\"/>");
    boost::filesystem::path fp = filePath;
    std::string fileName = fp.filename().generic_string();
    formatter->WriteLine("<title>" + fileName + "</title>");
    formatter->WriteLine("<link rel=\"stylesheet\" type=\"text/css\" href=\"" + styleSheetFilePath + "\"/>");
    formatter->DecIndent();
    formatter->WriteLine("</head>");
    formatter->WriteLine("<body>");
    formatter->WriteLine("<pre>");
}

void HtmlProjectFormatter::EndFormat()
{
    formatter->WriteLine("</pre>");
    formatter->WriteLine("</body>");
    formatter->WriteLine("</html>");
}

void HtmlProjectFormatter::FormatName(const std::string& projectName)
{
    formatter->WriteLine("<span class=\"projectFileKeyword\">project</span> <span class=\"projectName\">" + projectName + "</span>;");
}

void HtmlProjectFormatter::FormatTarget(Target target)
{
    std::string targetStr;
    if (target == Target::program)
    {
        targetStr = "program";
    }
    else if (target == Target::library)
    {
        targetStr = "library";
    }
    formatter->WriteLine("<span class=\"projectFileKeyword\">target</span>=<span class=\"target\">" + targetStr + "</span>;");
}

void HtmlProjectFormatter::FormatAssemblyReference(const std::string& assemblyReference)
{
    formatter->WriteLine("<span class=\"projectFileKeyword\">reference</span> &lt;<span class=\"reference\">" + assemblyReference + "</span>&gt;;");
}

void HtmlProjectFormatter::FormatSourceFilePath(const std::string& sourceFilePath)
{
    formatter->WriteLine("<span class=\"projectFileKeyword\">source</span> &lt;<span class=\"source\">" + sourceFilePath + "</span>&gt;;");
}

HtmlSolutionFormatter::HtmlSolutionFormatter(const std::string& solutionFilePath_, const std::string& styleSheetFilePath_) : styleSheetFilePath(styleSheetFilePath_)
{
    filePath = solutionFilePath_;
    std::string  solutionFilePath(solutionFilePath_);
    solutionFilePath.append(".html");
    htmlFile.open(solutionFilePath.c_str(), std::ios_base::out);
    formatter.reset(new CodeFormatter(htmlFile));
}

void HtmlSolutionFormatter::BeginFormat()
{
    formatter->WriteLine("<!DOCTYPE html>");
    formatter->WriteLine("<html lang=\"en\" xmlns=\"http://www.w3.org/1999/xhtml\">");
    formatter->WriteLine("<head>");
    formatter->IncIndent();
    formatter->WriteLine("<meta charset=\"utf-8\"/>");
    boost::filesystem::path fp = filePath;
    std::string fileName = fp.filename().generic_string();
    formatter->WriteLine("<title>" + fileName + "</title>");
    formatter->WriteLine("<link rel=\"stylesheet\" type=\"text/css\" href=\"" + styleSheetFilePath + "\"/>");
    formatter->DecIndent();
    formatter->WriteLine("</head>");
    formatter->WriteLine("<body>");
    formatter->WriteLine("<pre>");
}

void HtmlSolutionFormatter::EndFormat()
{
    formatter->WriteLine("</pre>");
    formatter->WriteLine("</body>");
    formatter->WriteLine("</html>");
}

void HtmlSolutionFormatter::FormatName(const std::string& solutionName) 
{
    formatter->WriteLine("<span class=\"solutionFileKeyword\">solution</span> <span class=\"solutionName\">" + solutionName + "</span>;");
}

void HtmlSolutionFormatter::FormatProjectFilePath(const std::string& projectFilePath)
{
    formatter->WriteLine("<span class=\"solutionFileKeyword\">project</span> &lt;<span class=\"projectFilePath\">" + projectFilePath + "</span>&gt;;");
}

struct InitDone
{
    InitDone()
    {
        cminor::parsing::Init();
        cminor::util::unicode::Init();
    }
    ~InitDone()
    {
        cminor::util::unicode::Done();
        cminor::parsing::Done();
    }
};

int main(int argc, const char** argv)
{
    try
    {
        InitDone initDone;
        if (argc < 2)
        {
            PrintHelp();
            return 0;
        }
        std::string styleSheetPath = "code.css";
        bool prevWasStyle = false;
        std::vector<std::string> sourceFiles;
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (!arg.empty() && arg[0] == '-')
            {
                if (arg == "-s" || arg == "--style")
                {
                    prevWasStyle = true;
                }
                else if (arg == "-h" || arg == "--help")
                {
                    PrintHelp();
                    return 0;
                }
                else
                {
                    throw std::runtime_error("unknown option '" + arg + "'");
                }
            }
            else if (prevWasStyle)
            {
                styleSheetPath = arg;
                prevWasStyle = false;
            }
            else
            {
                sourceFiles.push_back(arg);
            }
        }
        SourceTokenGrammar* grammar = SourceTokenGrammar::Create();
        ProjectGrammar* projectGrammar = ProjectGrammar::Create();
        SolutionGrammar* solutionGrammar = SolutionGrammar::Create();
        for (const std::string& sourceFile : sourceFiles)
        {
            boost::filesystem::path sfp(sourceFile);
            if (sfp.extension() == ".cminor")
            {
                MappedInputFile file(sourceFile);
                HtmlSourceTokenFormatter formatter(sourceFile, styleSheetPath);
                grammar->Parse(file.Begin(), file.End(), 0, sourceFile, &formatter);
            }
            else if (sfp.extension() == ".cminorp")
            {
                MappedInputFile file(sourceFile);
                std::unique_ptr<Project> project(projectGrammar->Parse(file.Begin(), file.End(), 0, sourceFile, "debug"));
                project->ResolveDeclarations();
                HtmlProjectFormatter projectFormatter(sourceFile, styleSheetPath);
                project->Format(projectFormatter);
            }
            else if (sfp.extension() == ".cminors")
            {
                MappedInputFile file(sourceFile);
                std::unique_ptr<Solution> solution(solutionGrammar->Parse(file.Begin(), file.End(), 0, sourceFile));
                solution->ResolveDeclarations();
                HtmlSolutionFormatter solutionFormatter(sourceFile, styleSheetPath);
                solution->Format(solutionFormatter);
            }
        }
    }
    catch (const Exception& ex)
    {
        std::cerr << ex.What() << std::endl;
        return 1;
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
