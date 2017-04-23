namespace System.Text.Parsing
{
    grammar stdlib
    {
        spaces;
        newline;
        comment;
        line_comment;
        block_comment;
        spaces_and_comments;
        digit_sequence;
        sign;
        sbyte : sbyte;
        byte : byte;
        short : short;
        ushort : ushort;
        int : int;
        uint : uint;
        long : long;
        ulong : ulong;
        hexbyte : byte;
        hexword : ushort;
        hexdword : uint;
        hexqword : ulong;
        hex : ulong;
        hex_literal : ulong;
        octaldigit : int;
        float : float;
        double : double;
        ureal : double;
        fractional_real;
        exponent_part;
        number : double;
        bool : bool;
        identifier : string;
        qualified_id : string;
        escape : char;
        char : char;
        string(var StringBuilder s) : string;
    }
}
