#pragma once

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <ostream>
#include "value.hpp"

namespace wjson {

bool decode_json (std::string const& str, value_type& root);

std::string encode_json (value_type const& value,
    int const padding = 0, int const margin = 0);
void encode_json (std::ostream& out, value_type const& value,
    int const padding = 0, int const margin = 0);

}//namespace wjson

