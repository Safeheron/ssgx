#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <set>
#include <array>
#include <tuple>
#include <utility>
#include <stdexcept>
#include <cstdint>
#include <cmath>
#include <limits>

#include <fmt/format.h>
#include <fmt/args.h>
#include <fmt/ranges.h>
#include <fmt/compile.h>

// ─── SGX porting invariants (any violation fails the build) ─────────────────
#ifndef FMT_NO_IO
#  error "FMT_NO_IO must be defined for SGX enclave; check base.h preset"
#endif
static_assert(FMT_USE_LOCALE == 0,
              "FMT_USE_LOCALE must be 0 in SGX enclave");
static_assert(FMT_USE_EXCEPTIONS,
              "fmt in SGX requires C++ exceptions (do not use -fno-exceptions)");
static_assert(!FMT_USE_FLOAT128,
              "__float128 unsupported in SGX (no libquadmath)");

#include "ssgx_testframework_t.h"

// ─── Custom types for FmtCustom suite ────────────────────────────────────────

struct Point {
    int x, y;
};

template <>
struct fmt::formatter<Point> {
    constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }
    auto format(const Point& p, fmt::format_context& ctx) const {
        return fmt::format_to(ctx.out(), FMT_STRING("({}, {})"), p.x, p.y);
    }
};

// Formatter with custom format spec: {: .Nf} maps to N-digit precision
struct Vec3 {
    double x, y, z;
};

template <>
struct fmt::formatter<Vec3> {
    int prec = 2;
    constexpr auto parse(fmt::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it >= '0' && *it <= '9') {
            prec = *it++ - '0';
        }
        return it;
    }
    auto format(const Vec3& v, fmt::format_context& ctx) const {
        return fmt::format_to(ctx.out(), "({:.{}f}, {:.{}f}, {:.{}f})",
                              v.x, prec, v.y, prec, v.z, prec);
    }
};

// Formatter with hex/dec spec
struct Rgb {
    uint8_t r, g, b;
};

template <>
struct fmt::formatter<Rgb> {
    bool hex = true;
    constexpr auto parse(fmt::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it == 'd') { hex = false; ++it; }
        else if (it != ctx.end() && *it == 'x') { ++it; }
        return it;
    }
    auto format(const Rgb& c, fmt::format_context& ctx) const {
        if (hex)
            return fmt::format_to(ctx.out(), "#{:02X}{:02X}{:02X}", c.r, c.g, c.b);
        else
            return fmt::format_to(ctx.out(), "rgb({}, {}, {})", c.r, c.g, c.b);
    }
};

// ─── FmtBasic ────────────────────────────────────────────────────────────────

TEST(FmtBasic, FormatInteger) {
    auto r1 = fmt::format(FMT_STRING("{}"), 42);
    auto r2 = fmt::format(FMT_STRING("{}"), -1);
    auto r3 = fmt::format(FMT_STRING("{}"), 0);
    ssgx::utils_t::Printf("  fmt(\"{}\", 42)  => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{}\", -1)  => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  fmt(\"{}\", 0)   => \"%s\"\n", r3.c_str());
    ASSERT_STR_EQ(r1, "42");
    ASSERT_STR_EQ(r2, "-1");
    ASSERT_STR_EQ(r3, "0");
}

TEST(FmtBasic, FormatString) {
    auto r1 = fmt::format(FMT_STRING("{}"), "hello");
    auto r2 = fmt::format(FMT_STRING("{}world"), "hello ");
    auto r3 = fmt::format(FMT_STRING("{0}{0}"), "ab");
    ssgx::utils_t::Printf("  fmt(\"{}\", \"hello\")        => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{}world\", \"hello \")  => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  fmt(\"{0}{0}\", \"ab\")       => \"%s\"\n", r3.c_str());
    ASSERT_STR_EQ(r1, "hello");
    ASSERT_STR_EQ(r2, "hello world");
    ASSERT_STR_EQ(r3, "abab");
}

TEST(FmtBasic, FormatFloat) {
    auto r1 = fmt::format(FMT_STRING("{:.2f}"), 3.14159);
    auto r2 = fmt::format(FMT_STRING("{:.0f}"), 2.5);
    auto r3 = fmt::format(FMT_STRING("{:e}"), 1000.0);
    ssgx::utils_t::Printf("  fmt(\"{:.2f}\", 3.14159)  => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:.0f}\", 2.5)      => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:e}\", 1000.0)     => \"%s\"\n", r3.c_str());
    ASSERT_STR_EQ(r1, "3.14");
    ASSERT_STR_EQ(r2, "2");
    ASSERT_STR_EQ(r3, "1.000000e+03");
}

TEST(FmtBasic, FormatBool) {
    auto r1 = fmt::format(FMT_STRING("{}"), true);
    auto r2 = fmt::format(FMT_STRING("{}"), false);
    ssgx::utils_t::Printf("  fmt(\"{}\", true)   => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{}\", false)  => \"%s\"\n", r2.c_str());
    ASSERT_STR_EQ(r1, "true");
    ASSERT_STR_EQ(r2, "false");
}

