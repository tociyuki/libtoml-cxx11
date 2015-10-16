#include <iostream>
#include "toml.hpp"

void
example1 ()
{
    std::string input (
R"EOS(
[[products]]
name = "Hammer"
sku = 738594937

[[products]]

[[products]]
name = "Nail"
sku = 284758393
color = "gray"

[[fruit]]
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
)EOS"
    );

    toml::doc_type doc;
    if (! doc.decode_toml (input)) {
        std::cerr << "syntax error" << std::endl;
        exit (EXIT_FAILURE);
    }
    std::cout << "/* example 1 */" << std::endl;
    doc.encode_json (std::cout, 2);
    std::cout << std::endl;
}

void
example2 ()
{
    toml::doc_type doc;
    doc.root = doc.table ();
    doc["fruit"][0]["name"] = "apple";
    doc["fruit"][0]["physical"]["color"] = "red";
    doc["fruit"][0]["physical"]["shape"] = "round";
    doc["fruit"][0]["variety"][0]["name"] = "red delicious";
    doc["fruit"][0]["variety"][1]["name"] = "granny smith";
    doc["fruit"][1]["name"] = "banana";
    doc["fruit"][1]["variety"][0]["name"] = "plantain";

    std::cout << "# example 2" << std::endl;
    doc.encode_toml (std::cout);
}

int
main ()
{
    example1 ();
    std::cout << std::endl;
    example2 ();
    return EXIT_SUCCESS;
}
