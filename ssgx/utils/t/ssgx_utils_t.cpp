#include <stdexcept>
#include <string>
#include <vector>

#include "sgx_edger8r.h"
#include "sgx_eid.h"
#include "sgx_lfence.h"
#include "sgx_trts.h"

#include "ssgx_utils_t.h"
#include "ssgx_utils_t_t.h"

using ssgx::utils_t::Printf;
using ssgx::utils_t::FormatStr;
using ssgx::utils_t::MallocOutside;
using ssgx::utils_t::CallocOutside;
using ssgx::utils_t::FreeOutside;
using ssgx::utils_t::EnclaveInfo;

#define MAX_BUF_LEN 4 * 1024

namespace ssgx {
namespace utils_t {

int Printf(const char* fmt, ...) {
    if (!fmt)
        return -900001;

    char buf[MAX_BUF_LEN] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    // If an encoding error occurs, a negative number is returned.
    int ret = vsnprintf(buf, MAX_BUF_LEN, fmt, ap);
    va_end(ap);
    if (ret < 0) {
        return -900002; // Error code for format failure
    }
    sgx_status_t status = ssgx_ocall_printf(&ret, buf);
    if (status != SGX_SUCCESS)
        return -900003;
    int expected_len = (int)strnlen(buf, MAX_BUF_LEN);
    // On success, the total number of characters written is returned.
    // On failure, a negative number is returned.
    if (ret < 0 || ret == expected_len)
        return ret;
    // Some error unexpected in untrusted code.
    return -900004;
}

std::string FormatStr(const char* fmt, ...) {
    if (!fmt)
        throw std::invalid_argument("Format string is null");

    /**
     * int vsnprintf( char* restrict buffer, size_t bufsz, const char* restrict format, va_list vlist ); (4)	(since
     * C99) See: https://en.cppreference.com/w/c/io/vfprintf
     *
     * Writes the results to a character string buffer. At most bufsz - 1 characters are written.
     * The resulting character string will be terminated with a null character, unless bufsz is zero.
     * If bufsz is zero, nothing is written and buffer may be a null pointer, however the return value (number of bytes
     * that would be written not including the null terminator) is still calculated and returned.
     *
     * The number of characters written if successful or negative value if an error occurred.
     */
    va_list ap;
    va_start(ap, fmt);
    int size_s = vsnprintf(nullptr, 0, fmt, ap);
    va_end(ap);

    if (size_s < 0) {
        throw std::runtime_error("Formatting error in format_str");
    }

    size_t size = static_cast<size_t>(size_s) + 1;
    std::vector<char> buf(size);

    va_start(ap, fmt);
    vsnprintf(buf.data(), size, fmt, ap);
    va_end(ap);

    return std::string(buf.data(), size - 1);
}

void* MallocOutside(size_t size) {
    if (size == 0)
        return nullptr;

    uint8_t* outside_buf = nullptr;
    sgx_status_t status = ssgx_ocall_malloc(size, &outside_buf);
    if (status != SGX_SUCCESS || !outside_buf)
        return nullptr;

    if (sgx_is_outside_enclave(outside_buf, size) != 1) {
        throw std::runtime_error(
            "Failed in sgx_is_outside_enclave in func malloc_outside: sgx_is_outside_enclave(outside_buf, size) != 1");
    }

    sgx_lfence();

    memset(outside_buf, 0, size);

    return outside_buf;
}

void* CallocOutside(size_t num, size_t size) {
    if (size == 0 || num == 0)
        return nullptr;

    uint8_t* outside_buf = nullptr;
    sgx_status_t status = ssgx_ocall_calloc(num, size, &outside_buf);
    if (status != SGX_SUCCESS || !outside_buf)
        return nullptr;

    if (sgx_is_outside_enclave(outside_buf, num * size) != 1) {
        throw std::runtime_error(
            "Failed in sgx_is_outside_enclave in func calloc_outside: sgx_is_outside_enclave(outside_buf, size) != 1");
    }
    sgx_lfence();

    memset(outside_buf, 0, num * size);

    return outside_buf;
}

void FreeOutside(void* ptr_outside, size_t size) {
    if (ptr_outside != nullptr && sgx_is_outside_enclave(ptr_outside, size) == 1) {
        ssgx_ocall_free((uint8_t*)ptr_outside);
        ptr_outside = nullptr;
    }
}

void* MemcpyToOutside(void* dest_outside_enclave, const void* src, std::size_t count) {
    if (dest_outside_enclave == nullptr || (sgx_is_outside_enclave(dest_outside_enclave, count) != 1)) {
        throw std::invalid_argument(std::string("Failed in \"dest_outside_enclave != nullptr && "
                                                "sgx_is_outside_enclave(dest_outside_enclave, count) == 0\" "));
    }
    sgx_lfence();
    return memcpy(dest_outside_enclave, src, count);
}

char* StrndupOutside(const char* str_ptr, std::size_t str_len) {
    if (!str_ptr) {
        return nullptr;
    }

    if(strnlen(str_ptr, str_len + 1) != str_len) return nullptr;

    size_t alloc_size = str_len + 1;
    auto outside_str = static_cast<char *>(MallocOutside(alloc_size));
    if (!outside_str) {
        return nullptr;
    }

    memcpy(outside_str, str_ptr, str_len);
    outside_str[str_len] = '\0';
    return outside_str;
}

void* MemdupOutside(const void* buf_ptr, std::size_t buf_size) {
    if (!buf_ptr || buf_size == 0) {
        return nullptr;
    }

    void* outside_buf = MallocOutside(buf_size);
    if (!outside_buf) {
        return nullptr;
    }

    memcpy(outside_buf, buf_ptr, buf_size);
    return outside_buf;
}

void Sleep(uint32_t seconds) {
    // Use OCall to invoke the host's sleep function
    ssgx_ocall_sleep(seconds);
}

sgx_enclave_id_t get_enclave_eid() {
    return EnclaveInfo::GetCurrentEnclave().GetEnclaveEid();
}

} // namespace utils_t
} // namespace ssgx