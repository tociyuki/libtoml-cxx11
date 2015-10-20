#include <string>
#include <vector>
#include <map>
#include <utility>
#include <ostream>
#include <sstream>
#include "toml.hpp"
#include "encode-utf8.hpp"

namespace wjson {

static void encode_section (std::ostream& out, value_type const& value,
    std::vector<std::wstring>& path);
static void encode_path (std::ostream& out,
    char const* lft, std::vector<std::wstring>& path, char const* rgt);
static void encode_table (std::ostream& out,
    value_type const& value, std::vector<std::wstring>& path);
static void encode_key (std::ostream& out, std::wstring const& key);
static void encode_flow (std::ostream& out, value_type const& value);
static void encode_flonum (std::ostream& out, double const x);
static void encode_string (std::ostream& out, std::wstring const& str);
static void encode_bare (std::ostream& out, std::wstring const& str);

std::string
encode_toml (value_type const& root)
{
    std::ostringstream got;
    std::vector<std::wstring> path;
    encode_section (got, root, path);
    return got.str ();
}

void
encode_toml (std::ostream& out, value_type const& root)
{
    std::vector<std::wstring> path;
    encode_section (out, root, path);
}

static void
encode_section (std::ostream& out, value_type const& value,
    std::vector<std::wstring>& path)
{
    if (value.tag () == VALUE_TABLE) {
        encode_path (out, "\n[", path, "]\n");
        encode_table (out, value, path);
    }
    else if (value.tag () == VALUE_ARRAY) {
        for (value_type const& item : value.array ()) {
            encode_path (out, "\n[[", path, "]]\n");
            encode_table (out, item, path);
        }
    }
}

static void
encode_path (std::ostream& out,
    char const* lft, std::vector<std::wstring>& path, char const* rgt)
{
    if (path.empty ())
        return;
    out << lft;
    int c = 0;
    for (auto& x : path) {
        if (c++ > 0) out << ".";
        encode_key (out, x);
    }
    out << rgt;
}

static void
encode_table (std::ostream& out,
    value_type const& value, std::vector<std::wstring>& path)
{
    for (auto x : value.table ()) {
        if (x.second.tag () != VALUE_TABLE && x.second.tag () != VALUE_ARRAY) {
            encode_key (out, x.first);
            out << "=";
            encode_flow (out, x.second);
            out << std::endl;
        }
    }
    for (auto x : value.table ()) {
        path.push_back (x.first);
        if (x.second.tag () == VALUE_TABLE) {
            encode_section (out, x.second, path);
        }
        else if (x.second.tag () == VALUE_ARRAY
                && x.second.size () > 0
                && x.second.get (0).tag () == VALUE_TABLE) {
            encode_section (out, x.second, path);
        }
        else if (x.second.tag () == VALUE_ARRAY) {
            encode_key (out, x.first);
            out << "=";
            encode_flow (out, x.second);
            out << std::endl;            
        }
        path.pop_back ();
    }
}

static void
encode_key (std::ostream& out, std::wstring const& key)
{
    bool barekey = true;
    for (int c : key) {
        if (! (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')
                || ('0' <= c && c <= '9')
                || '_' == c || '-' == c)) {
            barekey = false;
            break;
        }
    }
    if (barekey)
        encode_bare (out, key);
    else
        encode_string (out, key);
}

static void
encode_bare (std::ostream& out, std::wstring const& key)
{
    for (int c : key)
        out.put (c);
}

static void
encode_flow (std::ostream& out, value_type const& value)
{
    int c = 0;
    switch (value.tag ()) {
    case VALUE_BOOLEAN:
        if (value.boolean ())
            out << "true";
        else
            out << "false";
        break;
    case VALUE_FIXNUM: out << value.fixnum (); break;
    case VALUE_FLONUM: encode_flonum (out, value.flonum ()); break;
    case VALUE_DATETIME: encode_bare (out, value.datetime ()); break;
    case VALUE_STRING: encode_string (out, value.string ()); break;
    case VALUE_TABLE:
        out << "{";
        for (auto x : value.table ()) {
            if (c++ > 0)
                out << ",";
            encode_key (out, x.first);
            out << "=";
            encode_flow (out, x.second);
        }
        out << "}";
        break;
    case VALUE_ARRAY:
        out << "[";
        for (auto x : value.array ()) {
            if (c++ > 0)
                out << ",";
            encode_flow (out, x);
        }
        out << "]";
        break;
    default:
        break;
    }
}

static void
encode_flonum (std::ostream& out, double const x)
{
    char buf[32];
    std::snprintf (buf, sizeof (buf) / sizeof (buf[0]), "%.15g", x);
    std::string t (buf);
    if (t.find_first_of (".e") == std::string::npos)
        t += ".0";
    out << t;
}

static void
encode_string (std::ostream& out, std::wstring const& str)
{
    out.put ('"');
    for (std::wstring::const_iterator s = str.cbegin (); s < str.cend (); ++s) {
        uint32_t const uc = static_cast<uint32_t> (*s);
        if (uc < 0x80) {
            switch (uc) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\t': out << "\\t"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            default:
                if (' ' <= uc && uc <= 0x7e) {
                    out.put (uc);
                }
                else {
                    out << "\\u00";
                    uint32_t const x = (uc >> 4) & 15;
                    uint32_t const y = uc & 15;
                    out.put (x < 10 ? x + '0' : x + 'a' - 10);
                    out.put (y < 10 ? y + '0' : y + 'a' - 10);
                }
                break;
            }
        }
        else {
            encode_utf8 (out, uc);
        }
    }
    out.put ('"');
}

}//namespace toml
