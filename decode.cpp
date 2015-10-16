#include <stdexcept>
#include <cctype>
#include <string>
#include <vector>
#include <map>
#include <deque>
#include <utility>
#include "toml.hpp"

namespace toml {

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
    TOKEN_ENDMARK
};

struct parsed_type {
    int kind;
    int64_t literal_fixnum;
    double literal_flonum;
    std::string literal;
    std::string::const_iterator s;
};

class decoder_type {
public:
    decoder_type (std::string const& x)
        : string (x), iter (x.cbegin ()), kvstate (0), mark () {}

    bool parse (doc_type& doc);

private:
    std::string const& string;
    std::string::const_iterator iter;
    int kvstate;  // 0: INIT, 1: KEY, 2: VALUE
    std::map<std::string,int> mark;

    int next_token (doc_type& doc, value_id& value);
    bool scan_space (std::string::const_iterator s, parsed_type& parsed);
    bool scan_barekey (std::string::const_iterator s, parsed_type& parsed);
    bool scan_number (std::string::const_iterator s, parsed_type& parsed);
    bool scan_string (std::string::const_iterator s, parsed_type& parsed);
    void encode_codepoint (uint32_t const uc, std::string& literal);

    value_id merge_exclusive (doc_type& doc,value_id const a, value_id const b);
    std::string path_to_string (doc_type& doc,value_id path);
    value_id merge_table (doc_type& doc,
        value_id const root, value_id const path, value_id const body);
    value_id merge_array (doc_type& doc,
        value_id const root, value_id const path, value_id const body);
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

int
decoder_type::next_token (doc_type& doc, value_id& value)
{
    value = VALUE_NULL;
    std::string::const_iterator const e = string.cend ();
    parsed_type space;
    if (! scan_space (iter, space))
        return TOKEN_INVALID;
    iter = space.s;
    if (kvstate > 0 && TOKEN_ENDLINE == space.kind)
        return TOKEN_ENDLINE;
    if (iter >= e)
        return TOKEN_ENDMARK;
    if (0 == kvstate)
        kvstate = 1;
    parsed_type parsed;
    uint32_t octet = ord (*iter++);
    switch (octet) {
    case '[': return TOKEN_LBRACKET;
    case ']': return TOKEN_RBRACKET;
    case '{': kvstate = 1; return TOKEN_LBRACE;
    case '}': return TOKEN_RBRACE;
    case '.': return TOKEN_DOT;
    case ',': return TOKEN_COMMA;
    case '=': kvstate = 2; return TOKEN_EQUAL;
    case '\'':
    case '"':
        --iter;
        if (! scan_string (iter, parsed))
            return TOKEN_INVALID;
        iter = parsed.s;
        value = doc.string (parsed.literal);
        return parsed.kind;
    }
    --iter;
    if (1 == kvstate && scan_barekey (iter, parsed)) {
        iter = parsed.s;
        value = doc.string (parsed.literal);
        return parsed.kind;
    }
    else if (2 == kvstate && scan_number (iter, parsed)) {
        if (parsed.kind == TOKEN_FIXNUM)
            value = doc.fixnum (parsed.literal_fixnum);
        else if (parsed.kind == TOKEN_FLONUM)
            value = doc.flonum (parsed.literal_flonum);
        else if (parsed.kind == TOKEN_DATETIME)
            value = doc.datetime (parsed.literal);
        else
            return TOKEN_INVALID;
        iter = parsed.s;
        return parsed.kind;
    }
    else if (2 == kvstate && scan_barekey (iter, parsed)) {
        if (parsed.literal == "false")
            value = doc.boolean (false);
        else if (parsed.literal == "true")
            value = doc.boolean (true);
        else
            return TOKEN_INVALID;
        iter = parsed.s;
        return TOKEN_BOOLEAN;
    }
    return TOKEN_INVALID;
}

bool
decoder_type::scan_space (std::string::const_iterator s, parsed_type& parsed)
{
    std::string::const_iterator const e = string.cend ();
    parsed.kind = TOKEN_STRING;
    for (int next_state = 1; next_state && s < e; ++s) {
        uint32_t octet = ord (*s);
        if ('\n' == octet)
            parsed.kind = TOKEN_ENDLINE;
        if (1 == next_state) {
            next_state = ' ' == octet || '\t' == octet || '\n' == octet ? 1
                       : '#' == octet ? 2 : '\r' == octet ? 3 : 0;
            if (! next_state)
                break;
        }
        else if (2 == next_state) {
            next_state = '\r' == octet ? 3 : '\n' == octet ? 1 : 2;
        }
        else if (3 == next_state) {
            if ('\n' != octet)
                return false;
            next_state = 1;
        }
    }
    parsed.s = s;
    return true;
}

bool
decoder_type::scan_barekey (std::string::const_iterator s, parsed_type& parsed)
{
    std::string::const_iterator const e = string.cend ();
    parsed.kind = TOKEN_INVALID;
    if (s < e && (std::isalnum (*s) || '-' == *s || '_' == *s)) {
        std::string::const_iterator c0 = s++;
        while (s < e && (std::isalnum (*s) || '-' == *s || '_' == *s))
            ++s;
        parsed.kind = TOKEN_BAREKEY;
        parsed.s = s;
        parsed.literal.assign (c0, s);
    }
    return parsed.kind != TOKEN_INVALID;
}

bool
decoder_type::scan_number (std::string::const_iterator s, parsed_type& parsed)
{
    static const uint32_t CCLASS[32] = {
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00040350, 0x11111111, 0x11700000,
        0x00000600, 0x00000000, 0x00008000, 0x00900002,
        0x00000600, 0x00000000, 0x00000000, 0x00000000,
    };
    static const int BASE[36] = {
          0,   0,   4,   6,  16,  22,   1,  12,  12,  18,  19,  26,  30,  32, 
         30,  37,  38,  37,  41,  42,  45,  44,  55,  50,  51,  46,  56,  59, 
         51,  61,  65,  62,  67,  75,  71,  79, 
    };
    static const int SHIFT[90] = {
              0, 0x00201, 0x00706, 0x01d01, 0x01d01, 0x00302, 0x01d02, 0x00403,
        0x01d03, 0x01f02, 0x02102, 0x01f03, 0x02103, 0x00807, 0x00102, 0x00908,
        0x00103, 0x00504, 0x01d04, 0x00a09, 0x00b0a, 0x01f04, 0x02104, 0x00505,
        0x01d05, 0x00605, 0x00104, 0x01f05, 0x02105, 0x0170b, 0x0170b, 0x00d0c,
        0x00105, 0x00e0d, 0x00c0b, 0x01c0b, 0x0030b, 0x00f0e, 0x0100f, 0x01110,
        0x01711, 0x01711, 0x01312, 0x01413, 0x01211, 0x01615, 0x01c11, 0x00311,
        0x01714, 0x01714, 0x01514, 0x01817, 0x01918, 0x01a19, 0x01c14, 0x00314,
        0x01616, 0x01b1a, 0x01716, 0x01716, 0x01c1b, 0x0031c, 0x01e1d, 0x0201f,
        0x01c16, 0x00316, 0x01e1e, 0x01d1e, 0x02020, 0x01f20, 0x01f1e, 0x0211e,
        0x02322, 0x02120,       0, 0x0011e, 0x02321, 0x00220, 0x02221, 0x02221,
        0x02323, 0x02223,       0,       0,       0,       0,       0,       0,
              0, 0x00223, 
    };
    static const int NSHIFT = sizeof (SHIFT) / sizeof (SHIFT[0]);
    static const uint32_t MATCH = 10;
    parsed.kind = TOKEN_INVALID;
    std::string literal;
    std::string::const_iterator const e = string.cend ();
    for (int next_state = 1; s <= e; ++s) {
        uint32_t octet = s == e ? '\0' : static_cast<uint8_t> (*s);
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
            int const k = (SHIFT[m] >> 8) & 0xff;
            parsed.kind = 1 == k ? TOKEN_FIXNUM
                        : 2 == k ? TOKEN_FLONUM
                        :          TOKEN_DATETIME;
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

void
decoder_type::encode_codepoint (uint32_t const uc, std::string& literal)
{
    if (uc < 0x80) {
        literal.push_back (uc);
    }
    else if (uc < 0x800) {
        literal.push_back (((uc >>  6) & 0xff) | 0xc0);
        literal.push_back (( uc        & 0x3f) | 0x80);
    }
    else if (uc < 0x10000) {
        literal.push_back (((uc >> 12) & 0x0f) | 0xe0);
        literal.push_back (((uc >>  6) & 0x3f) | 0x80);
        literal.push_back (( uc        & 0x3f) | 0x80);
    }
    else {
        literal.push_back (((uc >> 18) & 0x07) | 0xf0);
        literal.push_back (((uc >> 12) & 0x3f) | 0x80);
        literal.push_back (((uc >>  6) & 0x3f) | 0x80);
        literal.push_back (( uc        & 0x3f) | 0x80);
    }
}

bool
decoder_type::scan_string (std::string::const_iterator s, parsed_type& parsed)
{
    static const uint32_t LOWERBOUNDS[5] = {0, 0x10000L, 0x0800L, 0x80L};
    static const uint32_t UPPERBOUND = 0x10ffffL;
    static const uint32_t CCLASS[32] = {
        0x00000000, 0x09d00000, 0x00000000, 0x00000000,
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
        128, 138, 183, 139, 144, 157, 195, 
    };
    static const int SHIFT[209] = {
              0, 0x02401, 0x00201, 0x10902, 0x10a02, 0x10b02, 0x00107, 0x30c02,
        0x30c02, 0x30c02, 0x30c02, 0x30c02, 0x00d02, 0x30c02, 0x00302, 0x00403,
        0x00208, 0x00103, 0x11604, 0x11704, 0x11804, 0x10a09, 0x31904, 0x31904,
        0x31904, 0x31904, 0x31904, 0x01a04, 0x31904, 0x30504, 0x01904, 0x11605,
        0x11705, 0x11805, 0x10b0a, 0x31905, 0x31905, 0x31905, 0x31905, 0x31905,
        0x01a05, 0x31905, 0x30605, 0x31905, 0x11606, 0x11706, 0x11806, 0x20c0b,
        0x31906, 0x31906, 0x31906, 0x31906, 0x31906, 0x01a06, 0x31906, 0x40706,
        0x31906, 0x1090c, 0x10a0c, 0x10b0c, 0x60f0e, 0x30c0c, 0x30c0c, 0x30c0c,
        0x30c0c, 0x30c0c, 0x00d0c, 0x30c0c, 0x0080c, 0x50c0d, 0x50c0d, 0x00e0d,
        0x0120d, 0x50c0d, 0x50c0d, 0x50c0d, 0x50c0d, 0x6100f, 0x61110, 0x61211,
        0x61312, 0x61413, 0x61514, 0x70c15, 0x11716, 0x11817, 0x21918, 0x11619,
        0x11719, 0x11819, 0x61c1b, 0x31919, 0x31919, 0x31919, 0x31919, 0x31919,
        0x01a19, 0x31919, 0x30519, 0x31919, 0x5191a, 0x5191a, 0x01b1a, 0x01f1a,
        0x5191a, 0x5191a, 0x5191a, 0x5191a, 0x0231a, 0x61d1c, 0x61e1d, 0x61f1e,
        0x6201f, 0x62120, 0x62221, 0x71922, 0x11623, 0x11723, 0x11823, 0x12a29,
        0x31923, 0x31923, 0x31923, 0x31923, 0x02323, 0x01a23, 0x31923, 0x30523,
        0x02323, 0x12924, 0x12a24, 0x12b24, 0x12b2a, 0x32c24, 0x32c24, 0x32c24,
        0x32c24, 0x32c24, 0x32c24, 0x02524, 0x32c24, 0x02625, 0x22c2b, 0x12e2d,
        0x00125, 0x12d26, 0x12e26, 0x12f26, 0x12f2e, 0x33026, 0x33026, 0x33026,
        0x33026, 0x33026, 0x33026, 0x32726, 0x33026, 0x03026, 0x12d27, 0x12e27,
        0x12f27, 0x2302f, 0x33027, 0x33027, 0x33027, 0x33027, 0x33027, 0x33027,
        0x32827, 0x33027, 0x33027, 0x12d28, 0x12e28, 0x12f28,       0, 0x33028,
        0x33028, 0x33028, 0x33028, 0x33028, 0x33028, 0x40728, 0x33028, 0x33028,
        0x1292c, 0x12a2c, 0x12b2c,       0, 0x32c2c, 0x32c2c, 0x32c2c, 0x32c2c,
        0x32c2c, 0x32c2c, 0x0072c, 0x32c2c, 0x12d30, 0x12e30, 0x12f30,       0,
        0x33030, 0x33030, 0x33030, 0x33030, 0x33030, 0x33030, 0x32730, 0x33030,
        0x33030, 
    };
    static const int NSHIFT = sizeof (SHIFT) / sizeof (SHIFT[0]);
    static const int MATCH = 0x0e;
    std::string literal;
    uint32_t uc = 0;
    int mbyte = 1;
    std::string::const_iterator const e = string.cend ();
    parsed.kind = TOKEN_INVALID;
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
            parsed.kind = 1 == ((SHIFT[m] >> 8) & 0xff) ? TOKEN_STRING : TOKEN_STRKEY;
            parsed.literal = literal;
            parsed.s = s;
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
            literal.push_back (octet);
            break;
        case 2:
            uc = (uc << 6) | (0x3f & octet);
            if (uc < LOWERBOUNDS[mbyte] || UPPERBOUND < uc)
                return false;
            if (0xd800L <= uc && uc <= 0xdfffL)
                return false;
            literal.push_back (octet);
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
            default: return false;
            }
            break;
        case 6:
            uc = (uc << 4) + hex (octet);
            break;
        case 7:
            uc = (uc << 4) + hex (octet);
            if ((0xd800L <= uc && uc <= 0xdfffL) || UPPERBOUND < uc)
                return false;
            encode_codepoint (uc, literal);
            uc = 0;
            break;
        default:
            throw std::logic_error ("unexpected lex string action number");
        }
    }
    return parsed.kind != TOKEN_INVALID;
}

