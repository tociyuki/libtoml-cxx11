#include "toml.hpp"
#include "taptests.hpp"
#include <limits>
#include <cmath>
#include <cstdio>

bool
almost (double x, double y)
{
    return std::abs (x - y) <=
        std::numeric_limits<double>::epsilon ()
            * std::min (std::abs (x), std::abs (y));
}

void
test_comment (test::simple& ts)
{
    std::string input (
R"q(# This is a full-line comment
key = "value" # This is a comment at the end of a line
)q");
    wjson::value_type got;
    ts.ok (wjson::decode_toml (input, got), "toml decode comment");
    ts.ok (got.size () == 1, "toml decode comment size 1");
    ts.ok (got[L"key"].string () == L"value", "toml decode key=value");
}

void
test_string_1 (test::simple& ts)
{
    std::string input (
R"q(str = "I'm a string. \"You can quote me\". Name\tJos\u00E9\nLocation\tSF."
)q");
    std::wstring expected (L"I'm a string. \"You can quote me\". Name\tJos\u00E9\nLocation\tSF.");
    wjson::value_type got;
    ts.ok (wjson::decode_toml (input, got), "toml decode string_1");
    ts.ok (got.size () == 1, "toml decode string_1 size 1");
    ts.ok (got[L"str"].string () == expected, "toml decode string_1");
}

