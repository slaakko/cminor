namespace Code
{
    grammar BasicTypeGrammar
    {
        BasicType: Type;
    }
    grammar ExpressionGrammar
    {
        Expression(ParsingContext ctx): DomObject;
        Disjunction(ParsingContext ctx): DomObject;
        Conjunction(ParsingContext ctx): DomObject;
        BitOr(ParsingContext ctx): DomObject;
        BitXor(ParsingContext ctx): DomObject;
        BitAnd(ParsingContext ctx): DomObject;
        Equality(ParsingContext ctx, var Operator op): DomObject;
        Relational(ParsingContext ctx, var Operator op): DomObject;
        Shift(ParsingContext ctx, var Operator op): DomObject;
        Additive(ParsingContext ctx, var Operator op): DomObject;
        Multiplicative(ParsingContext ctx, var Operator op): DomObject;
        Prefix(ParsingContext ctx, var Operator op): DomObject;
        Postfix(ParsingContext ctx): DomObject;
        Primary(ParsingContext ctx): DomObject;
        CastExpr(ParsingContext ctx): DomObject;
        NewExpr(ParsingContext ctx): DomObject;
        ArgumentList(ParsingContext ctx, DomObject o);
        ExpressionList(ParsingContext ctx, DomObject o);
    }
    grammar IdentifierGrammar
    {
        Identifier: Identifier;
        QualifiedId: Identifier;
    }
    grammar KeywordGrammar
    {
        Keyword;
    }
    grammar LiteralGrammar
    {
        Literal: Literal;
        BooleanLiteral: Literal;
        FloatingLiteral: Literal;
        FloatingLiteralValue: string;
        FractionalFloatingLiteral;
        ExponentFloatingLiteral;
        ExponentPart;
        IntegerLiteral: Literal;
        IntegerLiteralValue: string;
        HexIntegerLiteral;
        DecIntegerLiteral;
        CharLiteral: Literal;
        StringLiteral: Literal;
        NullLiteral: Literal;
        CharEscape;
        DecDigitSequence;
        HexDigitSequence;
        HexDigit4;
        HexDigit8;
        OctalDigitSequence;
        Sign;
    }
    grammar NamespaceGrammar
    {
        NamespaceImport: NamespaceImport;
        Alias: Alias;
    }
    grammar StatementGrammar
    {
        Statement(ParsingContext ctx): Statement;
        Label: string;
        LabeledStatement(ParsingContext ctx): Statement;
        ControlStatement(ParsingContext ctx): Statement;
        CompoundStatement(ParsingContext ctx): CompoundStatement;
        ReturnStatement(ParsingContext ctx): Statement;
        IfStatement(ParsingContext ctx): Statement;
        WhileStatement(ParsingContext ctx): Statement;
        DoStatement(ParsingContext ctx): Statement;
        ForStatement(ParsingContext ctx): ForStatement;
        ForInitStatement(ParsingContext ctx): Statement;
        ForLoopExpressionStatement(ParsingContext ctx): Statement;
        ForEachStatement(ParsingContext ctx): Statement;
        BreakStatement(ParsingContext ctx): Statement;
        ContinueStatement(ParsingContext ctx): Statement;
        GotoStatement(ParsingContext ctx): Statement;
        SwitchStatement(ParsingContext ctx): SwitchStatement;
        CaseStatement(ParsingContext ctx): CaseStatement;
        DefaultStatement(ParsingContext ctx): DefaultStatement;
        GotoCaseStatement(ParsingContext ctx): Statement;
        GotoDefaultStatement(ParsingContext ctx): Statement;
        AssignmentExpressionStatement(ParsingContext ctx): AssignmentStatement;
        AssignmentStatement(ParsingContext ctx): AssignmentStatement;
        ConstructionExpressionStatement(ParsingContext ctx): ConstructionStatement;
        ConstructionStatement(ParsingContext ctx): ConstructionStatement;
        IncrementExpressionStatement(ParsingContext ctx): IncrementStatement;
        IncrementStatement(ParsingContext ctx): IncrementStatement;
        DecrementExpressionStatement(ParsingContext ctx): DecrementStatement;
        DecrementStatement(ParsingContext ctx): DecrementStatement;
        ExpressionStatement(ParsingContext ctx): Statement;
        EmptyStatement(ParsingContext ctx): Statement;
        ThrowStatement(ParsingContext ctx): Statement;
        TryStatement(ParsingContext ctx): TryStatement;
        Catch(ParsingContext ctx): Catch;
        Finally(ParsingContext ctx): Finally;
        UsingStatement(ParsingContext ctx): Statement;
    }
    grammar TemplateGrammar
    {
        TemplateId(ParsingContext ctx): TemplateId;
    }
    grammar TypeExprGrammar
    {
        TypeExpr(ParsingContext ctx): DomObject;
        PrefixTypeExpr(ParsingContext ctx): DomObject;
        PostfixTypeExpr(ParsingContext ctx);
        PrimaryTypeExpr(ParsingContext ctx);
    }
}
