#include <string>
#include <vector>
#include <map>
#include <deque>
#include <utility>
#include "toml.hpp"

namespace wjson {

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

toml_decoder_type::toml_decoder_type (std::string const& str)
    : kvstate (0), string (str), iter (str.cbegin ()), mark ()
{
}

bool
toml_decoder_type::decode (value_type& root)
{
    enum { NCHECK = 330, ACCEPT = 255 };
    static const int BASE[65] = {
          0,   0, -13,   4,  -3, -11,   7,  -2,  19,  23,  27,  -3,  37,  39,
         33,  49,  41,  57,  71,  63,  75,   0,  77,  71,  75,  77,  88,  95,
        102, 109, 116, 130, 141,   9,  82, 150,  89,  15,  31, 178,  57, 188,
         37,  98, 105, 202,  37, 204,  39, 150, 196, 232, 235, 250, 252,  41,
        254,  94, 282, 112, 293, 296, 202, 300, 133 
    };
    static const int CHECK[NCHECK] = {
             0, 0x0801, 0x0901, 0xff02, 0x0d05, 0x0803, 0x0903, 0x0c04,
        0x0806, 0x0906, 0x0601, 0x1107, 0x120b, 0xfb04, 0x0603, 0x2315,
        0xfa01, 0x0f06, 0x0201, 0x0401, 0xfc03, 0x0701, 0x0301, 0x0a03,
        0x2d21, 0x0703, 0x3025, 0x0e06, 0x1006, 0x0501, 0xef08, 0xef08,
        0xef08, 0x0b03, 0xee09, 0xee09, 0xee09, 0x0c0a, 0x080c, 0x090c,
        0xec0d, 0xec0d, 0x3126, 0xfd0a, 0x150e, 0x160e, 0x342a, 0x140c,
        0x372e, 0xec0d, 0x080f, 0x090f, 0xf010, 0xf010, 0x3830, 0xec0d,
        0x3c37, 0x130c, 0x100c, 0x1e11, 0x1d11, 0x1911, 0x1a11, 0x1b11,
        0x1c11, 0x2011,      0, 0x1f11, 0xe228, 0x170f, 0x100f, 0x3328,
        0xed12, 0xed12, 0x2113, 0x1613, 0x0814, 0x0914, 0x0816, 0x0916,
        0x1811, 0xed12, 0x2517, 0x1617, 0xd718,      0, 0xeb19, 0xed12,
        0xeb19, 0xd718, 0xd718, 0xeb19, 0xeb19, 0x2e22, 0x1622, 0x2214,
        0x1014, 0xea1a, 0x2416, 0xea1a, 0xf124, 0xf124, 0xea1a, 0xea1a,
        0xe91b, 0xe039, 0xe91b, 0xdb2b, 0xe039, 0xe91b, 0xe91b, 0xe81c,
        0x352b, 0xe81c, 0xd92c,      0, 0xe81c, 0xe81c, 0xe71d, 0xd92c,
        0xe71d, 0xd83b,      0, 0xe71d, 0xe71d, 0xe61e, 0xd83b, 0xe61e,
             0,      0, 0xe61e, 0xe61e, 0xde1f, 0xde1f, 0xde1f, 0xde1f,
        0xde1f, 0xde1f, 0xde1f,      0, 0xde1f, 0xde1f, 0x0820, 0x0920,
        0xdf40, 0x291f,      0, 0xdf40,      0,      0, 0xdc20, 0x0823,
        0x0923,      0, 0x261f, 0x281f, 0x271f,      0,      0, 0xe531,
        0xf423, 0xe531, 0x0720,      0, 0xe531, 0xe531, 0xf423,      0,
        0x2a20, 0x2b20, 0x2c20, 0x0723, 0x2f23,      0,      0,      0,
             0,      0,      0, 0x0523, 0x1e27, 0x1d27, 0x1927, 0x1a27,
        0x1b27, 0x1c27, 0x2027,      0, 0x1f27, 0xe327, 0xdd29, 0xdd29,
        0xdd29, 0xdd29, 0xdd29, 0xdd29, 0xdd29,      0, 0xdd29, 0xdd29,
             0, 0x3227, 0xdd29, 0x082d, 0x092d, 0x082f, 0x092f, 0xde32,
             0,      0, 0xde32, 0x2932, 0xf82d, 0xde3e, 0xf52f,      0,
        0xde3e, 0x293e, 0xf82d,      0, 0xf52f,      0, 0x3932, 0x072d,
        0x362d, 0x072f,      0,      0, 0x403e,      0,      0, 0x052d,
             0, 0x0b2f, 0xde33, 0xde33, 0xde33, 0xde33, 0xde33, 0xde33,
        0xde33,      0, 0xde33, 0xde33, 0xe434,      0, 0xe434, 0x2933,
             0, 0xe434, 0xe434, 0x0835, 0x0935, 0x0836, 0x0936, 0x0838,
        0x0938,      0, 0x3a33, 0xda35,      0,      0, 0xf936,      0,
        0xf238,      0,      0,      0, 0xf936,      0, 0xf238, 0x0735,
             0, 0x0736,      0, 0x0738, 0x3d38,      0,      0, 0x3b35,
             0, 0x0b36,      0, 0x0538, 0x1e3a, 0x1d3a, 0x193a, 0x1a3a,
        0x1b3a, 0x1c3a, 0x203a,      0, 0x1f3a, 0xe13a, 0x083c, 0x093c,
             0, 0x083d, 0x093d,      0,      0, 0x083f, 0x093f, 0xf63c,
             0, 0x3e3a, 0xf33d,      0,      0, 0xf63c, 0xf73f,      0,
        0xf33d,      0, 0x073c, 0x3f3c, 0xf73f, 0x073d,      0,      0,
             0, 0x073f, 0x053c,      0,      0, 0x0b3d,      0,      0,
             0, 0x0b3f 
    };
    static const int GOTO[40] = {
        17, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 21, 21,
        22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 25, 25, 26, 26,
        27, 27, 27, 28, 28, 29 
    };
    static const int NRHS[40] = {
        1, 2, 1, 1, 0, 6, 5, 8, 7, 5, 4, 7, 6, 3, 1, 1, 1, 3, 2, 1, 1, 1, 1,
        1, 1, 3, 3, 1, 1, 3, 3, 5, 0, 1, 0, 1, 2, 1, 3, 3 
    };
    iter = string.cbegin ();
    kvstate = 0;
    mark.clear ();
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
            default:
                std::swap (value, v[1]);
                break;
            case 1: // toml: statements sections
                value = merge_exclusive (v[1], v[2]);
                break;
            case 5: // sections: sections "[" keypath "]" ENDLINE statements
                value = merge_table (v[1], v[3], v[6]);
                break;
            case 6: // sections: sections "[" keypath "]" ENDLINE
                value = merge_table (v[1], v[3], ::wjson::table ());
                break;
            case 7: // sections: sections "[" "[" keypath "]" "]" ENDLINE statements
                value = merge_array (v[1], v[4], v[8]);
                break;
            case 8: // sections: sections "[" "[" keypath "]" "]" ENDLINE
                value = merge_array (v[1], v[4], ::wjson::table ());
                break;
            case 9: // sections: "[" keypath "]" ENDLINE statements
                value = ::wjson::table ();
                value = merge_table (value, v[2], v[5]);
                break;
            case 10: // sections: "[" keypath "]" ENDLINE
                value = ::wjson::table ();
                value = merge_table (value, v[3], ::wjson::table ());
                break;
            case 11: // sections: "[" "[" keypath "]" "]" ENDLINE statements
                value = ::wjson::table ();
                value = merge_array (value, v[3], v[7]);
                break;
            case 12: // sections: "[" "[" keypath "]" "]" ENDLINE
                value = ::wjson::table ();
                value = merge_array (value, v[3], ::wjson::table ());
                break;
            case 13: // keypath: keypath "." key
                value = std::move (v[1].push_back (std::move (v[3])));
                break;
            case 14: // keypath: key
                value = ::wjson::array ().push_back (std::move (v[1]));
                break;
            case 17: // statements: statements pair ENDLINE
                value = merge_exclusive (v[1], v[2]);
                break;
            case 25: // value: "[" array "]"
                std::swap (value, v[2]);
                break;
            case 26: // value: "{" table "}"
                kvstate = 2;
                std::swap (value, v[2]);
                break;
            case 30: // value_list: endln value endln
                value = ::wjson::array ().push_back (std::move (v[2]));
                break;
            case 31: // value_list: value_list "," endln value endln
                value = unify_back (v[1], v[4]);
                break;
            case 34: // table:
                value = ::wjson::table ();
                break;
            case 38: // pair_list: pair_list "," pair
                value = merge_exclusive (v[1], v[3]);
                break;
            case 39: // pair: key "=" value
                kvstate = 1;
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
                std::logic_error ("parser: GRAMMAR table error");
            sstack.push_back (gnext_state);
            dstack.push_back (value);
        }
    }
    return false;
}

