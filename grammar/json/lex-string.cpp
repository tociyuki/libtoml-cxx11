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

struct toml_string_type : public layoutable, public check_builder_type {
    // character class
    enum { UF0 = 1, UE0, UC0, U80, U00, HEX, ELU, QESC, QQ, MATCH };
    // action
    enum { PUTUNDO, PUTMB, PUTWC, PUTSB, PUTESC, PUTHEX, PUTUC, PUTU16PAIR };

    std::vector<int> base;
    std::vector<int> check;
    bool defined;

    toml_string_type () : check_builder_type (), base (), check (), defined (false) {}

    void
    define_charset ()
    {
        charset (0, 255, 0);
        charset (0xf0, 0xf7,  UF0);
        charset (0xe0, 0xef,  UE0);
        charset (0xc0, 0xdf,  UC0);
        charset (0x80, 0xbf,  U80);
        charset (0x20, 0x7e,  U00);
        charset ("0123456789abcdefABCDEF", HEX);
        charset ('u',         ELU);
        charset ('\\',        QESC);
        charset ('"',         QQ);
    }

    void
    define_state ()
    {
        static const std::vector<int> ASCII { U00, HEX, ELU, QESC, QQ };

        to ( 1, QQ,     2);

        to ( 2, UF0,    3,  PUTMB);
        to ( 2, UE0,    4,  PUTMB);
        to ( 2, UC0,    5,  PUTMB);
        for (auto c : ASCII) to ( 2, c, 6,  PUTSB);
        to ( 2, QESC,   7);
        to ( 2, QQ,    18);

        to ( 3, U80,    4,  PUTMB);
        to ( 4, U80,    5,  PUTMB);
        to ( 5, U80,    6,  PUTWC);

        to ( 6, UF0,    3,  PUTMB);
        to ( 6, UE0,    4,  PUTMB);
        to ( 6, UC0,    5,  PUTMB); 
        for (auto c : ASCII) to ( 6, c, 6,  PUTSB);
        to ( 6, QESC,   7);
        to ( 6, QQ,    18);

        for (auto c : ASCII) to ( 7, c, 6,  PUTESC);
        to ( 7, ELU,    8);

        to ( 8, HEX,    9,  PUTHEX);
        to ( 9, HEX,   10,  PUTHEX);
        to (10, HEX,   11,  PUTHEX);
        to (11, HEX,    6,  PUTUC);

        // UTF-16 surrogate pair lo code
        to (12, QESC,  13);
        to (13, ELU,   14);
        to (14, HEX,   15,  PUTHEX);
        to (15, HEX,   16,  PUTHEX);
        to (16, HEX,   17,  PUTHEX);
        to (17, HEX,    6,  PUTU16PAIR);

        to (18, MATCH, TOKEN_STRING);
    }

    toml_string_type&
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
json_decoder_type::scan_string (value_type& value)
{
    enum { NSHIFT = @<ncheck@> };
    static const uint32_t U16SPHFROM = 0xd800L;
    static const uint32_t U16SPHLAST = 0xdbffL;
    static const uint32_t U16SPLFROM = 0xdc00L;
    static const uint32_t U16SPLLAST = 0xdfffL;
    static const uint32_t U16SPOFFSET = 0x35fdc00L;
    static const uint32_t LOWERBOUNDS[5] = {0, 0x10000L, 0x0800L, 0x80L};
    static const uint32_t UPPERBOUND = 0x10ffffL;
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
    uint32_t uc = 0;
    uint32_t u16hi = 0;
    int mbyte = 1;
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
        if (0 < m && m < NSHIFT && (SHIFT[m] & 0xff) == prev_state) {
            kind = (SHIFT[m] >> 8) & 0xff;
            value = ::wjson::string (literal);
            iter = s;
        }
        if (! next_state)
            break;
        switch (SHIFT[j] >> 16) {
        case 0:
            break;
        case 1:
            switch (cls) {
            case 1: uc = 0x07 & octet; mbyte = cls; break;
            case 2: uc = 0x0f & octet; mbyte = cls; break;
            case 3: uc = 0x1f & octet; mbyte = cls; break;
            case 4: uc = (uc << 6) | (0x3f & octet); break;
            }
            break;
        case 2:
            uc = (uc << 6) | (0x3f & octet);
            if (uc < LOWERBOUNDS[mbyte] || UPPERBOUND < uc)
                return false;
            if (U16SPHFROM <= uc && uc <= U16SPLLAST)
                return false;
            literal.push_back (uc);
            uc = 0;
            break;
        case 3:
            literal.push_back (octet);
            break;
        case 4:
            switch (octet) {
            case 'b':  literal.push_back ('\b'); break;
            case 't':  literal.push_back ('\t'); break;
            case 'n':  literal.push_back ('\n'); break;
            case 'f':  literal.push_back ('\f'); break;
            case 'r':  literal.push_back ('\r'); break;
            case '\\': literal.push_back ('\\'); break;
            case '/':  literal.push_back ('/'); break;
            case '"':  literal.push_back ('\"'); break;
            default: return false;
            }
            break;
        case 5:
            uc = (uc << 4) + hex (octet);
            break;
        case 6:
            uc = (uc << 4) + hex (octet);
            if ((U16SPLFROM <= uc && uc <= U16SPLLAST) || UPPERBOUND < uc)
                return false;
            if (U16SPHFROM <= uc && uc <= U16SPHLAST) {
                u16hi = uc;
                next_state = 12;
            }
            else {
                literal.push_back (uc);
            }
            uc = 0;
            break;
        case 7:
            uc = (uc << 4) + hex (octet);
            if (uc < U16SPLFROM || U16SPLLAST < uc)
                return false;
            literal.push_back ((u16hi << 10) + uc - U16SPOFFSET);
            u16hi = 0;
            uc = 0;
            break;
        default:
            throw std::logic_error ("unexpected lex string action number");
        }
    }
    return kind;
}
)EOS"
);

int
main ()
{
    toml_string_type string;
    std::cout << string.define ().render (layout);
    return EXIT_SUCCESS;
}