TEST(FmtBasic, FormatChar) {
    auto r1 = fmt::format(FMT_STRING("{}"), 'A');
    auto r2 = fmt::format(FMT_STRING("{:d}"), 'A');   // char as decimal
    auto r3 = fmt::format(FMT_STRING("{:#x}"), 'A');  // char as hex
    ssgx::utils_t::Printf("  fmt(\"{}\",   'A')   => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:d}\", 'A')   => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:#x}\",'A')   => \"%s\"\n", r3.c_str());
    ASSERT_STR_EQ(r1, "A");
    ASSERT_STR_EQ(r2, "65");
    ASSERT_STR_EQ(r3, "0x41");
}

TEST(FmtBasic, FormatUnsigned) {
    auto r1 = fmt::format(FMT_STRING("{}"), 255u);
    auto r2 = fmt::format(FMT_STRING("{}"), std::size_t(1024));
    auto r3 = fmt::format(FMT_STRING("{}"), uint8_t(200));
    ssgx::utils_t::Printf("  fmt(\"{}\", 255u)          => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{}\", size_t(1024))  => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  fmt(\"{}\", uint8_t(200))  => \"%s\"\n", r3.c_str());
    ASSERT_STR_EQ(r1, "255");
    ASSERT_STR_EQ(r2, "1024");
    ASSERT_STR_EQ(r3, "200");
}

TEST(FmtBasic, FormatLargeInt) {
    auto r1 = fmt::format(FMT_STRING("{}"), int64_t(9223372036854775807LL));   // INT64_MAX
    auto r2 = fmt::format(FMT_STRING("{}"), uint64_t(18446744073709551615ULL)); // UINT64_MAX
    auto r3 = fmt::format(FMT_STRING("{}"), int64_t(-9223372036854775807LL - 1)); // INT64_MIN
    ssgx::utils_t::Printf("  INT64_MAX   => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  UINT64_MAX  => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  INT64_MIN   => \"%s\"\n", r3.c_str());
    ASSERT_STR_EQ(r1, "9223372036854775807");
    ASSERT_STR_EQ(r2, "18446744073709551615");
    ASSERT_STR_EQ(r3, "-9223372036854775808");
}

TEST(FmtBasic, MultipleArgs) {
    auto r1 = fmt::format(FMT_STRING("{} + {} = {}"), 1, 2, 3);
    auto r2 = fmt::format(FMT_STRING("{2}{1}{0}"), 'c', 'b', 'a');
    auto r3 = fmt::format(FMT_STRING("{0} {1} {0}"), "ping", "pong");
    ssgx::utils_t::Printf("  fmt(\"{} + {} = {}\", 1, 2, 3)         => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{2}{1}{0}\", 'c','b','a')        => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  fmt(\"{0} {1} {0}\", \"ping\",\"pong\") => \"%s\"\n", r3.c_str());
    ASSERT_STR_EQ(r1, "1 + 2 = 3");
    ASSERT_STR_EQ(r2, "abc");
    ASSERT_STR_EQ(r3, "ping pong ping");
}

// ─── FmtFloat ────────────────────────────────────────────────────────────────

TEST(FmtFloat, GeneralFormat) {
    auto r1 = fmt::format(FMT_STRING("{:g}"), 100.0);
    auto r2 = fmt::format(FMT_STRING("{:g}"), 0.0001);
    auto r3 = fmt::format(FMT_STRING("{:G}"), 1234567.0);
    ssgx::utils_t::Printf("  fmt(\"{:g}\",  100.0)     => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:g}\",  0.0001)    => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:G}\",  1234567.0) => \"%s\"\n", r3.c_str());
    ASSERT_STR_EQ(r1, "100");
    ASSERT_STR_EQ(r2, "0.0001");
    ASSERT_STR_EQ(r3, "1.23457E+06");
}

TEST(FmtFloat, ExponentNotation) {
    auto r1 = fmt::format(FMT_STRING("{:e}"), 0.001);
    auto r2 = fmt::format(FMT_STRING("{:E}"), 0.001);
    auto r3 = fmt::format(FMT_STRING("{:.2e}"), 123456.789);
    ssgx::utils_t::Printf("  fmt(\"{:e}\",  0.001)       => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:E}\",  0.001)       => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:.2e}\",123456.789)  => \"%s\"\n", r3.c_str());
    ASSERT_STR_EQ(r1, "1.000000e-03");
    ASSERT_STR_EQ(r2, "1.000000E-03");
    ASSERT_STR_EQ(r3, "1.23e+05");
}

