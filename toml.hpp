#ifndef TOML_HPP
#define TOML_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <ostream>

namespace toml {

enum variation {
    VALUE_NULL,
    VALUE_BOOLEAN,
    VALUE_FIXNUM,
    VALUE_FLONUM,
    VALUE_DATETIME,
    VALUE_STRING,
    VALUE_ARRAY,
    VALUE_TABLE,
    VALUE_BROKEN_HEART
};

typedef std::size_t value_id;

struct cell_type {
    variation tag;
    std::size_t oop;
    cell_type (variation x, std::size_t y) : tag (x), oop (y) {}
};

class doc_type;
class table_type;
class array_type;

class node_type {
public:
    node_type (doc_type& x, std::shared_ptr<node_type> y, value_id z);
    virtual ~node_type ();
    virtual value_id forse (variation tag) = 0;
    virtual value_id object_id () const = 0;
    bool boolean () const;
    int64_t fixnum () const;
    double flonum () const;
    std::string datetime () const;
    std::string string () const;
    std::string str () const;

protected:
    doc_type& doc;
    std::shared_ptr<node_type> parent;
    value_id self;
};

class table_type : public node_type {
public:
    table_type (doc_type& x, std::shared_ptr<node_type> y, value_id z, std::string const& k);
    table_type& operator= (value_id other);
    table_type& operator= (std::string const& v);
    table_type operator[] (std::string const& k);
    array_type operator[] (std::size_t const ix);
    value_id forse (variation tag);
    value_id object_id () const;

protected:
    std::string key;
};

class array_type : public node_type {
public:
    array_type (doc_type& x, std::shared_ptr<node_type> y, value_id z, std::size_t const& ix);
    array_type& operator= (value_id other);
    array_type& operator= (std::string const& v);
    table_type operator[] (std::string const& k);
    array_type operator[] (std::size_t const ix);
    value_id forse (variation tag);
    value_id object_id () const;

protected:
    std::size_t index;
};

class doc_type {
public:
    value_id root;

    doc_type ();

    table_type operator[] (std::string const& k);

    value_id boolean (bool const x);
    value_id fixnum (int64_t const x);
    value_id flonum (double const x);
    value_id datetime (std::string const& x);
    value_id string (std::string const& x);
    value_id array ();
    value_id table ();

    variation at_tag (value_id const id) const;
    bool at_boolean (value_id const id) const;
    int64_t at_fixnum (value_id const id) const;
    double at_flonum (value_id const id) const;
    std::string at_datetime (value_id const id) const;
    std::string const& at_string (value_id const id) const;
    std::string& at_string (value_id const id);
    std::vector<value_id>& at_array (value_id const id);
    std::vector<value_id> const& at_array (value_id const id) const;
    std::map<std::string,value_id>& at_table (value_id const id);
    std::map<std::string,value_id> const& at_table (value_id const id) const;

    bool exists (value_id const id, std::string const& key) const;
    std::size_t size (value_id const id) const;
    value_id get (value_id const id, std::size_t const idx) const;
    value_id get (value_id const id, std::string const& key) const;
    value_id set (value_id const id, std::size_t const idx, value_id value);
    value_id set_unify (value_id const id, std::size_t const idx, value_id value);
    value_id set (value_id const id, std::string const& key, value_id value);

    bool decode_toml (std::string const& source);
    void encode_toml (std::ostream& out) const;
    void encode_json (std::ostream& out) const;
    std::string str (value_id const id) const;

private:
    std::vector<cell_type> cell;
    std::vector<int64_t> fixnums;
    std::vector<double> flonums;
    std::vector<std::string> strings;
    std::vector< std::vector<value_id> > arrays;
    std::vector< std::map<std::string,value_id> > tables;

    std::string encode_quoted (std::string::const_iterator s, std::string::const_iterator const e) const;
    std::string encode_quoted (std::string const& str) const;
    void encode_toml_section (std::ostream& out, value_id const id, std::vector<std::string>& path) const;
    void encode_toml_path (std::ostream& out, char const* lft, std::vector<std::string>& path, char const* rgt) const;
    void encode_toml_table_section (std::ostream& out, value_id const id, std::vector<std::string>& path) const;
    void encode_toml_key (std::ostream& out, std::string const& key) const;
    void encode_toml_flow (std::ostream& out, value_id const id) const;
    void encode_json (std::ostream& out, value_id const id) const;
};

}//namespace toml

std::ostream& operator<< (std::ostream& stream, toml::node_type const& x);

#endif
