namespace Syntax
{
    grammar PrimitiveGrammar
    {
        Primitive : Parser;
        Char : Parser;
        String : Parser;
        CharSet(var bool inverse, var StringBuilder s) : Parser;
        CharSetRange(var StringBuilder s) : string;
        CharSetChar : char;
        Keyword : Parser;
        KeywordBody : Parser;
        KeywordList : Parser;
        KeywordListBody(var List<string> keywords) : Parser;
        Empty : Parser;
        Space : Parser;
        AnyChar : Parser;
        Letter : Parser;
        Digit : Parser;
        HexDigit : Parser;
        UpperLetter : Parser;
        LowerLetter : Parser;
        TitleLetter : Parser;
        ModifierLetter : Parser;
        OtherLetter : Parser;
        CasedLetter : Parser;
        Mark : Parser;
        NonspacingMark : Parser;
        SpacingMark : Parser;
        EnclosingMark : Parser;
        Number : Parser;
        DecimalNumber : Parser;
        LetterNumber : Parser;
        OtherNumber : Parser;
        Punctuation : Parser;
        ConnectorPunctuation : Parser;
        DashPunctuation : Parser;
        OpenPunctuation : Parser;
        ClosePunctuation : Parser;
        InitialPunctuation : Parser;
        FinalPunctuation : Parser;
        OtherPunctuation : Parser;
        Symbol : Parser;
        MathSymbol : Parser;
        CurrencySymbol : Parser;
        ModifierSymbol : Parser;
        OtherSymbol : Parser;
        Separator : Parser;
        SpaceSeparator : Parser;
        LineSeparator : Parser;
        ParagraphSeparator : Parser;
        Other : Parser;
        Control : Parser;
        Format : Parser;
        Surrogate : Parser;
        PrivateUse : Parser;
        Unassigned : Parser;
        Graphic : Parser;
        BaseChar : Parser;
        Alphabetic : Parser;
        IdStart : Parser;
        IdCont : Parser;
        Range : Parser;
        CodePoint : uint;
    }
    grammar GrammarGrammar
    {
        Grammar(Scope enclosingScope, ParsingContext ctx) : Grammar;
        GrammarContent(Grammar grammar, ParsingContext ctx);
        StartClause(Grammar grammar);
        SkipClause(Grammar grammar);
    }
    grammar ParserFileGrammar
    {
        ParserFile(int id_, ParsingDomain parsingDomain_, var ParsingContext ctx) : ParserFileContent;
        UsingDeclarations(ParserFileContent parserFileContent);
        UsingDeclaration(ParserFileContent parserFileContent);
        NamespaceContent(ParserFileContent parserFileContent, ParsingContext ctx);
        Namespace(ParserFileContent parserFileContent, ParsingContext ctx);
    }
    grammar PrimaryGrammar
    {
        Primary(Scope enclosingScope, ParsingContext ctx) : Parser;
        RuleCall(ParsingContext ctx, var NonterminalParser nt, var NonterminalArgumentAdder adder) : Parser;
        Nonterminal : string;
        Alias : string;
        Grouping(Scope enclosingScope, ParsingContext ctx) : Parser;
        Token(Scope enclosingScope, ParsingContext ctx) : Parser;
        Expectation(Parser child) : Parser;
        Action(Parser child, ParsingContext ctx) : ActionParser;
    }
    grammar ProjectFileGrammar
    {
        ProjectFile : Project;
        ProjectFileContent(Project project);
        Source : string;
        Reference : string;
        FilePath : string;
    }
    grammar CompositeGrammar
    {
        Alternative(Scope enclosingScope, ParsingContext ctx) : Parser;
        Sequence(Scope enclosingScope, ParsingContext ctx) : Parser;
        Difference(Scope enclosingScope, ParsingContext ctx) : Parser;
        ExclusiveOr(Scope enclosingScope, ParsingContext ctx) : Parser;
        Intersection(Scope enclosingScope, ParsingContext ctx) : Parser;
        List(Scope enclosingScope, ParsingContext ctx) : Parser;
        Postfix(Scope enclosingScope, ParsingContext ctx) : Parser;
    }
    grammar LibraryFileGrammar
    {
        LibraryFile(ParsingDomain parsingDomain, var ParsingContext ctx);
        NamespaceContent(ParsingDomain parsingDomain, ParsingContext ctx);
        Namespace(ParsingDomain parsingDomain, ParsingContext ctx);
        Grammar(Scope enclosingScope, ParsingContext ctx) : Grammar;
        GrammarContent(Grammar grammar, ParsingContext ctx);
        Rule(Scope enclosingScope, ParsingContext ctx) : RuleParser;
    }
    grammar RuleGrammar
    {
        Rule(Scope enclosingScope, ParsingContext ctx) : RuleParser;
        RuleHeader(Scope enclosingScope, ParsingContext ctx) : RuleParser;
        RuleBody(RuleParser rule, ParsingContext ctx);
    }
    grammar ElementGrammar
    {
        RuleLink(Grammar grammar);
        Signature(RuleParser rule, ParsingContext ctx);
        ParameterList(RuleParser rule, ParsingContext ctx);
        Variable(RuleParser rule, ParsingContext ctx);
        Parameter(RuleParser rule, ParsingContext ctx);
        ReturnType(RuleParser rule, ParsingContext ctx);
        Keyword;
        Identifier : string;
        QualifiedId : string;
        StringArray(List<string> array);
    }
}
