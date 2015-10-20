#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include "../check-builder.hpp"
#include "../layoutable.hpp"

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

struct toml_value_type : public layoutable, public check_builder_type {
    std::vector<int> base;
    std::vector<int> check;
    bool defined;

    toml_value_type () : check_builder_type (), base (), check (), defined (false) {}

    enum {WS=1,COMMENT, CR, LF, QUOTE, COMMA, DOT, EQ,
          LBRACKET, RBRACKET, LBRACE, RBRACE, PLUS, NUMERIC, LETTER, MATCH};
    enum {PUTLIT = 1};

    void
    define_charset ()
    {
        charset (0,127, 0);
        charset ('\t', WS);
        charset (' ', WS);
        charset ('#', COMMENT);
        charset ('\r', CR);
        charset ('\n', LF);
        charset ('"', QUOTE);
        charset ('\'', QUOTE);
        charset (',', COMMA);
        charset ('[', LBRACKET);
        charset (']', RBRACKET);
        charset ('{', LBRACE);
        charset ('}', RBRACE);
        charset ('0','9', NUMERIC);
        charset ('A','Z', LETTER);
        charset ('a','z', LETTER);
        charset ('_', LETTER);
        charset ('-', NUMERIC);
        charset ('+', PLUS);
    }

    void
    define_state ()
    {
        to (1, WS, 1);
        to (1, COMMENT, 2);
        to (1, CR, 3);
        to (1, LF, 4);
        to (1, QUOTE, 5);
        to (1, PLUS, 14);
        to (1, NUMERIC, 14);
        to (1, LETTER, 6, PUTLIT);
        to (1, LBRACE, 7);
        to (1, RBRACE, 8);
        to (1, LBRACKET, 9);
        to (1, RBRACKET, 10);
        to (1, DOT, 11);
        to (1, EQ, 12);
        to (1, COMMA, 13);

        to (2, CR, 3);
        to (2, LF, 4);

        to (3, LF, 4);

        to (4, WS, 4);
        to (4, COMMENT, 2);
        to (4, CR, 3);
        to (4, LF, 4);
        to (4, MATCH, TOKEN_ENDLINE);

        to (5, MATCH, TOKEN_STRING);

        to (6, LETTER, 6, PUTLIT);
        to (6, NUMERIC, 6, PUTLIT);
        to (6, MATCH, TOKEN_BOOLEAN);

        to (7, MATCH, TOKEN_LBRACE);
        to (8, MATCH, TOKEN_RBRACE);
        to (9, MATCH, TOKEN_LBRACKET);
        to (10, MATCH, TOKEN_RBRACKET);
        to (11, MATCH, TOKEN_DOT);
        to (12, MATCH, TOKEN_EQUAL);
        to (13, MATCH, TOKEN_COMMA);
        to (14, MATCH, TOKEN_FIXNUM);
    }

    toml_value_type&
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
toml_decoder_type::scan_value (value_type& value)
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
        if ('#' == octet) {
            while (s < e && ('\n' != *s && '\r' != *s))
                ++s;
            --s;
        }
        int const prev_state = next_state;
        next_state = 0;
        int const j = BASE[prev_state] + cls;
        int const m = BASE[prev_state] + MATCH;
        if (0 < j && j < NSHIFT && (SHIFT[j] & 0xff) == prev_state)
            next_state = (SHIFT[j] >> 8) & 0xff;
        if (0 < m && m < NSHIFT && (SHIFT[m] & 0xff) == prev_state) {
            kind = (SHIFT[m] >> 8) & 0xff;
            switch (kind) {
            case TOKEN_STRING:
                iter = s - 1;
                return scan_string (value);
            case TOKEN_FIXNUM:
                iter = s - 1;
                return scan_number (value);
            case TOKEN_LBRACE:
                kvstate = 1;
                break;
            default:
                break;
            }
            iter = s;
        }
        if (! next_state || s == e)
            break;
        if (SHIFT[j] >> 16)
            literal.push_back (octet);
    }
    if (kind == TOKEN_BOOLEAN) {
        if (literal == L"true")
            value = ::wjson::boolean (true);
        else if (literal == L"false")
            value = ::wjson::boolean (false);
    }
    if (kind == TOKEN_INVALID && s == e) {
        kind = TOKEN_ENDMARK;
        iter = s;
    }
    return kind;
}
)EOS"
);

int
main ()
{
    toml_value_type token;
    std::cout << token.define ().render (layout);
    return EXIT_SUCCESS;
}
