#ifndef SAFEHERON_SGX_TEST_FRAMEWORK_H
#define SAFEHERON_SGX_TEST_FRAMEWORK_H

#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "ssgx_utils_t.h"

namespace ssgx {
/**
 * @namespace ssgx::testframework_t
 * @brief This module is a testing framework designed for test cases within the
 * enclave.
 */
namespace testframework_t {

#define ANSI_ESCAPE_CODE_RESET "\033[0m"
#define ANSI_ESCAPE_CODE_RED "\033[31m"
#define ANSI_ESCAPE_CODE_GREEN "\033[32m"
#define ANSI_ESCAPE_CODE_YELLOW "\033[33m"
#define ANSI_ESCAPE_CODE_BLUE "\033[34m"

/**
 * @brief Internal exceptions thrown by the test framework.
 */
struct InternalAssertionException : public std::runtime_error {
    InternalAssertionException(const std::string& msg, const char* file, int line, const char* func)
        : std::runtime_error(FormatMsg(msg, file, line, func)) {
    }

  private:
    static std::string FormatMsg(const std::string& msg, const char* file, int line, const char* func) {
        return std::string(ANSI_ESCAPE_CODE_RED "[ASSERT FAILED]" ANSI_ESCAPE_CODE_RESET " ") + msg + " @ " + file +
               ":" + std::to_string(line) + " in " + func + "()";
    }
};


/**
 * @brief Manages registration and execution of test cases grouped into suites.
 *
 * @details This class provides a simple, static test framework infrastructure. It allows
 * users to define test functions and register them under specific test case names
 * within named test suites. It offers functionalities to run all tests within a
 * specific suite or to run all registered suites sequentially.
 *
 */
class TestManager {
  public:
    using TestFunc = std::function<void()>;

    /**
     * @brief Registers a test case with the framework.
     *
     * @details Associates a test function (`func`) with a specific test name (`testName`)
     * within a named test suite (`suiteName`). If the suite does not exist, it is created.
     * If the suite already exists, the test case is added to it.
     * This function should be called during test setup or initialization phase.
     *
     * @param[in] suiteName The name of the test suite to which this test belongs.
     * @param[in] testName The specific name identifying this test case within the suite.
     * @param[in] func The function object (lambda, function pointer, std::function) containing the test logic.
     *
     * @note Tests are stored internally and executed later via RunSuite() or RunAllSuites().
     */
    static void AddTest(const char* suiteName, const char* testName, TestFunc func) {
        getSuites()[suiteName].emplace_back(testName, func);
    }

    /**
     * @brief Executes all registered test cases within a specific test suite.
     *
     * @details Looks up the specified suite name and runs each registered test case
     * sequentially. It captures exceptions (`std::exception` and others) to determine
     * test failure. Results, including PASS/FAIL status, test name, and execution
     * time (in milliseconds), are printed to standard output. A summary for the
     * suite (total passed/failed) is printed at the end.
     *
     * @param[in] suiteName The name of the test suite to execute.
     *
     */
    static void RunSuite(const char* suiteName) {
        const auto& suites = getSuites();
        if (suites.find(suiteName) == suites.end()) {
            ssgx::utils_t::Printf(ANSI_ESCAPE_CODE_RED "No tests found for suite: %s\n" ANSI_ESCAPE_CODE_RESET,
                                  suiteName);
            return;
        }

        size_t passed = 0, failed = 0;

        ssgx::utils_t::Printf(
            ANSI_ESCAPE_CODE_BLUE "\n========== Running Suite: %s ==========\n" ANSI_ESCAPE_CODE_RESET, suiteName);

        for (const auto& [name, test] : suites.at(suiteName)) {
            auto start = ssgx::utils_t::PreciseTime::NowInMilliseconds();
            try {
                test();
                auto end = ssgx::utils_t::PreciseTime::NowInMilliseconds();
                auto duration = end - start;
                ssgx::utils_t::Printf(ANSI_ESCAPE_CODE_GREEN "[PASS]" ANSI_ESCAPE_CODE_RESET " %-30s (Time: %lldms)\n",
                                      name, duration);
                ++passed;
            } catch (const std::exception& ex) {
                auto end = ssgx::utils_t::PreciseTime::NowInMilliseconds();
                auto duration = end - start;
                ssgx::utils_t::Printf(ANSI_ESCAPE_CODE_RED "[FAIL]" ANSI_ESCAPE_CODE_RESET
                                                           " %-30s: %s (Time: %lldms)\n",
                                      name, ex.what(), duration);
                ++failed;
            } catch (...) {
                auto end = ssgx::utils_t::PreciseTime::NowInMilliseconds();
                auto duration = end - start;
                ssgx::utils_t::Printf(ANSI_ESCAPE_CODE_RED "[FAIL]" ANSI_ESCAPE_CODE_RESET
                                                           " %-30s: Unknown exception (Time: %lldms)\n",
                                      name, duration);
                ++failed;
            }
        }

        ssgx::utils_t::Printf(ANSI_ESCAPE_CODE_BLUE "========== Suite Summary ==========\n" ANSI_ESCAPE_CODE_RESET);
        ssgx::utils_t::Printf(ANSI_ESCAPE_CODE_GREEN "Passed: %zu" ANSI_ESCAPE_CODE_RESET ", " ANSI_ESCAPE_CODE_RED
                                                     "Failed: %zu\n" ANSI_ESCAPE_CODE_RESET,
                              passed, failed);
    }

