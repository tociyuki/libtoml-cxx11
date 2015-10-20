#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include "../check-builder.hpp"
#include "../layoutable.hpp"

enum {
    TOKEN_INVALID,
    TOKEN_SCALAR,
    TOKEN_STRING,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_ENDMARK,
};

struct toml_number_type : public layoutable, public check_builder_type {
    // character class
    enum { ZERO = 1, DIGIT, POINT, EXP, PLUS, MINUS, MATCH };

    std::vector<int> base;
    std::vector<int> check;
    bool defined;

    toml_number_type () : check_builder_type (), base (), check (), defined (false) {}

    void
    define_charset ()
    {
        charset (0, 127, 0);
        charset ('0',       ZERO);
        charset ('1','9',   DIGIT);
        charset ('.',       POINT);
        charset ('e',       EXP);
        charset ('E',       EXP);
        charset ('+',       PLUS);
        charset ('-',       MINUS);
    }

    void
    define_state ()
    {
        // RFC 7159 JSON  6. Numbers
        // [-]? ([0] / [1-9][0-9]*) ([.] [0-9]+)? ([eE][+-]?[0-9]+)?
        to (1,  MINUS,  2);
        to (1,  ZERO,   3);
        to (1,  DIGIT,  4);

        to (2,  ZERO,   3);
        to (2,  DIGIT,  4);

        to (3,  POINT,  5);
        to (3,  EXP,    7);
        to (3,  MATCH,  1);

        to (4,  ZERO,   4);
        to (4,  DIGIT,  4);
        to (4,  POINT,  5);
        to (4,  EXP,    7);
        to (4,  MATCH,  1);

        to (5,  ZERO,   6);
        to (5,  DIGIT,  6);

        to (6,  ZERO,   6);
        to (6,  DIGIT,  6);
        to (6,  EXP,    7);
        to (6,  MATCH,  2);

        to (7,  PLUS,   8);
        to (7,  MINUS,  8);
        to (7,  ZERO,   9);
        to (7,  DIGIT,  9);

        to (8,  ZERO,   9);
        to (8,  DIGIT,  9);

        to (9,  ZERO,   9);
        to (9,  DIGIT,  9);
        to (9,  MATCH,  2);
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
int
json_decoder_type::scan_number (value_type& value)
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
    int kind = TOKEN_INVALID;
    std::wstring literal;
    std::string::const_iterator s = iter;
    std::string::const_iterator const e = string.cend ();
    for (int next_state = 1; s <= e; ++s) {
        uint32_t octet = s == e ? '\0' : ord (*s);
        int const cls = s == e ? 0 : lookup_cls (CCLASS, @<nlookup@>U, octet);
        int const prev_state = next_state;
        next_state = 0;
        int const j = BASE[prev_state] + cls;
        int const m = BASE[prev_state] + MATCH;
        if (0 < j && j < NSHIFT && (SHIFT[j] & 0xff) == prev_state)
            next_state = (SHIFT[j] >> 8) & 0xff;
        if (next_state && s < e)
            literal.push_back (octet);
        if (0 < m && m < NSHIFT && (SHIFT[m] & 0xff) == prev_state) {
            int isfixnum = 1 == ((SHIFT[m] >> 8) & 0xff);
            if (isfixnum)
                try {
                    value = ::wjson::fixnum (std::stoll (literal));
                }
                catch (std::out_of_range) {
                    isfixnum = false;
                }
            if (! isfixnum)
                try {
                    value = ::wjson::flonum (std::stod (literal));
                }
                catch (std::out_of_range) {
                    value = ::wjson::null ();
                    return TOKEN_INVALID;
                }
            kind = TOKEN_SCALAR;
            iter = s;
        }
        if (! next_state)
            break;
    }
    return kind;
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
