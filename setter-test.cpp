#include "value.hpp"
#include "taptests.hpp"
#include <sstream>

void
wjson_setter_test (test::simple& ts)
{
    wjson::value_type value = wjson::table ();
    value[L"path"][L"to"] = wjson::string (L"leaf");
    ts.ok (value.table ().count (L"path") > 0, "value[path]");
    ts.ok (value.get (L"path").tag () == wjson::VALUE_TABLE,
        "value[path] is table");
    ts.ok (value.get (L"path").get (L"to").tag () == wjson::VALUE_STRING,
        "value[path][to] is string");
    ts.ok (value.get (L"path").get (L"to").string () == L"leaf",
        "value[path][to] value");
    ts.ok (value[L"path"].tag () == wjson::VALUE_TABLE,
        "value[path] is table");
    ts.ok (value[L"path"][L"to"].tag () == wjson::VALUE_STRING,
        "value[path][to] is string");
    ts.ok (value[L"path"][L"to"].string () == L"leaf", "value[path][to] value");
    ts.ok (wjson::exists (value[L"path"][L"to"]), "exists value[path][to]");

    std::wstringstream got;
    got << value[L"path"][L"to"];
    ts.ok (got.str () == L"leaf", "ostream << setter");

    value[L"quick"][L"brown"][L"fox"] = L"jumps over";
    ts.ok (value[L"quick"][L"brown"][L"fox"].string () == L"jumps over",
        "value[quick][brown][fox] value");

    value[wjson::string(L"lazy")] = L"dog";
    ts.ok (value[wjson::string(L"lazy")].string () == L"dog",
        "value[string(lazy)] == dog");
}

void
fruit_test (test::simple& ts)
{
    wjson::value_type doc = wjson::table ();
    doc[L"fruit"][0][L"name"] = L"apple";
    doc[L"fruit"][0][L"physical"][L"color"] = L"red";
    doc[L"fruit"][0][L"physical"][L"shape"] = L"round";
    doc[L"fruit"][0][L"variety"][0][L"name"] = L"red delicious";
    doc[L"fruit"][0][L"variety"][1][L"name"] = L"granny smith";
    doc[L"fruit"][1][L"name"] = L"banana";
    doc[L"fruit"][1][L"variety"][0][L"name"] = L"plantain";

    ts.ok (doc[L"fruit"].tag () == wjson::VALUE_ARRAY, "fruit array");
    ts.ok (doc[L"fruit"].array ().size () == 2, "fruit array size 2");
    ts.ok (doc[L"fruit"][0].tag () == wjson::VALUE_TABLE, "fruit[0] table");
    ts.ok (doc[L"fruit"][0][L"name"].tag () == wjson::VALUE_STRING,
        "fruit[0]name string");
    ts.ok (doc[L"fruit"][0][L"name"].string () == L"apple",
        "fruit[0]name == apple");
    ts.ok (doc[L"fruit"][0][L"physical"].tag () == wjson::VALUE_TABLE,
        "fruit[0]physical table");
    ts.ok (doc[L"fruit"][0][L"physical"][L"color"].tag () == wjson::VALUE_STRING,
        "fruit[0]physical.color string");
    ts.ok (doc[L"fruit"][0][L"physical"][L"color"].string () == L"red",
        "fruit[0]physical.color == red");
    ts.ok (doc[L"fruit"][0][L"physical"][L"shape"].tag () == wjson::VALUE_STRING,
        "fruit[0]physical.shape string");
    ts.ok (doc[L"fruit"][0][L"physical"][L"shape"].string () == L"round",
        "fruit[0]physical.shape == round");
    ts.ok (doc[L"fruit"][0][L"variety"].tag () == wjson::VALUE_ARRAY,
        "fruit[0]variety array");
    ts.ok (doc[L"fruit"][0][L"variety"][0].tag () == wjson::VALUE_TABLE,
        "fruit[0]variety[0] table");
    ts.ok (doc[L"fruit"][0][L"variety"][0][L"name"].tag () == wjson::VALUE_STRING,
        "fruit[0]variety[0].name string");
    ts.ok (doc[L"fruit"][0][L"variety"][0][L"name"].string () == L"red delicious",
        "fruit[0]variety[0].name == red delicious");
    ts.ok (doc[L"fruit"][0][L"variety"][1][L"name"].string () == L"granny smith",
        "fruit[0]variety[0].name == granny smith");

    ts.ok (doc[L"fruit"][1].tag () == wjson::VALUE_TABLE, "fruit[1] table");
    ts.ok (doc[L"fruit"][1][L"name"].tag () == wjson::VALUE_STRING,
        "fruit[1]name string");
    ts.ok (doc[L"fruit"][1][L"name"].string () == L"banana",
        "fruit[1]name == banana");
    ts.ok (doc[L"fruit"][1][L"variety"][0].tag () == wjson::VALUE_TABLE,
        "fruit[1]variety[0] table");
    ts.ok (doc[L"fruit"][1][L"variety"][0][L"name"].tag () == wjson::VALUE_STRING,
        "fruit[1]variety[0].name string");
    ts.ok (doc[L"fruit"][1][L"variety"][0][L"name"].string () == L"plantain",
        "fruit[1]variety[0].name plantain");
}

int
main ()
{
    test::simple ts;

    wjson_setter_test (ts);
    fruit_test (ts);

    return ts.done_testing ();
}
