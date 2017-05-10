namespace System.Xml
{
    grammar XmlContentGrammar
    {
        XmlContent(XmlProcessor processor);
    }
    grammar XmlDocumentGrammar
    {
        Document(XmlProcessor processor);
        Char;
        S;
        NameStartChar;
        NameChar;
        Name : string;
        Names;
        Nmtoken;
        Nmtokens;
        EntityValue(XmlProcessor processor, var StringBuilder entityValueBuilder) : string;
        AttValue(XmlProcessor processor, var StringBuilder attValueBuilder) : string;
        SystemLiteral(XmlProcessor processor);
        PubidLiteral(XmlProcessor processor);
        PubidChar;
        CharData(XmlProcessor processor);
        Comment(XmlProcessor processor);
        PI(XmlProcessor processor);
        PITarget : string;
        CDSect(XmlProcessor processor);
        CDStart;
        CData : string;
        CDEnd;
        Prolog(XmlProcessor processor);
        XMLDecl(XmlProcessor processor);
        VersionInfo(XmlProcessor processor);
        Eq;
        VersionNum;
        Misc(XmlProcessor processor);
        DocTypeDecl(XmlProcessor processor);
        DeclSep(XmlProcessor processor);
        IntSubset(XmlProcessor processor);
        MarkupDecl(XmlProcessor processor);
        ExtSubset(XmlProcessor processor);
        ExtSubsetDecl(XmlProcessor processor);
        SDDecl;
        Element(XmlProcessor processor);
        STag(XmlProcessor processor);
        Attribute(XmlProcessor processor);
        ETag(XmlProcessor processor);
        Content(XmlProcessor processor);
        EmptyElemTag(XmlProcessor processor, var string tagName);
        ElementDecl;
        ContentSpec;
        Children;
        CP;
        Choice;
        Seq;
        Mixed;
        AttlistDecl(XmlProcessor processor);
        AttDef(XmlProcessor processor);
        AttType;
        StringType;
        TokenizedType;
        EnumeratedType;
        NotationType;
        Enumeration;
        DefaultDecl(XmlProcessor processor);
        ConditionalSect(XmlProcessor processor);
        IncludeSect(XmlProcessor processor);
        IgnoreSect;
        IgnoreSectContents;
        Ignore;
        CharRef : char;
        Reference(XmlProcessor processor);
        EntityRef(XmlProcessor processor);
        PEReference(XmlProcessor processor);
        EntityDecl(XmlProcessor processor);
        GEDecl(XmlProcessor processor);
        PEDecl(XmlProcessor processor);
        EntityDef(XmlProcessor processor) : string;
        PEDef(XmlProcessor processor);
        ExternalID(XmlProcessor processor);
        NDataDecl;
        TextDecl(XmlProcessor processor);
        ExtParsedEnt(XmlProcessor processor);
        EncodingDecl(XmlProcessor processor);
        EncName : string;
        NotationDecl(XmlProcessor processor);
        PublicID(XmlProcessor processor);
    }
}
