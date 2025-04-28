#include <memory>
#include <cstring>

#include "ssgx_testframework_t.h"
#include "ssgx_utils_t.h"

using namespace ssgx::utils_t;

// ---- FormatStr Tests ----

TEST(FormatStrTest, FormatInt) {
    std::string result = FormatStr("Int: %d", 42);
    ASSERT_EQ(result, "Int: 42");
}

TEST(FormatStrTest, FormatNegativeInt) {
    std::string result = FormatStr("Int: %d", -999);
    ASSERT_EQ(result, "Int: -999");
}

TEST(FormatStrTest, FormatHexOctBin) {
    std::string result = FormatStr("Hex: %x Oct: %o", 255, 255);
    ASSERT_EQ(result, "Hex: ff Oct: 377");
}

TEST(FormatStrTest, FormatUnsigned) {
    std::string result = FormatStr("Unsigned: %u", 4294967295U);
    ASSERT_EQ(result, "Unsigned: 4294967295");
}

TEST(FormatStrTest, FormatString) {
    std::string result = FormatStr("Name: %s", "Alice");
    ASSERT_EQ(result, "Name: Alice");
}

TEST(FormatStrTest, FormatEmptyString) {
    std::string result = FormatStr("Empty: %s", "");
    ASSERT_EQ(result, "Empty: ");
}

TEST(FormatStrTest, FormatFloat) {
    std::string result = FormatStr("Pi = %.2f", 3.14159);
    ASSERT_STR_EQ(result.c_str(), "Pi = 3.14");
}

TEST(FormatStrTest, FormatScientific) {
    std::string result = FormatStr("Exp: %e", 0.00123);
    ASSERT_STR_EQ(result.c_str(), "Exp: 1.230000e-03");
}

TEST(FormatStrTest, FormatChar) {
    std::string result = FormatStr("Char: %c", 'Z');
    ASSERT_EQ(result, "Char: Z");
}

TEST(FormatStrTest, FormatPercentLiteral) {
    std::string result = FormatStr("100%% sure");
    ASSERT_EQ(result, "100% sure");
}

TEST(FormatStrTest, FormatMultipleArgs) {
    std::string result = FormatStr("Name: %s, Age: %d, Score: %.1f", "Bob", 23, 98.5);
    ASSERT_EQ(result, "Name: Bob, Age: 23, Score: 98.5");
}

TEST(FormatStrTest, FormatWithPadding) {
    std::string result = FormatStr("Padded: %5d", 42);
    ASSERT_EQ(result, "Padded:    42");
}

TEST(FormatStrTest, FormatZeroPadding) {
    std::string result = FormatStr("ZeroPad: %04d", 7);
    ASSERT_EQ(result, "ZeroPad: 0007");
}

TEST(FormatStrTest, FormatDynamicWidth) {
    std::string result = FormatStr("Width: %*d", 6, 123);
    ASSERT_EQ(result, "Width:    123");
}

TEST(FormatStrTest, FormatNewlines) {
    std::string result = FormatStr("Line1\nLine2\n");
    ASSERT_STR_EQ(result.c_str(), "Line1\nLine2\n");
}

TEST(FormatStrTest, FormatLargeString) {
    std::string big(1000, 'x');
    std::string result = FormatStr("Msg: %s", big.c_str());
    ASSERT_TRUE(result.size() > 1000);
}

TEST(FormatStrTest, FormatStrInvalidFormat) {
    ASSERT_THROW(FormatStr("Unknown: %q", 123), std::runtime_error);
}

TEST(FormatStrTest, FormatStrMissingArgument) {
    ASSERT_THROW(FormatStr("Missing %d and %s", 123), std::runtime_error);
}

TEST(FormatStrTest, FormatStrExtraArgument) {
    std::string result = FormatStr("Only one: %s", "yes", 42, "extra");
    ASSERT_STR_EQ(result.c_str(), "Only one: yes");
}

TEST(FormatStrTest, FormatLongLine) {
    std::string line;
    for (int i = 0; i < 100; ++i) line += FormatStr("Num:%d ", i);
    ASSERT_TRUE(line.find("Num:99") != std::string::npos);
}

// ---- Printf Tests ----

TEST(PrintfTest, PrintfBasic) {
    int ret = Printf("Hello, %s!\n", "World");
    ASSERT_GT(ret, 0);
}

TEST(PrintfTest, PrintfWithMixedTypes) {
    int ret = Printf("Pi: %.3f, Int: %d, Hex: %X\n", 3.14159, 42, 42);
    ASSERT_GT(ret, 0);
}

TEST(PrintfTest, PrintfNewlines) {
    int ret = Printf("Line1\nLine2\nLine3\n");
    ASSERT_GT(ret, 0);
}

TEST(PrintfTest, PrintfEmpty) {
    int ret = Printf("");
    ASSERT_EQ(ret, 0);
}
