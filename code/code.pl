namespace cminor.code
{
    grammar DeclarationGrammar
    {
        BlockDeclaration: cminor::codedom::CppObject*;
        SimpleDeclaration(var std::unique_ptr<SimpleDeclaration> sd): cminor::codedom::SimpleDeclaration*;
        DeclSpecifierSeq(cminor::codedom::SimpleDeclaration* declaration);
        DeclSpecifier: cminor::codedom::DeclSpecifier*;
        StorageClassSpecifier: cminor::codedom::StorageClassSpecifier*;
        TypeSpecifier: cminor::codedom::TypeSpecifier*;
        SimpleTypeSpecifier: cminor::codedom::TypeSpecifier*;
        TypeName: cminor::codedom::TypeName*;
        TemplateArgumentList(cminor::codedom::TypeName* typeName);
        TemplateArgument: cminor::codedom::CppObject*;
        Typedef: cminor::codedom::DeclSpecifier*;
        CVQualifier: cminor::codedom::TypeSpecifier*;
        NamespaceAliasDefinition: cminor::codedom::UsingObject*;
        UsingDeclaration: cminor::codedom::UsingObject*;
        UsingDirective: cminor::codedom::UsingObject*;
    }
    grammar StatementGrammar
    {
        Statement: cminor::codedom::Statement*;
        LabeledStatement: cminor::codedom::Statement*;
        Label: std::u32string;
        EmptyStatement: cminor::codedom::Statement*;
        ExpressionStatement: cminor::codedom::Statement*;
        CompoundStatement(var std::unique_ptr<CompoundStatement> cs): cminor::codedom::CompoundStatement*;
        SelectionStatement: cminor::codedom::Statement*;
        IfStatement: cminor::codedom::Statement*;
        SwitchStatement: cminor::codedom::Statement*;
        IterationStatement: cminor::codedom::Statement*;
        WhileStatement: cminor::codedom::Statement*;
        DoStatement: cminor::codedom::Statement*;
        ForStatement: cminor::codedom::Statement*;
        ForInitStatement: cminor::codedom::CppObject*;
        JumpStatement: cminor::codedom::Statement*;
        BreakStatement: cminor::codedom::Statement*;
        ContinueStatement: cminor::codedom::Statement*;
        ReturnStatement: cminor::codedom::Statement*;
        GotoStatement: cminor::codedom::Statement*;
        GotoTarget: std::u32string;
        DeclarationStatement: cminor::codedom::Statement*;
        Condition(var std::unique_ptr<TypeId> ti): cminor::codedom::CppObject*;
        TryStatement: cminor::codedom::TryStatement*;
        HandlerSeq(TryStatement* st);
        Handler: cminor::codedom::Handler*;
        ExceptionDeclaration(var std::unique_ptr<ExceptionDeclaration> ed): cminor::codedom::ExceptionDeclaration*;
    }
    grammar DeclaratorGrammar
    {
        InitDeclaratorList(var std::unique_ptr<InitDeclaratorList> idl): cminor::codedom::InitDeclaratorList*;
        InitDeclarator: cminor::codedom::InitDeclarator*;
        Declarator: std::u32string;
        DirectDeclarator(var std::unique_ptr<CppObject> o);
        DeclaratorId(var std::unique_ptr<CppObject> o);
        TypeId(var std::unique_ptr<TypeId> ti): cminor::codedom::TypeId*;
        Type(var std::unique_ptr<Type> t): cminor::codedom::Type*;
        TypeSpecifierSeq(cminor::codedom::TypeId* typeId);
        AbstractDeclarator: std::u32string;
        DirectAbstractDeclarator(var std::unique_ptr<CppObject> o);
        PtrOperator;
        CVQualifierSeq(var std::unique_ptr<CppObject> o);
        Initializer: cminor::codedom::Initializer*;
        InitializerClause: cminor::codedom::AssignInit*;
        InitializerList(cminor::codedom::AssignInit* init);
    }
    grammar ExpressionGrammar
    {
        Expression: cminor::codedom::CppObject*;
        ConstantExpression: cminor::codedom::CppObject*;
        AssignmentExpression(var std::unique_ptr<CppObject> lor): cminor::codedom::CppObject*;
        AssingmentOp: Operator;
        ThrowExpression: cminor::codedom::CppObject*;
        ConditionalExpression: cminor::codedom::CppObject*;
        LogicalOrExpression: cminor::codedom::CppObject*;
        LogicalAndExpression: cminor::codedom::CppObject*;
        InclusiveOrExpression: cminor::codedom::CppObject*;
        ExclusiveOrExpression: cminor::codedom::CppObject*;
        AndExpression: cminor::codedom::CppObject*;
        EqualityExpression: cminor::codedom::CppObject*;
        EqOp: Operator;
        RelationalExpression: cminor::codedom::CppObject*;
        RelOp: Operator;
        ShiftExpression: cminor::codedom::CppObject*;
        ShiftOp: Operator;
        AdditiveExpression: cminor::codedom::CppObject*;
        AddOp: Operator;
        MultiplicativeExpression: cminor::codedom::CppObject*;
        MulOp: Operator;
        PmExpression: cminor::codedom::CppObject*;
        PmOp: Operator;
        CastExpression(var std::unique_ptr<CppObject> ce, var std::unique_ptr<CppObject> ti): cminor::codedom::CppObject*;
        UnaryExpression(var std::unique_ptr<CppObject> ue): cminor::codedom::CppObject*;
        UnaryOperator: Operator;
        NewExpression(var bool global, var TypeId* typeId, var bool parens): cminor::codedom::CppObject*;
        NewTypeId(var std::unique_ptr<TypeId> ti): cminor::codedom::TypeId*;
        NewDeclarator: std::u32string;
        DirectNewDeclarator(var std::unique_ptr<CppObject> e);
        NewPlacement: std::vector<cminor::codedom::CppObject*>;
        NewInitializer: std::vector<cminor::codedom::CppObject*>;
        DeleteExpression(var bool global, var bool array): cminor::codedom::CppObject*;
        PostfixExpression(var std::unique_ptr<CppObject> pe): cminor::codedom::CppObject*;
        TypeSpecifierOrTypeName: cminor::codedom::CppObject*;
        PostCastExpression: cminor::codedom::CppObject*;
        ExpressionList: std::vector<cminor::codedom::CppObject*>;
        PrimaryExpression(var std::unique_ptr<CppObject> pe): cminor::codedom::CppObject*;
        IdExpression: cminor::codedom::IdExpr*;
        OperatorFunctionId;
        Operator;
    }
    grammar LiteralGrammar
    {
        Literal: cminor::codedom::Literal*;
        IntegerLiteral: cminor::codedom::Literal*;
        DecimalLiteral;
        OctalLiteral;
        OctalDigit;
        HexadecimalLiteral;
        IntegerSuffix;
        UnsignedSuffix;
        LongLongSuffix;
        LongSuffix;
        CharacterLiteral: cminor::codedom::Literal*;
        NarrowCharacterLiteral;
        UniversalCharacterLiteral;
        WideCharacterLiteral;
        CCharSequence: std::u32string;
        CChar;
        EscapeSequence;
        SimpleEscapeSequence;
        OctalEscapeSequence;
        HexadecimalEscapeSequence;
        FloatingLiteral: cminor::codedom::Literal*;
        FractionalConstant;
        DigitSequence;
        ExponentPart;
        Sign;
        FloatingSuffix;
        StringLiteral: cminor::codedom::Literal*;
        EncodingPrefix;
        SCharSequence;
        SChar;
        BooleanLiteral: cminor::codedom::Literal*;
        PointerLiteral: cminor::codedom::Literal*;
    }
    grammar IdentifierGrammar
    {
        Identifier: std::u32string;
        QualifiedId: std::u32string;
    }
    grammar KeywordGrammar
    {
        Keyword;
    }
}
