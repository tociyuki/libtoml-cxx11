#include <string>
#include <vector>
#include <map>
#include <deque>
#include <utility>
#include "json.hpp"

namespace wjson {

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

static inline int
lookup_cls (uint32_t const tbl[], std::size_t const n, uint32_t const octet)
{
    uint32_t const i = octet >> 3;
    uint32_t const count = (7 - (octet & 7)) << 2;
    return octet < n ? ((tbl[i] >> count) & 0x0f) : 0;
}

static inline uint32_t
ord (char const c)
{
    return static_cast<uint8_t> (c);
}

static inline uint32_t
hex (uint32_t const octet)
{
    return '0' <= octet && octet <= '9' ? octet - '0'
          : 'a' <= octet && octet <= 'f' ? octet - 'a' + 10
          : 'A' <= octet && octet <= 'F' ? octet - 'A' + 10
          : 0;
}

json_decoder_type::json_decoder_type (std::string const& str)
    : string (str), iter (str.cbegin ())
{
}

bool
json_decoder_type::decode (value_type& root)
{
    enum { NCHECK = 83, ACCEPT = 255 };
    static const int BASE[23] = {
         0,  0, -5,  4, 10, 19, 13,  1, 28, 21, 31, 34, -1, 40, 49, 49, 21,
        60, 39, 21, 60, 71, 62 
    };
    static const int CHECK[NCHECK] = {
             0, 0x0301, 0x0401, 0x0601, 0xff02, 0x0501, 0x110c, 0x0d07,
        0xfd03, 0x0e07, 0xfd03, 0x0201, 0xfd03, 0xfd03, 0xfc04, 0x0c06,
        0xfc04, 0x0b06, 0xfc04, 0xfc04, 0x0305, 0x0405, 0x0605, 0x1310,
        0x0505, 0x0805, 0x0a06, 0xf609, 0x1513, 0xf609, 0x0905, 0x0705,
        0xf908,      0, 0xf908, 0x0f0a, 0xf908, 0xf908, 0xf80b, 0x100a,
        0xf80b,      0, 0xf80b, 0xf80b, 0xfb0d, 0xf712, 0xfb0d, 0xf712,
        0xfb0d, 0xfb0d, 0x030e, 0x040e, 0x060e, 0xfa0f, 0x050e, 0xfa0f,
             0, 0xfa0f, 0xfa0f,      0, 0x120e, 0x0311, 0x0411, 0x0611,
        0xf414, 0x0511, 0xf516,      0, 0xf414,      0, 0xf516, 0x1411,
        0x0315, 0x0415, 0x0615,      0, 0x0515,      0,      0,      0,
             0,      0, 0x1615 
    };
    static const int GOTO[11] = {
        10, 11, 11, 11, 11, 11, 11, 12, 12, 13, 13 
    };
    static const int NRHS[11] = {
        1, 1, 1, 3, 3, 2, 2, 3, 1, 5, 3 
    };
    iter = string.cbegin ();
    std::deque<int> sstack {1};
    std::deque<value_type> dstack;
    dstack.emplace_back (); // centinel
    value_type token_value;
    int token_type = next_token (token_value);
    for (;;) {
        int prev_state = sstack.back ();        
        int j = BASE[prev_state] + token_type;
        int ctrl = 0;
        if (0 < j && j < NCHECK && (CHECK[j] & 0xff) == prev_state) {
            ctrl = CHECK[j] >> 8;
        }
        if (! ctrl)
            break;
        else if (ctrl < 128) {  // shift
            sstack.push_back (ctrl);
            dstack.push_back (token_value);
            token_type = next_token (token_value);
        }
        else if (ctrl == ACCEPT) {
            std::swap (root, dstack.back ());
            return true;
        }
        else {    // reduce
            int prod = 256 - ctrl - 2;
            int nrhs = NRHS[prod];
            std::deque<value_type>::iterator v = dstack.end () - nrhs - 1;
            value_type value;
            switch (prod) {
            case  0: // start: value
            case  1: // value: SCALAR
            case  2: // value: STRING
                std::swap (value, v[1]);
                break;
            case  3: // value: "[" array "]"
            case  4: // value: "{" table "}"
                std::swap (value, v[2]);
                break;
            case  5: // value: "[" "]"
                value = ::wjson::array ();
                break;
            case  6: // value: "{" "}"
                value = ::wjson::table ();
                break;
            case  7: // array: array "," value
                value = std::move (v[1].push_back (std::move (v[3])));
                break;
            case  8: // array: value
                value = ::wjson::array ().push_back (std::move (v[1]));
                break;
            case  9: // table: table "," STRING ":" value
                value = std::move (v[1].set (v[3], std::move (v[5])));
                break;
            case 10: // table: STRING ":" value
                value = ::wjson::table ().set (v[1], std::move (v[3]));
                break;
            }
            for (int i = 0; i < nrhs; ++i)
                sstack.pop_back ();
            for (int i = 0; i < nrhs; ++i)
                dstack.pop_back ();
            int gprev_state = sstack.back ();
            int g = BASE[gprev_state] + GOTO[prod];
            int gnext_state = 0;
            if (0 < g && g < NCHECK && (CHECK[g] & 0xff) == gprev_state)
                gnext_state = CHECK[g] >> 8;
            if (! gnext_state)
                std::logic_error ("json_decoder::decode: grammar table error");
            sstack.push_back (gnext_state);
            dstack.push_back (value);
        }
    }
    return false;
}

int
json_decoder_type::next_token (value_type& value)
{
    enum { NSHIFT = 21, SNUMBER = 10 };
    static const uint32_t CCLASS[16] = {
        0x00000000, 0x0bb00b00, 0x00000000, 0x00000000,
        0xb0200000, 0x00008a00, 0xaaaaaaaa, 0xaa700000,
        0x00000000, 0x00000000, 0x00000000, 0x00050600,
        0x01111111, 0x11111111, 0x11111111, 0x11130400,
    };
    static const int BASE[11] = {
        0, 0, 8, 0, 1, 2, 3, 4, 5, 6, 7 
    };
    static const int SHIFT[NSHIFT] = {
              0, 0x20201, 0x00301, 0x00901, 0x00a01, 0x00701, 0x00801,
        0x00601, 0x00401, 0x20202, 0x00501, 0x10101, 0x00203, 0x00804,
        0x00a05, 0x00706, 0x00507, 0x00608, 0x00309, 0x0040a, 0x00102
    };
    static const uint32_t MATCH = 12U;
    int kind = TOKEN_INVALID;
    std::wstring literal;
    std::string::const_iterator s = iter;
    std::string::const_iterator const e = string.cend ();
    for (int next_state = 1; s <= e; ++s) {
        uint32_t octet = s == e ? '\0' : ord (*s);
        int const cls = s == e ? 0 : lookup_cls (CCLASS, 128U, octet);
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

int
json_decoder_type::scan_string (value_type& value)
{
    enum { NSHIFT = 37 };
    static const uint32_t U16SPHFROM = 0xd800L;
    static const uint32_t U16SPHLAST = 0xdbffL;
    static const uint32_t U16SPLFROM = 0xdc00L;
    static const uint32_t U16SPLLAST = 0xdfffL;
    static const uint32_t U16SPOFFSET = 0x35fdc00L;
    static const uint32_t LOWERBOUNDS[5] = {0, 0x10000L, 0x0800L, 0x80L};
    static const uint32_t UPPERBOUND = 0x10ffffL;
    static const uint32_t CCLASS[32] = {
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x55955555, 0x55555555, 0x66666666, 0x66555555,
        0x56666665, 0x55555555, 0x55555555, 0x55558555,
        0x56666665, 0x55555555, 0x55555755, 0x55555550,
        0x44444444, 0x44444444, 0x44444444, 0x44444444,
        0x44444444, 0x44444444, 0x44444444, 0x44444444,
        0x33333333, 0x33333333, 0x33333333, 0x33333333,
        0x22222222, 0x22222222, 0x11111111, 0x00000000,
    };
    static const int BASE[19] = {
         0, -8,  1,  1,  7,  8, 12, 17, 10, 21, 22, 23, 22, 24, 26, 27, 28,
        29, 26 
    };
    static const int SHIFT[NSHIFT] = {
              0, 0x00201, 0x10302, 0x10402, 0x10502, 0x10403, 0x30602,
        0x30602, 0x30602, 0x00702, 0x01202, 0x10504, 0x20605, 0x10306,
        0x10406, 0x10506, 0x50908, 0x30606, 0x30606, 0x30606, 0x00706,
        0x01206, 0x40607, 0x40607, 0x00807, 0x40607, 0x40607, 0x50a09,
        0x50b0a, 0x6060b, 0x00d0c, 0x00e0d, 0x50f0e, 0x5100f, 0x51110,
        0x70611, 0x00212 
    };
    static const uint32_t MATCH = 10U;
    int kind = TOKEN_INVALID;
    std::wstring literal;
    uint32_t uc = 0;
    uint32_t u16hi = 0;
    int mbyte = 1;
    std::string::const_iterator s = iter;
    std::string::const_iterator const e = string.cend ();
    for (int next_state = 1; s <= e; ++s) {
        uint32_t octet = s == e ? '\0' : ord (*s);
        int const cls = s == e ? 0 : lookup_cls (CCLASS, 256U, octet);
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

int
json_decoder_type::scan_number (value_type& value)
{
    enum { NSHIFT = 35 };
    static const uint32_t CCLASS[16] = {
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00050630, 0x12222222, 0x22000000,
        0x00000400, 0x00000000, 0x00000000, 0x00000000,
        0x00000400, 0x00000000, 0x00000000, 0x00000000,
    };
    static const int BASE[10] = {
         0,  0,  2,  4, 11,  8, 15, 19, 25, 27 
    };
    static const int SHIFT[NSHIFT] = {
            0, 0x301, 0x401, 0x302, 0x402,     0, 0x201, 0x503, 0x703, 0x605,
        0x605, 0x103, 0x404, 0x404, 0x504, 0x704, 0x606, 0x606, 0x104, 0x706,
        0x907, 0x907, 0x206,     0, 0x807, 0x807, 0x908, 0x908, 0x909, 0x909,
            0,     0,     0,     0, 0x209 
    };
    static const uint32_t MATCH = 7U;
    int kind = TOKEN_INVALID;
    std::wstring literal;
    std::string::const_iterator s = iter;
    std::string::const_iterator const e = string.cend ();
    for (int next_state = 1; s <= e; ++s) {
        uint32_t octet = s == e ? '\0' : ord (*s);
        int const cls = s == e ? 0 : lookup_cls (CCLASS, 128U, octet);
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

}//namespace wjson
