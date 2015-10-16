#include <string>
#include <vector>
#include <map>
#include <utility>
#include <ostream>
#include <sstream>
#include "toml.hpp"

namespace toml {

std::string
doc_type::encode_quoted (std::string::const_iterator s, std::string::const_iterator const e) const
{
    std::string buf = "\"";
    for (; s < e; ++s) {
        uint32_t const code = static_cast<uint8_t> (*s);
        switch (code) {
        case '"': buf += "\\\""; break;
        case '\\': buf += "\\\\"; break;
        case '\b': buf += "\\b"; break;
        case '\t': buf += "\\t"; break;
        case '\n': buf += "\\n"; break;
        case '\f': buf += "\\f"; break;
        case '\r': buf += "\\r"; break;
        default:
            if (' ' <= code && code <= 0x7e) {
                buf.push_back (code);
            }
            else if (0xc0 <= code && code <= 0xdf && s + 1 < e) {
                buf.append (s, s + 2); s += 1;
            }
            else if (0xe0 <= code && code <= 0xef && s + 2 < e) {
                buf.append (s, s + 3); s += 2;
            }
            else if (0xf0 <= code && code <= 0xf8 && s + 3 < e) {
                buf.append (s, s + 4); s += 3;
            }
            else {
                buf += "\\x";
                uint32_t const x = code & 15;
                buf.push_back (x < 10 ? x + '0' : x + 'a' - 10);
                uint32_t const y = (code >> 4) & 15;
                buf.push_back (y < 10 ? y + '0' : y + 'a' - 10);
            }
            break;
        }
    }
    return buf + "\"";
}

std::string
doc_type::encode_quoted (std::string const& str) const
{
    return encode_quoted (str.cbegin (), str.cend ());
}

void
doc_type::encode_toml (std::ostream& out) const
{
    std::vector<std::string> path;
    encode_toml_section (out, root, path);
}

void
doc_type::encode_toml_section (std::ostream& out, value_id const id,
    std::vector<std::string>& path) const
{
    if (at_tag (id) == VALUE_TABLE) {
        encode_toml_path (out, "\n[", path, "]\n");
        encode_toml_table_section (out, id, path);
    }
    else if (at_tag (id) == VALUE_ARRAY) {
        for (auto ix : at_array (id)) {
            encode_toml_path (out, "\n[[", path, "]]\n");
            encode_toml_table_section (out, ix, path);
        }
    }
}

void
doc_type::encode_toml_path (std::ostream& out,
    char const* lft, std::vector<std::string>& path, char const* rgt) const
{
    if (path.empty ())
        return;
    out << lft;
    int c = 0;
    for (auto& x : path) {
        if (c++ > 0) out << ".";
        encode_toml_key (out, x);
    }
    out << rgt;
}

void
doc_type::encode_toml_table_section (std::ostream& out,
    value_id const id, std::vector<std::string>& path) const
{
    for (auto x : at_table (id)) {
        if (at_tag (x.second) != VALUE_TABLE && at_tag (x.second) != VALUE_ARRAY) {
            encode_toml_key (out, x.first);
            out << "=";
            encode_toml_flow (out, x.second);
            out << std::endl;
        }
    }
    for (auto x : at_table (id)) {
        path.push_back (x.first);
        if (at_tag (x.second) == VALUE_TABLE) {
            encode_toml_section (out, x.second, path);
        }
        else if (at_tag (x.second) == VALUE_ARRAY
                && size (x.second) > 0
                && at_tag (get (x.second, 0)) == VALUE_TABLE) {
            encode_toml_section (out, x.second, path);
        }
        else if (at_tag (x.second) == VALUE_ARRAY) {
            encode_toml_key (out, x.first);
            out << "=";
            encode_toml_flow (out, x.second);
            out << std::endl;            
        }
        path.pop_back ();
    }
}

void
doc_type::encode_toml_key (std::ostream& out, std::string const& key) const
{
    bool barekey = true;
    for (auto c : key) {
        if (! (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')
                || ('0' <= c && c <= '9')
                || '_' == c || '-' == c)) {
            barekey = false;
            break;
        }
    }
    if (barekey)
        out << key;
    else
        out << encode_quoted (key);
}

void
doc_type::encode_toml_flow (std::ostream& out, value_id const id) const
{
    int c = 0;
    switch (at_tag (id)) {
    case VALUE_BOOLEAN:
        if (at_boolean (id))
            out << "true";
        else
            out << "false";
        break;
    case VALUE_FIXNUM: out << at_fixnum (id); break;
    case VALUE_FLONUM: out << at_flonum (id); break;
    case VALUE_DATETIME: out << at_datetime (id); break;
    case VALUE_STRING: out << encode_quoted (at_string (id)); break;
    case VALUE_TABLE:
        out << "{";
        for (auto x : at_table (id)) {
            if (c++ > 0)
                out << ",";
            encode_toml_key (out, x.first);
            out << "=";
            encode_toml_flow (out, x.second);
        }
        out << "}";
        break;
    case VALUE_ARRAY:
        out << "[";
        for (auto x : at_array (id)) {
            if (c++ > 0)
                out << ",";
            encode_toml_flow (out, x);
        }
        out << "]";
        break;
    default:
        break;
    }
}

void
doc_type::encode_json (std::ostream& out, value_id const x,
    int const margin, int const padding) const
{
    std::string indent (margin, ' ');
    std::string nest (margin + padding, ' ');
    std::string space ((padding ? 1 : 0), ' ');
    std::ostringstream outendl;
    outendl << std::endl;
    std::string endl = outendl.str ();
    if (! padding)
        endl.clear ();
    int c = 0;
    switch (at_tag (x)) {
    case VALUE_NULL: out << "NULL"; break;
    case VALUE_BOOLEAN:
        if (at_boolean (x))
            out << "true";
        else
            out << "false";
        break;
    case VALUE_FIXNUM: out << at_fixnum (x); break;
    case VALUE_FLONUM: out << at_flonum (x); break;
    case VALUE_DATETIME: out << encode_quoted (at_datetime (x)); break;
    case VALUE_STRING: out << encode_quoted (at_string (x)); break;
    case VALUE_ARRAY:
        if (size (x) == 0) {
            out << "[]";
        }
        else {
            out << "[" << endl;
            for (auto y : at_array (x)) {
                if (c++ > 0)
                    out << "," << endl;
                out << nest;
                encode_json (out, y, margin + padding, padding);
            }
            out << endl << indent << "]";
        }
        break;
    case VALUE_TABLE:
        if (size (x) == 0) {
            out << "{}";
        }
        else {
            out << "{" << endl;
            for (auto y : at_table (x)) {
                if (c++ > 0)
                    out << "," << endl;
                out << nest << encode_quoted (y.first) << ":" << space;
                encode_json (out, y.second, margin + padding, padding);
            }
            out << endl << indent << "}";
        }
        break;
    default: break;
    }
}

void
doc_type::encode_json (std::ostream& out, int const padding) const
{
    encode_json (out, root, 0, padding);
    if (padding)
        out << std::endl;
}

std::string
doc_type::str (value_id const id) const
{
    if (at_tag (id) == VALUE_STRING)
        return at_string (id);
    std::ostringstream out;
    encode_toml_flow (out, id);
    return out.str ();
}

}//namespace toml

std::ostream&
operator<< (std::ostream& stream, toml::node_type const& x)
{
    stream << x.str ();
    return stream;
}
