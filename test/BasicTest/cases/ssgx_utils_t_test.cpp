#include <cstring>
#include <stdexcept>

#include "ssgx_testframework_t.h"
#include "ssgx_utils_t.h"

using namespace ssgx::utils_t;

TEST(UtilsTestSuite, TestPrintf) {
    int ret = ssgx::utils_t::Printf("Hello, %s!\n", "World");
    ASSERT_GT(ret, 0);
}

TEST(UtilsTestSuite, TestFormatStr) {
    std::string result = FormatStr("Value: %d", 42);
    ASSERT_EQ(result, "Value: 42");
}

TEST(UtilsTestSuite, TestMallocOutside) {
    size_t size = 100;
    void* ptr = MallocOutside(size);
    ASSERT_NE(ptr, nullptr); // The allocated pointer should not be nullptr
    memset(ptr, 0, size);
    FreeOutside(ptr, size);
}

TEST(UtilsTestSuite, TestCallocOutside) {
    // Test memory allocation of calloc_outside function
    size_t num = 10;
    size_t size = sizeof(int);
    int* ptr = static_cast<int*>(CallocOutside(num, size));
    ASSERT_NE(ptr, nullptr); // The allocated pointer should not be nullptr
    // Check if the allocated memory is initialized to zero
    for (size_t i = 0; i < num; ++i) {
        ASSERT_EQ(ptr[i], 0);
    }
    // Free the memory
    FreeOutside(ptr, num * size);
}

TEST(UtilsTestSuite, TestMemcpyToOutside) {
    const char* src = "Test data";
    size_t len = strlen(src) + 1;
    char* dest = static_cast<char*>(MallocOutside(len));
    ASSERT_NE(dest, nullptr); // The allocated pointer should not be nullptr
    MemcpyToOutside(dest, src, len);
    ASSERT_TRUE(strlen(dest) == strlen(src) && 0 == memcmp(dest, src, strlen(src)));
    FreeOutside(dest, len);
}

TEST(UtilsTestSuite, TestSleep) {
    int64_t start = PreciseTime::NowInNanoseconds();
    ssgx::utils_t::Printf("Sleeping for 5s...\n");
    ssgx::utils_t::Sleep(5); // Assuming a custom sleep function

    int64_t end = PreciseTime::NowInNanoseconds();

    ssgx::utils_t::Printf("end - start = %zu\n", end - start);
    ASSERT_GE(end - start, 5 * (int64_t)1000000000); // 5s in nanoseconds
}
