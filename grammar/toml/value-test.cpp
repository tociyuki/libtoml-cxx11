#include <string>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <iostream>
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

    int scan_value (value_type& value);
    int scan_string (value_type& value);
    int scan_number (value_type& value);
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

#include "toml-decoder-value.cpp"
#include "toml-decoder-string.cpp"
#include "toml-decoder-number.cpp"

}//namespace wjson

#include "../../taptests.hpp"

void
test1 (test::simple& ts)
{
    std::string input (
    "'foo'\n"
    "bar='baz'\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    de.kvstate = 2;
    ts.ok (de.scan_value (value) == wjson::TOKEN_STRING, "'foo' scan");
    ts.ok (value.string () == L"foo", "foo string");
    std::string postmatch1 (de.iter, input.cend ());
    ts.ok (postmatch1 == "\nbar='baz'\n", "foo postmatch");
    ts.ok (de.kvstate == 2, "foo kvstate 1");

    ts.ok (de.scan_value (value) == wjson::TOKEN_ENDLINE, "endline scan");
    std::string postmatch2 (de.iter, input.cend ());
    ts.ok (postmatch2 == "bar='baz'\n", "= postmatch");
    ts.ok (de.kvstate == 2, "= kvstate 2");
}

void
test2 (test::simple& ts)
{
    std::string input (
    "['a','b']\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    de.kvstate = 2;
    ts.ok (de.scan_value (value) == wjson::TOKEN_LBRACKET, "[ scan");
    ts.ok (de.scan_value (value) == wjson::TOKEN_STRING, "'a' scan");
    ts.ok (value.string () == L"a", "a string");
    ts.ok (de.scan_value (value) == wjson::TOKEN_COMMA, ", scan");
    ts.ok (de.scan_value (value) == wjson::TOKEN_STRING, "'b' scan");
    ts.ok (value.string () == L"b", "b string");
    ts.ok (de.scan_value (value) == wjson::TOKEN_RBRACKET, "] scan");
}

void
test3 (test::simple& ts)
{
    std::string input (
    "# comment \n"
    "\n"
    "  # comment \n"
    "\n"
    "  'foo',\n"
    "\n"
    "# comment \n"
    "  'bar',\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    de.kvstate = 2;
    ts.ok (de.scan_value (value) == wjson::TOKEN_ENDLINE, "endline scan");
    ts.ok (de.scan_value (value) == wjson::TOKEN_STRING, "foo scan");
    ts.ok (value.string () == L"foo", "foo string");
    ts.ok (de.scan_value (value) == wjson::TOKEN_COMMA, ", scan");
    ts.ok (de.scan_value (value) == wjson::TOKEN_ENDLINE, "endline scan");
    ts.ok (de.scan_value (value) == wjson::TOKEN_STRING, "bar scan");
    ts.ok (value.string () == L"bar", "bar string");
}

void
test4 (test::simple& ts)
{
    std::string input (
    "{'foo'}\n"
    );
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    de.kvstate = 2;
    ts.ok (de.scan_value (value) == wjson::TOKEN_LBRACE, "{ scan");
    ts.ok (de.kvstate == 1, "{ kvstate 1");
    de.kvstate = 2;
    ts.ok (de.scan_value (value) == wjson::TOKEN_STRING, "foo scan");
    ts.ok (value.string () == L"foo", "foo string");
    ts.ok (de.scan_value (value) == wjson::TOKEN_RBRACE, "} scan");
    ts.ok (de.kvstate == 2, "} kvstate 2");
}

void
test5 (test::simple& ts)
{
    std::string input ("true , false\n");
    wjson::toml_decoder_type de (input);
    wjson::value_type value;
    de.kvstate = 2;
    ts.ok (de.scan_value (value) == wjson::TOKEN_BOOLEAN, "true scan");
    ts.ok (de.scan_value (value) == wjson::TOKEN_COMMA, ", scan");
    ts.ok (de.scan_value (value) == wjson::TOKEN_BOOLEAN, "false scan");
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
    return ts.done_testing ();
}