bool
decoder_type::parse (doc_type& doc)
{
    enum {
        NPROD = 40, NSTATE = 65, NGRAMMAR = 330, LRCOL = 30, ACCEPT = 255
    };
    static const int BASE[NSTATE] = {
          0,   0, -13,   4,  -3, -11,   7,  -2,  19,  23,  27,  -3,  37,  39,
         33,  49,  41,  57,  71,  63,  75,   0,  77,  71,  75,  77,  88,  95,
        102, 109, 116, 130, 141,   9,  82, 150,  89,  15,  31, 178,  57, 188,
         37,  98, 105, 202,  37, 204,  39, 150, 196, 232, 235, 250, 252,  41,
        254,  94, 282, 112, 293, 296, 202, 300, 133, 
    };
    static const int GRAMMAR[NGRAMMAR] = {
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
             0, 0x0b3f, 
    };
    static const int GOTO[NPROD] = {
        17, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 21, 21,
        22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 25, 25, 26, 26,
        27, 27, 27, 28, 28, 29
    };
    static const int NRHS[NPROD] = {
        11,  2,  1,  1,  0,  6,  5,  8,  7,  5,  4,  7,  6,  3,  1,  1,  1,
         3,  2,  1,  1,  1,  1,  1,  1,  3,  3,  1,  1,  3,  3,  5,  0,  1,
         0,  1,  2,  1,  3,  3
    };
    iter = string.cbegin ();
    kvstate = 0;
    mark.clear ();
    std::deque<int> sstack {1};
    std::deque<value_id> dstack {0}; // centinel
    value_id token_value;
    int token_type = next_token (doc, token_value);
    for (;;) {
        int prev_state = sstack.back ();        
        int j = BASE[prev_state] + token_type;
        int ctrl = 0;
        if (0 < j && j < NGRAMMAR && (GRAMMAR[j] & 0xff) == prev_state) {
            ctrl = GRAMMAR[j] >> 8;
        }
        if (! ctrl)
            break;
        else if (ctrl < 128) {  // shift
            sstack.push_back (ctrl);
            dstack.push_back (token_value);
            token_type = next_token (doc, token_value);
        }
        else if (ctrl == ACCEPT) {
            doc.root = dstack.back ();
            return true;
        }
        else {    // reduce
            int prod = 256 - ctrl - 2;
            int nrhs = NRHS[prod];
            std::deque<value_id>::iterator v = dstack.end () - nrhs - 1;
            value_id value = nrhs > 0 ? v[1] : 0;
            switch (prod) {
            case 1: // toml: statements sections
                value = merge_exclusive (doc, v[1], v[2]);
                break;
            case 5: // sections: sections "[" keypath "]" ENDLINE statements
                value = merge_table (doc, v[1], v[3], v[6]);
                break;
            case 6: // sections: sections "[" keypath "]" ENDLINE
                value = merge_table (doc, v[1], v[3], doc.table ());
                break;
            case 7: // sections: sections "[" "[" keypath "]" "]" ENDLINE statements
                value = merge_array (doc, v[1], v[4], v[8]);
                break;
            case 8: // sections: sections "[" "[" keypath "]" "]" ENDLINE
                value = merge_array (doc, v[1], v[4], doc.table ());
                break;
            case 9: // sections: "[" keypath "]" ENDLINE statements
                value = merge_table (doc, doc.table (), v[2], v[5]);
                break;
            case 10: // sections: "[" keypath "]" ENDLINE
                value = merge_table (doc, doc.table (), v[3], doc.table ());
                break;
            case 11: // sections: "[" "[" keypath "]" "]" ENDLINE statements
                value = merge_array (doc, doc.table (), v[3], v[7]);
                break;
            case 12: // sections: "[" "[" keypath "]" "]" ENDLINE
                value = merge_array (doc, doc.table (), v[3], doc.table ());
                break;
            case 13: // keypath: keypath "." key
                value = doc.set (v[1], doc.size (v[1]), v[3]);
                break;
            case 14: // keypath: key
                value = doc.set (doc.array (), 0, v[1]);
                break;
            case 17: // statements: statements pair ENDLINE
                value = merge_exclusive (doc, v[1], v[2]);
                break;
            case 25: // value: "[" array "]"
                value = v[2];
                break;
            case 26: // value: "{" table "}"
                kvstate = 2;
                value = v[2];
                break;
            case 30: // value_list: endln value endln
                value = doc.set (doc.array (), 0, v[2]);
                break;
            case 31: // value_list: value_list "," endln value endln
                value = doc.set_unify (v[1], doc.size (v[1]), v[4]);
                break;
            case 34: // table:
                value = doc.table ();
                break;
            case 38: // pair_list: pair_list "," pair
                value = merge_exclusive (doc, v[1], v[3]);
                break;
            case 39: // pair: key "=" value
                kvstate = 1;
                value = doc.set (doc.table (), doc.at_string (v[1]), v[3]);
                break;
            }
            for (int i = 0; i < nrhs; ++i)
                sstack.pop_back ();
            for (int i = 0; i < nrhs; ++i)
                dstack.pop_back ();
            int gprev_state = sstack.back ();
            int g = BASE[gprev_state] + GOTO[prod];
            int gnext_state = 0;
            if (0 < g && g < NGRAMMAR && (GRAMMAR[g] & 0xff) == gprev_state)
                gnext_state = GRAMMAR[g] >> 8;
            if (! gnext_state)
                std::logic_error ("parser: GRAMMAR table error");
            sstack.push_back (gnext_state);
            dstack.push_back (value);
        }
    }
    return false;
}