int
toml_decoder_type::next_token (value_type& value)
{
    if (kvstate < 2)
        return scan_key (value);
    else
        return scan_value (value);
}

int
toml_decoder_type::scan_key (value_type& value)
{
    enum { NSHIFT = 32 };
    static const uint32_t CCLASS[16] = {
        0x00000000, 0x01400300, 0x00000000, 0x00000000,
        0x10520000, 0x00006d70, 0xdddddddd, 0xdd000800,
        0x0ddddddd, 0xdddddddd, 0xdddddddd, 0xddd90a0d,
        0x0ddddddd, 0xdddddddd, 0xdddddddd, 0xdddb0c00,
    };
    static const int BASE[14] = {
         0,  0, 11, 12, 16,  7,  9, 10, 11, 12, 13, 14, 15, 17 
    };
    static const int SHIFT[NSHIFT] = {
              0, 0x00101, 0x00201, 0x00301, 0x00401, 0x00501, 0x00d01,
        0x00b01, 0x00c01, 0x00901, 0x00a01, 0x00701, 0x00801, 0x10601,
        0x00302, 0x00402, 0x00403, 0x00404, 0x00204, 0x00304, 0x00404,
        0x00205, 0x10606, 0x00106, 0x00807, 0x00908, 0x00a09, 0x00b0a,
        0x00c0b, 0x00d0c, 0x00f04, 0x00e0d 
    };
    static const uint32_t MATCH = 14U;
    int kind = TOKEN_INVALID;
    std::wstring literal;
    std::string::const_iterator s = iter;
    std::string::const_iterator const e = string.cend ();
    for (int next_state = 1; s <= e; ++s) {
        uint32_t octet = s == e ? '\0' : ord (*s);
        int const cls = s == e ? 0 : lookup_cls (CCLASS, 128U, octet);
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
            case TOKEN_BAREKEY:
                value = ::wjson::string (literal);
                break;
            case TOKEN_STRKEY:
                iter = s - 1;
                return scan_string (value);
            case TOKEN_EQUAL:
                kvstate = 2;
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
    if (kind == TOKEN_ENDLINE && 0 == kvstate)
        return scan_key (value); // skip ENDLINE at TOPFILE
    if (0 == kvstate)
        kvstate = 1;
    if (kind == TOKEN_INVALID && s == e) {
        kind = TOKEN_ENDMARK;
        iter = s;
    }
    return kind;
}

int
toml_decoder_type::scan_value (value_type& value)
{
    enum { NSHIFT = 36 };
    static const uint32_t CCLASS[16] = {
        0x00000000, 0x01400300, 0x00000000, 0x00000000,
        0x10520005, 0x000d6e00, 0xeeeeeeee, 0xee000000,
        0x0fffffff, 0xffffffff, 0xffffffff, 0xfff90a0f,
        0x0fffffff, 0xffffffff, 0xffffffff, 0xfffb0c00,
    };
    static const int BASE[15] = {
         0,  0, 13, 14, 18,  7, 10, 11, 12, 13, 14, 15, 16, 17, 19 
    };
    static const int SHIFT[NSHIFT] = {
              0, 0x00101, 0x00201, 0x00301, 0x00401, 0x00501, 0x00d01,
        0x00b01, 0x00c01, 0x00901, 0x00a01, 0x00701, 0x00801, 0x00e01,
        0x00e01, 0x10601, 0x00302, 0x00402, 0x00403, 0x00404, 0x00204,
        0x00304, 0x00404, 0x00305, 0x10606, 0x10606, 0x00406, 0x00807,
        0x00908, 0x00a09, 0x00b0a, 0x00c0b, 0x00d0c, 0x00e0d, 0x00f04,
        0x0050e 
    };
    static const uint32_t MATCH = 16U;
    int kind = TOKEN_INVALID;
    std::wstring literal;
    std::string::const_iterator s = iter;
    std::string::const_iterator const e = string.cend ();
    for (int next_state = 1; s <= e; ++s) {
        uint32_t octet = s == e ? '\0' : ord (*s);
        int const cls = s == e ? 0 : lookup_cls (CCLASS, 128U, octet);
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

int
toml_decoder_type::scan_string (value_type& value)
{
    enum { NSHIFT = 209 };
    static const uint32_t LOWERBOUNDS[5] = {0, 0x10000L, 0x0800L, 0x80L};
    static const uint32_t UPPERBOUND = 0x10ffffL;
    static const uint32_t CCLASS[32] = {
        0x00000000, 0x09d00d00, 0x00000000, 0x00000000,
        0x95c5555b, 0x55555555, 0x66666666, 0x66555555,
        0x56666665, 0x55555555, 0x55555755, 0x5555a555,
        0x56666665, 0x55555555, 0x55555855, 0x55555550,
        0x44444444, 0x44444444, 0x44444444, 0x44444444,
        0x44444444, 0x44444444, 0x44444444, 0x44444444,
        0x33333333, 0x33333333, 0x33333333, 0x33333333,
        0x22222222, 0x22222222, 0x11111111, 0x00000000,
    };
    static const int BASE[49] = {
          0, -10,   2,   3,  17,  30,  43,  -8,   2,  17,  30,  43,  56,  64,
         54,  71,  72,  73,  74,  75,  76,  77,  80,  81,  82,  86,  95,  84,
        103, 104, 105, 106, 107, 108, 109, 115, 128, 130, 144, 157, 170, 115,
        128, 138, 183, 139, 144, 157, 195 
    };
    static const int SHIFT[NSHIFT] = {
              0, 0x02401, 0x00201, 0x10902, 0x10a02, 0x10b02, 0x00307,
        0x30c02, 0x30c02, 0x30c02, 0x30c02, 0x30c02, 0x00d02, 0x30c02,
        0x00302, 0x00403, 0x00208, 0x00303, 0x11604, 0x11704, 0x11804,
        0x10a09, 0x31904, 0x31904, 0x31904, 0x31904, 0x31904, 0x01a04,
        0x31904, 0x30504, 0x01904, 0x11605, 0x11705, 0x11805, 0x10b0a,
        0x31905, 0x31905, 0x31905, 0x31905, 0x31905, 0x01a05, 0x31905,
        0x30605, 0x31905, 0x11606, 0x11706, 0x11806, 0x20c0b, 0x31906,
        0x31906, 0x31906, 0x31906, 0x31906, 0x01a06, 0x31906, 0x40706,
        0x31906, 0x1090c, 0x10a0c, 0x10b0c, 0x60f0e, 0x30c0c, 0x30c0c,
        0x30c0c, 0x30c0c, 0x30c0c, 0x00d0c, 0x30c0c, 0x0080c, 0x50c0d,
        0x50c0d, 0x00e0d, 0x0120d, 0x50c0d, 0x50c0d, 0x50c0d, 0x50c0d,
        0x6100f, 0x61110, 0x61211, 0x61312, 0x61413, 0x61514, 0x70c15,
        0x11716, 0x11817, 0x21918, 0x11619, 0x11719, 0x11819, 0x61c1b,
        0x31919, 0x31919, 0x31919, 0x31919, 0x31919, 0x01a19, 0x31919,
        0x30519, 0x31919, 0x5191a, 0x5191a, 0x01b1a, 0x01f1a, 0x5191a,
        0x5191a, 0x5191a, 0x5191a, 0x0231a, 0x61d1c, 0x61e1d, 0x61f1e,
        0x6201f, 0x62120, 0x62221, 0x71922, 0x11623, 0x11723, 0x11823,
        0x12a29, 0x31923, 0x31923, 0x31923, 0x31923, 0x02323, 0x01a23,
        0x31923, 0x30523, 0x02323, 0x12924, 0x12a24, 0x12b24, 0x12b2a,
        0x32c24, 0x32c24, 0x32c24, 0x32c24, 0x32c24, 0x32c24, 0x02524,
        0x32c24, 0x02625, 0x22c2b, 0x12e2d, 0x00325, 0x12d26, 0x12e26,
        0x12f26, 0x12f2e, 0x33026, 0x33026, 0x33026, 0x33026, 0x33026,
        0x33026, 0x32726, 0x33026, 0x03026, 0x12d27, 0x12e27, 0x12f27,
        0x2302f, 0x33027, 0x33027, 0x33027, 0x33027, 0x33027, 0x33027,
        0x32827, 0x33027, 0x33027, 0x12d28, 0x12e28, 0x12f28,       0,
        0x33028, 0x33028, 0x33028, 0x33028, 0x33028, 0x33028, 0x40728,
        0x33028, 0x33028, 0x1292c, 0x12a2c, 0x12b2c,       0, 0x32c2c,
        0x32c2c, 0x32c2c, 0x32c2c, 0x32c2c, 0x32c2c, 0x0072c, 0x32c2c,
        0x12d30, 0x12e30, 0x12f30,       0, 0x33030, 0x33030, 0x33030,
        0x33030, 0x33030, 0x33030, 0x32730, 0x33030, 0x33030 
    };
    static const uint32_t MATCH = 14U;
    int kind = TOKEN_INVALID;
    std::wstring literal;
    uint32_t uc = 0;
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

int
toml_decoder_type::scan_number (value_type& value)
{
    enum { NSHIFT = 90 };
    static const uint32_t CCLASS[16] = {
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00040350, 0x11111111, 0x11700000,
        0x00000600, 0x00000000, 0x00008000, 0x00900002,
        0x00000600, 0x00000000, 0x00000000, 0x00000000,
    };
    static const int BASE[36] = {
         0,  0,  4,  6, 16, 22,  1, 12, 12, 18, 19, 26, 30, 32, 30, 37, 38,
        37, 41, 42, 45, 44, 55, 50, 51, 46, 56, 59, 51, 61, 65, 62, 67, 75,
        71, 79 
    };
    static const int SHIFT[NSHIFT] = {
             0, 0x0201, 0x0706, 0x1d01, 0x1d01, 0x0302, 0x1d02, 0x0403,
        0x1d03, 0x1f02, 0x2102, 0x1f03, 0x2103, 0x0807, 0x0502, 0x0908,
        0x0503, 0x0504, 0x1d04, 0x0a09, 0x0b0a, 0x1f04, 0x2104, 0x0505,
        0x1d05, 0x0605, 0x0504, 0x1f05, 0x2105, 0x170b, 0x170b, 0x0d0c,
        0x0505, 0x0e0d, 0x0c0b, 0x1c0b, 0x070b, 0x0f0e, 0x100f, 0x1110,
        0x1711, 0x1711, 0x1312, 0x1413, 0x1211, 0x1615, 0x1c11, 0x0711,
        0x1714, 0x1714, 0x1514, 0x1817, 0x1918, 0x1a19, 0x1c14, 0x0714,
        0x1616, 0x1b1a, 0x1716, 0x1716, 0x1c1b, 0x071c, 0x1e1d, 0x201f,
        0x1c16, 0x0716, 0x1e1e, 0x1d1e, 0x2020, 0x1f20, 0x1f1e, 0x211e,
        0x2322, 0x2120,      0, 0x051e, 0x2321, 0x0620, 0x2221, 0x2221,
        0x2323, 0x2223,      0,      0,      0,      0,      0,      0,
             0, 0x0623 
    };
    static const uint32_t MATCH = 10U;
    int kind = TOKEN_INVALID;
    std::wstring literal;
    std::string::const_iterator s = iter;
    std::string::const_iterator const e = string.cend ();
    for (int next_state = 1; s <= e; ++s) {
        uint32_t octet = s == e ? '\0' : ord (*s);
        int cls = s == e ? 0 : lookup_cls (CCLASS, 128U, octet);
        int const prev_state = next_state;
        next_state = 0;
        int j = BASE[prev_state] + cls;
        int m = BASE[prev_state] + MATCH;
        if (0 < j && j < NSHIFT && (SHIFT[j] & 0xff) == prev_state)
            next_state = (SHIFT[j] >> 8) & 0xff;
        if (next_state && s < e && '_' != octet)
            literal.push_back (octet);
        if (0 < m && m < NSHIFT && (SHIFT[m] & 0xff) == prev_state) {
            kind = (SHIFT[m] >> 8) & 0xff;
            try {
                if (TOKEN_FIXNUM == kind)
                    value = ::wjson::fixnum (std::stoll (literal));
                else if (TOKEN_FLONUM == kind)
                    value = ::wjson::flonum (std::stod (literal));
                else
                    value = ::wjson::datetime (literal);
            }
            catch (std::out_of_range) {
                value = ::wjson::null ();
                return TOKEN_INVALID;
            }
            iter = s;
        }
        if (! next_state)
            break;
    }
    return kind;
}

value_type&
toml_decoder_type::merge_exclusive (value_type& x, value_type const& y)
{
    for (auto& i : y.table ()) {
        if (x.table ().count (i.first) > 0)
            throw std::out_of_range ("merge_exclusive: conflict table");
        x.set (i.first, i.second);
    }
    return x;
}

value_type&
toml_decoder_type::merge_table (value_type& x, value_type const& path, value_type const& y)
{
    if (path.size () > 0) {
        std::wstring pathstr = path_to_string (path);
        if (mark.count (pathstr) > 0)
            throw std::out_of_range ("merge_table: path once");
        mark[pathstr] = 1;
    }
    value_type* node = &x;
    for (std::size_t i = 0;;) {
        if (node->tag () == VALUE_ARRAY)
            node = &(node->array ().back ());
        else if (node->tag () != VALUE_TABLE)
            throw std::out_of_range ("merge_table: conflict table");
        else if (i < path.size ()) {
            std::wstring key = path.get (i).string ();
            if (node->table ().count (key) == 0)
                node->set (key, ::wjson::table ());
            node = &(node->get (key));
            ++i;
        }
        else {
            merge_exclusive (*node, y);
            break;
        }
    }
    return x;
}

value_type&
toml_decoder_type::merge_array (value_type& x, value_type const& path, value_type const& y)
{
    if (path.size () > 0) {
        std::wstring pathstr = path_to_string (path);
        if (mark.count (pathstr) > 0 && mark.at (pathstr) == 1)
            throw std::out_of_range ("merge_array: path once");
        mark[pathstr] = 2;
    }
    value_type* node = &x;
    for (std::size_t i = 0;;) {
        if (node->tag () == VALUE_ARRAY)
            node = &(node->array ().back ());
        else if (i + 1 < path.size ()) {
            if (node->tag () != VALUE_TABLE)
                throw std::out_of_range ("merge_array: conflict table");
            std::wstring key = path.get (i).string ();
            if (node->table ().count (key) == 0)
                node->set (key, ::wjson::table ());
            node = &(node->get (key));
            ++i;
        }
        else {
            std::wstring key = path.get (i).string ();
            if (node->table ().count (key) == 0)
                node->set (key, ::wjson::array ());
            node = &(node->get (key));
            if (node->tag () != VALUE_ARRAY)
                throw std::out_of_range ("merge_array: conflict table");
            unify_back (*node, y);
            break;
        }
    }
    return x;
}

value_type&
toml_decoder_type::unify_back (value_type& x, value_type const& y)
{
    std::size_t n = x.size ();
    if (n > 0 && x.get (0).tag () != y.tag ())
        throw std::out_of_range ("unify_back: different type");
    x.array ().push_back (y);
    return x;
}

std::wstring
toml_decoder_type::path_to_string (value_type const& path)
{
    std::wstring t;
    for (auto& k : path.array ()) {
        t.push_back (L'.');
        for (wchar_t c : k.string ())
            switch (c) {
            case L'.': t += L"\\."; break;
            case L'\\': t += L"\\\\"; break;
            default: t.push_back (c); break;
            }
    }
    return std::move (t);
}

}//namespace wjson