TEST(FmtFloat, WidthAndPrecision) {
    auto r1 = fmt::format(FMT_STRING("{:10.3f}"), 3.14);
    auto r2 = fmt::format(FMT_STRING("{:<10.3f}"), 3.14);
    auto r3 = fmt::format(FMT_STRING("{:010.3f}"), 3.14);
    ssgx::utils_t::Printf("  fmt(\"{:10.3f}\",  3.14)  => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:<10.3f}\", 3.14)  => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:010.3f}\", 3.14)  => \"%s\"\n", r3.c_str());
    ASSERT_STR_EQ(r1, "     3.140");
    ASSERT_STR_EQ(r2, "3.140     ");
    ASSERT_STR_EQ(r3, "000003.140");
}

TEST(FmtFloat, SignOptions) {
    auto r1 = fmt::format(FMT_STRING("{:+.2f}"), 3.14);
    auto r2 = fmt::format(FMT_STRING("{:+.2f}"), -3.14);
    auto r3 = fmt::format(FMT_STRING("{: .2f}"), 3.14);   // space for positive
    auto r4 = fmt::format(FMT_STRING("{: .2f}"), -3.14);
    ssgx::utils_t::Printf("  fmt(\"{:+.2f}\", 3.14)   => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:+.2f}\", -3.14)  => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  fmt(\"{: .2f}\", 3.14)   => \"%s\"\n", r3.c_str());
    ssgx::utils_t::Printf("  fmt(\"{: .2f}\", -3.14)  => \"%s\"\n", r4.c_str());
    ASSERT_STR_EQ(r1, "+3.14");
    ASSERT_STR_EQ(r2, "-3.14");
    ASSERT_STR_EQ(r3, " 3.14");
    ASSERT_STR_EQ(r4, "-3.14");
}

TEST(FmtFloat, SpecialValues) {
    double inf = std::numeric_limits<double>::infinity();
    double nan = std::numeric_limits<double>::quiet_NaN();
    auto r1 = fmt::format(FMT_STRING("{}"), inf);
    auto r2 = fmt::format(FMT_STRING("{}"), -inf);
    auto r3 = fmt::format(FMT_STRING("{}"), nan);
    ssgx::utils_t::Printf("  fmt(\"{}\",  +inf) => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{}\",  -inf) => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  fmt(\"{}\",  nan)  => \"%s\"\n", r3.c_str());
    ASSERT_STR_EQ(r1, "inf");
    ASSERT_STR_EQ(r2, "-inf");
    ASSERT_STR_EQ(r3, "nan");
}

TEST(FmtFloat, SmallAndLarge) {
    auto r1 = fmt::format(FMT_STRING("{:.2f}"), 0.0);
    auto r2 = fmt::format(FMT_STRING("{:.10f}"), 1.0 / 3.0);
    auto r3 = fmt::format(FMT_STRING("{:.2f}"), -0.001);
    ssgx::utils_t::Printf("  fmt(\"{:.2f}\",   0.0)      => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:.10f}\",  1.0/3.0)  => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:.2f}\",   -0.001)   => \"%s\"\n", r3.c_str());
    ASSERT_STR_EQ(r1, "0.00");
    ASSERT_STR_EQ(r2, "0.3333333333");
    ASSERT_STR_EQ(r3, "-0.00");
}

// ─── FmtWidth ────────────────────────────────────────────────────────────────

TEST(FmtWidth, PadRight) {
    auto r = fmt::format(FMT_STRING("{:<10}"), "left");
    ssgx::utils_t::Printf("  fmt(\"{:<10}\", \"left\")  => \"%s\"\n", r.c_str());
    ASSERT_STR_EQ(r, "left      ");
}

TEST(FmtWidth, PadCenter) {
    auto r = fmt::format(FMT_STRING("{:^10}"), "mid");
    ssgx::utils_t::Printf("  fmt(\"{:^10}\", \"mid\")  => \"%s\"\n", r.c_str());
    ASSERT_STR_EQ(r, "   mid    ");
}

TEST(FmtWidth, PadLeft) {
    auto r = fmt::format(FMT_STRING("{:>10}"), "right");
    ssgx::utils_t::Printf("  fmt(\"{:>10}\", \"right\")  => \"%s\"\n", r.c_str());
    ASSERT_STR_EQ(r, "     right");
}

TEST(FmtWidth, FillChar) {
    auto r1 = fmt::format(FMT_STRING("{:*>8}"), 42);
    auto r2 = fmt::format(FMT_STRING("{:0>6}"), 7);
    auto r3 = fmt::format(FMT_STRING("{:-^12}"), "hello");
    ssgx::utils_t::Printf("  fmt(\"{:*>8}\",   42)      => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:0>6}\",   7)       => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:-^12}\", \"hello\") => \"%s\"\n", r3.c_str());
    ASSERT_STR_EQ(r1, "******42");
    ASSERT_STR_EQ(r2, "000007");
    ASSERT_STR_EQ(r3, "---hello----");
}

