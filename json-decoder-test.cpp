#include "json.hpp"
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
test_null (test::simple& ts)
{
    std::string input ("null");
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode null");
    ts.ok (got.tag () == wjson::VALUE_NULL, "json decode null null");
}

void
test_true (test::simple& ts)
{
    std::string input ("true");
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode true");
    ts.ok (got.tag () == wjson::VALUE_BOOLEAN, "json decode true boolean");
    ts.ok (got.boolean (), "json decode true value");
}

void
test_false (test::simple& ts)
{
    std::string input ("false");
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode false");
    ts.ok (got.tag () == wjson::VALUE_BOOLEAN, "json decode false boolean");
    ts.ok (! got.boolean (), "json decode false value");
}

void
test_fixnum_zero (test::simple& ts)
{
    std::string input ("0");
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode 0");
    ts.ok (got.tag () == wjson::VALUE_FIXNUM, "json decode 0 fixnum");
    ts.ok (got.fixnum () == 0, "json decode 0 value");
}

void
test_fixnum_one (test::simple& ts)
{
    std::string input ("1");
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode 1");
    ts.ok (got.tag () == wjson::VALUE_FIXNUM, "json decode 1 fixnum");
    ts.ok (got.fixnum () == 1, "json decode 1 value");
}

void
test_fixnum_negative_one (test::simple& ts)
{
    std::string input ("-1");
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode -1");
    ts.ok (got.tag () == wjson::VALUE_FIXNUM, "json decode -1 fixnum");
    ts.ok (got.fixnum () == -1, "json decode -1 value");
}

void
test_fixnum_max (test::simple& ts)
{
    int64_t const fixmax = std::numeric_limits<long long>::max ();
    std::string input = std::to_string (fixmax);
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode " + input);
    ts.ok (got.tag () == wjson::VALUE_FIXNUM, "json decode " + input + " fixnum");
    ts.ok (got.fixnum () == fixmax, "json decode " + input + " value");
}

void
test_fixnum_lowest (test::simple& ts)
{
    int64_t const fixlowest = std::numeric_limits<long long>::lowest ();
    std::string input = std::to_string (fixlowest);
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode " + input);
    ts.ok (got.tag () == wjson::VALUE_FIXNUM, "json decode " + input + " fixnum");
    ts.ok (got.fixnum () == fixlowest, "json decode " + input + " value");
}

void
test_flonum_zero (test::simple& ts)
{
    std::string input ("0.0");
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode 0.0");
    ts.ok (got.tag () == wjson::VALUE_FLONUM, "json decode 0.0 flonum");
    ts.ok (almost (got.flonum (), 0.0), "json decode 0.0 value");
}

void
test_flonum_one (test::simple& ts)
{
    std::string input ("1.0");
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode 1.0");
    ts.ok (got.tag () == wjson::VALUE_FLONUM, "json decode 1.0 flonum");
    ts.ok (almost (got.flonum (), 1.0), "json decode 1.0 value");
}

void
test_flonum_negative_one (test::simple& ts)
{
    std::string input ("-1.0");
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode -1.0");
    ts.ok (got.tag () == wjson::VALUE_FLONUM, "json decode -1.0 flonum");
    ts.ok (almost (got.flonum (), -1.0), "json decode -1.0 value");
}

void
test_flonum_pi (test::simple& ts)
{
    std::string input ("3.1415926535897932");
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode 3.1415926535897932");
    ts.ok (got.tag () == wjson::VALUE_FLONUM, "json decode 3.1415926535897932 flonum");
    ts.ok (almost (got.flonum (), 3.1415926535897932), "json decode 3.1415926535897932 value");
}

void
test_flonum_g (test::simple& ts)
{
    std::string input ("6.67259e-11");
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode 6.67259e-11");
    ts.ok (got.tag () == wjson::VALUE_FLONUM, "json decode 6.67259e-11 flonum");
    ts.ok (almost (got.flonum (), 6.67259e-11), "json decode 6.67259e-11 value");
}

void
test_flonum_e30 (test::simple& ts)
{
    std::string input ("123456789012345678901234567890");
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode 123456789012345678901234567890");
    ts.ok (got.tag () == wjson::VALUE_FLONUM,
        "json decode 123456789012345678901234567890 flonum");
    ts.ok (almost (got.flonum (), 123456789012345678901234567890.0),
        "json decode 123456789012345678901234567890 value");
}

