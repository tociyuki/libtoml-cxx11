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

struct json_token_type : public layoutable, public check_builder_type {
    // character class
    enum { SNUMBER = 10, WS, MATCH };
    enum { SETITER = 1, PUTLIT };

    std::vector<int> base;
    std::vector<int> check;
    bool defined;

    json_token_type () : check_builder_type (), base (), check (), defined (false) {}

    void
    define_charset ()
    {
        //  'true' / 'false' / 'null'
        //  / [-]?('0'/[1-9][0-9]*)('.'[0-9]+)?([eE][-+]?[0-9]+)?
        //  / '{', '}', '[', ']', ':', ','
        charset (0, 127, 0);
        charset ('\t', WS);
        charset ('\n', WS);
        charset ('\r', WS);
        charset (' ', WS);
        charset ('"', TOKEN_STRING);
        charset (',', TOKEN_COMMA);
        charset ('-', SNUMBER);
        charset ('0','9', SNUMBER);
        charset (':', TOKEN_COLON);
        charset ('[', TOKEN_LBRACKET);
        charset (']', TOKEN_RBRACKET);
        charset ('a','z', TOKEN_SCALAR);
        charset ('{', TOKEN_LBRACE);
        charset ('}', TOKEN_RBRACE);
    }

    void
    define_state ()
    {
        to ( 1, WS,             1,  SETITER);
        to ( 1, TOKEN_STRING,   3);
        to ( 1, TOKEN_COMMA,    4);
        to ( 1, SNUMBER,        5);
        to ( 1, TOKEN_COLON,    6);
        to ( 1, TOKEN_LBRACKET, 7);
        to ( 1, TOKEN_RBRACKET, 8);
        to ( 1, TOKEN_SCALAR,   2,  PUTLIT);
        to ( 1, TOKEN_LBRACE,   9);
        to ( 1, TOKEN_RBRACE,  10);

        to ( 2, TOKEN_SCALAR,   2,  PUTLIT);
        to ( 2, MATCH, TOKEN_SCALAR);

        to ( 3, MATCH, TOKEN_STRING);
        to ( 4, MATCH, TOKEN_COMMA);
        to ( 5, MATCH, SNUMBER);
        to ( 6, MATCH, TOKEN_COLON);
        to ( 7, MATCH, TOKEN_LBRACKET);
        to ( 8, MATCH, TOKEN_RBRACKET);
        to ( 9, MATCH, TOKEN_LBRACE);
        to (10, MATCH, TOKEN_RBRACE);
    }

    json_token_type&
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
json_decoder_type::next_token (value_type& value)
{
    enum { NSHIFT = @<ncheck@>, SNUMBER = 10 };
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
        else if (0 < m && m < NSHIFT && (SHIFT[m] & 0xff) == prev_state) {
            kind = (SHIFT[m] >> 8) & 0xff;
            switch (kind) {
            case TOKEN_STRING:
                kind = scan_string (value);
                break;
            case SNUMBER:
                kind = scan_number (value);
                break;
            case TOKEN_SCALAR:
                if (literal == L"true")
                    value = ::wjson::boolean (true);
                else if (literal == L"false")
                    value = ::wjson::boolean (false);
                else if (literal == L"null")
                    value = ::wjson::null ();
                else
                    return false;
                iter = s;
                break;
            default:
                iter = s;
                break;
            }
        }
        if (! next_state || s == e)
            break;
        switch (SHIFT[j] >> 16) {
        case 1:
            iter = s + 1;
            break;
        case 2:
            literal.push_back (octet);
            break;
        }
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
    json_token_type token;
    std::cout << token.define ().render (layout);
    return EXIT_SUCCESS;
}