value_id
decoder_type::merge_exclusive (doc_type& doc,value_id const a, value_id const b)
{
    if (doc.at_tag (a) != VALUE_TABLE || doc.at_tag (b) != VALUE_TABLE)
        throw std::out_of_range ("merge_exclusive: invalid type");
    for (auto& i : doc.at_table (b)) {
        if (doc.exists (a, i.first))
            throw std::out_of_range ("merge_exclusive: conflict table");
        doc.set (a, i.first, i.second);
    }
    return a;
}

std::string
decoder_type::path_to_string (doc_type& doc,value_id path)
{
    if (doc.at_tag (path) != VALUE_ARRAY)
        std::out_of_range ("path_to_string: invalid type");
    std::string t;
    for (auto key : doc.at_array (path)) {
        if (doc.at_tag (key) != VALUE_STRING)
            throw std::out_of_range ("path_to_string: invalid key type");
        t.push_back ('.');
        for (auto c : doc.at_string (key))
            switch (c) {
            case '.': t += "\\."; break;
            case '\\': t += "\\\\"; break;
            default: t.push_back (c); break;
            }
    }
    return std::move (t);
}

value_id
decoder_type::merge_table (doc_type& doc,
    value_id const root, value_id const path, value_id const body)
{
    if (doc.at_tag (root) != VALUE_TABLE || doc.at_tag (path) != VALUE_ARRAY
            || doc.at_tag (body) != VALUE_TABLE)
        throw std::out_of_range ("merge_table: invalid type");
    if (doc.size (path) > 0) {
        std::string spath = path_to_string (doc, path);
        if (mark.count (spath) > 0)
            throw std::out_of_range ("merge_table: path already used");
        mark[spath] = 1;
    }
    value_id node = root;
    std::size_t i = 0;
    for (;;) {
        if (doc.at_tag (node) == VALUE_ARRAY)
            node = doc.get (node, doc.size (node) - 1);
        else if (doc.at_tag (node) != VALUE_TABLE)
            throw std::out_of_range ("merge_table: conflict table");
        else if (i < doc.size (path)) {
            std::string key = doc.at_string (doc.get (path, i));
            if (! doc.exists (node, key))
                doc.set (node, key, doc.table ());
            node = doc.get (node, key);
            ++i;
        }
        else {
            merge_exclusive (doc, node, body);
            break;
        }
    }
    return root;
}