void
test_flonum_out_of_range (test::simple& ts)
{
    std::string input ("1.0e2000");
    wjson::value_type got;
    ts.ok (! wjson::decode_json (input, got), "fail json decode 1.0e2000");
}

void
test_string_empty (test::simple& ts)
{
    std::string input (R"q("")q");
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode \"\"");
    ts.ok (got.tag () == wjson::VALUE_STRING, "json decode \"\" string");
    ts.ok (got.string () == L"", "json decode \"\" value");
}

void
test_string_ascii (test::simple& ts)
{
    std::string input (
    R"q("\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\b)q"
    R"q(\t\n\u000b\f\r\u000e\u000f)q"
    R"q(\u0010\u0011\u0012\u0013\u0014\u0015\u0016\u0017)q"
    R"q(\u0018\u0019\u001a\u001b\u001c\u001d\u001e\u001f)q"
    R"q( !\"#$%&'()*+,-.\/0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_)q"
    R"q(`abcdefghijklmnopqrstuvwxyz{|}~\u007f")q"
    );
    std::wstring expected;
    for (int c = 0; c < 128; ++c)
        expected.push_back (c);
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode ascii");
    ts.ok (got.tag () == wjson::VALUE_STRING, "json decode ascii string");
    ts.ok (got.string () == expected, "json decode ascii value");
}

void
test_string_mbyte (test::simple& ts)
{
    std::string input (u8R"q("いろはに")q");
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode mbyte");
    ts.ok (got.tag () == wjson::VALUE_STRING, "json decode mbyte string");
    ts.ok (got.string () == L"いろはに", "json decode mbyte value");
}

void
test_string_surrogate_pair (test::simple& ts)
{
    std::string input (R"q("\u0033\u0020\uD834\uDD1E")q");
    std::wstring expected (L"3 \U0001d11e");
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode surrogate");
    ts.ok (got.string () == expected, "json decode surrogate");
}

void
test_array_empty (test::simple& ts)
{
    std::string input (R"q([])q");
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode []");
    ts.ok (got.tag () == wjson::VALUE_ARRAY, "json decode [] array");
    ts.ok (got.size () == 0, "json decode [] size");
}

void
test_array_flat (test::simple& ts)
{
    std::string input (R"q([false,1,2.0,"2015-10-18T01:32:57Z","four"])q");
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode array flat");
    ts.ok (got.tag () == wjson::VALUE_ARRAY, "json decode array flat array");
    ts.ok (got.size () == 5, "json decode array flat size");
    ts.ok (got[0].tag () == wjson::VALUE_BOOLEAN,
        "json decode array flat [0] boolean");
    ts.ok (! got[0].boolean (), "json decode array flat [0] false");
    ts.ok (got[1].fixnum () == 1, "json decode array flat [1] 1");
    ts.ok (almost (got[2].flonum (), 2.0), "json decode array flat [2] 2.0");
    ts.ok (got[3].string () == L"2015-10-18T01:32:57Z",
        "json decode array flat [3] 2015-10-18T01:32:57Z");
    ts.ok (got[4].string () == L"four", "json decode array flat [4] four");
}

void
test_array_nest (test::simple& ts)
{
    std::string input (R"q([["lambda",["x","y"],["cons","x","y"]],"a",["b","c"]])q");
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode array nest");
    ts.ok (got[0][0].string () == L"lambda", "json decode array nest [0][0]");
    ts.ok (got[0][1][0].string () == L"x", "json decode array nest [0][1][0]");
    ts.ok (got[0][1][1].string () == L"y", "json decode array nest [0][1][1]");
    ts.ok (got[0][2][0].string () == L"cons", "json decode array nest [0][2][0]");
    ts.ok (got[0][2][1].string () == L"x", "json decode array nest [0][2][1]");
    ts.ok (got[0][2][2].string () == L"y", "json decode array nest [0][2][2]");
    ts.ok (got[1].string () == L"a", "json decode array nest [1]");
    ts.ok (got[2][0].string () == L"b", "json decode array nest [2][0]");
    ts.ok (got[2][1].string () == L"c", "json decode array nest [2][1]");
}

void
test_table_empty (test::simple& ts)
{
    std::string input (R"q({})q");
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode {}");
    ts.ok (got.tag () == wjson::VALUE_TABLE, "json decode {} table");
    ts.ok (got.size () == 0, "json decode {} size");
}

