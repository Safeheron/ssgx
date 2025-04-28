#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include "ssgx_testframework_t.h"

// Test suite for string equality and inequality assertions
TEST(TTestFramework, StringTests_StringEquality) {
    // ASSERT_STR_EQ
    ASSERT_STR_EQ("hello", "hello");              // Pass
    ASSERT_STR_EQ(std::string("test"), "test");   // Pass
    ASSERT_STR_EQ("world", std::string("world")); // Pass
    // ASSERT_STR_EQ("foo", "bar");                       // Uncomment to test failure

    // ASSERT_STR_NE
    ASSERT_STR_NE("hello", "world");          // Pass
    ASSERT_STR_NE(std::string("foo"), "bar"); // Pass
    ASSERT_STR_NE("abc", std::string("xyz")); // Pass
    // ASSERT_STR_NE("test", "test");                     // Uncomment to test failure
}

// Test suite for binary content equality and inequality
TEST(TTestFramework, BinaryTests_ByteContentEquality) {
    std::string str1 = "\x01\x02\x03";
    std::string str2 = "\x01\x02\x03";
    std::string str3 = "\x01\x02\x04";

    // ASSERT_BYTES_EQ
    ASSERT_BYTES_EQ(str1, str2); // Pass
    // ASSERT_BYTES_EQ(str1, str3);                       // Uncomment to test failure

    // ASSERT_BYTES_NE
    ASSERT_BYTES_NE(str1, str3); // Pass
    // ASSERT_BYTES_NE(str1, str2);                       // Uncomment to test failure
}

// Test suite for memory block equality and inequality
TEST(TTestFramework, MemoryTests_MemoryBlockEquality) {
    char block1[5] = {1, 2, 3, 4, 5};
    char block2[5] = {1, 2, 3, 4, 5};
    char block3[5] = {5, 4, 3, 2, 1};
    char block4[3] = {1, 2, 3};

    // ASSERT_MEM_EQ
    ASSERT_MEM_EQ(block1, 5, block2, 5); // Pass
    // ASSERT_MEM_EQ(block1, 5, block3, 5);               // Uncomment to test failure
    // ASSERT_MEM_EQ(block1, 5, block4, 3);               // Uncomment to test failure (length mismatch)

    // ASSERT_MEM_NE
    ASSERT_MEM_NE(block1, 5, block3, 5); // Pass
    ASSERT_MEM_NE(block1, 5, block4, 3); // Pass (length mismatch)
    // ASSERT_MEM_NE(block1, 5, block2, 5);               // Uncomment to test failure
}

TEST(ExceptionTests, ThrowCorrectException) {
    ASSERT_THROW(throw std::runtime_error("Test exception"), std::runtime_error);
}

TEST(ExceptionTests, ThrowWrongException) {
    // This will fail, because the thrown exception is not of type std::logic_error.
    // ASSERT_THROW(throw std::runtime_error("Test exception"), std::logic_error);
}

TEST(ExceptionTests, NoExceptionThrown) {
    // This will fail, because no exception is thrown.
    // ASSERT_THROW(int a = 42; (void)a;, std::runtime_error);
    ASSERT_NO_THROW(int a = 42; (void)a;);
}

TEST(NoThrowTests, SimpleExpression) {
    ASSERT_NO_THROW(int a = 42; int b = a + 1;);
}

TEST(NoThrowTests, SortVector) {
    std::vector<int> vec = {3, 1, 4, 1, 5};
    ASSERT_NO_THROW(std::sort(vec.begin(), vec.end()););
}