    /**
     * @brief Executes all registered test suites and their test cases.
     *
     * @details Iterates through all test suites that have been registered via `AddTest`.
     * For each suite, it runs all its test cases sequentially, printing individual
     * PASS/FAIL results and timing information, followed by a summary for that suite.
     * After all suites have been executed, a final summary showing the total number
     * of passed and failed tests across all suites is printed.
     *
     * @return Returns 0 if all test cases across all suites passed; returns -1 if one or more test cases failed.
     *
     */
    static int RunAllSuites() {
        size_t totalPassed = 0, totalFailed = 0;
        ssgx::utils_t::Printf(ANSI_ESCAPE_CODE_BLUE "\n========== Running All Test Suites "
                                                    "==========\n" ANSI_ESCAPE_CODE_RESET);

        for (const auto& [suiteName, tests] : getSuites()) {
            ssgx::utils_t::Printf(ANSI_ESCAPE_CODE_BLUE "\n---------- Suite: %s ----------\n" ANSI_ESCAPE_CODE_RESET,
                                  suiteName.c_str());
            size_t passed = 0, failed = 0;

            for (const auto& [name, test] : tests) {
                auto start = ssgx::utils_t::PreciseTime::NowInMilliseconds();
                try {
                    test();
                    auto end = ssgx::utils_t::PreciseTime::NowInMilliseconds();
                    auto duration = end - start;
                    ssgx::utils_t::Printf(ANSI_ESCAPE_CODE_GREEN "[PASS]" ANSI_ESCAPE_CODE_RESET
                                                                 " %-30s (Time: %lldms)\n",
                                          name, duration);
                    ++passed;
                } catch (const std::exception& ex) {
                    auto end = ssgx::utils_t::PreciseTime::NowInMilliseconds();
                    auto duration = end - start;
                    ssgx::utils_t::Printf(ANSI_ESCAPE_CODE_RED "[FAIL]" ANSI_ESCAPE_CODE_RESET
                                                               " %-30s: %s (Time: %lldms)\n",
                                          name, ex.what(), duration);
                    ++failed;
                } catch (...) {
                    auto end = ssgx::utils_t::PreciseTime::NowInMilliseconds();
                    auto duration = end - start;
                    ssgx::utils_t::Printf(ANSI_ESCAPE_CODE_RED "[FAIL]" ANSI_ESCAPE_CODE_RESET
                                                               " %-30s: Unknown exception (Time: %lldms)\n",
                                          name, duration);
                    ++failed;
                }
            }

            totalPassed += passed;
            totalFailed += failed;

            ssgx::utils_t::Printf(ANSI_ESCAPE_CODE_BLUE "\nSuite Summary for %s\n" ANSI_ESCAPE_CODE_RESET,
                                  suiteName.c_str());
            ssgx::utils_t::Printf(ANSI_ESCAPE_CODE_GREEN "Passed: %zu" ANSI_ESCAPE_CODE_RESET ", " ANSI_ESCAPE_CODE_RED
                                                         "Failed: %zu\n" ANSI_ESCAPE_CODE_RESET,
                                  passed, failed);
        }

        ssgx::utils_t::Printf(ANSI_ESCAPE_CODE_BLUE
                              "\n========== All Suites Summary ==========\n" ANSI_ESCAPE_CODE_RESET);
        ssgx::utils_t::Printf(ANSI_ESCAPE_CODE_GREEN "Total Passed: %zu" ANSI_ESCAPE_CODE_RESET
                                                     ", " ANSI_ESCAPE_CODE_RED
                                                     "Total Failed: %zu\n" ANSI_ESCAPE_CODE_RESET,
                              totalPassed, totalFailed);
        return totalFailed > 0 ? -1 : 0;
    }