TEST(FmtWidth, SignedIntegers) {
    auto r1 = fmt::format(FMT_STRING("{:+}"), 42);
    auto r2 = fmt::format(FMT_STRING("{:+}"), -42);
    auto r3 = fmt::format(FMT_STRING("{: }"), 42);   // space prefix for positive
    ssgx::utils_t::Printf("  fmt(\"{:+}\", 42)   => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:+}\", -42)  => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  fmt(\"{: }\", 42)   => \"%s\"\n", r3.c_str());
    ASSERT_STR_EQ(r1, "+42");
    ASSERT_STR_EQ(r2, "-42");
    ASSERT_STR_EQ(r3, " 42");
}

TEST(FmtWidth, ZeroPadding) {
    auto r1 = fmt::format(FMT_STRING("{:05d}"), 42);
    auto r2 = fmt::format(FMT_STRING("{:08.2f}"), 3.14);
    auto r3 = fmt::format(FMT_STRING("{:05d}"), -7);
    ssgx::utils_t::Printf("  fmt(\"{:05d}\",   42)    => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:08.2f}\", 3.14)  => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:05d}\",   -7)    => \"%s\"\n", r3.c_str());
    ASSERT_STR_EQ(r1, "00042");
    ASSERT_STR_EQ(r2, "00003.14");
    ASSERT_STR_EQ(r3, "-0007");
}

// ─── FmtNested (dynamic width / precision) ───────────────────────────────────

TEST(FmtNested, DynamicWidth) {
    int w = 10;
    auto r1 = fmt::format(FMT_STRING("{:{}}"), "hello", w);       // left-align (default for strings)
    auto r2 = fmt::format(FMT_STRING("{:>{}}"), "hello", w);      // right-align explicit
    auto r3 = fmt::format(FMT_STRING("{:^{}}"), "hi", w);         // center
    ssgx::utils_t::Printf("  fmt(\"{:{}}\",  \"hello\", 10)  => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:>{}}\", \"hello\", 10)  => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:^{}}\", \"hi\",    10)  => \"%s\"\n", r3.c_str());
    ASSERT_STR_EQ(r1, "hello     ");
    ASSERT_STR_EQ(r2, "     hello");
    ASSERT_STR_EQ(r3, "    hi    ");
}

TEST(FmtNested, DynamicPrecision) {
    int p = 3;
    auto r1 = fmt::format(FMT_STRING("{:.{}f}"), 3.14159, p);
    auto r2 = fmt::format(FMT_STRING("{:.{}f}"), 3.14159, 0);
    ssgx::utils_t::Printf("  fmt(\"{:.{}f}\", 3.14159, 3)  => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:.{}f}\", 3.14159, 0)  => \"%s\"\n", r2.c_str());
    ASSERT_STR_EQ(r1, "3.142");
    ASSERT_STR_EQ(r2, "3");
}

TEST(FmtNested, DynamicWidthAndPrecision) {
    auto r = fmt::format(FMT_STRING("{:{}.{}f}"), 3.14159, 10, 2);
    ssgx::utils_t::Printf("  fmt(\"{:{}.{}f}\", 3.14159, 10, 2)  => \"%s\"\n", r.c_str());
    ASSERT_STR_EQ(r, "      3.14");
}

// ─── FmtRadix ────────────────────────────────────────────────────────────────

TEST(FmtRadix, Hex) {
    auto r1 = fmt::format(FMT_STRING("{:x}"), 255);
    auto r2 = fmt::format(FMT_STRING("{:X}"), 255);
    auto r3 = fmt::format(FMT_STRING("{:#x}"), 255);
    auto r4 = fmt::format(FMT_STRING("{:08x}"), 0xDEADBEEF);
    ssgx::utils_t::Printf("  fmt(\"{:x}\",  255)         => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:X}\",  255)         => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:#x}\", 255)         => \"%s\"\n", r3.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:08x}\", 0xDEADBEEF) => \"%s\"\n", r4.c_str());
    ASSERT_STR_EQ(r1, "ff");
    ASSERT_STR_EQ(r2, "FF");
    ASSERT_STR_EQ(r3, "0xff");
    ASSERT_STR_EQ(r4, "deadbeef");
}

TEST(FmtRadix, Octal) {
    auto r1 = fmt::format(FMT_STRING("{:o}"), 8);
    auto r2 = fmt::format(FMT_STRING("{:#o}"), 8);
    auto r3 = fmt::format(FMT_STRING("{:o}"), 0755);
    ssgx::utils_t::Printf("  fmt(\"{:o}\",  8)     => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:#o}\", 8)     => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:o}\",  0755)  => \"%s\"\n", r3.c_str());
    ASSERT_STR_EQ(r1, "10");
    ASSERT_STR_EQ(r2, "010");
    ASSERT_STR_EQ(r3, "755");
}

TEST(FmtRadix, Binary) {
    auto r1 = fmt::format(FMT_STRING("{:b}"), 10);
    auto r2 = fmt::format(FMT_STRING("{:#b}"), 10);
    auto r3 = fmt::format(FMT_STRING("{:08b}"), 42);
    ssgx::utils_t::Printf("  fmt(\"{:b}\",   10)  => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:#b}\",  10)  => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:08b}\", 42)  => \"%s\"\n", r3.c_str());
    ASSERT_STR_EQ(r1, "1010");
    ASSERT_STR_EQ(r2, "0b1010");
    ASSERT_STR_EQ(r3, "00101010");
}

