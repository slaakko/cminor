namespace cpg.cpp
{
    grammar DeclarationGrammar
    {
        BlockDeclaration: cminor::pom::CppObject*;
        SimpleDeclaration(var std::unique_ptr<SimpleDeclaration> sd): cminor::pom::SimpleDeclaration*;
        DeclSpecifierSeq(cminor::pom::SimpleDeclaration* declaration);
        DeclSpecifier: cminor::pom::DeclSpecifier*;
        StorageClassSpecifier: cminor::pom::StorageClassSpecifier*;
        TypeSpecifier: cminor::pom::TypeSpecifier*;
        SimpleTypeSpecifier: cminor::pom::TypeSpecifier*;
        TypeName: cminor::pom::TypeName*;
        TemplateArgumentList(cminor::pom::TypeName* typeName);
        TemplateArgument: cminor::pom::CppObject*;
        Typedef: cminor::pom::DeclSpecifier*;
        CVQualifier: cminor::pom::TypeSpecifier*;
        NamespaceAliasDefinition: cminor::pom::UsingObject*;
        UsingDeclaration: cminor::pom::UsingObject*;
        UsingDirective: cminor::pom::UsingObject*;
    }
    grammar LiteralGrammar
    {
        Literal: cminor::pom::Literal*;
        IntegerLiteral: cminor::pom::Literal*;
        DecimalLiteral;
        OctalLiteral;
        OctalDigit;
        HexadecimalLiteral;
        IntegerSuffix;
        UnsignedSuffix;
        LongLongSuffix;
        LongSuffix;
        CharacterLiteral: cminor::pom::Literal*;
        NarrowCharacterLiteral;
        UniversalCharacterLiteral;
        WideCharacterLiteral;
        CCharSequence: std::string;
        CChar;
        EscapeSequence;
        SimpleEscapeSequence;
        OctalEscapeSequence;
        HexadecimalEscapeSequence;
        FloatingLiteral: cminor::pom::Literal*;
        FractionalConstant;
        DigitSequence;
        ExponentPart;
        Sign;
        FloatingSuffix;
        StringLiteral: cminor::pom::Literal*;
        EncodingPrefix;
        SCharSequence;
        SChar;
        BooleanLiteral: cminor::pom::Literal*;
        PointerLiteral: cminor::pom::Literal*;
    }
    grammar KeywordGrammar
    {
        Keyword;
    }
    grammar IdentifierGrammar
    {
        Identifier: std::string;
        QualifiedId: std::string;
    }
    grammar DeclaratorGrammar
    {
        InitDeclaratorList(var std::unique_ptr<InitDeclaratorList> idl): cminor::pom::InitDeclaratorList*;
        InitDeclarator: cminor::pom::InitDeclarator*;
        Declarator: std::string;
        DirectDeclarator(var std::unique_ptr<CppObject> o);
        DeclaratorId(var std::unique_ptr<CppObject> o);
        TypeId(var std::unique_ptr<TypeId> ti): cminor::pom::TypeId*;
        Type(var std::unique_ptr<Type> t): cminor::pom::Type*;
        TypeSpecifierSeq(cminor::pom::TypeId* typeId);
        AbstractDeclarator: std::string;
        DirectAbstractDeclarator(var std::unique_ptr<CppObject> o);
        PtrOperator;
        CVQualifierSeq(var std::unique_ptr<CppObject> o);
        Initializer: cminor::pom::Initializer*;
        InitializerClause: cminor::pom::AssignInit*;
        InitializerList(cminor::pom::AssignInit* init);
    }
    grammar ExpressionGrammar
    {
        Expression: cminor::pom::CppObject*;
        ConstantExpression: cminor::pom::CppObject*;
        AssignmentExpression(var std::unique_ptr<CppObject> lor): cminor::pom::CppObject*;
        AssingmentOp: Operator;
        ThrowExpression: cminor::pom::CppObject*;
        ConditionalExpression: cminor::pom::CppObject*;
        LogicalOrExpression: cminor::pom::CppObject*;
        LogicalAndExpression: cminor::pom::CppObject*;
        InclusiveOrExpression: cminor::pom::CppObject*;
        ExclusiveOrExpression: cminor::pom::CppObject*;
        AndExpression: cminor::pom::CppObject*;
        EqualityExpression: cminor::pom::CppObject*;
        EqOp: Operator;
        RelationalExpression: cminor::pom::CppObject*;
        RelOp: Operator;
        ShiftExpression: cminor::pom::CppObject*;
        ShiftOp: Operator;
        AdditiveExpression: cminor::pom::CppObject*;
        AddOp: Operator;
        MultiplicativeExpression: cminor::pom::CppObject*;
        MulOp: Operator;
        PmExpression: cminor::pom::CppObject*;
        PmOp: Operator;
        CastExpression(var std::unique_ptr<CppObject> ce, var std::unique_ptr<CppObject> ti): cminor::pom::CppObject*;
        UnaryExpression(var std::unique_ptr<CppObject> ue): cminor::pom::CppObject*;
        UnaryOperator: Operator;
        NewExpression(var bool global, var TypeId* typeId, var bool parens): cminor::pom::CppObject*;
        NewTypeId(var std::unique_ptr<TypeId> ti): cminor::pom::TypeId*;
        NewDeclarator: std::string;
        DirectNewDeclarator(var std::unique_ptr<CppObject> e);
        NewPlacement: std::vector<cminor::pom::CppObject*>;
        NewInitializer: std::vector<cminor::pom::CppObject*>;
        DeleteExpression(var bool global, var bool array): cminor::pom::CppObject*;
        PostfixExpression(var std::unique_ptr<CppObject> pe): cminor::pom::CppObject*;
        TypeSpecifierOrTypeName: cminor::pom::CppObject*;
        PostCastExpression: cminor::pom::CppObject*;
        ExpressionList: std::vector<cminor::pom::CppObject*>;
        PrimaryExpression(var std::unique_ptr<CppObject> pe): cminor::pom::CppObject*;
        IdExpression: cminor::pom::IdExpr*;
        OperatorFunctionId;
        Operator;
    }
    grammar StatementGrammar
    {
        Statement: cminor::pom::Statement*;
        LabeledStatement: cminor::pom::Statement*;
        Label: std::string;
        EmptyStatement: cminor::pom::Statement*;
        ExpressionStatement: cminor::pom::Statement*;
        CompoundStatement(var std::unique_ptr<CompoundStatement> cs): cminor::pom::CompoundStatement*;
        SelectionStatement: cminor::pom::Statement*;
        IfStatement: cminor::pom::Statement*;
        SwitchStatement: cminor::pom::Statement*;
        IterationStatement: cminor::pom::Statement*;
        WhileStatement: cminor::pom::Statement*;
        DoStatement: cminor::pom::Statement*;
        ForStatement: cminor::pom::Statement*;
        ForInitStatement: cminor::pom::CppObject*;
        JumpStatement: cminor::pom::Statement*;
        BreakStatement: cminor::pom::Statement*;
        ContinueStatement: cminor::pom::Statement*;
        ReturnStatement: cminor::pom::Statement*;
        GotoStatement: cminor::pom::Statement*;
        GotoTarget: std::string;
        DeclarationStatement: cminor::pom::Statement*;
        Condition(var std::unique_ptr<TypeId> ti): cminor::pom::CppObject*;
        TryStatement: cminor::pom::TryStatement*;
        HandlerSeq(TryStatement* st);
        Handler: cminor::pom::Handler*;
        ExceptionDeclaration(var std::unique_ptr<ExceptionDeclaration> ed): cminor::pom::ExceptionDeclaration*;
    }
}
