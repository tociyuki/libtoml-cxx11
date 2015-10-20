#pragma once

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <ostream>
#include "value.hpp"

namespace wjson {

class json_encoder_type {
public:
    std::string encode (value_type const& value,
        int const padding = 0, int const margin = 0) const;
    void encode (std::ostream& out, value_type const& value,
        int const padding = 0, int const margin = 0) const;
private:
    void encode_flonum (std::ostream& out, double const x) const;
    void encode_string (std::ostream& out, std::wstring const& str) const;
};

class json_decoder_type {
public:
    json_decoder_type (std::string const& str);
    bool decode (value_type& root);
private:
    std::string const& string;
    std::string::const_iterator iter;

    int next_token (value_type& value);
    int scan_string (value_type& value);
    int scan_number (value_type& value);
};

}//namespace wjson

