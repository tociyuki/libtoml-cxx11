#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include "check-builder.hpp"
#include "layoutable.hpp"

enum {
    TOKEN_INVALID,
    TOKEN_BAREKEY,
    TOKEN_STRKEY,
    TOKEN_STRING,
    TOKEN_BOOLEAN,
    TOKEN_FIXNUM,
    TOKEN_FLONUM,
    TOKEN_DATETIME,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_DOT,
    TOKEN_EQUAL,
    TOKEN_COMMA,
    TOKEN_ENDLINE,
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
decoder_type::parse (doc_type& doc)
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
    kvstate = 0;
    mark.clear ();
    std::deque<int> sstack {1};
    std::deque<value_id> dstack {0}; // centinel
    value_id token_value;
    int token_type = next_token (doc, token_value);
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
            token_type = next_token (doc, token_value);
        }
        else if (ctrl == ACCEPT) {
            doc.root = dstack.back ();
            return true;
        }
        else {    // reduce
            int prod = 256 - ctrl - 2;
            int nrhs = NRHS[prod];
            std::deque<value_id>::iterator v = dstack.end () - nrhs - 1;
            value_id value = nrhs > 0 ? v[1] : 0;
            switch (prod) {
            case 1: // toml: statements sections
                value = merge_exclusive (doc, v[1], v[2]);
                break;
            case 5: // sections: sections "[" keypath "]" ENDLINE statements
                value = merge_table (doc, v[1], v[3], v[6]);
                break;
            case 6: // sections: sections "[" keypath "]" ENDLINE
                value = merge_table (doc, v[1], v[3], doc.table ());
                break;
            case 7: // sections: sections "[" "[" keypath "]" "]" ENDLINE statements
                value = merge_array (doc, v[1], v[4], v[8]);
                break;
            case 8: // sections: sections "[" "[" keypath "]" "]" ENDLINE
                value = merge_array (doc, v[1], v[4], doc.table ());
                break;
            case 9: // sections: "[" keypath "]" ENDLINE statements
                value = merge_table (doc, doc.table (), v[2], v[5]);
                break;
            case 10: // sections: "[" keypath "]" ENDLINE
                value = merge_table (doc, doc.table (), v[3], doc.table ());
                break;
            case 11: // sections: "[" "[" keypath "]" "]" ENDLINE statements
                value = merge_array (doc, doc.table (), v[3], v[7]);
                break;
            case 12: // sections: "[" "[" keypath "]" "]" ENDLINE
                value = merge_array (doc, doc.table (), v[3], doc.table ());
                break;
            case 13: // keypath: keypath "." key
                value = doc.set (v[1], doc.size (v[1]), v[3]);
                break;
            case 14: // keypath: key
                value = doc.set (doc.array (), 0, v[1]);
                break;
            case 17: // statements: statements pair ENDLINE
                value = merge_exclusive (doc, v[1], v[2]);
                break;
            case 25: // value: "[" array "]"
                value = v[2];
                break;
            case 26: // value: "{" table "}"
                kvstate = 2;
                value = v[2];
                break;
            case 30: // value_list: endln value endln
                value = doc.set (doc.array (), 0, v[2]);
                break;
            case 31: // value_list: value_list "," endln value endln
                value = doc.set_unify (v[1], doc.size (v[1]), v[4]);
                break;
            case 34: // table:
                value = doc.table ();
                break;
            case 38: // pair_list: pair_list "," pair
                value = merge_exclusive (doc, v[1], v[3]);
                break;
            case 39: // pair: key "=" value
                kvstate = 1;
                value = doc.set (doc.table (), doc.at_string (v[1]), v[3]);
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
                std::logic_error ("parser: GRAMMAR table error");
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
