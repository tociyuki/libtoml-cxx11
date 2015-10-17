#pragma once

#include <vector>
#include <string>

class check_builder_type {
private:
    bool
    check_occupied (std::vector<int> const& check, int const start,
        std::vector<int> const& hotspots)
    {
        for (int c : hotspots) {
            int j = start + c - hotspots[0];
            if (j < check.size () && check[j] > 0)
                return true;
        }
        return false;
    }

public:
    std::vector<int> cclass;
    std::vector< std::vector<int> > table;

    check_builder_type () : cclass (), table () {}

    void
    charset (std::size_t const i, int const v)
    {
        if (i >= cclass.size ())
            cclass.resize (i + 1, 0);
        cclass[i] = v;
    }

    void
    charset (std::size_t const i, std::size_t const j, int const v)
    {
        for (std::size_t c = i; c <= j; ++c)
            charset (c, v);
    }

    void
    charset (std::string const& s, int const v)
    {
        for (std::size_t i = 0; i < s.size (); ++i)
            charset (static_cast<uint8_t> (s[i]), v);
    }

    void
    to (int const s0, int const c, int const s1, int const a = 0)
    {
        while (s0 >= table.size ())
            table.emplace_back (1, 0);
        if (c >= table[s0].size ())
            table[s0].resize (c + 1, 0);
        table[s0][c] = (a << 16) | (s1 << 8) | s0;
    }

    void
    squarize_table ()
    {
        std::size_t colsize = 0;
        for (std::size_t i = 0; i < table.size (); ++i)
            colsize = std::max (colsize, table[i].size ());
        for (std::size_t i = 0; i < table.size (); ++i)
            if (colsize > table[i].size ())
                table[i].resize (colsize, 0);
    }

    void
    pack_table (std::vector<int>& base, std::vector<int>& check)
    {
        base.resize (table.size (), 0);
        check.push_back (0);
        for (std::size_t state = 1; state < table.size (); ++state) {
            std::vector<int> hotspots;
            for (std::size_t i = 0; i < table[state].size (); ++i)
                if (table[state][i] > 0)
                    hotspots.push_back (static_cast<int> (i));
            for (int start = 1; true; ++start)
                if (! check_occupied (check, start, hotspots)) {
                    base[state] = start - hotspots[0];
                    break;
                }
            if (base[state] + hotspots.back () >= check.size ())
                check.resize (base[state] + hotspots.back () + 1, 0);
            for (int c : hotspots)
                check[base[state] + c] = table[state][c];
        }
    }
};

