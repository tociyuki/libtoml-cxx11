#include <string>
#include <utility>
#include <stdexcept>
#include "value.hpp"
#include "encode-utf8.hpp"

namespace wjson {

enum {BLOCK_IN, BLOCK_OUT, FLOW_OUT, FLOW_IN, BLOCK_KEY, FLOW_KEY};
enum {TOKEN_INVALID, TOKEN_FIXNUM, TOKEN_FLONUM, TOKEN_DATETIME};

class derivs_type {
public:
    derivs_type (std::string::const_iterator const bos, std::string::const_iterator const eos);
    ~derivs_type () {}
    derivs_type (derivs_type const& x);
    derivs_type& operator=(derivs_type const& x);
    bool scan (std::string const& pattern);
    bool scan_indent (int const n1, int const n2);
    bool lookahead (std::string const& pattern);
    bool check (std::string const& pattern);
    bool check_indent (int const n1, int const n2);
    bool check_bos ();
    bool check_bol ();
    bool check_eos ();
    int peek () const { return pbegin < peos ? pbegin[0] : -1; }
    int get ();
    void advance (std::size_t pos);
    bool match () { pbegin = pend; return true; }
    bool match (derivs_type const& x) { pbegin = pend = x.pend; return true; }
    bool fail () { pend = pbegin; return false; }
    std::string::const_iterator cbegin () const { return pbegin; }
    std::string::const_iterator cend () const { return pend; }
private:
    std::string::const_iterator pbos;
    std::string::const_iterator pbegin;
    std::string::const_iterator pend;
    std::string::const_iterator peos;
};

static int c7toi (int const c);
static int l_endstream (derivs_type s);
static bool l_document (derivs_type& s, value_type& value);
static bool c_forbidden (derivs_type s);
static bool c_directive (derivs_type& s0);
static bool s_l_block_node (derivs_type& s0, int const n, int const ctx, value_type& value);
static bool s_l_block_indented (derivs_type& s0, int const n, int const ctx, value_type& value);
static bool l_block_sequence (derivs_type& s0, int const n, value_type& value);
static bool ns_l_compact_sequence (derivs_type& s0, int const n, value_type& value);
static bool l_block_seq_entries (derivs_type& s0, int const n, value_type& value);
static bool l_block_mapping (derivs_type& s0, int const n, value_type& value);
static bool ns_l_compact_mapping (derivs_type& s0, int const n, value_type& value);
static bool ns_l_block_map_entry (derivs_type& s0, int const n, value_type& value);
static bool ns_flow_node (derivs_type& s0, int const n, int const ctx, value_type& value);
static bool ns_flow_scalar (derivs_type& s0, int const n, int const ctx, value_type& value);
static bool c_flow_sequence (derivs_type& s0, int const n, int const ctx0, value_type& value);
static bool c_flow_mapping (derivs_type& s0, int const n, int const ctx0, value_type& value);
static bool c_l_scalar (derivs_type& s0, int const n0, value_type& value);
static bool ns_plain (derivs_type& s0, int const n, int const ctx, value_type& value);
static int scan_number (std::string const& string, value_type& value);
static bool c_quoted (derivs_type& s0, int const n, int const ctx, value_type& value);
static bool c_escaped (derivs_type& s, int const n, int const ctx, std::string& lit);
static bool s_flow_folded (derivs_type& s0, int const n, int& nbreak);
static bool c_b_block_header (derivs_type& s0, int const n, int& m, int& t);
static bool l_trail_comments (derivs_type& s0, int const n);
static bool s_separate (derivs_type& s0, int const n, int const ctx);
static bool s_b_comment (derivs_type& s0);
static bool s_l_comment (derivs_type& s0);

static int c7toi (int c)
{
    static const int tbl[128] = {
    //                                      \t  \n
        50, 50, 50, 50, 50, 50, 50, 50, 50, 39, 40, 50, 50, 50, 50, 50,
        50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
    //       !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /
        39, 37, 37, 37, 36, 37, 37, 37, 36, 36, 37, 36, 38, 36, 36, 36,
    //   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 36, 36, 36, 36, 37, 36,
    //   @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
        37, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    //   P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _
        25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 38, 36, 38, 36, 36,
    //   `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
        36, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    //   p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~
        25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 38, 37, 38, 36, 50,
    };
    return 0 <= c && c <= 126 ? tbl[c] : 36;
}

std::string::size_type
decode_yaml (std::string const& input, value_type& value, std::string::size_type pos)
{
    derivs_type s (input.cbegin (), input.cend ());
    s.advance (pos);
    int endok = l_endstream (s);
    if (endok < 0)
        return std::string::npos;
    if (endok == 0) {
        value = ::wjson::null ();
        return input.size ();
    }
    bool ok = l_document (s, value);
    return ok ? s.cend () - input.cbegin () : std::string::npos;
}

static int
l_endstream (derivs_type s)
{
    while (s.scan ("^%.%.%.%b")) {
        if (! s_l_comment (s))
            return -1;
    }
    return s.check_eos () ? 0 : 1;
}

static bool
l_document (derivs_type& s0, value_type& value)
{
    derivs_type s = s0;
    bool first_document = s.check_bos ();
    if (! first_document && ! c_forbidden (s))
        return s0.fail ();
    if (first_document)
        s_l_comment (s);
    int docend = false;
    while (s.scan ("^%.%.%.%b")) {
        docend = true;
        if (! s_l_comment (s))
            return s0.fail ();
    }
    int ndirective = 0;
    if (first_document || docend)
        while (c_directive (s))
            ++ndirective;
    bool dirend = false;
    if (s.scan ("^---%b"))
        dirend = true;
    if (ndirective > 0 && ! dirend)
        return s0.fail ();
    if (! s_l_block_node (s, -1, BLOCK_IN, value)) {
        if (! s_l_comment (s))
            return s0.fail ();
        if (! first_document && ! dirend)
            return s0.fail ();
        value = ::wjson::null ();
    }
    if (! s.check_eos () && ! c_forbidden (s))
        return s0.fail ();
    return s0.match (s);
}

static bool
c_forbidden (derivs_type s)
{
    return s.lookahead ("^---%b") || s.lookahead ("^%.%.%.%b");
}

static bool
c_directive (derivs_type& s0)
{
    derivs_type s = s0;
    if (s.scan ("%%.{0,*}") && s_l_comment (s))
        return s0.match (s);
    return s0.fail ();
}

static bool
s_l_block_node (derivs_type& s0, int const n, int const ctx, value_type& value)
{
    derivs_type s = s0;
    if (s_separate (s, n + 1, ctx) && c_l_scalar (s, n, value))
        return s0.match (s);
    s = s0;
    int n1 = BLOCK_OUT == ctx ? n - 1 : n;
    if (s_l_comment (s)
            && (l_block_sequence(s, n1, value) || l_block_mapping (s, n, value)))
        return s0.match (s);
    s = s0;
    if (s_separate (s, n + 1, FLOW_OUT)
            && ns_flow_node (s, n + 1, FLOW_OUT, value)
            && s_l_comment (s))
        return s0.match (s);
    value = ::wjson::null ();
    return s0.fail ();
}

static bool
s_l_block_indented (derivs_type& s0, int const n, int const ctx, value_type& value)
{
    derivs_type s = s0;
    s.scan_indent (0, -1);
    int m = s.cend () - s0.cbegin ();
    if (ns_l_compact_sequence (s, n + 1 + m, value)
            || ns_l_compact_mapping (s, n + 1 + m, value))
        return s0.match (s);
    s = s0;
    if (s_l_block_node (s, n , ctx, value))
        return s0.match (s);
    s = s0;
    value = ::wjson::null ();
    if (s_l_comment (s))
        return s0.match (s);
    s = s0;
    return s0.fail ();
}

static bool
l_block_sequence (derivs_type& s0, int const n, value_type& value)
{
    derivs_type s = s0;
    if (! s.scan_indent (n + 1, -1))
        return s0.fail ();
    int n1 = s.cend () - s0.cbegin ();
    if (! s.scan ("-%b"))
        return s0.fail ();
    value = ::wjson::array ();
    return l_block_seq_entries (s0, n1, value);
}

static bool
ns_l_compact_sequence (derivs_type& s0, int const n, value_type& value)
{
    derivs_type s = s0;
    if (! s.scan ("-%b"))
        return s0.fail ();
    value = ::wjson::array ();;
    value_type item;
    if (! s_l_block_indented (s, n, BLOCK_IN, item))
        return s0.fail ();
    value.push_back (item);
    s0 = s;
    return l_block_seq_entries (s0, n, value);
}

static bool
l_block_seq_entries (derivs_type& s0, int const n, value_type& value)
{
    derivs_type s = s0;
    for (;;) {
        value_type item;
        if (! (s.scan_indent (n, n) && s.scan ("-%b")))
            break;
        if (! s_l_block_indented (s, n, BLOCK_IN, item))
            break;
        value.push_back (std::move (item));
    }
    s0 = s;
    return true;
}

static bool
l_block_mapping (derivs_type& s0, int const n, value_type& value)
{
    if (! s0.check_indent (n + 1, -1))
        return s0.fail ();
    int n1 = s0.cend () - s0.cbegin ();
    s0.match (s0);
    return ns_l_compact_mapping (s0, n1, value);
}

static bool
ns_l_compact_mapping (derivs_type& s0, int const n, value_type& value)
{
    derivs_type s = s0;
    value = ::wjson::table ();
    if (! ns_l_block_map_entry (s, n, value))
        return s0.fail ();
    for (;;) {
        derivs_type s1 = s;
        if (! (s1.scan_indent (n, n) && ns_l_block_map_entry (s1, n, value)))
            break;
        s.match (s1);
    }
    return s0.match (s);
}

static bool
ns_l_block_map_entry (derivs_type& s0, int const n, value_type& value)
{
    value_type key = ::wjson::string (L"");
    value_type item;
    derivs_type s = s0;
    ns_flow_scalar (s, 0, BLOCK_KEY, key);
    if (! s.scan ("%s{0,*}:"))
        return s0.fail ();
    if (s_l_block_node (s, n, BLOCK_OUT, item)) {
        value.set (key, item);
        return s0.match (s);
    }
    else if (s_l_comment (s)) {
        value.set (key, ::wjson::null ());
        return s0.match (s);
    }
    return s0.fail ();
}

static bool
ns_flow_scalar (derivs_type& s0, int const n, int const ctx, value_type& value)
{
    return ns_plain (s0, n, ctx, value) || c_quoted (s0, n, ctx, value);
}

static bool
ns_flow_node (derivs_type& s0, int const n, int const ctx, value_type& value)
{
    return ns_flow_scalar (s0, n, ctx, value)
        || c_flow_sequence (s0, n, ctx, value)
        || c_flow_mapping (s0, n, ctx, value);
}

static bool
c_flow_sequence (derivs_type& s0, int const n, int const ctx0, value_type& value)
{
    derivs_type s = s0;
    if (! s.scan ("["))
        return s0.fail ();
    value = ::wjson::array ();
    s_separate (s, n, ctx0);
    int const ctx = (BLOCK_KEY == ctx0 || FLOW_KEY == ctx0) ? FLOW_KEY : FLOW_IN;
    for (;;) {
        value_type item;
        if (! ns_flow_node (s, n, ctx, item))
            break;
        value.push_back (item);
        s_separate (s, n, ctx);
        if (! s.scan (","))
            break;
        s_separate (s, n, ctx);
    }
    if (! s.scan ("]"))
        return s0.fail ();
    return s0.match (s);
}

static bool
c_flow_mapping (derivs_type& s0, int const n, int const ctx0, value_type& value)
{
    derivs_type s = s0;
    if (! s.scan ("{"))
        return s0.fail ();
    value = ::wjson::table ();
    s_separate (s, n, ctx0);
    int const ctx = (BLOCK_KEY == ctx0 || FLOW_KEY == ctx0) ? FLOW_KEY : FLOW_IN;
    for (;;) {
        value_type key = ::wjson::string (L"");
        value_type item;
        int got = 0;
        derivs_type la = s;
        if (ns_flow_scalar (s, n, ctx, key)) {
            s_separate (s, n, ctx);
            ++got;
        }
        if (s.scan (":")) {
            s_separate (s, n, ctx);
            if (! s.lookahead (",") && ! s.lookahead ("}")
                    && ! ns_flow_node (s, n, ctx, item))
                break;
            s_separate (s, n,ctx);
            ++got;
        }
        if (got == 0)
            break;
        value.set (key, item);
        if (! s.scan (","))
            break;
        s_separate (s, n, ctx);
    }
    if (! s.scan ("}"))
        return s0.fail ();
    return s0.match (s);
}

static bool
c_l_scalar (derivs_type& s0, int const n0, value_type& value)
{
    std::string octets;
    derivs_type s = s0;
    int m;
    int t;
    if (! s0.lookahead ("|") && ! s0.lookahead (">"))
        return s0.fail ();
    int indicator = s.get ();
    if (! c_b_block_header (s, n0, m, t))
        return s0.fail ();
    int n = n0 + m;
    int nbreak = 0;
    bool clipable = false;
    bool foldable = false;
    for (;;) {
        derivs_type s1 = s;
        derivs_type s2 = s;
        derivs_type s3 = s;
        if (c_forbidden (s))
            break;
        if (s1.scan_indent (0, n) && s1.check ("\n")) {
            s.match (s1);
            ++nbreak;
        }
        else if (s2.scan_indent (n, n) && s2.check ("%s.{0,*}\n")) {
            if (nbreak > 0)
                octets.append (nbreak, '\n');
            octets.append (s2.cbegin (), s2.cend () - 1);
            s.match (s2);
            nbreak = 1;
            clipable = true;
            foldable = false;
        }
        else if (s3.scan_indent (n, n) && s3.check ("%S.{0,*}\n")) {
            if (! foldable && nbreak > 0)
                octets.append (nbreak, '\n');
            else if (nbreak == 1)
                octets.push_back (' ');
            else if (nbreak > 1)
                octets.append (nbreak - 1, '\n');
            octets.append (s3.cbegin (), s3.cend () - 1);
            s.match (s3);
            nbreak = 1;
            clipable = true;
            foldable = '>' == indicator;
        }
        else
            break;
    }
    if ('+' == t && nbreak > 0)
        octets.append (nbreak, '\n');
    else if (' ' == t && clipable)
        octets.push_back ('\n');
    if (! l_trail_comments (s, n))
        return s0.fail ();
    std::wstring lit;
    if (! decode_utf8 (octets, lit))
        return s0.fail ();
    value = ::wjson::string (lit);
    return s0.match (s);
}

static bool
ns_plain (derivs_type& s0, int const n, int const ctx, value_type& value)
{
    std::string octets;
    derivs_type s = s0;
    if (s.check_eos () || c_forbidden (s))
        return s0.fail ();
    int ch1 = s.peek ();
    if (! s.check ("%P"))
        return s0.fail ();
    if ('-' == ch1 || '?' == ch1 || ':' == ch1) {
        if (! s.check (".%S"))
            return s0.fail ();
        if (FLOW_IN == ctx || FLOW_KEY == ctx) {
            if (s.check (".%F"))
                return s0.fail ();
        }
    }
    octets.push_back (s.get ());
    for (;;) {
        derivs_type s1 = s;
        if (s1.scan ("%s{1,*}")) {
            if ('#' == s1.peek ())
                break;
        }
        if (s1.check_eos ())
            break;
        int nbreak = -1;
        if ('\n' == s1.peek ()) {
            if (BLOCK_KEY == ctx || FLOW_KEY == ctx)
                break;
            derivs_type la = s1;
            la.get ();
            if (c_forbidden (la))
                break;
            if (! s_flow_folded (s1, n, nbreak))
                break;
        }
        if (s1.lookahead (":%b"))
            break;
        if (FLOW_IN == ctx || FLOW_KEY == ctx) {
            if (s1.lookahead (":%F") || s1.check ("%F"))
                break;
        }
        if (nbreak < 0 && s.cbegin () < s1.cend ())
            octets.append (s.cbegin (), s1.cend ());
        else if (nbreak == 0)
            octets.push_back (' ');
        else if (nbreak > 0)
            octets.append (nbreak, '\n');
        s.match (s1);
        octets.push_back (s.get ());
    }
    if (BLOCK_KEY == ctx || FLOW_KEY == ctx) {
        std::wstring lit;
        if (! decode_utf8 (octets, lit))
            return s0.fail ();
        value = ::wjson::string (lit);
    }
    else {
        // YAML 1.2 Core Schema
        if (octets == "null" || octets == "NULL" || octets == "Null" || octets == "~")
            value = ::wjson::null ();
        else if (octets == "true" || octets == "TRUE" || octets == "True")
            value = ::wjson::boolean (true);
        else if (octets == "false" || octets == "FALSE" || octets == "False")
            value = ::wjson::boolean (false);
        else if (! scan_number (octets, value)) {
            std::wstring lit;
            if (! decode_utf8 (octets, lit))
                return s0.fail ();
            value = ::wjson::string (lit);
        }
    }
    return s0.match (s);
}

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

int
scan_number (std::string const& string, value_type& value)
{
    enum { PUTNUM = 1, PUTOCT, PUTHEX };
    enum { NSHIFT = 93 };
    static const uint32_t CCLASS[16] = {
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00060570, 0x12222222, 0x33900000,
        0x04444840, 0x00000000, 0x0000a000, 0x00b00000,
        0x04444840, 0x0000000c, 0x00000000, 0xd0000000,
    };
    static const int BASE[15] = {
         0,  0,  7, 10, 15, 22, 30, 34, 44, 47, 50, 64, 68, 74, 78 
    };
    static const int SHIFT[NSHIFT] = {
              0, 0x10201, 0x10a01, 0x10a01,       0, 0x10701, 0x10701,
        0x10801, 0x10802, 0x10802, 0x10802, 0x20403, 0x20403,       0,
        0x10b02, 0x10c02, 0x10404, 0x10404,       0, 0x00302, 0x00502,
        0x00102,       0, 0x30605, 0x30605, 0x30605, 0x30605,       0,
              0, 0x00104, 0x30605, 0x10606, 0x10606, 0x10606, 0x10606,
        0x10a07, 0x10a07, 0x10a07, 0x10606,       0,       0, 0x10807,
              0,       0, 0x00106, 0x10908, 0x10908, 0x10908, 0x10909,
        0x10909, 0x10909, 0x10a0a, 0x10a0a, 0x10a0a,       0, 0x10c09,
              0, 0x10b0a, 0x10c0a,       0,       0, 0x00209,       0,
              0, 0x0010a, 0x10b0b, 0x10b0b, 0x10b0b,       0, 0x10e0c,
        0x10e0c, 0x10e0c, 0x10c0b, 0x10d0c, 0x10d0c, 0x10e0d, 0x10e0d,
        0x10e0d, 0x0020b, 0x10e0e, 0x10e0e, 0x10e0e,       0,       0,
              0,       0,       0,       0,       0,       0,       0,
              0, 0x0020e 
    };
    static const uint32_t MATCH = 14U;
    int kind = TOKEN_INVALID;
    int base = 10;
    std::wstring literal;
    std::string::const_iterator s = string.cbegin ();
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

static bool
c_quoted (derivs_type& s0, int const n, int const ctx, value_type& value)
{
    std::string octets;
    derivs_type s = s0;
    if (! s.check ("\"") && ! s.check ("'"))
        return s0.fail ();
    int indicator = s.get ();
    std::string quote (1, indicator);
    for (;;) {
        if ('\'' == indicator && s.scan ("''")) {
            octets.push_back ('\'');
            continue;
        }
        if (s.scan (quote))
            break;
        if (s.lookahead ("%s{0,*}\n")) {
            if (BLOCK_KEY == ctx || FLOW_KEY == ctx)
                return s0.fail ();
            int nbreak = 0;
            if (s_flow_folded (s, n, nbreak)) {
                if (0 == nbreak)
                    octets.push_back (' ');
                else
                    octets.append (nbreak, '\n');
                continue;
            }
            return s0.fail ();
        }
        if (s.check ("%s{1,*}")) {
            octets.append (s.cbegin (), s.cend ());
            s.match (s);
            continue;
        }
        int ch = s.peek ();
        if ('\t' != ch && ch < ' ')
            return s0.fail ();
        if ('\'' == indicator || '\\' != ch) {
            octets.push_back (s.get ());
            continue;
        }
        if (! c_escaped (s, n, ctx, octets))
            return s0.fail ();
    }
    std::wstring lit;
    if (! decode_utf8 (octets, lit))
        return s0.fail ();
    value = ::wjson::string (lit);
    return s0.match (s);
}

static bool
c_escaped (derivs_type& s, int const n, int const ctx, std::string& octets)
{
    s.get (); // read '\\'
    if (s.check ("x%x%x") || s.check ("u%x{4,4}") || s.check ("U%x{8,8}")) {
        int x = 0;
        for (auto p = s.cbegin () + 1; p < s.cend (); ++p)
            x = x * 16 + c7toi (*p);
        if (x < 0x80) {
            octets.push_back (x);
        }
        else if (x < 0x800) {
            octets.push_back (((x >>  6) & 0xff) | 0xc0);
            octets.push_back (( x        & 0x3f) | 0x80);
        }
        else if (x < 0x10000) {
            octets.push_back (((x >> 12) & 0x0f) | 0xe0);
            octets.push_back (((x >>  6) & 0x3f) | 0x80);
            octets.push_back (( x        & 0x3f) | 0x80);
        }
        else if (x < 0x110000) {
            octets.push_back (((x >> 18) & 0x07) | 0xf0);
            octets.push_back (((x >> 12) & 0x3f) | 0x80);
            octets.push_back (((x >>  6) & 0x3f) | 0x80);
            octets.push_back (( x        & 0x3f) | 0x80);
        }
        return s.match (s);
    }
    int esc = s.peek ();
    if ('\n' == esc) {
        if (BLOCK_KEY == ctx || FLOW_KEY == ctx)
            return s.fail ();
        int nbreak = 0;
        if (s_flow_folded (s, n, nbreak)) {
            if (nbreak > 0)
                octets.append (nbreak, '\n');
            return s.match (s);
        }
        return s.fail ();
    }
    switch (esc) {
    case '0': octets.push_back ('\0'); break;
    case 'a': octets.push_back ('\a'); break;
    case 'b': octets.push_back ('\b'); break;
    case 't': case '\t': octets.push_back ('\t'); break;
    case 'n': octets.push_back ('\n'); break;
    case 'v': octets.push_back ('\x0b'); break;
    case 'f': octets.push_back ('\f'); break;
    case 'r': octets.push_back ('\r'); break;
    case 'e': octets.push_back ('\x1b'); break;
    case 'N': octets.push_back (0xc2);
              octets.push_back (0x85); break;
    case '_': octets.push_back (0xc2);
              octets.push_back (0xa0); break;
    case 'L': octets.push_back (0xe2);
              octets.push_back (0x80);
              octets.push_back (0xa8); break;
    case 'P': octets.push_back (0xe2);
              octets.push_back (0x80);
              octets.push_back (0xa9); break;
    default:
        if (' ' <= esc)
            octets.push_back (esc);
        else
            return s.fail ();
    }
    s.get ();
    return s.match ();
}

static bool
s_flow_folded (derivs_type& s0, int const n, int& nbreak)
{
    int const m = n < 0 ? 0 : n;
    derivs_type s = s0;
    if (! s.scan ("%s{0,*}\n"))
        return s0.fail ();
    nbreak = 0;
    while (s.scan ("%s{0,*}\n"))
        ++nbreak;
    if (s.check_eos ()
            || ! (s.scan_indent (m, m) && s.scan ("%s{0,*}") && s.lookahead ("%S")))
        return s0.fail ();
    return s0.match (s);
}

static bool
c_b_block_header (derivs_type& s0, int const n, int& m, int& t)
{
    derivs_type s = s0;
    m = 1;
    t = ' ';
    int decimal = ' ';
    if (s.check ("+") || s.check ("-"))
        t = s.get ();
    if (s.check ("%d"))
        decimal = s.get ();
    if (' ' == t && (s.check ("+") || s.check ("-")))
        t = s.get ();
    if (! s_b_comment (s))
        return s0.fail ();
    if ('0' <= decimal && decimal <= '9')
        m = decimal - '0';
    else {
        derivs_type la = s;
        while (la.scan ("%s{0,*}\n"))
            ;
        if (la.check (" {0,*}%S"))
            m = la.cend () - la.cbegin () - 1 - n;
    }
    if (m < 1)
        m = 1;
    return s0.match (s);
}

static bool
l_trail_comments (derivs_type& s0, int const n)
{
    derivs_type s = s0;
    int const m = n < 0 ? 0 : n;
    if (! (s.scan_indent (0, m) && s.scan ("#.{0,*}\n")))
        return s0.match (); /* l-trail-comments(n)? */
    while (! s.check_eos ())
        if (! (s.scan ("%s{0,*}$\n{0,1}") || s.scan ("%s{0,*}#.{0,*}$\n{0,1}")))
            break;
    return s0.match (s);
}

static bool
s_separate (derivs_type& s0, int const n, int const ctx)
{
    if (BLOCK_KEY == ctx || FLOW_KEY == ctx)
        return s0.scan ("%s{1,*}") || s0.check_bol ();
    int m = n < 0 ? 0 : n;
    derivs_type s = s0;
    if (s_l_comment (s) && s.scan_indent (m, m) && s.scan ("%s{0,*}"))
        return s0.match (s);
    return s0.scan ("%s{1,*}") || s0.check_bol ();
}

static bool
s_b_comment (derivs_type& s0)
{
    return s0.scan ("%s{0,*}$\n{0,1}")
         || s0.scan ("^#.{0,*}$\n{0,1}")
         || s0.scan ("%s{1,*}#.{0,*}$\n{0,1}");
}

static bool
s_l_comment (derivs_type& s0)
{
    derivs_type s = s0;
    if (! (s_b_comment (s) || s.check_bol ()))
        return s0.fail ();
    while (! s.check_eos ())
        if (! (s.scan ("%s{0,*}$\n{0,1}") || s.scan ("%s{0,*}#.{0,*}$\n{0,1}")))
            break;
    return s0.match (s);
}

derivs_type::derivs_type (std::string::const_iterator const bos, std::string::const_iterator const eos)
{
    pbos = bos;
    pbegin = bos;
    pend = bos;
    peos = eos;
}

derivs_type::derivs_type (derivs_type const& x)
{
    pbos = x.pbos;
    pbegin = x.pbegin;
    pend = x.pend;
    peos = x.peos;
}

derivs_type& derivs_type::operator=(derivs_type const& x)
{
    if (this != &x) {
        pbos = x.pbos;
        pbegin = x.pbegin;
        pend = x.pend;
        peos = x.peos;
    }
    return *this;
}

int
derivs_type::get ()
{
    if (pbegin >= peos)
        return -1;
    pend = ++pbegin;
    return pbegin[-1];
}

void
derivs_type::advance (std::size_t pos)
{
    pbegin = pbos + pos;
    if (pbegin >= peos)
        pbegin = peos;
    pend = pbegin;
}

bool
derivs_type::check_bos ()
{
    pend = pbegin;
    return pbegin == pbos;
}

bool
derivs_type::check_bol ()
{
    pend = pbegin;
    return (pbegin == pbos || '\n' == pbegin[-1]);
}

bool
derivs_type::check_eos ()
{
    pend = pbegin;
    return (pbegin >= peos);
}

bool
derivs_type::lookahead (std::string const& pattern)
{
    bool good = check (pattern);
    pend = pbegin;
    return good;
}

bool
derivs_type::scan (std::string const& pattern)
{
    if (! check (pattern))
        return false;
    return match ();
}

bool
derivs_type::scan_indent (int const n1, int const n2)
{
    if (! check_indent (n1, n2))
        return false;
    return match ();
}

bool
derivs_type::check_indent (int const n1, int const n2)
{
    std::string::const_iterator p = pend = pbegin;
    for (int i = 0; n2 < 0 || i < n2; ++i) {
        int c = p < peos ? *p : -1;
        if (' ' == c) {
            ++p;
            continue;
        }
        if (i < n1)
            return false;
        else
            break;
    }
    pend = p;
    return true;
}

bool
derivs_type::check (std::string const& pattern)
{
    std::string::const_iterator p = pend = pbegin;
    std::string::const_iterator ip = pattern.begin ();
    while (ip < pattern.end ()) {
        if ('^' == *ip) {
            if (p != pbos && '\n' != p[-1])
                return false;
            ++ip;
            continue;
        }
        if ('$' == *ip) {
            if (p < peos && '\n' != *p)
                return false;
            ++ip;
            continue;
        }
        int exact = *ip++;
        bool dot = ('.' == exact);
        int lower = 0; int upper = 0;
        if ('%' == exact && ip < pattern.end ()) {
            exact = *ip++;
            if ('b' == exact) {
                if (p < peos && ' ' < *p)
                    return false;
                continue;
            }
            lower = 'F' == exact ? 38 : 's' == exact ? 39 : 0;
            upper = 'd' == exact ? 10 : 'x' == exact ? 16 : 'P' == exact ? 37
                  : 'F' == exact ? 39 : 's' == exact ? 40 : 'S' == exact ? 39 : 0;
        }
        int n1 = 1; int n2 = 1;
        if (ip + 4 < pattern.end () && L'{' == *ip && L',' == ip[2] && L'}' == ip[4]) {
            n1 = c7toi (ip[1]);
            n2 = c7toi (ip[3]);
            n2 = n2 >= 36 ? -1 : n2;
            ip += 5;
        }
        for (int i = 0; n2 < 0 || i < n2; ++i) {
            int c = p < peos ? *p : -1;
            int x = c7toi (c);
            if (dot && ('\t' == c || ' ' <= c))
                ++p;
            else if (! dot && ((upper == 0 && exact == c) || (lower <= x && x < upper)))
                ++p;
            else if (i < n1)
                return false;
            else
                break;
        }
    }
    pend = p;
    return true;
}

}//namespace wjson
