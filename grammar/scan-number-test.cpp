#include <string>

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

struct parsed_type {
    int kind;
    int64_t literal_fixnum;
    double literal_flonum;
    std::string literal;
    std::string::const_iterator s;
};

// mock decoder_type for test
struct decoder_type {
    std::string const& string;
    decoder_type (std::string const& x) : string (x) {}

    bool scan_number (std::string::const_iterator s, parsed_type& parsed);
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

#include "decoder-scan-number.cpp"

#include "taptests.hpp"

void
test1 (test::simple& ts)
{
    std::string input (
    "+99 #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "+99 scan");
    ts.ok (parsed.kind == TOKEN_FIXNUM, "+99 kind");
    ts.ok (parsed.literal == "+99", "+99 literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "+99 postmatch");
}

void
test2 (test::simple& ts)
{
    std::string input (
    "42 #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "42 scan");
    ts.ok (parsed.kind == TOKEN_FIXNUM, "42 kind");
    ts.ok (parsed.literal == "42", "42 literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "42 postmatch");
}

void
test3 (test::simple& ts)
{
    std::string input (
    "0 #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "0 scan");
    ts.ok (parsed.kind == TOKEN_FIXNUM, "0 kind");
    ts.ok (parsed.literal == "0", "0 literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "0 postmatch");
}

void
test4 (test::simple& ts)
{
    std::string input (
    "-17 #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "-17 scan");
    ts.ok (parsed.kind == TOKEN_FIXNUM, "-17 kind");
    ts.ok (parsed.literal == "-17", "-17 literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "-17 postmatch");
}

void
test5 (test::simple& ts)
{
    std::string input (
    "1_000 #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "1_000 scan");
    ts.ok (parsed.kind == TOKEN_FIXNUM, "1_000 kind");
    ts.ok (parsed.literal == "1000", "1_000 literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "1_000 postmatch");
}

void
test6 (test::simple& ts)
{
    std::string input (
    "5_349_221 #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "5_349_221 scan");
    ts.ok (parsed.kind == TOKEN_FIXNUM, "5_349_221 kind");
    ts.ok (parsed.literal == "5349221", "5_349_221 literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "5_349_221 postmatch");
}

void
test7 (test::simple& ts)
{
    std::string input (
    "1_2_3_4_5 #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "1_2_3_4_5 scan");
    ts.ok (parsed.kind == TOKEN_FIXNUM, "1_2_3_4_5 kind");
    ts.ok (parsed.literal == "12345", "1_2_3_4_5 literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "1_2_3_4_5 postmatch");
}

void
test8 (test::simple& ts)
{
    std::string input (
    "+1.0 #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "+1.0 scan");
    ts.ok (parsed.kind == TOKEN_FLONUM, "+1.0 kind");
    ts.ok (parsed.literal == "+1.0", "+1.0 literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "+1.0 postmatch");
}

void
test9 (test::simple& ts)
{
    std::string input (
    "3.1415 #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "3.1415 scan");
    ts.ok (parsed.kind == TOKEN_FLONUM, "3.1415 kind");
    ts.ok (parsed.literal == "3.1415", "3.1415 literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "3.1415 postmatch");
}

void
test10 (test::simple& ts)
{
    std::string input (
    "-0.01 #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "-0.01 scan");
    ts.ok (parsed.kind == TOKEN_FLONUM, "-0.01 kind");
    ts.ok (parsed.literal == "-0.01", "-0.01 literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "-0.01 postmatch");
}

void
test11 (test::simple& ts)
{
    std::string input (
    "5e+22 #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "5e+22 scan");
    ts.ok (parsed.kind == TOKEN_FLONUM, "5e+22 kind");
    ts.ok (parsed.literal == "5e+22", "5e+22 literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "5e+22 postmatch");
}

void
test12 (test::simple& ts)
{
    std::string input (
    "1e6 #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "1e6 scan");
    ts.ok (parsed.kind == TOKEN_FLONUM, "1e6 kind");
    ts.ok (parsed.literal == "1e6", "1e6 literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "1e6 postmatch");
}

void
test13 (test::simple& ts)
{
    std::string input (
    "-2E-2 #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "-2E-2 scan");
    ts.ok (parsed.kind == TOKEN_FLONUM, "-2E-2 kind");
    ts.ok (parsed.literal == "-2E-2", "-2E-2 literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "-2E-2 postmatch");
}

void
test14 (test::simple& ts)
{
    std::string input (
    "6.626e-34 #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "6.626e-34 scan");
    ts.ok (parsed.kind == TOKEN_FLONUM, "6.626e-34 kind");
    ts.ok (parsed.literal == "6.626e-34", "6.626e-34 literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "6.626e-34 postmatch");
}

void
test15 (test::simple& ts)
{
    std::string input (
    "9_224_617.445_991_228_313 #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "9_224_617.445_991_228_313 scan");
    ts.ok (parsed.kind == TOKEN_FLONUM, "9_224_617.445_991_228_313 kind");
    ts.ok (parsed.literal == "9224617.445991228313", "9_224_617.445_991_228_313 literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "9_224_617.445_991_228_313 postmatch");
}

void
test16 (test::simple& ts)
{
    std::string input (
    "1e0_200 #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "1e0_200 scan");
    ts.ok (parsed.kind == TOKEN_FLONUM, "1e0_200 kind");
    ts.ok (parsed.literal == "1e0200", "1e0_200 literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "1e0_200 postmatch");
}

void
test17 (test::simple& ts)
{
    std::string input (
    "1979-05-27T07:32:00Z #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "1979-05-27T07:32:00Z scan");
    ts.ok (parsed.kind == TOKEN_DATETIME, "1979-05-27T07:32:00Z kind");
    ts.ok (parsed.literal == "1979-05-27T07:32:00Z", "1979-05-27T07:32:00Z literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "1979-05-27T07:32:00Z postmatch");
}

void
test18 (test::simple& ts)
{
    std::string input (
    "1979-05-27T00:32:00-07:00 #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "1979-05-27T00:32:00-07:00 scan");
    ts.ok (parsed.kind == TOKEN_DATETIME, "1979-05-27T00:32:00-07:00 kind");
    ts.ok (parsed.literal == "1979-05-27T00:32:00-07:00", "1979-05-27T00:32:00-07:00 literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "1979-05-27T00:32:00-07:00 postmatch");
}

void
test19 (test::simple& ts)
{
    std::string input (
    "1979-05-27T00:32:00.999999-07:00 #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "1979-05-27T00:32:00.999999-07:00 scan");
    ts.ok (parsed.kind == TOKEN_DATETIME, "1979-05-27T00:32:00.999999-07:00 kind");
    ts.ok (parsed.literal == "1979-05-27T00:32:00.999999-07:00", "1979-05-27T00:32:00.999999-07:00 literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "1979-05-27T00:32:00.999999-07:00 postmatch");
}

void
test20 (test::simple& ts)
{
    std::string input (
    "1979-05-27 #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "1979-05-27 scan");
    ts.ok (parsed.kind == TOKEN_DATETIME, "1979-05-27 kind");
    ts.ok (parsed.literal == "1979-05-27", "1979-05-27 literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "1979-05-27 postmatch");
}

void
test21 (test::simple& ts)
{
    std::string input (
    "1979-05-27Z #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "1979-05-27Z scan");
    ts.ok (parsed.kind == TOKEN_DATETIME, "1979-05-27Z kind");
    ts.ok (parsed.literal == "1979-05-27Z", "1979-05-27Z literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "1979-05-27Z postmatch");
}

void
test22 (test::simple& ts)
{
    std::string input (
    "1979-05-27-07:00 #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "1979-05-27-07:00 scan");
    ts.ok (parsed.kind == TOKEN_DATETIME, "1979-05-27-07:00 kind");
    ts.ok (parsed.literal == "1979-05-27-07:00", "1979-05-27-07:00 literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "1979-05-27-07:00 postmatch");
}

void
test23 (test::simple& ts)
{
    std::string input (
    "1979-05-27T00:32 #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "1979-05-27T00:32 scan");
    ts.ok (parsed.kind == TOKEN_DATETIME, "1979-05-27T00:32 kind");
    ts.ok (parsed.literal == "1979-05-27T00:32", "1979-05-27T00:32 literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "1979-05-27T00:32 postmatch");
}

void
test24 (test::simple& ts)
{
    std::string input (
    "1979-05-27T00:32Z #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "1979-05-27T00:32Z scan");
    ts.ok (parsed.kind == TOKEN_DATETIME, "1979-05-27T00:32Z kind");
    ts.ok (parsed.literal == "1979-05-27T00:32Z", "1979-05-27T00:32Z literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "1979-05-27T00:32Z postmatch");
}

void
test25 (test::simple& ts)
{
    std::string input (
    "1979-05-27T00:32-07:00 #\n"
    );
    decoder_type de (input);
    parsed_type parsed;
    ts.ok (de.scan_number (input.cbegin (), parsed), "1979-05-27T00:32-07:00 scan");
    ts.ok (parsed.kind == TOKEN_DATETIME, "1979-05-27T00:32-07:00 kind");
    ts.ok (parsed.literal == "1979-05-27T00:32-07:00", "1979-05-27T00:32-07:00 literal");
    std::string postmatch (parsed.s, input.cend ());
    ts.ok (postmatch == " #\n", "1979-05-27T00:32-07:00 postmatch");
}

int
main ()
{
    test::simple ts (100);
    test1 (ts);
    test2 (ts);
    test3 (ts);
    test4 (ts);
    test5 (ts);
    test6 (ts);
    test7 (ts);
    test8 (ts);
    test9 (ts);
    test10 (ts);
    test11 (ts);
    test12 (ts);
    test13 (ts);
    test14 (ts);
    test15 (ts);
    test16 (ts);
    test17 (ts);
    test18 (ts);
    test19 (ts);
    test20 (ts);
    test21 (ts);
    test22 (ts);
    test23 (ts);
    test24 (ts);
    test25 (ts);
    return ts.done_testing ();
}
