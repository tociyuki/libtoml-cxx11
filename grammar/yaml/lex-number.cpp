#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include "../check-builder.hpp"
#include "../layoutable.hpp"

enum {
    TOKEN_INVALID,
    TOKEN_FIXNUM,
    TOKEN_FLONUM,
    TOKEN_DATETIME,
};

struct yaml_number_type : public layoutable, public check_builder_type {
    enum { ZERO = 1, OCTET, DECIMAL, XDIGIT, MINUS, PLUS,
           POINT, EXP, COLON, TIME, UTC, LO, LX, MATCH };

    std::vector<int> base;
    std::vector<int> check;
    bool defined;

    yaml_number_type () : check_builder_type (), base (), check (), defined (false) {}

    void
    define_charset ()
    {
        charset (0,127,     0);
        charset ('0',       ZERO);
        charset ('1','7',   OCTET);
        charset ('8','9',   DECIMAL);
        charset ('A','F',   XDIGIT);
        charset ('a','f',   XDIGIT);
        charset ('-',       MINUS);
        charset ('+',       PLUS);
        charset ('.',       POINT);
        charset ('e',       EXP);
        charset ('E',       EXP);
        charset (':',       COLON);
        charset ('T',       TIME);
        charset ('Z',       UTC);
        charset ('o',       LO);
        charset ('x',       LX);
    }

    void
    define_state ()
    {
        enum { PUTNUM = 1, PUTOCT, PUTHEX };
        std::vector<int> octet {ZERO, OCTET};
        std::vector<int> digit {ZERO, OCTET, DECIMAL};
        std::vector<int> xdigit {ZERO, OCTET, DECIMAL, XDIGIT, EXP};

        // YAML 1.2 Core Schema
        // [-+]? [0-9]+ tag:yaml.org,2002:int
        // [0][o][0-7]+ tag:yaml.org,2002:int
        // [0][x][0-9a-fA-F]+ tag:yaml.org,2002:int
        // [-+]? ([.][0-9]+ | [0-9]+ ([.][0-9]*)?) ([eE][-+]?[0-9]+)?

        to (1, MINUS,   7, PUTNUM);
        to (1, PLUS,    7, PUTNUM);
        to (1, POINT,   8, PUTNUM);
        for (int k : digit) to (1, k, 10, PUTNUM);
        to (1, ZERO,    2, PUTNUM);

        to (2, LO,      3);
        to (2, LX,      5);
        to (2, POINT,   11, PUTNUM);
        for (int k : digit) to (2, k, 8, PUTNUM);
        to (2, EXP,     12, PUTNUM);
        to (2, MATCH,   TOKEN_FIXNUM);

        for (int k : octet) to (3, k, 4, PUTOCT);
        for (int k : octet) to (4, k, 4, PUTNUM);
        to (4, MATCH,   TOKEN_FIXNUM);

        for (int k : xdigit) to (5, k, 6, PUTHEX);
        for (int k : xdigit) to (6, k, 6, PUTNUM);
        to (6, MATCH,   TOKEN_FIXNUM);

        for (int k : digit) to (7, k, 10, PUTNUM);
        to (7, POINT,   8, PUTNUM);
        for (int k : digit) to (8, k, 9, PUTNUM);
        for (int k : digit) to (9, k, 9, PUTNUM);
        to (9, EXP,     12, PUTNUM);
        to (9, MATCH,   TOKEN_FLONUM);

        for (int k : digit) to (10, k, 10, PUTNUM);
        to (10, EXP,     12, PUTNUM);
        to (10, POINT,   11, PUTNUM);
        to (10, MATCH,   TOKEN_FIXNUM);
        for (int k : digit) to (11, k, 11, PUTNUM);
        to (11, EXP,     12, PUTNUM);
        to (11, MATCH,   TOKEN_FLONUM);

        to (12, MINUS,  13, PUTNUM);
        to (12, PLUS,   13, PUTNUM);
        for (int k : digit) to (12, k, 14, PUTNUM);
        for (int k : digit) to (13, k, 14, PUTNUM);
        for (int k : digit) to (14, k, 14, PUTNUM);
        to (14, MATCH,   TOKEN_FLONUM);
    }

    yaml_number_type&
    define ()
    {
        if (defined)
            return *this;
        defined = true;
        define_charset ();
        define_state ();
        squarize_table ();
        pack_table (base, check);
        return *this;
    }

    std::string
    render (std::string const& layout)
    {
        std::string s = render_int (layout, "@<ncclass@>", cclass.size () / 8);
        s = render_int (s, "@<nlookup@>", cclass.size ());
        s = render_int (s, "@<nbase@>", base.size ());
        s = render_int (s, "@<ncheck@>", check.size ());
        s = render_cclass (s, "@<cclass@>\n", cclass);
        s = render_vector_dec (s, "@<base@>\n", base);
        s = render_vector_hex (s, "@<check@>\n", check);
        s = render_int (s, "@<match@>", MATCH);
        return std::move (s);
    }
};

std::string const layout (R"EOS(
int
scan_number (std::string const& string, value_type& value)
{
    enum { PUTNUM = 1, PUTOCT, PUTHEX };
    enum { NSHIFT = @<ncheck@> };
    static const uint32_t CCLASS[@<ncclass@>] = {
        @<cclass@>
    };
    static const int BASE[@<nbase@>] = {
        @<base@>
    };
    static const int SHIFT[NSHIFT] = {
        @<check@>
    };
    static const uint32_t MATCH = @<match@>U;
    int kind = TOKEN_INVALID;
    int base = 10;
    std::wstring literal;
    std::string::const_iterator s = string.cbegin ();
    std::string::const_iterator const e = string.cend ();
    for (int next_state = 1; s <= e; ++s) {
        uint32_t octet = s == e ? '\0' : ord (*s);
        int cls = s == e ? 0 : lookup_cls (CCLASS, @<nlookup@>U, octet);
        int const prev_state = next_state;
        next_state = 0;
        int j = BASE[prev_state] + cls;
        int m = BASE[prev_state] + MATCH;
        if (0 < j && j < NSHIFT && (SHIFT[j] & 0xff) == prev_state)
            next_state = (SHIFT[j] >> 8) & 0xff;
        if (next_state)
            switch (SHIFT[j] >> 16) {
            case PUTNUM:
                literal.push_back (octet);
                break;
            case PUTOCT:
                base = 8;
                literal.push_back (octet);
                break;
            case PUTHEX:
                base = 16;
                literal.push_back (octet);
                break;
            }
        if (0 < m && m < NSHIFT && (SHIFT[m] & 0xff) == prev_state) {
            kind = (SHIFT[m] >> 8) & 0xff;
        }
        if (! next_state)
            break;
    }
    if (s < e)
        return TOKEN_INVALID;
    try {
        std::size_t pos = 0;
        if (TOKEN_FIXNUM == kind)
            value = ::wjson::fixnum (std::stoll (literal, &pos, base));
        else if (TOKEN_FLONUM == kind)
            value = ::wjson::flonum (std::stod (literal));
    }
    catch (std::out_of_range) {
        value = ::wjson::null ();
        return TOKEN_INVALID;
    }
    return kind;
}
)EOS"
);

int
main ()
{
    yaml_number_type number;
    std::cout << number.define ().render (layout);
    return EXIT_SUCCESS;
}
