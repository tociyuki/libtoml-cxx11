#include <string>
#include <cmath>
#include <limits>
#include <stdexcept>
#include "../../value.hpp"
#include "../../encode-quoted.hpp"

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
    int kvstate;
    std::string const& string;
    std::string::const_iterator iter;
    toml_decoder_type (std::string const& x) : kvstate (0), string (x), iter (x.cbegin ()) {}

    int scan_key (value_type& value);
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

#include "toml-decoder-key.cpp"
#include "toml-decoder-string.cpp"

}//namespace wjson

#include "../../taptests.hpp"

void
test1 (test::simple& ts)
{
    std::string input (
    "foo=\"bar\"\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    ts.ok (de.scan_key (value) == wjson::TOKEN_BAREKEY, "foo scan");
    ts.ok (value.string () == L"foo", "foo string");
    std::string postmatch1 (de.iter, input.cend ());
    ts.ok (postmatch1 == "=\"bar\"\n", "foo postmatch");
    ts.ok (de.kvstate == 1, "foo kvstate 1");

    ts.ok (de.scan_key (value) == wjson::TOKEN_EQUAL, "= scan");
    std::string postmatch2 (de.iter, input.cend ());
    ts.ok (postmatch2 == "\"bar\"\n", "= postmatch");
    ts.ok (de.kvstate == 2, "= kvstate 2");
}

void
test2 (test::simple& ts)
{
    std::string input (
    "# comment \n"
    "\n"
    "  # comment \n"
    "\n"
    "  foo=\"bar\"\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    ts.ok (de.scan_key (value) == wjson::TOKEN_BAREKEY, "foo scan");
    ts.ok (value.string () == L"foo", "foo string");
    std::string postmatch1 (de.iter, input.cend ());
    ts.ok (postmatch1 == "=\"bar\"\n", "foo postmatch");
    ts.ok (de.kvstate == 1, "foo kvstate 1");

    ts.ok (de.scan_key (value) == wjson::TOKEN_EQUAL, "= scan");
    std::string postmatch2 (de.iter, input.cend ());
    ts.ok (postmatch2 == "\"bar\"\n", "= postmatch");
    ts.ok (de.kvstate == 2, "= kvstate 2");
}

void
test3 (test::simple& ts)
{
    std::string input (
    "# comment \n"
    "\n"
    "  # comment \n"
    "\n"
    "  foo=\"bar\"\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    de.kvstate = 1;
    ts.ok (de.scan_key (value) == wjson::TOKEN_ENDLINE, "endline scan");
    std::string postmatch1 (de.iter, input.cend ());
    ts.ok (postmatch1 == "foo=\"bar\"\n", "endline postmatch");
    ts.ok (de.kvstate == 1, "endline kvstate 1");

    ts.ok (de.scan_key (value) == wjson::TOKEN_BAREKEY, "foo scan");
    ts.ok (value.string () == L"foo", "foo string");
    std::string postmatch2 (de.iter, input.cend ());
    ts.ok (postmatch2 == "=\"bar\"\n", "foo postmatch");
    ts.ok (de.kvstate == 1, "foo kvstate 1");

    ts.ok (de.scan_key (value) == wjson::TOKEN_EQUAL, "= scan");
    std::string postmatch3 (de.iter, input.cend ());
    ts.ok (postmatch3 == "\"bar\"\n", "= postmatch");
    ts.ok (de.kvstate == 2, "= kvstate 2");
}

void
test4 (test::simple& ts)
{
    std::string input (
    "# \n"
    "  [foo.bar] # comment\n"
    "\n"
    "# comment \n"
    "    baz=\"buzz\"\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    ts.ok (de.scan_key (value) == wjson::TOKEN_LBRACKET, "[ scan");
    ts.ok (de.kvstate == 1, "[ kvstate 1");
    ts.ok (de.scan_key (value) == wjson::TOKEN_BAREKEY, "foo scan");
    ts.ok (value.string () == L"foo", "foo string");
    ts.ok (de.scan_key (value) == wjson::TOKEN_DOT, ". scan");
    ts.ok (de.scan_key (value) == wjson::TOKEN_BAREKEY, "bar scan");
    ts.ok (value.string () == L"bar", "bar string");
    ts.ok (de.scan_key (value) == wjson::TOKEN_RBRACKET, "] scan");
    ts.ok (de.kvstate == 1, "[foo.bar] kvstate 1");
    ts.ok (de.scan_key (value) == wjson::TOKEN_ENDLINE, "endline scan");
    ts.ok (de.scan_key (value) == wjson::TOKEN_BAREKEY, "baz scan");
    ts.ok (value.string () == L"baz", "baz string");
    ts.ok (de.scan_key (value) == wjson::TOKEN_EQUAL, "= scan");
}

void
test5 (test::simple& ts)
{
    std::string input ("1234 = \"bare integer\"\n");
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    ts.ok (de.scan_key (value) == wjson::TOKEN_BAREKEY, "1234 scan barekey");
    ts.ok (value.string () == L"1234", "1234 scan");
}

void
test6 (test::simple& ts)
{
    std::string input ("\"127.0.0.1\" = \"value\"\n");
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    ts.ok (de.scan_key (value) == wjson::TOKEN_STRKEY, "scan 127.0.0.1 strkey");
    ts.ok (value.string () == L"127.0.0.1", "127.0.0.1 scan");
}

int
main ()
{
    test::simple ts;
    test1 (ts);
    test2 (ts);
    test3 (ts);
    test4 (ts);
    test5 (ts);
    test6 (ts);
    return ts.done_testing ();
}
