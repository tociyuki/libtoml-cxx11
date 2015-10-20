#include <string>
#include <stdexcept>
#include "../../value.hpp"

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

// mock toml_decoder_type for test
struct toml_decoder_type {
    std::string const& string;
    std::string::const_iterator iter;
    toml_decoder_type (std::string const& x) : string (x), iter (x.cbegin ()) {}

    int scan_string (value_type& value);
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

#include "toml-decoder-string.cpp"

}//namespace wjson

#include "../../taptests.hpp"

void
test1 (test::simple& ts)
{
    std::string input (
    "\"\" #\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    ts.ok (de.scan_string (value) == wjson::TOKEN_STRING, "\"\" kind");
    ts.ok (value.string () == L"", "\"\" literal");
    std::string postmatch (de.iter, input.cend ());
    ts.ok (postmatch == " #\n", "\"\" postmatch");
}

void
test2 (test::simple& ts)
{
    std::string input (
    "\"\"\"a\"\"b\"c\"\"\" #\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    ts.ok (de.scan_string (value) == wjson::TOKEN_STRING, "\"\"\" kind");
    ts.ok (value.string () == L"a\"\"b\"c", "\"\"\" literal");
    std::string postmatch (de.iter, input.cend ());
    ts.ok (postmatch == " #\n", "\"\"\" postmatch");
}

void
test3 (test::simple& ts)
{
    std::string input (
    "\"abc\" #\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    ts.ok (de.scan_string (value) == wjson::TOKEN_STRKEY, "\"abc\" kind");
    ts.ok (value.string () == L"abc", "\"abc\" literal");
    std::string postmatch (de.iter, input.cend ());
    ts.ok (postmatch == " #\n", "\"abc\" postmatch");
}

void
test4 (test::simple& ts)
{
    std::string input (
    "\"\"\"\n"
    "a\"\"b\"c\"\"\" #\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    ts.ok (de.scan_string (value) == wjson::TOKEN_STRING, "\"\"\"\\n kind");
    ts.ok (value.string () == L"a\"\"b\"c", "\"\"\"\\n literal");
    std::string postmatch (de.iter, input.cend ());
    ts.ok (postmatch == " #\n", "\"\"\"\\n postmatch");
}

void
test5 (test::simple& ts)
{
    std::string input (
    "\"\\t\\n\\r\\f\\\\\\\"\" #\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    ts.ok (de.scan_string (value) == wjson::TOKEN_STRKEY, "\"\\\" kind");
    ts.ok (value.string () == L"\t\n\r\f\\\"", "\"\\\" literal");
    std::string postmatch (de.iter, input.cend ());
    ts.ok (postmatch == " #\n", "\"\\\" postmatch");
}

void
test6 (test::simple& ts)
{
    std::string input (
    "\"\\u0030\\u0031\\U00000032\\U00000033\" #\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    ts.ok (de.scan_string (value) == wjson::TOKEN_STRKEY, "\"\\u\" kind");
    ts.ok (value.string () == L"0123", "\"\\u\" literal");
    std::string postmatch (de.iter, input.cend ());
    ts.ok (postmatch == " #\n", "\"\\u\" postmatch");
}

void
test7 (test::simple& ts)
{
    std::string input (
    "\"\"\"\\\n"
    "      A a \\\n"
    "      B b \\\n"
    "      C c\\\n"
    "\"\"\" #\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    ts.ok (de.scan_string (value) == wjson::TOKEN_STRING, "\"\"\"\\\\n kind");
    ts.ok (value.string () == L"A a B b C c", "\"\"\"\\\\n literal");
    std::string postmatch (de.iter, input.cend ());
    ts.ok (postmatch == " #\n", "\"\"\"\\\\n postmatch");
}

void
test8 (test::simple& ts)
{
    std::string input (
    "\"\"\"\\t\\n\\r\\f\\\\\\\"\"\"\" #\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    ts.ok (de.scan_string (value) == wjson::TOKEN_STRING, "\"\\\" kind");
    ts.ok (value.string () == L"\t\n\r\f\\\"", "\"\\\" literal");
    std::string postmatch (de.iter, input.cend ());
    ts.ok (postmatch == " #\n", "\"\\\" postmatch");
}

void
test9 (test::simple& ts)
{
    std::string input (
    "\"\"\"\\u0030\\u0031\\U00000032\\U00000033\"\"\" #\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    ts.ok (de.scan_string (value) == wjson::TOKEN_STRING, "\"\\u\" kind");
    ts.ok (value.string () == L"0123", "\"\\u\" literal");
    std::string postmatch (de.iter, input.cend ());
    ts.ok (postmatch == " #\n", "\"\\u\" postmatch");
}

void
test10 (test::simple& ts)
{
    std::string input (
    "'' #\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    ts.ok (de.scan_string (value) == wjson::TOKEN_STRING, "'' kind");
    ts.ok (value.string () == L"", "'' literal");
    std::string postmatch (de.iter, input.cend ());
    ts.ok (postmatch == " #\n", "'' postmatch");
}

void
test11 (test::simple& ts)
{
    std::string input (
    "'''a''b'c''' #\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    ts.ok (de.scan_string (value) == wjson::TOKEN_STRING, "''' kind");
    ts.ok (value.string () == L"a''b'c", "''' literal");
    std::string postmatch (de.iter, input.cend ());
    ts.ok (postmatch == " #\n", "''' postmatch");
}

void
test12 (test::simple& ts)
{
    std::string input (
    "'abc' #\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    ts.ok (de.scan_string (value) == wjson::TOKEN_STRING, "'abc' kind");
    ts.ok (value.string () == L"abc", "'abc' literal");
    std::string postmatch (de.iter, input.cend ());
    ts.ok (postmatch == " #\n", "'abc' postmatch");
}

void
test13 (test::simple& ts)
{
    std::string input (
    "'''\n"
    "a''b'c''' #\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    ts.ok (de.scan_string (value) == wjson::TOKEN_STRING, "'''\\n kind");
    ts.ok (value.string () == L"a''b'c", "'''\\n literal");
    std::string postmatch (de.iter, input.cend ());
    ts.ok (postmatch == " #\n", "'''\\n postmatch");
}

void
test14 (test::simple& ts)
{
    std::string input (
    "'a\\b\\c' #\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    ts.ok (de.scan_string (value) == wjson::TOKEN_STRING, "'a\\b\\c' kind");
    ts.ok (value.string () == L"a\\b\\c", "'a\\b\\c' literal");
    std::string postmatch (de.iter, input.cend ());
    ts.ok (postmatch == " #\n", "'a\\b\\c' postmatch");
}

void
test15 (test::simple& ts)
{
    std::string input (
    u8"\"いろはに\" #\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    ts.ok (de.scan_string (value) == wjson::TOKEN_STRKEY, "\"いろはに\" kind");
    ts.ok (value.string () == L"いろはに", "\"いろはに\" literal");
    std::string postmatch (de.iter, input.cend ());
    ts.ok (postmatch == " #\n", "\"いろはに\" postmatch");
}

void
test16 (test::simple& ts)
{
    std::string input (
    u8"\"abc #\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    ts.ok (de.scan_string (value) == wjson::TOKEN_INVALID, "\"abc #\\n invalid");
}

int
main ()
{
    test::simple ts (46);
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
    return ts.done_testing ();
}
