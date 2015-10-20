#include "json.hpp"
#include "taptests.hpp"
#include <sstream>
#include <limits>
#include <cstdio>

void
test_null (test::simple& ts)
{
    wjson::value_type input = wjson::null ();
    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == "null", "json encode null");
}

void
test_true (test::simple& ts)
{
    wjson::value_type input = wjson::boolean (true);
    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == "true", "json encode true");
}

void
test_false (test::simple& ts)
{
    wjson::value_type input = wjson::boolean (false);
    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == "false", "json encode false");
}

void
test_fixnum_zero (test::simple& ts)
{
    wjson::value_type input = wjson::fixnum (0);
    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == "0", "json encode 0");
}

void
test_fixnum_one (test::simple& ts)
{
    wjson::value_type input = wjson::fixnum (1);
    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == "1", "json encode 1");
}

void
test_fixnum_negative_one (test::simple& ts)
{
    wjson::value_type input = wjson::fixnum (-1);
    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == "-1", "json encode -1");
}

void
test_fixnum_max (test::simple& ts)
{
    int64_t const fixmax = std::numeric_limits<long long>::max ();
    wjson::value_type input = wjson::fixnum (fixmax);
    std::string expected = std::to_string (fixmax);

    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == expected, "json encode " + expected);
}

void
test_fixnum_lowest (test::simple& ts)
{
    int64_t const fixlowest = std::numeric_limits<long long>::lowest ();
    wjson::value_type input = wjson::fixnum (fixlowest);
    std::string expected = std::to_string (fixlowest);

    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == expected, "json encode " + expected);
}

void
test_flonum_zero (test::simple& ts)
{
    wjson::value_type input = wjson::flonum (0.0);
    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == "0.0", "json encode 0.0");
}

void
test_flonum_one (test::simple& ts)
{
    wjson::value_type input = wjson::flonum (1.0);
    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == "1.0", "json encode 1.0");
}

void
test_flonum_negative_one (test::simple& ts)
{
    wjson::value_type input = wjson::flonum (-1.0);
    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == "-1.0", "json encode -1.0");
}

void
test_flonum_max (test::simple& ts)
{
    double const flomax = std::numeric_limits<double>::max ();
    wjson::value_type input = wjson::flonum (flomax);

    char buf[32];
    std::snprintf (buf, sizeof(buf)/sizeof(buf[0]), "%.15g", flomax);
    std::string expected (buf);

    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == expected, "json encode " + expected);
}

void
test_flonum_lowest (test::simple& ts)
{
    double const flolowest = std::numeric_limits<double>::lowest ();
    wjson::value_type input = wjson::flonum (flolowest);

    char buf[32];
    std::snprintf (buf, sizeof(buf)/sizeof(buf[0]), "%.15g", flolowest);
    std::string expected (buf);

    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == expected, "json encode " + expected);
}

void
test_datetime (test::simple& ts)
{
    wjson::value_type input = wjson::datetime (L"2015-10-18T01:02:45Z");
    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == "\"2015-10-18T01:02:45Z\"", "json encode 2015-10-18T01:02:45Z");
}

void
test_string_empty (test::simple& ts)
{
    wjson::value_type input = wjson::string (L"");
    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == "\"\"", "json encode string \"\"");
}

void
test_string_ascii (test::simple& ts)
{
    std::wstring ascii;
    for (int c = 0; c < 128; ++c)
        ascii.push_back (c);
    wjson::value_type input = wjson::string (ascii);
    std::string expected (
    R"q("\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\b)q"
    R"q(\t\n\u000b\f\r\u000e\u000f)q"
    R"q(\u0010\u0011\u0012\u0013\u0014\u0015\u0016\u0017)q"
    R"q(\u0018\u0019\u001a\u001b\u001c\u001d\u001e\u001f)q"
    R"q( !\"#$%&'()*+,-.\/0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_)q"
    R"q(`abcdefghijklmnopqrstuvwxyz{|}~\u007f")q"
    );

    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == expected, "json encode string ascii");
}

void
test_string_mbyte (test::simple& ts)
{
    wjson::value_type input = wjson::string (L"いろはに");
    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == u8"\"いろはに\"", "json encode string mbyte");
}

void
test_array_empty (test::simple& ts)
{
    wjson::value_type input = wjson::array ();
    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == "[]", "json encode array []");
}

void
test_array_flat (test::simple& ts)
{
    wjson::value_type input = wjson::array ();
    input[0] = wjson::boolean (false);
    input[1] = wjson::fixnum (1);
    input[2] = wjson::flonum (2.0);
    input[3] = wjson::datetime (L"2015-10-18T01:32:57Z");
    input[4] = L"four";
    std::string expected (R"q([false,1,2.0,"2015-10-18T01:32:57Z","four"])q");
    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == expected, "json encode array flat");
}