  private:
    using Test = std::pair<const char*, TestFunc>;
    using SuiteMap = std::unordered_map<std::string, std::vector<Test>>;

    static SuiteMap& getSuites() {
        static SuiteMap suites;
        return suites;
    }
};

}; // namespace testframework_t
}; // namespace ssgx

#define TEST(suiteName, testName)                                                                                      \
    void suiteName##_##testName();                                                                                     \
    struct suiteName##_##testName##_Register {                                                                         \
        suiteName##_##testName##_Register() {                                                                          \
            ssgx::testframework_t::TestManager::AddTest(#suiteName, #testName, suiteName##_##testName);                \
        }                                                                                                              \
    } suiteName##_##testName##_register;                                                                               \
    void suiteName##_##testName()

#define ASSERT_TRUE(expr)                                                                                              \
    do {                                                                                                               \
        if (!(expr)) {                                                                                                 \
            throw ssgx::testframework_t::InternalAssertionException("ASSERT_TRUE failed: " #expr, __FILE__, __LINE__,  \
                                                                    __func__);                                         \
        }                                                                                                              \
    } while (0)

#define ASSERT_FALSE(expr)                                                                                             \
    do {                                                                                                               \
        if ((expr)) {                                                                                                  \
            throw ssgx::testframework_t::InternalAssertionException("ASSERT_FALSE failed: " #expr, __FILE__, __LINE__, \
                                                                    __func__);                                         \
        }                                                                                                              \
    } while (0)

#define ASSERT_EQ(a, b)                                                                                                \
    do {                                                                                                               \
        if (!((a) == (b))) {                                                                                           \
            throw ssgx::testframework_t::InternalAssertionException("ASSERT_EQ failed: " #a " == " #b, __FILE__,       \
                                                                    __LINE__, __func__);                               \
        }                                                                                                              \
    } while (0)

#define ASSERT_NE(a, b)                                                                                                \
    do {                                                                                                               \
        if (!((a) != (b))) {                                                                                           \
            throw ssgx::testframework_t::InternalAssertionException("ASSERT_NE failed: " #a " != " #b, __FILE__,       \
                                                                    __LINE__, __func__);                               \
        }                                                                                                              \
    } while (0)

#define ASSERT_LT(a, b)                                                                                                \
    do {                                                                                                               \
        if (!((a) < (b))) {                                                                                            \
            throw ssgx::testframework_t::InternalAssertionException("ASSERT_LT failed: " #a " < " #b, __FILE__,        \
                                                                    __LINE__, __func__);                               \
        }                                                                                                              \
    } while (0)

#define ASSERT_LE(a, b)                                                                                                \
    do {                                                                                                               \
        if (!((a) <= (b))) {                                                                                           \
            throw ssgx::testframework_t::InternalAssertionException("ASSERT_LE failed: " #a " <= " #b, __FILE__,       \
                                                                    __LINE__, __func__);                               \
        }                                                                                                              \
    } while (0)

#define ASSERT_GT(a, b)                                                                                                \
    do {                                                                                                               \
        if (!((a) > (b))) {                                                                                            \
            throw ssgx::testframework_t::InternalAssertionException("ASSERT_GT failed: " #a " > " #b, __FILE__,        \
                                                                    __LINE__, __func__);                               \
        }                                                                                                              \
    } while (0)

#define ASSERT_GE(a, b)                                                                                                \
    do {                                                                                                               \
        if (!((a) >= (b))) {                                                                                           \
            throw ssgx::testframework_t::InternalAssertionException("ASSERT_GE failed: " #a " >= " #b, __FILE__,       \
                                                                    __LINE__, __func__);                               \
        }                                                                                                              \
    } while (0)

#define ASSERT_NEAR(a, b, abs_error)                                                                                   \
    do {                                                                                                               \
        if (std::abs((a) - (b)) > abs_error) {                                                                         \
            throw ssgx::testframework_t::InternalAssertionException("ASSERT_NEAR failed: " #a " ~= " #b, __FILE__,     \
                                                                    __LINE__, __func__);                               \
        }                                                                                                              \
    } while (0)

#define ASSERT_STR_EQ(a, b)                                                                                            \
    do {                                                                                                               \
        if (std::string(a) != std::string(b)) {                                                                        \
            throw ssgx::testframework_t::InternalAssertionException("ASSERT_STR_EQ failed: \"" + std::string(a) +      \
                                                                        "\" != \"" + std::string(b) + "\"",            \
                                                                    __FILE__, __LINE__, __func__);                     \
        }                                                                                                              \
    } while (0)

