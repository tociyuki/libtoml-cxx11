#pragma once

#include <string>
#include <utility>
#include "value.hpp"

namespace wjson {

std::string::size_type decode_yaml (std::string const& input, value_type& value, std::string::size_type pos = 0);

}//namespace wjson