void
test_array_nest (test::simple& ts)
{
    wjson::value_type input = wjson::array ();
    input[0][0] = L"lambda";
    input[0][1][0] = L"x";
    input[0][1][1] = L"y";
    input[0][2][0] = L"cons";
    input[0][2][1] = L"x";
    input[0][2][2] = L"y";
    input[1] = L"a";
    input[2][0] = L"b";
    input[2][1] = L"c";
    std::string expected (R"q([["lambda",["x","y"],["cons","x","y"]],"a",["b","c"]])q");
    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == expected, "json encode array nest");
}

void
test_array_nest_indented (test::simple& ts)
{
    wjson::value_type input = wjson::array ();
    input[0][0] = L"lambda";
    input[0][1][0] = L"x";
    input[0][1][1] = L"y";
    input[0][2][0] = L"cons";
    input[0][2][1] = L"x";
    input[0][2][2] = L"y";
    input[1] = L"a";
    input[2][0] = L"b";
    input[2][1] = L"c";
    std::string expected (
R"q([
  [
    "lambda",
    [
      "x",
      "y"
    ],
    [
      "cons",
      "x",
      "y"
    ]
  ],
  "a",
  [
    "b",
    "c"
  ]
])q");
    std::ostringstream got;
    wjson::encode_json (got, input, 2);
    ts.ok (got.str () == expected, "json encode array nest indented");
}

void
test_table_empty (test::simple& ts)
{
    wjson::value_type input = wjson::table ();
    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == "{}", "json encode table {}");
}

void
test_table_flat (test::simple& ts)
{
    wjson::value_type input = wjson::table ();
    input[L"bool"] = wjson::boolean (false);
    input[L"fixnum"] = wjson::fixnum (1);
    input[L"flonum"] = wjson::flonum (2.0);
    input[L"datetime"] = wjson::datetime (L"2015-10-18T01:32:57Z");
    input[L"string"] = L"four";
    std::string expected (
    R"q({"bool":false,"datetime":"2015-10-18T01:32:57Z",)q"
    R"q("fixnum":1,"flonum":2.0,"string":"four"})q"
    );
    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == expected, "json encode table flat");
}

void
test_table_nest (test::simple& ts)
{
    wjson::value_type input = wjson::table ();
    input[L"a"][L"a0"] = L"A0";
    input[L"a"][L"a1"] = L"A1";
    input[L"b"] = L"B";
    input[L"c"][L"c0"][L"c00"] = L"C0";
    input[L"c"][L"c1"][L"c00"] = L"C1";
    std::string expected (
    R"q({"a":{"a0":"A0","a1":"A1"},"b":"B",)q"
    R"q("c":{"c0":{"c00":"C0"},"c1":{"c00":"C1"}}})q"
    );
    std::ostringstream got;
    wjson::encode_json (got, input);
    ts.ok (got.str () == expected, "json encode table nest");
}

void
test_table_nest_indented (test::simple& ts)
{
    wjson::value_type input = wjson::table ();
    input[L"a"][L"a0"] = L"A0";
    input[L"a"][L"a1"] = L"A1";
    input[L"b"] = L"B";
    input[L"c"][L"c0"][L"c00"] = L"C0";
    input[L"c"][L"c1"][L"c00"] = L"C1";
    std::string expected (
R"q({
  "a": {
    "a0": "A0",
    "a1": "A1"
  },
  "b": "B",
  "c": {
    "c0": {
      "c00": "C0"
    },
    "c1": {
      "c00": "C1"
    }
  }
})q"
    );
    std::ostringstream got;
    wjson::encode_json (got, input, 2);
    ts.ok (got.str () == expected, "json encode table nest indented");
}

void
test_fluit (test::simple& ts)
{
    wjson::value_type input = wjson::table ();
    input[L"fruit"][0][L"name"] = L"apple";
    input[L"fruit"][0][L"physical"][L"color"] = L"red";
    input[L"fruit"][0][L"physical"][L"shape"] = L"round";
    input[L"fruit"][0][L"variety"][0][L"name"] = L"red delicious";
    input[L"fruit"][0][L"variety"][1][L"name"] = L"granny smith";
    input[L"fruit"][1][L"name"] = L"banana";
    input[L"fruit"][1][L"variety"][0][L"name"] = L"plantain";
    std::string expected (
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

    std::ostringstream got;
    wjson::encode_json (got, input, 2);
    ts.ok (got.str () == expected, "json encode fluit example");
}

int
main ()
{
    test::simple ts;

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
    test_flonum_max (ts);
    test_flonum_lowest (ts);

    test_datetime (ts);

    test_string_empty (ts);
    test_string_ascii (ts);
    test_string_mbyte (ts);

    test_array_empty (ts);
    test_array_flat (ts);
    test_array_nest (ts);
    test_array_nest_indented (ts);

    test_table_empty (ts);
    test_table_flat (ts);
    test_table_nest (ts);
    test_table_nest_indented (ts);

    test_fluit (ts);

    return ts.done_testing ();
}