// ─── FmtStr ──────────────────────────────────────────────────────────────────

TEST(FmtStr, StringView) {
    std::string_view sv = "hello, SGX";
    auto r = fmt::format(FMT_STRING("{}"), sv);
    ssgx::utils_t::Printf("  fmt(\"{}\", string_view(\"hello, SGX\"))  => \"%s\"\n", r.c_str());
    ASSERT_STR_EQ(r, "hello, SGX");
}

TEST(FmtStr, StdString) {
    std::string s = "world";
    auto r1 = fmt::format(FMT_STRING("hello, {}!"), s);
    auto r2 = fmt::format(FMT_STRING("{:>10}"), s);
    ssgx::utils_t::Printf("  fmt(\"hello, {}!\", \"world\")  => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:>10}\",   \"world\")  => \"%s\"\n", r2.c_str());
    ASSERT_STR_EQ(r1, "hello, world!");
    ASSERT_STR_EQ(r2, "     world");
}

TEST(FmtStr, EscapedBraces) {
    auto r1 = fmt::format(FMT_STRING("{{}}"));          // literal "{}"
    auto r2 = fmt::format(FMT_STRING("{{{0}}}"), 42);   // "{42}"
    auto r3 = fmt::format(FMT_STRING("a{{b}}c{}"), 1);  // "a{b}c1"
    ssgx::utils_t::Printf("  fmt(\"{{}}\")           => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{{{{{0}}}}}\", 42) => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  fmt(\"a{{b}}c{}\", 1)   => \"%s\"\n", r3.c_str());
    ASSERT_STR_EQ(r1, "{}");
    ASSERT_STR_EQ(r2, "{42}");
    ASSERT_STR_EQ(r3, "a{b}c1");
}

TEST(FmtStr, ToStringHelper) {
    auto r1 = fmt::to_string(42);
    auto r2 = fmt::to_string(3.14f);
    auto r3 = fmt::to_string(true);
    ssgx::utils_t::Printf("  fmt::to_string(42)     => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt::to_string(3.14f)  => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  fmt::to_string(true)   => \"%s\"\n", r3.c_str());
    ASSERT_STR_EQ(r1, "42");
    ASSERT_STR_EQ(r3, "true");
    // 3.14f string representation is implementation-defined, just check non-empty
    ASSERT_EQ(r2.empty(), false);
}

// ─── FmtNamed ────────────────────────────────────────────────────────────────

TEST(FmtNamed, BasicNamedArg) {
    auto r = fmt::format(FMT_STRING("{name} is {age}"),
                         fmt::arg("name", "Alice"), fmt::arg("age", 30));
    ssgx::utils_t::Printf("  fmt(\"{name} is {age}\", name=\"Alice\", age=30)  => \"%s\"\n",
                           r.c_str());
    ASSERT_STR_EQ(r, "Alice is 30");
}

TEST(FmtNamed, MixedPositionalAndNamed) {
    auto r = fmt::format(FMT_STRING("Hello {name}!"), fmt::arg("name", "SGX"));
    ssgx::utils_t::Printf("  fmt(\"Hello {name}!\", name=\"SGX\")  => \"%s\"\n", r.c_str());
    ASSERT_STR_EQ(r, "Hello SGX!");
}

TEST(FmtNamed, NamedWithFormatSpec) {
    auto r = fmt::format(FMT_STRING("{val:08x}"), fmt::arg("val", 0xABCD));
    ssgx::utils_t::Printf("  fmt(\"{val:08x}\", val=0xABCD)  => \"%s\"\n", r.c_str());
    ASSERT_STR_EQ(r, "0000abcd");
}

// ─── FmtOutput ───────────────────────────────────────────────────────────────

TEST(FmtOutput, FormatTo) {
    std::string buf;
    fmt::format_to(std::back_inserter(buf), FMT_STRING("x={}"), 99);
    ssgx::utils_t::Printf("  format_to(buf, \"x={}\", 99)  => \"%s\"\n", buf.c_str());
    ASSERT_STR_EQ(buf, "x=99");
}

TEST(FmtOutput, FormatToAppend) {
    std::string buf = "prefix:";
    fmt::format_to(std::back_inserter(buf), FMT_STRING("{},{}"), 1, 2);
    fmt::format_to(std::back_inserter(buf), FMT_STRING(",{}"), 3);
    ssgx::utils_t::Printf("  append format_to  => \"%s\"\n", buf.c_str());
    ASSERT_STR_EQ(buf, "prefix:1,2,3");
}

