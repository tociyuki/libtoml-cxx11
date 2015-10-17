#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <utility>
#include "toml.hpp"

namespace toml {

node_type::node_type (doc_type& x, std::shared_ptr<node_type> y, value_id z)
    : doc (x), parent (y), self (z)
{
}

node_type::~node_type ()
{
}

variation
node_type::tag () const
{
    return doc.at_tag (object_id ());
}

bool
node_type::boolean () const
{
    return doc.at_boolean (object_id ());
}

int64_t
node_type::fixnum () const
{
    return doc.at_fixnum (object_id ());
}

double
node_type::flonum () const
{
    return doc.at_flonum (object_id ());
}

std::string
node_type::datetime () const
{
    return doc.at_datetime (object_id ());
}

std::string
node_type::string () const
{
    return doc.at_string (object_id ());
}

std::string
node_type::str () const
{
    return doc.str (object_id ());
}

table_type::table_type (doc_type& x, std::shared_ptr<node_type> y, value_id z, std::string const& k)
    : node_type (x, y, z), key (k)
{
}

table_type&
table_type::operator= (value_id other)
{
    if (! self)
        self = parent->forse (VALUE_TABLE);
    doc.set (self, key, other);
    return *this;
}

table_type&
table_type::operator= (std::string const& v)
{
    if (! self)
        self = parent->forse (VALUE_TABLE);
    value_id str = doc.string (v);
    doc.set (self, key, str);
    return *this;
}

value_id
table_type::forse (variation tag)
{
    if (! self)
        self = parent->forse (VALUE_TABLE);
    value_id child = doc.get (self, key);
    if (! child) {
        if (VALUE_TABLE == tag)
            child = doc.table ();
        else
            child = doc.array ();
        doc.set (self, key, child);
    }
    return child;
}

table_type
table_type::operator[] (std::string const& k)
{
    value_id child = doc.get (self, key);
    if (child && doc.at_tag (child) != VALUE_TABLE)
        throw std::logic_error ("table_type::operator[]:not a table");
    auto p = std::make_shared<table_type> (doc, parent, self, key);
    return table_type {doc, p, child, k};
}

array_type
table_type::operator[] (std::size_t const ix)
{
    value_id child = doc.get (self, key);
    if (child && doc.at_tag (child) != VALUE_ARRAY)
        throw std::logic_error ("table_type::operator[]:not an array");
    auto p = std::make_shared<table_type> (doc, parent, self, key);
    return array_type {doc, p, child, ix};
}

value_id
table_type::object_id () const
{
    return doc.get (self, key);
}

array_type::array_type (doc_type& x, std::shared_ptr<node_type> y, value_id z, std::size_t const& ix)
    : node_type (x, y, z), index (ix)
{
}

array_type&
array_type::operator= (value_id other)
{
    if (! self)
        self = parent->forse (VALUE_ARRAY);
    doc.set (self, index, other);
    return *this;
}

array_type&
array_type::operator= (std::string const& v)
{
    if (! self)
        self = parent->forse (VALUE_ARRAY);
    value_id str = doc.string (v);
    doc.set (self, index, str);
    return *this;
}

value_id
array_type::forse (variation tag)
{
    if (! self)
        self = parent->forse (VALUE_ARRAY);
    value_id child = doc.get (self, index);
    if (! child) {
        if (VALUE_TABLE == tag)
            child = doc.table ();
        else
            child = doc.array ();
        doc.set (self, index, child);
    }
    return child;
}

table_type
array_type::operator[] (std::string const& k)
{
    value_id child = doc.get (self, index);
    if (child && doc.at_tag (child) != VALUE_TABLE)
        throw std::logic_error ("table_type::operator[]:not a table");
    auto p = std::make_shared<array_type> (doc, parent, self, index);
    return table_type {doc, p, child, k};
}

array_type
array_type::operator[] (std::size_t const ix)
{
    value_id child = doc.get (self, index);
    if (child && doc.at_tag (child) != VALUE_ARRAY)
        throw std::logic_error ("table_type::operator[]:not an array");
    auto p = std::make_shared<array_type> (doc, parent, self, index);
    return array_type {doc, p, child, ix};
}

value_id
array_type::object_id () const
{
    return doc.get (self, index);
}

doc_type::doc_type ()
    : root (0), cell (), fixnums (), flonums (), strings (), arrays (), tables ()
{
    cell.emplace_back (VALUE_NULL, 0);
}

table_type
doc_type::operator[] (std::string const& k)
{
    return table_type {*this, nullptr, root, k};
}

variation
doc_type::at_tag (value_id const id) const
{
    return id && id < cell.size () ? cell.at (id).tag : VALUE_NULL;
}

value_id
doc_type::boolean (bool const x)
{
    value_id id = cell.size ();
    cell.emplace_back (VALUE_BOOLEAN, x);
    return id;
}

bool
doc_type::at_boolean (value_id const id) const
{
    if (at_tag (id) == VALUE_BOOLEAN)
        return cell.at (id).oop;
    throw std::out_of_range ("at_boolean: invalid type");
}

value_id
doc_type::fixnum (int64_t const x)
{
    std::size_t oop = fixnums.size ();
    fixnums.emplace_back (x);
    value_id id = cell.size ();
    cell.emplace_back (VALUE_FIXNUM, oop);
    return id;
}

int64_t
doc_type::at_fixnum (value_id const id) const
{
    if (at_tag (id) == VALUE_FIXNUM)
        return fixnums.at (cell.at (id).oop);
    throw std::out_of_range ("at_fixnum: invalid type");
}

value_id
doc_type::flonum (double const x)
{
    std::size_t oop = flonums.size ();
    flonums.emplace_back (x);
    value_id id = cell.size ();
    cell.emplace_back (VALUE_FLONUM, oop);
    return id;
}

double
doc_type::at_flonum (value_id const id) const
{
    if (at_tag (id) == VALUE_FLONUM)
        return flonums.at (cell.at (id).oop);
    throw std::out_of_range ("at_flonum: invalid type");
}

value_id
doc_type::datetime (std::string const& x)
{
    std::size_t oop = strings.size ();
    strings.emplace_back (x);
    value_id id = cell.size ();
    cell.emplace_back (VALUE_DATETIME, oop);
    return id;
}

std::string
doc_type::at_datetime (value_id const id) const
{
    if (at_tag (id) == VALUE_DATETIME)
        return strings.at (cell.at (id).oop);
    throw std::out_of_range ("at_datetime: invalid type");
}

value_id
doc_type::string (std::string const& x)
{
    std::size_t oop = strings.size ();
    strings.emplace_back (x);
    value_id id = cell.size ();
    cell.emplace_back (VALUE_STRING, oop);
    return id;
}

std::string const&
doc_type::at_string (value_id const id) const
{
    if (at_tag (id) == VALUE_STRING)
        return strings.at (cell.at (id).oop);
    throw std::out_of_range ("at_string: invalid type");
}

std::string&
doc_type::at_string (value_id const id)
{
    if (at_tag (id) == VALUE_STRING)
        return strings.at (cell.at (id).oop);
    throw std::out_of_range ("at_string: invalid type");
}

value_id
doc_type::array ()
{
    std::size_t oop = arrays.size ();
    arrays.push_back ({});
    value_id id = cell.size ();
    cell.emplace_back (VALUE_ARRAY, oop);
    return id;
}

std::vector<value_id>&
doc_type::at_array (value_id const id)
{
    if (at_tag (id) == VALUE_ARRAY)
        return arrays.at (cell.at (id).oop);
    throw std::out_of_range ("at_array: invalid type");
}

std::vector<value_id> const&
doc_type::at_array (value_id const id) const
{
    if (at_tag (id) == VALUE_ARRAY)
        return arrays.at (cell.at (id).oop);
    throw std::out_of_range ("at_array: invalid type");
}

value_id
doc_type::set (value_id const id, std::size_t const idx, value_id value)
{
    if (at_tag (id) != VALUE_ARRAY)
        throw std::out_of_range ("set array: invalid type");
    if (idx >= at_array (id).size ())
        at_array (id).resize (idx + 1, 0);
    at_array (id)[idx] = value;
    return id;
}

value_id
doc_type::set_unify (value_id const id, std::size_t const idx, value_id value)
{
    if (at_tag (id) != VALUE_ARRAY)
        throw std::out_of_range ("set_unify: invalid type");
    std::size_t n = size (id);
    if (n > 0 && at_tag (get (id, 0)) != at_tag (value))
        throw std::out_of_range ("set_unify: different type");
    if (idx >= at_array (id).size ())
        at_array (id).resize (idx + 1, 0);
    at_array (id)[idx] = value;
    return id;
}

value_id
doc_type::get (value_id const id, std::size_t const idx) const
{
    if (id && at_tag (id) != VALUE_ARRAY)
        throw std::out_of_range ("get array: invalid type");
    return id && idx < at_array (id).size () ? at_array (id).at (idx) : 0;
}

value_id
doc_type::table ()
{
    std::size_t oop = tables.size ();
    tables.push_back ({});
    value_id id = cell.size ();
    cell.emplace_back (VALUE_TABLE, oop);
    return id;
}

std::map<std::string,value_id>&
doc_type::at_table (value_id const id)
{
    if (at_tag (id) == VALUE_TABLE)
        return tables.at (cell.at (id).oop);
    throw std::out_of_range ("at_table: invalid type");
}

std::map<std::string,value_id> const&
doc_type::at_table (value_id const id) const
{
    if (at_tag (id) == VALUE_TABLE)
        return tables.at (cell.at (id).oop);
    throw std::out_of_range ("at_table: invalid type");
}

value_id
doc_type::set (value_id const id, std::string const& key, value_id value)
{
    if (at_tag (id) != VALUE_TABLE)
        throw std::out_of_range ("set table: invalid type");
    at_table (id)[key] = value;
    return id;
}

value_id
doc_type::get (value_id const id, std::string const& key) const
{
    if (id && at_tag (id) != VALUE_TABLE)
        throw std::out_of_range ("get table: invalid type");
    return id && at_table (id).count (key) > 0 ? at_table (id).at (key) : 0;
}

bool
doc_type::exists (value_id const id, std::string const& key) const
{
    if (id && at_tag (id) != VALUE_TABLE)
        throw std::out_of_range ("exists table: invalid type");
    return id && at_table (id).count (key) > 0;
}

std::size_t
doc_type::size (value_id const id) const
{
    switch (at_tag (id)) {
    case VALUE_TABLE:  return at_table (id).size ();
    case VALUE_ARRAY:  return at_array (id).size ();
    case VALUE_STRING: return at_string (id).size ();
    default: return 0;
    }
}

}
