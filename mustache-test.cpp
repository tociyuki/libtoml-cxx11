#include <string>
#include <sstream>
#include <iostream>
#include "value.hpp"
#include "json.hpp"
#include "mustache.hpp"
#include "taptests.hpp"

/* mustache template
 * see http://mustache.github.io/mustache.5.html
 *
 *      {{ key }}       html escape expand
 *      {{{ key }}}     raw expand
 *      {{& key }}      raw expand
 *      {{# key}}block{{/ key}}  for or if
 *      {{^ key}}block{{/ key}}  unless
 *      {{! comment out }}
 *
 *      key : [\w?!/.-]+
 *
 *      {{> filename}}  not implemented (partial)
 *      {{=<% %>=}}     not implemented (change delimiters)
 */

void
trim_bang (std::string& s)
{
    if (s.empty ())
        return;
    if (s[0] == '\n')
        s.erase (0, 1);
}

void
test_typical (test::simple& ts)
{
    std::string input_str (R"q(
Hello {{name}}
You have just won {{value}} dollars!
{{#in_ca}}
Well, {{taxed_value}} dollars, after taxes.
{{/in_ca}}
)q");
    trim_bang (input_str);
    std::string input_json (R"q(
{
  "name": "Chris",
  "value": 10000,
  "taxed_value": 6000.0,
  "in_ca": true
}
)q");
    std::string expected (R"q(
Hello Chris
You have just won 10000 dollars!
Well, 6000.0 dollars, after taxes.
)q");
    trim_bang (expected);
    wjson::mustache mustache;
    wjson::value_type param;
    std::ostringstream got;

    ts.ok (mustache.assemble (input_str), "typical assemble");
    ts.ok (wjson::decode_json (input_json, param), "typical decode");
    mustache.render (param, got);
    ts.ok (got.str () == expected, "typical render");
}

void
test_variables (test::simple& ts)
{
    std::string input_str (R"q(
* {{name}}
* {{age}}
* {{company}}
* {{{company}}}
)q");
    trim_bang (input_str);
    std::string input_json (R"q(
{
  "name": "Chris",
  "company": "<b>GitHub</b>"
}
)q");
    std::string expected (R"q(
* Chris
* 
* &lt;b&gt;GitHub&lt;/b&gt;
* <b>GitHub</b>
)q");
    trim_bang (expected);
    wjson::mustache mustache;
    wjson::value_type param;
    std::ostringstream got;

    ts.ok (mustache.assemble (input_str), "variables assemble");
    ts.ok (wjson::decode_json (input_json, param), "variables decode");
    mustache.render (param, got);
    ts.ok (got.str () == expected, "variables render");
}

void
test_sections_false_values (test::simple& ts)
{
    std::string input_str (R"q(
Shown.
{{#person}}
  Never shown!
{{/person}}
)q");
    trim_bang (input_str);
    std::string input_json (R"q(
{
  "person": false
}
)q");
    std::string expected (R"q(
Shown.
)q");
    trim_bang (expected);
    wjson::mustache mustache;
    wjson::value_type param;
    std::ostringstream got;

    ts.ok (mustache.assemble (input_str), "sections_false_values assemble");
    ts.ok (wjson::decode_json (input_json, param), "sections_false_values decode");
    mustache.render (param, got);
    ts.ok (got.str () == expected, "sections_false_values render");
}

void
test_sections_non_empty_lists (test::simple& ts)
{
    std::string input_str (R"q(
{{#repo}}
  <b>{{name}}</b>
{{/repo}}
)q");
    trim_bang (input_str);
    std::string input_json (R"q(
{
  "repo": [
    { "name": "resque" },
    { "name": "hub" },
    { "name": "rip" }
  ]
}
)q");
    std::string expected (R"q(
  <b>resque</b>
  <b>hub</b>
  <b>rip</b>
)q");
    trim_bang (expected);
    wjson::mustache mustache;
    wjson::value_type param;
    std::ostringstream got;

    ts.ok (mustache.assemble (input_str), "sections_non_empty_lists assemble");
    ts.ok (wjson::decode_json (input_json, param), "sections_non_empty_lists decode");
    mustache.render (param, got);
    ts.ok (got.str () == expected, "sections_non_empty_lists render");
}

void
test_sections_non_false_values (test::simple& ts)
{
    std::string input_str (R"q(
{{#person?}}
  Hi {{name}}!
{{/person?}}
)q");
    trim_bang (input_str);
    std::string input_json (R"q(
{
  "person?": { "name": "Jon" }
}
)q");
    std::string expected (R"q(
  Hi Jon!
)q");
    trim_bang (expected);
    wjson::mustache mustache;
    wjson::value_type param;
    std::ostringstream got;

    ts.ok (mustache.assemble (input_str), "sections_non_false_values assemble");
    ts.ok (wjson::decode_json (input_json, param), "sections_non_false_values decode");
    mustache.render (param, got);
    ts.ok (got.str () == expected, "sections_non_false_values render");
}

void
test_inverted_sections (test::simple& ts)
{
    std::string input_str (R"q(
{{#repo}}
  <b>{{name}}</b>
{{/repo}}
{{^repo}}
  No repos :(
{{/repo}}
)q");
    trim_bang (input_str);
    std::string input_json (R"q(
{
  "repo": []
}
)q");
    std::string expected (R"q(
  No repos :(
)q");
    trim_bang (expected);
    wjson::mustache mustache;
    wjson::value_type param;
    std::ostringstream got;

    ts.ok (mustache.assemble (input_str), "inverted_sections assemble");
    ts.ok (wjson::decode_json (input_json, param), "inverted_sections decode");
    mustache.render (param, got);
    ts.ok (got.str () == expected, "inverted_sections render");
}

void
test_comments (test::simple& ts)
{
    std::string input_str (R"q(
<h1>Today{{! ignore me }}.</h1>
)q");
    trim_bang (input_str);
    std::string input_json (R"q(
{
  "repo": []
}
)q");
    std::string expected (R"q(
<h1>Today.</h1>
)q");
    trim_bang (expected);
    wjson::mustache mustache;
    wjson::value_type param;
    std::ostringstream got;

    ts.ok (mustache.assemble (input_str), "comments assemble");
    ts.ok (wjson::decode_json (input_json, param), "comments decode");
    mustache.render (param, got);
    ts.ok (got.str () == expected, "comments render");
}

int
main ()
{
    test::simple ts (21);

    test_typical (ts);
    test_variables (ts);
    test_sections_false_values (ts);
    test_sections_non_empty_lists (ts);
    test_sections_non_false_values (ts);
    test_inverted_sections (ts);
    test_comments (ts);

    return ts.done_testing ();
}