TEST(FmtOutput, FormatToN) {
    char buf[9] = {};
    auto res = fmt::format_to_n(buf, sizeof(buf) - 1, FMT_STRING("{:08b}"), 42);
    buf[res.size] = '\0';
    ssgx::utils_t::Printf("  format_to_n(buf, 8, \"{:08b}\", 42)  => \"%s\"  (size=%zu)\n",
                           buf, res.size);
    ASSERT_STR_EQ(std::string(buf), "00101010");
}

TEST(FmtOutput, FormatToNTruncates) {
    char buf[6] = {};
    auto res = fmt::format_to_n(buf, 5, FMT_STRING("{}"), "hello world");
    buf[5] = '\0';
    ssgx::utils_t::Printf("  format_to_n truncate \"hello world\" to 5  => \"%s\"  (total=%zu)\n",
                           buf, res.size);
    ASSERT_STR_EQ(std::string(buf), "hello");
    ASSERT_EQ(res.size, std::size_t(11));  // total (untruncated) length of "hello world"
}

TEST(FmtOutput, FormattedSize) {
    auto s1 = fmt::formatted_size(FMT_STRING("{}"), 12345);
    auto s2 = fmt::formatted_size(FMT_STRING("{:.3f}"), 3.14);
    auto s3 = fmt::formatted_size(FMT_STRING("{:08b}"), 42);
    ssgx::utils_t::Printf("  formatted_size(\"{}\",    12345)  => %zu\n", s1);
    ssgx::utils_t::Printf("  formatted_size(\"{:.3f}\",3.14)   => %zu\n", s2);
    ssgx::utils_t::Printf("  formatted_size(\"{:08b}\",42)     => %zu\n", s3);
    ASSERT_EQ(s1, std::size_t(5));
    ASSERT_EQ(s2, std::size_t(5));
    ASSERT_EQ(s3, std::size_t(8));
}

// ─── FmtBuffer ───────────────────────────────────────────────────────────────

TEST(FmtBuffer, MemoryBuffer) {
    fmt::memory_buffer buf;
    fmt::format_to(fmt::appender(buf), FMT_STRING("Hello, {}!"), "buffer");
    std::string result(buf.data(), buf.size());
    ssgx::utils_t::Printf("  memory_buffer: \"Hello, buffer!\"  => \"%s\"\n", result.c_str());
    ASSERT_STR_EQ(result, "Hello, buffer!");
}

TEST(FmtBuffer, MemoryBufferAppendMultiple) {
    fmt::memory_buffer buf;
    for (int i = 1; i <= 5; ++i) {
        fmt::format_to(fmt::appender(buf), FMT_STRING("{}"), i);
        if (i < 5) fmt::format_to(fmt::appender(buf), FMT_STRING(","));
    }
    std::string result(buf.data(), buf.size());
    ssgx::utils_t::Printf("  memory_buffer append 1..5  => \"%s\"\n", result.c_str());
    ASSERT_STR_EQ(result, "1,2,3,4,5");
}

// ─── FmtDynamic ──────────────────────────────────────────────────────────────

TEST(FmtDynamic, DynamicArgStore) {
    fmt::dynamic_format_arg_store<fmt::format_context> store;
    store.push_back(10);
    store.push_back(std::string("hello"));
    auto r = fmt::vformat("{} {}", store);
    ssgx::utils_t::Printf("  vformat(\"{} {}\", {10, \"hello\"})  => \"%s\"\n", r.c_str());
    ASSERT_STR_EQ(r, "10 hello");
}

TEST(FmtDynamic, DynamicNamedArgs) {
    fmt::dynamic_format_arg_store<fmt::format_context> store;
    store.push_back(fmt::arg("val", 42));
    auto r = fmt::vformat("{val}", store);
    ssgx::utils_t::Printf("  vformat(\"{val}\", {val=42})  => \"%s\"\n", r.c_str());
    ASSERT_STR_EQ(r, "42");
}

TEST(FmtDynamic, BuildFormatStringAtRuntime) {
    // vformat with a runtime format string built dynamically
    std::string fmt_str = "{:";
    fmt_str += ">10}";
    std::string val = "sgx";
    auto r = fmt::vformat(fmt_str, fmt::make_format_args(val));
    ssgx::utils_t::Printf("  vformat(\"{:>10}\", \"sgx\") built at runtime  => \"%s\"\n",
                           r.c_str());
    ASSERT_STR_EQ(r, "       sgx");
}

TEST(FmtDynamic, DynamicMultipleNamedArgs) {
    fmt::dynamic_format_arg_store<fmt::format_context> store;
    store.push_back(fmt::arg("a", 3));
    store.push_back(fmt::arg("b", 4));
    store.push_back(fmt::arg("sum", 7));
    auto r = fmt::vformat("{a} + {b} = {sum}", store);
    ssgx::utils_t::Printf("  vformat(\"{a} + {b} = {sum}\", a=3,b=4,sum=7)  => \"%s\"\n",
                           r.c_str());
    ASSERT_STR_EQ(r, "3 + 4 = 7");
}

// ─── FmtRanges ───────────────────────────────────────────────────────────────

