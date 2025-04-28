#include <cstdint>
#include <stdexcept>
#include "sgx_error.h"

#include "ssgx_utils_t_t.h"
#include "ssgx_utils_t_time.h"

#include "TimeVerifier.h"

namespace ssgx {
namespace utils_t {

int64_t PreciseTime::NowInNanoseconds() {
    static TimeVerifier nanosecond_time_verifier;

    uint64_t t_now = 0;
    sgx_status_t status = ssgx_ocall_time_in_nanoseconds(&t_now);

    if (status != SGX_SUCCESS) {
        throw std::runtime_error(std::string("Failed in ssgx_ocall_time_in_nanoseconds (error code: ") +
                                 std::to_string(status) + ")");
    }

    if (!nanosecond_time_verifier.Verify(t_now)) {
        throw std::runtime_error(
            std::string("Failed in ssgx_ocall_time_in_nanoseconds (invalid time from untrusted system)"));
    }

    return static_cast<int64_t>(t_now);
}

int64_t PreciseTime::NowInMilliseconds() {
    static TimeVerifier milliseconds_time_verifier;

    uint64_t t_now = 0;
    sgx_status_t status = ssgx_ocall_time_in_milliseconds(&t_now);

    if (status != SGX_SUCCESS) {
        throw std::runtime_error(std::string("Failed in ssgx_ocall_time_in_milliseconds (error code: ") +
                                 std::to_string(status) + ")");
    }

    if (!milliseconds_time_verifier.Verify(t_now)) {
        throw std::runtime_error(
            std::string("Failed in ssgx_ocall_time_in_milliseconds (invalid time from untrusted system)"));
    }

    return static_cast<int64_t>(t_now);
}

} // namespace utils_t
} // namespace ssgx