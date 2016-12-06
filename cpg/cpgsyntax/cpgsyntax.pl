namespace cpg.syntax
{
    grammar CompositeGrammar
    {
        Alternative(cminor::parsing::Scope* enclosingScope): cminor::parsing::Parser*;
        Sequence(cminor::parsing::Scope* enclosingScope): cminor::parsing::Parser*;
        Difference(cminor::parsing::Scope* enclosingScope): cminor::parsing::Parser*;
        ExclusiveOr(cminor::parsing::Scope* enclosingScope): cminor::parsing::Parser*;
        Intersection(cminor::parsing::Scope* enclosingScope): cminor::parsing::Parser*;
        List(cminor::parsing::Scope* enclosingScope): cminor::parsing::Parser*;
        Postfix(cminor::parsing::Scope* enclosingScope): cminor::parsing::Parser*;
    }
    grammar ElementGrammar
    {
        RuleLink(cminor::parsing::Grammar* grammar);
        Signature(cminor::parsing::Rule* rule);
        ParameterList(cminor::parsing::Rule* rule);
        Variable(cminor::parsing::Rule* rule);
        Parameter(cminor::parsing::Rule* rule);
        ReturnType(cminor::parsing::Rule* rule);
        Keyword;
        Identifier: std::string;
        QualifiedId: std::string;
        StringArray(std::vector<std::string>* array);
    }
    grammar ParserFileGrammar
    {
        ParserFile(int id_, cminor::parsing::ParsingDomain* parsingDomain_): ParserFileContent*;
        IncludeDirectives(ParserFileContent* parserFileContent);
        IncludeDirective(ParserFileContent* parserFileContent);
        FileAttribute: std::string;
        IncludeFileName: std::string;
        NamespaceContent(ParserFileContent* parserFileContent);
        Namespace(ParserFileContent* parserFileContent);
    }
    grammar LibraryFileGrammar
    {
        LibraryFile(cminor::parsing::ParsingDomain* parsingDomain);
        NamespaceContent(cminor::parsing::ParsingDomain* parsingDomain);
        Namespace(cminor::parsing::ParsingDomain* parsingDomain);
        Grammar(cminor::parsing::Scope* enclosingScope): cminor::parsing::Grammar*;
        GrammarContent(cminor::parsing::Grammar* grammar);
        Rule(cminor::parsing::Scope* enclosingScope): cminor::parsing::Rule*;
    }
    grammar GrammarGrammar
    {
        Grammar(cminor::parsing::Scope* enclosingScope): cminor::parsing::Grammar*;
        GrammarContent(cminor::parsing::Grammar* grammar);
        StartClause(cminor::parsing::Grammar* grammar);
        SkipClause(cminor::parsing::Grammar* grammar);
        CCClause(cminor::parsing::Grammar* grammar, var bool doSkip);
    }
    grammar RuleGrammar
    {
        Rule(cminor::parsing::Scope* enclosingScope): cminor::parsing::Rule*;
        RuleHeader(cminor::parsing::Scope* enclosingScope): cminor::parsing::Rule*;
        RuleBody(cminor::parsing::Rule* rule);
    }
    grammar PrimaryGrammar
    {
        Primary(cminor::parsing::Scope* enclosingScope): cminor::parsing::Parser*;
        RuleCall: cminor::parsing::Parser*;
        Nonterminal: std::string;
        Alias: std::string;
        Grouping(cminor::parsing::Scope* enclosingScope): cminor::parsing::Parser*;
        Token(cminor::parsing::Scope* enclosingScope): cminor::parsing::Parser*;
        Expectation(cminor::parsing::Parser* child): cminor::parsing::Parser*;
        Action(cminor::parsing::Parser* child): cminor::parsing::Parser*;
        CCOpt(cminor::parsing::Parser* child): cminor::parsing::Parser*;
    }
    grammar PrimitiveGrammar
    {
        Primitive: cminor::parsing::Parser*;
        Char: cminor::parsing::Parser*;
        String: cminor::parsing::Parser*;
        CharSet(var bool inverse, var std::string s): cminor::parsing::Parser*;
        CharSetRange: std::string;
        CharSetChar: char;
        Keyword: cminor::parsing::Parser*;
        KeywordBody: cminor::parsing::Parser*;
        KeywordList: cminor::parsing::Parser*;
        KeywordListBody(var std::vector<std::string> keywords): cminor::parsing::Parser*;
        Empty: cminor::parsing::Parser*;
        Space: cminor::parsing::Parser*;
        AnyChar: cminor::parsing::Parser*;
        Letter: cminor::parsing::Parser*;
        Digit: cminor::parsing::Parser*;
        HexDigit: cminor::parsing::Parser*;
        Punctuation: cminor::parsing::Parser*;
    }
    grammar ProjectFileGrammar
    {
        ProjectFile: Project*;
        ProjectFileContent(Project* project);
        Source: std::string;
        Reference: std::string;
        FilePath: std::string;
    }
}
