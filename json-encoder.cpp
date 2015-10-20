#include <string>
#include <vector>
#include <map>
#include <ostream>
#include <sstream>
#include "json.hpp"

namespace wjson {

static void encode_flonum (std::ostream& out, double const x);
static void encode_string (std::ostream& out, std::wstring const& str);

std::string
encode_json (value_type const& value, int const padding, int const margin)
{
    std::ostringstream got;
    encode_json (got, value, padding, margin);
    return got.str ();
}

void
encode_json (std::ostream& out, value_type const& value,
    int const padding, int const margin)
{
    std::string indent (margin, ' ');
    std::string nest (margin + padding, ' ');
    std::string space ((padding ? 1 : 0), ' ');
    std::string endl ((padding ? 1 : 0), '\n');
    int count = 0;
    switch (value.tag ()) {
    case VALUE_NULL: out << "null"; break;
    case VALUE_BOOLEAN:
        if (value.boolean ())
            out << "true";
        else
            out << "false";
        break;
    case VALUE_FIXNUM: out << value.fixnum (); break;
    case VALUE_FLONUM: encode_flonum (out, value.flonum ()); break;
    case VALUE_DATETIME: encode_string (out, value.datetime ()); break;
    case VALUE_STRING: encode_string (out, value.string ()); break;
    case VALUE_ARRAY:
        if (value.size () == 0)
            out << "[]";
        else {
            out << "[" << endl;
            for (auto& x : value.array ()) {
                if (count++ > 0)
                    out << "," << endl;
                out << nest;
                encode_json (out, x, padding, margin + padding);
            }
            out << endl << indent << "]";
        }
        break;
    case VALUE_TABLE:
        if (value.size () == 0)
            out << "{}";
        else {
            out << "{" << endl;
            for (auto& x : value.table ()) {
                if (count++ > 0)
                    out << "," << endl;
                out << nest;
                encode_string (out, x.first);
                out << ":" << space;
                encode_json (out, x.second, padding, margin + padding);
            }
            out << endl << indent << "}";
        }
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
            case '/': out << "\\/"; break;
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
        else if (uc < 0x800) {
            out.put (((uc >>  6) & 0xff) | 0xc0);
            out.put (( uc        & 0x3f) | 0x80);
        }
        else if (uc < 0x10000) {
            out.put (((uc >> 12) & 0x0f) | 0xe0);
            out.put (((uc >>  6) & 0x3f) | 0x80);
            out.put (( uc        & 0x3f) | 0x80);
        }
        else if (uc < 0x110000) {
            out.put (((uc >> 18) & 0x07) | 0xf0);
            out.put (((uc >> 12) & 0x3f) | 0x80);
            out.put (((uc >>  6) & 0x3f) | 0x80);
            out.put (( uc        & 0x3f) | 0x80);
        }
    }
    out.put ('"');
}

}//namespace wjson
