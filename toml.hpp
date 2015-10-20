#pragma once

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <ostream>
#include "value.hpp"

namespace wjson {

class toml_encoder_type {
public:
    std::string encode (value_type const& root) const;
    void encode (std::ostream& out, value_type const& root) const;

private:
    void encode_section (std::ostream& out, value_type const& value,
        std::vector<std::wstring>& path) const;
    void encode_path (std::ostream& out,
        char const* lft, std::vector<std::wstring>& path, char const* rgt) const;
    void encode_table (std::ostream& out,
        value_type const& value, std::vector<std::wstring>& path) const;
    void encode_key (std::ostream& out, std::wstring const& key) const;
    void encode_flow (std::ostream& out, value_type const& value) const;
    void encode_flonum (std::ostream& out, double const x) const;
    void encode_string (std::ostream& out, std::wstring const& str) const;
    void encode_bare (std::ostream& out, std::wstring const& str) const;
};

class toml_decoder_type {
public:
    toml_decoder_type (std::string const& str);
    bool decode (value_type& root);
private:
    int kvstate;
    std::string const& string;
    std::string::const_iterator iter;
    std::map<std::wstring,int> mark;

    int next_token (value_type& value);
    int scan_key (value_type& value);
    int scan_value (value_type& value);
    int scan_string (value_type& value);
    int scan_number (value_type& value);

    value_type& merge_exclusive (value_type& x, value_type const& y);
    value_type& merge_table (value_type& x, value_type const& path, value_type const& y);
    value_type& merge_array (value_type& x, value_type const& path, value_type const& y);
    value_type& unify_back (value_type& x, value_type const& y);
    std::wstring path_to_string (value_type const& path);
};

}//namespace wjson

