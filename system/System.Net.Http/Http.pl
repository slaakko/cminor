namespace System.Net.Http
{
    grammar HttpConnectionHeaderGrammar
    {
        ConnectionTokenList : List<ConnectionToken>;
    }
    grammar HttpHeaderGrammar
    {
        HttpHeader : HttpHeader;
    }
    grammar HttpStatusGrammar
    {
        HttpStatus : HttpStatus;
    }
    grammar HttpGrammar
    {
        OCTET;
        CHAR : char;
        UPALPHA;
        LOALPHA;
        ALPHA;
        DIGIT;
        CTL;
        CR;
        LF;
        SP;
        HT;
        QUOTE;
        CRLF;
        LWS;
        TEXT;
        HEX;
        separator;
        http_token;
        comment;
        ctext;
        quoted_string(var StringBuilder s) : string;
        qdtext;
        quoted_pair : char;
        HTTP_Version : string;
        StatusCode : ushort;
        ReasonPhrase : string;
        StatusLine : HttpStatus;
        HTTP_Date;
        RFC1123_date;
        RFC850_date;
        asctime_date;
        Year;
        Year2;
        Day;
        Day1;
        date1;
        date2;
        date3;
        time;
        Hour;
        Min;
        Sec;
        wkday;
        weekday;
        month;
        delta_seconds;
        charset;
        content_coding;
        transfer_coding;
        transfer_extension;
        parameter;
        attribute;
        value;
        media_type;
        type;
        subtype;
        header : HttpHeader;
        field_name : string;
        field_value : string;
        ConnectionTokens : List<ConnectionToken>;
        connection_token : ConnectionToken;
    }
    grammar UriGrammar
    {
        UriReference : UriReference;
        AbsoluteUri(UriReference uriReference);
        RelativeUri(UriReference uriReference);
        HierPart(UriReference uriReference);
        OpaquePart;
        UriCNoSlash;
        NetPath(UriReference uriReference);
        AbsPath;
        RelPath;
        RelSegment;
        Scheme;
        Authority(UriReference uriReference);
        RegName;
        Server(UriReference uriReference, var string userInfo);
        UserInfo;
        HostPort(UriReference uriReference);
        Host;
        HostName(var string dl);
        DomainLabel;
        TopLabel;
        IPv4Address;
        Port : uint;
        Path;
        PathSegments;
        Segment;
        Param;
        PChar;
        Query;
        Fragment;
        UriC;
        Reserved;
        Unreserved;
        Mark;
        Escaped;
        AlphaNumOrDash;
        AlphaNum;
        Alpha;
        Digit;
    }
    grammar MediaTypeGrammar
    {
        MimeType : MimeType;
        MediaRanges : List<MediaRange>;
        AcceptParams(MediaRange mediaRange);
        AcceptExtension(MediaRange mediaRange, var string extattr);
        MediaRange : MediaRange;
        type(MimeType mimeType);
        discrete_type(MimeType mimeType);
        extension_token;
        x_token;
        ietf_token;
        composite_type(MimeType mimeType);
        subtype(MimeType mimeType);
        parameter(MimeType mimeType);
        attribute : string;
        value : string;
        qvalue : double;
    }
}
