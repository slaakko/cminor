namespace cminor.parser
{
    grammar ExpressionGrammar
    {
        Expression(ParsingContext* ctx): Node*;
        Disjunction(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s): Node*;
        Conjunction(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s): Node*;
        BitOr(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s): Node*;
        BitXor(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s): Node*;
        BitAnd(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s): Node*;
        Equality(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s, var Operator op): Node*;
        Relational(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s, var Operator op): Node*;
        Shift(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s, var Operator op): Node*;
        Additive(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s, var Operator op): Node*;
        Multiplicative(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s, var Operator op): Node*;
        Prefix(ParsingContext* ctx, var Span s, var Operator op): Node*;
        Postfix(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s): Node*;
        Primary(ParsingContext* ctx): Node*;
        ArgumentList(ParsingContext* ctx, Node* node);
        ExpressionList(ParsingContext* ctx, Node* node);
    }
    grammar BasicTypeGrammar
    {
        BasicType: Node*;
    }
    grammar SpecifierGrammar
    {
        Specifiers: Specifiers;
        Specifier: Specifiers;
    }
    grammar FunctionGrammar
    {
        Function(ParsingContext* ctx, var std::unique_ptr<FunctionNode> fun, var Span s): FunctionNode*;
        FunctionGroupId(ParsingContext* ctx): FunctionGroupIdNode*;
        OperatorFunctionGroupId(ParsingContext* ctx, var std::unique_ptr<Node> typeExpr): FunctionGroupIdNode*;
    }
    grammar IdentifierGrammar
    {
        Identifier: IdentifierNode*;
        QualifiedId: IdentifierNode*;
    }
    grammar LiteralGrammar
    {
        Literal: Node*;
        BooleanLiteral: Node*;
        FloatingLiteral(var Span s): Node*;
        FloatingLiteralValue: double;
        FractionalFloatingLiteral;
        ExponentFloatingLiteral;
        ExponentPart;
        IntegerLiteral(var Span s): Node*;
        IntegerLiteralValue: uint64_t;
        HexIntegerLiteral: uint64_t;
        DecIntegerLiteral: uint64_t;
        CharLiteral(var utf32_char litValue): Node*;
        StringLiteral(var utf32_string s): Node*;
        NullLiteral: Node*;
        CharEscape: utf32_char;
        DecDigitSequence: uint64_t;
        HexDigitSequence: uint64_t;
        HexDigit4: uint16_t;
        HexDigit8: uint32_t;
        OctalDigitSequence: uint64_t;
        Sign;
    }
    grammar KeywordGrammar
    {
        Keyword;
    }
    grammar TypeExprGrammar
    {
        TypeExpr(ParsingContext* ctx): Node*;
        PostfixTypeExpr(ParsingContext* ctx, var std::unique_ptr<Node> typeExpr, var Span s): Node*;
        PrimaryTypeExpr(ParsingContext* ctx): Node*;
    }
    grammar ParameterGrammar
    {
        ParameterList(ParsingContext* ctx, Node* owner);
        Parameter(ParsingContext* ctx): ParameterNode*;
    }
    grammar StatementGrammar
    {
        Statement(ParsingContext* ctx): StatementNode*;
        LabelId: std::string;
        Label(var std::string label): LabelNode*;
        LabeledStatement(ParsingContext* ctx): StatementNode*;
        ControlStatement(ParsingContext* ctx): StatementNode*;
        CompoundStatement(ParsingContext* ctx): CompoundStatementNode*;
        AssignmentStatement(ParsingContext* ctx, var std::unique_ptr<Node> targetExpr): StatementNode*;
        ConstructionStatement(ParsingContext* ctx): StatementNode*;
    }
}
