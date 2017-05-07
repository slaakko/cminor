namespace System.Json
{
    grammar JsonGrammar
    {
        Value : System.Json.Value;
        Object : System.Json.Object;
        Field(System.Json.Object o);
        Array : System.Json.Array;
        String(var System.Text.StringBuilder s) : System.Json.String;
        Escape : char;
        Number : System.Json.Number;
    }
}