void
test_table_flat (test::simple& ts)
{
    std::string input (
    R"q({"bool":false,"datetime":"2015-10-18T01:32:57Z",)q"
    R"q("fixnum":1,"flonum":2.0,"string":"four"})q"
    );
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode table flat");
    ts.ok (got.size () == 5, "json decode table flat size");
    ts.ok (! got[L"bool"].boolean (), "json decode table flat [bool]");
    ts.ok (got[L"datetime"].string () == L"2015-10-18T01:32:57Z",
        "json decode table flat [datetime]");
    ts.ok (got[L"fixnum"].fixnum () == 1, "json decode table flat [fixnum]");
    ts.ok (almost (got[L"flonum"].flonum (), 2.0),
        "json decode table flat [flonum]");
    ts.ok (got[L"string"].string () == L"four", "json decode table flat [string]");
}

void
test_table_nest (test::simple& ts)
{
    std::string input (
    R"q({"a":{"a0":"A0","a1":"A1"},"b":"B",)q"
    R"q("c":{"c0":{"c00":"C0"},"c1":{"c00":"C1"}}})q"
    );
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode table nest");
    ts.ok (got[L"a"][L"a0"].string () == L"A0", "json decode table nest [a][a0]");
    ts.ok (got[L"a"][L"a1"].string () == L"A1", "json decode table nest [a][a1]");
    ts.ok (got[L"b"].string () == L"B", "json decode table nest [b]");
    ts.ok (got[L"c"][L"c0"][L"c00"].string () == L"C0",
        "json decode table nest [c][c0][c00]");
    ts.ok (got[L"c"][L"c1"][L"c00"].string () == L"C1",
        "json decode table nest [c][c1][c00]");
}

void
test_table_fluit (test::simple& ts)
{
    std::string input (
R"q({
  "fruit": [
    {
      "name": "apple",
      "physical": {
        "color": "red",
        "shape": "round"
      },
      "variety": [
        {
          "name": "red delicious"
        },
        {
          "name": "granny smith"
        }
      ]
    },
    {
      "name": "banana",
      "variety": [
        {
          "name": "plantain"
        }
      ]
    }
  ]
})q"
    );
    wjson::value_type got;
    ts.ok (wjson::decode_json (input, got), "json decode fruit");
    ts.ok (got[L"fruit"][0][L"name"].string () == L"apple",
        "json decode [fruit][0][name]");
    ts.ok (got[L"fruit"][0][L"physical"][L"color"].string () == L"red",
        "json decode [fruit][0][physical][color]");
    ts.ok (got[L"fruit"][0][L"physical"][L"shape"].string () == L"round",
        "json decode [fruit][0][physical][shape]");
    ts.ok (got[L"fruit"][0][L"variety"][0][L"name"].string () == L"red delicious",
        "json decode [fruit][0][variety][0][name]");
    ts.ok (got[L"fruit"][0][L"variety"][1][L"name"].string () == L"granny smith",
        "json decode [fruit][0][variety][1][name]");
    ts.ok (got[L"fruit"][1][L"name"].string () == L"banana",
        "json decode [fruit][1][name]");
    ts.ok (got[L"fruit"][1][L"variety"][0][L"name"].string () == L"plantain",
        "json decode [fruit][1][variety][0][name]");
}

int main ()
{
    test::simple ts (99);

    test_null (ts);
    test_true (ts);
    test_false (ts);
    test_fixnum_zero (ts);
    test_fixnum_one (ts);
    test_fixnum_negative_one (ts);
    test_fixnum_max (ts);
    test_fixnum_lowest (ts);
    test_flonum_zero (ts);
    test_flonum_one (ts);
    test_flonum_negative_one (ts);
    test_flonum_pi (ts);
    test_flonum_g (ts);
    test_flonum_e30 (ts);
    test_flonum_out_of_range (ts);
    test_string_empty (ts);
    test_string_ascii (ts);
    test_string_mbyte (ts);
    test_string_surrogate_pair (ts);
    test_array_empty (ts);
    test_array_flat (ts);
    test_array_nest (ts);
    test_table_empty (ts);
    test_table_flat (ts);
    test_table_nest (ts);
    test_table_fluit (ts);

    return ts.done_testing ();
}
