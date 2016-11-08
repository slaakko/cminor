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
        CastExpr(ParsingContext* ctx): Node*;
        NewExpr(ParsingContext* ctx): Node*;
        ArgumentList(ParsingContext* ctx, Node* node);
        ExpressionList(ParsingContext* ctx, Node* node);
    }
    grammar BasicTypeGrammar
    {
        BasicType: Node*;
    }
    grammar ClassGrammar
    {
        Class(ParsingContext* ctx): ClassNode*;
        InheritanceAndInterfaces(ParsingContext* ctx, ClassNode* classNode);
        BaseClassOrInterface(ParsingContext* ctx): Node*;
        ClassContent(ParsingContext* ctx, ClassNode* classNode);
        ClassMember(ParsingContext* ctx, ClassNode* classNode): Node*;
        StaticConstructor(ParsingContext* ctx, ClassNode* classNode, var AttributeMap attributeMap, var std::unique_ptr<IdentifierNode> id): StaticConstructorNode*;
        Constructor(ParsingContext* ctx, ClassNode* classNode, var AttributeMap attributeMap, var std::unique_ptr<IdentifierNode> id, var std::unique_ptr<ConstructorNode> ctor): Node*;
        Initializer(ParsingContext* ctx): InitializerNode*;
        MemberFunction(ParsingContext* ctx, ClassNode* classNode, var AttributeMap attributeMap, var std::unique_ptr<MemberFunctionNode> memFun, var std::unique_ptr<IdentifierNode> qid): Node*;
        MemberVariable(ParsingContext* ctx): Node*;
        Property(ParsingContext* ctx, var std::unique_ptr<Node> property): Node*;
        Indexer(ParsingContext* ctx, var std::unique_ptr<Node> indexer): Node*;
        GetterOrSetter(ParsingContext* ctx, Node* subject);
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
    grammar SolutionGrammar
    {
        Solution: Solution*;
        Declaration: SolutionDeclaration*;
        SolutionProjectDeclaration: SolutionDeclaration*;
        FilePath: std::string;
    }
    grammar InterfaceGrammar
    {
        Interface(ParsingContext* ctx): InterfaceNode*;
        InterfaceContent(ParsingContext* ctx, InterfaceNode* intf);
        InterfaceMemFun(ParsingContext* ctx, var std::unique_ptr<MemberFunctionNode> memFun): Node*;
        InterfaceFunctionGroupId: FunctionGroupIdNode*;
    }
    grammar CompileUnitGrammar
    {
        CompileUnit(ParsingContext* ctx): CompileUnitNode*;
        NamespaceContent(ParsingContext* ctx, CompileUnitNode* compileUnit, NamespaceNode* ns);
        UsingDirectives(ParsingContext* ctx, NamespaceNode* ns);
        UsingDirective(ParsingContext* ctx, NamespaceNode* ns);
        UsingAliasDirective(var std::unique_ptr<IdentifierNode> id): Node*;
        UsingNamespaceDirective: Node*;
        Definitions(ParsingContext* ctx, CompileUnitNode* compileUnit, NamespaceNode* ns);
        Definition(ParsingContext* ctx, CompileUnitNode* compileUnit, NamespaceNode* ns): Node*;
        NamespaceDefinition(ParsingContext* ctx, CompileUnitNode* compileUnit, NamespaceNode* ns): NamespaceNode*;
        FunctionDefinition(ParsingContext* ctx, CompileUnitNode* compileUnit): FunctionNode*;
        ClassDefinition(ParsingContext* ctx, CompileUnitNode* compileUnit): ClassNode*;
        InterfaceDefinition(ParsingContext* ctx, CompileUnitNode* compileUnit): InterfaceNode*;
    }
    grammar FunctionGrammar
    {
        Function(ParsingContext* ctx, var AttributeMap attributeMap, var std::unique_ptr<FunctionNode> fun, var Span s, var std::unique_ptr<IdentifierNode> qid): FunctionNode*;
        FunctionGroupId(ParsingContext* ctx): FunctionGroupIdNode*;
        OperatorFunctionGroupId(ParsingContext* ctx, var std::unique_ptr<Node> typeExpr): FunctionGroupIdNode*;
        Attributes(AttributeMap* attributeMap);
        NameValuePair(AttributeMap* attributeMap);
        Name: std::string;
        Value: std::string;
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
    grammar ProjectGrammar
    {
        Project(std::string config): Project*;
        Declaration: ProjectDeclaration*;
        ReferenceDeclaration: ProjectDeclaration*;
        SourceFileDeclaration: ProjectDeclaration*;
        TargetDeclaration: ProjectDeclaration*;
        Target: Target;
        FilePath: std::string;
    }
    grammar SpecifierGrammar
    {
        Specifiers: Specifiers;
        Specifier: Specifiers;
    }
    grammar StatementGrammar
    {
        Statement(ParsingContext* ctx): StatementNode*;
        LabelId: std::string;
        Label(var std::string label): LabelNode*;
        LabeledStatement(ParsingContext* ctx): StatementNode*;
        ControlStatement(ParsingContext* ctx): StatementNode*;
        CompoundStatement(ParsingContext* ctx): CompoundStatementNode*;
        ReturnStatement(ParsingContext* ctx): StatementNode*;
        IfStatement(ParsingContext* ctx): StatementNode*;
        WhileStatement(ParsingContext* ctx): StatementNode*;
        DoStatement(ParsingContext* ctx): StatementNode*;
        ForStatement(ParsingContext* ctx): StatementNode*;
        ForInitStatement(ParsingContext* ctx): StatementNode*;
        ForLoopExpressionStatement(ParsingContext* ctx): StatementNode*;
        BreakStatement(ParsingContext* ctx): StatementNode*;
        ContinueStatement(ParsingContext* ctx): StatementNode*;
        AssignmentExpressionStatement(ParsingContext* ctx, var std::unique_ptr<Node> targetExpr): StatementNode*;
        AssignmentStatement(ParsingContext* ctx): StatementNode*;
        ConstructionStatement(ParsingContext* ctx): StatementNode*;
        IncrementExpressionStatement(ParsingContext* ctx, var std::unique_ptr<Node> expr): StatementNode*;
        IncrementStatement(ParsingContext* ctx): StatementNode*;
        DecrementExpressionStatement(ParsingContext* ctx, var std::unique_ptr<Node> expr): StatementNode*;
        DecrementStatement(ParsingContext* ctx): StatementNode*;
        ExpressionStatement(ParsingContext* ctx, var std::unique_ptr<Node> expr): StatementNode*;
        EmptyStatement(ParsingContext* ctx): StatementNode*;
    }
    grammar TemplateGrammar
    {
        TemplateId(ParsingContext* ctx, var std::unique_ptr<TemplateIdNode> templateId): Node*;
        TemplateParameter(ParsingContext* ctx): TemplateParameterNode*;
        TemplateParameterList(ParsingContext* ctx, Node* owner);
    }
}
