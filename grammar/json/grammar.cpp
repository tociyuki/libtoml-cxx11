#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include "../check-builder.hpp"
#include "../layoutable.hpp"

enum {
    TOKEN_INVALID,
    TOKEN_SCALAR,
    TOKEN_STRING,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_ENDMARK,
};

struct toml_grammar_type : public layoutable, public check_builder_type {
    std::vector<int> base;
    std::vector<int> check;
    std::vector<int> nrhs;
    std::vector<int> colgoto;
    bool defined;

    toml_grammar_type ()
        : check_builder_type (), base (), check (), nrhs (), colgoto (),
          defined (false) {}

// ruby grammar.rb > grammar-define-state.cpp
#include "grammar-define-state.cpp"

    toml_grammar_type&
    define ()
    {
        if (defined)
            return *this;
        defined = true;
        define_state ();
        squarize_table ();
        pack_table (base, check);
        return *this;
    }

    std::string
    render (std::string const& layout)
    {
        std::string s = render_int (layout, "@<nprod@>", nrhs.size ());
        s = render_int (s, "@<nbase@>", base.size ());
        s = render_int (s, "@<ncheck@>", check.size ());
        s = render_vector_dec (s, "@<base@>\n", base);
        s = render_vector_hex (s, "@<check@>\n", check);
        s = render_vector_dec (s, "@<nrhs@>\n", nrhs);
        s = render_vector_dec (s, "@<colgoto@>\n", colgoto);
        return std::move (s);
    }
};

std::string const layout (R"EOS(
bool
json_decoder_type::decode (value_type& root)
{
    enum { NCHECK = @<ncheck@>, ACCEPT = 255 };
    static const int BASE[@<nbase@>] = {
        @<base@>
    };
    static const int CHECK[NCHECK] = {
        @<check@>
    };
    static const int GOTO[@<nprod@>] = {
        @<colgoto@>
    };
    static const int NRHS[@<nprod@>] = {
        @<nrhs@>
    };
    iter = string.cbegin ();
    std::deque<int> sstack {1};
    std::deque<value_type> dstack;
    dstack.emplace_back (); // centinel
    value_type token_value;
    int token_type = next_token (token_value);
    for (;;) {
        int prev_state = sstack.back ();        
        int j = BASE[prev_state] + token_type;
        int ctrl = 0;
        if (0 < j && j < NCHECK && (CHECK[j] & 0xff) == prev_state) {
            ctrl = CHECK[j] >> 8;
        }
        if (! ctrl)
            break;
        else if (ctrl < 128) {  // shift
            sstack.push_back (ctrl);
            dstack.push_back (token_value);
            token_type = next_token (token_value);
        }
        else if (ctrl == ACCEPT) {
            std::swap (root, dstack.back ());
            return true;
        }
        else {    // reduce
            int prod = 256 - ctrl - 2;
            int nrhs = NRHS[prod];
            std::deque<value_type>::iterator v = dstack.end () - nrhs - 1;
            value_type value;
            switch (prod) {
            case  0: // start: value
            case  1: // value: SCALAR
            case  2: // value: STRING
                std::swap (value, v[1]);
                break;
            case  3: // value: "[" array "]"
            case  4: // value: "{" table "}"
                std::swap (value, v[2]);
                break;
            case  5: // value: "[" "]"
                value = ::wjson::array ();
                break;
            case  6: // value: "{" "}"
                value = ::wjson::table ();
                break;
            case  7: // array: array "," value
                value = std::move (v[1].push_back (std::move (v[3])));
                break;
            case  8: // array: value
                value = ::wjson::array ().push_back (std::move (v[1]));
                break;
            case  9: // table: table "," STRING ":" value
                value = std::move (v[1].set (v[3], std::move (v[5])));
                break;
            case 10: // table: STRING ":" value
                value = ::wjson::table ().set (v[1], std::move (v[3]));
                break;
            }
            for (int i = 0; i < nrhs; ++i)
                sstack.pop_back ();
            for (int i = 0; i < nrhs; ++i)
                dstack.pop_back ();
            int gprev_state = sstack.back ();
            int g = BASE[gprev_state] + GOTO[prod];
            int gnext_state = 0;
            if (0 < g && g < NCHECK && (CHECK[g] & 0xff) == gprev_state)
                gnext_state = CHECK[g] >> 8;
            if (! gnext_state)
                std::logic_error ("json_decoder::decode: grammar table error");
            sstack.push_back (gnext_state);
            dstack.push_back (value);
        }
    }
    return false;
}
)EOS"
);

int
main ()
{
    toml_grammar_type grammar;
    std::cout << grammar.define ().render (layout);
    return EXIT_SUCCESS;
}