TEST(FmtRanges, FormatVector) {
    std::vector<int> v = {1, 2, 3};
    auto r = fmt::format(FMT_STRING("{}"), v);
    ssgx::utils_t::Printf("  fmt(\"{}\", {1,2,3})  => \"%s\"\n", r.c_str());
    ASSERT_STR_EQ(r, "[1, 2, 3]");
}

TEST(FmtRanges, FormatNestedVector) {
    std::vector<std::vector<int>> v = {{1, 2}, {3, 4}};
    auto r = fmt::format(FMT_STRING("{}"), v);
    ssgx::utils_t::Printf("  fmt(\"{}\", {{1,2},{3,4}})  => \"%s\"\n", r.c_str());
    ASSERT_STR_EQ(r, "[[1, 2], [3, 4]]");
}

TEST(FmtRanges, FormatArray) {
    std::array<int, 4> a = {10, 20, 30, 40};
    auto r = fmt::format(FMT_STRING("{}"), a);
    ssgx::utils_t::Printf("  fmt(\"{}\", array{10,20,30,40})  => \"%s\"\n", r.c_str());
    ASSERT_STR_EQ(r, "[10, 20, 30, 40]");
}

TEST(FmtRanges, FormatSet) {
    std::set<int> s = {3, 1, 4, 1, 5};  // duplicates removed, sorted
    auto r = fmt::format(FMT_STRING("{}"), s);
    ssgx::utils_t::Printf("  fmt(\"{}\", set{3,1,4,1,5})  => \"%s\"\n", r.c_str());
    ASSERT_STR_EQ(r, "{1, 3, 4, 5}");
}

TEST(FmtRanges, FormatMap) {
    std::map<int, int> m = {{1, 10}, {2, 20}, {3, 30}};
    auto r = fmt::format(FMT_STRING("{}"), m);
    ssgx::utils_t::Printf("  fmt(\"{}\", map{{1:10,2:20,3:30}})  => \"%s\"\n", r.c_str());
    ASSERT_STR_EQ(r, "{1: 10, 2: 20, 3: 30}");
}

TEST(FmtRanges, FormatPair) {
    auto p = std::make_pair(42, 3.14);
    auto r = fmt::format(FMT_STRING("{}"), p);
    ssgx::utils_t::Printf("  fmt(\"{}\", pair(42, 3.14))  => \"%s\"\n", r.c_str());
    ASSERT_STR_EQ(r, "(42, 3.14)");
}

TEST(FmtRanges, FormatTuple) {
    auto t = std::make_tuple(1, 2, 3);
    auto r1 = fmt::format(FMT_STRING("{}"), t);
    auto t2 = std::make_tuple(true, 42, 3.14);
    auto r2 = fmt::format(FMT_STRING("{}"), t2);
    ssgx::utils_t::Printf("  fmt(\"{}\", tuple(1,2,3))          => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{}\", tuple(true,42,3.14))   => \"%s\"\n", r2.c_str());
    ASSERT_STR_EQ(r1, "(1, 2, 3)");
    ASSERT_STR_EQ(r2, "(true, 42, 3.14)");
}

TEST(FmtRanges, JoinRange) {
    std::vector<int> v = {1, 2, 3};
    auto r = fmt::format(FMT_STRING("{}"), fmt::join(v, "-"));
    ssgx::utils_t::Printf("  fmt(\"{}\", join({1,2,3}, \"-\"))  => \"%s\"\n", r.c_str());
    ASSERT_STR_EQ(r, "1-2-3");
}

TEST(FmtRanges, JoinWithFormatSpec) {
    std::vector<int> bytes = {0xCA, 0xFE, 0xBA, 0xBE};
    auto r = fmt::format("{:02x}", fmt::join(bytes, ":"));
    ssgx::utils_t::Printf("  fmt(\"{:02x}\", join({0xCA,...}, \":\"))  => \"%s\"\n", r.c_str());
    ASSERT_STR_EQ(r, "ca:fe:ba:be");
}

TEST(FmtRanges, JoinStrings) {
    std::vector<std::string> words = {"the", "quick", "brown", "fox"};
    auto r = fmt::format(FMT_STRING("{}"), fmt::join(words, " "));
    ssgx::utils_t::Printf("  join words with space  => \"%s\"\n", r.c_str());
    ASSERT_STR_EQ(r, "the quick brown fox");
}

// ─── FmtCustom ───────────────────────────────────────────────────────────────

TEST(FmtCustom, CustomPoint) {
    Point p{3, 4};
    auto r = fmt::format(FMT_STRING("point: {}"), p);
    ssgx::utils_t::Printf("  fmt(\"point: {}\", Point{3,4})  => \"%s\"\n", r.c_str());
    ASSERT_STR_EQ(r, "point: (3, 4)");
}

