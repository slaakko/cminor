namespace cminor.syntax
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
    grammar RuleGrammar
    {
        Rule(cminor::parsing::Scope* enclosingScope): cminor::parsing::Rule*;
        RuleHeader(cminor::parsing::Scope* enclosingScope): cminor::parsing::Rule*;
        RuleBody(cminor::parsing::Rule* rule);
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
        Identifier: std::u32string;
        QualifiedId: std::u32string;
        StringArray(std::vector<std::u32string>* array);
    }
    grammar GrammarGrammar
    {
        Grammar(cminor::parsing::Scope* enclosingScope): cminor::parsing::Grammar*;
        GrammarContent(cminor::parsing::Grammar* grammar);
        StartClause(cminor::parsing::Grammar* grammar);
        SkipClause(cminor::parsing::Grammar* grammar);
    }
    grammar PrimitiveGrammar
    {
        Primitive: cminor::parsing::Parser*;
        Char: cminor::parsing::Parser*;
        String: cminor::parsing::Parser*;
        CharSet(var bool inverse, var std::u32string s): cminor::parsing::Parser*;
        CharSetRange: std::u32string;
        CharSetChar: char32_t;
        Keyword: cminor::parsing::Parser*;
        KeywordBody: cminor::parsing::Parser*;
        KeywordList: cminor::parsing::Parser*;
        KeywordListBody(var std::vector<std::u32string> keywords): cminor::parsing::Parser*;
        Range: cminor::parsing::Parser*;
        CodePoint: uint32_t;
        Empty: cminor::parsing::Parser*;
        Space: cminor::parsing::Parser*;
        AnyChar: cminor::parsing::Parser*;
        Letter: cminor::parsing::Parser*;
        Digit: cminor::parsing::Parser*;
        HexDigit: cminor::parsing::Parser*;
        UpperLetter: cminor::parsing::Parser*;
        LowerLetter: cminor::parsing::Parser*;
        TitleLetter: cminor::parsing::Parser*;
        ModifierLetter: cminor::parsing::Parser*;
        OtherLetter: cminor::parsing::Parser*;
        CasedLetter: cminor::parsing::Parser*;
        Mark: cminor::parsing::Parser*;
        NonspacingMark: cminor::parsing::Parser*;
        SpacingMark: cminor::parsing::Parser*;
        EnclosingMark: cminor::parsing::Parser*;
        Number: cminor::parsing::Parser*;
        DecimalNumber: cminor::parsing::Parser*;
        LetterNumber: cminor::parsing::Parser*;
        OtherNumber: cminor::parsing::Parser*;
        Punctuation: cminor::parsing::Parser*;
        ConnectorPunctuation: cminor::parsing::Parser*;
        DashPunctuation: cminor::parsing::Parser*;
        OpenPunctuation: cminor::parsing::Parser*;
        ClosePunctuation: cminor::parsing::Parser*;
        InitialPunctuation: cminor::parsing::Parser*;
        FinalPunctuation: cminor::parsing::Parser*;
        OtherPunctuation: cminor::parsing::Parser*;
        Symbol: cminor::parsing::Parser*;
        MathSymbol: cminor::parsing::Parser*;
        CurrencySymbol: cminor::parsing::Parser*;
        ModifierSymbol: cminor::parsing::Parser*;
        OtherSymbol: cminor::parsing::Parser*;
        Separator: cminor::parsing::Parser*;
        SpaceSeparator: cminor::parsing::Parser*;
        LineSeparator: cminor::parsing::Parser*;
        ParagraphSeparator: cminor::parsing::Parser*;
        Other: cminor::parsing::Parser*;
        Control: cminor::parsing::Parser*;
        Format: cminor::parsing::Parser*;
        Surrogate: cminor::parsing::Parser*;
        PrivateUse: cminor::parsing::Parser*;
        Unassigned: cminor::parsing::Parser*;
        Graphic: cminor::parsing::Parser*;
        BaseChar: cminor::parsing::Parser*;
        Alphabetic: cminor::parsing::Parser*;
        IdStart: cminor::parsing::Parser*;
        IdCont: cminor::parsing::Parser*;
    }
    grammar ParserFileGrammar
    {
        ParserFile(int id_, cminor::parsing::ParsingDomain* parsingDomain_): ParserFileContent*;
        IncludeDirectives(ParserFileContent* parserFileContent);
        IncludeDirective(ParserFileContent* parserFileContent);
        FileAttribute: std::u32string;
        IncludeFileName: std::u32string;
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
    grammar PrimaryGrammar
    {
        Primary(cminor::parsing::Scope* enclosingScope): cminor::parsing::Parser*;
        RuleCall: cminor::parsing::Parser*;
        Nonterminal: std::u32string;
        Alias: std::u32string;
        Grouping(cminor::parsing::Scope* enclosingScope): cminor::parsing::Parser*;
        Token(cminor::parsing::Scope* enclosingScope): cminor::parsing::Parser*;
        Expectation(cminor::parsing::Parser* child): cminor::parsing::Parser*;
        Action(cminor::parsing::Parser* child): cminor::parsing::Parser*;
    }
    grammar ProjectFileGrammar
    {
        ProjectFile: Project*;
        ProjectFileContent(Project* project);
        Source: std::u32string;
        Reference: std::u32string;
        FilePath: std::u32string;
    }
}
