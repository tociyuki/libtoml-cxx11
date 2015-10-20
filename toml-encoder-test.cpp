#include "toml.hpp"
#include "taptests.hpp"
#include "encode-quoted.hpp"
#include <sstream>
#include <limits>
#include <cstdio>

void
test_boolean (test::simple& ts)
{
    wjson::value_type input = wjson::table ();
    input[L"bool1"] = wjson::boolean (true);
    input[L"bool2"] = wjson::boolean (false);
    std::string expected (
    "bool1=true\n"
    "bool2=false\n"
    );

    std::ostringstream got;
    wjson::encode_toml (got, input);
    ts.ok (got.str () == expected, "toml encode boolean");
}

void
test_fixnum (test::simple& ts)
{
    int64_t const fixmax = std::numeric_limits<long long>::max ();
    int64_t const fixlowest = std::numeric_limits<long long>::lowest ();

    wjson::value_type input = wjson::table ();
    input[L"fix0"] = wjson::fixnum (fixmax);
    input[L"fix1"] = wjson::fixnum (1);
    input[L"fix2"] = wjson::fixnum (0);
    input[L"fix3"] = wjson::fixnum (-1);
    input[L"fix4"] = wjson::fixnum (fixlowest);
    std::ostringstream expected;
    expected << "fix0=" << fixmax << "\n"
             << "fix1=1\n"
             << "fix2=0\n"
             << "fix3=-1\n"
             << "fix4=" << fixlowest << "\n";

    std::ostringstream got;
    wjson::encode_toml (got, input);
    ts.ok (got.str () == expected.str (), "toml encode fixnum");
}

void
test_flonum (test::simple& ts)
{
    double const flomax = std::numeric_limits<double>::max ();
    double const flolowest = std::numeric_limits<double>::lowest ();

    wjson::value_type input = wjson::table ();
    input[L"fix0"] = wjson::flonum (flomax);
    input[L"fix1"] = wjson::flonum (1);
    input[L"fix2"] = wjson::flonum (0);
    input[L"fix3"] = wjson::flonum (-1);
    input[L"fix4"] = wjson::flonum (flolowest);
    std::ostringstream expected;
    char buf[32];
    std::snprintf (buf, sizeof(buf)/sizeof(buf[0]), "%.15g", flomax);
    expected << "fix0=" << buf << "\n";
    expected << "fix1=1.0\n"
             << "fix2=0.0\n"
             << "fix3=-1.0\n";
    std::snprintf (buf, sizeof(buf)/sizeof(buf[0]), "%.15g", flolowest);
    expected << "fix4=" << buf << "\n";

    std::ostringstream got;
    wjson::encode_toml (got, input);
    ts.ok (got.str () == expected.str (), "toml encode flonum");
}

void
test_datetime (test::simple& ts)
{
    wjson::value_type input = wjson::table ();
    input[L"date"] = wjson::datetime (L"2015-10-18T01:02:45Z");
    std::string expected (
    "date=2015-10-18T01:02:45Z\n"
    );

    std::ostringstream got;
    wjson::encode_toml (got, input);
    ts.ok (got.str () == expected, "toml encode datetime");
}


void
test_string (test::simple& ts)
{
    std::wstring ascii;
    for (int c = 0; c < 128; ++c)
        ascii.push_back (c);

    wjson::value_type input = wjson::table ();
    input[L"str1"] = L"";
    input[L"str2"] = ascii;
    input[L"str3"] = L"いろはに";
    std::string expected (
    "str1=\"\"\n"
    "str2=\""
        R"q(\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\b)q"
        R"q(\t\n\u000b\f\r\u000e\u000f)q"
        R"q(\u0010\u0011\u0012\u0013\u0014\u0015\u0016\u0017)q"
        R"q(\u0018\u0019\u001a\u001b\u001c\u001d\u001e\u001f)q"
        R"q( !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_)q"
        R"q(`abcdefghijklmnopqrstuvwxyz{|}~\u007f)q"
        "\"\n"
    u8"str3=\"いろはに\"\n"
    );

    std::ostringstream got;
    wjson::encode_toml (got, input);
    ts.ok (got.str () == expected, "toml encode string");
}