void
test_string_2 (test::simple& ts)
{
    std::string input (
R"q(str1 = """
Roses are red
Violets are blue"""
)q");
    std::wstring expected (L"Roses are red\nViolets are blue");
    wjson::value_type got;
    ts.ok (wjson::decode_toml (input, got), "toml decode string_2");
    ts.ok (got.size () == 1, "toml decode string_2 size 1");
    ts.ok (got[L"str1"].string () == expected, "toml decode string_2");
}

void
test_string_3 (test::simple& ts)
{
    std::string input (
R"q(# The following strings are byte-for-byte equivalent:
str1 = "The quick brown fox jumps over the lazy dog."

str2 = """
The quick brown \


  fox jumps over \
    the lazy dog."""

key3 = """\
       The quick brown \
       fox jumps over \
       the lazy dog.\
       """
)q");
    std::wstring expected (L"The quick brown fox jumps over the lazy dog.");
    wjson::value_type got;
    ts.ok (wjson::decode_toml (input, got), "toml decode string_3");
    ts.ok (got.size () == 3, "toml decode string_3 size 3");
    ts.ok (got[L"str1"].string () == expected, "toml decode string_3 str1");
    ts.ok (got[L"str2"].string () == expected, "toml decode string_3 str2");
    ts.ok (got[L"key3"].string () == expected, "toml decode string_3 key3");
}

void
test_string_4 (test::simple& ts)
{
    std::string input (
R"q(# What you see is what you get.
winpath  = 'C:\Users\nodejs\templates'
winpath2 = '\\ServerX\admin$\system32\'
quoted   = 'Tom "Dubs" Preston-Werner'
regex    = '<\i\c*\s*>'
)q");
    wjson::value_type got;
    ts.ok (wjson::decode_toml (input, got), "toml decode string_4");
    ts.ok (got.size () == 4, "toml decode string_4 size 4");
    ts.ok (got[L"winpath"].string () == LR"q(C:\Users\nodejs\templates)q",
        "toml decode string_4 winpath");
    ts.ok (got[L"winpath2"].string () == LR"q(\\ServerX\admin$\system32\)q",
        "toml decode string_4 winpath2");
    ts.ok (got[L"quoted"].string () == LR"q(Tom "Dubs" Preston-Werner)q",
        "toml decode string_4 quoted");
    ts.ok (got[L"regex"].string () == LR"q(<\i\c*\s*>)q",
        "toml decode string_4 regex");
}

void
test_string_5 (test::simple& ts)
{
    std::string input (
R"q(regex2 = '''I [dw]on't need \d{2} apples'''
lines  = '''
The first newline is
trimmed in raw strings.
   All other whitespace
   is preserved.
'''
)q");
    std::wstring expected_lines (
LR"q(The first newline is
trimmed in raw strings.
   All other whitespace
   is preserved.
)q"
);
    wjson::value_type got;
    ts.ok (wjson::decode_toml (input, got), "toml decode string_5");
    ts.ok (got.size () == 2, "toml decode string_5 size 2");
    ts.ok (got[L"regex2"].string () == LR"q(I [dw]on't need \d{2} apples)q",
        "toml decode string_5 regex2");
    ts.ok (got[L"lines"].string () == expected_lines,
        "toml decode string_5 lines");
}

void
test_integer_1 (test::simple& ts)
{
    std::string input (
R"q(int1 = +99
int2 = 42
int3 = 0
int4 = -17
)q");
    wjson::value_type got;
    ts.ok (wjson::decode_toml (input, got), "toml decode integer_1");
    ts.ok (got.size () == 4, "toml decode integer_1 size 4");
    ts.ok (got[L"int1"].fixnum () == 99LL, "toml decode integer_1 int1");
    ts.ok (got[L"int2"].fixnum () == 42LL, "toml decode integer_1 int2");
    ts.ok (got[L"int3"].fixnum () == 0LL, "toml decode integer_1 int3");
    ts.ok (got[L"int4"].fixnum () == -17LL, "toml decode integer_1 int4");
}

void
test_integer_2 (test::simple& ts)
{
    std::string input (
R"q(int5 = 1_000
int6 = 5_349_221
int7 = 1_2_3_4_5     # valid but inadvisable
)q");
    wjson::value_type got;
    ts.ok (wjson::decode_toml (input, got), "toml decode integer_2");
    ts.ok (got.size () == 3, "toml decode integer_2 size 3");
    ts.ok (got[L"int5"].fixnum () == 1000LL, "toml decode integer_2 int5");
    ts.ok (got[L"int6"].fixnum () == 5349221LL, "toml decode integer_2 int6");
    ts.ok (got[L"int7"].fixnum () == 12345LL, "toml decode integer_2 int7");
}

void
test_float (test::simple& ts)
{
    std::string input (
R"q(# fractional
flt1 = +1.0
flt2 = 3.1415
flt3 = -0.01

# exponent
flt4 = 5e+22
flt5 = 1e6
flt6 = -2E-2

# both
flt7 = 6.626e-34

flt8 = 9_224_617.445_991_228_313
flt9 = 1e1_00  # change due to 1e1_000 out_of_range
)q");
    wjson::value_type got;
    ts.ok (wjson::decode_toml (input, got), "toml decode float_1");
    ts.ok (got.size () == 9, "toml decode float_1 size 9");
    ts.ok (almost (got[L"flt1"].flonum (), 1.0), "toml decode float_1 flt1");
    ts.ok (almost (got[L"flt2"].flonum (), 3.1415), "toml decode float_1 flt2");
    ts.ok (almost (got[L"flt3"].flonum (), -0.01), "toml decode float_1 flt3");
    ts.ok (almost (got[L"flt4"].flonum (), 5.0e+22), "toml decode float_1 flt4");
    ts.ok (almost (got[L"flt5"].flonum (), 1.0e6), "toml decode float_1 flt5");
    ts.ok (almost (got[L"flt6"].flonum (), -2.0E-2), "toml decode float_1 flt6");
    ts.ok (almost (got[L"flt7"].flonum (), 6.626e-34), "toml decode float_1 flt7");
    ts.ok (almost (got[L"flt8"].flonum (), 9224617.445991228313), "toml decode float_1 flt8");
    ts.ok (almost (got[L"flt9"].flonum (), 1.0e100), "toml decode float_1 flt9");
}

void
test_boolean (test::simple& ts)
{
    std::string input (
R"q(bool1 = true
bool2 = false
)q");
    wjson::value_type got;
    ts.ok (wjson::decode_toml (input, got), "toml decode boolean");
    ts.ok (got.size () == 2, "toml decode boolean size 2");
    ts.ok (got[L"bool1"].boolean (), "toml decode boolean bool1");
    ts.ok (! got[L"bool2"].boolean (), "toml decode boolean bool2");
}

void
test_datetime (test::simple& ts)
{
    std::string input (
R"q(date1 = 1979-05-27T07:32:00Z
date2 = 1979-05-27T00:32:00-07:00
date3 = 1979-05-27T00:32:00.999999-07:00
date4 = 1979-05-27T07:32:00
date5 = 1979-05-27T00:32:00.999999
date6 = 1979-05-27
)q");
    wjson::value_type got;
    ts.ok (wjson::decode_toml (input, got), "toml decode datetime");
    ts.ok (got.size () == 6, "toml decode datetime size 6");
    ts.ok (got[L"date1"].datetime () == L"1979-05-27T07:32:00Z",
        "toml decode datetime date1");
    ts.ok (got[L"date2"].datetime () == L"1979-05-27T00:32:00-07:00",
        "toml decode datetime date2");
    ts.ok (got[L"date3"].datetime () == L"1979-05-27T00:32:00.999999-07:00",
        "toml decode datetime date3");
    ts.ok (got[L"date4"].datetime () == L"1979-05-27T07:32:00",
        "toml decode datetime date4");
    ts.ok (got[L"date5"].datetime () == L"1979-05-27T00:32:00.999999",
        "toml decode datetime date5");
    ts.ok (got[L"date6"].datetime () == L"1979-05-27",
        "toml decode datetime date6");
}

void
test_array_1 (test::simple& ts)
{
    std::string input (
R"q(arr1 = [ 1, 2, 3 ]
)q");
    wjson::value_type got;
    ts.ok (wjson::decode_toml (input, got), "toml decode array_1");
    ts.ok (got[L"arr1"].tag () == wjson::VALUE_ARRAY, "toml decode array_1 type");
    ts.ok (got[L"arr1"][0].fixnum () == 1, "toml decode array_1 [arr1][0]");
    ts.ok (got[L"arr1"][1].fixnum () == 2, "toml decode array_1 [arr1][1]");
    ts.ok (got[L"arr1"][2].fixnum () == 3, "toml decode array_1 [arr1][2]");
}

void
test_array_2 (test::simple& ts)
{
    std::string input (
R"q(arr2 = [ "red", "yellow", "green" ]
)q");
    wjson::value_type got;
    ts.ok (wjson::decode_toml (input, got), "toml decode array_2");
    ts.ok (got[L"arr2"].tag () == wjson::VALUE_ARRAY, "toml decode array_2 type");
    ts.ok (got[L"arr2"][0].string () == L"red", "toml decode array_2 [arr2][0]");
    ts.ok (got[L"arr2"][1].string () == L"yellow", "toml decode array_2 [arr2][1]");
    ts.ok (got[L"arr2"][2].string () == L"green", "toml decode array_2 [arr2][2]");
}

void
test_array_3 (test::simple& ts)
{
    std::string input (
R"q(arr3 = [ [ 1, 2 ], [3, 4, 5] ]
)q");
    wjson::value_type got;
    ts.ok (wjson::decode_toml (input, got), "toml decode array_3");
    ts.ok (got[L"arr3"].tag () == wjson::VALUE_ARRAY, "toml decode array_3 type");
    ts.ok (got[L"arr3"][0][0].fixnum () == 1, "toml decode array_3 [arr3][0][0]");
    ts.ok (got[L"arr3"][0][1].fixnum () == 2, "toml decode array_3 [arr3][0][1]");
    ts.ok (got[L"arr3"][1][0].fixnum () == 3, "toml decode array_3 [arr3][1][0]");
    ts.ok (got[L"arr3"][1][1].fixnum () == 4, "toml decode array_3 [arr3][1][1]");
    ts.ok (got[L"arr3"][1][2].fixnum () == 5, "toml decode array_3 [arr3][1][2]");
}

void
test_array_4 (test::simple& ts)
{
    std::string input (
R"q(arr4 = [ "all", 'strings', """are the same""", '''type''']
)q");
    wjson::value_type got;
    ts.ok (wjson::decode_toml (input, got), "toml decode array_4");
    ts.ok (got[L"arr4"].tag () == wjson::VALUE_ARRAY, "toml decode array_4 type");
    ts.ok (got[L"arr4"][0].string () == L"all", "toml decode array_4 [arr4][0]");
    ts.ok (got[L"arr4"][1].string () == L"strings", "toml decode array_4 [arr4][1]");
    ts.ok (got[L"arr4"][2].string () == L"are the same", "toml decode array_4 [arr4][2]");
    ts.ok (got[L"arr4"][3].string () == L"type", "toml decode array_4 [arr4][3]");
}

void
test_array_5 (test::simple& ts)
{
    std::string input (
R"q(arr7 = [
  1, 2, 3
]

arr8 = [
  1,
  2, # this is ok
]
)q");
    wjson::value_type got;
    ts.ok (wjson::decode_toml (input, got), "toml decode array_5");
    ts.ok (got[L"arr7"].tag () == wjson::VALUE_ARRAY, "toml decode array_5 type");
    ts.ok (got[L"arr7"][0].fixnum () == 1, "toml decode array_5 [arr7][0]");
    ts.ok (got[L"arr7"][1].fixnum () == 2, "toml decode array_5 [arr7][1]");
    ts.ok (got[L"arr7"][2].fixnum () == 3, "toml decode array_5 [arr7][2]");
    ts.ok (got[L"arr8"][0].fixnum () == 1, "toml decode array_5 [arr8][0]");
    ts.ok (got[L"arr8"][1].fixnum () == 2, "toml decode array_5 [arr8][1]");
}

void
test_table_1 (test::simple& ts)
{
    std::string input (
u8R"q([table]
key = "value"
bare_key = "value"
bare-key = "value"
1234 = "bare integer"

"127.0.0.1" = "value"
"character encoding" = "value"
"ʎǝʞ" = "value"
)q");
    wjson::value_type got;
    ts.ok (wjson::decode_toml (input, got), "toml decode table_1");
    ts.ok (got[L"table"].tag () == wjson::VALUE_TABLE, "toml decode table_1 type");
    ts.ok (got[L"table"][L"key"].string () == L"value",
        "toml decode table_1 [table][key]");
    ts.ok (got[L"table"][L"bare_key"].string () == L"value",
        "toml decode table_1 [table][bare_key]");
    ts.ok (got[L"table"][L"bare-key"].string () == L"value",
        "toml decode table_1 [table][bare-key]");
    ts.ok (got[L"table"][L"1234"].string () == L"bare integer",
        "toml decode table_1 [table][1234]");
    ts.ok (got[L"table"][L"127.0.0.1"].string () == L"value",
        "toml decode table_1 [table][127.0.0.1]");
    ts.ok (got[L"table"][L"character encoding"].string () == L"value",
        "toml decode table_1 [table][character encodin]");
    ts.ok (got[L"table"][L"ʎǝʞ"].string () == L"value",
        u8"toml decode table_1 [table][ʎǝʞ]");
}

void
test_table_2 (test::simple& ts)
{
    std::string input (
R"q([dog."tater.man"]
type = "pug"
)q");
    wjson::value_type got;
    ts.ok (wjson::decode_toml (input, got), "toml decode table_2");
    ts.ok (got[L"dog"].tag () == wjson::VALUE_TABLE, "toml decode table_2 type");
    ts.ok (got[L"dog"][L"tater.man"][L"type"].string () == L"pug",
        "toml decode table_2 [dog][tater.man][type]");
}

void
test_table_3 (test::simple& ts)
{
    std::string input (
R"q([a.b]
c = 1

[a]
d = 2
)q");
    wjson::value_type got;
    ts.ok (wjson::decode_toml (input, got), "toml decode table_3");
    ts.ok (got[L"a"].tag () == wjson::VALUE_TABLE, "toml decode table_3 type");
    ts.ok (got[L"a"][L"b"][L"c"].fixnum () == 1,
        "toml decode table_3 [a][b][c]");
}

void
test_inline_table (test::simple& ts)
{
    std::string input (
R"q(name = { first = "Tom", last = "Preston-Werner" }
point = { x = 1, y = 2 }
)q");
    wjson::value_type got;
    ts.ok (wjson::decode_toml (input, got), "toml decode inline_table");
    ts.ok (got[L"name"].tag () == wjson::VALUE_TABLE, "toml decode inline_table type");
    ts.ok (got[L"name"][L"first"].string () == L"Tom",
        "toml decode inline_table [name][first]");
    ts.ok (got[L"name"][L"last"].string () == L"Preston-Werner",
        "toml decode inline_table [name][last]");
    ts.ok (got[L"point"][L"x"].fixnum () == 1,
        "toml decode inline_table [point][x]");
    ts.ok (got[L"point"][L"y"].fixnum () == 2,
        "toml decode inline_table [point][y]");
}

void
test_array_of_table_1 (test::simple& ts)
{
    std::string input (
R"q([[products]]
name = "Hammer"
sku = 738594937

[[products]]

[[products]]
name = "Nail"
sku = 284758393
color = "gray"
)q");
    wjson::value_type got;
    ts.ok (wjson::decode_toml (input, got), "toml decode array_of_table_1");
    ts.ok (got[L"products"][0][L"name"].string () == L"Hammer",
        "toml decode array_of_table_1e [products][0][name]");
    ts.ok (got[L"products"][0][L"sku"].fixnum () == 738594937,
        "toml decode array_of_table_1 [products][0][sku]");
    ts.ok (got[L"products"][1].tag () == wjson::VALUE_TABLE,
        "toml decode array_of_table_1 [products][1]");
    ts.ok (got[L"products"][2][L"name"].string () == L"Nail",
        "toml decode array_of_table_1 [products][2][name]");
    ts.ok (got[L"products"][2][L"sku"].fixnum () == 284758393,
        "toml decode array_of_table_1 [products][2][sku]");
    ts.ok (got[L"products"][2][L"color"].string () == L"gray",
        "toml decode array_of_table_1 [products][2][color]");
}

void
test_array_of_table_2 (test::simple& ts)
{
    std::string input (
R"q([[fruit]]
  name = "apple"

  [fruit.physical]
    color = "red"
    shape = "round"

  [[fruit.variety]]
    name = "red delicious"

  [[fruit.variety]]
    name = "granny smith"

[[fruit]]
  name = "banana"

  [[fruit.variety]]
    name = "plantain"
)q");
    wjson::value_type got;
    ts.ok (wjson::decode_toml (input, got), "toml decode array_of_table_2");
    ts.ok (got[L"fruit"][0][L"name"].string () == L"apple",
        "toml decode array_of_table_2 [fruit][0][name]");
    ts.ok (got[L"fruit"][0][L"physical"][L"color"].string () == L"red",
        "toml decode array_of_table_2 [fruit][0][physical][color]");
    ts.ok (got[L"fruit"][0][L"physical"][L"shape"].string () == L"round",
        "toml decode array_of_table_2 [fruit][0][physical][shape]");
    ts.ok (got[L"fruit"][0][L"variety"][0][L"name"].string () == L"red delicious",
        "toml decode array_of_table_2 [fruit][0][variety][0][name]");
    ts.ok (got[L"fruit"][0][L"variety"][1][L"name"].string () == L"granny smith",
        "toml decode array_of_table_2 [fruit][0][variety][1][name]");
    ts.ok (got[L"fruit"][1][L"name"].string () == L"banana",
        "toml decode array_of_table_2 [fruit][1][name]");
    ts.ok (got[L"fruit"][1][L"variety"][0][L"name"].string () == L"plantain",
        "toml decode array_of_table_2 [fruit][1][variety][0][name]");
}

int main ()
{
    test::simple ts (124);
    test_comment (ts);
    test_string_1 (ts);
    test_string_2 (ts);
    test_string_3 (ts);
    test_string_4 (ts);
    test_string_5 (ts);
    test_integer_1 (ts);
    test_integer_2 (ts);
    test_float (ts);
    test_boolean (ts);
    test_datetime (ts);
    test_array_1 (ts);
    test_array_2 (ts);
    test_array_3 (ts);
    test_array_4 (ts);
    test_array_5 (ts);
    test_table_1 (ts);
    test_table_2 (ts);
    test_table_3 (ts);
    test_inline_table (ts);
    test_array_of_table_1 (ts);
    test_array_of_table_2 (ts);
    return ts.done_testing ();
}
