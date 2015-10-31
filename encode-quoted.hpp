#ifndef INSPECT_HPP
#define INSPECT_HPP

#include <string>
#include <typeinfo>

template<typename Iter>
std::string
encode_quoted (Iter s, Iter const e)
{
    using T = typename Iter::value_type;
    std::string buf = "\"";
    if (typeid (wchar_t) == typeid (T))
        buf = "L\"";
    else if (typeid (char32_t) == typeid (T))
        buf = "U\"";
    for (; s < e; ++s) {
        uint32_t const code
            = 1 == sizeof (T) ? static_cast<uint8_t> (*s)
            : 4 == sizeof (T) ? static_cast<uint32_t> (*s)
            : static_cast<uint16_t> (*s);
        if (code < 0x80 || sizeof (T) == 1) {
            switch (code) {
            case '"':
                buf += "\\\"";
                break;
            case '\\':
                buf += "\\\\";
                break;
            case '\b':
                buf += "\\b";
                break;
            case '\t':
                buf += "\\t";
                break;
            case '\n':
                buf += "\\n";
                break;
            case '\r':
                buf += "\\r";
                break;
            case '\f':
                buf += "\\f";
                break;
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
        else if (code < 0x800) {
            buf.push_back (((code >>  6) & 0xff) | 0xc0);
            buf.push_back (( code        & 0x3f) | 0x80);
        }
        else if (code < 0x10000) {
            buf.push_back (((code >> 12) & 0x0f) | 0xe0);
            buf.push_back (((code >>  6) & 0x3f) | 0x80);
            buf.push_back (( code        & 0x3f) | 0x80);
        }
        else if (code < 0x110000) {
            buf.push_back (((code >> 18) & 0x07) | 0xf0);
            buf.push_back (((code >> 12) & 0x3f) | 0x80);
            buf.push_back (((code >>  6) & 0x3f) | 0x80);
            buf.push_back (( code        & 0x3f) | 0x80);
        }
        else {
            buf += "\\U";
            uint32_t x = code;
            for (std::size_t i = 0; i < 8; ++i) {
                x = (x << 4) | (x >> 28);
                uint32_t const y = x & 15;
                buf.push_back (y < 10 ? y + '0' : y + 'a' - 10);
            }
        }
    }
    return buf + "\"";
}

template<typename C>
std::string
encode_quoted (std::basic_string<C> const& str)
{
    return encode_quoted (str.cbegin (), str.cend ());
}

#endif
