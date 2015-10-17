#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include "check-builder.hpp"
#include "layoutable.hpp"

enum {
    TOKEN_INVALID,
    TOKEN_BAREKEY,
    TOKEN_STRKEY,
    TOKEN_STRING,
    TOKEN_BOOLEAN,
    TOKEN_FIXNUM,
    TOKEN_FLONUM,
    TOKEN_DATETIME,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_DOT,
    TOKEN_EQUAL,
    TOKEN_COMMA,
    TOKEN_ENDLINE,
    TOKEN_ENDMARK,
};

struct toml_number_type : public layoutable, public check_builder_type {
    enum { DIGIT = 1, USCORE, MINUS, PLUS, CDOT, EXP, COLON, TIME, UTC, MATCH };

    std::vector<int> base;
    std::vector<int> check;
    bool defined;

    toml_number_type () : check_builder_type (), base (), check (), defined (false) {}

    void
    define_charset ()
    {
        charset (0,127,     0);
        charset ('0','9',   DIGIT);
        charset ('_',       USCORE);
        charset ('-',       MINUS);
        charset ('+',       PLUS);
        charset ('.',       CDOT);
        charset ('e',       EXP);
        charset ('E',       EXP);
        charset (':',       COLON);
        charset ('T',       TIME);
        charset ('Z',       UTC);
    }

    void
    define_state ()
    {
        // datetime|integer|float
        to ( 1, DIGIT,   2);

        to ( 2, DIGIT,   3);
        to ( 2, USCORE, 29);
        to ( 2, MATCH,  TOKEN_FIXNUM);
        to ( 2, CDOT,   31);
        to ( 2, EXP,    33);

        to ( 3, DIGIT,   4);
        to ( 3, USCORE, 29);
        to ( 3, MATCH,  TOKEN_FIXNUM);
        to ( 3, CDOT,   31);
        to ( 3, EXP,    33);

        to ( 4, DIGIT,   5);
        to ( 4, USCORE, 29);
        to ( 4, MATCH,  TOKEN_FIXNUM);
        to ( 4, CDOT,   31);
        to ( 4, EXP,    33);

        to ( 5, MINUS,   6);
        to ( 5, DIGIT,   5);
        to ( 5, USCORE, 29);
        to ( 5, MATCH,  TOKEN_FIXNUM);
        to ( 5, CDOT,   31);
        to ( 5, EXP,    33);

        to ( 6, DIGIT,   7);
        to ( 7, DIGIT,   8);
        to ( 8, MINUS,   9);
        to ( 9, DIGIT,  10);
        to (10, DIGIT,  11);
        to (11, TIME,   12);
        to (11, UTC,    28);
        to (11, PLUS,   23);
        to (11, MINUS,  23);
        to (11, MATCH,  TOKEN_DATETIME);
        to (12, DIGIT,  13);
        to (13, DIGIT,  14);
        to (14, COLON,  15);
        to (15, DIGIT,  16);
        to (16, DIGIT,  17);
        to (17, COLON,  18);
        to (17, UTC,    28);
        to (17, PLUS,   23);
        to (17, MINUS,  23);
        to (17, MATCH,  TOKEN_DATETIME);
        to (18, DIGIT,  19);
        to (19, DIGIT,  20);
        to (20, UTC,    28);
        to (20, PLUS,   23);
        to (20, MINUS,  23);
        to (20, CDOT,   21);
        to (20, MATCH,  TOKEN_DATETIME);
        to (21, DIGIT,  22);
        to (22, DIGIT,  22);
        to (22, UTC,    28);
        to (22, PLUS,   23);
        to (22, MINUS,  23);
        to (22, MATCH,  TOKEN_DATETIME);
        to (23, DIGIT,  24);
        to (24, DIGIT,  25);
        to (25, COLON,  26);
        to (26, DIGIT,  27);
        to (27, DIGIT,  28);
        to (28, MATCH,  TOKEN_DATETIME);

        // integer|float
        to ( 1, PLUS,   29);
        to ( 1, MINUS,  29);
        to (29, DIGIT,  30);
        to (30, DIGIT,  30);
        to (30, USCORE, 29);
        to (30, MATCH,  TOKEN_FIXNUM);
        to (30, CDOT,   31);
        to (30, EXP,    33);
        to (31, DIGIT,  32);
        to (32, DIGIT,  32);
        to (32, USCORE, 31);
        to (32, MATCH,  TOKEN_FLONUM);
        to (32, EXP,    33);
        to (33, PLUS,   34);
        to (33, MINUS,  34);
        to (33, DIGIT,  35);
        to (34, DIGIT,  35);
        to (35, DIGIT,  35);
        to (35, USCORE, 34);
        to (35, MATCH,  TOKEN_FLONUM);
    }

    toml_number_type&
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
bool
decoder_type::scan_number (std::string::const_iterator s, parsed_type& parsed)
{
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
    parsed.kind = TOKEN_INVALID;
    std::string literal;
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
        if (next_state && s < e && '_' != octet)
            literal.push_back (octet);
        if (0 < m && m < NSHIFT && (SHIFT[m] & 0xff) == prev_state) {
            parsed.kind = (SHIFT[m] >> 8) & 0xff;
            try {
                if (TOKEN_FIXNUM == parsed.kind)
                    parsed.literal_fixnum = std::stoll (literal);
                else if (TOKEN_FLONUM == parsed.kind)
                    parsed.literal_flonum = std::stod (literal);
            }
            catch (...) {
                parsed.kind = TOKEN_INVALID;
                parsed.literal.clear ();
                return false;
            }
            parsed.literal = literal;
            parsed.s = s;
        }
        if (! next_state)
            break;
    }
    return TOKEN_INVALID != parsed.kind;
}
)EOS"
);

int
main ()
{
    toml_number_type number;
    std::cout << number.define ().render (layout);
    return EXIT_SUCCESS;
}