#define ASSERT_STR_NE(a, b)                                                                                            \
    do {                                                                                                               \
        if (std::string(a) == std::string(b)) {                                                                        \
            throw ssgx::testframework_t::InternalAssertionException("ASSERT_STR_NE failed: \"" + std::string(a) +      \
                                                                        "\" == \"" + std::string(b) + "\"",            \
                                                                    __FILE__, __LINE__, __func__);                     \
        }                                                                                                              \
    } while (0)

#define ASSERT_BYTES_EQ(a, b)                                                                                          \
    do {                                                                                                               \
        if ((a).size() != (b).size() || std::memcmp((a).data(), (b).data(), (a).size()) != 0) {                        \
            throw ssgx::testframework_t::InternalAssertionException(                                                   \
                "ASSERT_BYTES_EQ failed: \"" + std::string((a).begin(), (a).end()) + "\" != \"" +                      \
                    std::string((b).begin(), (b).end()) + "\"",                                                        \
                __FILE__, __LINE__, __func__);                                                                         \
        }                                                                                                              \
    } while (0)

#define ASSERT_BYTES_NE(a, b)                                                                                          \
    do {                                                                                                               \
        if ((a).size() == (b).size() && std::memcmp((a).data(), (b).data(), (a).size()) == 0) {                        \
            throw ssgx::testframework_t::InternalAssertionException(                                                   \
                "ASSERT_BYTES_NE failed: \"" + std::string((a).begin(), (a).end()) + "\" == \"" +                      \
                    std::string((b).begin(), (b).end()) + "\"",                                                        \
                __FILE__, __LINE__, __func__);                                                                         \
        }                                                                                                              \
    } while (0)

#define ASSERT_MEM_EQ(ptr1, len1, ptr2, len2)                                                                          \
    do {                                                                                                               \
        if ((len1) != (len2)) {                                                                                        \
            throw ssgx::testframework_t::InternalAssertionException(                                                   \
                "ASSERT_MEM_EQ failed: Memory blocks have different lengths", __FILE__, __LINE__, __func__);           \
        }                                                                                                              \
        if (std::memcmp((ptr1), (ptr2), (len1)) != 0) {                                                                \
            throw ssgx::testframework_t::InternalAssertionException(                                                   \
                "ASSERT_MEM_EQ failed: Memory blocks are not equal", __FILE__, __LINE__, __func__);                    \
        }                                                                                                              \
    } while (0)

#define ASSERT_MEM_NE(ptr1, len1, ptr2, len2)                                                                          \
    do {                                                                                                               \
        if ((len1) == (len2) && std::memcmp((ptr1), (ptr2), (len1)) == 0) {                                            \
            throw ssgx::testframework_t::InternalAssertionException("ASSERT_MEM_NE failed: Memory blocks are equal",   \
                                                                    __FILE__, __LINE__, __func__);                     \
        }                                                                                                              \
    } while (0)

#define ASSERT_THROW(expr, exception_type)                                                                             \
    do {                                                                                                               \
        try {                                                                                                          \
            expr;                                                                                                      \
            throw ssgx::testframework_t::InternalAssertionException("ASSERT_THROW failed: No exception thrown",        \
                                                                    __FILE__, __LINE__, __func__);                     \
        } catch (const exception_type&) {                                                                              \
        } catch (...) {                                                                                                \
            throw ssgx::testframework_t::InternalAssertionException(                                                   \
                "ASSERT_THROW failed: Wrong exception type thrown", __FILE__, __LINE__, __func__);                     \
        }                                                                                                              \
    } while (0)

#define ASSERT_NO_THROW(expr)                                                                                          \
    do {                                                                                                               \
        try {                                                                                                          \
            expr;                                                                                                      \
        } catch (...) {                                                                                                \
            throw ssgx::testframework_t::InternalAssertionException("ASSERT_NO_THROW failed: Exception thrown",        \
                                                                    __FILE__, __LINE__, __func__);                     \
        }                                                                                                              \
    } while (0)

#define SUCCEED()                                                                                                      \
    do {                                                                                                               \
        ssgx::utils_t::Printf(ANSI_ESCAPE_CODE_GREEN "[SUCCEED]" ANSI_ESCAPE_CODE_RESET                              \
                              " %s:%d in %s()\n", __FILE__, __LINE__, __func__);                                       \
    } while (0)

#endif // SAFEHERON_SGX_TEST_FRAMEWORK_H