TEST(FmtCustom, CustomVec3WithPrecision) {
    Vec3 v{1.0, 2.5, 3.333};
    auto r1 = fmt::format("{}", v);          // default prec=2
    auto r2 = fmt::format("{:4}", v);        // prec=4
    ssgx::utils_t::Printf("  fmt(\"{}\",  Vec3{1,2.5,3.333})  => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:4}\",Vec3{1,2.5,3.333})  => \"%s\"\n", r2.c_str());
    ASSERT_STR_EQ(r1, "(1.00, 2.50, 3.33)");
    ASSERT_STR_EQ(r2, "(1.0000, 2.5000, 3.3330)");
}

TEST(FmtCustom, CustomRgbSpec) {
    Rgb c{0xFF, 0x80, 0x00};
    auto r1 = fmt::format("{}", c);     // default: hex
    auto r2 = fmt::format("{:x}", c);  // explicit hex
    auto r3 = fmt::format("{:d}", c);  // decimal
    ssgx::utils_t::Printf("  fmt(\"{}\",  Rgb(255,128,0))  => \"%s\"\n", r1.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:x}\",Rgb(255,128,0))  => \"%s\"\n", r2.c_str());
    ssgx::utils_t::Printf("  fmt(\"{:d}\",Rgb(255,128,0))  => \"%s\"\n", r3.c_str());
    ASSERT_STR_EQ(r1, "#FF8000");
    ASSERT_STR_EQ(r2, "#FF8000");
    ASSERT_STR_EQ(r3, "rgb(255, 128, 0)");
}

TEST(FmtCustom, VectorOfCustomTypes) {
    std::vector<Point> pts = {{1, 2}, {3, 4}, {5, 6}};
    auto r = fmt::format(FMT_STRING("{}"), pts);
    ssgx::utils_t::Printf("  fmt(\"{}\", vector<Point>)  => \"%s\"\n", r.c_str());
    ASSERT_STR_EQ(r, "[(1, 2), (3, 4), (5, 6)]");
}

// ─── FmtCompile ──────────────────────────────────────────────────────────────

TEST(FmtCompile, FmtCompileMacro) {
    auto r = fmt::format(FMT_COMPILE("{}+{}={}"), 1, 2, 3);
    ssgx::utils_t::Printf("  fmt(FMT_COMPILE(\"{}+{}={}\"), 1, 2, 3)  => \"%s\"\n", r.c_str());
    ASSERT_STR_EQ(r, "1+2=3");
}

TEST(FmtCompile, FormatToCompiled) {
    std::string buf;
    fmt::format_to(std::back_inserter(buf), FMT_COMPILE("{:>10}"), "sgx");
    ssgx::utils_t::Printf("  format_to(FMT_COMPILE(\"{:>10}\"), \"sgx\")  => \"%s\"\n",
                           buf.c_str());
    ASSERT_STR_EQ(buf, "       sgx");
}

TEST(FmtCompile, CompiledFormattedSize) {
    auto n = fmt::formatted_size(FMT_COMPILE("{:08b}"), 255);
    ssgx::utils_t::Printf("  formatted_size(FMT_COMPILE(\"{:08b}\"), 255)  => %zu\n", n);
    ASSERT_EQ(n, std::size_t(8));
}

TEST(FmtCompile, CompiledFormatToN) {
    char buf[12] = {};
    auto res = fmt::format_to_n(buf, sizeof(buf) - 1,
                                FMT_COMPILE("{:010.3f}"), 3.14);
    buf[res.size] = '\0';
    ssgx::utils_t::Printf("  format_to_n FMT_COMPILE(\"{:010.3f}\"), 3.14  => \"%s\"\n", buf);
    ASSERT_STR_EQ(std::string(buf), "000003.140");
}

// ─── FmtError ────────────────────────────────────────────────────────────────

TEST(FmtError, InvalidFormatString) {
    ssgx::utils_t::Printf("  fmt::format(runtime(\"{\"), 1)  => expect format_error\n");
    ASSERT_THROW(fmt::format(fmt::runtime("{"), 1), fmt::format_error);
}

TEST(FmtError, ArgIndexOutOfRange) {
    ssgx::utils_t::Printf("  fmt::format(runtime(\"{1}\"), 42)  => expect format_error\n");
    ASSERT_THROW(fmt::format(fmt::runtime("{1}"), 42), fmt::format_error);
}

TEST(FmtError, InvalidSpecForType) {
    // {:d} is invalid for a string argument
    ssgx::utils_t::Printf("  fmt::format(runtime(\"{:d}\"), \"str\")  => expect format_error\n");
    ASSERT_THROW(fmt::format(fmt::runtime("{:d}"), std::string("str")), fmt::format_error);
}

TEST(FmtError, UnmatchedBrace) {
    ssgx::utils_t::Printf("  fmt::format(runtime(\"}\"))  => expect format_error\n");
    ASSERT_THROW(fmt::format(fmt::runtime("}")), fmt::format_error);
}

TEST(FmtError, TooFewArgs) {
    ssgx::utils_t::Printf("  fmt::format(runtime(\"{0} {1}\"), 42)  => expect format_error\n");
    ASSERT_THROW(fmt::format(fmt::runtime("{0} {1}"), 42), fmt::format_error);
}
