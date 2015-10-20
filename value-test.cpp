#include "value.hpp"
#include "taptests.hpp"

void
wjson_value_test (test::simple& ts)
{
    wjson::value_type anull;
    ts.ok (anull.tag () == wjson::VALUE_NULL, "null tag");
    wjson::value_type atrue = wjson::boolean (true);
    ts.ok (atrue.tag () == wjson::VALUE_BOOLEAN, "true tag");
    ts.ok (atrue.boolean (), "true value");
    wjson::value_type afalse = wjson::boolean (false);
    ts.ok (afalse.tag () == wjson::VALUE_BOOLEAN, "false tag");
    ts.ok (! afalse.boolean (), "false value");
    wjson::value_type afixnum = wjson::fixnum (123);
    ts.ok (afixnum.tag () == wjson::VALUE_FIXNUM, "fixnum tag");
    ts.ok (afixnum.fixnum () == 123, "fixnum value");
    wjson::value_type aflonum = wjson::flonum (1.23);
    ts.ok (aflonum.tag () == wjson::VALUE_FLONUM, "flonum tag");
    ts.ok (aflonum.flonum () == 1.23, "flonum value");
    wjson::value_type astring = wjson::string (L"value");
    ts.ok (astring.tag () == wjson::VALUE_STRING, "string tag");
    ts.ok (astring.string () == L"value", "string value");
    wjson::value_type anarray = wjson::array ();
    ts.ok (anarray.tag () == wjson::VALUE_ARRAY, "array tag");
    ts.ok (anarray.array ().size () == 0, "array size 0");
    anarray.set (0, wjson::fixnum (1));
    ts.ok (anarray.array ().size () == 1, "array size 1");
    ts.ok (anarray.get (0).tag () == wjson::VALUE_FIXNUM, "array[0] fixnum");
    ts.ok (anarray.get (0).fixnum () == 1, "array[0] value == 1");
    anarray.set (1, wjson::fixnum (2));
    ts.ok (anarray.array ().size () == 2, "array size 2");
    ts.ok (anarray.get (0).tag () == wjson::VALUE_FIXNUM, "array[0] fixnum");
    ts.ok (anarray.get (0).fixnum () == 1, "array[0] value == 1");
    ts.ok (anarray.get (1).tag () == wjson::VALUE_FIXNUM, "array[1] fixnum");
    ts.ok (anarray.get (1).fixnum () == 2, "array[1] value == 2");
    anarray.set (1, wjson::fixnum (3));
    ts.ok (anarray.array ().size () == 2, "array size 2");
    ts.ok (anarray.get (0).tag () == wjson::VALUE_FIXNUM, "array[0] fixnum");
    ts.ok (anarray.get (0).fixnum () == 1, "array[0] value == 1");
    ts.ok (anarray.get (1).tag () == wjson::VALUE_FIXNUM, "array[1] fixnum");
    ts.ok (anarray.get (1).fixnum () == 3, "array[1] value == 3");
    anarray.array()[1] = wjson::fixnum (4);
    ts.ok (anarray.get (1).fixnum () == 4, "array[1] value == 4");
    anarray.push_back (wjson::fixnum (5));
    ts.ok (anarray.get (2).fixnum () == 5, "array push_back");
    wjson::value_type atable = wjson::table ();
    ts.ok (atable.tag () == wjson::VALUE_TABLE, "table tag");
    ts.ok (atable.table ().size () == 0, "table size 0");
    atable.set (L"foo", wjson::string (L"Foo"));
    ts.ok (atable.table ().size () == 1, "table size 1");
    ts.ok (atable.table ().at (L"foo").tag () == wjson::VALUE_STRING, "table[foo] string");
    ts.ok (atable.table ().at (L"foo").string () == L"Foo", "table[foo] == Foo");
    ts.ok (atable.get (L"foo").string () == L"Foo", "table.get(foo) == Foo");
    atable.set (wjson::string (L"bar"), wjson::string (L"Bar"));
    ts.ok (atable.table ().size () == 2, "table size 2");
    ts.ok (atable.table ().at (L"bar").string () == L"Bar", "table[bar] == Bar");
    ts.ok (atable.get (L"bar").string () == L"Bar", "table.get(bar) == Bar");
    ts.ok (atable.get (wjson::string (L"bar")).string () == L"Bar",
        "table.get(string(bar)) == Bar");
}

int
main ()
{
    test::simple ts (38);

    wjson_value_test (ts);

    return ts.done_testing ();
}
