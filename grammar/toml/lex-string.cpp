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

struct toml_string_type : public layoutable, public check_builder_type {
    // character class
    enum {
        UF0 = 1, UE0, UC0, U80, U00,
        HEX, EUU, ELU, WS, QESC, QS, QQ, LF, MATCH
    };
    // action
    enum {PUTUNDO, PUTMB, PUTWC, PUTSB, UNPUTQQ, PUTESC, PUTHEX, PUTUC};

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
        charset (0x21, 0x7e,  U00);
        charset ("0123456789abcdefABCDEF", HEX);
        charset ('U',         EUU);
        charset ('u',         ELU);
        charset ('\t',        WS);
        charset (' ',         WS);
        charset ('\r',        LF);
        charset ('\n',        LF);
        charset ('\\',        QESC);
        charset ('\'',        QS);
        charset ('"',         QQ);
    }

    void
    define_state ()
    {
        static const std::vector<int> ASCII {
            U00, HEX, EUU, ELU, WS, QESC, QS,QQ };
        static const std::vector<int> ASCIILN {
            U00, HEX, EUU, ELU, WS, QESC, QS,QQ, LF };

        to ( 1, QQ,     2);

        to ( 2, UF0,    9,  PUTMB);
        to ( 2, UE0,   10,  PUTMB);
        to ( 2, UC0,   11,  PUTMB);
        for (auto c : ASCII) to ( 2, c, 12,  PUTSB);
        to ( 2, QESC,  13);
        to ( 2, QQ,     3);

        to ( 3, QQ,     4);
        to ( 3, MATCH,  TOKEN_STRING);

        to ( 4, UF0,   22,  PUTMB);
        to ( 4, UE0,   23,  PUTMB);
        to ( 4, UC0,   24,  PUTMB);
        for (auto c : ASCII) to ( 4, c, 25,  PUTSB);
        to ( 4, LF,    25);
        to ( 4, QESC,  26);
        to ( 4, QQ,     5,  PUTSB);

        to ( 5, UF0,   22,  PUTMB);
        to ( 5, UE0,   23,  PUTMB);
        to ( 5, UC0,   24,  PUTMB);
        for (auto c : ASCIILN) to ( 5, c, 25,  PUTSB);
        to ( 5, QESC,  26);
        to ( 5, QQ,     6,  PUTSB);

        to ( 6, UF0,   22,  PUTMB);
        to ( 6, UE0,   23,  PUTMB);
        to ( 6, UC0,   24,  PUTMB);
        for (auto c : ASCIILN) to ( 6, c, 25,  PUTSB);
        to ( 6, QESC,  26);
        to ( 6, QQ,     7,  UNPUTQQ);

        to ( 7, MATCH,  TOKEN_STRING);
        to ( 8, MATCH,  TOKEN_STRKEY);

        // ["] ... ["]
        to ( 9, U80,  10,  PUTMB);
        to (10, U80,  11,  PUTMB);
        to (11, U80,  12,  PUTWC);

        to (12, UF0,   9,  PUTMB);
        to (12, UE0,  10,  PUTMB);
        to (12, UC0,  11,  PUTMB); 
        for (auto c : ASCII) to (12, c, 12,  PUTSB);
        to (12, QESC, 13);
        to (12, QQ,    8);

        for (auto c : ASCII) to (13, c, 12,  PUTESC);
        to (13, EUU,  14);
        to (13, ELU,  18);

        to (14, HEX,  15,  PUTHEX);
        to (15, HEX,  16,  PUTHEX);
        to (16, HEX,  17,  PUTHEX);
        to (17, HEX,  18,  PUTHEX);
        to (18, HEX,  19,  PUTHEX);
        to (19, HEX,  20,  PUTHEX);
        to (20, HEX,  21,  PUTHEX);
        to (21, HEX,  12,  PUTUC);

        // ["]["]["] ... ["]["]["]
        to (22, U80,  23,  PUTMB);
        to (23, U80,  24,  PUTMB);
        to (24, U80,  25,  PUTWC);

        to (25, UF0,  22,  PUTMB);
        to (25, UE0,  23,  PUTMB);
        to (25, UC0,  24,  PUTMB);
        for (auto c : ASCIILN) to (25, c, 25,  PUTSB);
        to (25, QESC, 26);
        to (25, QQ,    5,  PUTSB);

        for (auto c : ASCII) to (26, c, 25,  PUTESC);
        to (26, EUU,  27);
        to (26, ELU,  31);
        to (26, LF,   35);

        to (27, HEX,  28,  PUTHEX);
        to (28, HEX,  29,  PUTHEX);
        to (29, HEX,  30,  PUTHEX);
        to (30, HEX,  31,  PUTHEX);
        to (31, HEX,  32,  PUTHEX);
        to (32, HEX,  33,  PUTHEX);
        to (33, HEX,  34,  PUTHEX);
        to (34, HEX,  25,  PUTUC);

        to (35, UF0,  22,  PUTMB);
        to (35, UE0,  23,  PUTMB);
        to (35, UC0,  24,  PUTMB);
        for (auto c : ASCII) to (35, c, 25,  PUTSB);
        to (35, QESC, 26);
        to (35, WS,   35);
        to (35, LF,   35);
        to (35, QQ,    5,  PUTSB);

        // ['] ... ['] | ['][']['] ... [']['][']
        to ( 1, QS,   36);

        to (36, UF0,  41,  PUTMB);
        to (36, UE0,  42,  PUTMB);
        to (36, UC0,  43,  PUTMB);
        for (auto c : ASCII)   to (36, c, 44,  PUTSB);
        to (36, QS,   37);

        to (37, QS,   38);
        to (37, MATCH, TOKEN_STRING);

        to (38, UF0,  45,  PUTMB);
        to (38, UE0,  46,  PUTMB);
        to (38, UC0,  47,  PUTMB);
        for (auto c : ASCII)   to (38, c, 48,  PUTSB);
        to (38, LF,   48);
        to (38, QS,   39,  PUTSB);

        to (39, UF0,  45,  PUTMB);
        to (39, UE0,  46,  PUTMB);
        to (39, UC0,  47,  PUTMB);
        for (auto c : ASCIILN) to (39, c, 48,  PUTSB);
        to (39, QS,   40,  PUTSB);

        to (40, UF0,  45,  PUTMB);
        to (40, UE0,  46,  PUTMB);
        to (40, UC0,  47,  PUTMB);
        for (auto c : ASCIILN) to (40, c, 48,  PUTSB);
        to (40, QS,    7,  UNPUTQQ);

        // ['] ... [']
        to (41, U80,  42,  PUTMB);
        to (42, U80,  43,  PUTMB);
        to (43, U80,  44,  PUTWC);
        to (44, UF0,  41,  PUTMB);
        to (44, UE0,  42,  PUTMB);
        to (44, UC0,  43,  PUTMB);
        for (auto c : ASCII)   to (44, c, 44,  PUTSB);
        to (44, QS,    7);

        // ['][']['] ... [']['][']
        to (45, U80,  46,  PUTMB);
        to (46, U80,  47,  PUTMB);
        to (47, U80,  48,  PUTWC);
        to (48, UF0,  45,  PUTMB);
        to (48, UE0,  46,  PUTMB);
        to (48, UC0,  47,  PUTMB);
        for (auto c : ASCIILN) to (48, c, 48,  PUTSB);
        to (48, QS,   39,  PUTSB);
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
toml_decoder_type::scan_string (value_type& value)
{
    enum { NSHIFT = @<ncheck@> };
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
        else if (0 < m && m < NSHIFT && (SHIFT[m] & 0xff) == prev_state) {
            // else-if hack exclusive ["]["] or ["]["]["]
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
                return TOKEN_INVALID;
            if (0xd800L <= uc && uc <= 0xdfffL)
                return TOKEN_INVALID;
            literal.push_back (uc);
            uc = 0;
            break;
        case 3:
            literal.push_back (octet);
            break;
        case 4:
            literal.pop_back ();    // ["]["]["] => ["]
            literal.pop_back ();
            break;
        case 5:
            switch (octet) {
            case 'b':  literal.push_back ('\b'); break;
            case 't':  literal.push_back ('\t'); break;
            case 'n':  literal.push_back ('\n'); break;
            case 'f':  literal.push_back ('\f'); break;
            case 'r':  literal.push_back ('\r'); break;
            case '\\': literal.push_back ('\\'); break;
            case '"':  literal.push_back ('\"'); break;
            default: return TOKEN_INVALID;
            }
            break;
        case 6:
            uc = (uc << 4) + hex (octet);
            break;
        case 7:
            uc = (uc << 4) + hex (octet);
            if ((0xd800L <= uc && uc <= 0xdfffL) || UPPERBOUND < uc)
                return TOKEN_INVALID;
            literal.push_back (uc);
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
