// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_PARSING_MATCH_INCLUDED
#define CMINOR_PARSING_MATCH_INCLUDED

namespace cminor { namespace parsing {

class Match
{
public:
    Match(bool hit_, int length_): hit(hit_), length(length_) {}
    bool Hit() const { return hit; }
    int Length() const { return length; }
    void Concatenate(const Match& that)
    {
        length += that.length;
    }
    static Match Empty() 
    {
        return Match(true, 0);
    }
    static Match One()
    {
        return Match(true, 1);
    }
    static Match Nothing()
    {
        return Match(false, -1);
    }
private:
    bool hit;
    int length;
};

} } // namespace cminor::parsing

#endif // CMINOR_PARSING_MATCH_INCLUDED

