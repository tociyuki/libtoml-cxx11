#pragma once

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <ostream>
#include "value.hpp"

namespace wjson {

bool decode_toml (std::string const& str, value_type& root);

std::string encode_toml (value_type const& root);
void encode_toml (std::ostream& out, value_type const& root);

}//namespace wjson