value_id
decoder_type::merge_array (doc_type& doc,
    value_id const root, value_id const path, value_id const body)
{
    if (doc.at_tag (root) != VALUE_TABLE || doc.at_tag (path) != VALUE_ARRAY
            || doc.at_tag (body) != VALUE_TABLE)
        throw std::out_of_range ("merge_array: invalid type");
    if (doc.size (path) > 0) {
        std::string spath = path_to_string (doc, path);
        if (mark.count (spath) > 0 && mark.at (spath) == 1)
            throw std::out_of_range ("merge_array: path already used as table");
        mark[spath] = 2;
    }
    value_id node = root;
    std::size_t i = 0;
    for (;;) {
        if (doc.at_tag (node) == VALUE_ARRAY)
            node = doc.get (node, doc.size (node) - 1);
        else if (i + 1 < doc.size (path)) {
            if (doc.at_tag (node) != VALUE_TABLE)
                throw std::out_of_range ("merge_array: conflict table");
            std::string key = doc.at_string (doc.get (path, i));
            if (! doc.exists (node, key))
                doc.set (node, key, doc.table ());
            node = doc.get (node, key);
            ++i;
        }
        else {
            std::string key = doc.at_string (doc.get (path, i));
            if (! doc.exists (node, key))
                doc.set (node, key, doc.array ());
            node = doc.get (node, key);
            if (doc.at_tag (node) != VALUE_ARRAY)
                throw std::out_of_range ("merge_array: conflict table");
            doc.set_unify (node, doc.size (node), body);
            break;
        }
    }
    return root;
}

bool
doc_type::decode_toml (std::string const& source)
{
    decoder_type decoder (source);
    return decoder.parse (*this);
}

}//namespace toml
