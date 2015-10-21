#include <string>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <iostream>
#include "../../value.hpp"
#include "../../encode-quoted.hpp"

namespace wjson {

enum {
    TOKEN_INVALID,
    TOKEN_FIXNUM,
    TOKEN_FLONUM,
    TOKEN_DATETIME,
};

static inline int
lookup_cls (uint32_t const tbl[], std::size_t const n, uint32_t const octet)
{
    uint32_t const i = octet >> 3;
    uint32_t const count = (7 - (octet & 7)) << 2;
    return octet < n ? ((tbl[i] >> count) & 0x0f) : 0;
}

static inline uint32_t
ord (char const c)
{
    return static_cast<uint8_t> (c);
}

#include "yaml-decoder-number.cpp"

}//namespace wjson

#include "../../taptests.hpp"

bool
almost (double x, double y)
{
    return std::abs (x - y) <=
        std::numeric_limits<double>::epsilon ()
            * std::min (std::abs (x), std::abs (y));
}

void
test_fixnum_zero (test::simple& ts)
{
    std::string input ("0");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FIXNUM, "0 scan");
    ts.ok (value.fixnum () == 0, "0 fixnum");
}

void
test_fixnum_pluszero (test::simple& ts)
{
    std::string input ("+0");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FIXNUM, "+0 scan");
    ts.ok (value.fixnum () == 0, "+0 fixnum");
}

void
test_fixnum_minuszero (test::simple& ts)
{
    std::string input ("-0");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FIXNUM, "-0 scan");
    ts.ok (value.fixnum () == 0, "-0 fixnum");
}

void
test_fixnum_1 (test::simple& ts)
{
    std::string input ("1");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FIXNUM, "1 scan");
    ts.ok (value.fixnum () == 1, "1 fixnum");
}

void
test_fixnum_plus1 (test::simple& ts)
{
    std::string input ("+1");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FIXNUM, "+1 scan");
    ts.ok (value.fixnum () == 1, "+1 fixnum");
}

void
test_fixnum_minus1 (test::simple& ts)
{
    std::string input ("-1");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FIXNUM, "-1 scan");
    ts.ok (value.fixnum () == -1, "-1 fixnum");
}

void
test_fixnum_11235 (test::simple& ts)
{
    std::string input ("11235");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FIXNUM, "11235 scan");
    ts.ok (value.fixnum () == 11235, "11235 fixnum");
}

void
test_fixnum_plus11235 (test::simple& ts)
{
    std::string input ("+11235");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FIXNUM, "+11235 scan");
    ts.ok (value.fixnum () == 11235, "+11235 fixnum");
}

void
test_fixnum_minus11235 (test::simple& ts)
{
    std::string input ("-11235");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FIXNUM, "-11235 scan");
    ts.ok (value.fixnum () == -11235, "-11235 fixnum");
}

void
test_fixnum_0o11235 (test::simple& ts)
{
    std::string input ("0o11235");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FIXNUM, "0o11235 scan");
    ts.ok (value.fixnum () == 011235, "0o11235 fixnum");
}

void
test_fixnum_0x11235 (test::simple& ts)
{
    std::string input ("0x11235");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FIXNUM, "0x11235 scan");
    ts.ok (value.fixnum () == 0x11235, "0x11235 fixnum");
}

void
test_flonum_0p0 (test::simple& ts)
{
    std::string input ("0.0");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FLONUM, "0.0 scan");
    ts.ok (almost (value.flonum (), 0.0), "0.0 flonum");
}

void
test_flonum_p0 (test::simple& ts)
{
    std::string input (".0");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FLONUM, ".0 scan");
    ts.ok (almost (value.flonum (), 0.0), ".0 flonum");
}

void
test_flonum_0p (test::simple& ts)
{
    std::string input ("0.");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FLONUM, "0. scan");
    ts.ok (almost (value.flonum (), 0.0), "0. flonum");
}

void
test_flonum_plus0p0 (test::simple& ts)
{
    std::string input ("+0.0");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FLONUM, "+0.0 scan");
    ts.ok (almost (value.flonum (), 0.0), "0.0 flonum");
}

void
test_flonum_plusp0 (test::simple& ts)
{
    std::string input ("+.0");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FLONUM, "+.0 scan");
    ts.ok (almost (value.flonum (), 0.0), "+.0 flonum");
}

void
test_flonum_plus0p (test::simple& ts)
{
    std::string input ("+0.");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FLONUM, "+0. scan");
    ts.ok (almost (value.flonum (), 0.0), "+0. flonum");
}

void
test_flonum_minus0p0 (test::simple& ts)
{
    std::string input ("-0.0");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FLONUM, "-0.0 scan");
    ts.ok (almost (value.flonum (), 0.0), "-0.0 flonum");
}

void
test_flonum_minusp0 (test::simple& ts)
{
    std::string input ("-.0");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FLONUM, "-.0 scan");
    ts.ok (almost (value.flonum (), 0.0), "-.0 flonum");
}

void
test_flonum_minus0p (test::simple& ts)
{
    std::string input ("-0.");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FLONUM, "-0. scan");
    ts.ok (almost (value.flonum (), 0.0), "-0. flonum");
}

void
test_flonum_conical (test::simple& ts)
{
    std::string input ("1.23015e+3");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FLONUM, "1.23015e+3 scan");
    ts.ok (almost (value.flonum (), 1.23015e+3), "1.23015e+3 flonum");
}

void
test_flonum_exponential (test::simple& ts)
{
    std::string input ("12.3015e+02");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FLONUM, "12.3015e+02 scan");
    ts.ok (almost (value.flonum (), 12.3015e+02), "12.3015e+02 flonum");
}

void
test_flonum_fixed (test::simple& ts)
{
    std::string input ("1230.15");
    wjson::value_type value;
    ts.ok (wjson::scan_number (input, value) == wjson::TOKEN_FLONUM, "1230.15 scan");
    ts.ok (almost (value.flonum (), 1230.15), "1230.15 flonum");
}

int
main ()
{
    test::simple ts;

    test_fixnum_zero (ts);
    test_fixnum_pluszero (ts);
    test_fixnum_minuszero (ts);
    test_fixnum_1 (ts);
    test_fixnum_plus1 (ts);
    test_fixnum_minus1 (ts);
    test_fixnum_11235 (ts);
    test_fixnum_plus11235 (ts);
    test_fixnum_minus11235 (ts);
    test_fixnum_0o11235 (ts);
    test_fixnum_0x11235 (ts);

    test_flonum_0p0 (ts);
    test_flonum_p0 (ts);
    test_flonum_0p (ts);
    test_flonum_plus0p0 (ts);
    test_flonum_plusp0 (ts);
    test_flonum_plus0p (ts);
    test_flonum_minus0p0 (ts);
    test_flonum_minusp0 (ts);
    test_flonum_minus0p (ts);

    test_flonum_conical (ts);
    test_flonum_exponential (ts);
    test_flonum_fixed (ts);

    return ts.done_testing ();
}
