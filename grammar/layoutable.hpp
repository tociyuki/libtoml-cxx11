#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>

class layoutable {
public:
    enum { SCRWIDTH = 77 };

    std::string
    subst32 (std::string const& layout, std::vector<int> const& param, int row)
    {
        std::string t;
        std::string::size_type q = 0;
        for (int i = 0; i < 32; ++i) {
            std::string::size_type const p1 = q;
            q = layout.find ('$', q);
            std::string::size_type const p2 = q;
            ++q;
            if (p1 < p2)
                t += layout.substr (p1, p2 - p1);
            int x = param[row + i];
            t.push_back (x < 10 ? x + '0' : x - 10 + 'a');
        }
        if (q < layout.size ())
            t += layout.substr (q);
        return t;
    }

    std::string
    render_cclass (std::string const& layout, std::string const& key, std::vector<int> const& param)
    {
        static const std::string C (
            "0x$$$$$$$$, 0x$$$$$$$$, 0x$$$$$$$$, 0x$$$$$$$$,\n"
        );
        std::size_t const pos = layout.find (key);
        std::size_t const margin = pos - layout.rfind ('\n', pos) - 1;
        std::string indent (margin, ' ');
        std::string content;
        for (int row = 0; row < param.size (); row += 32) {
            if (row > 0)
                content += indent;
            content += subst32 (C, param, row);
        }
        return layout.substr (0, pos)
             + content
             + layout.substr (pos + key.size ());
    }

    std::string
    render_int (std::string const& layout, std::string const& key, int const param)
    {
        std::string content;
        for (std::size_t q = 0; q < layout.size (); q += key.size ()) {
            std::size_t const p = q;
            q = layout.find (key, p);
            if (q == std::string::npos)
                q = layout.size ();
            if (p < q)
                content.append (layout.cbegin () + p, layout.cbegin () + q);
            if (q < layout.size ())
                content += std::to_string (param);
        }
        return content;
    }

    std::string
    render_vector_dec (std::string const& layout, std::string const& key, std::vector<int> const& param)
    {
        std::size_t const pos = layout.find (key);
        std::size_t const indent = pos - layout.rfind ('\n', pos) - 1;
        std::size_t tw = 0;
        for (int x : param)
            tw = std::max (tw, std::to_string (x).size ());
        std::size_t const wcol = tw;
        std::size_t const ncol = (SCRWIDTH - indent - wcol - 1) / (wcol + 2);
        std::ostringstream content;
        for (std::size_t k = 0; k < param.size(); k += ncol + 1) {
            if (k > 0 && indent > 0)
                content << std::setw (indent) << "";
            for (std::size_t j = 0; j < ncol + 1; ++j) {
                if (k + j < param.size ()) {
                    content << std::right << std::setw (wcol) << param[k + j];
                    if (k + j < param.size () - 1)
                        content << ",";
                    if (j < ncol)
                        content << " ";
                }
            }
            content << std::endl;
        }
        return layout.substr (0, pos)
             + content.str ()
             + layout.substr (pos + key.size ());
    }

    std::string
    render_vector_hex (std::string const& layout, std::string const& key, std::vector<int> const& param)
    {
        std::size_t const pos = layout.find (key);
        std::size_t const indent = pos - layout.rfind ('\n', pos) - 1;
        std::size_t tw = 0;
        for (int x : param) {
            std::ostringstream oss;
            oss << std::hex << std::showbase << x;
            tw = std::max (tw, oss.str ().size ());
        }
        std::size_t const wcol = tw;
        std::size_t const ncol = (SCRWIDTH - indent - wcol - 1) / (wcol + 2);
        std::ostringstream content;
        for (std::size_t k = 0; k < param.size(); k += ncol + 1) {
            if (k > 0 && indent > 0)
                content << std::setfill (' ') << std::setw (indent) << "";
            for (std::size_t j = 0; j < ncol + 1; ++j) {
                if (k + j < param.size ()) {
                    if (0 == param[k + j]) {
                        content << std::right << std::setfill (' ')
                            << std::setw (wcol) << std::hex << param[k + j];
                    }
                    else {
                        content << "0x" << std::right << std::setfill ('0')
                            << std::setw (wcol - 2) << std::hex << param[k + j];
                    }
                    if (k + j < param.size () - 1)
                        content << ",";
                    if (j < ncol)
                        content << " ";
                }
            }
            content << std::endl;
        }
        return layout.substr (0, pos)
             + content.str ()
             + layout.substr (pos + key.size ());
    }
};