void
test_array_1 (test::simple& ts)
{
    wjson::value_type input = wjson::table ();
    input[L"a"] = wjson::array ();
    input[L"b"][0] = wjson::boolean (false);
    input[L"b"][1] = wjson::fixnum (1);
    input[L"b"][2] = wjson::flonum (2.0);
    input[L"b"][3] = wjson::datetime (L"2015-10-18T01:32:57Z");
    input[L"b"][4] = L"four";
    std::string expected (
    "a=[]\n"
    "b=" R"q([false,1,2.0,2015-10-18T01:32:57Z,"four"])q" "\n"
    );

    std::ostringstream got;
    wjson::encode_toml (got, input);
    ts.ok (got.str () == expected, "toml encode array_1");    
}

void
test_array_2 (test::simple& ts)
{
    wjson::value_type input = wjson::table ();
    input[L"a"][0][0] = L"lambda";
    input[L"a"][0][1][0] = L"x";
    input[L"a"][0][1][1] = L"y";
    input[L"a"][0][2][0] = L"cons";
    input[L"a"][0][2][1] = L"x";
    input[L"a"][0][2][2] = L"y";
    input[L"a"][1] = L"a";
    input[L"a"][2][0] = L"b";
    input[L"a"][2][1] = L"c";
    std::string expected (
    "a=" R"q([["lambda",["x","y"],["cons","x","y"]],"a",["b","c"]])q" "\n"
    );

    std::ostringstream got;
    wjson::encode_toml (got, input);
    ts.ok (got.str () == expected, "toml encode array_2");    
}

void
test_table_1 (test::simple& ts)
{
    wjson::value_type input = wjson::table ();
    input[L"a"] = wjson::table ();
    input[L"b"][L"bool"] = wjson::boolean (false);
    input[L"b"][L"fixnum"] = wjson::fixnum (1);
    input[L"b"][L"flonum"] = wjson::flonum (2.0);
    input[L"b"][L"datetime"] = wjson::datetime (L"2015-10-18T01:32:57Z");
    input[L"b"][L"string"] = L"four";
    std::string expected (
R"q(
[a]

[b]
bool=false
datetime=2015-10-18T01:32:57Z
fixnum=1
flonum=2.0
string="four"
)q"
    );

    std::ostringstream got;
    wjson::encode_toml (got, input);
    ts.ok (got.str () == expected, "toml encode table_1");
}

void
test_table_2 (test::simple& ts)
{
    wjson::value_type input = wjson::table ();
    input[L"a"][L"a0"] = L"A0";
    input[L"a"][L"a1"] = L"A1";
    input[L"b"] = L"B";
    input[L"c"][L"c0"][L"c00"] = L"C0";
    input[L"c"][L"c1"][L"c00"] = L"C1";
    std::string expected (
R"q(b="B"

[a]
a0="A0"
a1="A1"

[c]

[c.c0]
c00="C0"

[c.c1]
c00="C1"
)q"
    );

    std::ostringstream got;
    wjson::encode_toml (got, input);
    ts.ok (got.str () == expected, "toml encode table_2");
}

void
test_fruit (test::simple& ts)
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
R"q(
[[fruit]]
name="apple"

[fruit.physical]
color="red"
shape="round"

[[fruit.variety]]
name="red delicious"

[[fruit.variety]]
name="granny smith"

[[fruit]]
name="banana"

[[fruit.variety]]
name="plantain"
)q"
    );

    std::ostringstream got;
    wjson::encode_toml (got, input);
    ts.ok (got.str () == expected, "toml encode fruit");
}

int
main ()
{
    test::simple ts (10);

    test_boolean (ts);
    test_fixnum (ts);
    test_flonum (ts);
    test_datetime (ts);
    test_string (ts);
    test_array_1 (ts);
    test_array_2 (ts);
    test_table_1 (ts);
    test_table_2 (ts);
    test_fruit (ts);

    return ts.done_testing ();
}
