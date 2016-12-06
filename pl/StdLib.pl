namespace cminor.parsing
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
        int: int;
        uint: unsigned;
        long: int64_t;
        ulong: uint64_t;
        hex: uint64_t;
        hex_literal: uint64_t;
        real: double;
        ureal: double;
        fractional_real;
        exponent_real;
        exponent_part;
        number: double;
        bool: bool;
        identifier: std::string;
        qualified_id: std::string;
        escape: char;
        char: char;
        string: std::string;
    }
}
